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
#include "msg.h"
#include "proj.h"
#include "symbolArr.h"
#include "scopeMgr.h"
#include "genArr.h"
#include "customize.h"
#include "genError.h"
#include "genTmpfile.h"
#include "shell_calls.h"

#ifndef _WIN32
#include <sys/param.h>
#endif

#include "cLibraryFunctions.h"
#include "machdep.h"
#include "fileCache.h"
#include "db_intern.h"
#include "CMBase.h"
#include "attribute.h"

static char const *mrg_msg = "mrg pass: ";

class mrgGroupModules {
public:
    projModule *grp;
    objNameSet *modules;

    int contains (projModule *);
};


genArr(mrgGroupModules);
static genArrOf(mrgGroupModules) *mrg_arr;

static char line[MAXPATHLEN];
static char inst_sep = ':';
static char attr_sep = '#';

static objNameSet *module_set = NULL;
static symbolArr *needed_update_modules = NULL;

static void mrg_psetname (projModule *mod, genString& mrg_file, char const *ext)
{
    int res = 0;
    mrg_file = mod->paraset_file_name ();
    int len = mrg_file.length();
    if (mrg_file.length()) {
	char *str = (char *)mrg_file;
	// substitute .pset with .mrg (discover offset diff)
	char *pset = strrchr (str, 'p');
	if (pset && strcmp (pset, "pset") == 0) {
	    while (pset[0]) {
		pset[0] = ext[0];
		if (ext[0] == 0)
		    break;

		pset ++;
		ext  ++;
	    }
	    res = 1;
	}
    }
    if (res == 0)
	mrg_file = 0;
}

static void proj_mrg_filename (projModule *mod, genString& mrg_file)
{
    mrg_psetname(mod, mrg_file, "mrg");
}


// 0 if identical
// -1 if failed
// 1 if differ
static int proj_mrg_identical_files (char const *f1, char const *f2)
{
    Initialize(proj_mrg_identical_files);
    FILE *s1 = OSapi_fopen (f1, "r");
    if (s1 == 0) {
	msg("$1failed to read file $2", error_sev)
	    << mrg_msg << eoarg
	    << f1 << eom;
	return -1;
    }
    FILE *s2 = OSapi_fopen (f2, "r");
    if (s2 == 0) {
	msg("$1failed to read file $2", error_sev)
	    << mrg_msg << eoarg
	    << f2 << eom;
	return -1;
    }
    
    int res = 0;
    while (1) {
	char c1 = fgetc(s1);
	char c2 = fgetc(s2);
	if (c1 != c2) {
	    res = 1;
	    break;
	} else if (c1 == EOF)
	    break;
    }

    OSapi_fclose(s1);
    OSapi_fclose(s2);
    return res;
}

int projModule_retrieve_revision (projModule *mod, char const *ver, char const *fname);
void projModule_get_built_revision (projModule *mod, genString& ver);

// Copies into src_name the original source that last -proj pass built
// If there is no CM preference, or "no version" is stored in the PMOD,
// then the pset file source gets copied.
// Returns:  -1     - on failure to find source file
//            >=0   - succeeded
int projModule_retrieve_built_source (projModule *mod, char const *src_name)
{
    Initialize(projModule_retrieve_built_source);
    int res = -1;
    if (!mod || !src_name || !src_name[0])
	return res;

    if (customize::no_cm() == false) {
	genString ver;
	projModule_get_built_revision (mod, ver);
	if (ver.length()) {
	    msg("$1module $2; modelbuild version is $3. Retrieving ...",
		    normal_sev)
		    << mrg_msg << eoarg
		    << mod->get_name() << eoarg
		    << (char const *)ver << eom;
	    res = projModule_retrieve_revision (mod, ver, src_name);
	    if (res == -1) {
		msg("$1module $2; failed to retrieve version $3", error_sev)
		    << mrg_msg << eoarg
		    << mod->get_name() << eoarg
		    << (char const *)ver << eom;
	    } else {
		msg("$1module $2; retrieved version $3", normal_sev)
		    << mrg_msg << eoarg
		    << mod->get_name() << eoarg
		    << (char const *)ver << eom;
	    }
	}
    }
    
    if (res == -1) {
	genString pset_file = mod->paraset_file_name ();
	if (pset_file.length() == 0)
	    pset_file = "<null>";
	else
	    res = db_read_src_from_pset (pset_file, src_name);
	if (res == -1) {
	    msg("$1module $2; failed to extract source code from .pset $3",
		error_sev)
		<< mrg_msg << eoarg
		<< mod->get_name() << eoarg
		<< (char const *)pset_file << eom;
	} else {
	    msg("$1module $2; extracted .pset file source", normal_sev)
		<< mrg_msg << eoarg
		<< mod->get_name() << eom;
	}
    }
    return res;
}

// Runs diff between modelbuild and current versions of the source file
// returns:     0 - versions are identical
//              1 - versions differ
//                  Creates:     .src extracted source file from .pset
//                               .mrg file
//             -1 - failed to find sources to differ
//
static int proj_mrg_diff_module (projModule *mod)
{
    Initialize(proj_mrg_diff_module);
    int res = 0;
    char const *cur_ver = mod->get_phys_filename();
    char const *mod_nm  = mod->get_name ();
    char const *basename= strrchr (cur_ver, '/');
    char *tmp_dir       = strdup(OSapi_getenv("TMPDIR"));
    int tmp_len         = OSapi_strlen(tmp_dir);

    // remove all slashes at the end of the directory name
    for (int ii = tmp_len - 1; ii > 0 ; ii -- ) {
	if (tmp_dir[ii] != '/') {
	    break;
	}
	tmp_dir[ii] = 0;
    }
    
    // The temporary filename of the old version must have the same basename
    genString old_ver;
    old_ver.printf ("%s%s", tmp_dir, basename);
    free(tmp_dir);

    OSapi_unlink((char const *)old_ver);
    int res1 = projModule_retrieve_built_source (mod, (char const *)old_ver);
    if (res1 == -1) {
	msg("$1no built source version for $2", error_sev)
	    << mrg_msg << eoarg
	    << mod_nm << eom;
	return res;
    }
    
    msg("$1module $2; diff current and modelbuild versions ...", normal_sev)
	<< mrg_msg << eoarg
	<< mod_nm << eom;

    res1 = proj_mrg_identical_files ((char const *)old_ver, cur_ver);
    if (res1 != 0) {
	msg("$1module $2; modified since modelbuild", normal_sev)
	    << mrg_msg << eoarg
	    << mod_nm << eom;

	genString mrg_file;
	proj_mrg_filename (mod, mrg_file);
	FILE *mrg_res = NULL;
	if (mrg_file.length()) 
	    mrg_res = OSapi_fopen ((char const *)mrg_file, "w");
	if (mrg_res) {
	    genTmpfile mrg_from("MRG_");
	    genString exe;
	    exe.printf ("%s/bin/mrg_diff", OSapi_getenv("PSETHOME"));
	    int status =  vsysteml_redirect (mrg_from.name(), exe, (char const *)old_ver, cur_ver, (char const *)NULL);
	    
	    FILE *mrg_tmp = NULL;
	    if (status != 0 || !(mrg_tmp = OSapi_fopen(mrg_from.name(), "r")) ) {
		msg("$1failed to create mrg file$2\n"
		    "old version: $3\n"
		    "cur version: $4", error_sev)
		    << mrg_msg << eoarg
		    << (char const *)mrg_file << eoarg
		    << (char const *)old_ver << eoarg
		    << cur_ver << eom;
	    } else {
		res = 1;
		genString ver;
		projModule_get_built_revision (mod, ver);
		if (ver.length() == 0)
		    ver = "(nil)";

		// comments
		OSapi_fprintf (mrg_res, "# mrg_diff %s %s\n", (char const *)old_ver, (char const *)cur_ver);
		OSapi_fprintf (mrg_res, "# module    : %s\n", (char const *)mod->get_name());
		OSapi_fprintf (mrg_res, "# phys    fn: %s\n", cur_ver);
		OSapi_fprintf (mrg_res, "# pset    fn: %s\n", (char const *)mod->paraset_file_name());
		OSapi_fprintf (mrg_res, "# \n"); 
		OSapi_fprintf (mrg_res, "# model   fn: %s\n", (char const *)old_ver);
		OSapi_fprintf (mrg_res, "# model  rev: %s\n", (char const *)ver);
		OSapi_fprintf (mrg_res, "# \n");
		char line[MAXPATHLEN];
		while (OSapi_fgets(line, MAXPATHLEN, mrg_tmp))
		    OSapi_fputs (line, mrg_res);

		OSapi_fclose(mrg_res);
		OSapi_fclose(mrg_tmp);

		genString src_file;
		mrg_psetname (mod, src_file, "src");
		status = shell_cp ((char const *)old_ver, (char const *)src_file);
		if (status == 0)
		    msg("$1module $2; created mrg and src files", normal_sev)
			<< mrg_msg << eoarg
			<< mod_nm << eom;
		else
		    msg("$1module $2; failed to create src file", error_sev)
			<< mrg_msg << eoarg
			<< mod_nm << eom;
	    }
	}
    } else {
	msg("$1module $2 identical sources. Skipping.", normal_sev)
	    << mrg_msg << eoarg
	    << mod_nm << eom;
    }
    OSapi_unlink((char const *)old_ver);
    return res;
}


// Returns:   -1 - failed
//             0 - writable .mrg file is not needed (sources identical)
//             1 - writable and diffed
//                 Creates:    .src extracted source file from .pset
//                             .mrg file
//
int mrg_projModule_checkout (projModule *mod)
{
    Initialize(mrg_projModule_checkout);
    // Let's get i if the project is readonly
    int status = 0;
    symbolPtr sym = mod;

    AutoCO("Checked out by DIScover", ATT_physical_filename(sym));

// returns:     0 - versions are identical
//              1 - versions differ
//             -1 - failed to find sources to differ
    status = proj_mrg_diff_module (mod);
    return status;
}


// Returns:   -1 - failed
//             0 - writable .mrg file is not needed (sources identical)
//             1 - writable and merged, created .mrg file
//                 Creates  .src extracted source file from .pset
//                          .mrg file
//
int mrg_projModule_diff (projModule *mod)
{
    Initialize(mrg_projModule_diff);

// returns:     0 - versions are identical
//              1 - versions differ
//             -1 - failed to find sources to differ
    int status = proj_mrg_diff_module (mod);
    return status;
}
