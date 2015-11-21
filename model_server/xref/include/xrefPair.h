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
#ifndef _xrefPair_h
#define _xrefPair_h

#include <ddKind.h>
#include <xrefSymbol.h>
#include <dd-forward.h>

#define MAX_HASH 431

//   hashPair and xrefPair are used together, to temporarily store all the symbol
//     and link information for a module.  For this purpose, 4 levels are used:
// L1:    hashPair - table of information about the module, hashed by symbol
// L2:        xrefPair - one instance per symbol
//               each instance has src, pointer to an xrefSymbol,
//               and  hp, a pointer to level 3
// L3:        hashPair - one table per symbol.  The table is hashed by destination symbol
// L4:            xrefPair - one instance per link
//                     each instance has "src", pointer to a destination xrefSymbol
//                     and lt, a link type

//  insert_module() builds a single hashPair L1, representing all symbol-link-symbol info
//     Then for each source symbol, it calls  add_links(hp), passing it the L3 hashPair

//  xrefPair is also used to store pairs of symbols, where the two are in different
//     pmods.

class hashPair;
class xrefPair
{
public:
  xrefSymbol* src;
  union {
    xrefSymbol* trg;
    linkType lt;
  };
  hashPair* hp;
  xrefPair* next;

  xrefPair(xrefSymbol* s);
  xrefPair(xrefSymbol* s, linkType t)    { src = s; lt = t;  hp=0; next = 0;}
  xrefPair(xrefSymbol* s, xrefSymbol* t) { src = s; trg = t; hp=0; next = 0;}
  ~xrefPair();
  void set_mark() {hp = (hashPair *) 1;}
  int get_mark() {return (int) hp;}
};

class hashPair {
  xrefPair** table;
  int cur_ind;
  xrefPair* next_xp;
  int table_size;
public:
  int cnt;

  hashPair(int size = MAX_HASH);
  void reset() { cur_ind = -1; next_xp = 0; cnt = 0;}
  xrefPair* get_first(int i = 0);
  xrefPair* get_next(int i = 0);
  xrefPair* insert(xrefSymbol* src, xrefSymbol* trg);        // connect two symbols of diff pmods
  xrefPair* insert(xrefSymbol* src, linkType lt, xrefSymbol* trg);  // represent a link of symbol src
  xrefPair* insert(xrefSymbol* dest, linkType lt);  
  xrefPair* lookup(xrefSymbol* src);
  xrefPair* lookup(xrefSymbol* dest, linkType lt);
  xrefPair* lookup(xrefSymbol* src, linkType lt, xrefSymbol* trg);
  xrefPair* lookup(xrefSymbol* src, xrefSymbol* trg);
  int hash(xrefSymbol*);
  ~hashPair();
  void init();
//  void remove(xrefSymbol* src, linkType lt);

};

class hashElmt
{
public:
  xrefSymbol*  el;
  hashElmt* next;
  hashElmt* prev;
  hashElmt(xrefSymbol* si) { next = 0; prev = 0; el = si;}
};
hashPair *get_remove_hashPair();
extern hashPair *rhp;

#endif
