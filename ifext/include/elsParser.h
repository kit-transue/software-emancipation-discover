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
#ifndef _iffParser
#define _iffParser

#include <iffYacc.h>
#include <genArr.h>
#include <Hash.h>

#ifndef ISO_CPP_HEADERS
#include <string.h>
#else /* ISO_CPP_HEADERS */
#include <cstring>
using namespace std;
#endif /* ISO_CPP_HEADERS */

#include <genString.h>

#ifndef ISO_CPP_HEADERS
#include <iostream.h>
#else /* ISO_CPP_HEADERS */
#include <iostream>
#endif /* ISO_CPP_HEADERS */

class stringHash : public nameHash
{
  virtual const char* name(const Object* oo)const;
  virtual bool isEqualObjects(const Object& o1, const Object&o2) const;
 public:
  const char* put(const char* nm);
};
struct memPool;
struct memStringPool;
genArr(iffRel);
genArr(int);

class iffSymPtrArr;
struct iffSymNode : public iffSymAtr {
  const char* code;
  const char* name;
  iffRegion* region;
  genString attributes;
  intArr dependencies;
  void add_dependency(int ind){dependencies.append(&ind);}
  int batrs() const { return ( (char*)this)[1];}
  void print(const iffSymPtrArr&, int ind, int file_ind, ostream&) const;
};
typedef iffSymNode*iffSymPtr;

struct iffMap {
  enum TYPE {FIXED, COPY};
  iffPosition out;
  unsigned int in_line ;
  int  col_delta;
  iffRegion* in_fixed;
  unsigned int type : 8;  
  unsigned int out_end_col  : 24; // 0 means 'to the end'
};

struct iffLineMap {
  enum TYPE {NONE, LINE, MAP};
  short  type;      // NONE, LINE or MAP  
  short file_ind;   // =0 OUTPUT, >0 INPUT or FOREIGN
  int   line;       // index in maps if MAP
};
struct iffError {
  int line;
  char* msg;
};

genArr(iffLineMap);
genArr(iffMap);
genArr(iffKeyword);
genArr(iffSymPtr);
genArr(iffRefId);
genArr(iffError);

struct iffSrcFile {
  genString fname;
  genString fdump;
  int file_ind;
  int ftype;
  iffAstNode* ast;
  iffAstNode* ast_replace;
  iffAstNode* first_ast;
  iffAstNode* last_ast;
  
  iffRefIdArr ref_ids;
  iffKeywordArr keywords;
  intArr   src_symbols;
  int      symid;
  char*    sym_mask;  // bool

  private:
  iffSrcFile();
  public:
  iffSrcFile(int tp)  : file_ind(0), ftype(tp),
    ast(NULL), ast_replace(NULL), 
    first_ast(NULL), last_ast(NULL), symid(0), sym_mask(NULL)
 {}
  ~iffSrcFile() { reset();}
  void add_symbol(int ind) { src_symbols.append(&ind);}
  void reset();
  void add_ast_root(iffAstNode*);

  void print(iffAstNode*node, ostream&os, int level) const;
  void print(ostream&os) const;
  void print_relations(ostream&os) const;

  void propagate_symbols(const iffSymPtrArr&symbols);
};

typedef iffSrcFile*iffSrcPtr;
genArr(iffSrcPtr);

struct iffParser {
  enum FileType {FOREIGN, INPUT, OUTPUT} cur_file_type;
  int cur_file_ind;
  iffSrcFile* cur_file;
  int cur_sym_ind;

  iffSrcPtrArr  sources;
  iffSymPtrArr  symbols;
  iffLineMapArr line_maps;
  iffMapArr     maps;
  iffErrorArr   errors;
  iffRelArr     relations;
  
  memPool*region_pool;
  memPool*ast_pool;
  memPool*ast_string_pool;
  memPool*symbol_pool;
  memStringPool *string_pool;
 
  intArr purged_symbols;

  genString language;

  iffParser();
  ~iffParser();

  void print(ostream&os) const;

  void process_maps();
  int sort_maps();
  iffLineMap::TYPE map(iffPosition*pos, int is_start, iffRegion**);  // return MAP if FIXED
  int parse(const char* fn); // return code
  FileType report_file(const char*fn);  

  bool lookup_src(const char*fn);
  iffSrcFile* add_file(const char*fn, const char* df, int ftype);
  void set_output(const char*fn);

  int map_region(iffRegion*reg);
  int map_ast_region(iffRegion*reg, iffRegion**fixed);
  void transfer_ast_regions();
  void extract();
  void extract_ast();
  void extract_ref_ids();
  void extract_keywords();
  void extract_symbols();
  void generate_dependencies();
  void remove_functions(const char* prefix);
  void reset_macro_region();
  void reset_output_file();

  void propagate_symbols();
  static iffRegion* macro_region;

};
struct iffAstString : public iffAstNode {
  const char * string;
};

const char* ifp_get_code(const char*code);
extern "C" int yy_parser_error(char*fmt , ...);
extern iffParser* context;

#endif



