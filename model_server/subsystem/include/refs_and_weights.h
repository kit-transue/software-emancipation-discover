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
// /aset/subsystem/include/refs_and_weights.h
//------------------------------------------
// description:
// 
//------------------------------------------
// History:
//
// Wed Jul 19 10:12:30 1995:	Created by William M. Miller
//
//------------------------------------------


#ifndef _refs_and_weights_h
#define _refs_and_weights_h

#include <cLibraryFunctions.h>
#ifndef ISO_CPP_HEADERS
#include <stddef.h>
#else /* ISO_CPP_HEADERS */
#include <cstddef>
using namespace std;
#endif /* ISO_CPP_HEADERS */
      
#ifndef _bit_array_h
#include <bit_array.h>
#endif

#ifndef _objArr_h
#include <objArr.h>
#endif
      
//#include <stdio.h>

struct ref_and_weight {
   unsigned targ_ref : 24;
   unsigned weight : 8;
};


//========================================
// Class refs_and_weights
//	Created Wed Jul 19 10:12:30 1995 by William M. Miller
//----------------------------------------
// Description:
//
// Represents a list of weighted references from this entity to others.      
//========================================

class refs_and_weights {
public:
	refs_and_weights();
	~refs_and_weights();
	void add_ref(size_t targ_ref, unsigned char weight);
	size_t calc_binding(const bit_array& mask);
	void tally_hits(size_t totals[]);
	void reset();
	void dump(FILE*, const objArr&);	void normalize_refs(size_t max_ref);
	// for debugging
private:
	friend class refs_and_weights_iter;
	size_t num_allocated;
	size_t num_refs;
	ref_and_weight* data;
};



//========================================
// Class refs_and_weights_iter
//	Created Wed Jul 19 10:28:48 1995 by William M. Miller
//----------------------------------------
// Description:
//
//========================================

class refs_and_weights_iter {
public:
	refs_and_weights_iter(refs_and_weights& rw);
	ref_and_weight* next();
	void reset();
private:
	refs_and_weights& targ_rw;
	size_t idx;
};

//========================================
// refs_and_weights_iter::next
//----------------------------------------
// Description:
//
//----------------------------------------
// History:
// 
// Wed Jul 19 10:32:26 1995:	Created by William M. Miller
//========================================

inline ref_and_weight* refs_and_weights_iter::next () {
   ref_and_weight* p;
   if (idx >= targ_rw.num_refs) {
      p = NULL;
   }
   else p = &targ_rw.data[idx++];
   return p;
}


#endif
