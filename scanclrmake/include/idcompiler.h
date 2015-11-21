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
#ifndef IDCOMPILER_H
#define IDCOMPILER_H

#include <string>
#include <utility>
#include <vector>
namespace std {}
using namespace std;

// This module is given the compiler name from a ClearCase config record,
// and figures out from that the compiler ID used by parsercmd to translate
// the compiler flags into a set of flags usable by aset_CCcc.
//
// The state of a CompilerIdentifier object retains
//     (1) the "prefs", a user override to the automatic methods
//     (2) a cache of resolved compiler names.
//
class CompilerIdentifier {
public:
    CompilerIdentifier(const char *compiler_list_prefs, bool verbose);
    bool id(const string &name, const string &path, string &id);
private:
    bool userOverride(const string &name, string &id);
    bool checkCache(const string &path, string &id);
    void cacheId(const string &path, const string &id);
    const char *const compilerListPrefs;

    typedef vector<pair<string, string> > CacheT;
    CacheT cache;
    bool beVerbose;
};

#endif // IDCOMPILER_H
