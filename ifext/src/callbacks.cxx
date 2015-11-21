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
#include <elsParser.h>

#ifndef ISO_CPP_HEADERS
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#else /* ISO_CPP_HEADERS */
#include <cstdio>
using namespace std;
#include <cerrno>
#include <cstdlib>
#endif /* ISO_CPP_HEADERS */

#include <Hash.h>
#include <elsParser.h>

#ifndef ISO_CPP_HEADERS
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#else /* ISO_CPP_HEADERS */
#include <cstdio>
using namespace std;
#include <cerrno>
#include <cstdlib>
#endif /* ISO_CPP_HEADERS */

#include <Hash.h>
#include <string.h>
#include "msg.h"

#if 0
extern "C" void enter()
{
  static count;
  count++;
}
#else 
#define enter()
#endif

typedef char * charPtr;
genArr(charPtr);

struct memPool {
protected:
  int el_size;
  int chunk_size;
  charPtrArr chunks;
  char* next_ptr;
  int no_avail;
public:
  char* alloc();
protected:
  void new_chunk();
public:
  void reset();
  void pop(void*) {if(no_avail != chunk_size) { ++no_avail; next_ptr -= el_size;}}
  memPool(int elsz, int chsz=1000) : el_size(elsz), chunk_size(chsz), no_avail(0) {}
  ~memPool() {reset();}
};

char* memPool::alloc()
{
  if(no_avail == 0) new_chunk();
  --no_avail;
  char* cur_ptr = next_ptr;
  next_ptr += el_size;
  return cur_ptr;
}

void memPool::new_chunk()
{
  next_ptr = (char*)calloc(chunk_size, el_size);
  chunks.append(&next_ptr);
  no_avail = chunk_size;
}

void memPool::reset()
{
  int no_chunks = chunks.size();
  for(int ii=0; ii<no_chunks; ++ii)
    free(*(chunks[ii]));
  no_avail = 0;
  next_ptr = NULL;
  chunks.reset();
}

struct memStringPool : public memPool {
private:
  charPtrArr biggies;
public:
  memStringPool(int bufsize = 36000) : memPool(1,bufsize) {}
  ~memStringPool() {reset();}
  void reset();
private:
  char* alloc(int len);
public:
  char* strdup(const char*);
};

char* memStringPool::alloc(int len)
{
  if(no_avail < len) {
    if(len >= chunk_size){
      char*big = (char*)malloc(len);
      biggies.append(&big);
      return big;
    } else {
      new_chunk();
    }
  }
  no_avail-=len;
  char* cur_ptr = next_ptr;
  next_ptr += len;
  return cur_ptr;
}
char* memStringPool::strdup(const char*string)
{
  if(!string)
    return NULL;
  int len = strlen(string);
  char* ptr = alloc(len+1);
  if(ptr)
    strcpy(ptr, string);
  return ptr;
}
void memStringPool::reset()
{
  int no_big = biggies.size();
  for(int ii=0; ii<no_big; ++ii)
    free(*(biggies[ii]));
  biggies.reset();
  memPool::reset();
}

void iffSrcFile::reset()
{
  ast = first_ast = last_ast = NULL;
  ref_ids.reset();
  keywords.reset();
  src_symbols.reset();
  if(sym_mask)
    free(sym_mask);
  sym_mask = NULL;
}
void iffParser::reset_output_file()
{
  if(sources.size() > 0)
    msg("too late to reset output file", error_sev) << eom;
  else {
    iffSrcFile*src = new iffSrcFile(OUTPUT);
    sources.append(&src);
  }
}

iffSrcFile* iffParser::add_file(const char*fn, const char* df, int ftype)
{
  iffSrcFile*src = new iffSrcFile(ftype);
  src->file_ind = sources.size();
  sources.append(&src);

  if(fn[0] == '"')
    src->fname = fn;
  else
    src->fname.printf("\"%s\"", fn);

  src->fdump = df;
  ifp_line = 1;
  return src;
}

// Sets the output (sources[0]) file name if it is currently "" or null.
void iffParser::set_output(const char*fn) 
{
  iffSrcFile*out = *sources[0];
  const char*old_fn = out->fname;
  if(old_fn && old_fn[2]){  // skip ""
    if(strcmp(old_fn, fn) != 0)
      msg("changing ofile from $1 to $2", normal_sev) << old_fn << eoarg << fn << eoarg << eom;
    return;
  }
  if(fn[0] == '"')
    out->fname = fn;
  else
    out->fname.printf("\"%s\"", fn);
}

void ifp_set_output(char*fn)
{
  context->set_output(fn);
}
iffParser* context;
iffParser::iffParser() 
{ 
  region_pool = new memPool(sizeof(iffRegion));
  ast_pool = new memPool(sizeof(iffAstNode));
  ast_string_pool = new memPool(sizeof(iffAstString));
  symbol_pool = new memPool(sizeof(iffSymNode));
  string_pool = new memStringPool();
  context = this;
  reset_macro_region();
  reset_output_file();
}
iffParser::~iffParser()
{ 
  context = NULL; 
  delete region_pool;
  delete ast_pool;
  delete ast_string_pool;
  delete symbol_pool;
  delete string_pool;
  macro_region = NULL;
  int sz = sources.size();
  for(int ii=0; ii<sz; ++ii)
   (* sources[ii])->reset();
}
iffRegion*iffParser::macro_region;
void iffParser::reset_macro_region()
{
   macro_region = (iffRegion*) context->region_pool->alloc();
}
const char* stringHash::name(const Object* oo)const 
{
  return (const char*) oo;
}
bool stringHash::isEqualObjects(const Object& o1, const Object&o2) const
  {return strcmp((char*)&o1,(char*)&o2) == 0 ;}
const char* stringHash::put(const char* nm)
{
  char* obj = (char*)findObjectWithKey(*((Object*)nm));
  if(!obj){
    obj = context->string_pool->strdup(nm);
    add(*((Object*)obj));
  }
  return obj; 
}

static  stringHash code_names;
static  stringHash keyword_names;
static  stringHash relation_names;

const char* ifp_get_code(const char*code)
{
 return code_names.put(code);
}
char* ifp_rel_get_code(char*code)
{
 return (char*)relation_names.put(code);
}

void file_make(char*fn)
{
  context->report_file(fn);
}
void language(char*lan)
{
  if(context->language.str() == NULL)
    context->language = lan;
}

extern "C" int ifextparse();
int iffParser::parse(const char*fn)
{
  int code = 0;
  FILE* f = fopen(fn, "r");
  if(f) {
    ifp_sym_offset = symbols.size();
    ifp_init_input (f);
    code = ifextparse();
    ifp_reset_input();
    fclose(f);
  } else {
    msg("cannot open $1", catastrophe_sev) << fn << eoarg << eom;
    code = errno;
  }
  return code;
}

bool iffParser::lookup_src(const char*fn){
  int no_src = sources.size();
  cur_file = NULL;
  for(int ii=0; ii<no_src; ++ii){
    iffSrcFile*src = *sources[ii];
    if(src->fname==fn){
      cur_file = src;
      break;
    }
  }
  if (cur_file == NULL) {
    cur_file = add_file(fn, NULL, iffParser::FOREIGN);
  }
  cur_file_ind = cur_file->file_ind;
  cur_file_type = (enum iffParser::FileType)cur_file->ftype;
  return (cur_file_type != FOREIGN);
}

iffParser::FileType iffParser::report_file(const char*fn)
{
  if(sources.size() == 1)
    add_file(fn, NULL, INPUT);

  if(lookup_src(fn)){
    if(cur_file_type == INPUT) {
      ifp_skip_flag = 0;
    } else if(cur_file_type == OUTPUT) {
      ifp_skip_flag = 0;
      process_maps();
    }
  } else {  // FOREIGN
    ifp_skip_flag = 1;
  }
  return cur_file_type;
}

void map_copy_pppp(iffPosition*p1, iffPosition*p2, iffPosition*p3, iffPosition*p4)
{
  if(p2->column == 0 && p1->column == 1 && p3->column == 1)  // MAP lines
    return;

  if(p2->column == 1) {
    p2->line --;
    p2->column = 0;
    p4->column --;
    if(p4->column == 0){
      p4->line --;
    }
  }
  int no_lines = p2->line - p1->line;
  if(no_lines > 0) {
     map_copy_pcp(p1, 0, p3);
     int line1 = p1->line+1;
     int line2 = p2->line-1;
     if(line2 >= line1)
       map_lines(line1, line2, p3->line+1);
     p2->column = 1;
     int last_col = p4->column;
     p4->column = 1;
     map_copy_pcp(p2, last_col, p4);
  } else {
     map_copy_pcp(p1, p4->column, p3);
  }
}

static void create_map(iffMap::TYPE  type, iffPosition *start, 
              int last_col, int src_line, int col_del, iffRegion*in_fixed)
{
  iffMap*map = context->maps.grow();
  map->type = type;
  map->out = *start;
  map->in_line = src_line;
  map->in_fixed = in_fixed;
  map->col_delta = col_del;
  map->out_end_col = last_col;
}

void map_copy_pcp(iffPosition*p1, int last_col, iffPosition*p3)
{
  int col_delta =  p3->column - p1->column;
  create_map(iffMap::COPY, p3, last_col, p1->line, col_delta, 0);
}

static void map_provide(iffLineMapArr& lmarr, int en) 
{
  int sz = lmarr.size();
  if(sz <= en){
    int del = en-sz+1;
    iffLineMap*m = lmarr.grow(del);
    for(int ii=0; ii< del; ++ii)
      m[ii].type = iffLineMap::NONE;
  }
}

void map_fixed_pppp(iffPosition*p1, iffPosition*p2, iffPosition*p3, iffPosition*p4)
{
  int no_lines = p4->line - p3->line;
  iffRegion*fixed =  (iffRegion*) context->region_pool->alloc();
  fixed->st = *p1;
  fixed->en = *p2;
  if(no_lines > 0) {
    create_map(iffMap::FIXED, p3, 0, p1->line, p1->column, fixed);
    int cur_in_line = p1->line + 1;
    for(int ii=p3->line +1; ii < p4->line; ++ii){
      iffPosition pos;
      pos.line = ii;
      pos.column = 1;
      create_map(iffMap::FIXED, &pos, 0, cur_in_line++, 1, fixed);
    }
    int last_col = p4->column;
    p4->column = 1;
    create_map(iffMap::FIXED, p4, last_col, cur_in_line, 1, fixed);
  } else {
    create_map(iffMap::FIXED, p3, p4->column, p1->line, p1->column, fixed);
  }

// chack line maps
  int new_en = p4->line;
  if(new_en >= context->line_maps.size()){
    iffLineMapArr& lmarr = context->line_maps;
    map_provide(lmarr, new_en);
    int f_ind =  context->cur_file_ind;
    iffLineMap*lm = lmarr[p3->line];
    for(int ii = p3->line; ii<=new_en; ++ii){
      lm->file_ind = f_ind;
      ++lm;
    }
  }
}

void map_lines(int st, int en, int new_st)
{
  int f_ind = context->cur_file_ind;
  int new_en = new_st + en - st;
  iffLineMapArr& lmarr = context->line_maps;
  map_provide(lmarr, new_en);
  iffLineMap*lm = lmarr[new_st];
  for(int ii=st; ii <= en; ++ii){
    lm->type = iffLineMap::LINE;
    lm->line = ii;
    lm->file_ind = f_ind;
    ++lm;
  }
}


iffLineMap::TYPE iffParser::map(iffPosition*pos, int is_start, iffRegion**fixed)
{
  int line = pos->line;
  if(line_maps.size() <= line)
    return iffLineMap::NONE;
  iffLineMap* lm = line_maps[line];
  iffLineMap::TYPE type = (iffLineMap::TYPE)lm->type;
  if(type == iffLineMap::NONE)
    return type;
  
  if (type == iffLineMap::LINE) {
    pos->line = lm->line;
    return type;
  }
  iffMap*m  = maps[lm->line];
  for(iffMap*cur = m; (cur->out.line == line); ++cur){
    if(cur->out.column > pos->column)
      continue;
    if(cur->out_end_col && (cur->out_end_col < pos->column))
      continue;
    if(cur->type == iffMap::COPY){
      pos->line = cur->in_line;
      pos->column -= cur->col_delta;
      type = iffLineMap::LINE;
    } else {  // FIXED
      iffRegion*reg = cur->in_fixed;
      *pos = is_start ? reg->st : reg->en;
      *fixed = reg;
    }
    return type;      
  }
  pos->line = m->in_line;
  return iffLineMap::LINE;
}

int iffParser::map_region(iffRegion*reg) // returns src_ind
{
  if(!reg)
    return 0;

  int line = reg->st.line;
  if(line_maps.size() <= line)
    return 0;

  int file_ind = line_maps[line]->file_ind;
  iffRegion*fixed;

  iffLineMap::TYPE st_type =  map(&reg->st, 1, &fixed);
  if(st_type != iffLineMap::LINE)
    return 0;

  iffLineMap::TYPE en_type;
  if(reg->en.line != 0){
    en_type =  map(&reg->en, 0, &fixed);
    if(en_type != iffLineMap::LINE)
    return 0;
  }

  reg->file_ind = file_ind;
  return file_ind;
}

int iffParser::map_ast_region(iffRegion*reg, iffRegion**fixed) // returns src_ind
{
  if(!reg)
    return 0;

  int line = reg->st.line;
  if(line_maps.size() <= line)
    return 0;

  *fixed = NULL;
  int file_ind = line_maps[line]->file_ind;
  iffRegion*fixed_st=NULL;
  iffRegion*fixed_en=NULL;
  iffLineMap::TYPE st_type =  map(&reg->st, 1, &fixed_st);
  if(st_type == iffLineMap::NONE)
    return 0;

  iffLineMap::TYPE en_type;
  if(reg->en.line != 0){
    en_type =  map(&reg->en, 0, &fixed_en);
    if(en_type == iffLineMap::NONE)
      return 0;
  }
  if(st_type == iffLineMap::MAP && en_type == iffLineMap::MAP){
    if(fixed_st == fixed_en)
      *fixed = fixed_st;
  }
  reg->file_ind = file_ind;
  return file_ind;
}

iffRegion* reg_make(iffPosition*pos)
{
  if (context->cur_file_type == iffParser::OUTPUT) {
    if(context->line_maps.size() <= pos->line) {
//      msg("position not mapped: $1 / $2", warning_sev) << pos->line << eoarg << pos->column << eoarg << eom;
      return NULL;
    }
    int file_ind =  context->line_maps[pos->line]->file_ind;
    if(file_ind == 0){
      if(ast_level == 1)
	ifp_skip_block(0);
      return NULL;
    }
  }
  iffRegion*reg = (iffRegion*) context->region_pool->alloc();
  if(reg){
    reg->st = *pos;
    reg->file_ind = context->cur_file_ind;
  }
  return reg;
}
iffRegion* reg_end(iffRegion*reg, iffPosition*pos)
{
  if(!reg || reg == iffParser::macro_region)
    return reg;
  reg->en = *pos;

  return reg;
}
iffSymAtr* sym_make(int ind, char* code)
{
  iffSymNode* sym = (iffSymNode*) context->symbol_pool->alloc();
  iffSymPtrArr& symbols = context->symbols;
  if(ind >= symbols.size()){
    int del = ind - symbols.size() + 1;
    iffSymPtr*sp = symbols.grow(del);
    for(int ii=0; ii<del; ++ii)
      sp[ii] = NULL;
  }
  iffSymNode* premature_sym;
  *(int * ) (iffSymAtr*)sym = 0;
  if ((premature_sym = *(symbols[ind])) != NULL) {
    // Attributes may have been stated in advance of SYM line.
    ((char *)sym)[1] = ((char *)premature_sym)[1];
  }
  *(symbols[ind]) = sym;
  if(code)
    sym->code = code_names.put(code);
  else
    sym->code = NULL;
  sym->region = NULL;

  if(context->cur_file && code != NULL) {
    // Add the index when the SYM line is processed.
    // The list can't abide duplicates.
    context->cur_file->add_symbol(ind);
  }
  context->cur_sym_ind = ind;

  return sym;
}
static void track_file_sym(iffSymNode *node, char *name) {
  iffSrcPtrArr &sources = context->sources;
  int no_src = sources.size();
  iffSrcFile *src = NULL;
  if (name[0] != '"') {
    msg("Expected name to begin with \".", catastrophe_sev) << eom;
    exit(1);
  }
  for(int ii=0; ii<no_src; ++ii){
    iffSrcFile*s = *sources[ii];
    if(s->fname==name){
      src = s;
      break;
    }
  }
  if (src == NULL) {
    src = context->add_file(name, NULL, iffParser::FOREIGN);
  }
  if (src->symid == 0) {
    src->symid = context->cur_sym_ind;
  }
}
void sym_add_name(iffSymAtr* sym, char*name)
{
  static const char* file_code = ifp_get_code("file"); 
  iffSymNode *node = (iffSymNode *)sym;
  enter();
  node->name = context->string_pool->strdup(name);

  // Track symid of each file.
  if (node->code == file_code) {
    track_file_sym(node, name);
  }
}

void sym_add_reg(iffSymAtr* sym, iffRegion*reg)
{
  enter();
  if(reg)
    ((iffSymNode*)sym)->region = reg;
}

iffSymAtr* sym_get_atr(int ind)
{
  return ((context->symbols.size() <= ind) || (! * context->symbols[ind])) ? 
         sym_make(ind, NULL) :  * context->symbols[ind];
}
void sym_add_atr(iffSymAtr*sym, char*atr)
{
  genString& attrs = ((iffSymNode*)sym)->attributes;
  if(attrs.str())
    attrs.printf("%s,%s", attrs.str(), atr);
  else
    attrs = atr;
}

iffRel* sym_add_rel(int src, const char *code, int trg)
{
  static const char*type_code = relation_names.put("\"type\"");
  static const char*specialize_code = relation_names.put("\"specialize\"");
  static const char*define_code = relation_names.put("\"define\"");
  static const char*include_code = relation_names.put("\"include\"");
  static const char*context_code = relation_names.put("\"context\"");

  iffRel* rel = context->relations.grow();
  rel->code = code;
  rel->src = src;
  rel->trg = trg;
  rel->atr = NULL;
  if(code == type_code)
    rel->propagate = -1;  // backward
  else if (code == define_code) {
    rel->propagate = -1; // backward
    if (ifp_sym_offset && src - ifp_sym_offset == 1)
      rel->propagate = 0; 
                    // We skip REL [temporary file] "define" Macro/Typedef
                    // temporary .c file, generated by dis_sql from
                    // actual emabedded SQL source file
                    // We assume that order of processing two input .iff
                    // files is: first is a file, produced by dis_sql with
                    // ast_replace stuff in it and second is produced by
                    // aset_CCcc from temporary .[Cc] file (which was produced
                    // by dis_sql). This assumption based on the fact, that if 
                    // we switch order, than ifext crashes even without
                    // modification this function. 
                    // Second assumption is that each .iff file,
                    // produced by any .iff generartor (aset_CCcc, dis_sql ...)
                    // has SYM [1], representing compilation unit.
                    // 10/24/99
  }
  else if (code == include_code && 
	   ifp_sym_offset && src - ifp_sym_offset == 1) {
                    // This change will affect embedded SQL files only.
                    // "include" relation is propagated from the dis_sql 
                    // generated .[Cc] file to the original .pc[c] file. The 2
                    // assumptions mentioned above are being utilised here.
                    // In the "include" relation if the src symbol refers to
                    // temp .[Cc] file it is changed to [1] (ie .pc[c] file).
    rel->src = 1;
    rel->propagate = 1;
  }
  else if (code == specialize_code)
    rel->propagate = 1;
  else if (code == context_code)
    rel->propagate = -1;
  else
    rel->propagate = 0;

  return rel;
}

void rel_add_atr(iffRel*rel, char*atr)
{
  rel->atr = (char*) code_names.put(atr);
}

void iffSrcFile::add_ast_root(iffAstNode* root)
{
  static  const char* root_name = code_names.put("ast_root");
  static  const char* rep_name = code_names.put("ast_replace");
  if(!root)
    return;
  // push root in front of context->ast
  iffAstNode*last = root;
  if(root->code == rep_name) {
    ast_replace = root;
    root = root->next;
    ast_replace->next = NULL;
    if(!root)
      return;
  }
  if(root->code == root_name) {
    while(last->next)
      last = last->next;
  } else {
    iffAstNode*node = ast_make_node((char*)root_name);
    node->first = root;
    root = node;
    last = node;
  }
  last->next = ast;
  ast = root;
}
void ast_add_roots(iffAstNode* root)
{
  context->cur_file->add_ast_root(root);
}

iffAstNode* ast_make_node(char* code)
{
  iffAstNode* node = (iffAstNode*)context->ast_pool->alloc();
  node->code = (char*)code_names.put(code);
  *(int * ) node = 0;
  node->ind = 0;
  node->first = 0;
  node->next = 0;
  if(context->cur_file_type == iffParser::INPUT)
    node->region = iffParser::macro_region;
  else
    node->region = NULL; 

  return node;
}
iffAstNode* ast_make_string(char* str)
{
  static char* string_code = (char*)code_names.put("___string_ast_node");
  iffAstString* node = (iffAstString*)context->ast_string_pool->alloc();
  node->code = string_code;
  *(int * ) node = 0;
  node->ind = 0;
  node->first = 0;
  node->next = 0;
  node->region = 0;
  node->string = context->string_pool->strdup(str);
  return node;
}

iffAstNode* ast_add_extern_region(iffAstNode*node, char*file, iffRegion*reg)
{
  if(!reg)
    return node;
  if(context->cur_file_type == iffParser::INPUT) {
    if((*context->sources[0])->fname == file)
      node->region->file_ind = 0;
    else
      msg("Cannot map to $1", warning_sev) << file << eoarg << eom;
  } else {
    node = NULL;
  }
  return node;
}

void iffParser::process_maps()
{
  sort_maps();
  int no_maps = maps.size();
  int cur_line = 0;
  for(int ii=0; ii<no_maps; ++ii){
    int out_line = maps[ii]->out.line;
    if(out_line != cur_line){
      cur_line = out_line;
      iffLineMap*lm = line_maps[cur_line];
      lm->type = iffLineMap::MAP;
      lm->line = ii;
    }
  }
  // add sentinel map at the end
  iffMap*m = maps.grow();
  m->out.line = 0;
  m->out.column = -1;
}

extern "C" int
iffMap_compare(const void *first_void,
	       const void *second_void)
{
  iffMap *first = (iffMap *) first_void;
  iffMap *second = (iffMap *) second_void;

  if (first->out.line < second->out.line)
    return -1;

  if (first->out.line > second->out.line)
    return 1;

  if (first->type == iffMap::FIXED && second->type == iffMap::COPY)
    return 1;

  if (second->type == iffMap::FIXED && first->type == iffMap::COPY)
    return -1;

  return 0;
}

int
iffParser::sort_maps()
{
  enter();
  int no_maps = maps.size();
  qsort((void *) maps[0], no_maps, sizeof(iffMap), iffMap_compare);
  return no_maps;
}

iffKeyword* key_make(char*code)
{
  enter();
  iffKeyword* key = context->cur_file->keywords.grow();
  key->code = (char*) keyword_names.put(code);
  key->region = NULL;
  return key;
}

void key_add_region(iffKeyword*key, iffRegion*reg)
{
  enter();
  if(reg)
    key->region = reg;
  else
    context->cur_file->keywords.pop();
}

void ref_make(iffRefId*ref)
{
  context->cur_file->ref_ids.append(ref);
}
void err_make(int line, char*msg)
{
  enter();
  iffError*err = context->errors.grow();
  err->line = line;
  err->msg = context->string_pool->strdup(msg);
}

void iffParser::extract()
{
  iffSrcFile*output_file = *sources[0];
  if(output_file->fname.str()){
    generate_dependencies();

    if(output_file->ast)
      extract_ast();
    if(output_file->keywords.size())
      extract_keywords();
    if(output_file->ref_ids.size())
      extract_ref_ids();
    if(output_file->src_symbols.size())
      extract_symbols();
  }
}

void iffParser::extract_ref_ids()
{
  iffSrcFile*cur_src;
  int cur_ind = 0;
  iffSrcFile* output_file = *(sources[0]);
  int no_ids = output_file->ref_ids.size();
  for(int ii=0; ii<no_ids; ++ii){
    iffRefId*ref = (output_file->ref_ids)[ii];
    if(!ref->region)
      continue;
    int ind = map_region(ref->region);
    if(ind > 0){
       if(ind != cur_ind){
	 cur_ind = ind;
	 cur_src = *sources[ind];
       }
       if (cur_src->ftype != FOREIGN) {
         cur_src->ref_ids.append(ref);
       }
    }
  }
  output_file->ref_ids.reset();
}

void iffParser::extract_keywords()
{
  iffSrcFile*cur_src;
  int cur_ind = 0;
  iffSrcFile* output_file = *(sources[0]);
  int no_key = output_file->keywords.size();
  for(int ii=0; ii<no_key; ++ii){
    iffKeyword*key = (output_file->keywords)[ii];
    int ind = map_region(key->region);
    if(ind > 0){
       if(ind != cur_ind){
	 cur_ind = ind;
	 cur_src = *sources[ind];
       }
       if (cur_src->ftype != FOREIGN) {
         cur_src->keywords.append(key);
       }
    }
  }
  output_file->keywords.reset();
}

static void map_ast_tree(iffAstNode*ast, int cur_src_ind, iffRegion*par)
{
  iffRegion*reg = ast->region;
  if(reg){
    iffRegion * fixed = NULL;
    int ind = context->map_ast_region(reg, &fixed);
    if(ind != cur_src_ind) {
      ast->region = NULL;
    } else if(fixed) {
      if(par)
	ast->region = NULL;
      else
	par = fixed;
    }
  }
  for(iffAstNode*cur=ast->first; cur; cur=cur->next){
    map_ast_tree(cur, cur_src_ind, par);
  }
}
void iffParser::extract_ast()
{
  iffSrcFile*cur_src;
  int cur_ind = 0;
  iffSrcFile* output_file = *(sources[0]);
  iffAstNode* ast = output_file->ast;
  iffAstNode*cur;
  iffAstNode*next;
  for(cur=ast->first; cur; cur=next){
    next = cur->next;

    iffRegion*reg = cur->region;
    if(!reg)
      continue;
    int line = reg->st.line;
    int ind = (line_maps.size() > line) ? (line_maps[line]->file_ind) : 0;
    if(ind > 0){
      if(ind != cur_ind){
	cur_ind = ind;
	cur_src = *sources[ind];
      }
      if(!cur_src->last_ast){
	cur_src->first_ast = cur_src->last_ast = cur;
      } else {
	cur_src->last_ast->next = cur;
	cur_src->last_ast = cur;
      }
      cur->next = NULL;
      map_ast_tree(cur, cur_ind, NULL);
    }
  }
  output_file->ast = NULL;  
  int sz = sources.size();
  for(int ii=0; ii<sz; ++ii){
    iffSrcFile * cur_src = *sources[ii];
    if (cur_src->ftype == INPUT) {
      cur_src->add_ast_root(cur_src->first_ast);
    }
  }
}

void iffParser::extract_symbols()
{
  iffSrcFile*cur_src;
  int cur_ind = 0;
  iffSrcFile* output_file = *(sources[0]);
  int total = symbols.size();
  int no_sym = output_file->src_symbols.size();
  iffRegion*fixed;
  for(int ii=0; ii<no_sym; ++ii){
    int sind = * output_file->src_symbols[ii];

    if(sind >= total){
      msg("too many symbols encountered", warning_sev) << eom;
      continue;
    }
    iffSymNode*sym = *symbols[sind];
    if(!sym){
      continue;
    }
    if(!sym->region)
      continue;

    int ind = map_ast_region(sym->region, &fixed);
    if(ind < 1) {
      sym->region = NULL;
      continue;
    }
    if(ind != cur_ind){
      cur_ind = ind;
      cur_src = *sources[ind];
    }
    if (cur_src->ftype != FOREIGN) {
      cur_src->src_symbols.append(&sind);
    }
    if (cur_src->symid != 0) {
      static const char *define_code = relation_names.put("\"define\"");
      sym_add_rel(cur_src->symid, define_code, sind);
    }
  }
  output_file->src_symbols.reset();
}
void ifp_report_bad_symbol(int ind);
void iffParser::generate_dependencies()
{
  int sz = relations.size();
  int no_sym = symbols.size();
  for(int ii=0; ii<sz; ++ii){
    iffRel* rel = relations[ii];
    int src = rel->src;
    int trg = rel->trg;
    if(src >= no_sym) {
      ifp_report_bad_symbol(src);
      rel->src = 0;
      rel->trg = 0;
    } else if (trg >= no_sym) {
      ifp_report_bad_symbol(trg);
      rel->src = 0;
      rel->trg = 0;
    } else {
      iffSymNode* ssym = *(symbols[src]);
      iffSymNode* tsym = *(symbols[trg]);
      if(!ssym){
        ifp_report_bad_symbol(src);
	rel->src = 0;
	rel->trg = 0;
      } else if(!tsym){
        ifp_report_bad_symbol(trg);
	rel->src = 0;
	rel->trg = 0;
      } else if(rel->propagate){
	  if(rel->propagate == 1)
	    ssym->add_dependency(trg);
	  else
	    tsym->add_dependency(src);
      }
    }
  }
}

static void purge_ast(iffAstNode*cur, iffSymPtrArr& symbols)
{
  int id = cur->ind;
  if((id > 0) && (!*symbols[id])){
    cur->ind = 0;
  } 

  for(iffAstNode*child=cur->first; child; child=child->next){
      purge_ast(child, symbols);
  }
}
void iffParser::remove_functions(const char* prefix)
{
  static const char* fun_code = ifp_get_code("function"); 
  int pref_len = strlen(prefix);
  int first_ch = prefix[0];

  int no_sym = symbols.size();
  for(int sind=1; sind<no_sym; ++sind){
    iffSymNode*sym = *symbols[sind];
    if(!sym){
      continue;
    }
    if(sym->code != fun_code)
      continue;
    const char*name = sym->name + 1; // skip \"
    if((first_ch==name[0]) && (strncmp(name,prefix, pref_len)==0)){
      *symbols[sind] = NULL;
      purged_symbols.append(&sind);
    }
  }
  if(purged_symbols.size() == 0)
    return;

  iffSrcFile* output_file = *(sources[0]);
  int no_ids = output_file->ref_ids.size();
  for(int ii=0; ii<no_ids; ++ii){
    iffRefId*ref = (output_file->ref_ids)[ii];
    if(!ref->region)
      continue;
    int id = ref->id;
    if(!*symbols[id])
      ref->region = NULL;
  }
  for(iffAstNode*cur=output_file->ast; cur; cur=cur->next)
    purge_ast(cur, symbols);
}

void parser_error(int line_num, char *formatted, char *context, char *column_locator)
{
  if (context != NULL) {
    msg("ifext parser error at line $1: $2\n   context: $3\n   context: $4", error_sev)
	  << ifp_line << eoarg
	  << formatted << eoarg
	  << context << eoarg
	  << column_locator << eoarg << eom;
  }
  else {
    msg("ifext parser error at line $1 column $2: $3", error_sev)
	  << ifp_line << eoarg
	  << strlen(column_locator) << eoarg
	  << formatted << eoarg << eom;
  }
}

/* The following estring functions ("error string") provide a C interface
 * for putting together a string into an error message.
 */
void *start_estring()
{
  return (void *)new genString();
}

void  append_estring(void *obj, char ch)
{
  (*(genString *)obj) += ch;
}

void  vsprintf_estring(void *obj, const char* fmt, va_list ap1, va_list ap2)
{
    ((genString *)obj)->vsprintf(fmt, ap1, ap2);
}

char *finish_estring(void *obj)
{
  return (char *)((genString *)obj)->str();
}

void  free_estring(void *obj)
{
  delete ((genString *)obj);
}
