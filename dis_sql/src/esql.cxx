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
 * E S Q L . C
 *
 * 1997, 1998 Software Emancipation Technology, Inc.
 *
 * Adapted for ELS March 1998 Mark B. Kaminsky
 * Created by William M. Miller
 *
 * This file contains the supporting routines for Embedded SQL
 * Bison actions. All output to the resulting host C/C++ file is done here.
 */

#ifndef ISO_CPP_HEADERS
#include <stdio.h>
#include <string.h>
#else /* ISO_CPP_HEADERS */
#include <cstdio>
using namespace std;
#include <cstring>
#endif /* ISO_CPP_HEADERS */
#include <malloc.h>
#ifndef ISO_CPP_HEADERS
#include <stdlib.h>
#include <ctype.h>
#else /* ISO_CPP_HEADERS */
#include <cstdlib>
#include <cctype>
#endif /* ISO_CPP_HEADERS */

#include "machdep.h"

#ifdef _WIN32
#include <io.h>
#else
#include <unistd.h>
#endif

#include "sql_scanner.h"	/* yylval and tokens */
#include "sql_iff.h"		/* output IF file routines */
#include "sql_host.h"		/* mapping of host (C/C++) file */

#define R_OK 4

/* variables */
#define MAXPATHLEN 1024
static char aset_wd[MAXPATHLEN] = { '.' };

/* routine from esql_driver.C */

EXTERN_C int esql_access(const char* fn, int md) {
    return access(fn, md);
}


/* The following variable is used to keep track of the output line
 * number.  The lexer stores the input line number in each
 * token, and the routine that copies tokens to the output inserts
 * newlines until the output line number is at least as large as the
 * line number in the token before outputting the text.  File inclusion
 * pushes and pops the input and output line numbers appropriately.
 */

EXTERN_C unsigned int olineno = 1;
EXTERN_C unsigned int ocolumn = 0;
EXTERN_C unsigned int ocolumn_last = 1;

/* If the following variable is true, any host variable references that
 * occur in file scope (i.e., with scope_nesting_level == 0) will be
 * suppressed.
 */

EXTERN_C int suppress_global_references = 0;

EXTERN_C int suppress_references = 0;

/* Host C/C++ file handle.  Zero for no output */
EXTERN_C FILE* outf = 0;

static char* default_incl = 0;

/* This variable corresponds to the DEF_SQLCODE option; if true, the
 * output for the main file will contain an Oracle-specific (C)
 * preprocessor definition.  The process_def_sqlcode() function
 * determines the correct value for the variable if the option is seen.
 */

static int do_sqlcode = 0;
static void process_def_sqlcode(const char* true_false);

static int compare_caseless(const char* opt, const char* label);

/* The following functions set up search paths for include files */

static void add_include_path_str(const char* path);
static void add_sysinclude_path_str(const char* path);

static void process_config_file(const char* filename);

/* The following functions provide the symbol table for conditional
 * compilation.
 */

static void define_id_str(const char* id);
EXTERN_C void add_id(const char* name);
EXTERN_C int is_defined(const char* name);
EXTERN_C void free_hashtable();

/**********************************************************************/

EXTERN_C int esql_command_line_option (const char* arg)
{
   /* The following are Oracle Pro*C-style arguments */
        
   /* Set the paths for searching for include files */
   
   if (compare_caseless(arg, "INCLUDE=")) {
      add_include_path_str(arg + 8);
   }
   
   /* Set the paths for looking for system include files (i.e., a
    * replacement for /usr/include)
    */
   
   else if (compare_caseless(arg, "SYS_INCLUDE=")) {
      add_sysinclude_path_str(arg + 12);
   }
   
   /* Define a variable for use in conditional compilation */
   
   else if (compare_caseless(arg, "DEFINE=")) {
      define_id_str(arg + 7);
   }
   
   /* Specify a file with additional configuration options */
   
   else if (compare_caseless(arg, "CONFIG=")) {
      process_config_file(arg + 7);
   }
   
   /* Request the SQLCODE preprocessor definition */
   
   else if (compare_caseless(arg, "DEF_SQLCODE=")) {
      process_def_sqlcode(arg + 12);
   }
   return (0);
}


/********** text copying routines and flags **********/

/* The following struct represents an IFDEF (and its matching ELSE, if
 * any).  The "selected" field is initialized when the IFDEF is seen;
 * if the IFDEF occurs in a selected region or outside of any
 * conditional directive, it is true or false depending on whether the
 * contained identifier has been defined.  If the IFDEF is in an
 * unselected region, "selected" will always be false.  The conditions
 * are, naturally, maintained in a LIFO stack based on the "cond_stack"
 * variable.
 */

typedef struct cond {
    struct cond* prev;
    int selected;
} cond;

static cond* cond_stack = 0;

/* The following function is used to determine whether output should be
 * discarded (i.e., the current processing point is in an unselected
 * region of text) or processed normally.
 */

static int selected() {
    return !cond_stack || cond_stack->selected;
}

/* save last column position of previous line
 * and reset column number
 */
static void increment_olineno (void)
{
   ocolumn_last = ocolumn;
   olineno++;
   ocolumn = 0;
}

/* All host output from the preprocessor flows through the following
 * function;
 */
EXTERN_C void do_output_len (const char* txt, size_t len) {
   if (outf) {
      fwrite (txt, (size_t) 1, len, outf);
   }
}

EXTERN_C void do_output(const char* txt) {
    if (selected()) {
	   const char *s;
	   const char *snl;
	   size_t len = strlen(txt);

	   /* moved from output_text because of need to track column */
	   for (s = txt; snl = strchr (s, '\n'); s = snl + 1) {
	      increment_olineno ();
	   }
	   if (s == txt) {
	      ocolumn += len;
	   } else {
	      ocolumn += strlen (s);
	   }

	   if (outf) {
	      /* fprintf(outf, "%s", txt); */
	      do_output_len (txt, len);
	   }
    }
}

EXTERN_C void do_output_chr(char c) {
    if (selected() && outf) {
       do_output_len ((char*) &c, (size_t)1);
       ocolumn++;
    }
}

EXTERN_C void do_output_newline() {
    if (selected() && outf) {
       do_output_len ("\n", (size_t)1);
       increment_olineno ();
       iff_map_newline_check ();
    }
}

/* We keep the complexity of mappings down, so that nearly all of the
 * IF MAP directives consist of the MAP lines for includes. 
 * The sql.l file keeps track of these.  It needs the olineno
 * and at the same time it must provide us with the corresponding ilineno
 * so that we can put tokens onto exactly the correct point in the
 * output (intermediate C) file.
 */
static unsigned int lineno_offset = 0;
EXTERN_C unsigned int sync_linenos(unsigned int ilineno) {
    lineno_offset = olineno - ilineno;
    return olineno;
}

/* The following function positions the output to the necessary output
 * line number to maintain the token correspondence between input and
 * output.
 */

static void get_to_line(unsigned int targ) {
    if (selected()) {
	while (olineno < targ) {
	    do_output_newline();
	}
    }
}

static void get_to_column(unsigned int targ) {
   if (selected()) {
      /* first decrement targ for character about to print */
      --targ;
      while (ocolumn < targ) {
	 do_output_chr(' ');	/* bumps ocolumn */
      }
   }
}

/* The following function uses the preceding to position to the line
 * associated with a given token (or the first token of a multi-token
 * aggregation).
 */

EXTERN_C void get_to_token_line(sqltype tok) {
    token* p = get_token(tok.start_token);
    if (p) {
        get_to_line(p->lineno + lineno_offset);
    }
}

/* The following function copies its input to the output, possibly with
 * a trailing space (to separate tokens); it also examines the input
 * for newlines to keep the current output line number up-to-date.
 * (Newlines can occur in string tokens and in literal output.)
 */

static void output_text(const char* txt, int with_space) {
    size_t len = strlen(txt);
    if (len) {
	do_output(txt);
	if (with_space && txt[len - 1] != '\n' && txt[len - 1] != '\\') {
	    do_output(" ");
	}
    }
}

/* The following function copies the text of a token or aggregate of
 * tokens to the output, maintaining the correspondence between input
 * and output line numbers.
 */

EXTERN_C void copy_out_tokens(int start_token,
			      int end_token,
			      int map_flag)
{
   unsigned int i;
   token* p;
   token* plast;
   int first = 1;
   for (i = start_token; i <= end_token; i++) {
      p = get_token(i);
      if (p) {
	 plast = p;
	 get_to_line(p->lineno + lineno_offset);
	 get_to_column(p->column);
	 if (first) {
	    first = 0;
	    if (map_flag) {
	       iff_map_copy_start_loc (p->lineno, p->column,
				       olineno, ocolumn+1);
	    }
	 }
	 output_text(p->text, 0);
      }
   }
   if (( ! first ) && map_flag && plast) {
      iff_map_copy_end_loc (plast->lineno,
			plast->column + strlen (plast->text) - 1,
			olineno, ocolumn);
   }
}


EXTERN_C void copy_out(sqltype val, int map_flag) {
    copy_out_tokens(val.start_token, val.end_token, map_flag);
}

/* The following function is identical to the preceding, except that
 * #line directives are used instead of newlines to position the
 * token correctly in the output.  (This gives the ability to reorder
 * tokens, as is required for the VARCHAR translation.)
 */

static void copy_out_with_line_directive(sqltype val) {
    copy_out(val, 1);
}
/* The following variable is used to generate the names of container
 * functions for file-scope host variable references.
 */

static int fcn_no;

/* The following variable is set either to 0 or to the line number
 * for which a "MAP copy" with an endling column of 0 was done.  If
 * there is an attempt to put the closing "}" for an executable SQL
 * statement on that same line, we will move to the next line first
 * (otherwise, conflicting map locations for that character will be
 * output and ifext will get confused).
 */

static unsigned map_to_eol_on_lineno;

/* The following function is called to introduce a host variable reference.
 * If we are at global scope (i.e., scope_nesting_level == 0), we will
 * output either nothing (if suppress_global_references is true) or the
 * beginning of a function definition.  If scope_nesting_level > 0, we
 * will output just a "{".
 */

EXTERN_C void prefix_reference(sqltype val) {
    map_to_eol_on_lineno = 0;
    /* check if prefix is on previous line and needs map copy */
    token* p = get_token (val.start_token);
    if (p) {
       if (p->lineno < ilineno) {
	 /* copy of token */
	 iff_map_copy_start_loc (p->lineno, p->column, olineno, ocolumn + 1);
	 iff_map_copy_end_loc   (p->lineno, p->column, olineno, ocolumn + 1);
	 /* copy of rest of line (column zero)
	  * needed because otherwise ifext adjusts rest of line
	  */
	 iff_map_copy_start_loc (olineno - lineno_offset, 2, olineno, ocolumn + 2);
	 iff_map_copy_end_loc (olineno - lineno_offset, 0, olineno, 0);
	 map_to_eol_on_lineno = olineno;
       }
    }

    if (scope_nesting_level) {
	do_output("{");
    } else {
	char buff[64];
	sprintf(buff, "static void __DISCOVER_ESQL_DUMMY_%d(){", fcn_no++);
	do_output(buff);
    }
}


/* The following function is called to end a host variable reference.
 * If we are at global scope and suppress_global_references is true,
 * we do nothing (the prefix and reference were also suppressed);
 * otherwise, we output "}".
 */

EXTERN_C void postfix_reference(sqltype val) {
   unsigned int i = val.end_token;
   token* p = get_token(i);
   int end_column;
   if (p) {
      /* position so that closing brace is at end of statement */
      get_to_line(p->lineno + lineno_offset);
      end_column = p->column + strlen (p->text) - 1;
      get_to_column (end_column);

      /* check if closing brace column will be after end of statement
       * because of reference output.  If so create a map copy record
       * so end of block matches
       */
      if ((ocolumn + 1) > end_column) {
	 if (map_to_eol_on_lineno == olineno) {
	     get_to_line(olineno + 1);
	 }
	 iff_map_copy_start_loc (p->lineno, end_column, olineno, ocolumn + 1);
	 iff_map_copy_end_loc (p->lineno, end_column, olineno, ocolumn + 1);
      }
   }
   do_output("}");
}

/* The following function is called to translate a reference to a C/C++
 * variable in the embedded SQL into an expression statement in the
 * output.
 */

EXTERN_C void reference_var(sqltype ref) {
    if (!(scope_nesting_level == 0 && suppress_global_references)
	&& !suppress_references) {
       copy_out_tokens(ref.start_token, ref.end_token, 0);
       do_output(";");
    }
}

/* The following function is used to create a single string from the
 * text of a number of tokens.  It is used for creating pathnames for
 * inclusion directives, since the lexer breaks them up during its
 * normal processing.  The string that it returns has been malloced and
 * must be freed by the caller.
 */

static char* concat_tokens(sqltype tokens) {
    unsigned int i;
    token* tp;
    size_t sz  = 0;
    char* result;
    char* p;
    
    for (i = tokens.start_token; i <= tokens.end_token; i++) {
        tp = get_token(i);
        if (tp) {
            sz += strlen(tp->text);
        }
    }
#ifdef ISO_CPP_HEADERS
    result = (char*)std::malloc(sz + 1);
#else
    result = (char*)malloc(sz + 1);
#endif
    p = result;
    for (i = tokens.start_token; i <= tokens.end_token; i++) {
        tp = get_token(i);
        if (tp) {
            strcpy(p, tp->text);
            p += strlen(tp->text);
        }
    }
    *p = 0;
    return result;
}

/* The following function is used to get the text of a single token.
 * It returns the original string, not a copy, so the result must NOT
 * be freed by the caller.
 */

static const char* get_token_text(sqltype tok) {
    token* p = get_token(tok.start_token);
    if (p) {
        return p->text;
    }
    return "";
}

/********** file inclusion handling code **********/

#ifdef _WIN32
#define PATH_SEPARATOR '\\'
#define DEFAULT_SYSINCLUDE "/usr/include"   /* ??? */
#else
#define PATH_SEPARATOR '/'
#define DEFAULT_SYSINCLUDE "/usr/include"
#endif

/* The following routine adds a component (directory or filename) to an
 * existing directory name.  It does the right thing regardless of
 * whether the original directory string ends in a separator character
 * or not.  The result is malloced and must be freed by the caller.
 */

static char* concat_path(const char* basepath, const char* component) {
    size_t blen = strlen(basepath);
    size_t clen = strlen(component);
#ifdef ISO_CPP_HEADERS
    char* result = (char*)std::malloc(blen + clen + 2);
#else
    char* result = (char*)malloc(blen + clen + 2);
#endif
    strcpy(result, basepath);
    if (basepath[blen - 1] != PATH_SEPARATOR) {
        result[blen++] = PATH_SEPARATOR;
    }
    strcpy(result + blen, component);
    return result;
}

/* The following structure represents one of a list of path specifiers,
 * used for searching for include files.  They are kept in a
 * singly-linked list.
 */

typedef struct path {
    struct path* next;
    const char* text;
} path;

/* There are two sets of path specifications kept by the Oracle
 * preprocessor, SYSINCLUDE and INCLUDE.  Because the specifications
 * accumulate in FIFO order, we keep both head and tail pointers for
 * each list to allow efficient insertion at the end.
 */

static path* sysinclude_head = 0;
static path* sysinclude_tail = 0;
static path* include_head = 0;
static path* include_tail = 0;

/* The following function deletes the previous set of specifications
 * and reinitializes the lists.
 */

static void init_include_paths() {
    path* p;
    while (sysinclude_head) {
	p = sysinclude_head;
	sysinclude_head = p->next;
#ifdef ISO_CPP_HEADERS
	std::free(p);
#else
	free(p);
#endif
    }
    sysinclude_tail = 0;
    while (include_head) {
	p = include_head;
	include_head = p->next;
#ifdef ISO_CPP_HEADERS
	std::free(p);
#else
	free(p);
#endif
    }
    include_tail = 0;
}

/* The following function adds a single path specification to the list
 * specified by the "head" and "tail" parameters.  The "text" argument
 * is assumed to be malloced and will be freed by init_include_paths.
 * The arguments is assumed to have no leading blanks, but trailing
 * blanks are truncated here.
 */

static void add_path(path** head, path** tail, char* text) {
    size_t len = strlen(text);
    path* p;
    int i;
    for (i = len - 1; i >= 0 && text[i] == ' '; i--) {
	text[i] = 0;
    }
    if (i < 0) {
        /* empty specification */
#ifdef ISO_CPP_HEADERS
        std::free(text);
#else
        free(text);
#endif
    }
    else {
#ifdef ISO_CPP_HEADERS
        p = (path*) std::malloc(sizeof(path));
#else
        p = (path*) malloc(sizeof(path));
#endif
        p->next = 0;
        p->text = text;
        if (*tail) {
            (*tail)->next = p;
        }
        else *head = p;
        *tail = p;
    }
}

/* The following function takes an inclusion specification in the form
 * of an Oracle option (either a single spec or a parenthesized,
 * comma-separated list of specs) and uses the preceding function to
 * add it/them to the list specified by the "head" and "tail"
 * parameters. 
 */

static void add_paths(path** head, path** tail, const char* text) {
    if (*text == '(') { /* process list */
        text++;
        for (;;) {
            size_t segment_len;
            while (*text == ' ') {
                text++;
            }
            segment_len = strcspn(text, ",)");
            if (segment_len) {
#ifdef ISO_CPP_HEADERS
                char* segment = (char*) std::malloc(segment_len + 1);
#else
                char* segment = (char*) malloc(segment_len + 1);
#endif
                memcpy(segment, text, segment_len);
                segment[segment_len] = 0;
                add_path(head, tail, segment);
            }
            if (text[segment_len] == ',') {
                text += segment_len + 1;
            }
            else break;
        }
    }
    else add_path(head, tail, OSapi_strdup((char*) text));
}

/* The following routine implements the Oracle convention that included
 * files with no extension are assumed to end in ".h".  If a ".h" is
 * added, the result is malloced and must be freed by the caller;
 * otherwise, the input is returned as the result.  (The caller must
 * compare the result and the argument to decide whether to free the
 * result.)
 */

static char* add_dot_h(const char* filename) {
    char* result;
    if (strchr(filename, '.') == 0) {
	int all_uppercase = 1;
	size_t len = strlen(filename);
	const char* p;
	for (p = filename + len - 1; p >= filename; p--) {
	    if (islower(*p)) {
		all_uppercase = 0;
		break;
	    }
	    if (*p == PATH_SEPARATOR) {
		break;
	    }
	}
#ifdef ISO_CPP_HEADERS
	result = (char*) std::malloc(len + 3);
#else
	result = (char*) malloc(len + 3);
#endif
	strcpy(result, filename);
	if (all_uppercase) {
	    strcat(result, ".H");
	}
	else strcat(result, ".h");
    }
    else result = (char*) filename;
    return result;
}

/* return allocated lowercase string if input is all uppercase */
static char* lowercase_string(const char* filename) {
   char* result = 0;
   int all_uppercase = 1;
   size_t len = strlen (filename);
   const char *p;
   for (p = filename + len - 1; p >= filename; p--) {
      if (islower(*p)) {
	 all_uppercase = 0;
	 break;
      }
   }
   if (all_uppercase) {
#ifdef ISO_CPP_HEADERS
      result = (char*) std::malloc (len + 1);
#else
      result = (char*) malloc (len + 1);
#endif
      int i;
      for (i = 0, p = filename; i < len ; ++i, ++p) {
	 result[i] = (isupper(*p)) ? tolower(*p) : *p;
      }
      result[i] = '\0';
   }
   return (result);
}
 
 
static void include_file_strip(char* filename) {
   char * s;
   /* if name is delimited by quotes -- remove them */
   if (filename && (*filename == '"')) {
      /* overwrite quote by moving up all characters */
      for (s = filename; *(s+1) && (*(s+1) != '"'); ++s) {
	 *s = *(s+1);
      }
      /* zap closing quote */
      *s = '\0';
   }
}


static char * include_file_find (const char* filename) {
    int found = 0;
    char* path_name;
    path* p;
    
    if (!sysinclude_head) {
	path_name = concat_path(DEFAULT_SYSINCLUDE, filename);
	if (!(found = (esql_access(path_name, R_OK) == 0))) {
#ifdef ISO_CPP_HEADERS
		std::free(path_name);
#else
		free(path_name);
#endif
	}
    }
    else for (p = sysinclude_head; !found && p; p = p->next) {
	path_name = concat_path(p->text, filename);
	if (!(found = (esql_access(path_name, R_OK) == 0))) {
#ifdef ISO_CPP_HEADERS
		std::free(path_name);
#else
		free(path_name);
#endif
	}
    }
    if (!found) {
	path_name = concat_path(aset_wd, filename);
	if (!(found = (esql_access(path_name, R_OK) == 0))) {
#ifdef ISO_CPP_HEADERS
		std::free(path_name);
#else
		free(path_name);
#endif
	}
    }
    if (!found && default_incl) {
	path_name = concat_path(default_incl, filename);
	if (!(found = (esql_access(path_name, R_OK) == 0))) {
#ifdef ISO_CPP_HEADERS
		std::free(path_name);
#else
		free(path_name);
#endif
	}
    }
    if (!found) {
	for (p = include_head; !found && p; p = p->next) {
	    path_name = concat_path(p->text, filename);
	    if (!(found = (esql_access(path_name, R_OK) == 0))) {
#ifdef ISO_CPP_HEADERS
		std::free(path_name);
#else
		free(path_name);
#endif
	    }
	}
    }

    if (!found) {
       path_name = 0;
    }

    return (path_name);
}

/* The following function is called when an include directive is
 * encountered.  It looks in all the places and in the order specified
 * by the Oracle preprocessor and, if the file is found, opens it for
 * input.
 */

static void include_file_str(const char* filename) {

    char* path_name = 0;
    char* lowercase_name = 0;
    
    /* Ignore include in unselected branch of conditional compilation */

    if (!selected()) {
        return;
    }
    
    /* add ".h" to filename if needed */
    
    char* file_to_include = add_dot_h(filename);

    path_name = include_file_find (file_to_include);

    /* In Oracle the filename is sometimes listed in uppercase.
     * If not found and filename is uppercase check lowercase too.
     */
    if (!path_name) {
       lowercase_name = lowercase_string (file_to_include);
       if (lowercase_name) {
	  path_name = include_file_find (lowercase_name);
       }
    }

    if (!path_name) {
	char buff[MAXPATHLEN];
	sprintf(buff, "Cannot open include file %s\n",
		file_to_include);
	iff_error(buff);
    } else {
#if 1
       esql_push_include (path_name);
#else
       do_output ("#include \"");
       do_output (path_name);
       do_output ("\"");
#endif
    }

#ifdef ISO_CPP_HEADERS
    std::free (path_name);
#else
    free (path_name);
#endif

    /* Free result of "add_dot_h" if needed */

    if (file_to_include != filename) {
#ifdef ISO_CPP_HEADERS
	std::free(file_to_include);
#else
	free(file_to_include);
#endif
    }

    /* Free result of "lowercase_string" if needed */

    if (lowercase_name) {
#ifdef ISO_CPP_HEADERS
	std::free(lowercase_name);
#else
	free(lowercase_name);
#endif
    }
}

/* The following routine is called from the Bison action after
 * recognizing an include directive; it translates the token(s) into a
 * string and uses the preceding function to do the inclusion.
 */

EXTERN_C void include_file(sqltype filename) {
    char* p = concat_tokens(filename);
    get_to_token_line(filename);
    include_file_strip(p);
    include_file_str(p);
#ifdef ISO_CPP_HEADERS
    std::free(p);
#else
    free(p);
#endif
}

/* Add a spec to the INCLUDE list */

static void add_include_path_str(const char* path) {
    add_paths(&include_head, &include_tail, path);
}

EXTERN_C void add_include_path(sqltype path) {
    char* p = concat_tokens(path);
    add_include_path_str(p);
#ifdef ISO_CPP_HEADERS
    std::free(p);
#else
    free(p);
#endif
}

/* Add a spec to the SYSINCLUDE list */

static void add_sysinclude_path_str(const char* path) {
    add_paths(&sysinclude_head, &sysinclude_tail, path);
}

EXTERN_C void add_sysinclude_path(sqltype path) {
    char* p = concat_tokens(path);
    add_sysinclude_path_str(p);
#ifdef ISO_CPP_HEADERS
    std::free(p);
#else
    free(p);
#endif
}

/********** conditional compilation **********/

/* The following function is just a wrapper around the symbol table */

static void define_id_str(const char* id) {

    /* Ignore definition in unselected branch of conditional compilation */

    if (!selected()) {
        return;
    }

    add_id(id);
}

EXTERN_C void define_id(sqltype id) {
    define_id_str(get_token_text(id));
}

/* The following routine pushes a new condition on the condition stack.
 * This must be done even in unselected regions of the text in order to
 * balance the "if" and "endif" directives.  The initial value of the
 * "selected" field in the condition is true IFF the "ifdef" occurs in
 * a selected region and the specified identifier is in the symbol
 * table.
 */

EXTERN_C void do_ifdef(sqltype id) {
#ifdef ISO_CPP_HEADERS
    cond* p = (cond*) std::malloc(sizeof(cond));
#else
    cond* p = (cond*) malloc(sizeof(cond));
#endif
    int in_selected_region = (cond_stack) ? cond_stack->selected : 1;
    p->prev = cond_stack;
    cond_stack = p;
    p->selected = in_selected_region && is_defined(get_token_text(id));
}

/* The following routine is just like the former except that the
 * condition depends on the identifier NOT being present in the symbol
 * table.
 */

EXTERN_C void do_ifndef(sqltype id) {
#ifdef ISO_CPP_HEADERS
    cond* p = (cond*) std::malloc(sizeof(cond));
#else
    cond* p = (cond*) malloc(sizeof(cond));
#endif
    int in_selected_region = (cond_stack) ? cond_stack->selected : 1;
    p->prev = cond_stack;
    cond_stack = p;
    p->selected = in_selected_region && !is_defined(get_token_text(id));
}

/* The following routine is called when an "else" directive is
 * recognized.  The value of the current condition's "selected" field
 * is toggled, but only if the "ifdef" occurred in a selected region.
 */

EXTERN_C void do_else(sqltype) {
    if (cond_stack) {
	int in_selected_region = !cond_stack->prev ||
		cond_stack->prev->selected;
	if (in_selected_region) {
	    cond_stack->selected = !cond_stack->selected;
	}
    }
}

/* The following routine pops a condition off the stack when an "endif"
 * is seen.
 */

EXTERN_C void do_endif(sqltype) {
    cond* p = cond_stack;
    if (p) {
	cond_stack = p->prev;
#ifdef ISO_CPP_HEADERS
	std::free(p);
#else
	free(p);
#endif
    }
}

/* The following function is called when a simple VARCHAR declaration
 * (i.e., not a pointer, not an array, and with no initialization) is
 * seen.  It adds an anonymous struct (like Oracle, as opposed to the
 * ANSI SQL spec, which just uses a simple character string), moves the
 * size specification to the appropriate struct member, and then copies
 * the identifier.
 */

EXTERN_C void expand_varchar(sqltype id, sqltype size) {
    do_output("struct{unsigned short len;unsigned char arr");
    copy_out(size, 1);
    do_output(";}");
    copy_out_with_line_directive(id);
    do_output(";");
}

/* The following function works basically like the preceding, except
 * that an initializer is added (the "=" from the source is not part of
 * the token(s) passed in).  Because the item declared is no longer a
 * scalar but an aggregate, braces and an initializer for the "len"
 * field are also added.
 */

EXTERN_C void expand_varchar_init(sqltype id, sqltype size, sqltype init) {
    do_output("struct{unsigned short len;unsigned char arr");
    copy_out(size, 1);
    do_output(";}");
    copy_out_with_line_directive(id);
    output_text("\n=\n{0,", 0);
    copy_out(init, 1);
    output_text("\n};", 0);
}

/* MBK 1998/03/30 same as above but copy init tokens as is */
EXTERN_C void expand_varchar_init2(sqltype id, sqltype size, sqltype init) {
    do_output("struct{unsigned short len;unsigned char arr");
    copy_out(size, 1);
    do_output(";}");
    copy_out_with_line_directive(id);
    copy_out(init, 1);
    do_output(";");
}

/* The following function is like the previous ones except that it is
 * called when an array of VARCHARs is declared; the size specification
 * is moved inside the struct and the array bound is left following the
 * identifier.
 */

EXTERN_C void expand_varchar_array(sqltype id, sqltype size, sqltype bound) {
    do_output("struct{unsigned short len;unsigned char arr");
    copy_out(size, 1);
    do_output(";}");
    copy_out_with_line_directive(id);
    copy_out(bound, 1);
    do_output(";");
}

/* The following function is called when a VARCHAR with a pointer operator
 * (*, &, const, volatile) is seen.  This can occur in a declaration
 * In this case, there is no array bound in the source, so a
 * dummy bound of 1 is supplied; also, since this doesn't represent a
 * "whole" construct, the rest of the context is left unchanged.
 */

EXTERN_C void expand_varchar_ptr(sqltype ptr_op) {
    do_output("struct{unsigned short len;unsigned char arr[1];}");
    copy_out(ptr_op, 0);
    token* p = get_token (ptr_op.start_token);
    if (p) {
       /* generate copy entry from current position to EOL (zero column) */
       iff_map_copy_start_loc (p->lineno, p->column+1, olineno, ocolumn+1);
       iff_map_copy_end_loc (p->lineno, 0, olineno, 0);
    }
}

/* VARCHAR without a size specified.  Use size of one */
EXTERN_C void expand_varchar_default (sqltype id) {
    do_output("struct{unsigned short len;unsigned char arr[1];}");
    copy_out_with_line_directive(id);
    do_output(";");
}

/* VARCHAR pointer cast.  Above does not work because the C compiler
 * complains when a ptr to a unnamed struct is assigned to another
 * one.  So this changes VARCHAR to void. tok is the VARCHAR token.
 */
EXTERN_C void expand_varchar_ptr_cast (sqltype tok) {
    iff_map_token_start_fixed (tok);
    do_output ("void   ");
    iff_map_token_end_fixed (tok);
}

/********** utilities **********/

/* expand in place */
EXTERN_C void expand_sqlcursor (sqltype) {
    do_output(
       "struct {unsigned curocn;void*ptr1;void*ptr2;unsigned long magic;} ");
}

/* The following function is called to process a configuration file.
 * The options recognized are essentially those that can be specified
 * on the command line, except that you cannot specify a nested option
 * file.  Unrecognized options are simply ignored.
 */

#define LINESIZE 512

static void process_config_file(const char* filename) {
    FILE* cfg;
    char* file_to_open = add_dot_h(filename);
    if (cfg = fopen(file_to_open, "r")) {
	char line[LINESIZE];
	while (fgets(line, LINESIZE, cfg)) {
	    char* p = line + strspn(line, " \t");
	    if (compare_caseless(p, "INCLUDE=")) {
		add_include_path_str(p + 8);
	    }
	    else if (compare_caseless(p, "SYS_INCLUDE=")) {
		add_sysinclude_path_str(p + 12);
	    }
	    else if (compare_caseless(p, "DEFINE=")) {
		define_id_str(p + 7);
	    }
	    else if (compare_caseless(p, "DEF_SQLCODE=")) {
		process_def_sqlcode(p + 12);
	    }
	}
    }
    else fprintf(stderr, "WARNING: cannot open config file %s for reading\n",
		 file_to_open);
    if (file_to_open != filename) {
#ifdef ISO_CPP_HEADERS
	std::free(file_to_open);
#else
	free(file_to_open);
#endif
    }
}

/* The following function is called when a DEF_SQLCODE option is
 * recognized; it just sets a variable to
 * tell whether to output the relevant preprocessor definition.
 */

static void process_def_sqlcode(const char* true_false) {
    if (toupper(*true_false) == 'Y' || toupper(*true_false) == 'T'
	|| *true_false == '1') {
	do_sqlcode = 1;
    }
    else do_sqlcode = 0;
}

static int compare_caseless(const char* opt, const char* label) {
    while(*label) {
        if (toupper(*opt) != *label) {
            return 0;
        }
        label++;
        opt++;
    }
    return 1;
}

/* Routine to start a map fixed entry at a token.
 * Called before first token is output.
 */
EXTERN_C void iff_map_token_start_fixed (sqltype val)
{
   token* p = get_token (val.start_token);
   if (p) {
      /* ocolumn+1 is next output column (where token will start) */
      iff_map_fixed_start_loc (p->lineno, p->column, olineno, ocolumn+1);
   }
}
      
/* Routine to end a map fixed entry at a token.
 * Called after last token is output.
 */
EXTERN_C void iff_map_token_end_fixed (sqltype val) {
   token* p = get_token (val.end_token ? val.end_token : val.start_token);
   if (p) {
      /* input file ending position:
       * minus one because column is one-relative
       */
      int end_pos = p->column + strlen (p->text) - 1;
      iff_map_fixed_end_loc (p->lineno, end_pos, olineno, ocolumn);
   }
}
