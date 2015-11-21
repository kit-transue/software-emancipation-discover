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
#ifndef _charstream_h
#define _charstream_h

#ifndef ISO_CPP_HEADERS
#include <iostream.h>
#include <stdlib.h>
#include <strstream.h>
#else /* ISO_CPP_HEADERS */
#include <iostream>
using namespace std;
#include <cstdlib>
#include <sstream>
#endif /* ISO_CPP_HEADERS */

struct charArray
{
  unsigned int capacity;
  char* ptr;

  charArray();
  charArray(int sz);
  char* provide(int);
  ~charArray() {if(ptr)  delete (ptr);}
};

class charArray_buf : public streambuf {
  protected:
    charArray buf;
  public:
    charArray_buf () : buf() {  setbuf(buf.ptr, buf.capacity);}
    void provide(int n);
    int overflow (int ch);
    int underflow ();
    // Defining xsputn is an optional optimization.
    // (streamsize was recently added to ANSI C++, not portable yet.)
    // streamsize xsputn (const char* text, streamsize n);

    void reset() { setp(buf.ptr, buf.ptr + buf.capacity);}
    char * ptr() {return buf.ptr;}  /* returned string still controlled by buf */
    char * str();  /* returned string must be freed manually */
};


class ocharstream : public ostream
{
 public:
  ocharstream() : ostream(new charArray_buf) {}
  charArray_buf *rdbuf() { return (charArray_buf *) ostream::rdbuf(); }
  char* str() { return rdbuf()->str(); }
  void reset() { rdbuf()->reset(); }
  char * ptr() { return rdbuf()->ptr(); }
};

#endif
