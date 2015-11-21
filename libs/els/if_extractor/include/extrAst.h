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

#ifndef _extrAst_h
#define _extrAst_h

#ifndef _ifExtractor_h
#include <ifExtractor.h>
#endif

#ifndef CANT_FWD_DECL_NESTED
class ifExtractor::extrAst
#else
class extrAst
#endif
{
public:
  static void *operator new(unsigned int);
  static void  operator delete(void *);

private:
  static const int   blockSize;
  static parray     *freeExtrAsts;
  
public:
  extrAst(void);
  extrAst(astnodeList *);
  ~extrAst(void);
  
public:
  int print(char *, char *, raFile *);
  int resolveLocs(void);
  int walkForSyms(char *, int);

private:
  int resolveLocsAstNode(astnode *);
  int printAstNode(astnode *, char *, raFile *);
  int walkForSyms(astnodeList *, char *, int);
  int outputWrapper(char *, char *, raFile *);

private:
  astnodeList *ast;
};

#endif 
// _extrAst_h

