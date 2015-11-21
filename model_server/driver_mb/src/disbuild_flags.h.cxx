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
#include <disbuild.h>
#include <disbuild_analysis.h>
#include <general.h>
#include <cLibraryFunctions.h>
#ifndef ISO_CPP_HEADERS
#include <string.h>
#include <stdio.h>
#else /* ISO_CPP_HEADERS */
#include <cstring>
namespace std {};
using namespace std;
#include <cstdio>
#endif /* ISO_CPP_HEADERS */
#include <proj.h>

// Clean up after objRelation.h stomps over some useful names.
#undef U
#undef S
#undef D
#undef M
#undef C
#undef W

#include <genString.h>
#include <genError.h>
#include <genTry.h>
#include <symbolPtr.h>
#include <machdep.h>
#include <msg.h>
#include <smt_cppparm.h>
#include <startprocess.h>

#define IFF_EXT "iff"

extern char const *ATT_language( symbolPtr& sym);
extern void        get_cc_direct (char const *filename, genString& gs);
extern void        get_make_string(char const *fn, genString&);
extern char const *els_flags(char const *lan);
extern char const *els_guess_language(char const *fn);
extern void        extern_els_init ();

genArr(genString);

#define EDG_C        "C"
#define EDG_CPP      "CPP"
#define EDG_ESQL_C   "ESQL_C"
#define EDG_ESQL_CPP "ESQL_CPP"
#define END_TAG      "NULL"

static char const *edg_languages [] = {
    EDG_C,
    EDG_CPP,
    EDG_ESQL_C,
    EDG_ESQL_CPP,
    END_TAG
};

static int is_edg_language (char const *lan)
{
    Initialize(is_edg_language);
    int res  = 0;
    if (!lan || !lan[0]) return res;

    int i = 0;
    while (1) {
	char const *cur = edg_languages[i];
	if (strcmp(cur, END_TAG) == 0) break;
	if ( strcmp(cur, lan) == 0 ) {
	    res = 1;
	    break;
	}
	i ++;
    }
    return res;
}

void disbuild_get_language (projModule *mod, genString& lan)
{
    Initialize(disbuild_get_language);
    lan = 0;
    if (mod) {
	symbolPtr sym(mod);
	lan = ATT_language (sym);
	if (lan.length() && strcmp(lan, "ELS") == 0) {
	    char const *fn = mod->get_phys_filename();
	    lan = els_guess_language (fn);
	}
    }
}


void disbuild_get_dir_name (projModule *mod, genString& dir)
{
    Initialize(disbuild_get_dir_name);
    char const *fname = mod->get_pr_name();
    get_cc_direct(fname, dir);
    if (dir.length() == 0) dir = "NULL";
}


// Given a an array of flag strings to be passed to the parser,
// escape it with quotes, backslashes appropriate to the shell/system command.
// Bourne shell on Unix should probably be different from Microsoft.
static void escape_for_shell(genArrOf(genString) &flags, genString &txt)
{
    // Stitch together with the C++ standard library way of doing things.
    list<string> xflags;
    for (int i = 0; i < flags.size(); i += 1) {
	xflags.push_back(string(flags[i]->str()));
    }
    // Do the work in startprocess.
    string xtxt = MBDriver::prepare_command_arguments(xflags);
    txt = xtxt.c_str();
}


// returns: 0 - succeeded
//         -1 - failed (wrong module)
//          1 - unknown language
//          2 - no make output
int disbuild_get_esql_flags (projModule *mod, char const *lan, genString& fla)
{
    Initialize(disbuild_get_esql_flags);
    fla = 0;
    char const *fn    = (mod) ? mod->get_phys_filename() : 0;
    if (!fn || !fn[0]) return -1;

    genString suff;  genString edg_flags; genString edg_fn; genString cur_lan; genString cur_fla;
    if (strcmp(lan,EDG_ESQL_C) == 0) {
	suff    = customize::getStrPref("cDfltSrcSuffix");
	cur_lan = "C";
	cur_fla = "--c";
    } else {
	suff    = customize::getStrPref("cPlusDfltSrcSuffix");
	cur_lan = "CPP";
	cur_fla = "--c++";
    }
    genString esql_flags;
    genArrOf(genString) esql_flag_array;
    int verbatim = false;
    int err = smt_get_esql_param_array (mod, verbatim, esql_flags, esql_flag_array);
    if (err) return 1;
    if (!verbatim) {
	escape_for_shell(esql_flag_array, esql_flags);
    }

    char const *esql =  (esql_flags.length() == 0) ? "" : esql_flags;

    char const *ptr_suff = suff;
    if (ptr_suff[0] == '.')
	edg_fn.printf("%s%s"  ,(char const *)fn, ptr_suff);
    else
	edg_fn.printf("%s.%s" ,(char const *)fn, ptr_suff);

    genArrOf(genString) edg_flag_array;
    err = smt_make_all_param_array ((char const *)edg_fn, (char const *)cur_lan, edg_flag_array);
    escape_for_shell(edg_flag_array, edg_flags);
    if (err) return 1;

    fla.printf ("%s__FLAGSflagsFLAGS__%s %s", esql, (char const *)cur_fla, (char const *)edg_flags);
    return 0;
}

static int disbuild_get_els_make_output(char const *fn, genString& output)
{
    static genString saved_make_action = "";
    static genString saved_make_output = "";
    int ret_val = 0;

    /* Get the action string from the make rule of the pdf. */
    genString make_str = "";
    get_make_string(fn, make_str);
    cnv_argstr_path_2OS((char const *)make_str);
    msg("Make action from pdf:\n$1", normal_sev) << (char const *)make_str << eom;

    /* If the make action is same as the saved make action use
       saved make output otherwise execute make action. */
    if (make_str.length() && saved_make_action.length() &&
	  make_str.compare((char const *)saved_make_action) == 0) {
        output = saved_make_output;
    }
    else if (make_str.length()) {
        genString cmd;
#ifdef _WIN32
	cmd.printf("cmd /c \"%s\"", (char const *)make_str);
	char const *type = "rt";
#else
	cmd.printf("sh -c \"%s\"", (char const *)make_str);
	char const *type = "r";
#endif
   	FILE *f = OSapi_popen((char const *)cmd, type);
	if (f) {
	    char line[256];
	    int len;
	    while (OSapi_fgets(line, 255, f) != NULL) {
	        len = OSapi_strlen(line);
		if (line[len-1] == '\n') {
		    line[len-1] = '\0';
		}
		output += line;
	    }
	    OSapi_pclose(f);

	    /* save make action and output for next time */
	    saved_make_action = (char const *)make_str;
	    saved_make_output = (char const *)output;
	}
	else {
	    msg("Failed to open pipe for command: $1", error_sev) << (char const *)cmd << eom;
	    ret_val = 1;
	}
    }
    msg("Make output:\n$1\n", normal_sev) << (char const *)output << eom; 
    return ret_val;
} 

// returns: 0 - succeeded
//         -1 - failed (wrong module)
//          1 - unknown language
//          2 - no make output
int disbuild_get_flags (projModule *mod , genString& fla)
{
    Initialize(disbuild_get_flags);
    int res = -1;
    fla     = "";

    if (!mod) return res;

    symbolPtr sym(mod);
    char const *lan = ATT_language (sym);
    if (!lan || !lan[0]) {
	res = 1;
	return res;
    }

    if (is_edg_language (lan)) {
	if (strncmp(lan, "ESQL", 4) == 0) {
	    res = disbuild_get_esql_flags (mod, lan, fla);
	} else {
	    genArrOf(genString) flag_array;
	    genArrOf(genString) cpp_flags;
	    genArrOf(genString) comp_flags;

	    if ( smt_get_cpp_param_array (mod, cpp_flags) == 0 ) {
		int comp = smt_get_comp_param_array (mod, comp_flags);

		lan = ATT_language (sym);
		genString edg_lan = ((strcmp(lan,"CPP") == 0) ? "--c++" : "--c");
		flag_array.append(&edg_lan);
		for (int i = 0; i < cpp_flags.size(); i += 1) {
		    flag_array.append(cpp_flags[i]);
		}
		if (comp == 0) {
		    for (int i = 0; i < comp_flags.size(); i += 1) {
			flag_array.append(comp_flags[i]);
		    }
		}
		escape_for_shell(flag_array, fla);

		res = 0;
		
	    } else
		res = 2;
	}
    } else {
	extern_els_init ();
	char const *fn      = mod->get_phys_filename();
	char const *els_lan = lan;
        
        if (strcmp(els_lan, "ELS") == 0) {
	    els_lan = els_guess_language (fn);
        }
	if (els_lan) {
	    char const *pref_flags = els_flags (els_lan); 
	    genString pdf_flags = "";
            disbuild_get_els_make_output(fn, pdf_flags);

	    if (!pref_flags && !pdf_flags.length()) 
		fla = "";
	    else if (pref_flags && !pdf_flags.length())
		fla = pref_flags;
	    else if (!pref_flags && pdf_flags.length())
		fla = pdf_flags;
	    else
		fla.printf ("%s %s", (char const *)pref_flags, (char const *)pdf_flags);

	    // boris 090398 it is OK for ELS not to have any flags (upto MG)
	    res = 0;
	}
    }
    return res;
}
