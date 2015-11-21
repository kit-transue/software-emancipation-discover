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
// File db_util.C
//#include <types.h>
#include <objOper.h>
#include <db_intern.h>
#include <proj.h>
//#include <steDocument.h>
#include <cLibraryFunctions.h>
#include <msg.h>
#include <machdep.h>

//  demo licensing not currently supported. -jef
void obj_unload (Obj*);

int db_get_incremental ();

void dd_put_func_arg_rtl(app*);
void set_equate_categories(app *);
/* mode = 0 - restore dd only, mode = 1 - complete restore */

app* db_restore (char const *fname, char const *lname)
// Restore a pset file for the file "fname".
// Its pset uses the logic name "lnmae".
{
    Initialize(db_restore(char const *,char const *));
    
    IF (! fname || ! *fname)
	return NULL;

    msg("Restoring $1 ... ", normal_sev) << realOSPATH(fname) << eom; 
    ;

    app* header = db_read_bufs (fname, lname);

    if (header) {
	    header->set_phys_name (fname);
	    msg("done", normal_sev) << eom;
	} else
	    msg("Failed to restore $1", catastrophe_sev) << realOSPATH(fname) << eom;

    return header;
}
void proj_path_report_app (app *ah, projNode *prj);

app* db_restore (char const *fname)
// Restore a pset file for the file "fname".
// Its logic name could be obtained from "fname".
{
    Initialize (db_restore);
    genString lname;
    
    IF (! fname || ! *fname)
	return NULL;

    projNode*proj=NULL;
    projHeader::fn_to_ln (fname, lname, &proj);

    app* appl = db_restore(fname, lname);
    if(appl)
       proj_path_report_app(appl, proj);
    return appl;
}

bool db_restore_complete (const Relational* nd)
{
    Initialize (db_restore_complete);

    Relational* node = (Relational*)nd;

    Assert (node);

    app* header = NULL;
    if (is_appTree (node))
	header = ((appTree*)node)->get_main_header ();
    else if (is_app (node))
	header = (app*)node;
    
    Assert (header);

    char const *fname = header->get_phys_name ();

    Assert (fname);

    msg("Restoring main root $1 ...", normal_sev) << realOSPATH(fname) << eom;
    ;

    bool retval  = (db_read_bufs (fname, 0, header) != 0);

    if (retval) {
      msg("done", normal_sev) << eom;
    }
    else
	msg("Failed to restore $1", catastrophe_sev) << realOSPATH(fname) << eom;

    return retval;
}

void db_restore_roots(app * header, dbRootBufArr& roots, dbRel_format format)
{
  Initialize(db_restore_roots);
  int sz = roots.size();
  db_init_all();
  db_init_id(header);

  dbRel_format fmt_old = db_get_rel_format ();
  db_set_rel_format (format);
  for (int ii=0; ii<sz; ++ii) {
      dbRootBuf * rp = roots[ii];
      if (!rp->def->incremental || ! db_get_incremental ()) {
	  objTree * root = db_restore_tree(header, 0, rp);
	  Assert(root);
	  put_relation(*rp->def->rel, header, root);
	  db_after_rest rst = rp->def->after_rest;
	  appTree* rt = checked_cast (appTree, root);
	  (rt->*rst) (*rp->app_buf);
      } else {
	  db_skip_id (rp->last_id - rp->root_id + 1);
	  rp->root = checked_cast (objTree, DB_NULL);
	  header->put_dbnull (*rp->def->rel);
      }
  }
  db_put_rels ();
  db_end_id (header, roots);
  db_set_rel_format (fmt_old);

  // clear modified flag if it was set
  if (header) header->clear_modified();
}

void db_scan_tree(app* header, objTree * parent, dbRootBuf* rp);
void db_add_rels (int, int);

void db_restore_roots_complete(app * header, dbRootBufArr& roots, 
			       dbRel_format format)
{
  Initialize(db_restore_roots);
  int sz = roots.size();
  db_init_all();
  db_init_id (header);

  dbRel_format fmt_old = db_get_rel_format ();
  db_set_rel_format (format);
  int r_id, l_id;
  for (int ii=0; ii<sz; ++ii) {
      dbRootBuf * rp = roots[ii];
      if (rp->def->incremental) {
	  objTree * root = db_restore_tree(header, 0, rp);
	  Assert(root);
	  put_relation(*rp->def->rel, header, root);
	  db_after_rest rst = rp->def->after_rest;
	  appTree* rt = checked_cast (appTree, root);
	  (rt->*rst) (*rp->app_buf);
	  r_id = rp->root_id;
	  l_id = rp->last_id;
      } else 
	  db_scan_tree (header, 0, rp);
  }
  db_add_rels (r_id, l_id);
  db_end_id (header, roots);
  db_set_rel_format (fmt_old);
}

objTree * db_restore_tree(app* header, objTree * parent, dbRootBuf* rp)
{
  underInitialize(db_restore_tree);
    
  db_intern_buffer * ib = rp->int_buf;
  db_restore_node sn = rp->def->nd_restore;
  Assert(ib->is_node());
  objTree * prev = 0;

  db_app dba;

  do {
    ib->iget(dba);
    
    objTree * node = (*sn)(rp->header, (appTree*)parent, dba, *rp->app_buf);

    int src_id = db_put_id(node);
    int rel_id, trg_id, direction;

    while(ib->is_relation()){
      ib->get_rel(rel_id, trg_id, direction);
      db_store_rel(rel_id, src_id, trg_id, direction);      
    }
    
    if( parent){
      if( !prev)
         parent->put_first(node);
      else
         prev->put_after(node);
    } else {
	rp->root = node;
    }

    prev = node;

    if(!dba.is_leaf){
      db_restore_tree(header, node, rp);
    }
  } while (! dba.is_last);

  return prev;
}

void db_scan_tree(app* header, objTree *parent, dbRootBuf* rp)
{
  underInitialize(db_restore_tree);
    
  db_intern_buffer * ib = rp->int_buf;
  Assert(ib->is_node());
  objTree * prev = 0, *node;

  db_app dba;

  do {
    ib->iget(dba);
    
    if( parent){
      if( !prev)
         node = parent->get_first ();
      else
         node = prev->get_next ();
    } else {
	node = checked_cast (objTree, get_relation (*rp->def->rel, header));
	rp->root = node;
    }

    prev = node;

    int src_id = db_put_id_complete(node);
    int rel_id, trg_id, direction;

    while(ib->is_relation()){
      ib->get_rel(rel_id, trg_id, direction);
      db_store_rel(rel_id, src_id, trg_id, direction);      
    }
    
    if(!dba.is_leaf){
      db_scan_tree(header, node, rp);
    }
  } while (! dba.is_last);
}

/*
    START-LOG-------------------------------

    $Log: db_restore.cxx  $
    Revision 1.14 2000/11/30 09:55:27EST sschmidt 
    Fix for bug 20091: improve output during crashes, sensitivity of tests
// Revision 1.27  1994/07/24  19:03:30  so
// Bug track: 7203,7903,7295
// fix bugs 7203,7903,7295
//
// Revision 1.26  1994/04/07  20:25:29  builder
// Port
//
// Revision 1.25  1994/04/06  14:05:11  jon
// Re-Packaging ParaSET and it's use of License Management
//
// Revision 1.24  1994/01/30  19:43:47  aharlap
// bug track: 5322
//
// Revision 1.23  1993/12/22  14:00:36  aharlap
// added db_incremental
//
// Revision 1.22  1993/07/29  04:03:02  aharlap
// called db_put_id_complete()
//
// Revision 1.21  1993/07/25  18:48:55  trung
// ! //      dd_put_func_arg_rtl(header);
//
// Revision 1.20  1993/06/08  23:00:23  kws
// Only allow restoration of special doc files in ParaMODEL mode
//
// Revision 1.19  1993/05/06  15:21:48  aharlap
// cleanup
//
// Revision 1.18  1993/03/03  03:14:29  trung
// restore rtl when restore smt
//
// Revision 1.17  1993/02/08  03:40:00  aharlap
// changed db_end_id ()
//
// Revision 1.16  1993/02/07  04:19:50  aharlap
// optimized access to db_rel_format
//
// Revision 1.15  1993/02/06  00:41:17  aharlap
// added direction
//
// Revision 1.14  1993/02/03  03:36:39  aharlap
// *** empty log message ***
//
// Revision 1.13  1993/02/01  17:05:52  aharlap
// saved relations from apptree to app
//
// Revision 1.12  1993/01/29  20:47:23  aharlap
// put one side relations from header to DB_NULL
//
// Revision 1.11  1993/01/29  16:14:21  aharlap
// changed messages
//
// Revision 1.10  1993/01/28  21:11:55  aharlap
// changed message
//
// Revision 1.9  1993/01/28  18:39:03  aharlap
// added incremental restoring
//
// Revision 1.8  1993/01/26  01:08:24  aharlap
// set root_id and last_id
//
// Revision 1.7  1993/01/24  21:49:46  aharlap
// changed return type for db_restore ()
//
// Revision 1.6  1993/01/18  15:24:47  smit
// clear modified flag after restoring.
//
// Revision 1.5  1993/01/12  01:50:41  aharlap
// called db_put_rels ()
//
// Revision 1.4  1993/01/11  04:19:05  aharlap
// go to new db
//
// Revision 1.3  1993/01/10  23:35:04  aharlap
// called db_after_restore
//
// Revision 1.2  1993/01/08  08:15:35  aharlap
// fixed bug
//
// Revision 1.1  1993/01/07  04:11:01  mg
// Initial revision
//

    END-LOG---------------------------------
*/




















































