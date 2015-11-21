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
#include <iomanip.h>
#else /* ISO_CPP_HEADERS */
#include <cstring>
using namespace std;
#include <cstdio>
#include <iomanip>
#endif /* ISO_CPP_HEADERS */
#include <proj.h>
#include <genString.h>
#include <shell_calls.h>
#include <objArr.h>
#include <genError.h>
#include <symbolSet.h>
#include <fileEntry.h>
#include <proj_save.h>
#include <genTry.h>
#include <disbuild.h>
#include <disbuild_analysis.h>
#include <disbuild_flags.h>
#include <timer.h>
#include <charstream.h>
#include <shell_calls.h>
#include <machdep.h>
#include <evalMetaChars.h>

extern projModule *find_module_by_impname (char const *imp_name);

#define PRS "prs"
#define MOD "mod"

// 1024 bytes read blocks
#define mbBLOCK_LEN 1024

static int disbuild_fgets (ocharstream& os, FILE* d)
{
    int        ret = 0;
    static int eof = 0;
    if (!d || eof) return ret;

    static char buf[mbBLOCK_LEN];
    static char *head = buf;
    static int   tail = 0;

    int eol = 0;
    while (!eol && !eof) {
	if ( tail ) {
	    char *st = head; char *en = head;
	    while ( tail ) {
		char c = en[0]; 
		tail -- ;
		en   ++;

		if (c == '\n') {
		    head = en; eol = 1;
		    break;
		}
	    }
	    while (st < en) {
		os << st[0];
		st ++ ;
		ret = 1;
	    }
	} else {
	    head = buf;
	    tail = OSapi_fread (buf, 1, mbBLOCK_LEN, d);
	    if (!tail) eof = 1 ;
	}
    }
    return ret;
}


int proj_has_real_xref (projNode *pn);
projNode *disbuild_analysis_get_pmod_project ( projModule *m )
{
    projNode *proj = m->get_project_internal ();
    for ( projNode *cur = proj ; cur  ; cur = proj->find_parent ()) {
	proj = cur;
	if (proj->pmod_type == PMOD_PDF || proj_has_real_xref(proj)) break;
    }

    return proj;
}

static int analysis_proj_num = 0;     // 0 - not parallel. Summarize immediatly
static int analysis_mode = 0;         // 0 - no, 1 - analysis, 2 - merge, 3 - summarize
static projNode *analysis_current_proj = 0;
static int scratch = 0;

void disbuild_analysis_set_project (projNode *pn) { analysis_current_proj = pn; }

int disbuild_analysis_mode ()
{
    static int checked = 0;
    if (checked == 0) {
	checked = 1;
	char const *var = proj_get_env_disbuild ();
	if (var && strncmp(var, "analysis", 8) == 0) {
	    analysis_mode = 1;
	    char const *buf = var + 8;
	    if (strncmp(buf,"_merge", 6) == 0)
		analysis_mode = 2;
	    else if (strncmp(buf,"_sum", 4) == 0)
		analysis_mode = 3;

	    char const *p = customize::getStrPref("MBscratch");
	    if (p && strcmp(p,"yes") == 0) scratch = 1;
	}
    }
    return analysis_mode;
}

// takes: proj_num == 0, means not parallel, global; do analysis, merge and sum in one shot
//        proj_num != 0, parallel; first do local analysis for each project
int disbuild_analysis_start (int proj_num)
{
    if (disbuild_analysis_mode()) {
	analysis_proj_num = proj_num;
	char const *tmp_dir = disbuild_get_tmpdir ();
	if (tmp_dir) {
	    msg("disbuild: tmpdir: $1", normal_sev) << tmp_dir << eom;
	    if (proj_num == 0)
		msg("disbuild MSG: ===== analysis start (global) =====", normal_sev) << eom;
	    else
		msg("disbuild MSG: ===== analysis start (local) =====", normal_sev) << eom;
	}
    }
    return analysis_mode;
}
int disbuild_analysis_complete()
{
    if (analysis_mode) {
	if (analysis_proj_num == 0) { 
	    msg("disbuild MSG: ===== analysis end (global) =====", normal_sev) << eom;
	} else
	    msg("disbuild MSG: ===== analysis end (local) =====", normal_sev) << eom;
    }

    return (analysis_mode > 0);
}

// returns: 0  - success, -1  - failure
// Converts: $sharedModelRoot/pmod/.../top.sub1.sub2.pmod.prj
//     into: $DISBUILD_TMPDIR/top.sub1.sub2.pmod.bpc
//           bpc stands for build project cache
int disbuild_prj2bpc (genString &nm)
{
    Initialize(disbuild_prj2bpc);
    int ret = -1;
    char *buf = (char *)nm;
    int len = nm.length();
    if (len > 4) {
	char *ext = buf + (len - 3);
	if (ext[0] == 'p' && ext[1] == 'r' && ext[2] == 'j') {
	    char *base   = strrchr (buf, '/');
	    if (!base)
		base = strrchr (buf, '\\');

	    if (base) {
		ext[0]  = 'b';   ext[1]  = 'p';   ext[2]  = 'c';
		genString tmp = (base + 1);
		nm.printf("%s%s", disbuild_get_tmpdir(), (char *)tmp);
		ret = 0;
	    }
	}
    }
    return ret;
}

// returns:  1 if no pmods upper and down
//
int disbuild_is_correct_pmod_project (projNode *pn)
{
    Initialize(disbuild_is_correct_pmod_project);
    int correct = 0;
    if (!pn) return correct;

    char const *cur_nm = pn->get_name();

    /* check for parents pmod */
    Xref *proj_xref = pn->get_xref ();
    if (proj_xref && !proj_xref->is_unreal_file() && !pn->get_xref(1)) {
	projNode *par = proj_xref->get_projNode();
	if (par) {
	    char const *nm = par->get_name();
	    msg("disbuild: parent $1 of project $2 has pmod;\n\tprojects and their subprojects cannot both have pmods.", error_sev) << nm << eoarg << cur_nm << eom;
	}
    } else if (Xref::test_subproj_pmod(pn) == -1) { 		/* check for sub project pmod */
	msg("disbuild: project $1 has one or more pmod subprojects.\n\tProjects and their subprojects cannot both have pmods.", error_sev) << cur_nm << eom;
    } else
	correct =1;
    return correct;
}


static symbolSet mods;
static void add_module (projModule *m) { if (m) mods.insert (m); }
static void cleanup_mods () { mods.removeAll(); }


void disbuild_get_pmod_name (projNode *pn, char const *suffix, char const *ext, genString& f)
{
    Initialize(disbuild_get_pmod_name);
    f = 0;
    if (!pn) return;
    if (!suffix) suffix = "";
    if (!ext)    ext    = "";

    genString nm     = pn->get_name();
    genString sym_nm = pn->sym_name->get_name();

    char *base   = strrchr ((char *)sym_nm, '/');
    if (!base)
	base = strrchr ((char *)sym_nm, '\\');

    if (!base || strlen(base) < 2) return;

    base ++ ; base[0] = 0;
    base = (char *)nm;
    for (int ii = 0 ; base[ii] ; ii ++) {
	if (base[ii] == '/' || base[ii] == '\\')
	    base[ii] = '.';
    }

    f.printf ("%s%s%s.pmod.%s", OSPATH((char const *)sym_nm), (char const *)nm, (char const *)suffix, (char const *)ext);
}

int disbuild_flush_modules (symbolSet& sym_set, genString& f)
{
    Initialize(disbuild_flush_modules);
    int res = -1;
    if (!f.length()) return res;

    res = 0;
    int sz = sym_set.size();
    if (sz == 0) return res;

    FILE *ff = disbuild_open_model_file ((char const *)f, "w");
    
    if (ff == 0)
	res = -1;
    else {
	symbolPtr cur;
	ForEachT(cur,sym_set){
	    projModule *m = checked_cast(projModule,cur);
	    genString txt;
	    proj_get_text_to_save (m, 0, txt);
	    res = OSapi_fwrite ((char const *)txt, txt.length(), 1, ff);
	    if (res == 1)
		res = 0;
	    else {
		msg("disbuild: failed to write to file $1", error_sev) << (char const *)f << eom;
		res = -1;
		break;
	    }
	}
	OSapi_fclose (ff);
    }
    return res;
}

int disbuild_analysis_flush ()
{
    Initialize(disbuild_analysis_flush);
    int res = -1;
    if (analysis_current_proj) {
	genString loc_file;
	disbuild_get_pmod_name (analysis_current_proj, "_outdated", MOD, loc_file);
	res = disbuild_flush_modules (mods, loc_file);
	if (res == 0)
	    msg("Saved $1.", normal_sev) << loc_file.str() << eom;
	else
	    msg("Failed to save $1.", error_sev) << loc_file.str() << eom;
    } else
	res = disbuild_analysis_summarize_flush ();

    cleanup_mods ();
    return res;
}

static void complete_msg_report (char const *nm, int prev_sz, int real_dependency)
{
    Initialize(complete_msg_report);
    if (!nm || !nm[0]) return;
    int cur_sz = mods.size();
    int diff   = cur_sz - prev_sz;
    msg("Completed project $1.", normal_sev) << nm << eom;

    if (diff < 0)
	msg("Failed to analyze project $1.", error_sev) << nm << eom;
    else if (diff > 0) {
	if (scratch)
	    msg("Build from scratch all $1 files.", normal_sev) << diff << eom;
	else if (real_dependency)
	    msg("Detected $1 outdated files.", normal_sev) << diff << eom;
	else
	    msg("Added all $1 files.", normal_sev) << diff << eom;
    } else {
	if (real_dependency)
	    msg("No outdated modules in project $1.", normal_sev) << nm << eom;
	else
	    msg("Empty project $1.", warning_sev) << nm << eom; 
    }
}

int disbuild_analysis_proj (projNode *pn, int force_flush)
{
    Initialize(disbuild_analysis_proj__projNode*);
    int res = 0;
    char const *cur_nm = pn->get_name();
    if (pn->pmod_type != PMOD_PDF) {
	msg("disbuild: project $1 does not have pmod flag; skip analysis", warning_sev) << cur_nm << eom;
	return res;
    }
   
    timer::init(0, "Analysis pass", "");

    Xref *xr = pn->get_xref ( 13 );
    if (xr && xr->is_unreal_file()) xr = NULL;
    disbuild_analysis_set_project (pn);

    int prev_sz              = mods.size();
    int real_dependency      = 0;
    genError *error;
    genTry {
	msg("Started project $1", normal_sev) << cur_nm << eom;
	if (!scratch && xr) {
	    real_dependency = 1;
	    res = disbuild_analysis_dependency_proj (pn);
	} else if (disbuild_is_correct_pmod_project(pn)) {
	    // no pmod. Just add the project's modules to the outdated
            // module list. If force_flush, then flush the modules in the
	    // <projname>_outdated.pmod.mod
	    objArr modules;
	    proj_get_modules (pn, modules);
	    Obj *cur;
	    ForEach(cur, modules) {
		projModule *m = checked_cast(projModule,cur);
		add_module (m);
	    }
	}
	complete_msg_report (cur_nm, prev_sz, real_dependency);
	if (force_flush) res = disbuild_analysis_flush ();
    } genCatch(error) {
	msg("Failed to analyze project $1", error_sev) << cur_nm << eom;
	cleanup_mods ();
	res = -1;
    }
    genEndtry;

    timer::showtime(0, "Analysis pass", "");

    disbuild_analysis_set_project (0);
    return res;
}

int disbuild_analysis_proj (char const *pn_name, int force_flush)
{
    Initialize(disbuild_analysis_proj);

    int res = -1;
    projNode *pn = NULL;
    if (pn_name && pn_name[0]) {
	pn = projNode::find_project (pn_name);
	if (pn == 0) {
	    msg("disbuild: failed to find project $1", error_sev) << pn_name << eom;
	} else
	    res = disbuild_analysis_proj (pn, force_flush);
    }
    return res;
}

// add to outdated the modules from projects with no xref,
// run dependency on the projects, that have xref
int disbuild_analysis_pmod_projects ()
{
    Initialize(disbuild_analysis_pmod_projects);
    int res = 0;
    disbuild_analysis_start ( 0 );

    objArr& pmod_projs = disbuild_get_pmod_subprojects ();
    Obj *cur;
    ForEach(cur,pmod_projs) {
	int no_flush = 0;
	res = disbuild_analysis_proj (checked_cast(projNode,cur), no_flush);
	if (res != 0 && disbuild_stop_after_failure ())
	    break;
    }

    if (res == 0 || !disbuild_stop_after_failure ()) {
	res = disbuild_analysis_summarize_flush ();
	cleanup_mods ();
	res = disbuild_analysis_complete ();
    }
    return res;
}

static int add_line (FILE *f, char const *txt, char const *fname, int& cnt)
{
    Initialize(add_line);
    int res = -1;
    if (!f || !fname || !fname[0]) return res;

    res = 0;
    if (!txt || !txt[0]) return res;

    if ( (OSapi_fwrite (txt, strlen(txt), 1, f) != 1) ) {
	res = -1;
	msg("Failed to write $1", error_sev) << fname << eom;
    } else
	cnt ++ ;
    
    return res;
}

static int get_prs_txt (genString& prs_txt, char const *iff, projModule *m)
{
    Initialize(get_prs_txt);
    int res = -1;
    prs_txt = 0;
    genString lan; genString dir; genString flags;

    char const *phy = (m) ? m->get_phys_filename() : 0;
    if (!phy || !phy[0]) return res;

    disbuild_get_language (m, lan);
    if (lan.length() == 0) return res;

    disbuild_get_dir_name (m, dir);
    if (dir.length() == 0) return res;

    res = disbuild_get_flags (m, flags);
    if (res != 0) return res;

    prs_txt.printf ("\"%s\" \"%s\" \"%s\" \"%s\" %s\n", OSPATH(iff), OSPATH(phy), (char const *)lan, 
		    OSPATH((char const *)dir), (char const *)flags);

    return res;
}

#define mbMSGMAX 40

static int summarize_rule_modules (FILE *nif, char const *nif_file, int& nif_cnt)
{
    Initialize(summarize_rule_modules);
    int res = 0;
    char const *rul_file = get_rules_filename();
    struct OStype_stat buf;
    if (OSapi_stat((char const *)rul_file, &buf) != 0)
	return res;

    res = -1;
    FILE *ff = OSapi_fopen ((char const *)rul_file, "r");
    if (!ff) {
	msg("Could not read $1", error_sev) << rul_file << eom;
	return res;
    }
    res = 0;
    char line[MAXPATHLEN];
    int line_cnt = 0;
    char const *line_buf;
    // parse line: "link_name" "proj_name" "mod_name"\n
    //   nif line: "link_name" "iff_name"
    while ( res == 0 && (line_buf = OSapi_fgets (line, MAXPATHLEN, ff)) ) {
	line_cnt ++ ;
	char const *arr[3];

	res = disbuild_split_line (arr, line, 3);
	if (res <= 0)
	    res = -1;
	else if (res == 2 && strncmp("NULL", arr[1], 4) == 0)
	    res = 0;  // fake link; ignore
	else if (res != 3)
	    res = -1;
	else {
	    res = -1;
	    projNode *proj = disbuild_get_top_project (arr[1]);
	    if (proj) {
		projModule *mod = proj->find_module (arr[2]);
		if (mod) {
		    res = 0;
		    if (mods.includes(mod)) {   // add only outdated modules
			genString txt; genString iff;
			disbuild_get_iff_name (mod, iff);
			if (iff.length()) {
			    projNode *pmod = disbuild_analysis_get_pmod_project (mod);
			    projNode *root = pmod->root_project();

			    txt.printf ("\"%s\" \"%s\" \"%s\" \"%s\" \"%s\"\n", OSPATH((char const *)iff), arr[0], 
					 pmod->get_name(), mod->get_name(), root->get_name());
			    res = add_line (nif, (char const *)txt, nif_file, nif_cnt);
			} else
			    res = -1;
		    }
		}
	    }
	}

	if (res < 0) {
	    if (strlen(line) > mbMSGMAX) 
		strcpy (&(line[mbMSGMAX-5]), " ...");
	    
	    msg("incorrect rule$1\n\t$2:$3", error_sev) << line << eoarg << rul_file << eoarg << line_cnt << eom;
	}
    }
    return res;
}


class flagsItem : public namedObject {
    genString line;
public:

    flagsItem(char const *name, char const *flags) : namedObject(name), line(flags) { }
    char const *get_line() { return (char const *)line; }
};

class namedFlags : public nameHash {
public:
    virtual char const *name(const Object*)const;
    virtual bool isEqualObjects(const Object&, const Object&) const;
};


bool namedFlags::isEqualObjects(const Object& o1, const Object& o2) const
{
    flagsItem *f1 = (flagsItem *)&o1;
    flagsItem *f2 = (flagsItem *)&o2;

    char const *n1 = f1->get_name();
    char const *n2 = f2->get_name();

    bool res = (strcmp(n1,n2) == 0) ? 1 : 0;
    return res;
}

char const *namedFlags::name (const Object*o)const
{
    flagsItem *f = (flagsItem *)o;
    return (char const *)f->get_name();
}


static namedFlags reusedFlags;

static char const *get_reused_flags (char const *tag) 
{
    Initialize(get_reused_flags);
    char const *line  = 0;
    Object     *found = 0;
    int x             = 0;
    
    reusedFlags.find (tag, x, found);
    if (found) {
	flagsItem *f = (flagsItem *)found;
	line         = f->get_line ();
    }
    return line;
}

static FILE *new_reuse_d   = 0;
static FILE *expanded_d    = 0;
static int old_reuse_flags = 0;
static int new_reuse_flags = 0;
static int expanded_flags  = 0;
static genString new_f;
static genString old_f;
static genString exp_f;


static int expand_add_flags (char const *imp, char const *phy, char const *flags)
{
    Initialize(expand_add_flags);
    int ret = 0;
    if (!expanded_d || !imp || !imp[0] || !phy || !phy[0] || !flags || !flags[0])
	return ret;
    
    genString line; genString exp_flags;
    if ( eval_shell_metachars(flags, exp_flags) ) {
	ret = 1;
	line.printf ("\"%s\" \"%s\" %s", imp, phy, (char const *)exp_flags);
	OSapi_fputs ((char const *)line, expanded_d); 
	expanded_flags ++ ;
    }

    return ret;
}

char const *projNode__ln_to_fn_imp (char const *imp)
{
    Initialize(projNode_find_module_by_imp_name);
    char const *phy = 0;
    if (!imp || !imp[0]) return phy;
    projModule *m = find_module_by_impname (imp);
    if (m)
	phy = m->get_phys_filename();

    return phy;
}

static int validate_expand_flags (char const *imp_nm, char const *flags)
{
    Initialize(validate_expand_flags);
    int ret = 0;
    if (!imp_nm || !imp_nm[0] || !flags || !flags[0])
	return ret;

    char const *phy = projNode__ln_to_fn_imp(imp_nm);
    ret = expand_add_flags (imp_nm, phy, flags);

    return ret;
}

static void add_old_reuse_flags ( char const *buf )
{
    if (new_reuse_d && buf && buf[0]) {
	OSapi_fputs (buf, new_reuse_d); 
	old_reuse_flags ++ ;
    }
}

static void add_new_reuse_flags ( char const *buf )
{
    if (new_reuse_d && buf && buf[0]) {
	OSapi_fputs (buf, new_reuse_d); 
	new_reuse_flags ++ ;
    }
}

static void done_reuse_flags ()
{
    if (new_reuse_d) {
	OSapi_fclose (new_reuse_d); new_reuse_d = 0;
	msg("Collected flags for $1 files: $2 old, $3 new", normal_sev) << new_reuse_flags + old_reuse_flags << eoarg << old_reuse_flags << eoarg << new_reuse_flags << eom;

	if (expanded_d) { OSapi_fclose (expanded_d); expanded_d = 0; }
	
	if (new_reuse_flags) {
	    struct OStype_stat buf;
	    if (OSapi_stat((char const *)old_f, &buf) == 0) {
		if (OSapi_access((char const *)old_f, W_OK) == 0) {
		    genString bak_f; bak_f.printf("%s.bak",(char const *)old_f);
		    msg("Backing up old flags file $1", normal_sev) << (char const *)bak_f << eom;
		    if (shell_mv ((char const *)old_f, (char const *)bak_f) == 0) {
			msg("Install new flags file $1", normal_sev) << (char const *)old_f << eom;
			if (shell_cp ((char const *)new_f, (char const *)old_f) != 0) {
			    msg("Failed to install flags file $1", error_sev) << (char const *)new_f << eom;
			}
		    } else {
			msg("Failed to back up flags file $1", error_sev) << (char const *)old_f << eom;
		    }
		} else {
		    msg("Flags file $1 not writable, temporarily saved flags in $2", warning_sev) << (char const *)old_f << eoarg << (char const *)new_f << eom;
		}
	    } else {
		msg("Install new flags file $1", normal_sev) << (char const *)old_f << eom;
		if (shell_cp ((char const *)new_f, (char const *)old_f) != 0) {
		    msg("Failed to install flags file $1", normal_sev) << (char const *)new_f << eom;
		}
	    }
	}
    }
}

static int flags_reuse_is_set(void)
{
    static int reuse = -1;
    if (reuse == -1) {
        char const *val = customize::getStrPref("MBflagsReuse");
	if (!val || !val[0] || strcmp("no", val) == 0) {
	   reuse = 0;
	}
	else reuse = 1;
    }
    return reuse;
}

extern void chop_slashes (char *val, char& del);
static genString disbuild_config;
static int init_reuse_flags ()
{
    Initialize(init_reuse_flags);
    static int here = 0;
    if (!here) {
	here = 1;

	char del = '/';
	char *admdir = strdup(OSapi_getenv ("ADMINDIR"));
	chop_slashes (admdir, del);
	disbuild_config.printf ("%s%cconfig%c", admdir, del, del, del);
        free(admdir);
	if (disbuild_config.length()) {
	    int res = shell_mkdir (disbuild_config, 1);  // force all the way (-p)
	    if (res != 0) {
		msg("disbuild: failed to create config directory $1", error_sev) << (char const *)disbuild_config << eom ;
	    }
	}

	new_f.printf("%snew_reuse.flg", disbuild_get_tmpdir());
	new_reuse_d = OSapi_fopen ((char const *)new_f, "w");
	if ( !new_reuse_d ) msg("Failed to open new reuse flags file $1.", error_sev) << (char const *)new_f << eom;
	old_f.printf("%s/config/reuse.flg", OSapi_getenv("ADMINDIR"));

	exp_f.printf("%sexpanded.flg", disbuild_get_tmpdir());
	expanded_d = OSapi_fopen ((char const *)exp_f, "w");
	if ( !expanded_d ) msg("Failed to open expanded flags file $1", error_sev) << (char const *)exp_f << eom;

	int flreuse = flags_reuse_is_set();	
	if (flreuse == 0) {
	    msg("MBflagsReuse set to \'no\'. Recalculate compiler flags.", normal_sev) << eom;
	} else {
	    char const *val = customize::getStrPref("MBflagsReuse");
	    if (strcmp("yes",val) == 0) {
	        msg("MBflagsReuse set to \'yes\'. Reuse compiler flags from $1", normal_sev) << (char const *)old_f << eom;

	    } else {
	        old_f = (char const *)val;
		msg("MBflagsReuse set to file $1", normal_sev) << (char const *)old_f << eom;
	    }
	}
        /* Create the hash even if MBflagsReuse is "no", because it will be
           used to get flags of "up to date" files during incremental
           modelbuild. */
	FILE *d = 0;
	d = OSapi_fopen ((char const *)old_f, "r");
	int lines = 0;
	if (d) {
	    int lines = 0;
	    while ( 1 ) {
	        ocharstream line;
		int res = disbuild_fgets (line,d);
		if (res == 0)
		    break;
		line << ends;
		char *buf = line.ptr();
		if (buf && buf[0] && buf[0] == '\"') {
		    char *st = buf + 1;
		    char *en = strstr(st, "\" \"");
		    if (en) {
		        en[0] = 0;
			genString key = st;
			en[0] = '\"';
			
			char const *found = get_reused_flags (key);
			if (!found) {
			    char *flags  = en + 2;
			    if (validate_expand_flags (key, flags)) {
			        if (flreuse) add_old_reuse_flags (buf);
				flagsItem *o = new flagsItem (key, flags);
				reusedFlags.add(*o);
				lines ++;
			    }
			}
		    }
		}
	    }
	    if (flreuse) {
	        if (lines)
		    msg("Reusing flags for $1 files.", normal_sev) << lines << eom;
		else {
		    msg("MBflagsReuse set, but cannot read any flag lines.  Recalculate compiler flags.", warning_sev) << eom;
		}
	    }
	    OSapi_fclose(d);
	}
	else if (flreuse) {
	    msg("MBflagsReuse set, but cannot read file $1.  Recalculate compiler flags.", warning_sev) << (char const *)old_f << eom;
	}
    }
    return reusedFlags.size();
}


static int disbuild_analysis_reuse_flags(genString& prs_txt, projModule *m, char const *iff) 
{
    Initialize(disbuild_analysis_reuse_flags);
    int ret = 0; prs_txt = 0;
    if (iff && iff[0] && init_reuse_flags()) {
	char const *key   = m->get_implicit_name();
	char const *phy   = m->get_pr_name ();
	char const *flags = get_reused_flags(key);
	
	if (flags) {
	    prs_txt.printf("\"%s\" \"%s\" %s", OSPATH(iff), OSPATH(phy), flags);
	    ret = 1;
	}
    }
    return ret;
}


static void disbuild_analysis_add_reuse_flags (genString& prs_txt, projModule *m) 
{
    Initialize(disbuild_analysis_add_reuse_flags);
    if (!m || !prs_txt.length()) return;

    char const *key = m->get_implicit_name();
    char *txt = prs_txt;
    char *en = strstr(txt, "\" \"");
    if (en) {
      //skip second column - physical name. Go to language/directory/flags
      en = en + 2; en = strstr(en,  "\" \"");
    }
    
    if (en) {
	char *flags = en + 2;
	genString reuse;
	reuse.printf ("\"%s\" %s", key, flags);
	add_new_reuse_flags ((char const *)reuse);
	expand_add_flags (key, m->get_phys_filename(), flags);
    }
}


// format: "<iff name>"  "<phys name>"  "<pmod proj name>"  "<ln name>"
static void get_nif_txt (genString& nif_txt, genString& iff, projNode *pmod, projModule *m)
{
    Initialize(get_nif_txt);
    char const *phy = m->get_phys_filename();
    disbuild_get_iff_name (m, iff);
    projNode *root = pmod->root_project();
    nif_txt.printf ("\"%s\" \"%s\" \"%s\" \"%s\" \"%s\"\n", OSPATH((char const *)iff), OSPATH(phy), pmod->get_name(), m->get_name(), root->get_name());
}

int disbuild_analysis_summarize_flush ()
{
    Initialize(disbuild_analysis_summarize_flush);
    int res = -1;

    objArr& pmod_projs = disbuild_get_pmod_subprojects ();
    int sz = pmod_projs.size();
    if (sz == 0) {
	msg("No pmod projects.", error_sev) << eom;
	return res;
    } else {
	msg("Preparing build info for $1 pmod projects:", normal_sev) << sz << eom;
    }

    init_reuse_flags();

    genString lst_file; genString prs_file; genString nif_file;
    int lst_cnt, prs_cnt, nif_cnt, rul_cnt, hdr_cnt, src_cnt, non_cnt; 
    src_cnt = hdr_cnt = lst_cnt = prs_cnt = nif_cnt = rul_cnt = non_cnt = 0;
    lst_file.printf ("%s%s", (char const *)disbuild_get_tmpdir(), LST_F);
    prs_file.printf ("%s%s", (char const *)disbuild_get_tmpdir(), PRS_F);
    nif_file.printf ("%s%s", (char const *)disbuild_get_tmpdir(), NIF_F);

    FILE *lst = disbuild_open_model_file((char const *)lst_file,"w"); if (!lst) return res;
    FILE *prs = disbuild_open_model_file((char const *)prs_file,"w"); if (!prs) return res;
    FILE *nif = disbuild_open_model_file((char const *)nif_file,"w"); if (!nif) return res;
    
    genError *error; res = 0;
    genTry {
	Obj *cur;
	ForEach(cur,pmod_projs) {
	    int skip = 0;
	    projNode *pn = checked_cast(projNode,cur);
	    if (res == 0) {
		msg("Project $1", normal_sev) << pn->get_name() << eom;
		objArr modules;
		proj_get_modules (pn, modules);
		Obj *cur_m;
		ForEach (cur_m, modules) {
		    genString lst_txt;
		    projModule *m = checked_cast(projModule,cur_m);
		    (void) proj_get_text_to_save (m, 0, lst_txt);

		    if (lst_txt.length()) {
			char *str = (char *)lst_txt;
			genString prs_txt; genString nif_txt;
			genString iff;
			get_nif_txt (nif_txt, iff, pn, m);
		        if (mods.includes(m)) {
			    if (str[0] == 's' && skip == 0) {
				src_cnt ++ ;
                                /* If flagsReuse is set to no or if 
                                   disbuild_analysis_reuse_flags fails 
                                   calculate flags and add to file. */
				if (!flags_reuse_is_set() || !disbuild_analysis_reuse_flags(prs_txt, m, iff)) {
				    get_prs_txt (prs_txt, iff, m);
				    disbuild_analysis_add_reuse_flags (prs_txt, m);
				}
				skip = (OSapi_getenv("DISMB_TEST")) ? 1 : 0;  // one file per project
			    } else if ( str[0] == 'h' || str[0] == 'i') {
				hdr_cnt ++;
			    } else {
				non_cnt ++;
			    }
			    if ( (res = add_line (prs, prs_txt, prs_file, prs_cnt) != 0) ||
			         (res = add_line (nif, nif_txt, nif_file, nif_cnt) != 0) )
			        break;
			}
			else if (str[0] == 's' && !flags_reuse_is_set()) {
			    /* Add flags to "reuse.flg" (even if the file is
                               not outdated during incremental modelbuild) 
                               since Submission Check uses it to get parser 
                               flags of selected files. */
			    disbuild_analysis_reuse_flags(prs_txt, m, iff);
			    disbuild_analysis_add_reuse_flags (prs_txt, m);

			}
			/* Add all files to "all.pmod.lst" (even if the
                           files are up to date during incrmental modelbuild) 
                           since Submission Check uses this list. */
			if ( (res = add_line (lst, lst_txt, lst_file, lst_cnt) != 0) ) {
			  break;
			}
		    }
		}
	    } else
		break;
	}
	done_reuse_flags ();

	int prv_cnt = nif_cnt;
	if (res == 0 && (res = summarize_rule_modules (nif, nif_file, nif_cnt)) == 0) {
	    int map_cnt = nif_cnt - prv_cnt;
	    msg("=========== Analysis Summary ===========", normal_sev) << eom;
	    msg("all PDF  modules: $1    $2", normal_sev) << setw(6) << lst_cnt << eoarg << LST_F << eoarg << eom;
	    msg("outdated modules: $1    $2", normal_sev) << setw(6) << nif_cnt << eoarg << NIF_F << eom;
	    msg("source files:     $1", normal_sev) << setw(6) << src_cnt << eom;
	    msg("parse  files:     $1    $2", normal_sev) << setw(6) << prs_cnt << eoarg << PRS_F << eom;
	    msg("header files:     $1", normal_sev) << setw(6) << hdr_cnt << eom;         
	    msg("other  files:     $1", normal_sev) << setw(6) << non_cnt << eom;         
	    msg("mapped files:     $1", normal_sev) << setw(6) << map_cnt << eom;
	    msg("to build modules: $1    $2", normal_sev) << setw(6) << nif_cnt << eoarg << NIF_F << eom;
	    msg("========================================", normal_sev) << eom;
	}
    } genCatch (error) {
	res = -1;
    }
    genEndtry;

    if (res != 0)
	msg("disbuild: failed summarizing analysis.", error_sev) << eom;

    OSapi_fclose (lst); OSapi_fclose (prs); OSapi_fclose (nif); 

    return res;
}

int disbuild_analysis_summarize ()
{
    int res = -1;
    return res;
}

int disbuild_analysis_merge ()
{
    int res = -1;
    return res;
}

int disbuild_analysis_dependency_proj (projNode *pn)
{
    Initialize(disbuild_analysis_dependency_proj);
    int res = -1;
    char const *nm = (pn) ? pn->get_name() : 0;
    if (!nm || !nm[0]) return res;

    pn->refresh_all();
    analysis_current_proj = pn;

    pn->update_xref();
    res = 0;
    return res;
}

void disbuild_analysis_outdated_module(projModule *m)
{
    Initialize(disbuild_analysis_outdated_module);
    add_module (m);
}

