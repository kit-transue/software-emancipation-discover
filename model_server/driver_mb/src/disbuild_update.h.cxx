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
#include <projList.h>
#include <symbolArr.h>
#include <symbolSet.h>

#include <customize.h>

#include <pathcheck.h>
#ifndef ISO_CPP_HEADERS
#include <stdlib.h>
#else /* ISO_CPP_HEADERS */
#include <cstdlib>
#endif /* ISO_CPP_HEADERS */

void xref_set_doing_second_pass(int);
int  proj_has_real_xref (projNode *pn);
int els_parse(const char*iff, projModule*mod);
void load_pdfs_complete();
void pr_bad_proj_name(const char *projname);

static symbolSet proj_nif_modules;
static symbolArr succeeded;
static symbolArr failed;

static int num_removed = 0;

static genString uncompress_exe;
static genString compress_ext;

static projNode *update_proj = 0;

int is_disbuild_update ()
{
    Initialize(is_disbuild_update);
    static int disbuild_update = -1;
    if (disbuild_update == -1) {
	disbuild_update = 0;
	const char *var = proj_get_env_disbuild ();
	if (var && strcmp(var, "update") == 0)
	    disbuild_update = 1;
    }
    return disbuild_update;
}

void disbuild_update_start() 
{
    Initialize(disbuild_update_start);
    int ret    = -1;

    const char *tmp_dir = disbuild_get_tmpdir ();
    if (tmp_dir && tmp_dir[0]) {
	ret = 0;
	msg("tmpdir: $1", normal_sev) << tmp_dir << eom;
	msg("===== update start =====", normal_sev) << eom;

	proj_nif_modules.removeAll();
	succeeded.removeAll();
	failed.removeAll();
        num_removed = 0;

	uncompress_exe = customize::getStrPref("MBiffUnCompressExe");
	compress_ext   = customize::getStrPref("MBiffCompressExt");
	if (uncompress_exe.length() == 0) {
	    if (compress_ext.length()) {
		msg("No pref \'MBiffUnCompressExe\'. Ignore pref \'MBiffCompressExt\'.", warning_sev) << eom;
		compress_ext = "";
	    }
	} else {
	    if (OSapi_access((char *)uncompress_exe, X_OK) != 0) {
		msg("No executable pref \'MBiffUnCompressExe\'= $1\n             Disable uncompress.", warning_sev) << (char *)uncompress_exe << eom;
		compress_ext = ""; uncompress_exe = "";
	    } else {
		if (compress_ext.length()) {
		    msg("Uncompress exe $1\n         compressed ext $2", normal_sev) << (char  *)uncompress_exe << eoarg << (char *)compress_ext << eom;
		} else {
		    msg("No Pref \'MBiffCompressExt\'. Ignore pref \'MBiffUnCompressExe\'.", warning_sev) << eom;
		    compress_ext = ""; uncompress_exe = "";
		}
	    }
	}
    }
}

void disbuild_update_end() 
{
    Initialize(disbuild_update_end);
    const char *nm = (update_proj) ? update_proj->get_name() : "NULL";
    int nif_sz = proj_nif_modules.size();
    int suc_sz = succeeded.size();
    int fal_sz = failed.size();

    msg("===== Update Summary project $1 =====", normal_sev) << nm << eom;
    msg("$1 \tmodule$2 succeeded update", normal_sev)
	<< suc_sz << eoarg
	<< disbuild_s_or_space(suc_sz) << eoarg << eom;
    msg("$1 \tmodule$2 needed update for project $3", normal_sev)
	<< nif_sz << eoarg
	<< disbuild_s_or_space(nif_sz) << eoarg
	<< nm << eoarg << eom;
    msg("$1 \tmodule$2 failed update", normal_sev)
	<< fal_sz << eoarg
	<< disbuild_s_or_space(fal_sz) << eom;
    msg("$1 \tmodule$2 removed", normal_sev)
	<< num_removed << eoarg
	<< disbuild_s_or_space(num_removed) << eom;

    msg("=====================================", normal_sev) << eom;
    msg("===== update end =====", normal_sev) << eom;
}

void disbuild_update_usage_error (const char* mesg)
{
    Initialize(disbuild_update_usage_error);
    msg("Usage error for DISBUILD=update.", error_sev) << eom;
    disbuild_exit (mbERROR, mesg);
}

static projNode *update_check_pmod(const char *proj_name)
{
    Initialize(update_check_pmod);
    projNode *projnode = projNode::find_project (proj_name);
    if (!projnode) pr_bad_proj_name(proj_name);
 
    if (!projnode || !projnode->pmod_type == PMOD_PDF)
	projnode = 0;
    else {
	Xref *proj_xref = projnode->get_xref();
	/* check for parents pmod */
	if (proj_xref && !proj_xref->is_unreal_file() && !projnode->get_xref(1)) {
	    projNode *par = proj_xref->get_projNode();

	    if (par) {
		msg("Parent project $1 of subproject $2 contains a pmod;\n           projects and their subprojects cannot both have pmods.", error_sev) << par->get_name() << eoarg << par->get_name() << eom;
		projnode = 0;

	    }
	} else if (Xref::test_subproj_pmod(projnode) == -1) {
	    /* check for sub project pmod     */
	    msg("Project $1 has one or more subprojects that contain a pmod\n           projects and their subprojects cannot both have pmods.", error_sev) << proj_name << eom;
	    projnode = 0;
	} else if (!projnode->get_xref(1)) {
	    genString xref_file;
	    projnode->xref_file_name(xref_file);
	    if (xref_file.length())
		projnode->set_xref(new Xref(xref_file, 1, projnode));
	}
     }
    update_proj = projnode;
    return projnode;
}


static int update_get_proj_nif_modules(projNode *pn, bool remove_flag)
{
    Initialize(update_get_proj_nif_modules);
    int mod_cnt = 0;
    if (!pn) return mod_cnt;

    genString nif_file;
    nif_file.printf ("%s%s", disbuild_get_tmpdir(), NIF_F);
    /* If remove_flag is true and nif_file does not exist, it is ok. */
    if (remove_flag && OSapi_access(nif_file, F_OK) == -1 ) {
        return mod_cnt;
    }

    FILE *nif = disbuild_open_model_file((char *)nif_file,"r");
    if (!nif) {
	msg("Failed to open nif file $1", error_sev) << (char *)nif_file << eom;
	return mod_cnt;
    }

    const char *proj_name = pn->get_name();
    projNode *root        = pn->root_project();
    root->refresh_all();

    objSet iffs;

    int line_cnt = 0;
    char *line_buf;
    char line[MAXPATHLEN];
    //   nif line: "iff_name" "phys_name" "pmod_proj" "ln name"
    while ( (line_buf = OSapi_fgets (line, MAXPATHLEN, nif)) ) {
	line_cnt ++ ;
	char const *arr[4];
	int res = disbuild_split_line (arr, line, 4);
	if (res == 0)
	    break;
	else if (res != 4) {
	    msg("Read wrong line $1 in nif file $2", error_sev) << line_cnt << eoarg << (char *)nif_file << eom;
	    mod_cnt = 0;
	    break;
	} else {
	    if (strcmp(proj_name,arr[2]) == 0) {
//		projModule *mod = root->find_module (arr[3]);
		int idx = -1;
		projNode *found_pn = 0;

		projModule *mod = find_module_by_physname(arr[1], pn, idx, found_pn);
		
		if (! mod) {
		    msg("Failed to find module $1 in project $2 : line $3, nif file $4", warning_sev) << arr[1] << eoarg << proj_name << eoarg << line_cnt << eoarg << (char *)nif_file << eoarg << eom;

		} else {
		    if (!iffs.includes (mod)) {
			iffs.insert(mod);
			int compressed = 0;
			if (uncompress_exe.length()) {
			    genString zip;
			    zip.printf ("%s%s", arr[0], (char *)compress_ext);
			    if (OSapi_access((char *)zip, R_OK) != 0) {
				msg("No compressed IFF $1", warning_sev) << (char *)zip << eom;
			    } else
				compressed = 1;
			}
			if (! compressed && OSapi_access(arr[0], R_OK) != 0) {
			    msg("No IFF $1", warning_sev) << arr[0] << eom;
			} else {
			    const char *psetname = mod->paraset_file_name ();
			    if (psetname && strncmp(psetname, arr[0], strlen((char *)psetname)) == 0) {
				proj_nif_modules.insert(mod);
				mod_cnt ++ ;
			    } else {
				msg("Wrong iff and pset names for module $1\n           pset  $2\n           iff   $3\n           line $4, nif file $5", error_sev) << arr[3] << eoarg << psetname << eoarg << arr[0] << eoarg << line_cnt << eoarg << (char *)nif_file << eom;
			    }
			}
		    }
		}
	    }
	}
    }
    OSapi_fclose (nif);
    return mod_cnt;
}

int update_set_home_project(projNode *pn)
{
    Initialize(update_set_home_project);
    int res = 0;
    // set the projnode to be current project
    static int complete_pdf;
    projNode::set_home_proj (pn);
    
    if (!complete_pdf) {
	load_pdfs_complete();
	complete_pdf = 1;
    }

    if (pn->get_xref() && customize::get_write_flag())
	pn->get_xref()->make_room(2000000);

    // Make sure that we scan this project and nothing else
    projList::search_list->scan_projects (pn->get_name());
    projNode* pr;
    objArr oarr;
    ForEachProj(i,pr) {
	if (pr->is_visible_proj() && pr != pn) 
	    oarr.insert_last(pr);
    }

    Obj *cur;
    ForEach(cur,oarr) {
	pr = checked_cast(projNode,cur);
	projList::search_list->rem_proj_from_all_lists(pr);
    }

    // get the xref from project node
    Xref *xref = pn->get_xref();

    if (!xref || !xref->is_writable()) {
	msg("Could not write pmod files for project $1", error_sev) << pn->get_name() << eom;
    } else
	res = 1;

    return res;
}

static int is_MBforceIFCleanup () 
{
    Initialize(is_MBforceIFCleanup);
    static int do_cleanup = -1;
    if (do_cleanup == -1) {
	do_cleanup = 0;
	genString val = customize::getStrPref("MBforceIFCleanup");
	if (val.length() && strncmp((char *)val,"yes",3) == 0)
	    do_cleanup =1;
    }
    return do_cleanup;
}

void start_metering();
void stop_metering(char const * root_fcn);

int disbuild_update_project (const char *proj_name, bool remove_modules)
{
    Initialize(disbuild_update_project);
    int res = 1;
    if (!is_disbuild_update()) return res;

    disbuild_update_start ();

    projNode *pn = 0;

    if (!proj_name || !proj_name[0])            disbuild_exit (mbABORT, "No project name");
    if ( !(pn = update_check_pmod(proj_name)) ) disbuild_exit (mbABORT, "Wrong project", proj_name);
    if (update_set_home_project(pn) == 0)       disbuild_exit (mbERROR, "Failed to set home project", proj_name);
    if (update_get_proj_nif_modules(pn, remove_modules) < 0)    disbuild_exit (mbERROR, "Wrong nif for project", proj_name);
    if (proj_nif_modules.size() == 0 && !remove_modules)           disbuild_exit (mbWARNING, "No nif modules for project", proj_name);

    xref_set_doing_second_pass (1);

    symbolPtr sym;
    char const *meter_start = OSapi_getenv("DIS_START_METERING");
    int start_num = (meter_start && meter_start[0]) ? atoi(meter_start) : -1;
    int cur_num = 0; int cur_meter = 0;

    //send total number of modules to be updated
    // <steps name="update_file" count=size() for="update pass"/>
    define_progress_steps("update pass", "update_file", proj_nif_modules.size());

    ForEachT(sym, proj_nif_modules) {
	cur_num ++;

	if (start_num == cur_num) {
	    start_metering();
	    cur_meter = 1;
	    msg("start_metering: module $1", normal_sev) << cur_num << eom;
	}

	projModule *m  = checked_cast(projModule,sym);
	const char *fn = m->get_phys_filename();
	genString iff;
	disbuild_get_iff_name (m, iff);

	if ( OSapi_access ((char *)iff, R_OK) != 0 ) {
	    genString zip;
	    zip.printf ("%s%s", (char *)iff, (char *)compress_ext);
	    msg("Unzip IFF: $1 ... ", normal_sev) << OSPATH((char *)zip) << eom;
	    int status = vsysteml (OSPATH((char *)uncompress_exe), OSPATH((char *)zip), (char *)0);
	    if (status == 0 && OSapi_access ((char *)iff, R_OK) == 0) {
		msg("unzip succeeded", normal_sev) << eom;
	    } else {
		msg("unzip FAILED", error_sev) << eom;
		failed.insert_last(m);
		continue;
	    }
	}

	msg("Processing IFF: $1 FN: $2 LN: $3", normal_sev) << (char *)iff << eoarg << fn << eoarg << m->get_name() << eom;    

	int delete_pset = 1;
	m->touch_module(delete_pset);

	res = els_parse((const char*)iff, m);
	if (res)
	    succeeded.insert_last(m);
	else {
	    msg("Failed on IFF: $1", error_sev) << (char *)iff << eom;
	    failed.insert_last(m);
	}
	if (is_MBforceIFCleanup()) OSapi_unlink((char*)iff);
	if ( cur_meter == 20 ) {
	    stop_metering(0);
	    stop_metering ("XrefTable_insert_module(XrefTable_local *, app *)");
	    stop_metering("els_parse__iff_parse__char*fname");
	    msg("stop_metering: module$1", normal_sev) << cur_num << eom;
	    cur_meter = 0;
	} else if (cur_meter)
	    cur_meter ++;

        //send a progress message after updating one file
        report_progress("update_file");
    }

    /*If remove_modules is set then go through all the modules in the pmod
    and remove the modules that either can't be mapped or do not have pset 
    files.*/
    if (remove_modules) {
         genString remlst_file;
         remlst_file.printf ("%s%s", disbuild_get_tmpdir(), REM_F);
         FILE *remlst = disbuild_open_model_file((char *)remlst_file,"w");
         Xref *xref = pn->get_xref();
         symbolArr allfiles;
         ddSelector selector;
         selector.add(DD_MODULE);
         xref->add_symbols(allfiles, selector);
         symbolPtr fsym;  
         genString logicalname;
         genString physicalname;
         bool remove = 0;
         ForEachS(fsym, allfiles) {
	     if (fsym->get_kind() == DD_MODULE && fsym.get_has_def_file()) {
	         fsym.get_name(logicalname);
                 remove = 0;
                 pn->ln_to_fn(logicalname, physicalname);
                 if (physicalname.length()) {
                     genString pset_file;
                     pset_file = get_paraset_file_name(physicalname, pn);
                     if (pset_file.str() && (OSapi_access(pset_file, F_OK) == -1)) {
                         remove = 1;
                     }
                 }
                 else {
                     remove = 1;
                     msg("Failed to convert logical name to physical name for file: $1", normal_sev) << logicalname.str() << eom; 
                 }
                 if (remove) {
                     msg("Removing module $1.", normal_sev) << logicalname.str() << eom;
                     xref->remove_module(logicalname);
                     if (remlst) {
                         genString txt;
                         txt.printf("\"%s\" \"%s\"\n", proj_name, logicalname.str());
                         OSapi_fwrite((char*)txt, txt.length(), 1, remlst);
                     }
                     num_removed++;
                 }
             }
         }
         if (remlst) OSapi_fclose(remlst);
    }

    if (succeeded.size() == 0 && num_removed == 0) {
	disbuild_exit (mbERROR, "no modules updated/removed for project", proj_name);
	return -1;
    }
    disbuild_update_end ();
    disbuild_exit (mbQUIT, "succeeded for project", proj_name);
    return 0;
}
