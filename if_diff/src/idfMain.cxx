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
#include <idfMain.h>
#include <general.h>
#ifndef ISO_CPP_HEADERS
#include <string.h>
#include <stdio.h>
#include <iostream.h>
#else /* ISO_CPP_HEADERS */
#include <cstring>
using namespace std;
#include <cstdio>
#include <iostream>
#endif /* ISO_CPP_HEADERS */
#include <idfDict.h>
#include <idfMatchSmt.h>
#include <idfMatchAst.h>
#include <idfMatchSymbol.h>
#include <idfReport.h>
#include <idfModel.h>
#include <idfLocation.h>
#include <iff_parser.h>

#if 0
extern "C" int yy_iff_parser_parse();
extern FILE *yy_iff_parser_in;
#endif

idfModel *idf_current_model = NULL;
idfSymbolDictionary *idf_current_symbols = NULL;
idfSmt *idf_prev_smt = NULL;
idfAstDictionary *idf_current_ast_roots = NULL;
const char *idf_current_filename = NULL;
const char *idf_interesting_filename = NULL;

//============================================================================
boolean idf_strmatch(const char *s1, const char *s2)
{
  return (s1 == NULL || s2 == NULL) ? s1 == s2
      	      	      	      	    : strcmp(s1, s2) == 0;
}

//============================================================================
char *idf_copy_string(const char *s1)
{
  if (s1 == NULL) {
    return NULL;
  }
  char *s2 = new char[strlen(s1) + 1];
  if (s2 != NULL) {
    strcpy(s2, s1);
  }
  return s2;
}

//============================================================================
static idfSmt *find_start(idfSmt *smt_p)
{
  idfSmt *last = NULL;
  while (smt_p != NULL) {
    last = smt_p;
    smt_p = smt_p->getPrev();
  }
  return last;
}

// !!! The main parser needs to set up the symbol dictionary and
// reset the prevSmt before each parsing run.

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! !/

The plan
--------
1. Parse 2 files.
2. Match files.
3. Report discrepancies.

We have
-------
A. data structures for symbols and SMT

To Do
-----
1. Data structures for the AST are not yet declared or built.

3. The reporting may take place in more than one pass.
   This can be the best way to use the symbol's already-reported flag.
   IdfSmt nodes don't have an already-reported flag yet.
   
2. IdfSmt nodes need to be sorted prior to matching.

/! !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */

//============================================================================
static boolean parse_file(idfModel &model,
      	      	          idfSymbolDictionary &symbols,
      	      	          idfSmt *&idfLastSmt,
      	      	          idfAstDictionary *&idfAstRoots,
      	      	          const char *filename)
{
  // Set up for parse to build structures.
  idf_current_filename = filename;
  idf_current_symbols = &symbols;
  idf_current_model = &model;
  idf_prev_smt = NULL;
  idf_current_ast_roots = new idfAstDictionary;

#if 0
  yy_iff_parser_in = file;
  int result = (yy_iff_parser_parse() == 0);       // Empirically, 0 -> success; 1 -> error.
#endif
  int result = (iff_parse((char *)filename) > 0);  // negative -> error; positive -> success.

  // Obtain results of parse.
  idfLastSmt = idf_prev_smt;
  idfAstRoots = idf_current_ast_roots;

  idf_current_model = NULL;
  idf_current_symbols = NULL;
  idf_current_filename = NULL;

  if (result) {
    model.setAst(idfAstRoots);
    model.setSmt(find_start(idfLastSmt));
    idf_filter_locations(model, idf_unmap_location);
    idf_filter_locations(model, idf_set_file);
  }
  return result; 
}

//============================================================================
static boolean process_arguments(int argc, char *argv[],
                                 const char *&fn1, const char *& fn2,
      	      	      	      	 const char *&config_filename,
      	      	      	      	 const char *&interest_filename)
{
  config_filename = NULL;
  interest_filename = NULL;
  if (argc < 2) {
    fn1 = "mod.if"; // !!! in order to debug
    fn2 = "dish";
    return true;
  }
  int positional_arg_count  = 1;
  boolean next_arg_is_config = false;
  boolean next_arg_is_interest = false;
  for (int i = 1; i < argc; i += 1) {
    const char *this_arg = argv[i];
    if (next_arg_is_config) {
      config_filename = this_arg;
      next_arg_is_config = false;
    }
    else if (next_arg_is_interest) {
      interest_filename = this_arg;
      next_arg_is_interest = false;
    }
    else if (this_arg[0] == '-') {
      if (strcmp(this_arg, "-config") == 0) {
      	next_arg_is_config = true;
      }
      if (strcmp(this_arg, "-interest") == 0) {
      	next_arg_is_interest = true;
      }
    }
    else {
      if (positional_arg_count == 1) {
	fn1 = this_arg;
      }
      else if (positional_arg_count == 2) {
	fn2 = this_arg;
      }
      positional_arg_count += 1;
    }
  }
  return (positional_arg_count == 3);
}

idfStringSetDictionary idf_configuration;
idfStringSet idf_no_values;

//============================================================================
static void configure_directive(const char *directive, const char *value)
{
  if (directive != NULL) {
    idfStringSet *set = idf_configuration.lookup(directive);
    if (set == NULL) {
      set = &idf_no_values;
      idf_configuration.add(directive, set);
    }
    if (value != NULL) {
      if (set == &idf_no_values) {
      	set = new idfStringSet;
      	idf_configuration.reassign(directive, set);
      }
      set->add(value);
    }
  }
}

//==============================================================================
// Read lines from the "if_diff.config" file.
// Each line is of the form:
//         directive
// or      directive(value)more
// and causes directives to be entered into a table of directives,
// and the value of a directive to be entered into a table for that
// directive.
//============================================================================
static boolean configure(const char *config_filename)
{
  FILE *cf = fopen(config_filename, "r");
  boolean opened = (cf != NULL);
  if (opened) {
    const int bufsize = 1000;
    char buffer[bufsize];
    boolean prev_line_unterminated = false;
    while (fgets(buffer, bufsize, cf) != NULL) {
      char *nl = strchr(buffer, '\n');
      if (!prev_line_unterminated && nl != NULL) {
      	// We have a complete line.
      	*nl = '\0';
      	// Look for parenthesized value.
      	char *lp = strchr(buffer, '(');
      	char *rp = NULL;
      	if (lp != NULL) {
      	  rp = strchr(lp, ')');
      	}
      	int value_length = 0;
      	char value_buffer[bufsize];
      	if (lp != NULL && rp != NULL) {
      	  value_length = (rp - lp) - 1;
       	  strncpy(value_buffer, lp + 1, value_length);
      	  value_buffer[value_length] = '\0';
      	  memmove(lp + 1, rp, (nl - rp) + 1);
      	}
        configure_directive(buffer, rp != NULL ? value_buffer : NULL);
      }
      prev_line_unterminated = (nl == NULL);
    }
  }
  return opened;
}

//============================================================================
int main(int argc, char *argv[])
{
  // Process arguments.
  const char *fn1;
  const char *fn2;
  const char *config_filename;
  if (!process_arguments(argc, argv, fn1, fn2, config_filename, idf_interesting_filename)) {
    cout << "Usage: if_diff <if_file1> <if_file2> [-config <config_file>] [-interest <interest_file>]" << endl;
    return 1;
  }
  FILE *f1;
  FILE *f2;
  if ((f1 = fopen(fn1, "r")) == NULL) {
    cerr << "Could not open file " << fn1 << "." << endl;
  }
  if ((f2 = fopen(fn2, "r")) == NULL) {
    cerr << "Could not open file " << fn2 << "." << endl;
  }
  if (f1 != NULL) {
    fclose(f1);
  }
  if (f2 != NULL) {
    fclose(f2);
  }
  if (f1 == NULL || f2 == NULL) {
    return 1;
  }

  boolean configured = configure(config_filename != NULL ? config_filename : "if_diff.config");
  if (config_filename != NULL && !configured) {
    cerr << "Could not open configuration file " << config_filename << "." << endl;
    return 1;
  }

  // Set up reporting; parser makes some consistency checks.
  set_report_stream(cout);

  // Parse the IF files and read their data into memory.
#if defined(IDF_DEBUG)
  cout << "Starting parse." << endl;
#endif
  idfModel model1(fn1);
  idfSmt *lastSmt;
  idfAstDictionary *astRoots;
  if (!parse_file(model1, model1.getDictionary(), lastSmt, astRoots, fn1)) {
    cerr << "Parsing first file " << fn1 << " failed." << endl;
    return 1;
  }
 
#if defined(IDF_DEBUG)
  cout << "Starting to parse second file." << endl;
#endif
  idfModel model2(fn2);
  idfSmt *lastSmt2;
  idfAstDictionary *astRoots2;
  if (!parse_file(model2, model2.getDictionary(), lastSmt, astRoots, fn2)) {
    cerr << "Parsing second file " << fn2 << " failed." << endl;
    return 1;
  }

  // Match data structures between the two files.

  // Find the first smt node in each file, sort the SMT nodes, and
  // match them between files.
#if defined(IDF_DEBUG)
  cout << "Starting match." << endl;
#endif

  int count = idf_match_smt(model1, model2);
#if 0
  if (count == 0) {
    cerr << "No matches found between the two sets of SMT records." << endl;
    return 1;
  }
#endif

  // Symbols are matched in relation to SMT, AST, and other symbols.
  // idf_match_symbols(symbols1, symbols2, firstSmt1, firstSmt2);

#if defined(IDF_DEBUG)
  cout << "Starting AST match." << endl;
#endif
  idf_match_ast(model1, model2);

  idf_match_symbols(model1, model2);

  // Report differences.
#if defined(IDF_DEBUG)
  cout << "Starting report of differences." << endl;
#endif
  idf_report_differences(model1, model2);
  
  if (idf_report_count(internal_error) > 0) {
    return 2;
  }
  if (idf_report_count(difference_note) > 0) {
    return 1;
  }
  return 0;
}
