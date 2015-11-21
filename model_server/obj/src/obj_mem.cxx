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
#include <cLibraryFunctions.h>
#include <msg.h>
#include <genError.h>
#include <objRelation.h>
#include <proj_clean.h>
#ifndef ISO_CPP_HEADERS
#include <stdio.h>
#else /* ISO_CPP_HEADERS */
#include <cstdio>
using namespace std;
#endif /* ISO_CPP_HEADERS */
#include <objArr_Int.h>

extern objArr_Int * rel_classes;

extern "C" void obj_mem_prt(int min)  // presumably for debugging
{

  class_descriptor * dd;

  objArr_Int & darr = *rel_classes;
  int sz = darr.size();
  int ii;

  for( ii=0; ii<sz; ++ii){
      dd = (class_descriptor *) darr[ii];
      if(dd->unused < min) continue;
      msg("$1 $2\n") << dd->name << eoarg << dd->unused << eom;
  }
}

void * obj_mem_alloc(descriptor dd, size_t sz)
{
    underInitialize(obj_mem_alloc);

    static int clean_flag = 1;

  DBG
  {
      if (clean_flag)
          msg("allocated $1 bytes") << sz << eom;
  }

  ((class_descriptor*)dd)->unused ++;

  char *p = (char *)::operator new(sz);
  if (p)
      return p;

  proj_clean();
  clean_flag++;

  p = (char *)::operator new(sz);

  DBG
  {
      if (!p)
          msg("After clean, still could not allocated") << eom;
      if (clean_flag == 1)
          msg("allocated $1 bytes") << sz << eom;
  }

  return p;
}

void obj_mem_free(descriptor dd, void*ptr)
{
  ((class_descriptor*)dd)->unused --;
  :: operator delete(ptr);
}
