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
#include <msg.h>
#include <shell_calls.h>
#include <proj.h>
#include <cmd.h>
#include <cmd_enums.h>
#include <objOper.h>
#ifndef ISO_CPP_HEADERS
#include <string.h>
#else /* ISO_CPP_HEADERS */
#include <cstring>
using namespace std;
#endif /* ISO_CPP_HEADERS */
#include <machdep.h>
#include <cLibraryFunctions.h>
#include <genTmpfile.h>
#include <smt.h>
#include <genString.h>
#ifndef ISO_CPP_HEADERS
#include <ctype.h>
#else /* ISO_CPP_HEADERS */
#include <cctype>
#endif /* ISO_CPP_HEADERS */
#include <ste_smod.h>
#include <OperPoint.h>
#ifndef ISO_CPP_HEADERS
#include <stdlib.h>
#else /* ISO_CPP_HEADERS */
#include <cstdlib>
#endif /* ISO_CPP_HEADERS */
#include <db.h>

#include "db_intern.h"

// if src_name == 0, only return the length
// if src_name != 0, src_name is a temporary file name, which contains the contents

int start_ast_merge(smtHeader*);
int end_ast_merge();;

class oneEdit : public Obj {
public:
    int diff_st;
    int diff_len;
    
    smtTree *st;
    smtTree *en;

    int st_offset;
    int en_offset;

    genString txt;

    oneEdit(smtHeader *h, int n, int m, const char *new_txt);
};

oneEdit::oneEdit (smtHeader *h, int n, int m, const char *new_txt) : 
    diff_st(n), diff_len(m), txt(new_txt), st_offset(0), en_offset(-1)
{
    Initialize(oneEdit::oneEdit);
    smtTree *root = checked_cast(smtTree,h->get_root());
    st = smt_get_token_by_offset(root, n, st_offset, 0);
    if (n == 0 && st == 0) {
	st = root;              // empty buffer
	m  = 0;
    }
    if (m == 0) {
	en        = st;         // insert
	en_offset = st_offset;
    } else
	en = smt_get_token_by_offset(root, n+m, en_offset, 1);
}


// returns: 1 if there are just whites
//          0 on a first non white char
static int mrg_is_just_whites (const char *txt, int len)
{
    Initialize(mrg_is_just_whites);
    int ret = 1;
    if (!txt || !len) return ret;
    for (int i = 0 ; i < len ; i ++) {
	if (!isspace(txt[i])) {
	    ret = 0;
	    break;
	}
    }
    return ret;
}

void smt_replace_region (int, smtTree *, int, smtTree *, const char *, int do_tidy);

// returns: 1 on success
//          0 on failure
static int mrg_apply (smtHeader *h, objArr& edits, int& do_not_save_flag) 
{
    Initialize(mrg_apply);
    int ret = 1; 
    int ins = 0; int del = 0; int sam = 0; int rep = 0; int conflict = 0;
    int just_whites = 1;

    int sz = edits.size();
    if (sz) {
	start_smt_editing();
	start_transaction()
        {
	    start_ast_merge(h);
	    for (int i = sz - 1 ; i >= 0 ; i --) {
		oneEdit *ed = (oneEdit *)edits[i];
		
		if (just_whites &&
		   (just_whites = mrg_is_just_whites ((const char *)ed->txt, ed->txt.length())) &&
		   (just_whites = mrg_is_just_whites (h->srcbuf + ed->diff_st, ed->diff_len)) )
		    ;		    

		smt_replace_region (ed->st_offset, ed->st, ed->en_offset, ed->en, ed->txt, 0);

		if     (!ed->diff_len &&  ed->txt.length())
		    ins ++;
		else if (ed->diff_len && !ed->txt.length())
		    del ++;
		else if (ed->diff_len &&  ed->txt.length())
		    rep ++;

		delete ed;
	    }

	    h->tidy_tree();
	    h->vrn = 1;
	    end_ast_merge();
	    h->clear_src_modified();
	}
	end_transaction();
	commit_smt_editing ();

	cmd_validate("num of inserted area",     ins);
	cmd_validate("num of deleted area",      del);
	cmd_validate("num of same changed area", sam);
	cmd_validate("num of replaced area",     rep);
	cmd_validate("num of conflicted area",   conflict);

	const char *whites = ""; if (just_whites) whites = "; just whites";
	int all      = ins + del + rep;
	msg("Merge_statistics:char_based_merge.C", normal_sev) << all << eoarg << whites << eoarg << ins << eoarg << del << eoarg << rep << eom;

	if (do_not_save_flag && just_whites)
	    do_not_save_flag = 2;
    }
    return ret;
}

#define LINE_LEN 1024

// returns: 1 on success
//          0 on failure
static int mrg_smt (smtHeader *h, FILE *diff, int& do_not_save_flag)
{
    Initialize(mrg_smt);
    objArr edits;

    char tmp[LINE_LEN];
    int n = 0; int m = 0;
    int in_txt  = 0; int in_long = 0; int sz = 0; char delem = 0;
    int linenm  = 0; int ret = 0; genString line; genString new_txt;

    while ( (fgets(tmp, LINE_LEN, diff)) ) {
	if (in_long) 
	    line += tmp;
	else {
	    line = tmp;
	    linenm ++;
	}

	sz = strlen(tmp);
	if (tmp[sz - 1] != '\n') {
	    in_long = 1;
	    continue;
	}

	in_long = 0;
	char *ll = (char *)line; char *ln_txt = 0; char *coma1 = 0;
	if (in_txt) {
	    if (ll[0] != '>' || ll[1] != ' ') {
		msg("Wrong continuation indent; line $1", error_sev) << linenm << eom;
		return ret;
	    }
	    ln_txt = ll + 2;
	} else {
	    coma1 = strchr (ll, ',');
	    if (!coma1) {
		msg("No first parameter; line $1", error_sev) << linenm << eom;
		return ret;
	    }
	    coma1[0] = 0; n = atoi (ll);
	    char *coma2 = strchr (coma1 + 1, ',');
	    if (!coma2) {
		msg("No second parameter; line $1", error_sev) << linenm << eom;
		return ret;
	    }

	    coma2[0]   = 0; m = atoi (coma1 + 1);
	    char *rest = coma2 + 1;
	    if (!rest[0] || !rest[1]) {
		msg("No delimiters; line $1", error_sev) << linenm << eom;
		return ret;
	    }
	    delem   = rest[0];
	    rest[0] = 0;
	    ln_txt  = rest + 1;
	}

	int len_ln      = strlen (ln_txt);
	char *end_delem = 0;
	if (len_ln > 1 && ln_txt[len_ln - 2] == delem)
	    end_delem = &ln_txt[len_ln - 2];
#ifdef _WIN32
	else if (len_ln > 2 && ln_txt[len_ln - 2] == '\015' && ln_txt[len_ln - 3] == delem)
	    end_delem = &ln_txt[len_ln - 3];
#endif

	in_txt = (end_delem) ? 0 : 1;

	if ( !in_txt ) {
	    end_delem[0] = 0;
	    new_txt += ln_txt;
	    oneEdit *ed = new oneEdit (h, n, m, (char *)new_txt);
	    if (ed->st && ed->en)
		edits.insert_last (ed);
	    else {
		msg("Failed to find token; offset $1, length $2", error_sev) << n << eoarg << m << eom;
		delete ed;
	    }

	    new_txt = 0; n = 0; m = 0; delem = 0;
	} else
	    new_txt += ln_txt;
    }
    ret = mrg_apply (h, edits, do_not_save_flag);
    return ret;
}

static void debug_merge_after (smtHeader *h, const char *file, const char *exe, 
			       const char *basename, const char *tmp_dir)
{
    msg("Debugging after Char Based Merge. TMPDIR = $1", normal_sev) << tmp_dir << eom;

    genString dbg_pset; genString dbg_src; genString dbg_diff;

    dbg_pset.printf ("%s%s%s", tmp_dir, basename, ".pset");
    dbg_src.printf  ("%s%s", tmp_dir, basename);
    dbg_diff.printf ("%s%s%s", tmp_dir, basename, ".diff");

    OSapi_unlink((char *)dbg_pset);
    OSapi_unlink((char *)dbg_src);
    OSapi_unlink((char *)dbg_diff);

    ::db_save(h, (char *)dbg_src);

    int status = vsysteml (exe, (char *)dbg_src, file, (char *)dbg_diff, (char *)NULL);
	
    FILE *diff = NULL;
    if (status != 0 || !(diff = OSapi_fopen((char *)dbg_diff, "r")) ) {
	msg("Failed to create DEBUG diff file\n          cmd: $1\n               $2\n               $3\n               > $4", error_sev) << (char *)exe << eoarg << (char *)dbg_src << eoarg << file << eoarg << (char *)dbg_diff << eom;
    } else {
	char bbb[3];
	int len = OSapi_fread (bbb, 1, 1, diff);
	if (len) {
	    msg("dbg_diff file is not EMPTY.\n          cmd: $1\n               $2\n               $3\n               > $4", error_sev) << (char *)exe << eoarg << (char *)dbg_src << eoarg << file << eoarg << (char *)dbg_diff << eom;
	} else {
	    msg("Merge succeeded; dbg_diff file is empty.\n          cmd: $1\n               $2\n               $3\n               > $4", normal_sev) << (char *)exe << eoarg << (char *)dbg_src << eoarg << file << eoarg << (char *)dbg_diff << eom;
	}
	OSapi_fclose (diff);
    }
}

// returns: 1 on success
//          0 on failure
//   unload flag      - unload after merge anf save
//   do_not_save_flag - skip saving if == 1; change to 2 if no changes or just spaces changed
//
int char_based_merge (app *aptr, char const *file, char const *paraset_file, int unload_flag,
		      int& do_not_save_flag)
{
    Initialize(char_based_merge);
    int ret = 0;
    if (!aptr || !file || !file[0] || !paraset_file || !paraset_file[0]) return ret;
    smtHeader *h = checked_cast(smtHeader,aptr);

    char const *basename = strrchr (file, '/');
    char const *dbg_dir  = OSapi_getenv("DIS_MERGE2FILE_DEBUG_DIR");
    char const *tmp_dir  = NULL;
    if (dbg_dir && dbg_dir[0]) {
	tmp_dir = dbg_dir;
    } else
	tmp_dir  = OSapi_getenv("TMPDIR");

    int  tmp_len   = OSapi_strlen(tmp_dir);

    if (!basename || !tmp_dir || !tmp_dir[0]) return ret;

    genString pset_src;
    // remove all slashes at the end of the directory name
    int len = tmp_len - 1;
    while (len > 0 && tmp_dir[len] == '/') {
        --len;
    }
    pset_src.put_value(tmp_dir, len);

    pset_src += basename;

    OSapi_unlink((char const *)pset_src);
    len = db_read_src_from_pset (paraset_file, (char const *)pset_src);
    if (len < 0) return ret;

    genTmpfile diff_txt("MRG_");
    genString exe;
    exe.printf ("%s/bin/mrg_diff", OSapi_getenv("PSETHOME"));
    int status = vsysteml (exe, (char *)pset_src, file, diff_txt.name(), (char *)NULL);
	    
    FILE *diff = NULL;
    if (status != 0 || !(diff = OSapi_fopen(diff_txt.name(), "r")) ) {
	msg("Failed to create diff file\n          cmd: $1\n               $2\n               $3\n               > $4", error_sev) << (char *)exe << eoarg << (char *)pset_src << eoarg << file << eoarg << diff_txt.name() << eom;
	return ret;
    }
    
    OSapi_unlink ((char *)pset_src);

    msg("Merging file $1", normal_sev) << file << eom;
    msg("pset_src: $1 , diff_txt: $2", normal_sev) << (char *)pset_src << eoarg << diff_txt.name() << eom;
    start_transaction ()
    {
	ret = mrg_smt (h, diff, do_not_save_flag);
    }
    end_transaction();

    OSapi_fclose (diff);
    OSapi_unlink (diff_txt.name());

    if (dbg_dir && dbg_dir[0])
	debug_merge_after ( h, file, exe, basename, tmp_dir );

    if (!do_not_save_flag)
	::db_save(h, file);

    if (unload_flag)
	obj_unload (h);

    return ret;
}

