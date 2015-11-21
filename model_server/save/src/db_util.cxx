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
 
#include <cLibraryFunctions.h>

#ifndef ISO_CPP_HEADERS
#include <stdlib.h>
#else /* ISO_CPP_HEADERS */
#include <cstdlib>
using namespace std;
#endif /* ISO_CPP_HEADERS */

#include <msg.h>
#include <psetmem.h> 
#include <Relational.h>
#include <objOper.h>
#include <db_intern.h>
#include <ddKind.h>
#include <objArr.h>

#include <machdep.h>
#include <messages.h>

genArr(relStruct);

static int rel_init;

static RelType** rels;

RelType* db_get_rel(int ii) {return rels[ii];}

static void db_prt_rel(RelType*rel)
{
  msg("$1 fsi $2") << rel->get_name() << eoarg << (int)rel->forw << (int)rel->save << (int)rel->ifl << eom;
  if (rel->xref_id) 
      msg(" xt $1\t") << linkType_name(rel->xref_id) << eom;
}

extern "C" void db_print_rels()
{
  for(int ii=1; ii <= db_rel_table_size; ++ii){
     RelType * rel = rels[ii];
     msg("[$1]  ") << ii << eom;
     if (!rel) msg("NULL") << eom;
     else {
	 db_prt_rel(rel);
	 db_prt_rel(rel->get_inv_type());
     }
     msg("") << eom;
  }
}


int  db_init_rels()
  {
  Initialize(db_init_rels);

  RelType *rel, *inv;
  int rel_id = 0;
  rels = (RelType**)psetmalloc (sizeof (RelType*) * (db_rel_table_size + 1));
  OS_dependent::bzero (rels, sizeof (RelType*) * (db_rel_table_size + 1));
  for (rel =RelType::first; rel; rel = rel->next){
    char const *name = rel->get_name();
    inv = rel->get_inv_type();
    int found = 0;
    int ii;
    for(ii = 0; ii< db_rel_table_size; ++ii) {
       if(strcmp(name, db_rel_table[ii]) == 0){
          found = 1;
          break;
       }
    }
    if(found){
       if(inv && inv != rel){        
           inv->save = 1;
           byte inv_id = inv->get_rel_id();
           Assert(!inv_id);
       }
       rels[ii+1] = rel;
       rel->put_rel_id(ii+1);
       rel->save = 1;
    } else {
       rel->put_rel_id(0);
    }
  }
  return 0;
}

int db_init_descr ()
{
    Initialize(db_init_descr);

    int ret_val = 0;

    for (int jj = 0; jj < db_def_tables_size; ++jj) {
	int ii;
	for (ii=0; ii< db_def_tables[jj].size; ++ii) {
	    db_def * def = db_def_tables[jj].def + ii;
	    class_descriptor* cur = def->head;
	    if(!cur) continue;
	    if(cur->type_id == 0xFF)
		* (byte*) (&cur->type_id) = (jj << 6) | ii;

	}
	ret_val += ii;
    }
    return ret_val;
}

int db_init_all()
{
  if(rel_init) return 0;
  db_init_rels();
  db_init_descr();
  
  rel_init = 1;
  return 1;
}

struct rel_rec
{
  RelType* rel;
  int src_id;
  int trg_id;
};

genArr(rel_rec);

struct db_rels_level {
  int db_cur_id;
  genArrOf(rel_rec) db_rels;
  objArr db_ids;
  db_rels_level* next;
  db_rels_level (app*);
  ~db_rels_level ();
  static db_rels_level* head; 
};

db_rels_level* db_rels_level::head = NULL;

dbRel_format db_calculate_rel_format (dbRootBufArr& roots)
{
    Initialize (db_set_rel_format);

    int no_roots = roots.size ();
    Assert (no_roots);
    dbRel_format format = R4_22_8;

    int last_id = roots[no_roots - 1]->last_id;
    if (last_id < 16384)
        format = R3_14_8;
    else if (last_id < 32728 && db_rel_table_size < 127)
        format = R3_15_7;
    else if (last_id < 65536 && db_rel_table_size < 64)
        format = R3_16_6;

    return format;
}

int db_get_cur_id ()
{
    return db_rels_level::head->db_cur_id;
}

db_rels_level::db_rels_level (app* h)
  : db_cur_id (db_start_id), db_rels (), db_ids (), next (db_rels_level::head)
{
    for (int ii=0; ii<db_start_id - 1; ++ii)
	db_ids.insert_last(NULL);
    db_ids.insert_last(h);
    h->put_id (db_start_id - 1);
    head = this;
}    

db_rels_level::~db_rels_level()
{
     head = next;
}

void db_init_id(app* h)
{
    new db_rels_level (h);
}

static void reset_tree_id (objTree*r)
{ 
    r->put_id (1);
    for (objTree*cur=r->get_first(); cur; cur=cur->get_next())
	reset_tree_id(cur);
}

void db_end_id (app*, dbRootBufArr&)
{
    db_rels_level * hd = db_rels_level::head;
    objArr& db_ids = hd->db_ids;
    int size = db_ids.size ();
    for (int ii = db_start_id; ii < size; ++ii)
        db_ids [ii]->put_id (1);
    delete hd;
}

void db_skip_id (int nn /* , Obj* obj */)
{
    for (int ii = 0; ii < nn; ++ii)
	db_rels_level::head->db_ids.insert_last (DB_NULL);

    db_rels_level::head->db_cur_id += nn;
}

void db_store_rel(int rel_id, int src_id, int trg_id, int direction)
{
  if (rel_id <= db_rel_table_size) {
    rel_rec * rp = db_rels_level::head->db_rels.grow(1);
    rp->src_id = src_id;
    rp->trg_id = trg_id;
    rp->rel = rels[rel_id];
    if (direction)
      rp->rel = rp->rel->get_inv_type ();
  } else {
    msg("Unrecognizable relation in pset file. Please contact UPSPRING Customer Support.\nPlease provide version of software you are using now\nand version used to build model.", warning_sev) << eom;
  }
}

void db_put_rels_into_arr(relStructArr& rel_arr)
{
    Initialize(db_put_rels_into_arr);

    int no_rel = db_rels_level::head->db_rels.size();
    relStruct * new_rel =  rel_arr.grow(no_rel);
    int count = 0;

    for(int ii=0; ii < no_rel;) {
	int group_size = 0;
	int group_status = 0;
	rel_rec* rp = db_rels_level::head->db_rels[ii];
	RelType* rel = rp->rel;
	int src_id = rp->src_id;
	objTree* src = (objTree*) db_rels_level::head->db_ids[src_id];
	int jj = ii;
	objTree* trg;

	do {
	    ++group_size;
	    trg = (objTree*) db_rels_level::head->db_ids[rp->trg_id];
	    if (trg == DB_NULL)
		++group_status;
	    else {
		new_rel->src = src;
		new_rel->trg = trg;
		new_rel->rel = rel;
		new_rel ++;
		count ++;
	    }
	    if (++jj == no_rel)
		break;
	    rp = db_rels_level::head->db_rels[jj];
	} while (rp->src_id == src_id && rp->rel == rel);
	 
	if (group_status) {
	    new_rel->src = src;
	    new_rel->trg = objTreePtr(DB_NULL);
	    new_rel->rel = rel;
	    new_rel ++;
	    count ++;
        }
	ii += group_size;
    }
    if(no_rel > count)
       rel_arr.grow(count - no_rel);  /* shrink */
}

void db_put_rels()
{
    Initialize(db_put_rels);

    relStructArr rel_arr;

    db_put_rels_into_arr(rel_arr);
    Relational::restore_rel(rel_arr);
}

void db_add_rels (int root_id, int last_id)
{
    Initialize(db_add_rels);	
    int no_rel = db_rels_level::head->db_rels.size();
    for(int ii=0; ii<no_rel;) {
	int group_size = 0;
	int group_status = 0;
	rel_rec* rp = db_rels_level::head->db_rels[ii];
	RelType* rel = rp->rel;
	int src_id = rp->src_id;
	objTree* src = (objTree*) db_rels_level::head->db_ids[src_id];
	int jj = ii;
	objTree* trg;

	if (rp->src_id >= root_id && rp->src_id <= last_id) {
	    trg = (objTree*) db_rels_level::head->db_ids[rp->trg_id];
	    src->restore_rel (*trg, rel);
	    ++ii;
	    continue;
	}
	
	do {
	    ++group_size;
	    if (rp->trg_id >= root_id && rp->trg_id <= last_id) {
		if(!group_status)
		    src->rem_dbnull (rel);
		++group_status;
		trg = (objTree*) db_rels_level::head->db_ids[rp->trg_id];
		src->restore_rel (*trg, rel);
	    }
	    if (++jj == no_rel)
		break;
	    rp = db_rels_level::head->db_rels[jj];
	} while (rp->src_id == src_id && rp->rel == rel);

	ii += group_size;
    }
}

int db_put_id(objTree* r)
{
  db_rels_level::head->db_ids.insert_last(r);
  int cur_id = db_rels_level::head->db_cur_id++;
  r->put_id(cur_id);
  for(objTree*cur=r->get_first(); cur; cur=cur->get_next())
    db_put_id(cur);
  return cur_id;
}

int db_put_id_complete(objTree* r)
{
  db_rels_level::head->db_ids.insert_last(r);
  int cur_id = db_rels_level::head->db_cur_id++;
  r->put_id(cur_id);
  return cur_id;
}

static int rel_test(RelType* rel, Relational * rp)
  {
  relArr& ra =  rp->get_arr();
  int sz = ra.size();
 
  RelType * rt;

  for(int i=0; i < sz; ++i)
    {
    Relation * cr = ra[i];
    rt = cr->get_rel_type();
    if(rel==rt) return 1;
    }
  return 0;
}

void db_get_roots(app * header, dbRootBufArr& roots)
{
    Initialize(db_get_roots);

    for (int jj = 0; jj < db_def_tables_size; ++jj)
	for(int ii=0; ii< db_def_tables[jj].size; ++ii){
	    db_def * def = db_def_tables[jj].def + ii;
	    class_descriptor* cur = def->head;
	    if(cur && cur != header->descr()) continue;
	    objTree * root = (objTree*) get_relation(*(def->rel), header);
	    if(root){
		dbRootBuf * rb = roots.grow(1);
		rb->header = header;
		rb->root =   root;
		rb->def = def;
		rb->app_buf = new db_buffer;
		rb->int_buf = new db_intern_buffer;
	    }
      }
}


objTree * db_get_node_from_id(int id)
{
    return (objTree*) db_rels_level::head->db_ids[id];
}
/*
    START-LOG-------------------------------

    $Log: db_util.cxx  $
    Revision 1.13 2001/07/25 20:42:54EDT Sudha Kallem (sudha) 
    Changes to support model_server.
// Revision 1.23  1994/05/05  21:25:45  aharlap
// *** empty log message ***
//
// Revision 1.22  1994/02/08  22:50:18  builder
// Port
//
// Revision 1.21  1994/01/30  19:43:47  aharlap
// bug track: 5322
//
// Revision 1.20  1994/01/13  02:52:21  kws
// Use psetmalloc
//
// Revision 1.19  1993/07/29  04:03:46  aharlap
// some cleanup
//
// Revision 1.18  1993/07/20  18:35:10  harry
// HP/Solaris Port
//
// Revision 1.17  1993/07/16  16:11:05  mg
// *** empty log message ***
//
// Revision 1.16  1993/06/04  19:00:05  bakshi
// typecasts to unconfuse c++3.0.1 arg matching
//
// Revision 1.15  1993/02/28  01:33:51  aharlap
// fixed dependency saved file from executable
//
// Revision 1.14  1993/02/22  21:51:02  builder
// include stdlib.h
//
// Revision 1.13  1993/02/08  03:40:00  aharlap
// changed db_end_id ()
//
// Revision 1.12  1993/02/07  04:19:50  aharlap
// optimized access to db_rel_format
//
// Revision 1.11  1993/02/06  00:41:17  aharlap
// added direction
//
// Revision 1.10  1993/02/05  03:39:49  aharlap
// changed db_put_rels () and db_add_rels ()
//
// Revision 1.9  1993/02/01  17:05:52  aharlap
// saved relations from apptree to app
//
// Revision 1.8  1993/01/28  18:40:41  aharlap
// added incremental restoring
//
// Revision 1.7  1993/01/26  01:10:51  aharlap
// added functions for using short format saving relations
//
// Revision 1.6  1993/01/25  00:45:32  aharlap
// cleanup
//
// Revision 1.5  1993/01/21  21:30:05  aharlap
// avoid using global static variables
//
// Revision 1.4  1993/01/12  00:48:15  aharlap
// *** empty log message ***
//
// Revision 1.3  1993/01/08  00:30:02  aharlap
// debugging
//
// Revision 1.2  1993/01/08  00:05:16  aharlap
// debugging
//
// Revision 1.1  1993/01/07  04:11:01  mg
// Initial revision
//

    END-LOG---------------------------------
*/

