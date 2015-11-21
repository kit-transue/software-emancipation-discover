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
#include <memory.h>
#include <stdlib.h>

void OSapi_bzero(void *buf, int cnt)
{
	memset(buf, 0, cnt);
}

char * gen_array_provide(char * ptr, int len, int& capacity, int objsize)
{
   size_t old_sz = capacity * objsize;

   if(len > capacity) {
      if(capacity == 0)
	    capacity = 1;
      while(capacity < len)
	    capacity *= 2;
      size_t new_sz = capacity * objsize;
      ptr = (ptr) ? (char *)realloc(ptr, new_sz) : (char *)malloc(new_sz);

      if (ptr && (new_sz > old_sz))
		  OSapi_bzero((ptr + old_sz), new_sz - old_sz);
   }
   return ptr;
}

void gen_arr_free(char*ptr)
{
  if (ptr) free(ptr); 
}			      

void gen_init_handler(const char* name)
{
}

void gen_exit_handler (const char* name)
{
}
