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
#include <simplify_globals.h>

#include <InclusionTree.h>

using std::vector;
using std::map;

const int SizeChange = 16384;
const int SmallSizeChange = 256;

InclusionTree* GetInclusionTree()
{
    static InclusionTree tree;
    return &tree;
}

map<int, symbol *> &GetSymbols()
{
    static map<int, symbol *> symbols;
    return symbols;
}

map<int, attributeList *> &GetAttributes()
{
    static map<int, attributeList *> attributes;
    return attributes;
}

vector<mapline *> &GetMapLines()
{
    static vector<mapline *> maplines;
    return maplines;
}

vector<Relation *> &GetIncludeRels()
{
    static vector<Relation *> IncludeRels;
    return IncludeRels;
}

map<int, uintList *> &GetDeclarations()
{
    static map<int, uintList *> declarations;
    return declarations;
}

vector<SymLoc *> &GetCppUses()
{
    static vector<SymLoc *> cppuses;
    return cppuses;
}

vector<SymLine *> &GetUses()
{
    static vector<SymLine *> uses;
    return uses;
}

map<int, unsigned int> &GetDefineRels()
{
    static map<int, unsigned int> defineRels;
    return defineRels;
}

vector<int> &GetIncludeLines()
{
    static vector<int> includeLines;
    return includeLines;
}

vector<char *> &GetIncludeDirs()
{
    static vector<char *> includeDirs;
    return includeDirs;
}
