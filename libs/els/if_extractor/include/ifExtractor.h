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

#ifndef _ifExtractor_h
#define _ifExtractor_h

class  astnodeList;
class  attributeList;
class  locationList;
class  uintList;
class  parray;
class  position;
class  raFile;
struct astnode;
struct attribute;
struct location;
struct relation;
struct symbol;

class  ifExtractor
{
public:

  // Constructor(s) / Destructor(s)
  ifExtractor(const char *);
  virtual ~ifExtractor(void);

  // Debugging interface
  static void showMapTable(int = 1);
  static void showProgress(int = 1);

  // Public Interface
  int extract(parray *, parray *);

public:

#ifdef CANT_FWD_DECL_NESTED
  // needed for IRIX & HP700 as extrRel refs extrSym
  class extrSym;
#endif
  
#ifndef CANT_FWD_DECL_NESTED
  class extrRel;
#else
#include <extrRel.h>
#endif

#ifndef CANT_FWD_DECL_NESTED
  class extrSym;
#else
#include <extrSym.h>
#endif

#ifndef CANT_FWD_DECL_NESTED
  class extrKeyword;
#else
#include <extrKeyword.h>
#endif

#ifndef CANT_FWD_DECL_NESTED
  class extrErr;
#else
#include <extrErr.h>
#endif

#ifndef CANT_FWD_DECL_NESTED
  class extrAst;
#else
#include <extrAst.h>
#endif

public:

  // Static interface
  static int      addSym(unsigned int, symbol *);
  static int      addSmt(unsigned int, int, locationList *);
  static int      addAtr(unsigned int, attributeList *);
  static int      addErr(char *, location *);
  static int      addAst(astnodeList *);
  static int      addKeyword(char *, locationList *);
  static int      addRel(relation *);
  static int      addAstRef(unsigned int, char *);
  static extrSym *getSymPtr(unsigned int);


protected:
  static ifExtractor *instance;

private:
  parray *syms; // array of extrSym
  parray *asts; // array of extrAst
  parray *rels; // array of extrRel
  parray *errs; // array of extrErr
  parray *kwds; // array of extrKeyword

  char   *if_file;

  static int showmaptable;
  static int showprogress;

private:
  extrSym *getSymFile(const char *);

  int doParse(char *);
  int collectOutput(char *);
  int writeOutput(char *, raFile *);
  int resolveLocs(void);
  int walkSymsFor(char *);
  int walkKeywordsFor(char *);
  int walkErrorsFor(char *);
  int walkAllAstsForSyms(char *);
  int dumpSyms(char *, raFile *);
  int dumpErrors(char *, raFile *);
  int dumpKeywords(char *, raFile *);
  int dumpAsts(char *, char *, raFile *);
  int reset(void);
};

#endif 
// _ifExtractor_h

