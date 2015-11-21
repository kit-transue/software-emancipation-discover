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
#include <proj_save.h>

#include <cstdio>
#include <sstream>
namespace std {};
using namespace std;

#include <proj.h>
#include <genString.h>
#include <machdep.h>
#include <cLibraryFunctions.h>
#include <msg.h>
#include <path.h>
#include <symbolPtr.h>
#include <fileCache.h>
#include <fileEntry.h>
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
#ifndef ISO_CPP_HEADERS
#include <errno.h>
#else /* ISO_CPP_HEADERS */
#include <cerrno>
#endif /* ISO_CPP_HEADERS */
#include <cmd.h>
#include <cmd_enums.h>

#ifdef _WIN32
   #include <windows.h>
   #include <winsock.h>
   #include <io.h>
#else
   #include <sys/socket.h>
   #include <netinet/in.h>
   #include <netinet/tcp.h>
   #include <arpa/inet.h>
   #include <netdb.h>
#endif

static genString hostname;
static genString admindir;

static const char *get_hostname()
{
    static int init = 0;
    if ( init == 0 ) {
	init = 1; 
	char hname[MAXPATHLEN]; 
	hname[0] = 0;

	OSapi_gethostname (hname, MAXPATHLEN);

	if (hname[0]) {
	    char **p;
	    struct hostent *hp = gethostbyname(hname);
	    if ( hp && (p = hp->h_addr_list) && *p) {
		struct in_addr in;
		(void) memcpy(&in.s_addr, *p, sizeof (in.s_addr));
		hostname = inet_ntoa(in);
	    }
	} else
	    hostname = 0;
    }
    return (const char *)hostname;
}

static const char *get_admindir()
{
    static int init = 0;
    if ( init == 0 ) {
	init = 1; 
	char *aenv = (char *)OSapi_getenv ("ADMINDIR");
	char *p = 0;

#ifdef _WIN32 
	extern char* DisFN_create_path_2DIS( char * path, char** dis_path );
	DisFN_create_path_2DIS( aenv, &p );
	aenv = p;
#endif

	if (aenv && *aenv)
	    admindir = aenv;
	else
	    admindir = 0;

	if (p) OSapi_free(p);
    }
    return admindir;
}

static char *NULL_NAME = "NULL";

extern "C" void driver_exit(int);

int proj_save (objArr& proj_set);
int proj_save (projNode *, int save_root);
void proj_cleanup (projNode *prnt);
int proj_is_up_to_date (projNode *pn);
int proj_has_real_xref (projNode *pn);
int get_Project_DebugLevel ();
const char *get_proj_restore_error_text (int er);

// The project name that is being saved
static genString current_save;
const char *proj_get_current_save () { return (const char *)current_save; }
extern int projNode_set_pmod_names(projNode *);
void proj_set_indent (int lvl, genString& txt)
{
    const int buf_size = MAX_LVL*LVL_SPACES;
    static char lvl_indent[buf_size+1];
    static int init = 1;

    if(init) {
	init = 0;
	for (int i = 0; i<buf_size; i++) {
	    lvl_indent[i] = ' ';
	}
	lvl_indent[buf_size] = '\0';
    }

    if (lvl > MAX_LVL)
	lvl = MAX_LVL;
    if (lvl) {
	int n = lvl * LVL_SPACES;
	lvl_indent[n] = '\0';
	txt = lvl_indent;
	lvl_indent[n] = ' ';
    } else
	txt = "";
}

char proj_save_get_separator () 
{
    static char sep = 0;
    if (!sep) {
	sep = ' ';    // default separator is space
	char *pref_sep = customize::getStrPref("DIS_model.PRJ_Separator");
	char c;
	while (pref_sep && (c = *pref_sep) ) {
	    if (isspace(c))
		pref_sep ++;
	    else {
		char next_c = pref_sep[1];
		if (next_c == 0 || isspace(next_c)) {
		    sep = c;
		} else if ( (strncmp("SPACE", pref_sep, 5) == 0) ||
			    (strncmp("space", pref_sep, 5) == 0) )
		    sep = ' ';
		else if ( (strncmp("TAB", pref_sep, 3) == 0) ||
			  (strncmp("tab", pref_sep, 3) == 0) )
		    sep = '\t';
		else {
		    msg("Wrong preference DIS_model.PRJ_Separator: $1. Use default: SPACE", warning_sev) << pref_sep << eom; 
		}
		break;
	    }
	}
    }
    return sep;
}

// Returns: 0 on success;
//         -1 on failure;
int proj_get_text_to_save (projNode *prj, int lvl, genString& txt)
{
    int ret = -1;
    char const *left_name = prj->get_name();
    fileEntry *fe = NULL;
    char const *sym_str = NULL;
    struct OStype_stat *null_arg = NULL;

    int pmod_size = 0;
    {
	genString sym_name;
	if (prj->pmod_type == PMOD_PDF || proj_has_real_xref(prj) || !prj->find_parent()) {
	    prj->xref_file_name (sym_name);
	    sym_name += ".sym";
	}

	if (sym_name.length()) {
	    fe = global_cache.add_entry ((char *)sym_name);
	    if (fe) {
		ostringstream os;
		struct OStype_stat buf_stat;
		int res_stat = fe->stat (&buf_stat);
		fe->put_signature (os);
		os << ends;
		sym_str = strdup(os.str().c_str());
		if (res_stat == 0)
		    pmod_size = buf_stat.st_size;
	    }
	}
    }

    genString indent;
    proj_set_indent (lvl, indent);
    const char *ttl = "P"; // PMOD_UNKNOWN
    if (prj->is_visible_proj() == 0)
	ttl = "R";
    else if (prj->pmod_type == PMOD_NO) {
	ttl = "N";  // PMOD_NO
    } else if (prj->pmod_type != PMOD_UNKNOWN)
	ttl = "X";   // PMOD_PDF

    genString dir_name;
    genString meta_name;
    char *dir_str = NULL;

    prj->ln_to_fn_imp (left_name, dir_name, DIR_TYPE, 0, 1);
    if (prj->fake_name.length() == 0)
	prj->fn_to_ln(dir_name, prj->fake_name, DIR_TYPE, 0, 1);

    meta_name = prj->fake_name;

    fe = global_cache.add_entry((char *)dir_name);
    if (fe) {
	ostringstream nos;
	(void) fe->stat (null_arg);
	fe->put_signature (nos);
	nos << ends;
	dir_str = strdup(nos.str().c_str());
    }
    if (!sym_str || !*sym_str)
	sym_str = NULL_NAME;

    if (!dir_str || !*dir_str)
	dir_str = NULL_NAME;

    char sep = proj_save_get_separator();

    txt.printf ("%s%s%c%d%c%s%c%s%c%s%c%s\n", (char *)indent, ttl, sep, pmod_size,
                sep, (char*)quote_str_if_space(left_name),
                sep, (char*)quote_str_if_space(dir_str  ),
                sep, (char*)quote_str_if_space(meta_name),
                sep, (char*)quote_str_if_space(sym_str  ) );

    if (sym_str != NULL_NAME)
	delete sym_str;

    if (dir_str != NULL_NAME)
	delete dir_str;

    ret = 0;
    return ret;
}

int proj_get_text_to_save (projModule *mod, int lvl, genString& txt)
{
    genString indent;
    proj_set_indent (lvl, indent);

    ostringstream os;
    mod->save_signature (os);
    os << ends;
    char const *s = os.str().c_str();
    txt.printf ("%s%s\n", (char *)indent, s);
    return 0;
}

// Returns 0 on success
//        -1 on failure
//  proj_set - set of projects with pmods for future saving
int proj_save_recursive (projNode *prj, int lvl, FILE *prj_file, int *info, objArr& pmod_projs)
{
    Initialize(proj_save_recursive__projNode*_int_FILE*_int*_objArr&);
    genString cur_txt;
    int ret_path = 0;

    int res = proj_get_text_to_save (prj, lvl, cur_txt);
    if (res != 0)
	return -1;

    res = fwrite ((char *)cur_txt, cur_txt.length(), 1, prj_file);
    if (res != 1)
	return -1;

    if (is_disbuild_cache()) 
	disbuild_cache_project (prj, cur_txt);

    // boris: 971008, takes care about projects with scripts [[ ... ]]
    if (prj->visited() == 0) {
	if (prj->pmod_type == PMOD_PDF || proj_has_real_xref(prj)) {
	    pmod_projs.insert(prj);
	}

	if (prj->is_script())
	    prj->force_refresh( 1 );
	else
	    prj->refresh();
    }

    info[PRJ]++;
    lvl++;
    info[LVL] = (lvl > info[LVL]) ? lvl : info[LVL];

    symbolArr& proj_contents = prj->rtl_contents();
    symbolPtr sym;
    ForEachS(sym, proj_contents) {
        if (sym.get_kind() == DD_MODULE) {
	    projModule *mod = checked_cast(projModule,sym);
	    res = proj_get_text_to_save (mod, lvl, cur_txt);
	    if (res == 0) {
		res = fwrite ((char *)cur_txt, cur_txt.length(), 1, prj_file);

		if (is_disbuild_cache())
		    disbuild_cache_module (mod, cur_txt);

		if (res == 1)
		    info[MOD]++;
		else {
		    ret_path = -1;
		    break;
		}
	    }
	} else if (sym.get_kind() == DD_PROJECT) {
            appPtr app_head = sym.get_def_app();
            if(app_head && is_projHeader(app_head)) {
                projNode *nd = projNodePtr(app_head->get_root());
		ret_path     = proj_save_recursive (nd, lvl, prj_file, info, pmod_projs);
		if (ret_path != 0)
		   break;
	    }
	}
    }
    return ret_path;
}

static int prj_do_backups ()
{
    static int do_backs = -1;
    if (do_backs == -1) {
	do_backs = 0;
	char *val = customize::getStrPref("DIS_build.PRJ_Backups");
	if (val && val[0]) {
	    if (val[0] == '1' || strncmp (val, "yes", 3) == 0 || strncmp (val, "YES", 3) == 0)
		do_backs = 1;
	}
    }
    return do_backs;
}

static int store_build_info (FILE *file_prj, genString& to_file)
{
    int ret = 0;

    genString comments;
    const char *adir = get_admindir();
    const char *host = get_hostname();
    adir = (adir) ? adir : "";
    host = (host) ? host : "";
	
    comments.printf("%s%s\n%s%s\n", PROJ_ADMINDIR_TAG, (char *)adir, PROJ_HOST_TAG, (char *)host);
    int lll = fwrite ((char *)comments, comments.length(), 1, file_prj);
	
    if ( lll != 1 ) {
	msg("Failed to write comments into PRJ file $1\n\tComments:\n$2", error_sev) << (char *)to_file << eoarg << (char *)comments << eom;
	ret = -1;
    }

    return ret;
}


static int proj_save_project (projNode *prj, const char *filename, 
	objArr& pmod_projs)
{
    Initialize(proj_save_project__projNode*_char*_objArr&);
    if (!prj || !filename || !filename[0])
	return -1;
    
    int root_noxref = 0;
    char const *nm = prj->get_name();

    if (proj_has_real_xref (prj) == 0) {
	// Only a root project may not have the pmod file.
	// In this case we will save in the <root_name>.pmod.prj tree-list of
	// components, that do have the .pmod.sym files. In the case when the
	// .pmod files will not be found down the root project structure, than
	// the whole root project get stored in the file <root_name>.pmod.prj
	if (prj->find_parent()) {
	    msg("Save: Failed to save non-root project $1 without Xref.", catastrophe_sev) << nm << eom;
	    return -1;
	} else
	    root_noxref ++ ;
    }

    FILE *file_prj = NULL, *old_prj = NULL;
    genString to_file(filename);
    if (disbuild_analysis_mode() == 0) {
	old_prj = OSapi_fopen (filename, "r");
	// If file exists, do not overwrite it YET
	if (old_prj != 0) {
	    fclose (old_prj);
	    to_file += ".cur";
	}
    }

    if ( !(file_prj = OSapi_fopen ((char *)to_file, "w")) ) {
	msg("Failed to open file $1 for writing", error_sev) << (char *)to_file << eom;
	return -1;
    }

    msg("Saving ROOT $1", normal_sev) << nm << eom;

    if (store_build_info(file_prj, to_file) != 0) return -1;

    int info[3];
    info[LVL] = info[MOD] = info[PRJ] = 0;

    int ret_res = proj_save_recursive (prj, 0, file_prj, &info[0], pmod_projs);
    fclose (file_prj);

    if (ret_res != 0) {
	msg("ROOT save failed", error_sev) << eom;
	if (get_Project_DebugLevel() <= 0)
	    OSapi_unlink ((char *)to_file);
	else 
	    msg("ProjSAVE: did not unlink PRJ file $1", normal_sev) << (char *)to_file << eom;
    } else {
	msg("done", normal_sev) << eom;
	if (old_prj) {                // backup the existing file and overrite it
	    if (prj_do_backups()) {
		genString back_file (filename);
		back_file += ".bak";
		int res = OSapi_rename (filename, (char *)back_file);
		if (res != 0)
		    msg("Failed to backup existing file $1", error_sev) << filename << eom;
	    }

	    int res = OSapi_rename ((char *)to_file, filename);
	    if (res != 0) {
		msg("Failed to rename .cur file into $1", error_sev) << filename << eom;
		ret_res = errno;
		if (get_Project_DebugLevel() <= 0)
		    OSapi_unlink ((char *)to_file);
	    }
	}
    }

    if (get_Project_DebugLevel() > 0) {
	msg("DEBUG TOTAL INFO: levels $1, modules $2, projects $3", normal_sev) << info[LVL] << eoarg << info[MOD] << eoarg << info[PRJ] << eom;
    }
    return ret_res;
}	


static const char *get_Project_LogFile () 
{
    static const char* f = 0;
    if (f == 0) {
	f = customize::getStrPref("DIS_internal.PRJ_LogFile");
	if (!f) f = "";
    }
    return f;
}

// Returns: 0 on success
//         -1 on failure
int proj_save (projNode *prj, int save_root)
{
    Initialize(proj_save__projNode*);
    int ret = -1;
    if (!prj)
	return ret;

    char const *pr_name = prj->get_name();
    if (proj_has_real_xref (prj) == 0 && prj->find_parent()) {
	msg("Failed to save non-root project $1 without Xref", catastrophe_sev) << pr_name << eom;
	return ret;
    }

    (void) projNode_set_pmod_names(prj);
    genString to_save = get_Project_LogFile();
    if (to_save.length()) {
	msg("Using DIS_internal.PRJ_LogFile = $1", normal_sev) << (char *)to_save << eom;
    } else {
	to_save = prj->prj_name->get_name();
	if (is_disbuild_cache()) {
	    ret = disbuild_prj2bpc (to_save);
	    if (ret != 0) {
		msg("disbuild: wrong .bpc filename for $1", error_sev) << pr_name << eom;
		driver_exit (13);
		return ret;
	    }
	} else if (save_root) {
	    int flag = proj_is_up_to_date (prj);
	    if (flag == 1) {
		msg("SAVE: project $1 is up to date.", normal_sev) << pr_name << eom;
		return 0;
	    } else if (get_Project_DebugLevel() > 0) {
		const char *er = get_proj_restore_error_text (flag);
		msg("SAVE: project $1 is not up to date. $2", normal_sev) << pr_name << eoarg << er << eom;
	    }
	}
    }

    objArr pmod_projs;
    genError *err;
    genTry {
	current_save = pr_name;
	ret = proj_save_project (prj, (char *)to_save, pmod_projs);
    } genCatch(err) {
	ret = -1;
    }
    genEndtry;

    current_save = 0;
    if (ret != 0)
	msg("Save: Failed to save project $1", error_sev) << pr_name << eom;
    else {
	int projs_sz = pmod_projs.size();
	if (projs_sz == 0)
	    msg("There are no PMOD projects for top $1", warning_sev) << pr_name << eom;
    }
    return ret;
}

// Returns: 0 on success
//         -1 on failure
int proj_save (const char *pr_name, int save_root)
{
    Initialize(proj_save__char*);
    int ret = -1;
    projNode *prj = projNode::find_project (pr_name);
    if (prj == NULL) {
	if (!pr_name || !*pr_name)
	    pr_name = NULL_NAME;

	msg("Save: No project $1", error_sev) << pr_name << eom;
	return ret;
    }
    ret = proj_save (prj, save_root);
    return ret;
}


// Returns: 0 on success
//         -1 on failure
int proj_save_link_rules (projNode *prj)
{
    Initialize(proj_save_link_rules_projNode*);
    int ret = -1;
    int link_cnt = 0; int no_cnt = 0;
    msg("RULE proj $1 - ", normal_sev) << prj->get_name() << eom;
 
    genError *err;
    genTry {
	ret = 0;
	const char *phys_name = (prj->pr_name) ? prj->pr_name->get_name() : "/" ;
	projMap *parent_pm = prj->get_map();
	for (projMap *cur = (projMap *)parent_pm->get_first() ;
	     cur && ret == 0 ;
	     cur = (projMap *)cur->get_next()) {

	    if (cur->get_direction() != REAL_RIGHT_WAY && cur->get_flag() == MEMBER) {
		char full_path[MAXPATHLEN];
		if (cur->is_leading_slash_in_phy_path())
		    strcpy (full_path, "/");
		else
		    strncpy (full_path, phys_name, MAXPATHLEN);

		glob_t ggg;
		int no_stat_call = collect_file_names(ggg, full_path, cur, FILE_TYPE);

		for (int i = 0; i < ggg.gl_matchc ; ++i) {
		    genString new_full_path; genString ln; 
		    projNode *pn = NULL; projModule *mod = NULL;

		    combine_dir_and_sub_dir(full_path, ggg.gl_pathv[i], new_full_path);
		    convert_to_uniform_case((char *) new_full_path.str());
		    prj->fn_to_ln_internal((const char *)new_full_path, ln, &pn);

		    if (ln.length()) mod = projHeader::find_module (ln);
		    if (disbuild_cache_add_rules (new_full_path, mod, link_cnt, no_cnt) != 0) {
			ret = -1;
			break;
		    }
		}
		report_globfree(ggg);
	    }
	}
    } genCatch(err) {
	ret = -1;
    }
    genEndtry;

    if (ret != 0)
	msg("link rule analysis failed", error_sev) << eom;
    else {
	if ( (link_cnt + no_cnt) > 0 ) {
	    if (link_cnt) {
		msg("$1 file", normal_sev) << link_cnt << eom;
	    }
	    if (no_cnt) {
		msg(", $1 fake") << no_cnt << eom;
	    }
	} else
	    msg("no rules", normal_sev) << eom;
    }
    return ret;
}

// returns: 0 success
//         -1 failure
int proj_save_all(int save_root)
{
    Initialize(proj_save_all);
    int res = -1;
    if (disbuild_start_cache() == -1)
	return res;

    projNode *proj = 0;
    if (save_root)
	msg("Project SAVE: saving OUTDATED projects.", normal_sev) << eom;
    else
	msg("Project SAVE: saving ALL projects.", normal_sev) << eom;

    res = 0;
    objArr rule_projs;
    for (int i = 1; proj = projList::search_list->get_proj(i); i++) {
	char const *nm = proj->get_ln();
	if (nm && strcmp(nm, "/__ParaSPECIAL") && strcmp(nm, "/ParaDOCS") && strcmp(nm, "/tmp")) {
	    if (proj->is_visible_proj() == 0) {
		if (is_disbuild_cache()) {
		    rule_projs.insert_last(proj);
		}
	    } else {
		const char *to_save = get_Project_LogFile();
		if ( (to_save && to_save[0]) )
		    msg("Save all; ignore PRJ_LogFile", normal_sev) << eom;

		res = proj_save (proj, save_root);
		if (res != 0)
		    msg("Save: failed to save project $1", error_sev) << nm << eom;
	    }
	}
    }

    // save hidden after all the modules are expanded
    for (int ii = 0 ; ii < rule_projs.size() ; ii ++ ) {
	projNode *p = checked_cast(projNode,rule_projs[ii]);
	res = proj_save_link_rules (p);
    }

    disbuild_end_cache();
    res = 0;
    return res;
}

// This function is looking for first occurance of char c ignore any substrings in ".."
// return: like strchr( char* str, char c)
//
const char* str_find_char( const char* pszText, char c ) 
{ 
    const char *pszRet = 0;
    if ( pszText ) 
    { 
	const char  *psz; 
	while ( *pszText ) 
	{ 
	    if ( *pszText == c ) {
		pszRet = pszText;
		break; 
	    }
	    if ( *pszText == '\"' && (psz = strchr(pszText+1, '\"')) ) 
		pszText = psz; 
	    pszText++; 
	} 
    } 
    return pszRet; 
} 

char* str_find_char( char* pszText, char c ) 
{ 
    char *pszRet = 0;
    if ( pszText ) 
    { 
	char  *psz; 
	while ( *pszText ) 
	{ 
	    if ( *pszText == c ) {
		pszRet = pszText;
		break; 
	    }
	    if ( *pszText == '\"' && (psz = strchr(pszText+1, '\"')) ) 
		pszText = psz; 
	    pszText++; 
	} 
    } 
    return pszRet; 
} 


genString quote_str_if_space (const char* pszIn )
{
	genString txtOut;
	if ( pszIn && *pszIn && str_find_char(pszIn, ' ') ) {
    txtOut.printf ("\"%s\"", pszIn );
		}
	else
		txtOut = pszIn;
	return txtOut;
}


genString dequote_str (const char* pszIn )
{
	genString txtOut;
	if ( pszIn ) 
	{
		char *p1, *p2;
		char  c;
		while ( *pszIn && ( (p1=strchr((char *)pszIn,'\"')) && (p2=strchr((char *)p1+1,'\"')) ) )
		{
			c = *p1;
			*p1 = 0;
			*p2 = 0;
			txtOut += pszIn;
			txtOut += (p1+1);
			*p1 = c;
			*p2 = c;
			pszIn = p2+1;
		}
    txtOut += pszIn;
	}
	else
		txtOut = pszIn;
	return txtOut;
}

int is_str_quoted(const char* s)
{
  int nRet = 0;
  if ( s && *s )
  {
    int len = strlen(s); 
    if (len >= 2 && (*s == '\"' && *(s+len-1) == '\"') )
      nRet = 1;
  }
  return nRet;
}

