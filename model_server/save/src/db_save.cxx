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
// File smt_save.C

#include <vpopen.h>
#include <genError.h>
#include <objOper.h>
#include <db_intern.h>
#include <saveXref.h>
#include <smt.h>
#include <xref.h>
#include "objRawApp.h"  //#include <ste_interface.h>
#include <fileCache.h>
#include <Question.h>
#include <cLibraryFunctions.h>
#include <msg.h>
#include <machdep.h>
#include <messages.h>
#include <externApp.h>
#include <driver_mode.h>

extern bool is_forgiving();
int ste_interface_raw_save(objRawApp*);
#define F_OK            0       /* does file exist */
#define X_OK            1       /* is it executable by caller */
#define W_OK            2       /* is it writable by caller */
#define R_OK            4       /* is it readable by caller */

static struct db_app dba_static;

// internal call to return correct dialog answer (popup_Question) :
// 0 - failure, 1 - success, 2- unchanged source,  -1 - Cancel.
int db_save_or_cancel(app *h, char const *fname, int save_xref)
{
    Initialize(db_save_or_cancel);

    db_init_all();
    
    memset (&dba_static, '\0', sizeof (dba_static));
    dba_static.is_node = 1;
    if (is_objRawApp(h)) {
        char const *the_name = fname ? fname : h->get_filename ();
	if ( fname || !((objRawApp *)h)->is_new() ) {
	    if (OS_dependent::access (the_name, W_OK)) {
		if (!is_gui()) {
		    msg("File $1 is write protected.", error_sev) << fname << eom;
		    return 0;
		} else {
		  int answer = dis_question (T_SAVE, B_YES, B_NO, Q_FILEWRITEPROTECTED, the_name);
		  if (answer != 1)
		    return answer;     // 0 or -1
		}
	    }
        }

	int res = ste_interface_raw_save (objRawAppPtr(h));
	if (((objRawApp *)h)->is_new() && res) {
	    ((objRawApp *)h)->is_new(0);
	    global_cache.invalidate (the_name);
	}

        return res;
    }

    char const *file = fname ? fname : h->get_phys_name ();

    IF (!file)
	file = h->get_filename ();

    Assert(file);

    byte desc_id = h->descr()->type_id;
    Assert(desc_id != 0xFF);

    db_init_id(h);

    // dis_message(NULL,MSG_INFORM,"M_DB_SAVINGMODULE", file); // moved to after save.

    dbRootBufArr roots;
    db_get_roots(h, roots);
    int no_roots = roots.size();
    int ii, retval = 0;
    for(ii=0; ii<no_roots; ++ii){
        dbRootBuf * rp = roots[ii];
	rp->root_id = db_get_cur_id ();
        db_put_id(rp->root);
	rp->last_id = db_get_cur_id () - 1;
    }
    dbRel_format fmt_old = db_get_rel_format ();
    db_set_rel_format (db_calculate_rel_format (roots));

    for(ii=0; ii<no_roots; ++ii){
        dbRootBuf * rp = roots[ii];
        db_save_root(rp);
    }

    char need_save_reparse_bit = 0;
    if (is_smtHeader(h))
    {
        smtHeader *smtH = checked_cast(smtHeader,h);
//        if (XREF_file_included_by(smtH) == 0) // only for the source file
        {
            if (smtH->get_actual_mapping()== 0)
                need_save_reparse_bit = 1;
        }
    }

    if ((retval = db_write_bufs(roots, fname)) > 0) {
	h->clear_modified();
	h->clear_imported();
	h->clear_src_modified ();
	msg("Saved model of '$1'.", normal_sev) << realOSPATH(file) << eom;
    } else {
        msg("Attempt to save file '$1' failed.", catastrophe_sev)
		<< realOSPATH(file) << eom;
    }
    db_end_id (h, roots);
    db_set_rel_format (fmt_old);

    if (need_save_reparse_bit) {
	if (is_model_build())    // ignore forgive for interactive session
	    db_set_bit_need_reparse(file, is_forgiving() ? 2 : 1);
	else
	    db_set_bit_need_reparse(file, 1);
    }

    if (retval > 0)
	// update modification date in xref
	XREF_set_last_modified_date (h);

    // save xref for this file.
    if (retval > 0 && save_xref)
        save_cur_xref (h);

    return retval;
}    

// call this one when we want to control whether to
// save the Xref module along with the source file
// (save_xref != 0 enables saving, 0 disables)

int db_save(app *h, char const *fname, int save_xref)
{
// In this call "Cancel" behaves like unsuccess;
// db_save called from too many places which care only about 0/1;
// eventually they should call db_save_or_cancel and distinguish 0/1/-1

    Initialize(db_save);
    int retval = 0;

    if(is_externApp(h)){
       externApp*ap = (externApp*) h;
       retval = (int) ap->externSave();
       return retval;
    }

    retval = db_save_or_cancel(h, fname, save_xref);
    if (retval < 1)
        retval = 0;
    
    return retval;
}    
// this one can be called when we always want
// to save the Xref module along with the source file

int db_save(app *h, char const *fname)
{
    Initialize(db_save);
    return db_save(h, fname, 1);
}    


void db_save_root(dbRootBuf *rp)
{
    Initialize(db_save_root);
    db_save_tree(rp, rp->root);
    return;   
}


void db_save_tree(dbRootBuf *rp, objTree * r)
{
    struct db_app dba = dba_static; 

    dba.is_leaf = !r->get_first ();
    dba.is_last = !r->get_next ();  

    if (rp->root == r)
	dba.is_last = 1;

    appTree *at = (appTree*) r;

    db_save_node sn = rp->def->nd_save;
    (*sn)(rp->header, at, dba, *rp->app_buf);
    rp->int_buf->iput(dba);

    db_save_rels(*rp->int_buf, at);

    for(r=at->get_first(); r; r = r->get_next())
	db_save_tree(rp, r);
}

void db_save_rels(db_intern_buffer& rp, appTree * node)
{
    int src_id = node->get_id ();
    relArr& ar = node->get_arr();
    int sz = ar.size();
    for(int ii = 0; ii < sz; ii++) {
	Relation * rel = ar[ii];
	RelType * rt = rel->get_rel_type();
        RelType *inv = rt->get_inv_type ();

        byte rid = rt->get_rel_id();
	byte inv_rid = inv->get_rel_id ();
	byte stored_rid = rid ? rid : inv_rid;
	if(stored_rid){
	    Obj * mb = rel->get_members();
	    Obj * m;
	    ForEach(m, *mb){
              int trg_id = m->get_id();
	      if (src_id >= trg_id && trg_id > db_start_id - 2) {
		  int direction = (stored_rid != rid);
		  rp.put_rel(stored_rid, trg_id, direction);
	      }
            }
        }
    }
}

/*
  START-LOG-------------------------------
  
  $Log: db_save.cxx  $
  Revision 1.20 2001/07/25 20:42:52EDT Sudha Kallem (sudha) 
  Changes to support model_server.
 * Revision 1.1  1996/12/05  15:38:50  joshj
 * Initial revision
 *
// Revision 1.43  1994/07/07  01:06:37  boris
// Bug track: 6713
// Added Batch mode to Ext Clients
//
// Revision 1.42  1994/05/05  21:23:29  aharlap
// *** empty log message ***
//
// Revision 1.41  1994/05/05  20:06:21  boris
// Bug track: 6794
// Fixed "slow merge" problem, and #6977 asyncronous save
//
// Revision 1.40  1994/04/15  14:27:59  davea
// bugs 6649, 6960
// use member access function, rather than direct access
// to member variable.
// Improve error handling for directory creation problems
//
// Revision 1.39  1994/04/06  14:06:08  jon
// Re-Packaging ParaSET and it's use of License Management
//
// Revision 1.38  1993/12/13  15:01:22  mg
// Bug track: 4934
// restructured dependency_analysis to prepesare for raw filoes
//
// Revision 1.37  1993/12/03  21:00:24  so
// Bug track: n/a
// don't do dependency analysis while saving a file but instead do it when run
// -batch -u locally.
//
// Revision 1.36  1993/12/03  14:56:05  himanshu
// Bug track: newcode
// added a call gpi_save_out to send a save file signal to an external file
//
// Revision 1.35  1993/11/18  17:45:42  aharlap
// Bug track: N/A
// First stage of delayed smt mapping
//
// Revision 1.34  1993/09/30  23:25:16  trung
// Bug track: 0
// change save_cur_xref to take app, part of fix for saving includes file for smit
//
// Revision 1.33  1993/09/03  23:32:19  boris
// Fixed return value on db_save
//
// Revision 1.32  1993/09/03  21:25:09  aharlap
// bug # 4594
//
// Revision 1.31  1993/09/03  03:03:43  boris
// Fixed bug #4617 with wrong message on saved unmodified text
//
// Revision 1.30  1993/09/01  16:09:17  boris
// Fixed performance problem, bug #4656
//
// Revision 1.29  1993/08/22  22:46:15  sergey
// Added another layer to db_save - db_save_or_cancel. Fixed bug #4388.
//
// Revision 1.28  1993/08/19  16:13:18  boris
// Fixed "new" file saving
//
// Revision 1.27  1993/08/12  00:28:38  aharlap
// bit need_reparse set for .h too
//
// Revision 1.26  1993/07/20  18:34:13  harry
// HP/Solaris Port
//
// Revision 1.25  1993/06/26  03:45:15  so
// fix by davea => add one space
//
// Revision 1.24  1993/06/25  13:43:57  so
// set need_reparse bit
//
// Revision 1.23  1993/06/17  19:16:17  davea
// bug 3736 - suppress popup if non-interactive
//
// Revision 1.22  1993/06/14  18:00:15  so
// fix bug 3686
// use chmod and utimes to replace "system(chmod fn; touch fn; chmod fn");
//
// Revision 1.21  1993/05/17  19:51:36  so
// Implement Impact Analysis.
//
// Revision 1.20  1993/04/13  19:12:23  kws
// Handle asking overwrite of read only files
//
// Revision 1.19  1993/04/13  11:14:12  kws
// If the app is a raw app - call teh ste interface to save the file
//
// Revision 1.18  1993/03/09  23:33:24  aharlap
// db_save.Cadded field  dont_save_unchanged_src
//
// Revision 1.17  1993/02/16  09:05:37  smit
// set modified date in xref after save.
//
// Revision 1.16  1993/02/12  14:52:19  efreed
// add new db_save which distinguishes whether to call the Xref
// module save method. the existing db_save function now calls
// the new one to do its work, with xref module saving turned on.
//
// Revision 1.15  1993/02/11  00:27:47  aharlap
// changed return value for db_save.C
//
// Revision 1.14  1993/02/09  18:33:19  kws
// Activate new lmgr interface
//
// Revision 1.13  1993/02/08  03:40:00  aharlap
// changed db_end_id ()
//
// Revision 1.12  1993/02/07  04:19:50  aharlap
// optimized access to db_rel_format
//
// Revision 1.11  1993/02/06  01:57:52  smit
// *** empty log message ***
//
// Revision 1.10  1993/02/06  01:07:51  smit
// fix buildxref save problem.
//
// Revision 1.9  1993/02/06  00:41:17  aharlap
// added direction
//
// Revision 1.8  1993/02/04  01:22:08  aharlap
// *** empty log message ***
//
// Revision 1.7  1993/02/03  03:36:39  aharlap
// *** empty log message ***
//
// Revision 1.6  1993/02/01  17:05:52  aharlap
// saved relations from apptree to app
//
// Revision 1.5  1993/01/26  01:09:44  aharlap
// added call to db_set_short_rel ()
//
// Revision 1.4  1993/01/18  15:22:45  smit
// clear modified flag after save.
//
// Revision 1.3  1993/01/17  22:15:39  aharlap
// return value from db_save
//
// Revision 1.2  1993/01/12  00:47:44  aharlap
// fixed bug
//
// Revision 1.1  1993/01/07  04:11:01  mg
// Initial revision
//
  
  END-LOG---------------------------------

*/


