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
#ifndef _genStringPlus_h
#define _genStringPlus_h
#include <genArr.h>
#include <genString.h>

//////////////////
// The idea is to write an genString that is effient for huge strings (many megs).
// The existing genstring uses strlen() to determine size.
// The extending of the array is also in-efficient.
// genArr(char) solves both of these problems, but lacked += and char*
// for convenience.

// Avoid collisions with genArr(char).
typedef char genStringPlus__char;

genArr(genStringPlus__char);


// Note these string are NOT null terminated.
class genStringPlus : public genArrOf(genStringPlus__char)
{
 public:
  genStringPlus() : genArrOf(genStringPlus__char) () {}
  genStringPlus(int sz) : genArrOf(genStringPlus__char) (sz) {}
  genStringPlus(const char *str) : genArrOf(genStringPlus__char) () 
    { operator+=(str); }

  operator char* () ;
  char& operator [](int ii);
  char  operator [](int ii) const;
  genStringPlus& operator+=(char ch) ;
  genStringPlus& operator+=(genStringPlus &str) ;
  genStringPlus& operator+=(const char *str) ;
  genStringPlus& operator+=(int ii) ;
};
#endif
