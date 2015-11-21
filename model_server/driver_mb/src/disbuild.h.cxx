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
#include <general.h>
#include <cLibraryFunctions.h>
#include <msg.h>
#ifndef ISO_CPP_HEADERS
#include <string.h>
#include <stdio.h>
#else /* ISO_CPP_HEADERS */
#include <cstring>
using namespace std;
#include <cstdio>
#endif /* ISO_CPP_HEADERS */
#include <proj.h>
#include <genString.h>
#include <shell_calls.h>
#include <objArr.h>
#include <genError.h>
#include <disbuild.h>

extern "C" void driver_exit(int);

static char const *env_disbuild = NULL;
char const *proj_get_env_disbuild ()
{
    static int env_read = 0;
    if (env_read == 0) {
	env_read = 1;
	env_disbuild = OSapi_getenv ("DISBUILD");
    }

    return env_disbuild;
}

enum
{
    INIT  = 0,
    SLASH = 1,
    CHAR  = 2,
    BOTH  = 3
};

// chops all slashes at the end, returns: the slash - delemeter 
void chop_slashes (char *val, char& del)
{
    if ( val && val[0] ) {
	// find first '/' or '\\' in the tmp directory name, in order 
	del = '/';
	int state = INIT;
	for (int i = strlen(val) - 1 ; i >= 0 && state < BOTH; i --) {
	    char c = val[i];
	    switch (state) {
	    case INIT:
		if (c == '/' || c == '\\') {
		    state = SLASH;
		    del = c;
		    val[i] = '\0';
		} else
		    state = CHAR;
		break;

	    case SLASH:
		if (c == '/' || c == '\\') {
		    val[i] = '\0';
		} else
		    state = BOTH;
		break;

	    case CHAR:
		if (c == '/' || c == '\\') {
		    del = c;
		    state = BOTH;
		}
		break;
		    
	    case BOTH:
		break;
	    }
	}
    } 
}
	    
static genString disbuild_tmpdir;  // XXX: leakage?
char const *disbuild_get_tmpdir ()
{
    static int env_tmpdir_read = 0;
    if (env_tmpdir_read == 0) {
	env_tmpdir_read = 1;
	char del = '/';
        char *dup = strdup(OSapi_getenv ("DISBUILD_TMPDIR"));  // XXX: core if not defined!
	if (dup && dup[0]) {
	    chop_slashes (dup, del);
	    disbuild_tmpdir.printf("%s%c", dup, del);
	} else {
            free (dup);
	    dup = strdup(OSapi_getenv ("ADMINDIR"));
	    if (dup && dup[0]) {
		chop_slashes (dup, del);
		disbuild_tmpdir.printf ("%s%ctmp%cdisbuild%c", dup, del, del, del);
	    } else {
#ifdef _WIN32		
		disbuild_tmpdir = "c:\\temp\\disbuild\\";
#else
		disbuild_tmpdir = "/usr/tmp/disbuild/";
#endif
	    }
	}
        free(dup);

	if (disbuild_tmpdir.length()) {
	    int res = shell_mkdir (disbuild_tmpdir, 1);  // force all the way (-p)
	    if (res != 0) {
		msg("disbuild ERR: failed to create tmp directory $1", error_sev) << (char const *)disbuild_tmpdir << eom ;
		disbuild_tmpdir = 0;
	    }
	}
    }
    return (char const *)disbuild_tmpdir;
}

FILE *disbuild_open_model_file (char const *fname, char const *mode)
{
    Initialize(*disbuild_open_model_file);
    FILE *ff = NULL;
    if (!fname || !fname[0] || !mode || !mode[0]) return ff;
    genString dir = fname;
    char *dir_txt = dir;
    char *buf = strrchr (dir_txt, '/');
    if (!buf) buf = strrchr (dir_txt, '\\');
    if (!buf) {
	msg("disbuild ERR: failed to open file $1", error_sev) << (char const *)fname << eom;
	return ff;
    }

    buf[0] = 0;
    int res = shell_mkdir (dir_txt, 1);
    if (res != 0) {
	msg("disbuild ERR: failed to create directory $1", error_sev) << dir_txt << eom;
	return ff;
    }

    ff = OSapi_fopen (fname, mode);
    if (!ff) {
	msg("disbuild ERR: failed to open file $1, mode $2", error_sev) << fname << eoarg << mode << eom;
    }
    
    return ff;
}

int disbuild_stop_after_failure ()
{
    Initialize(disbuild_stop_after_failure);
    static int env_stop = -1;
    if (env_stop == -1) {
	env_stop = 0;
	char const *env = OSapi_getenv ("DISBUILD_FAILURE_STOP");
	if (env)
	    env_stop = 1;
    }
    return env_stop;
}

static void collect_pmod_projects (objArr& arr, projNode *pn)
{
    Initialize(collect_pmod_projects);
    char const *ln = pn->get_ln();
    if(!pn->is_writable()) {
	msg("disbuild ERR: readonly project $1", error_sev) << ln << eom;
	return;
    }
	
    if (pn->pmod_type == PMOD_PDF) {
	Xref *xr = pn->get_xref ( 13 );
	if (!xr || xr->is_unreal_file ())
	    xr = proj_attach_existing_xref (pn);

	arr.insert_last(pn);
	return;
    }

    pn->refresh_projects();
    Obj *os = parentProject_get_childProjects(pn);
    ObjPtr ob;
    ForEach(ob, *os) {
	projNode* pr = checked_cast(projNode, ob);
	collect_pmod_projects(arr, pr);
    }
}

static objArr pmod_projs;

objArr& disbuild_get_pmod_subprojects ()
{
    Initialize(disbuild_get_pmod_subprojects);
    genString flist;
    static int collected = 0;
    if (collected == 0) {
	collected = 1;
	projNode*proj;
	for (int i = 1; proj = projList::search_list->get_proj(i); i++) {
	    char const *ln = proj->get_ln();
	    if (proj->is_visible_proj() && strcmp(ln, "/ParaDOCS") && strcmp(ln, "/tmp"))
		collect_pmod_projects(pmod_projs, proj);
	}
    }

    return pmod_projs;
}

static genString rule_file;
char const *get_rules_filename ()
{
    Initialize(get_rules_filename);
    static int rule_done = 0;
    if (rule_done == 0) {
	rule_done = 1;
	
	char const *dir = disbuild_get_tmpdir ();
	if (dir)
	    rule_file.printf ("%s%s",dir,"RULES.pmod.lst");
    }
    return rule_file;
}

static genString roots_file;
char const *get_roots_filename ()
{
    Initialize(get_roots_filename);
    static int roots_done = 0;
    if (roots_done == 0) {
	roots_done = 1;
	
	char const *dir = disbuild_get_tmpdir ();
	if (dir)
	    roots_file.printf ("%s%s",dir,"ROOTS.pmod.lst");
    }
    return roots_file;
}

static genString pmods_file;
char const *get_pmods_filename ()
{
    Initialize(get_pmods_filename);
    static int pmods_done = 0;
    if (pmods_done == 0) {
	pmods_done = 1;
	
	char const *dir = disbuild_get_tmpdir ();
	if (dir)
	    pmods_file.printf ("%s%s",dir,"PMODS.pmod.lst");
    }
    return pmods_file;
}

// Splits the line in 'params' parameters, and fills the arr with pointers
//
// returns:  -1 no line
//              0 - line is empty (params == 0)
//             >0 - number of parameters found (must be <= params)
//
//   removes quotes around a param; fills arr[] with found pointers
//   without quotes; substitute space,quote or newline at the end of
//   a parameter with '\0'
int disbuild_split_line (char const *arr[], char *line, int params)
{
    Initialize(disbuild_split_line);
    int ret = -1;
    if (!line) return ret;

    ret = 0;
    if (params <= 0 || !line[0]) return ret;

    char *st = line;
    int ind  = 0;
    int in_quotes = 0;
    int in_param  = 0;
    int st_param  = 0;
    int en_param  = 0;
    char c;
    while ( (c = st[0]) ) {
	if (c == '\n' || c == '\r') {
	    st[0] = 0;
	    break;
	}

	if (c == '\"') {
	    if (in_quotes)
		en_param = 1;
	    else 
		in_quotes = 1;
	} else if (c == ' ') {
	    if (in_quotes == 0) {
		if (in_param == 1)
		    en_param = 1;
	    } else
		if (in_param == 0)
		    st_param = 1;
	} else 
	    if (in_param == 0)
		st_param = 1;
	    
	if (st_param == 1) {
	    in_param = 1;
	    st_param = 0;
	    arr[ind ++ ] = st;
	    ret ++ ;
	} else if (en_param == 1) {
	    en_param  = 0;
	    in_param  = 0;
	    in_quotes = 0;
	    st[0]     = 0;
	    if (ret == params)
		break;
	} 

	st ++ ;
    }
    return ret;
}

projNode *disbuild_get_top_project (char const *nm)
{
    Initialize(disbuild_get_top_project);
    projNode *ans = NULL;
    if (!nm || !nm[0]) return ans;

    projNode *top;
    for (int i = 1 ; top = projList::search_list->get_proj(i) ; i ++) {
	char const *top_nm = top->get_name();
	if (strcmp(top_nm, nm) == 0) {
	    ans = top;
	    break;
	}
    }
    return ans;
}

static char ss = 's';
static char sp = ' ';

char disbuild_s_or_space(int sz)
{
    char ans = ss;
    if (sz == 1)
	ans = sp;

    return ans;
}

void disbuild_get_iff_name (projModule *mod, genString& iff)
{
    Initialize(disbuild_get_iff_name);
    iff = 0;
    char const *pname = mod->paraset_file_name ();
    if (pname && pname[0])
	iff.printf ("%s.%s", (char const *)pname, IFF_EXT);
}

void disbuild_exit (int err_code , char const *msg1, char const *msg2)
{
    Initialize(disbuild_exit);
    char const *pass_name = proj_get_env_disbuild ();
    if (!pass_name) pass_name = "disbuild";

    msg_sev exit_status;

    switch (err_code) {
    case mbQUIT:    exit_status = normal_sev; break;
    case mbWARNING: exit_status = warning_sev; break;
    case mbERROR:   exit_status = error_sev; break;
    case mbABORT:   exit_status = error_sev; break;
    case mbCRASH:   exit_status = catastrophe_sev; break;
    }

    err_code = (err_code == mbWARNING) ? mbQUIT : err_code;

    genString message("");

    if (msg1 && msg1[0]) {
	message = msg1;
	if (msg2 && msg2[0]) {
	    message += " ";
	    message += msg2;
	}
    }

    msg("Pass $1 exiting.  $2", exit_status) << pass_name << eoarg << message.str() << eoarg << eom;

    driver_exit (err_code);
}

