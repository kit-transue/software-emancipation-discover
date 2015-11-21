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
#include <stdio.h>
#else /* ISO_CPP_HEADERS */
#include <cstdio>
namespace std {};
using namespace std;
#endif /* ISO_CPP_HEADERS */
#include <proj.h>
#include <genString.h>
#include <machdep.h>
#include <cLibraryFunctions.h>
#include <msg.h>
#include <path.h>
#include <symbolPtr.h>
#include <fileCache.h>
#include <fileEntry.h>
#include <proj_save.h>
#include <genTry.h>
#include <systemMessages.h>
#include <evalMetaChars.h>
#ifndef ISO_CPP_HEADERS
#include <ctype.h>
#else /* ISO_CPP_HEADERS */
#include <cctype>
#endif /* ISO_CPP_HEADERS */
#include <disbuild.h>
#include <disbuild_cache.h>
#include <disbuild_analysis.h>
#include <disbuild_update.h>
#ifndef ISO_CPP_HEADERS
#include <errno.h>
#else /* ISO_CPP_HEADERS */
#include <cerrno>
#endif /* ISO_CPP_HEADERS */
#include <cmd.h>
#include <cmd_enums.h>
#include <Interpreter.h>
#include <driver_mode.h>

static genString model_admindir;
static genString model_host;

extern void proj_restore_validate_result (const char *pname, int rslt, const char *err_msg);
extern void proj_restore_validate_info  (const char *pname, int mods, int prjs, int lnum);

extern projModule *proj_restore_module (projNode *prnt, projLine& pline);
extern projNode *projNode_make_project (char const *nm, projNode *prnt, char const *, 
		projMap*, char const *ln, const char *sym_nm, const char *phys_name);
extern projNode *projNode_make_project (char const *nm, projNode *prnt, char const *, projMap*, char const *ln, const char *sym_nm, const char *phys_name, int pmod_type);
extern int projNode_set_pmod_names(projNode *);

const char *proj_get_current_save();
const char *get_proj_restore_error_text (int er);
extern "C" void driver_exit(int);
extern char* create_path_2DIS_no_realpath( char * path, char** dis_path );

static int force_noprj;
static const char *NULL_NAME = "NULL";

// never report more than once
static void report_noprj ()
{
    static int noprj = 0;
    if (noprj) {
	noprj = 1;
	msg("DIS_NOPRJ is set; skipping the .prj files.", warning_sev) << eom;
    }
}

// if set, than does not remove temp files and prints statistics at the end.
int get_Project_DebugLevel () 
{
    static int level = -1;
    if (level == -1) {
	level = customize::getIntPref("DIS_internal.PRJ_DebugLevel");
	if (level < 0) level = 0;
    }
    return level;
}


char const *noprj_flag          = OSapi_getenv ("DIS_NOPRJ");
static char const *prj_writable = OSapi_getenv ("DIS_PRJ_WRITABLE");

#define PRJ_LINE 2048

class prjParser 
{
  public:
    int empty_lines;
    int total_modules;
    int total_projects;
    genString err_msg;

    int linenum;
    char lbuf[PRJ_LINE];

    projLine pline;
    int level;
    
    prjParser();
    int extract_flags (char *buf_ptr, PRJ_field);
    int get_next (char *from, char *& st, char *& en);
    int parse_line ();
    void reset_line();
};

// Returns: 1 on success
//          0 on failure
int prjParser::parse_line ()
{
    Initialize(prjParser::parse_line);
    if (lbuf[0] == 0)
	return 0;

    linenum ++;

    char *st = 0, *en;
    int ret = 0;

    en = lbuf+ strlen(lbuf) -1;
    if ( *en == '\n' )
       *en = 0;

    // Title: [P X N] or [m s h i], indent is always spaces
    for (int ii = 0 ; lbuf[ii] ; ii ++) {
	if (!isspace(lbuf[ii])) {
	    st = &lbuf[ii];
	    en = st + 1;
	    break;
	}
    }

    if (!st || !*en) return 0;

    int lvl_indent = st - lbuf;
    level = lvl_indent / LVL_SPACES;

    if (st[0] == 'M' || st[0] == 'm' || st[0] == 'i' || st[0] == 'h' || st[0] == 's') {
	pline.is_proj = 0;
	pline.included = (st[0] == 'i') ? 1 : 0;
    } else if (st[0] == 'P' || st[0] == 'X' || st[0] == 'N') {
	pline.is_proj = 1;
	pline.pmod_type = PMOD_UNKNOWN;
	if (st[0] == 'X')
	    pline.pmod_type = PMOD_PDF;
	else if (st[0] == 'N')
	    pline.pmod_type = PMOD_NO;
	pline.sep = st[1];
    } else {
	err_msg = "Neither Project nor Module.";
	return 0;
    }

    // language-or-size
    ret = get_next (en, st, en);
    if (!ret) return 0;

    if (en) en[0] = 0;
    int numb = OSapi_atoi(st);
    if (en) en[0] = pline.sep;

    if (pline.is_proj)
	pline.proj_size = numb;
    else
	pline.lang = (enum fileLanguage)numb;

    // left name
    ret = get_next (en, st, en);
    if (!ret) return 0;

    if (en) en[0] = 0;
    pline.left_name = dequote_str( st );
    if (en) en[0] = pline.sep;
    
    // physical name
    ret = get_next (en, st, en);
    if (!ret) return 0;

    if (en) en[0] = 0;
    ret = extract_flags( (char*)dequote_str(st), PRJ_phys);
    if (en) en[0] = pline.sep;
    if (!ret) {
	err_msg = "Wrong physical name syntax.";
	return 0;
    }

    // Right name (meta name)
    ret = get_next (en, st, en);
    if (!ret) return 0;

    if (en) en[0] = 0;
    pline.right_name = dequote_str( st );
    if (en) en[0] = pline.sep;
    
    // Model name
    ret = get_next (en, st, en);
    if (!ret) return 0;

    if (en) en[0] = 0;
    ret = extract_flags( (char*)dequote_str(st), PRJ_model);
    if (en) en[0] = pline.sep;
    if (!ret) {
	err_msg = "Wrong model name syntax.";
	return 0;
    }
    return 1;
}

void project_path_sys_vars_translate(char const *m, genString &ret_path);

// returns 1 on success
//         0 on failure
int prjParser::extract_flags (char *buf_ptr, PRJ_field fld)
{
    Initialize(prjParser::extract_flags);
    int ret = 0;
    if (!buf_ptr || !buf_ptr[0])
	return ret;

    char *flags   = (fld == PRJ_phys) ? pline.phys_flags : pline.model_flags;
    char *del_str = strstr(buf_ptr, prjDELEMETER);

    if (!del_str) {
	// All flags and file names are supposed to be zero at this point
	// and does not need to be cleaned
	if (OSapi_strncmp (buf_ptr, NULL_NAME, OSapi_strlen(NULL_NAME)) == 0) {
	    ret = 1;
	    if (fld == PRJ_model)
		pline.model_name = NULL_NAME;
	}
    } else {
	del_str[0] = 0;
	{
	    genString tmp; char *buf_tmp = 0;
	    project_path_sys_vars_translate( buf_ptr, tmp);
	    buf_tmp = (char *)tmp;

	    genString& trg = (fld == PRJ_phys) ? pline.phys_name : pline.model_name;

	    char *cvt_ptr = 0;
	    trg = create_path_2DIS_no_realpath( buf_tmp, &cvt_ptr );
	    if (cvt_ptr) free (cvt_ptr);
	}
	del_str[0] = prjDELEMETER[0];

	del_str = del_str + OSapi_strlen(prjDELEMETER);
	for (int i = 0 ; i < PRJ_Flag_Last && del_str[i] ; i++)
	    flags[i] = del_str[i];

	flags[PRJ_Flag_Last] = 0;
	ret = 1;
    }
    return ret;
}

// returns 1 on success
//         0 on failure
int prjParser::get_next (char *buf, char *& st, char *& en)
{
    Initialize(prjParser::get_next);
    int ret = 0;
    st = en = 0;

    if (!buf || !buf[0])
	return ret;

    int ii;
    for (ii = 0 ; buf[ii] && buf[ii] == pline.sep; ii++);

    st = &buf[ii];
    if ( *st )
    {
      ret = 1;
      en = str_find_char( st, pline.sep );
    }
    else
      err_msg = "Not enough parameters.";
    
    return ret;
}

prjParser::prjParser () : empty_lines(0), total_modules(0), total_projects(0), linenum(-1),
                          level (0)
{
    reset_line();
}

void prjParser::reset_line ()
{
    if (linenum != 0) {
	if (linenum == -1)
	    linenum = 0;
	
	lbuf[0] = 0;
	err_msg = 0;
	pline.reset();
    }
}

void projLine::reset ()
{
    is_proj = 0; proj_size = 0;
    lang = FILE_LANGUAGE_UNKNOWN;
    left_name  = 0;
    phys_name  = 0;
    right_name = 0;
    model_name = 0;

    for (int i = 0 ; i <= PRJ_Flag_Last ; i++) {
	phys_flags[i] = 0;
	model_flags[i] = 0;    
    }
}

int projLine::is_flag_ok (PRJ_field fld, PRJ_flag flg)
{
    int ret = 0;
    if (flg < PRJ_Flag_Last && fld < PRJ_Field_Last) {
	char *flags = (fld == PRJ_phys) ? phys_flags : model_flags;
	ret = flags[flg] == '1';
    }
    return ret;
}

// Returns: 1 if the project has real pmod file
//          0 if the project has unreal pmod or does not have pmod
int proj_has_real_xref (projNode *pn)
{
    Initialize(proj_is_real_xref__projNode*);
    int ret = 0;
    if (pn) {
	Xref *xr = pn->get_xref (1);
	if (xr && !xr->is_unreal_file())
	    ret = 1;
    }
    return ret;
}


// returns:   0 - lbuf does not start with PROJ_ADMINDIR_TAG
//            1 - model_admindir is already set
//            2 - succeeded to set model_admindir
int proj_set_model_admindir (const char *ln)
{
    static int size = strlen(PROJ_ADMINDIR_TAG);
    int res = (strncmp(PROJ_ADMINDIR_TAG, ln, size) == 0) ? 1 : 0;
    
    if (res && ! model_admindir.length()) {
	model_admindir = ln + strlen(PROJ_ADMINDIR_TAG);
	model_admindir.trim();
	res = 2;
    }
    return res;
}


// returns:   0 - lbuf does not start with PROJ_HOST_TAG
//            1 - model_host is already set
//            2 - succeeded to set model_host
int proj_set_model_host (const char *ln)
{
    static int size = strlen(PROJ_HOST_TAG);
    int res = (strncmp(PROJ_HOST_TAG, ln, size) == 0) ? 1 : 0;
    
    if (res && ! model_host.length()) {
	model_host = ln + strlen(PROJ_HOST_TAG);
	model_host.trim();
	res = 2;
    }
    return res;
}


static char *next_line_skip_comments (char *lbuf, int size, FILE *f)
{
    char *ln = 0;
    while ( (ln = OSapi_fgets(lbuf, size, f)) ) {
	if (ln[0] == '#') {
	    if ( ! proj_set_model_admindir(lbuf) )
		(void) proj_set_model_host(lbuf);
	} else
	    break;
    }
    return ln;
}


static void check_build_info () 
{
    Initialize(check_build_info);
    static int first_init = -1;
    if (first_init > 0) return;

    first_init = 1;

    if (model_admindir.length() && model_host.length()) return;

    char const *adir = OSapi_getenv ("ADMINDIR");
    if (!adir || !adir[0]) return;

    genString binfo;
    binfo.printf ("%s/tmp/disbuild/build_info", adir);
    if (OSapi_access ((char *)binfo, R_OK)) return;

    FILE *fh = OSapi_fopen ((char *)binfo, "r");
    if (!fh) return;

    char lbuf[PRJ_LINE];
    (void) next_line_skip_comments (lbuf, PRJ_LINE, fh);
    if (model_admindir.length() && model_host.length()) {
	msg("Set build info from $1 : #HOST=$2  #ADMINDIR=$3", normal_sev)
		<< (char *)binfo << eoarg
		<< (char *)model_host << eoarg
		<< (char *)model_admindir << eom;
    }
    OSapi_fclose(fh);
}


// returns 1 on success. 
//         0 on failure.
int proj_restore_loop (projNode *prnt, FILE *prj_file, prjParser& lnp)
{
    Initialize(proj_restore_loop__projNode*__FILE*__prjParser);
    if (!prnt)
	return 0;

    char *ln = NULL;
    int ret = 1;

    projNode *cur_prnt = prnt;
    int cur_level = 1;

    while (ret && (ln = next_line_skip_comments(lnp.lbuf, PRJ_LINE, prj_file))) {
	if (lnp.lbuf[0] == 0 || lnp.lbuf[0] == '#') {
	    lnp.linenum ++; lnp.empty_lines ++;
	    continue;
	}

	if ( (ret = lnp.parse_line ()) ) {
	    while (lnp.level >= 1 && (lnp.level < cur_level) && cur_prnt) {
		cur_level --;
                cur_prnt = cur_prnt->find_parent ();
	    }
							    
	    if (cur_prnt == NULL || lnp.level != cur_level) {
		ret = 0; lnp.err_msg = "Wrong indentation level.";
		break;
	    }

	    if (lnp.pline.is_proj) {
		const char *sym_name = (lnp.pline.model_name.length()) ? (const char *)lnp.pline.model_name : 0;
		projNode *new_prnt = projNode_make_project ((char *)NULL, cur_prnt, (char *)NULL, 
		    (projMap *)NULL, (char *)lnp.pline.left_name, sym_name, lnp.pline.phys_name, lnp.pline.pmod_type);
		if (new_prnt) {
		    lnp.total_projects ++;
		    // set 'visited' for not leaf PMOD projects
		    if (lnp.pline.proj_size != -1) {
			new_prnt->set_visited();
			cur_level ++;
			cur_prnt = new_prnt;
		    }
		} else {
		    ret = 0;
		    lnp.err_msg = "Failed to create projNode.";
		}
	    } else {
		projModule *new_mod = proj_restore_module (cur_prnt, lnp.pline);
		if (new_mod)
		    lnp.total_modules ++;
		else {
		    ret = 0; lnp.err_msg = "Failed to create projModule.";
		}
	    }
	    if (ret) lnp.reset_line(); 
	}
    }

    return ret;
}

extern void proj_get_modules(projNode *proj, objArr& modules, int depth); 

void proj_cleanup (projNode *prnt)
{
    Initialize(proj_cleanup);
    projContents *cont = (prnt) ? prnt->get_contents_node() : NULL;
    if (!cont)
	return;

    objArr mods;
    proj_get_modules (prnt, mods, -1);
    {
	Obj *cur2 = 0;
	ForEach(cur2, mods) {
	    projModule *mod = checked_cast(projModule,cur2);
	    obj_unload(mod);
	}
    }

    Obj *pn_set = proj_get_subs(cont);
    objSet prjs = pn_set;

    {
	Obj *cur1 = 0;
	ForEach(cur1,prjs) {
	    projNode *pn = checked_cast(projNode,cur1);
	    pn->delete_projnode();
	}
    }
}

const char *get_proj_restore_error_text (int er)
{
    Initialize(get_proj_restore_error_text__int);
    const char *ans = NULL;
    if (er == 1)
	ans = TXT("The prj file is OK!");
    else if ( er == 0)
	ans = TXT("The prj file does not exist.");
    else if ( er == -1)
	ans = TXT("The prj file cannot be read.");
    else if ( er == -2)
	ans = TXT("The sym file does not exist.");
    else if ( er == -3)
	ans = TXT("The prj file is outdated.");
    else if ( er == -4)
	ans = TXT("The real pmod size differs from the size saved in the prj file.");
    else if ( er == -5)
	ans = TXT("The first line of the prj file does not correspond to the project.");
    else
	ans = TXT("Unknown reason.");

    return ans;
}

// returns: 1 - the prj file is OK, prj_file is the PRJ filename 
//          0 - the prj file does not exist
//         -1 - the prj file cannot be read
//         -2 - the sym file does not exist
//         -3 - the prj file is outdated
//              sschmidt:  I have disabled -3, and return 0, because this is
//                         not a very interesting problem.
//         -4 - the real pmod size differs from the size saved in the prj file
//         -5 - the first line of the prj file does not correspond to the project
int proj_is_up_to_date (projNode *pn)
{
    Initialize(proj_is_up_to_date__projNode*);
    if (proj_get_env_disbuild ()) return 1;   // .bpc files are always upto date

    if (pn == 0) return -2;
    char const *pn_name = pn->get_name();
    (void) projNode_set_pmod_names (pn);
    const char *prj_file = pn->prj_name->get_name();
    if (!prj_file || !prj_file[0] || !pn_name || !pn_name[0])
	return 0;

    const char *sym_file = pn->prj_name->get_name();
    struct OStype_stat sym_buf, prj_buf;

    pn->prj_name->reset();
    if (pn->prj_name->stat (&prj_buf) != 0)
	return 0;
    
    if (pn->prj_name->is_rok() != 1)
	return -1;

    FILE *the_prj = OSapi_fopen ((char *)prj_file, "r");
    if (!the_prj) return -1;

    prjParser lnp;
    int ret = -5;
    char *txt = next_line_skip_comments(lnp.lbuf, PRJ_LINE, the_prj);
    if (txt && (lnp.parse_line() == 1) && lnp.pline.is_proj == 1) {
	if (strcmp((char *)lnp.pline.left_name, pn_name) == 0) {
	    pn->sym_name->reset();
	    if (pn->sym_name->stat (&sym_buf) != 0)
		ret = -2;
	    else
		ret = (lnp.pline.proj_size == sym_buf.st_size) ? 1 : -4;
	}
    }


    OSapi_fclose(the_prj);
    return ret;
}

// assumes that the project was not restored before
// returns 1 on success.
// returns 0 on failure.
int proj_restore (projNode *pn)
{
    Initialize(proj_restore__projNode*);
    int ret = 0;
    if (!pn || pn == projNode::get_control_project()) return ret;

    check_build_info ();

    // we are in save -or- do not use PRJ
    char const *pn_name = pn->get_name();
    (void) projNode_set_pmod_names (pn);
    genString prj_file = pn->prj_name->get_name();
    if (!prj_file.length() || !pn_name || !pn_name[0])
	return ret;

    if (disbuild_analysis_mode() || is_disbuild_update()) {
	if (pn->find_parent()) return ret;    // From DISCOVER 6.1 PRJ files only on top
	    
	int res = disbuild_prj2bpc(prj_file);
	if (res != 0) {
	    msg("disbuild: wrong .bpc filename for $1", error_sev)
		<< pn_name << eom;
	    driver_exit (13);
	    return ret;
	} else {
	    FILE *ff = OSapi_fopen ((char *)prj_file, "r");
	    if (ff)
		OSapi_fclose(ff);
	    else {
		msg("disbuild: proj $1: cannot read .bpc file $2", normal_sev)
			<< pn_name << eoarg
			<< (char *)prj_file << eom;
		return ret;
	    }
	}
    }
    
    if (proj_get_current_save())
	return ret;

    int env_noprj = (noprj_flag != 0) ? 1 : 0;
    if (env_noprj || force_noprj) {
	proj_restore_validate_info (pn_name, 0, 0, 0);
	proj_restore_validate_result (pn_name, 0, "Force NOPRJ");
	report_noprj();
	return ret;
    }

    if (cmd_available_p(cmdPrjWritableProjects)) {
	// boris: 980518, bug15000. For interactive session only: skip PRJ 
	// for writable projects unless DIS_PRJ_WRITABLE is set.
	if (is_gui() && pn->is_writable() && !prj_writable) {
	    proj_restore_validate_info (pn_name, 0, 0, 0);
	    proj_restore_validate_result (pn_name, 0, "NOPRJ for writable");
	    return 0;
	}
    }

    int flag = proj_is_up_to_date (pn);
    const char *er = get_proj_restore_error_text (flag);

    if ( cmd_available_p(cmdPrjWritableProjects) || get_Project_DebugLevel() > 0) {
	if (flag == 0 || flag == -1 || flag == -5) {
	    if (get_Project_DebugLevel() || flag < 0)
		msg("Skip restoring project $1: $2", warning_sev)
			<< pn_name << eoarg
			<< er << eom;
	    proj_restore_validate_info (pn_name, 0, 0, 0);
	    proj_restore_validate_result (pn_name, 0, er);
	    return ret;
	}
    }

    FILE *the_prj = OSapi_fopen ((char *)prj_file, "r");
    if (!the_prj) {
	msg("Failed restoring project $1: can not open", warning_sev)
		<< pn_name << eom;
	proj_restore_validate_info (pn_name, 0, 0, 0);
	proj_restore_validate_result (pn_name, 0, "Failed to read PRJ file");
	return ret;
    }

    if ( get_Project_DebugLevel() || (flag < 0 && flag != -2) ) {
	msg("Issue restoring project $1: $2", warning_sev)
		<< pn_name << eoarg
		<< er << eom;
    }

    msg("Restoring project $1 ... ", normal_sev) << pn_name << eom;
    prjParser lnp;

    // read the very first line with top project description
    char *txt = next_line_skip_comments(lnp.lbuf, PRJ_LINE, the_prj);
    if (!txt)
	lnp.err_msg = "Could not read project description line.";
    else if ( !(ret = lnp.parse_line()) ) {
	if (lnp.err_msg.length() == 0) 
	    lnp.err_msg = "Could not parse project description line.";
    } else if (lnp.pline.is_proj == 0)
	lnp.err_msg = "Wrong project description line.";
    else if (strcmp((char *)lnp.pline.left_name, pn_name) != 0)
	lnp.err_msg.printf ("Different projects SYM: %s and PRJ: %s",pn_name, (char *)lnp.pline.left_name);
    else {
	lnp.level = 1;
	ret = proj_restore_loop (pn, the_prj, lnp);
    }

    OSapi_fclose(the_prj);

    if (ret == 1) {
	pn->set_visited();
	if (lnp.total_projects <= 1 && lnp.total_modules == 0)
	    msg("$1 is empty.", warning_sev) << (char *)prj_file << eom;
    } else {
	msg("Restore project failed, prj file $1", error_sev) << (char *)prj_file << eom;
	if (lnp.err_msg.length()) {
	    msg("Restore Error: $1 : Location: $2:$3", error_sev)
		<< (char *)lnp.err_msg << eoarg
		<< (char *)prj_file << eoarg
		<< lnp.linenum << eom;
	}
	proj_cleanup (pn);
    }

    if (lnp.empty_lines && (get_Project_DebugLevel() > 0) ) {
	msg("There are $1 empty lines in $2", warning_sev)
		<< lnp.empty_lines << eoarg
		<< (char *)prj_file << eom;
    }

    proj_restore_validate_info   (pn_name, lnp.total_modules, lnp.total_projects, lnp.linenum);
    proj_restore_validate_result (pn_name, ret, (char *)lnp.err_msg);
    return ret;
}

static int build_infoCmd(ClientData cd, Tcl_Interp *interp, int argc, char const **argv)
{
  Interpreter *i = (Interpreter*) cd;

  const char * sub   = argv[1];
  const char *answer = "";

  if(sub && strcmp(sub, "admindir") == 0) {
    answer = OSPATH( model_admindir.str() );
  } else if(sub && strcmp(sub, "host") == 0){
    answer = model_host.str();
  } else {
    ostream& os = i->GetOutputStream();
    os << "usage: build_info {admindir host}" << endl;
    return TCL_ERROR;
  }

  if (answer && answer[0])
    Tcl_SetResult(i->interp, (char*) answer, TCL_VOLATILE);
  return TCL_OK;
}

static int add_commands()
{
  new cliCommandInfo("build_info", build_infoCmd);

  return 0;
}
static int dummy = add_commands();
