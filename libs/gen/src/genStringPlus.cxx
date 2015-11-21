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
#include <genStringPlus.h>
#include <genString.h>

char& genStringPlus::operator [] (int ii) 
{
  return *genArrOf(genStringPlus__char)::operator [] (ii);
}

char genStringPlus::operator [] (int ii) const
{
  return *genArrOf(genStringPlus__char)::operator [] (ii);
}

genStringPlus::operator char* () 
{
  // Make sure string is null terminated, but length is not altered.
  provide(length+1);
  *(ptr+length) = 0;
  return ptr;
}

genStringPlus& genStringPlus::operator+=(char ch) 
{
  append(&ch);
  return *this;
}

genStringPlus& genStringPlus::operator+=(genStringPlus &str) 
{
  char* pp = grow(str.size());
  memcpy(pp, (char*)str, str.size());
  return *this;
}

genStringPlus& genStringPlus::operator+=(const char *str) 
{
  int slen = strlen(str);
  char* pp = grow(slen);
  memcpy(pp, str, slen);
  return *this;
}

genStringPlus& genStringPlus::operator+=(int ii) 
{
  genString buf;
  buf.printf("%d", ii );
  operator += ((char *)buf);
  return *this;
}
