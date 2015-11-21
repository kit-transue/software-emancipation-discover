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
//-----------------------------------------------------------------------------
// gArray.h
//
// Macro for generating array classes.
//-----------------------------------------------------------------------------

#ifndef _gArray_h
#define _gArray_h

#ifndef _general_h
#include "ggeneral.h"
#endif

#include <psetmem.h>

#define gArrayOf(TYPE) paste(TYPE,Arr)

#define gArray(TYPE)							      \
class gArrayOf (TYPE)							      \
{									      \
  protected:								      \
    int	capacity;						      \
    int	length;							      \
    TYPE*	ptr;							      \
									      \
  public:								      \
    gArrayOf (TYPE) (void) : ptr (0), length (0), capacity (0) {}	      \
    gArrayOf (TYPE) (int sz) : ptr (0), length (0), capacity (0) {	      \
	provide (sz);							      \
    }									      \
    ~gArrayOf (TYPE) (void) { if (ptr) psetfree (ptr); }		      \
									      \
    void reset (void) { length = 0; }					      \
    int size (void) const { return length; }			      \
    TYPE* grow (int ii = 1) {						      \
	int ol = length;						      \
	length += ii;							      \
	provide (length);						      \
	return &ptr[ol];						      \
    }									      \
    TYPE* append (const TYPE& el) {					      \
	TYPE* pp = grow ();						      \
	*pp = el;							      \
	return pp;							      \
    }									      \
    void remove (int ind) { shrink (ind, 1); }				      \
    void shrink (int ind,  int len) {					      \
	int new_length = length - len;					      \
	int ii, jj;							      \
	for (ii = ind, jj = ind + len; ii < new_length; ++ii, ++jj)	      \
	    ptr[ii] = ptr[jj];						      \
	length = new_length;						      \
    }									      \
    TYPE& operator[] (int ii) const {					      \
	return (ii < 0 || ii >= length) ? ptr[length - 1] : ptr[ii]; }		      \
    void provide (int len) {						      \
	if (len > capacity) {						      \
	    if (capacity == 0)						      \
		capacity = 1;						      \
	    while (capacity < len)					      \
		capacity *= 2;						      \
	    ptr = (TYPE*) (ptr ?					      \
			psetrealloc ((char *)ptr, capacity * sizeof (TYPE)) : \
			psetmalloc (capacity * sizeof (TYPE)));		      \
	}								      \
    }									      \
}

#endif // _gArray_h
