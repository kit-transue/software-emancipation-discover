/*************************************************************************
* Copyright (c) 2015, Synopsys, Inc.                                     *
* All rights reserved.                                                   *
*                                                                        *
* Redistribution and use in source and binary forms, with or without     *
* modification, are permitted provided that the following conditions are *
* met:                                                                   *
*                                                                        *
* 1. Redistributions of source code must retain the above copyright      *
* notice, this list of conditions and the following disclaimer.          *
*                                                                        *
* 2. Redistributions in binary form must reproduce the above copyright   *
* notice, this list of conditions and the following disclaimer in the    *
* documentation and/or other materials provided with the distribution.   *
*                                                                        *
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS    *
* "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT      *
* LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR  *
* A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT   *
* HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, *
* SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT       *
* LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,  *
* DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY  *
* THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT    *
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE  *
* OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.   *
*************************************************************************/
/*
 * S Q L _ D R I V E R . C
 *
 * 1997 Software Emancipation Technology, Inc.
 *
 * Created Nov. 1997 Mark B. Kaminsky
 *
 * This file contains the main routine for the SQL (Structured Query
 * Language), ESQL (Embedded SQL -- C/C++ programs with SQL statements),
 * and PL/SQL (Procedural Language / SQL) parser and
 * ELS (External Language Support) IFF (Internal Format File)
 * generator.  It handles any command line options, opens input
 * and output files, and then calls the parser to process each
 * SQL program.  After each program is parsed it calls routines
 * finish the output.
 *
 * In the case of esql source files, a C/C++ file will be generated
 * with the sql statements removed (except for certain placeholders).
 *
 * This program is normally called as part of a model build (from the
 * els.dis script).  However, it can also be called standalone to
 * check SQL syntax, generate a symbols file, or for debugging to
 * see the IFF output or (in the case of esql) the host file output.
 */

#ifndef ISO_CPP_HEADERS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#else /* ISO_CPP_HEADERS */
#include <cstdio>
using namespace std;
#include <cstdlib>
#include <cstring>
#include <cerrno>
#endif /* ISO_CPP_HEADERS */
#include <sys/types.h>
#include <sys/stat.h>

#include "machdep.h"

#include "sql_scanner.h"		// scanner/parser entry
#include "sql_iff.h"			// output routinues
#include "sql_symbol.h"			// symbol table routinues
#include "sql_host.h"			// host file routinues

#define IFFILE_EXTEN   "iff"		// default output filename ext
#define NAMEBUFSIZ	1024		// filename buffer size
#define DEFAULT_CFILENAME "esql_out.c"	// default cfile output
#define STRING_MAX_LINES 40		// default max lines for a string

// processing a -symbols or -include file (specification file)
EXTERN_C int symbol_specification_phase = 0;

// file i/o ptr to current spec file
EXTERN_C FILE * symbol_specification_file = 0;

// flag: processing an esql (Embedded SQL) file
EXTERN_C int esql_mode = 0;

// for debugging: outputs symbol name not number,
// so that a diff can be run on IF files.
EXTERN_C int iff_test_output_flag = 0;

// the symbol table entry for the current source filename
EXTERN_C SQL_SYM * sql_symbol_main_file = 0;

// Database vendor name and value.
// Oracle is default.
EXTERN_C const char * dbvendor_name = DBVNAME_ORACLE;
EXTERN_C int dbvendor = DBVENDOR_ORACLE;

// maximum allowable lines in a multi-line string before
// scanner gives error about missing close quote.
EXTERN_C int sql_string_max_lines = STRING_MAX_LINES;

// for esql do not output VARCHAR variables found in global scope
EXTERN_C int suppress_global_references;	// in esql.C

// routine below to process each SQL program
static int process_file (const char * filename,
			 const char * cfilename,
			 const char * iffilename,
			 const char * iffilemode);
static int process_specification_file (const char * filename, int create_it);

#if SQL_DEBUG != 0
/* extern int yy_sql_debug;*/		// bison debug flag

int sql_debug_level = 0;		// level of debug info output
#endif

// prtvendors ---
// Print usage message listing valid database vendors
//
static int prtvendors (const char* prefix)
{
   fprintf (stderr, "%sValid database vendors are:\n", prefix);
#if 0
   fprintf (stderr, "%s\t%s\n", prefix, DBVNAME_ANSI);
#endif
   fprintf (stderr, "%s\t%s (default)\n", prefix, DBVNAME_ORACLE);
   fprintf (stderr, "%s\t%s\n", prefix, DBVNAME_INFORMIX);
#if 0
   fprintf (stderr, "%s\t%s\n", prefix, DBVNAME_SYBASE);
   fprintf (stderr, "%s\t%s\n", prefix, DBVNAME_DB2);
#endif
   return (0);
}

// prtusage ---
// Print usage message listing valid command line options
//
static int prtusage (const char * prgname)
{
   fprintf (stderr, "usage: %s [options] [filename ...]\n", prgname);

   fprintf (stderr, "\tOptions:\n");
   fprintf (stderr, "\t\t-database dbvendor_name\n");
   prtvendors ("\t\t\t");
   fprintf (stderr,
	    "\t\t-include specificationfile [-include specificationfile] ...\n");
   fprintf (stderr, "\t\t-symbols writable_specfile\n");
   fprintf (stderr, "\t\t-o if_outputfile\n");
   fprintf (stderr,
	    "\t\t-m maximum_number_lines_for_single_string (default %d)\n",
	    STRING_MAX_LINES);

   fprintf (stderr, "\tAdditional Embedded SQL Options:\n");
   fprintf (stderr, "\t\t-c output_cfile\n");
   fprintf (stderr, "\t\t-l\t\t(local host references only)\n");
   fprintf (stderr, "\t\tsql_optionname=optionvalue\n");
   fprintf (stderr, "\t\t\tSQL Options:\n");
   fprintf (stderr, "\t\t\t\tINCLUDE=include_path\n");

#if SQL_DEBUG != 0
   fprintf (stderr, "\tAdditional Debugging Options:\n");
   fprintf (stderr, "\t\t-<debug_level>\n");
   fprintf (stderr, "\t\t\t<debug_level> = 1, 2, 3, or 4\n");
#endif
   return (0);
}

// Simple list of specification file names
// these are passed on command line with -include or -symbols
// (symbol_flag is set for the latter).
//
// The reason for this list is that these files need to be
// processed after certain initialization has been done.
// For example, adding database vendor symbols, as 
// indicated by the -database vendor flag which may be placed
// on the command line after the spec files.
//
typedef struct specfile_entry spec_entry;
struct specfile_entry {
   const char *	filename;
   int		symbol_flag;
   spec_entry *	next;
};

// Class to add specification files to linked-list and process them
class specfile {
 public:
   specfile_entry * list;
   specfile_entry * list_end;
   specfile (void);
   ~specfile ();
   int clear (void);
   int add (const char * filename, int symbol_flag);
   int process (void);
};

// main
//
// exit with 0 for no problems including no parse errors.
// exit with high value (e.g. 111) when error occurred
// high value is used because calling TCL script els.dis
// was having problems confusing low exit values with signals.
// 
int main (int argc, char** argv)
{
   int ret = 0;
   int errret = 0;
   int c;
   int idx;
   int set_esql_mode = 0;		// source file is esql
   char * outname = 0;			// IF output filename
   char * inname = 0;			// source input filename
   char * cfilename = 0;		// C/C++ output filename
   char namebuf [NAMEBUFSIZ];		// buffer to build filename
   class specfile specfile;		// list of specification files

   // process command line arguments
   //
   // Note that idx is also incremented inside the loop
   // in the case of flags that take arguments
   for (idx = 1; idx < argc; ++idx) {
      if (*(argv [idx]) == '-') {
	 // get letter after hyphen
	 c = *(argv [idx] + 1);
	 // handle double dash "--argname"
	 if (c == '-') {
	    c = *(argv [idx] + 2);
	 }
	 switch (c) {
	 // check debug level -- higher number means more info
	 case '1':
	 case '2':
	 case '3':
	 case '4':
#if SQL_DEBUG != 0
	    sql_debug_level = c - (int)'0';

/****
	    if (sql_debug_level >= 4) {
	       yy_sql_debug = 1;
	    }
****/
#endif
	    break;

	 case 'c':
	    // C/C++ filename
	    // assumes -e
	    set_esql_mode = 1;
	    cfilename = argv [++idx];
	    break;

	 case 'd':
	    // vendor: oracle, informix, etc
	    dbvendor_name = argv [++idx];
	    if (OSapi_strcasecmp (dbvendor_name, DBVNAME_ORACLE) == 0) {
	       dbvendor = DBVENDOR_ORACLE;
	    } else if(OSapi_strcasecmp (dbvendor_name, DBVNAME_INFORMIX) == 0) {
	       dbvendor = DBVENDOR_INFORMIX;
	    } else if(OSapi_strcasecmp (dbvendor_name, DBVNAME_SYBASE) == 0) {
	       dbvendor = DBVENDOR_SYBASE;
#if 0
	    } else if(OSapi_strcasecmp (dbvendor_name, DBVNAME_ANSI) == 0) {
	       dbvendor = DBVENDOR_ANSI;
	    } else if(OSapi_strcasecmp (dbvendor_name, DBVNAME_DB2) == 0) {
	       dbvendor = DBVENDOR_DB2;
#endif
	    } else {
	       fprintf (stderr, "Unsupported database vendor name: %s\n",
			dbvendor_name);
	       prtvendors ("");
	       exit (112);
	    }
	    break;

	 case 'e':
	    // ESQL mode
	    // also turned on by -c
	    // set flag to set esql_mode flag after specification processing
	    set_esql_mode = 1;
	    break;

	 case 'i':
	    // [include] specification filename
	    specfile.add (argv [++idx], 0);
	    break;

	 case 'l':
	    // -l => local references only, i.e., suppress translation of host
	    // host variable references when in global scope.
	    //
	    suppress_global_references = 1;
	    break;

	 case 'm':
	    // maximum number of lines that a single string can span
	    sql_string_max_lines = atoi (argv [++idx]);
	    if (sql_string_max_lines <= 0) {
	       fprintf (stderr,
			"Invalid maximum number of lines per string: %s\n",
			argv [idx]);
	       prtusage (argv[0]);
	       exit (113);	       
	    }
	    break;

	 case 'o':
	    // output filename (IF file)
	    outname = argv [++idx];
	    break;

	 case 's':
	    // [symbols] specification filename
	    specfile.add (argv [++idx], 1);
	    break;

	 case 't':
	    // test: output symbol name instead of number
	    iff_test_output_flag = 1;
	    break;

	 case '?':
	 default:
	    prtusage (argv[0]);
	    exit (111);

	 }
      } else if (strchr (argv[idx], '=')) {
	 // process oracle option (e.g. DEFINE=foo)
	 esql_command_line_option (argv[idx]);
      } else {
	 break;
      }
   }

   // initialize database vendor specific info
   sql_symbols_initialize (dbvendor);

   // process -include and -symbols files
   specfile.process ();
   specfile.clear ();

   // set esql mode here - after any specification processing
   // which handles only non-embedded SQL
   //
   if (set_esql_mode) {
      esql_mode = 1;
   }

   // After handling command line options all that is left are
   // the SQL program names.  Process each program.  In normal
   // model build processing there will be only one filename
   //
   if (idx < argc) {
      for ( ; idx < argc; idx++) {
	 inname = argv [idx];

	 if (outname) {
	    // all filenames given -- process
	    errret = process_file (inname, cfilename, outname, "w");
	 } else {
	    // no output filename specified use input name and append .iff
	    // add 2 for '.' and '\0'
	    if ((strlen (inname) + strlen (IFFILE_EXTEN) + 2) > NAMEBUFSIZ) {
	       errret = 2;
	       fprintf (stderr, "name too long: %s\n", inname);
	       continue;
	    }
	    sprintf (namebuf, "%s.%s", inname, IFFILE_EXTEN);
	    errret = process_file (inname, cfilename, namebuf, "w");
	 }

	 if (errret) {
	    ret = errret;
	 }
      }
   } else {
      // no program names passed -- use stdin
      if (outname) {
	 ret = process_file (0, cfilename, outname, "w");
      } else {
	 // no output name either -- use stdout
	 ret = process_file (0, cfilename, 0, 0);
      }
   }

   // close -symbols file
   if (symbol_specification_file) {
      fclose (symbol_specification_file);
      symbol_specification_file = 0;
   }

   // problems with low return values being confused with signals
   if (ret) {
      exit (111);
   } else {
      exit (0);
   }

   return 0;
}

// Handle each SQL program file.
// If filename is 0 use stdin. If iffilename is 0 use stdout.
//
// Opens files and calls parser, then calls wrapup processing,
// lastly files are closed and result is returned.
//
static int process_file (const char * filename,
			 const char * cfilename,
			 const char * iffilename,
			 const char * iffilemode)
{
   int ret = 0;
   FILE *fp = 0;

   // open input file (SQL program source file)
   if (filename) {
#if SQL_DEBUG != 0
      if (sql_debug_level > 0) {
	 fprintf (stderr, "Processing %s\n", filename);
      }
#endif

      if (iffilename) {
	 if (strcmp (filename, iffilename) == 0) {
	    fprintf (stderr, "Input and Output files are the same: %s\n",
	       filename);
	       return (-1);
	 }
      }

      if ((fp = fopen (filename, "r")) == NULL) {
	 perror (filename);
	 ret = 1;
      }
   } else {
#if SQL_DEBUG != 0
      if (sql_debug_level > 0) {
	 fprintf (stderr, "Using stdin/stdout\n");
      }
#endif
      fp = 0;
   }

   // open output file
   if (ret == 0) {
      if (iffilename) {
	 // opens output file
	 if (iff_openfile (iffilename, iffilemode) < 0) {
	    ret = 2;
	 }
      } else {
	 // uses stdout
	 if (iff_openfile (0, 0) < 0) {
	    ret = 2;
	 }
      }
   }

   // print language record to output file
   if (ret == 0) {
      if (iff_language () < 0) {
	 ret = 5;
      }
   }
   
   // print filename record to output file
   if (ret == 0) {
      // set name for error messages
      iff_source_filename (filename);

      // add a symbol table entry for source filename
      if ((sql_symbol_main_file = iff_file (filename)) == 0) {
	 ret = 3;
      }
   }

   // For esql open C/C++ file
   if (ret == 0 && esql_mode) {
      if (cfilename) {
	 if (open_hostfile (cfilename) < 0) {
	    ret = 4;
	 }
      } else {
#if 0 // no C output
	 if (open_hostfile (DEFAULT_CFILENAME) < 0) {
	    ret = 4;
	 }
#endif // 0
      }
   }

   if (ret == 0) {
      // perform scanner initialization and call parser to
      // process input file.
      //
       if (dbvendor == DBVENDOR_SYBASE) {
	   ret = sybase_process(fp);
       }
       else ret = sql_process (fp);

      // set filename for map entries
      if (esql_mode) {
	 iff_map_set_src_filename (filename);
      }

      // Using info stored by the parser, walk symbol list and
      // syntax tree to output IFF records.
      //
      iff_ast ();	// ast is Abstract Syntax Tree
   }

   // close files
   if (esql_mode) {
      close_hostfile ();
   }
   if (fp) {
      iff_closefile ();
      fclose (fp);
      iff_source_filename (0);
   }

   return (ret);
}

// Parse a sql file for its symbols
// Open file for append if create_it is set
//
static int process_specification_file (const char * filename, int create_it)
{
   int ret = 0;
   FILE *fp = 0;
   struct stat sb;
   char * mode = (create_it) ? "r+" : "r";

   // open input file (SQL program source file)
   if ((fp = fopen (filename, mode)) == NULL) {
      // check for new file
      if (create_it && (errno == ENOENT)) {
	 if ((fp = fopen (filename, "w")) == NULL) {
	    perror (filename);
	    ret = 1;
	 }
      } else {
	 perror (filename);
	 ret = 1;
      }
   }

   // process file if not empty
   if (fp) {
      if (fstat (OSapi_fileno(fp), &sb) != -1) {
	 if (sb.st_size) {
	    // set filename for error logging
	    iff_source_filename (filename);

	    // perform scanner initialization and call parser to
	    // process input file.
	    //
	    if (dbvendor == DBVENDOR_SYBASE) {
		ret = sybase_process (fp);
	    }
	    else {
		ret = sql_process (fp);
	    }
	    
	    // clear filename for error logging
	    iff_source_filename (0);
	 }
      } else {
	 perror (filename);
	 ret = 1;
      }
   }

   // close input file or set global handle
   if (fp) {
      if (create_it) {
	 // file handle used to append any new symbol specification found
	 symbol_specification_file = fp;
      } else {
	 fclose (fp);
      }
   }

   return (ret);
}

// specfile class
specfile::specfile (void) : list (0), list_end (0) {}
 
specfile::~specfile () {
   clear ();
}

// delete list
int specfile::clear (void) {
   spec_entry * se;
   spec_entry * se_next;
   for (se = list; se; se = se_next) {
      se_next = se->next;
      delete se;
   }
   list = 0;
   list_end = 0;
   return (0);
}

// add filename to list
int specfile::add (const char * filename, int symbol_flag) {
   spec_entry * se = new spec_entry;
   se->filename = filename;
   se->symbol_flag = symbol_flag;
   se->next = 0;

   if (list_end) {
      list_end->next = se;
   } else {
      list = se;
   }
   list_end = se;
   return (0);
}

// specfile::process ---
// process each -include and -symbols file on list
//
// sets symbol_specification_phase global during processing
// so that IFF routines know not to output symbol info
// because primary file is not being processed.
//
// resets any symbol numbers found so they flaged as unused
// unless they are encounted in the primary file.
//
int specfile::process (void) {
   spec_entry * se;
   if (list) {
      // set global
      symbol_specification_phase = 1;
      for (se = list; se; se = se->next) {
	 // check for multiple -symbols files (only one can be active)
	 // first close any previous one - acts like -include
	 if (se->symbol_flag && symbol_specification_file) {
	    fclose (symbol_specification_file);
	    symbol_specification_file = 0;
	 }
	 process_specification_file (se->filename, se->symbol_flag);
      }

      // clear symbol numbers
      sql_symbols_reset ();

      // clear global
      symbol_specification_phase = 0;
   }
   return (0);
}
