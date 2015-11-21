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
// /aset/subsystem/include/subsys_decomposition.h
//------------------------------------------
// description:
// 
//------------------------------------------
// History:
//
// Wed Jul 19 13:31:42 1995:	Created by William M. Miller
//
//------------------------------------------


#ifndef _subsys_decomposition_h
#define _subsys_decomposition_h

#ifndef ISO_CPP_HEADERS
#include <stddef.h>
#include <stdio.h>
#else /* ISO_CPP_HEADERS */
#include <cstddef>
using namespace std;
#include <cstdio>
#endif /* ISO_CPP_HEADERS */
      
#ifndef _bit_array_h
#include <bit_array.h>
#endif
      
class Entity;  
class groupHdr;
typedef groupHdr* groupHdrPtr;
class RTLNode;

//========================================
// Class proto_subsys
//	Created Wed Jul 19 13:31:42 1995 by William M. Miller
//----------------------------------------
// Description:
//
//========================================

      
#include "objArr.h"

class proto_subsys: public objArr {
public:
	proto_subsys(size_t subsys_no);
	size_t calc_binding(Entity*);
	void recalc_ref_flags(size_t recalc_factor);
	static void entity_count(size_t cnt) { num_entities = cnt; }
	static void subsys_count(size_t cnt) { num_subsys = cnt; }
	void dump_ref_flags(FILE* fp, const objArr& entities);
	void set_subsystem(groupHdrPtr subsys);
private:
	bit_array ref_flags;
	static size_t num_entities;
	static size_t num_subsys;
	groupHdrPtr sub;
};



//========================================
// Class subsys_decomposition
//	Created Wed Jul 19 15:17:33 1995 by William M. Miller
//----------------------------------------
// Description:
//
//========================================

class subsys_decomposition{
public:
	subsys_decomposition(const objArr& entities, size_t num_subsys, size_t recalc_factor);
	size_t iterate();
	void create_subsystems(groupHdrPtr parent, RTLNode* subsys_rtl);
	~subsys_decomposition();
	void dump_ref_flags(const char* filename, const objArr& entities);
private:
	size_t n_subsys;
	proto_subsys** proto;
	size_t recalc_f;
	size_t find_best_proto(Entity* e, size_t cur_proto);
};

#endif

