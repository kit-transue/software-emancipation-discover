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
#ifndef ISO_CPP_HEADERS
#include <stdlib.h>
#include <assert.h>
#else /* ISO_CPP_HEADERS */
#include <cstdlib>
namespace std {};
using namespace std;
#include <cassert>
#endif /* ISO_CPP_HEADERS */

#include <cLibraryFunctions.h>
#include <msg.h>
#include "smt.h"
#include "symbolPtr.h"
#include "driver.h"
#include "cmd.h"
#include "RTL.h"
#include "db.h"
#include "proj.h"
#include "machdep.h"

#define	MAXPATHLEN	1024
#define F_OK 0

int move_relations(Relational *, Relational *, int);

extern "C" {
    void* import_any_file(const char*);
    void viewerShell_open_view(appPtr, repType, appTreePtr);
    void viewerShell_map();
    void init_stat ();
    void get_stat (char*);
    int smt_to_cpp (smtHeader*);
}

extern "C" int smt_check_asynch_data ();

//
// proj_path_report_app()  in necessary to report newly imported files to 
// the app hash table.
//
extern app *proj_path_find_app (const char *fname, int no_raw = 0);
extern void proj_path_report_app (app *, projNode *prj = NULL);
extern app *proj_path_convert_app (genString& path, genString& real_path, projNode *prj = NULL);
    

smtHeader * smt_header;

projNode*   pmod_proj_for_import = 0;
void set_pmod_proj_for_import(projNode *pr, int check)
{
  if (check && (pmod_proj_for_import == 0))
    pmod_proj_for_import = pr;
  else if (!check)
    pmod_proj_for_import = pr;    
}


#ifdef XXX_smt_prog_import // only used from browser_import.*?
static int temp_lname = 0;
smtHeader * smt_prog_import(char const *pn, char const */*text*/, smtLanguage l,
                            smtTree * /*scope*/)
// import file pn;
// If pn is a header file, use the pmods to find a .C file that includes it
// always reparse pn  
{
  Initialize(smt_prog_import);

  char lnbuf[400];
  smtHeader * h = 0, * old_h = 0;
  init_stat ();

  if(pn == 0 || *pn == 0) {
    pn = (char *)((l == smt_C || l == smt_ESQL_C) ? "*.c" : "*.C");
  }
  else {
       // check if file is really embedded SQL
       smtLanguage guessed_l = smtLanguage(guess_file_language(pn));
       // don't change between C and C++, only whether is ESQL or not:
       if (l == smt_C) {
	   if (guessed_l == smt_ESQL_C || guessed_l == smt_ESQL_CPLUSPLUS) {
	       l = smt_ESQL_C;
	   }
       }
       else if (l == smt_CPLUSPLUS) {
	   if (guessed_l == smt_ESQL_C || guessed_l == smt_ESQL_CPLUSPLUS) {
	       l = smt_ESQL_CPLUSPLUS;
	   }
       }
  }

  if(*pn == '*') {
    OSapi_sprintf(lnbuf, "*%d%S", temp_lname++, pn);
    pn = lnbuf;
  }

  // Try to find existent SMT
  h = smtHeader::find_from_realpath (pn);
  old_h = h;

  if (h && h->parsed && h->drn == 0 && h->ast_exist) return h;

  if(h && h->parsed) {
    h->cut_ast();
    obj_unload(h);
    h = 0;
  }

  // get logical name from physical name
  projNode *proj = 0;
  genString ln;

  // find the nearest project that contains a pmod
  proj = projNode::fn_to_pmod_proj(pn, ln);
  set_pmod_proj_for_import(proj, 0);
   
  if (XREF_check_files_including((char *)ln)) {
        if (h)
            obj_delete(h);

	h = new smtHeader((char *) ln, pn, l, &proj);

	if (h->load() == -1) {
	  obj_delete(h);
	  return 0;
	}
	h->parse();
        if (check_smt_asynch_data()) {
            smt_asynch_data* ad = get_smt_asynch_data ();
	    set_smt_asynch_data (h, ad->parse_header, ad->oper,
		                 ad->data);
	}
        smtHeader* main_h = (smtHeader*)XREF_get_files_including((char *)ln);
	if (main_h) {
	    main_h->make_ast ();
	    if (!check_smt_asynch_data ())
		if (main_h->srcbuf == 0)
		    obj_unload(main_h);
	    return h;
	} else {
	    obj_unload(h);
	    return 0;
	}
  }

  msg("No model to load for $1", catastrophe_sev) << (char *)ln << eom;
  return 0;
#endif //}
#ifdef XXX_load_smtHeader
smtHeader * load_smtHeader(char * pn, char * text, smtLanguage l,smtTree* scope)
{
  Initialize(load_smtHeader);

  Assert (text);
  char lnbuf[400];
  if(pn == 0 || *pn == 0)
  {
    pn = (char *)((l == smt_C || l == smt_ESQL_C) ? "*.c" : "*.C");
  }

  if(*pn == '*')
  {
    OSapi_sprintf(lnbuf, "*%d%S", temp_lname++, pn);
    pn = lnbuf;
  }

  genString ln;
  projHeader::fn_to_ln (pn, ln);
  smtHeader* h = new smtHeader(ln, pn, l);

  // Open SMT
  if(h->srcbuf)
  {
    free(h->srcbuf);
    h->srcbuf = 0;
  }
  h->load(text);

  if(scope)
  {
    h->scope = 1;
    put_relation(scope_of_smtHeader, h, scope);
  }

  h->parse();
  return h;
}
#endif
