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
#ifndef _db_intern_h
#define _db_intern_h

#include <db.h>
#include <objRelation.h>

//  ---- internal structures -----------

#include "db_decls.h"
#include "db_str_desc.h"


struct dbHeader
{
  unsigned table : 2;
  unsigned ind : 6;
  byte language;
  char title[1];  // contains \0 at the end
};

#define bit(x) unsigned x : 1
#define bits(x,i) unsigned x : i

struct dbRecDesc
{
  bit(is_node);  // =1
  bits(dh, 2);   // data_header length : 2, 2a, 3a, 6 bytes
  bit(is_leaf);
  bit(is_last);
  bits(flag,3);  //
};

struct dbRecHeader2 /* : public dbRecDesc */
{
    // This is looks perferct for all architecures only 8 bits are used
    bits(extype, 8);
};

struct dbRecHeader2a /* : public dbRecDesc  // ctrl->flags */
{                 
    // This is looks perferct for all architecures only 8 bits are used
    bits(extype, 5);
    bits(rec_len,3);
};



struct dbRecHeader6 /* : public dbRecDesc  // 6 bytes */
{
  bits(rec_len,24);
  bits(extype , 8);
  bits(type, 8); // 
};

enum dbRel_format { R3_15_7, R3_16_6, R3_14_8, R4_22_8 };



enum db_app_format { F2, F2a, F3b, F6};

class db_intern_buffer : public db_buffer
{
 public:
  int iput(db_app&);
  int put_rel(int rel_id, int node_id, int direction);
  int iget(db_app&);
  int get_rel(int& lel_id, int& trg_id, int& direction);
  unsigned is_node ();
  unsigned is_relation ();

  db_intern_buffer ();
  int no_F2;
  int no_F2a;
  int no_F3b;
  int no_F6;
  int no_rel;
};

inline db_intern_buffer::db_intern_buffer ()
     : db_buffer (), no_F2 (0), no_F2a (0), no_F3b (0), no_F6 (0), no_rel (0)
{}

inline unsigned db_intern_buffer::is_node ()
{
    return get_position () < size () && ((*get_cur ()) & 0x80) != 0;
}

inline unsigned db_intern_buffer::is_relation ()
{
    return get_position () < size () && ((*get_cur ()) & 0x80) == 0;
}

// READ ONLY
class db_buffer_ro : public db_intern_buffer {
  public:
    db_buffer_ro (void* addr, int size);
    ~db_buffer_ro () { ptr = 0; }
};

inline db_buffer_ro::db_buffer_ro (void* addr, int size) : db_intern_buffer ()
{
    ptr = (byte*)addr;
    capacity = size;
    length = size;
}

struct db_def
{
    class_descriptor * head;	// Address of header descriptor
    class_descriptor * node;	// Address of node descriptor
    db_save_node       nd_save;
    db_restore_node    nd_restore;
    db_restore_header  hd_restore;
    db_after_rest      after_rest;
    db_after_rest_head after_rest_head;
    RelType ** rel;		// Type of relation to root
    char *suff;		// Suffix for text file  NULL means no text
    int incremental;            // if set don't restore for first pass
    int dont_save_unchanged_src;
};

struct db_tables
{
    db_def* def;
    unsigned size;
};

extern db_tables 	db_def_tables [];
extern int 		db_def_tables_size;

extern char const *db_rel_table[];
extern int db_rel_table_size;

struct dbRootBuf
{
app * header;
objTree*root;
db_def * def;
db_buffer  *  app_buf;
db_intern_buffer * int_buf;
int root_id;
int last_id;
};

genArr(dbRootBuf);

void db_save_rels(db_intern_buffer& rp, appTree * node);
void db_save_tree(dbRootBuf *rp, objTree * r);
void db_save_root(dbRootBuf *rp);
int  db_save(app *h, char const *fname);
int  db_write_bufs(dbRootBufArr&, char const *);
void db_get_roots(app*, dbRootBufArr&);


void db_restore_roots(app * header, dbRootBufArr& roots, dbRel_format);
void db_restore_roots_complete (app * header, dbRootBufArr& roots, dbRel_format);
objTree * db_restore_tree(app* headr, objTree * parent, dbRootBuf* rp);
app* db_read_bufs (char const *fname, char const *lname, app* app_header = 0);
void db_put_rels ();
void db_init_id(app*);
void db_end_id (app*, dbRootBufArr&);
int db_init_all();
dbRel_format db_get_rel_format ();
dbRel_format db_calculate_rel_format (dbRootBufArr&);
void db_set_rel_format (dbRel_format);
int db_put_id(objTree*node);
int db_put_id_complete(objTree*node);
void db_store_rel(int rel_id, int src_id, int trg_id, int direction );
int db_get_cur_id ();
void db_skip_id (int);

static const int db_start_id = 13;

int db_get_bit_need_reparse_no_forgive(char const *fname);
int db_get_bit_need_reparse (char const *fname);
void db_set_bit_need_reparse (char const *fname, int need_reparse_bit);
int db_get_bit_forgive(char const *fname);
int db_get_db_format(char const *fname);


int db_get_pset_need_reparse_no_forgive(char const *pset_name);
//int db_get_pset_need_reparse (char const *pset_name);
void db_set_pset_need_reparse (char const *pset_name, int need_reparse_bit);
int db_get_pset_forgive(char const *pset_name);
int db_get_pset_format(char const *pset_name);
int db_get_pset_written (char const *fname);
void db_set_pset_written (char const *in_name, int written);

int db_read_src_from_pset(char const *pset_fname, char const *src_name = 0);
extern bool db_change_to_writable(char const *fname);
#endif // _db_intern_h
