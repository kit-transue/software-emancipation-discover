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
// This code is a -*- C++ -*- header file

#ifndef _extrSym_h
#define _extrSym_h

#ifndef _ifExtractor_h
#include <ifExtractor.h>
#endif

#ifndef CANT_FWD_DECL_NESTED
class ifExtractor::extrSym
#else
class extrSym
#endif
{
public:
#ifndef CANT_FWD_DECL_NESTED
  class extrSmt;
#else
#include <extrSmt.h>
#endif

public:
  static void *operator new(unsigned int);
  static void  operator delete(void *);

private:
  static const int   blockSize;
  static parray     *freeExtrSyms;
  
public:
  extrSym(void);
  extrSym(unsigned int, symbol *);
  ~extrSym(void);
  
public:
  int   addAtr(attributeList *);
  int   addRel(ifExtractor::extrRel *);
  int   addSmt(int, locationList *);
  int   addSym(symbol *);
  int   isFile(const char *);
  int   print(char *, raFile *);
  int   printable(void);
  int   refsFile(char *);
  int   resolveLocs(void);
  int   setNum(unsigned int);
  int   smtRefsFile(char *);
  int   tag(void);
  char *fromFile(void);

private:
  unsigned int  num;
  symbol       *sym;
  parray       *attrs; // array of attribute ptrs 
  parray       *smts;  // array of extrSmt prts
  parray       *rels;  // pointers to relations that involve this symbol
  int          printflag;
  int          visited;

private:
  int  tagFromRels(void);
};

#endif 
// _extrSym_h

