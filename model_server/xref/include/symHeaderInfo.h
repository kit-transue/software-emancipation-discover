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
#ifndef _symHeaderInfo_h
#define _symHeaderInfo_h
#include <sys/types.h>

// This class is obsolete, and should be part of symHeaderInfo.  But
//   lots of code uses it directly, so we preserve the header pointer
//   into it.
typedef unsigned int uint;

class headerInfo
{
public:
  uint table_max; // max size of table
  uint entry_cnt; // number of symbols 
  uint fragment_cnt; // number of deleted symbols 
  uint sym_file_size;// current allocation of sym map file
  uint ind_file_size; // current allocation of index map file
  uint link_file_size; // current allocation of link map file
  uint next_offset_sym;  // next offset in mem. map file to be used
  uint next_offset_ind;  // next offset in ind. map file to be used
  uint next_offset_link; // next offset in .lnk map

  void print();
};

// pmod.sym files begin with this pattern of bytes
//  note that this isn't portable to little-endian machines, since
//  they want to share the exact files.
class symHeaderInfo
{
public:
    long magic;			// Magic number  #SYM
    long version;		// Version of save file format
    int date;			// Date of original header creation
    long checksum;		// Checksum of source file
    uint offset_to_data;	// offset from bof to begin of xrefSymbol's
    uint dummy[4];		// reserve.  32 byte aligned for start of body
    headerInfo  oldheader;      // this is where the old header goes
    
    void insert_time_t(time_t *);
    time_t * extract_time_t(time_t *);
};
#endif
