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
#ifndef ISO_CPP_HEADERS
#include <stdio.h>
#include <memory.h>
#else /* ISO_CPP_HEADERS */
#include <cstdio>
namespace std {};
using namespace std;
#include <memory>
#endif /* ISO_CPP_HEADERS */
#include <charstream.h>

const int DEF_SIZE = 15;
charArray::charArray() : capacity(DEF_SIZE), ptr( new char[DEF_SIZE+1]) {}
charArray::charArray(int sz) : capacity(sz), ptr( new char[sz+1]) {}

char* charArray::provide(int n)
{ 
  if (n > capacity) {
    int oldcap = capacity;
    do
      capacity *= 2; 
    while(capacity < n);

    char*newptr = new char[capacity + 1];
    memcpy(newptr, ptr, oldcap);
  
    delete ptr;
    ptr = newptr;
  }
  return ptr;
}


void charArray_buf::provide  (int n)
{
  int used_sz = pptr () - pbase ();

  int oldsize = buf.capacity;
  char* oldptr = buf.ptr;

  char * ppp = buf.provide(n);

  int newsize = buf.capacity;

  setp(ppp, ppp+newsize);
  pbump(used_sz);
}

int charArray_buf::underflow()
{
  return 0;
}

int charArray_buf::overflow (int ch)
{ 
 
 if(ch == EOF){
   sync();
   return 0;
 }
  provide(buf.capacity + 1);

  sputc(ch);

  return 0;

}

char* charArray_buf::str() 
{ 
   char*ppp = buf.ptr;
   buf.ptr = 0;
   buf.capacity = 0;
   return ppp;
}


#if 0  /* 000713 Adjustment of cerr/cout is galaxy artifact.  Don't do it no more. */
class fwritestream : public ocharstream
{
 protected:
    FILE* ff;
    int sync ();
 public:
    fwritestream(FILE* x) : ff(x){}
};

static fwritestream wout(stdout);
static fwritestream werr(stderr);
#endif

void OSapi_cout_and_cerr()
{
#if 0  /* 000713 Adjustment of cerr/cout is galaxy artifact.  Don't do it no more. */
#ifndef sun5
  cout = wout;
  cerr = werr;
#else
  cout.rdbuf(wout.rdbuf());
  cerr.rdbuf(werr.rdbuf());
#endif
#endif
}

