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
#include <cLibraryFunctions.h>
#include <msg.h>
#include <disbuild.h>
#include <general.h>
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
#include <machdep.h>
#include <path.h>

int proj_has_real_xref (projNode *pn);

int is_disbuild_cache()
{
    char const *var = proj_get_env_disbuild ();
    int ret = 0;
    if (var && strcmp(var, "cache") == 0)
	ret = 1;

    return ret;
}

static FILE *projs_ff = 0;
static FILE *roots_ff = 0;
static FILE *pmods_ff = 0;
static FILE *files_ff = 0;
static FILE *rules_ff = 0;

static objSet roots;
static objSet pmods;
static objSet projs;
static objSet files;

static int disbuild_map_cnt = 0;

//returns: 0 - success, 1 - failure
int disbuild_start_cache() 
{
    int ret = 0;
    if (is_disbuild_cache() == 0) return ret;

    ret = -1;
    char const *tmp_dir = disbuild_get_tmpdir ();
    if (!tmp_dir) return ret;

    msg("disbuild: tmpdir: $1", normal_sev) << tmp_dir << eom;
    msg("disbuild MSG: ===== cache start =====", normal_sev) << eom;

    char const *roots_fname = get_roots_filename();
    roots_ff = OSapi_fopen ((char const *)roots_fname, "w");
    if (!roots_ff)
	msg("Failed to write file $1", error_sev) << roots_fname << eom ;
    else {
	char const *pmods_fname = get_pmods_filename();
	pmods_ff = OSapi_fopen ((char const *)pmods_fname, "w");
	if (!pmods_ff)
	    msg("Failed to write file $1", error_sev) << pmods_fname << eom ;
	else {
	    char const *rules_fname = get_rules_filename();
	    rules_ff = OSapi_fopen ((char const *)rules_fname, "w");
	    if (!rules_ff)
		msg("Failed to write file $1", error_sev) << rules_fname << eom ;
	    else {
		genString projs_fname; projs_fname.printf ("%s%s",tmp_dir,"projects.pmod.ddb");
		projs_ff = OSapi_fopen ((char const *)projs_fname, "w");
		if (!projs_ff)
		    msg("Failed to write file $1", error_sev) << (char const *)projs_fname << eom ;
		else {
		    genString files_fname; files_fname.printf ("%s%s",tmp_dir,"modules.pmod.ddb");
		    files_ff = OSapi_fopen ((char const *)files_fname, "w");
		    if (!files_ff)
			msg("Failed to write file $1", error_sev) << (char const *)files_fname << eom ;
		    else 
			ret = 0;
		}
	    }
	}
    }

    roots.remove_all(); pmods.remove_all(); projs.remove_all(); files.remove_all(); disbuild_map_cnt = 0;
    return ret;
}

static char const *basename (char const* f)
{
    char const* ans = "NULL";
    if (f && f[0]) {
	ans = &f[strlen(f)];
	for (int ii = strlen(f) - 1 ; ii >= 0 ; ii --) {
	    if (f[ii] == '/' || f[ii] == '\\')
		break;

	    ans = &f[ii];
	}
    }

    if ( ans[0] == 0 || ans[0] == '\n' || ans[0] == '\015' )
	ans = "NULL";
    return ans;
}

void disbuild_end_cache ()
{
    if (is_disbuild_cache() == 0) return;

    if (roots_ff) fclose (roots_ff);
    if (pmods_ff) fclose (pmods_ff);
    if (rules_ff) fclose (rules_ff);
    if (projs_ff) fclose (projs_ff);
    if (files_ff) fclose (files_ff);

    int roots_sz = roots.size();
    int pmods_sz = pmods.size();
    char const *indent = "    ";
    msg("===== Cache Summary =====\n$1$2 ROOT$3\t $4", normal_sev) << indent << eoarg << roots_sz << eoarg << disbuild_s_or_space(roots_sz) << eoarg << basename(get_roots_filename()) << eom;

    msg("$1$2 PMOD$3\t $4", normal_sev) << indent << eoarg << pmods_sz << eoarg << disbuild_s_or_space(pmods_sz) << eoarg << basename(get_pmods_filename()) << eom;

    if (disbuild_map_cnt == 0) {
	OSapi_unlink(get_rules_filename());
	msg("NO mapped files.", warning_sev) << eom;
    } else {
	msg("$1$2 mapping$3\t $4", normal_sev) << indent << eoarg << disbuild_map_cnt << eoarg << disbuild_s_or_space(disbuild_map_cnt) << eoarg << basename(get_rules_filename()) << eom;
    }
    msg("=========================", normal_sev) << eom;


    roots.remove_all();
    pmods.remove_all();
}

// returns: 0 - success; -1 failure
static int add_line (FILE *ff, char const *txt)
{
    int ret = -1;
    if (ff && txt && txt[0]) {
	size_t len = OSapi_fwrite (txt, strlen(txt), 1, ff);
	if (len == 1)
	    ret = 0;
    }
    return ret;
}

int disbuild_cache_add_rules (char const *txt, projModule *mod, int& map_cnt, int& no_cnt)
{
    Initialize(add_rules);
    int ret = -1;
    if (!txt || !txt[0]) return ret;

    projNode *pn = NULL;
    genString mod_txt;
    if ( mod && (pn = mod->get_project_internal()) ) {
	for (projNode *proj = pn ; proj ; proj = proj->find_parent ())
	    pn = proj;

	mod_txt.printf ("\"%s\" \"%s\" \"%s\" \"%s\"\n", txt, pn->get_name(), mod->get_name(), mod->get_phys_filename());
	map_cnt ++ ;
    } else {
	mod_txt.printf ("\"%s\" NULL\n", txt);
	no_cnt ++ ;
    }

    ret = add_line (rules_ff, mod_txt);
    if (ret == 0)
	disbuild_map_cnt ++;

    return ret;
}

static int add_roots (projNode *prj, char const *txt)
{
    int ret = 0;
    if (!roots.includes(prj)) {
	ret = add_line (roots_ff, txt);
	if (ret == 0)
	    roots.insert(prj);
    }
    return ret;
}

static int add_pmods (projNode *prj, char const *txt)
{
    int ret = 0;
    if (!pmods.includes(prj)) {
	ret = add_line (pmods_ff, txt);
	if (ret == 0)
	    pmods.insert(prj);
    }
    return ret;
}

static int add_projs (projNode *prj, char const *txt)
{
    int ret = 0;
    if (!projs.includes(prj)) {
	ret = add_line (projs_ff, txt);
	if (ret == 0)
	    projs.insert(prj);
    }
    return ret;
}

static int add_files (projModule *mod, char const *txt)
{
    int ret = 0;

    if (!files.includes(mod)) {
	ret = add_line (files_ff, txt);
	if (ret == 0)
	    files.insert(mod);
    }
    return ret;
}

static char const *null_name = "NULL";
static genString cur_root    = null_name;
static genString cur_pmod    = null_name;

static int dbase_project (projNode *prj)
{
    static int rec_num      = 0;
    char const *parent_root = null_name;
    char const *parent_pmod = null_name;

    if (!prj|| !prj->get_name()) return -1;
    char const *left_name = prj->get_name();

    genString dir_name; char const *dir_str;
    prj->ln_to_fn_imp (left_name, dir_name, DIR_TYPE, 0, 1);
    if ( !(dir_str = dir_name) ) 
	return -1;

    dir_str   = OSPATH(dir_str);

    if (prj->fake_name.length() == 0)
	prj->fn_to_ln(dir_name, prj->fake_name, DIR_TYPE, 0, 1);
	
    char const *meta_name = prj->fake_name;
    if (!meta_name || !meta_name[0])
	meta_name = null_name;

    int is_visible  = prj->is_visible_proj ();
    int is_root     = (prj->find_parent() == 0);
    int is_pmod     = (prj->pmod_type == PMOD_PDF || proj_has_real_xref(prj));

    if (is_root) {
	cur_root = left_name;
	cur_pmod = null_name;
    } else
	parent_root = cur_root;


    if (is_pmod)
	cur_pmod = left_name;
    else
	parent_pmod = cur_pmod;

    genString sym_name; char const *sym_str = null_name;
    if (is_pmod || is_root) {
	prj->xref_file_name (sym_name);
	sym_name += ".sym";
	sym_str = OSPATH((char const *)sym_name);
    }

    genString record;
    // proj record: "left" "middle" "right" "model" is_visible is_root is_pmod "parent_root" "parent_pmod"
    record.printf("\"%s\" \"%s\" \"%s\" \"%s\" %d %d %d \"%s\" \"%s\"\n", left_name, dir_str, meta_name, sym_str,
		  is_visible, is_root, is_pmod, parent_root, parent_pmod);

    if (add_projs (prj, record) == 0) 
	return ++rec_num;

    return -1;
}


// returns: 0 - success; -1 failure
int disbuild_cache_project (projNode *prj, char const *txt)
{
    int ret = -1;
    if (!prj || !txt || !txt[0]) return ret;

    int rec_num = dbase_project (prj);

    ret = 0;
    if (prj->is_visible_proj () == 0) return ret;

    if (prj->find_parent() == 0)
	ret = add_roots (prj, txt);
	    
    if (ret == 0 && prj->pmod_type == PMOD_PDF)
	ret = add_pmods (prj, txt);
    
    return ret;
}

// returns: 0 - success; -1 failure
int disbuild_cache_module (projModule *mod, char const *txt)
{
    int ret = -1;
    if (!mod || !txt || !txt[0]) return ret;

    for ( ; isspace(txt[0]) ; txt++ )
	;

    genString tmp = txt; char *ptr = tmp; int len = strlen (ptr);
    if (!len) return ret;

    for ( int ii = len - 1 ; ii && isspace(ptr[ii]) ; ii -- )
	ptr[ii] = 0;
    
    genString record;
    record.printf ("%s \"%s\" \"%s\"\n", ptr, (char const *)cur_root, (char const *)cur_pmod);
    
    if (add_files (mod, record) == 0)
	ret = 0;

    return ret;
}
 
