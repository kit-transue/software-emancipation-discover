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
#include <sstream>
#ifndef _WIN32
using namespace std;
#endif
#include "msg.h"
#define CR  '\n'


void ifp_report_bad_symbol(int ind)
{
   intArr& pur = context->purged_symbols;
   if(pur.size() == 0)
     msg("index [$1] refers to nonexisting symbol", error_sev) << ind << eoarg << eom;
}

extern "C" void prast(iffAstNode*node)
{
  iffSrcFile*src = *(context->sources[0]);
  if(!src)
    return;
  src->print(node, cout, 1);
  msg("") << eom;
}

extern "C" void print_ast(int ind)
{
  iffSrcFile*src = *context->sources[ind];
  if(!src)
    return;
  for(iffAstNode*node=src->ast; node; node = node->next)
    src->print(node, cout, 1);
  msg("") << eom;
}
extern "C" void prsyms(int ind)
{

  iffSrcFile*src = *context->sources[ind];
  if(!src)
    return;
  msg("$1") << src->fname.str() << eom;
  int sz = src->src_symbols.size();
  for(int ii=0; ii<sz; ++ii){
    msg(" $1") << *src->src_symbols[ii] << eom;
  }
  msg("") << eom;
}
extern "C" void print_syms(int no_sym, const char*syms)
{
  msg("syms") << eom;
  for(int ii=1; ii<no_sym; ++ii){
    if(syms[ii])
      msg(" $1") << ii << eom;
  }
  msg("") << eom;
}
extern "C" void printdepend(int ind)
{
  msg("SymNode [$1] ") << ind << eom;
  if(!context){
    msg("no context\n") << eom;
    return;
  }
  iffSymPtrArr& symbols = context->symbols;
  if(symbols.size() <= ind){
    msg("does not exist\n") << eom;
    return;
  }

  iffSymNode* sym = * symbols[ind];
  if(!sym) {
    msg("is deleted\n") << eom;
    return;
  }
  intArr&deps = sym->dependencies;
  int sz = deps.size();
  msg("$1 dependencies") << sz << eom;
  for(int ii=0; ii<sz; ++ii)
    msg(" [$1]") << *deps[ii] << eom;
  msg("") << eom;
}
extern "C" void print_dependencies()
{
  if(!context)
    return;
  int sz = context->symbols.size();
  for(int ii=1; ii<sz; ++ii)
    printdepend(ii);
}

static void print_region(iffRegion*reg, ostream&os)
{
  if(!reg || reg == iffParser::macro_region)
    return;
  int f_ind = reg->file_ind;
  if(f_ind == 0){
    f_ind = context->map_region(reg);
    if(f_ind < 1)
      return;
  }
  if ((*(context->sources[f_ind]))->ftype == iffParser::FOREIGN)
    return;

  os << ' ' << reg->st.line << '/' << reg->st.column;
  if(reg->en.line)
    os << ' ' << reg->en.line << '/' << reg->en.column;
  else if(reg->en.column)
    os << ' ' << reg->en.column;
}

static const char* ppp[] = {"", "private", "protected", "public"};
void iffSymNode::print(const iffSymPtrArr&symbols, int ind, int file_ind, ostream&os) const
{
  os << "SYM [" << ind << "] ";
  os << code;
  if(cg) os << ":cg";
  if(def) os << (char)(cg ? ',' : ':') << "def";
  os << ' ' << name;
  if(region){
    if(region->file_ind == file_ind)
      print_region(region, os);
  }
  os << CR;

  // attributes
  const char* attrs = attributes;
  if(attrs && *attrs){
    os << "ATR [" << ind << "] " << attrs << CR;
  }
  if(batrs()){
    os << "ATR [" << ind << "]";
    int no_bi = 0;
#define pbia(x,y) if(x) os << (char)(no_bi++ ? ',' : ' ') << y
    pbia(au,"auto");
    pbia(ctor,"constructor");
    pbia(dtor,"destructor");
    pbia(inln,"inline");
    pbia(linkc,"linkage(c)");
    pbia(ellipsis,"ellipsis");
    pbia(virt,"virtual");
    os << CR;
  }
}

void iffSrcFile::print_relations(ostream&os) const
{
  int no_rels = context->relations.size();
  for(int ii=0; ii<no_rels; ++ii){
    iffRel*rel = context->relations[ii];
    int src = rel->src;
    int trg = rel->trg;
    if(!sym_mask[src] || !sym_mask[trg])
      continue;
    
    os << "REL [" << src << "] " << rel->code << " [" << trg << "] ";
    if(rel->atr)
      os << rel->atr;
    else if (rel->ppp)
      os << ppp[rel->ppp];
#define prel(x,y) else if(rel->x) os << y
    prel(virt,"virtual");
    prel(constant,"const");
    prel(pointer,"pointer");
    prel(reference,"reference");
    os << CR;
  }
}
static void print_indents(ostream&os, int level)
{
  for(int ii=0; ii<level; ++ii) os << " ";
}

void iffSrcFile::print(iffAstNode*node, ostream&os, int level) const
{
  print_indents(os, level);
  static const char* string_code = ifp_get_code("___string_ast_node");
  if(node->code == string_code){
    os << ((iffAstString*)node)->string << CR;
    return;
  }
  os << node->code;
  if(node->cg) os << ":cg";
  if(node->def) os << (char) (node->cg ? ',' : ':') << "def";
  if(node->ind) os << " [" << (unsigned int)node->ind << "]";

  if(node->region){
    print_region(node->region, os);
  }

  if(node->first){
    os << " {\n";
    int new_level = level + 1;
    for(iffAstNode*cur=node->first; cur; cur=cur->next)
      print(cur, os, new_level);
    print_indents(os, level);
    os << "}\n";
  } else {
    os << " {}" << CR;
  }
}

void iffParser::print(ostream&os) const
{
  int sz = sources.size();
  for(int ii=0; ii<sz; ++ii){
    iffSrcFile*src = *sources[ii];
    if (src->ftype == INPUT) {
      src->print(os);
    }
  }
  os << flush;
}
    
void iffSrcFile::print(ostream&os) const
{
  if(context->language.str() != NULL)
    os << "SMT language " << context->language.str() << CR;
  os << "SMT file " << fname.str() << CR;
  iffSymPtrArr & symbols = context->symbols;
  int no_sym = symbols.size();
  int ii;
  for( ii = 1; ii<no_sym; ++ii){
    if(sym_mask[ii]){
      iffSymNode*sym = *symbols[ii];
      if(sym)
	sym->print(symbols, ii, file_ind, os);
      else
	ifp_report_bad_symbol(ii);
    }
  }

  print_relations(os);

  int no_key = keywords.size();
  for( ii = 0; ii<no_key; ++ii){
    iffKeyword* key = keywords[ii];
    if(key->region && key->region != iffParser::macro_region){
      os << "SMT keyword " << key->code;
      print_region(key->region, os);
      os << CR;
    }
  }

  int no_id = ref_ids.size();
  for( ii = 0; ii<no_id; ++ii){
    iffRefId* ref = ref_ids[ii];
    if(ref->region && ref->region != iffParser::macro_region){
      os << "SMT [" << (unsigned int)ref->id << "]";
      print_region(ref->region, os);
      os << CR;
    }
  }
  if(ast){
    os << "AST {" << CR;
    for(iffAstNode*cur = ast; cur; cur = cur->next)
      print(cur, os, 1);
    os << "}" << CR; 
  }

}
////////////////

static void put_sym(int ind, int no_sym, char*syms, const iffSymPtrArr& symbols)
{
  if(ind < no_sym && (*symbols[ind] != NULL))
    syms[ind] = 1;
  else
    ifp_report_bad_symbol(ind);
 
}

void  collect_refs(int no_sym, const iffSymPtrArr& symbols, iffAstNode*node, char*syms)
{
  if(node->ind)
      put_sym(node->ind, no_sym, syms, symbols);

  for(iffAstNode*cur=node->first; cur; cur=cur->next)
    collect_refs(no_sym, symbols, cur, syms);
}

int collect_propagate(int no_sym, const iffSymPtrArr& symbols, char*syms)
{
  int count;
  intArr pending[2];
  int which = 0;
  for(int ii=0; ii<no_sym; ++ii){
    if(syms[ii]==1)
      pending->append(&ii);
  }  
  for(count=0; ; ++count){
    intArr&pen = pending[which];
    intArr&other = pending[!which];
    int sz = pen.size();
    if(!sz)
      break;

    for(int jj=0; jj<sz; ++jj){
      int ind = *pen[jj];
      if(syms[ind] <=1){
	syms[ind] = 2;
	iffSymNode* sym = * symbols[ind];
	if(!sym)
	  continue;
	intArr&dependencies = sym->dependencies;
        int no_depend = dependencies.size();
	for(int di=0; di<no_depend; ++di){
	  int trg = *dependencies[di];
	  if(syms[trg] <=1)
	    if(NULL != symbols[trg])
	      other.append(&trg);
	}
      }
    }
    pen.reset();
    which = !which;
  }
  return count;
}

void iffParser::propagate_symbols()
{
// print_dependencies();
  int sz = sources.size();
  for(int ii=0; ii<sz; ++ii){
    iffSrcFile*src = *sources[ii];
    if (src->ftype == INPUT) {
      src->propagate_symbols(symbols);
    }
  }
}

void iffSrcFile::propagate_symbols(const iffSymPtrArr&symbols)
{
  // symbol is native if it has region
  int no_sym =  symbols.size();
  sym_mask = (char*)calloc(no_sym, 1);

  int sz = src_symbols.size();
  int ii;
  for( ii=0; ii<sz; ++ii){
    int ind = *src_symbols[ii];
    sym_mask[ind] = 1;
  }

  // tokens (SMT) in case there is no ast (parse errors)
  int no_id = ref_ids.size();
  for(ii = 0; ii<no_id; ++ii){
    iffRefId* ref = ref_ids[ii];
    if(ref->region && ref->region != iffParser::macro_region){
      put_sym(ref->id, no_sym, sym_mask, symbols);
    }
  }
 
  // referenced in AST symbols
  for(iffAstNode*node=ast; node; node=node->next){
    collect_refs(no_sym, symbols, node, sym_mask);
  }

  collect_propagate(no_sym, symbols, sym_mask);
}

inline bool pos_lt(iffPosition& a, iffPosition&b)
{
  return (a.line < b.line) || ((a.line == b.line) && (a.column < b.column));
}

inline bool pos_eq(iffPosition& a, iffPosition&b)
{
  return (a.line == b.line) && (a.column == b.column);
}

// find the src region in or under the passed node
//
// Note: Nodes on same level might not be in positional order
// so we continue with the next sibling even when the previous
// sibling is past the matching region.  Checks children if
// parent region brackets matching region or is blank.
iffAstNode* ifp_find_node(iffRegion*src, iffAstNode*root)
{
  iffAstNode*rst = NULL;
  for(iffAstNode*cur = root->first; cur && !rst; cur=cur->next){
    iffRegion*cur_reg = cur->region;
    if(cur_reg) {
      if(pos_lt(cur_reg->en, src->en))
	continue;
      if(pos_lt(src->st, cur_reg->st)) 
	continue;
      if(pos_eq(cur_reg->en, src->en) && pos_eq(cur_reg->st, src->st)) {
	rst = cur;
	break;
      }
    }

    rst = ifp_find_node(src, cur);
  }
  return rst;
}

typedef iffAstNode * iffAstPtr;
genArr(iffAstPtr);

void ifp_collect_nodes(iffAstNode*root, const char*code, iffAstPtrArr&arr)
{
  if(root->code == code){
    arr.append(&root);
  } else {
    for(iffAstNode*cur = root->first; cur; cur=cur->next)
      ifp_collect_nodes(cur, code, arr);
  }
}
void ifp_move_node(iffAstNode*src, iffAstNode*trg)
{
  iffAstNode*next = trg->next;
  *trg = *src;
  trg->next = next;
}

bool ifp_move_tree(iffAstNode*src, iffAstNode*trg)
{
   static const char* host_code = ifp_get_code("host_variable");
   bool ret = true;

   iffAstPtrArr src_arr;
   ifp_collect_nodes(src, host_code, src_arr);

   int sz = src_arr.size();
   for(int ii=0; ii<sz; ++ii){
     iffAstNode*src_var = *src_arr[ii];
     iffRegion* reg = src_var->region;
     iffAstNode*trg_var = ifp_find_node(reg, trg);
     if(!trg_var){
       ostringstream regstrm;
       print_region(reg, regstrm);
       msg("could not find node $1", error_sev) << regstrm.str() << eoarg << eom;
       continue;
     }				
     // move backward, (pull back)
       
     ifp_move_node(trg_var, src_var);
   }
   ifp_move_node(src, trg);

   return ret;
}

void ifp_transfer_nodes(iffAstNode*src_root, iffAstNode*trg_root)
{
  for(iffAstNode*cur = src_root->first; cur; cur=cur->next){
    iffRegion*reg = cur->region;
    if(!reg) {
      msg("no region", warning_sev) << eom;
      continue;
    }
    iffAstNode*trg = ifp_find_node(reg, trg_root);
    if(!trg){
      ostringstream regstrm;
      print_region (reg, regstrm);
      msg("No match for source region (line/column endpoints):", warning_sev)
	<< regstrm.str() << eoarg << eom;
      continue;
    }
    bool res = ifp_move_tree(cur, trg);
    if(!res)
      msg(" move failed", warning_sev) << eom;
  }
}
void iffParser::transfer_ast_regions()
{
  int sz = sources.size();
  for(int ii=0; ii<sz; ++ii){
    iffSrcFile*file = *sources[ii];
    if(file->ftype == INPUT && file->ast_replace)
      ifp_transfer_nodes(file->ast_replace, file->ast);
  }
}

