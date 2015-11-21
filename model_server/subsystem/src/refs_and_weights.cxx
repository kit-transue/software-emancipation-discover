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
// /aset/subsystem/src/refs_and_weights.C
//------------------------------------------
// description:
// 
//------------------------------------------
// History:
//
// Wed Jul 19 10:12:30 1995:	Created by William M. Miller
//
//------------------------------------------
//#include <stdio.h>
#include <cLibraryFunctions.h>
#include "genError.h"


#include <Entity.h>

#ifndef __psetmem_h
#include <psetmem.h>
#endif

#ifndef AUTOSUBSYS_WEIGHTS_H
#include <autosubsys-weights.h>
#endif


//****************************************
// Implementations for class refs_and_weights
//****************************************



//========================================
// refs_and_weights::refs_and_weights
//----------------------------------------
// Description:
//
//----------------------------------------
// History:
// 
// Wed Jul 19 10:14:07 1995:	Created by William M. Miller
//========================================

refs_and_weights::refs_and_weights(): num_allocated(32), num_refs(0),
      data((ref_and_weight*) psetmalloc(32 * sizeof(ref_and_weight))) { }


//========================================
// refs_and_weights::~refs_and_weights
//----------------------------------------
// Description:
//
//----------------------------------------
// History:
// 
// Wed Jul 19 10:14:21 1995:	Created by William M. Miller
//========================================

refs_and_weights::~refs_and_weights () {
   Initialize(refs_and_weights::~refs_and_weights);

   psetfree(data);
}


//========================================
// refs_and_weights::add_ref
//----------------------------------------
// Description:
//
//----------------------------------------
// History:
// 
// Wed Jul 19 10:14:46 1995:	Created by William M. Miller
//========================================

void refs_and_weights::add_ref (size_t targ_ref, unsigned char weight) {
   Initialize(refs_and_weights::add_ref);

   if (num_refs >= num_allocated) {
      num_allocated += 32;
      char* p;
      Assert(p = psetrealloc((char*) data, num_allocated * sizeof(ref_and_weight)));
      data = (ref_and_weight*) p;
   }
   data[num_refs].targ_ref = targ_ref;
   data[num_refs].weight = weight;
   num_refs++;
}


//========================================
// refs_and_weights_iter::refs_and_weights_iter
//----------------------------------------
// Description:
//
// Iterator for elements of class refs_and_weights.
//----------------------------------------
// History:
// 
// Wed Jul 19 10:31:20 1995:	Created by William M. Miller
//========================================

refs_and_weights_iter::refs_and_weights_iter (refs_and_weights& rw): targ_rw(rw),
      idx(0) { }



//------------------------------------------
// refs_and_weights::dump
//------------------------------------------

void refs_and_weights::dump(FILE* f, const objArr& entities) {
   Initialize(refs_and_weights::dump);

   refs_and_weights_iter it(*this);
   ref_and_weight* ref;
   while (ref = it.next()) {
      OSapi_fprintf(f, "\t%2d:%4d %s\n", ref->weight,
	      defaultweights[ref->weight].m_weight,
	      EntityPtr(entities[ref->targ_ref])->get_name());
   }
   OSapi_fprintf(f, "\n");
}
