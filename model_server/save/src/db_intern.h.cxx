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
#include <string.h>
#else /* ISO_CPP_HEADERS */
#include <cstring>
namespace std {};
using namespace std;
#endif /* ISO_CPP_HEADERS */
#include <objOper.h>
#include <genError.h>
#include <db_intern.h>
#include <machdep.h> 
#ifndef ISO_CPP_HEADERS
#include <stdio.h>
#else /* ISO_CPP_HEADERS */
#include <cstdio>
#endif /* ISO_CPP_HEADERS */
#include "db_save_str.h"
#include "db_str_desc.h"

#include "db_read_str.h"
#include "db_str_desc.h"


static dbRel_format db_rel_format = R4_22_8;

dbRel_format db_get_rel_format ()
{
    return db_rel_format;
}

void db_set_rel_format (dbRel_format fmt)
{
    db_rel_format = fmt;
}

union db_cast {
    struct {
	unsigned type : 8;
	unsigned flag : 1;
    } nd;
   
    struct {
	unsigned type : 6;
	unsigned flag : 3;
    } db;
};

void db_app::clean()
{
  memset(this, '\0', sizeof(db_app));
}

db_app::db_app(objTree*n)
{
  clean();
  is_leaf = ! n->get_first();
  is_last = ! n->get_next();
  is_node = 1;
}

#ifdef DBA_DEBUG
extern "C" void print_dba(db_app& dba)
{
    printf("======dba=====\n");
    printf("is_node : %x\n", dba.is_node);
    printf("dh      : %x\n", dba.dh);
    printf("is_leaf : %x\n", dba.is_leaf);
    printf("is_last : %x\n", dba.is_last);
    printf("flag    : %x\n", dba.flag);
    printf("val     : %x\n", dba.val);
    printf("extype  : %x\n", dba.extype);
    printf("type    : %x\n", dba.type);
    printf("==============\n");
    fflush(stdout);
}
#else
extern "C" void print_dba(db_app& dba) {}
#endif

int db_intern_buffer::iput(db_app& dba)
{
    int no_bytes;
    db_cast cast;

    if (dba.type == 0 && dba.val == 0) {
	dba.set_dh(F2);
	no_bytes      = 2;     
	SAVEABLE_STRUCT_NAME(db_app_F2) tmp;
	tmp.is_node = dba.is_node;
	tmp.dh      = dba.dh;
	tmp.is_leaf = dba.is_leaf;
	tmp.is_last = dba.is_last;
	tmp.flag    = dba.flag;
	tmp.extype  = dba.extype;
	StructSaver tmp_res(&tmp, STRUCT_SAVE_PARAMS(db_app_F2));
	byte* pointer = (byte*)tmp_res.get_buffer();
        put_byte (pointer[0]);
	put_byte (pointer[1]);
	no_F2++;
    } else if (dba.flag < 2 && dba.type < 4 
	       && dba.extype < 32 && dba.val < 8) {
	dba.set_dh (F2a);
	no_bytes        = 2;
	int packed_flag = ((dba.type << 1) & 6) | (dba.flag & 1);
	dba.flag        = packed_flag;
	SAVEABLE_STRUCT_NAME(db_app_F2) tmp_f2;
	tmp_f2.is_node = dba.is_node;
	tmp_f2.dh      = dba.dh;
	tmp_f2.is_leaf = dba.is_leaf;
	tmp_f2.is_last = dba.is_last;
	tmp_f2.flag    = dba.flag;
	StructSaver tmp_f2_res(&tmp_f2, STRUCT_SAVE_PARAMS(db_app_F2));
	byte* pointer = (byte*)tmp_f2_res.get_buffer();
	put_byte (pointer[0]);
	SAVEABLE_STRUCT_NAME(db_rec_header2a) tmp2a;
        tmp2a.extype  = dba.extype;
	tmp2a.rec_len = dba.val;
	StructSaver tmp2a_res(&tmp2a, STRUCT_SAVE_PARAMS(db_rec_header2a));
	pointer = (byte*)tmp2a_res.get_buffer();
	put_byte (pointer[0]);
	no_F2a++;
    } else if (dba.flag < 2 && dba.extype < 32 
	       && dba.type < 64 && dba.val < 128) {
	dba.set_dh (F3b);
	no_bytes      = 3;
	dba.flag      = ((dba.type << 1) & 6) | (dba.flag & 1);
	dba.type      = dba.type >> 2;
	SAVEABLE_STRUCT_NAME(db_app_F2) tmp_f2;
	tmp_f2.is_node = dba.is_node;
	tmp_f2.dh      = dba.dh;
	tmp_f2.is_leaf = dba.is_leaf;
	tmp_f2.is_last = dba.is_last;
	tmp_f2.flag    = dba.flag;
	StructSaver tmp_f2_res(&tmp_f2, STRUCT_SAVE_PARAMS(db_app_F2));
	byte* pointer = (byte*)tmp_f2_res.get_buffer();
	put_byte (pointer[0]);
	SAVEABLE_STRUCT_NAME(db_str_RecHeader3b) tmp;
	tmp.extype  = dba.extype;
	tmp.type    = dba.type;
	tmp.rec_len = dba.val;
	StructSaver tmp_save(&tmp, STRUCT_SAVE_PARAMS(db_str_RecHeader3b));
	put (tmp_save.get_buffer(), 2);
	no_F3b++;
    }  else {
        dba.set_dh (F6);
	no_bytes = 6;      
	SAVEABLE_STRUCT_NAME(db_str_app) tmp;
	OSapi_bcopy(&dba, &tmp, sizeof(tmp));
	tmp.val    = dba.val;
	tmp.extype = dba.extype;
	tmp.type   = dba.type;
	StructSaver tmp_save(&tmp, STRUCT_SAVE_PARAMS(db_str_app));
	put (tmp_save.get_buffer(), no_bytes);
	no_F6++;
    }
    print_dba(dba);
    return no_bytes; 
}

int db_intern_buffer::put_rel(int rel_id, int node_id, int direction)
{
    int no_bytes;

    switch (db_rel_format) {

      case R3_14_8: {
	  no_bytes = 3;
	  SAVEABLE_STRUCT_NAME(db_str_Rel3_14_8) dbr;
	  dbr.is_node   = 0;
	  dbr.node_id   = node_id;
	  dbr.rel_id    = rel_id;
	  dbr.direction = direction;
	  StructSaver dbr_buf(&dbr, STRUCT_SAVE_PARAMS(db_str_Rel3_14_8));
	  put (dbr_buf.get_buffer(), no_bytes);
	  break;
      }
      
      case R3_15_7: {
	  no_bytes = 3;
	  SAVEABLE_STRUCT_NAME(db_str_Rel3_15_7) dbr;
	  dbr.is_node   = 0;
	  dbr.node_id   = node_id;;
	  dbr.rel_id    = rel_id;
	  dbr.direction = direction;
	  StructSaver dbr_buf(&dbr, STRUCT_SAVE_PARAMS(db_str_Rel3_15_7));
	  put (dbr_buf.get_buffer(), no_bytes);
	  break;
      }
      
      case R3_16_6: {
	  no_bytes = 3;
	  SAVEABLE_STRUCT_NAME(db_str_Rel3_16_6) dbr;
	  dbr.is_node   = 0;
	  dbr.node_id   = node_id;
	  dbr.rel_id    = rel_id;
	  dbr.direction = direction;
	  StructSaver dbr_buf(&dbr, STRUCT_SAVE_PARAMS(db_str_Rel3_16_6));
	  put (dbr_buf.get_buffer(), no_bytes);
	  break;
      }
      
      case R4_22_8: {
	  no_bytes    = 4;
	  SAVEABLE_STRUCT_NAME(db_str_Rel4_22_8) dbr;
	  dbr.is_node   = 0;
	  dbr.node_id   = node_id;
	  dbr.rel_id    = rel_id;
	  dbr.direction = direction;
	  StructSaver dbr_buf(&dbr, STRUCT_SAVE_PARAMS(db_str_Rel4_22_8));
	  put (dbr_buf.get_buffer(), no_bytes);
	  break;
      }
    }
    no_rel++;
    return no_bytes;
}

int db_intern_buffer::iget(db_app& dba)
{
    db_cast cast;
    byte* dest = (byte*) &dba;
    SAVEABLE_STRUCT_NAME(db_app_header) tmp_header;
    SAVEABLE_STRUCT_NAME(db_app_header) tmp_header_buf;
    get(&tmp_header_buf, 1);
    StructReader::Read(&tmp_header_buf, &tmp_header, STRUCT_READ_FUNCTION(db_app_header));
    dba.is_node = tmp_header.is_node;
    dba.dh      = tmp_header.dh;
    dba.is_leaf = tmp_header.is_leaf;
    dba.is_last = tmp_header.is_last;
    dba.flag    = tmp_header.flag;
    switch (dba.dh) {
      case F2:
      {
	  byte tmp_byte;
	  get_byte(tmp_byte);
	  dba.extype = tmp_byte;
	  dba.val    = 0;
	  dba.type   = 0;
	  break;
      }
      case F2a:
      {
	  SAVEABLE_STRUCT_NAME(db_rec_header2a) rec2a;
	  SAVEABLE_STRUCT_NAME(db_rec_header2a) tmp;
	  byte* pointer = (byte*)&tmp;
	  get_byte (pointer[0]);
	  StructReader::Read(&tmp, &rec2a, STRUCT_READ_FUNCTION(db_rec_header2a));
	  dba.extype   = rec2a.extype;
	  dba.val      = rec2a.rec_len;
	  dba.type     = dba.flag >> 1; // Get 2 low bits of type 
	  dba.flag     = dba.flag & 1;
	  break;
      }
      case F3b:
      {
	  SAVEABLE_STRUCT_NAME(db_str_RecHeader3b) tmp1;
	  SAVEABLE_STRUCT_NAME(db_str_RecHeader3b) tmp_buf;
	  get(&tmp_buf, 2);
	  StructReader::Read(&tmp_buf, &tmp1, STRUCT_READ_FUNCTION(db_str_RecHeader3b));
	  dba.extype   = tmp1.extype;
	  dba.val      = tmp1.rec_len;
	  dba.type     = (tmp1.type << 2) | (dba.flag >> 1);
	  dba.flag     = dba.flag & 1;
	  break;
      }
      case F6:
      default:
      {
	  SAVEABLE_STRUCT_NAME(db_str_app_part) tmp;
	  SAVEABLE_STRUCT_NAME(db_str_app_part) tmp_buf;
	  get (&tmp_buf, 5);
	  StructReader::Read(&tmp_buf, &tmp, STRUCT_READ_FUNCTION(db_str_app_part));
	  dba.val    = tmp.val;
	  dba.extype = tmp.extype;
	  dba.type   = tmp.type;
	  break;
      }
    }
    print_dba(dba);
    return 1;
}

int db_intern_buffer::get_rel (int& rel_id, int& node_id, int& direction)
{
    switch (db_rel_format) {

      case R3_14_8: {
	  SAVEABLE_STRUCT_NAME(db_str_Rel3_14_8) dbr;
	  SAVEABLE_STRUCT_NAME(db_str_Rel3_14_8) dbr_buf;
	  get (&dbr_buf, 3);
	  StructReader::Read(&dbr_buf, &dbr, STRUCT_READ_FUNCTION(db_str_Rel3_14_8));
	  node_id   = dbr.node_id;
	  rel_id    = dbr.rel_id;
	  direction = dbr.direction;
	  break;
      }
      
      case R3_15_7: {
	  SAVEABLE_STRUCT_NAME(db_str_Rel3_15_7) dbr;
	  SAVEABLE_STRUCT_NAME(db_str_Rel3_15_7) dbr_buf;
	  get (&dbr_buf, 3);
	  StructReader::Read(&dbr_buf, &dbr, STRUCT_READ_FUNCTION(db_str_Rel3_15_7));
	  node_id   = dbr.node_id;
	  rel_id    = dbr.rel_id;
	  direction = dbr.direction;
	  break;
      }
      
      case R3_16_6: {
	  SAVEABLE_STRUCT_NAME(db_str_Rel3_16_6) dbr;
	  SAVEABLE_STRUCT_NAME(db_str_Rel3_16_6) dbr_buf;
	  get (&dbr_buf, 3);
	  StructReader::Read(&dbr_buf, &dbr, STRUCT_READ_FUNCTION(db_str_Rel3_16_6));
	  node_id   = dbr.node_id;
	  rel_id    = dbr.rel_id;
	  direction = dbr.direction;
	  break;
      }
      
      case R4_22_8: {
	  SAVEABLE_STRUCT_NAME(db_str_Rel4_22_8) dbr;
	  SAVEABLE_STRUCT_NAME(db_str_Rel4_22_8) dbr_buf;
	  get (&dbr_buf, 4);
	  StructReader::Read(&dbr_buf, &dbr, STRUCT_READ_FUNCTION(db_str_Rel4_22_8));
	  node_id   = dbr.node_id;
	  rel_id    = dbr.rel_id;
	  direction = dbr.direction;
	  break;
      }
    }
  return 1;
}
