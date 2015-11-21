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
#ifndef _sharedArr_h
#define _sharedArr_h

#ifndef _general_h
#include "general.h"
#endif
#ifndef ISO_CPP_HEADERS
#include <memory.h>
#else /* ISO_CPP_HEADERS */
#include <memory>
using namespace std;
#endif /* ISO_CPP_HEADERS */

extern "C" char * shared_array_provide(char * ptr, int len, int& capacity, int objsize);
extern "C" void   shared_arr_free(char*ptr);

//extern "C" char* ast_shared_malloc (int);
//extern "C" char* ast_shared_realloc (void*, int);
//extern "C" void ast_shared_free (void*);

#define sharedArrOf(TYPE)paste(TYPE,SharedArr)

#define sharedArr(TYPE)							      \
class sharedArrOf(TYPE)							      \
{									      \
  protected:								      \
    int capacity;						      \
    int length;						      \
    TYPE *   ptr;							      \
									      \
  public:								      \
 sharedArrOf(TYPE)() : capacity(0), length(0), ptr(0) {}     \
    sharedArrOf(TYPE)(int sz) : capacity(0), length(0), ptr(0) {		      \
	provide(sz);							      \
    }									      \
    ~sharedArrOf(TYPE)() { shared_arr_free((char*)ptr);}                            \
									      \
    void reset() { length = 0; }					      \
    unsigned size() const { return length; }				      \
    TYPE * grow(int ii = 1) {						      \
	int ol = length;						      \
	length += ii;							      \
	provide(length);						      \
	return ptr + ol;						      \
    }									      \
    TYPE * append(TYPE * el) {						      \
	TYPE * pp = grow(1);						      \
	memset((char*)pp, 0, sizeof(TYPE));				      \
	*pp = *el;							      \
	return pp;							      \
    }									      \
    void remove (int ind) { shrink(ind, 1); }				      \
    void shrink(int ind,  int len) {					      \
	int new_length = length - len;					      \
	int ii, jj;							      \
	for(ii=ind, jj=ind+len; ii<new_length; ++ii, ++jj)		      \
	    ptr[ii] = ptr[jj];						      \
	length = new_length;						      \
    }									      \
    TYPE* operator[](int ii) const {					      \
      return (ii < 0 || ii >= length) ? 0 : ptr + ii; }		              \
    void provide(int len) {						      \
      if(len > capacity)                                                      \
	ptr = (TYPE *) shared_array_provide((char*)ptr, len, capacity, sizeof(TYPE));   \
    }									      \
}

#endif // _sharedArr_h
