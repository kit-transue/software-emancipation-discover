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
#ifndef _simplify_globals_h
#define _simplify_globals_h

#include <vector>
#include <map>

#include <string.h>
#include <stdlib.h>

#include "attributeList.h"
#include "maplineStruct.h"
#include "uintList.h"

class InclusionTree;

class SymLine
{
 public:
  SymLine(int sym = -1, int location = -1) : id(sym), line(location) {}
  int id;
  int line;
};

class SymLoc
{
 public:
 SymLoc(int sym, int l, const char* f) : id(sym), line(l), file(0) 
        { file = strdup(f); }
    
  ~SymLoc() { free(file); file = 0; }
  int id;
  int line;
  char* file;
};

class Relation
{
 public:
  Relation(int s, int t) : source(s), target(t), preinclude(0) {}
  int   source;
  int   target;
  int   preinclude;
};

InclusionTree* GetInclusionTree();
class symbol;
std::map<int, symbol *> &GetSymbols();
std::vector<char *> &GetIncludeDirs();
std::map<int, attributeList *> &GetAttributes();
std::vector<Relation *> &GetIncludeRels();
std::vector<mapline *> &GetMapLines();
std::vector<SymLine *> &GetUses();
std::map<int, unsigned int> &GetDefineRels();
std::map<int, uintList *> &GetDeclarations();
std::vector<SymLoc *> &GetCppUses();
std::vector<int> &GetIncludeLines();


void FileMove(const char* f1, const char* f2);
void Error(int type, const char* msg = 0);
enum {NO_ERRORS = 0, NO_CHANGES, OUT_OF_MEMORY, CANNOT_OPEN_IFF, WRONG_COMMAND_LINE, FILE_DOESNOT_EXIST};
const char* GetSourceName();
extern unsigned int source_id;

#ifdef _WIN32
#define popen _popen
#define pclose _pclose
#endif

#endif










