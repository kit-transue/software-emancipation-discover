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
#ifndef _UTILS_H
#define _UTILS_H

#include <set>
namespace std {};
using namespace std;
class filename;

// define some pdf symbols
#define DIR_SEP   "/\\"
#define SLASH     "/"
#define BACK_SLASH "\\"
#define SPACE     "  "
#define COLON     ":"
#define MAPPER    "<->"
#define BEGIN     "{"
#define END       "}"
#define EXCLUDE   "^"
#define QUOTE     "\""
#define RULES_PROJ "__rules"
#define ALL_DIRS   "(**)"
#define SRC_PSET_MAP    "(*)/%/.pset"
#define PSET_MODEL_RT   "$ADMINDIR/model/pset/" 
#define PSET_MODEL_MAP1 "(1).pset"
#define PSET_MODEL_MAP2 "(1)/(2).pset"
#define SRC_MAKE_MAP    "(*)/%/.make"
#define MAKE_STRING     "\"$PSETHOME/bin/disperl\" \"$PSETHOME/mb/CR_GetFlags.pl\""
#define SRC_PMOD_MAP    "(*).pmod/%/.pmoddir"
#define PMOD_MODEL_RT   "$ADMINDIR/model/pmod/"
#define PMOD_MODEL_MAP  "(1).pmod"
#define RULES_MAPPER    "=>"
#define READ_ONLY_FLAG  "[[ R ]]"

void print_spaces(ofstream&, int);
void print_block_end(ofstream&, int level = 0);

bool name_has_space(const string&);
void replace_slash_with_backslash(string&);
bool is_reg_file_type(string&, string&);
void files_in_directory(string, string&, set<filename>&);

#endif
