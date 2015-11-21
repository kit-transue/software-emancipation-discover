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
/* file smt_make.C */

#include <msg.h>
#include <genError.h>
#include <proj.h>
#ifdef _WIN32
#include <limits.h>
#else
#include <sys/param.h>
#endif
#include <path.h>
#include <db_intern.h>
#include <Interpreter.h>
#include "cLibraryFunctions.h"

static genString argument_make;
static genString argument_pset;
static genString make_string;
static genString cc_direct;
static genString o_file;
static genString pset_fn;
static genString mkey;
static genString okey;
static projNode * pr_make;
static projNode * pr_pset;

int smt_reset_strings()
{
   argument_make = 0;
   argument_pset = 0;
   make_string = 0;
   cc_direct = 0;
   o_file = 0;
   pset_fn = 0;
   mkey = 0;
   okey = 0;
   pr_make = 0;
   pr_pset = 0;

   return 0;
}

extern "C" void	print_smt_make () 
{
    msg("make_string $1\n") << (char*)make_string << eom;
    msg("cc_direct $1\n") << (char*)cc_direct << eom;
    msg("o_file $1\n") << (char*)o_file << eom;
    msg("pset_fn $1\n") << (char*)pset_fn << eom;
}

static void smt_init_make_strings (const char* fn)
{
    Initialize (smt_init_make_strings);

    argument_make = fn;
    mkey = fn;
    mkey += "/%/.make";
    pr_make = NULL;
    make_string = 0;
    projHeader::fn_to_ln_special_rule (mkey, make_string, MAKE_RULE, pr_make);
    cc_direct = 0;

    DBG {
	print_smt_make ();
    }
}

int smt_get_pset_name(const char* fn, projNode* proj, genString& pname)
{
  if(!proj) return 1;
  if(!proj->get_map()) return 1;
  if(!proj->get_map()->is_para_rule_type(PSET_RULE)) return 1;

  genString rule = fn;
  rule += "/%/.pset";

  proj->fn_to_ln(rule, pname, FILE_TYPE, 0, 1);
  return 0;
}

void preappend_slash_if_NT_abs_path( genString& gs )
{
#ifdef _WIN32
  char resolved_path[MAXPATHLEN];
  if ( is_abs_path((char*)gs.str()) )
  {
    strcpy( resolved_path, gs.str() );
    gs= trn_path_2DIS( resolved_path );
  }
#endif
}

static void smt_init_pset_strings (const char* fn)
{
    Initialize (smt_init_pset_strings);

// boris: 04/21/96 I am fixing bug with the PDF where source files are
//        under ClearCase and pset files are not. In this case for the
//        same physical filename it is possible to have different
//        pset files depending on the project and project local pset rules.
//        The correct fix for this problem is to use "projNode *" along with
//        the physical file name for argument_make. But at the moment, I just
//        eliminate all this functonality by setting argument_make to 0
 
//    argument_pset = fn;
    argument_pset = 0;

    okey = fn;
    okey += "/%/.pset";
    pset_fn = 0;
    pr_pset = NULL;


    projHeader::fn_to_ln_special_rule (okey, pset_fn, PSET_RULE, pr_pset);
	preappend_slash_if_NT_abs_path(pset_fn);
    if (pset_fn.str () && pset_fn.str ()[0] != '/') {
	char resolved_path[MAXPATHLEN];
        char* src_name = resolved_path;
	if (fn[0] != '/')
	    paraset_realpath(fn, src_name);
	else
	    src_name = (char*)fn;
	char* last_slash = strrchr (src_name, '/');
	genString full_path;
	full_path.put_value (src_name, last_slash - src_name + 1);
	full_path += pset_fn;
	pset_fn = full_path;
    }
	    
    o_file = 0;

    DBG {
	print_smt_make ();
    }
}

void get_make_string (const char* filename, genString& gs)
{
    if (filename && *filename) {
	if (!argument_make.str () || strcmp (filename, argument_make))
	    smt_init_make_strings (filename);
	gs = make_string;
    }
}

void get_cc_direct (const char* filename, genString& gs)
{
    if (filename && *filename) {
	if (!argument_make.str () || strcmp (filename, argument_make))
	    smt_init_make_strings (filename);
	if (!cc_direct.str () && pr_make)
	    pr_make->fn_to_ln_imp (mkey, cc_direct);
	gs = cc_direct;
	preappend_slash_if_NT_abs_path(gs);
    }
}

void get_o_file (const char* filename, genString& gs)
{
    if (filename && *filename) {
	if (!argument_pset.str () || strcmp (filename, argument_pset))
	    smt_init_pset_strings (filename);
	if (!o_file.str () && pr_pset)
	    pr_pset->fn_to_ln_imp (okey, o_file);
	gs = o_file;
	preappend_slash_if_NT_abs_path(gs);
    }
}

static void old_get_pset_fn (const char* filename, genString& gs)
{
    if (filename && *filename) {
	if (!argument_pset.str () || strcmp (filename, argument_pset))
	    smt_init_pset_strings (filename);
	gs = pset_fn;
    }
}

void get_pset_fn (const char* filename, genString& gs)
{
    if (filename && *filename)
    {
	old_get_pset_fn(filename, gs);
    }
}

//uses global level pset rules (reallocates memory with each call)
const char * get_paraset_file_name (const char * fn)
{
    static genString rel_name;

    genString gs;
    get_pset_fn (fn, gs);

    if (gs.length ()) {
	rel_name = gs;
    } else {
	rel_name = fn;
	rel_name += DB_SUFF;
    }
    return (char*)rel_name;
}

// use first the subproject pset rule and if failed the global level pset rule
const char *get_paraset_file_name(const char * src_file, projNode *pr)
{
    Initialize(get_paraset_file_name__const_char*_projNode*_genString&);
    const char *pname = 0;
    static genString my_pname;

    if (src_file && src_file[0]) {
	for (; pr && !pr->get_xref (1); pr = pr->find_parent ());

	// returns 1 if failed to find map or map rule
	int res = smt_get_pset_name (src_file, pr, my_pname);
	if (res == 0 && my_pname.length() > 0) 
	    pname = (const char *)my_pname;
	else { 
	    // go to the global rules
	    pname = get_paraset_file_name (src_file);
	    res = 1;
	} 
    }
    return pname;
}

static int pdf_ruleCmd(ClientData cd, Tcl_Interp *interp, int argc, char const **argv)
{
  Interpreter *i = (Interpreter*) cd;

  const char * sub = argv[1];
  const char * fn_ext = argv[2];
  char * fn = 0;
  create_path_2DIS( (char*)fn_ext, &fn );

  genString answer;
  if(strcmp(sub, "directory") == 0){
    get_cc_direct(fn, answer);
    answer = OSPATH( answer.str() );
  } else if(strcmp(sub, "o_file") == 0){
    get_o_file(fn, answer);
    answer = OSPATH( answer.str() );
  } else if(strcmp(sub, "pset") == 0){
    get_pset_fn(fn, answer);
    answer = OSPATH( answer.str() );
  } else if(strcmp(sub, "make") == 0){
    get_make_string(fn, answer);
    //    answer = cnv_argstr_2OS( (char*)answer.str() );  //XXX: Don't correct on WIN32; use raw paths and we'll sort them out if need be.
  } else {
    ostream& os = i->GetOutputStream();
    os << "usage: {directory o_file pset make} <file_name>" << endl;
    return TCL_ERROR;
  }

  free(fn);
  if (answer.length())
    Tcl_SetResult(i->interp, (char*) answer, TCL_VOLATILE);
  return TCL_OK;
}

static int add_commands()
{
    new cliCommandInfo("pdf_rule", pdf_ruleCmd);

    return 0;
}
static int dummy = add_commands();
