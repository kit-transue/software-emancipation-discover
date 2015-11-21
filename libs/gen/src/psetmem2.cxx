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
#include <stdlib.h>

extern "C" char* pset_mem_global_ptr;
extern "C" char* psetmalloc_handler_c(unsigned size);
extern "C" char* psetrealloc_handler_c(char* ptr, unsigned size);
extern "C" char* psetmalloc_handler(unsigned size)
{
    return psetmalloc_handler_c(size);
}

extern "C" char* psetrealloc_handler(char* ptr, unsigned size)
{
    return psetrealloc_handler_c(ptr, size);
}

extern "C" char* psetmalloc(unsigned size) 
{
        return ((pset_mem_global_ptr = (char*)malloc(size)) ? 
	    pset_mem_global_ptr : 
	    psetmalloc_handler(size));

}

extern "C" char* psetcalloc(unsigned nelem, unsigned size) 
{
    return psetmalloc_handler (nelem*size);
}
 
extern "C" void psetfree(void* ptr)
{
  if (ptr) free(ptr);
}

