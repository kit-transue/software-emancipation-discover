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
// binsearch
//------------------------------------------
// synopsis:
// binary search class implementation
//
// description:
// This file provides the implementation of the only non-pure-virtual
// member function in class binsearch, namely, the search algorithm itself.
//------------------------------------------
// Restrictions:
// ...
//------------------------------------------

#ifndef ISO_CPP_HEADERS
#include <stddef.h>
#else /* ISO_CPP_HEADERS */
#include <cstddef>
namespace std {};
using namespace std;
#endif /* ISO_CPP_HEADERS */
#include "genError.h"
#include "binsearch.h"

size_t binsearch_table::find_item(const binsearch_item& target) const {
   Initialize(binsearch_table::find_item);

   long lower = 0;
   long upper = item_count() - 1;
   while (lower <= upper) {
      size_t mid = size_t(lower + (upper - lower) / 2);
      int cmp = item(mid).compare(target);
      if (cmp == 0)	// mid == target
	 ReturnValue(mid);
      if (cmp < 0)	// mid < target
	 lower = mid + 1;
      else upper = mid - 1;	// mid > target
   }
   ReturnValue(size_t(-1));	// not found
}

/*
    START-LOG-------------------------------

    $Log: binsearch.h.cxx  $
    Revision 1.2 2000/07/10 23:08:52EDT ktrans 
    mainline merge from Visual C++ 6/ISO (extensionless) standard header files
Revision 1.2.1.3  1992/11/24  15:02:07  wmm
Fix boundary condition bugs discovered writing add_typesafe_casts.

Revision 1.2.1.2  1992/10/09  19:14:46  builder
*** empty log message ***


    END-LOG---------------------------------
*/

