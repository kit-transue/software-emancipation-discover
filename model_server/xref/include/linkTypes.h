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
#ifndef _linkTypes_h
#define _linkTypes_h

#include <Relational.h>
#include <ddKind.h>

class linkTypes
{
public:
  linkTypes();
  int contain(linkType lt);
  int contains(linkType, ...);
  int operator == (linkType lt);
  linkTypes & operator = (linkType lt);
  operator linkType();
  void found(linkType lt);
  void remove(linkType lt);
  void add(linkType lt);
  int local_only();
  static int init_local_search();
  enum {MAX_LINKS = 128};
  static char local_search[MAX_LINKS];
  static char search_from[MAX_LINKS];
  int get_link(int i);
  void reset(linkTypes &la);
private:
  char links[MAX_LINKS];
  char find[MAX_LINKS];
};

enum SearchType {
  FROM_DEF_SYMBOLS,
  FROM_ALL_SYMBOLS,
  FROM_ANY_SYMBOLS
};

inline int linkTypes::get_link(int i) { return links[i];}
inline int linkTypes::contain(linkType lt) { return links[lt];}
inline int linkTypes::operator==(linkType lt) { return links[lt];}
inline linkTypes& linkTypes::operator=(linkType lt) { links[lt] = 1; return *this;}
inline void linkTypes::remove(linkType lt) { links[lt] = 0;}
inline void linkTypes::add(linkType lt) { links[lt] = lt;}
inline void linkTypes::found(linkType lt) { find[lt] = 1;}
#ifdef XXX_rawbuf
struct rawbuf
{
    int size;
    char* ptr;
    rawbuf() : size(0), ptr (0) {}
};
#endif
#endif

