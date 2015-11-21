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
#include "msg.h"
#include "Relational.h"
#include "objArr.h"
#include "objSet.h"
#include "xrefSymbol.h"
#include "symbolPtr.h"
#include "symbolArr.h"
#include "symbolSet.h"
#include "ddSelector.h"
#include "proj.h"

static const int debug = 0;

class callNode : public Relational {
public:
  symbolPtr sym;
  callNode*parent;
  callNode*aliasof;
  symbolArr aliases;
  int ind;
  virtual void	print(ostream& = cout, int level = 0) const;
  callNode(symbolPtr& fun, callNode*p, objArr&nodes);
  char const *name() const;
};
ostream& operator << (ostream& os, const callNode*node)
{
  if (node)
    os << '[' << node->ind << "] " << node->sym.get_name();
  else
    os << "[-1]";
  return os;
}

callNode::callNode(symbolPtr& fun, callNode*p, objArr&nodes) 
  : sym(fun), parent(p), aliasof(NULL)
{
  ind = nodes.size();
  nodes.insert_last(this);
}
char const *callNode::name() const 
{
  return sym.get_name();
}
void callNode::print(ostream& os, int lev) const
{
  for(int ii=0; ii<lev; ++ii)
    os << " ";
  os << "start " << this;
  if(parent)
    os << " parent " << parent;
  if(aliasof)
    os << " aliasof " << aliasof;
  os << endl;
}
void cnprt(callNode*node)
{
  node->print(cout);
}

class callContext {
  typedef void (*callQuery)(const symbolPtr&, symbolArr&);
  symbolSet all;
  objArr    nodes;
  symbolSet& ignore;
  symbolArr& out;
  callQuery query;

  void add_fun(int lev, symbolPtr& fun, callNode *caller);
  void add_def(int lev, callNode*node);
  void add_alias(callNode *caller, symbolPtr& fun);
public:
  callContext(symbolSet&ign, symbolArr&o, callQuery q) : ignore(ign), out(o), query(q) {}
  ~callContext();
  void process(symbolPtr& fun) {add_fun(0, fun, NULL);}
};
static void uses_functions(const symbolPtr& fun, symbolArr& res)
{
  symbolArr calls;
  fun->get_link(is_using, calls);
  int sz = calls.size();
  for(int ii=0; ii<sz; ++ii){
    symbolPtr& call = calls[ii];
    if(call.get_kind() == DD_FUNC_DECL)
      res.insert_last(call);
  }
}
callContext::~callContext()
{
  int sz = nodes.size();
  for(int ii=0; ii<sz; ++ii)
    delete nodes[ii];
}

void call_graph_closure(symbolPtr& fun, symbolSet& ignore, symbolArr& out)
{
  callContext cont(ignore, out, uses_functions);
  cont.process(fun);
}

void cli_defines_or_uses(symbolArr&src, symbolArr&result, ddSelector&, int def_flag);

static void get_functions(symbolArr&scope, symbolArr& funs)
{
  ddSelector sel;
  sel.add(DD_FUNC_DECL);
  cli_defines_or_uses(scope, funs, sel, 1);
}
static void get_funs(const symbolArr&in, symbolSet&out)
{
  symbolArr scopes;
  symbolArr defs;
  int sz = in.size();
  for(int ii=0; ii<sz; ++ii){
    symbolPtr& sym = in[ii];
    switch (sym.get_kind()) {
    case DD_FUNC_DECL:
      { 
	sym.get_def_symbols(defs, 2);
	out.insert(defs);
	defs.removeAll();
      }
      break;
    case DD_MODULE: case DD_PROJECT:case DD_SCOPE:
      {
	scopes.insert_last(sym);
      }
      break;
    case DD_CLASS:
      {
	sym->get_link(is_using, defs);
	out.insert(defs);
	defs.removeAll();
      }
      break;
    }
  }
  if(scopes.size()){
    get_functions(scopes, defs);
    out.insert(defs);
  }
  if(debug)
    msg("ignoring $1 functions\n") << out.size() << eom;
}
void call_graph_closure(symbolArr& ign, symbolArr& funs, symbolArr&out)
{
  
  symbolSet ignore;
  get_funs(ign, ignore);
  int sz = funs.size();
  for(int ii = 0; ii< sz; ++ii){
    call_graph_closure(funs[ii], ignore, out);
  }
}
void call_graph_closure(symbolArr& funs, symbolArr&out)
{
  
  symbolSet ignore;
  int sz = funs.size();
  for(int ii = 0; ii< sz; ++ii){
    call_graph_closure(funs[ii], ignore, out);
  }
}
void call_graph_closure(symbolArr& funs, symbolArr&out, void(*q)(const symbolPtr&, symbolArr&))
{
  symbolSet ignore;
  int sz = funs.size();
  for(int ii = 0; ii< sz; ++ii){
    callContext cont(ignore, out, q);
    cont.process(funs[ii]);
  }
}

void callContext::add_fun(int lev, symbolPtr& fun, callNode *caller)
{
  symbolArr defs;
  int no_def = fun.get_def_symbols(defs, 2);
  for(int ind=0; ind<no_def; ++ind){
    symbolPtr& def = defs[ind];
    if(ignore.includes(def))
      continue;
    if(!all.insert(def)) {
      add_alias(caller, def);
    } else {
      callNode*node = new callNode(def, caller, nodes);
      add_def(lev, node);

      if(!node->aliasof) {
	if(debug>1)
	  msg("output $1\n") << node << eom;
	out.insert_last(node->sym);
	ignore.insert(def);
	int sz = node->aliases.size();
	if(sz){
	  symbolArr& aliases = node->aliases;
	  for(int ii=0; ii<sz; ++ii)
	    ignore.insert(aliases[ii]);
	  if(debug) {
	    aliases.unsort();
	    sz = aliases.size();
	    int treshhold = debug>1 ? 0 : 20;
	    if(sz >= treshhold){
	      msg("$1 aliases: $2\n") << sz << eoarg << node << eom;
	      if(debug > 2){
		aliases.sort();
		for(int jj=0; jj<sz; ++jj)
		  msg("++ $1\n") << aliases[jj].get_name() << eom;
	      }
	    }
	  }
	}
      } else if (debug>1){
	msg("finish alias $1\n") << node << eom;
      }
    }
  }
}

void callContext::add_alias(callNode *caller, symbolPtr& fun)
{
  int ind = all.get_index(fun);
  callNode* aliasof = (callNode*) nodes[ind];
  while(aliasof->aliasof)
    aliasof = aliasof->aliasof;
  symbolArr& aliases = aliasof->aliases;

  for(callNode*cur = caller; cur!=aliasof; cur=cur->parent){
    if(cur->aliasof == aliasof)
      break;

    if(debug>1)
      msg("aliased $1 to $2\n") << cur << eoarg << aliasof << eom;

    aliases.insert_last(cur->sym);
    if(cur->aliases.size()){
      int sz = cur->aliases.size();
      for(int ii=0; ii<sz; ++ii)
	aliases.insert_last(cur->aliases[ii]);

      cur->aliases.removeAll();
    }
    cur->aliasof = aliasof;
//    nodes[cur->ind] = aliasof;
  }
}

void  callContext::add_def(int lev, callNode*node)
{
  if(debug>1)
    node->print(cout, lev);

  symbolArr calls;
  symbolPtr& fun = node->sym;
  (*query)(fun, calls);
  int sz = calls.size();
  ++lev;
  for(int ii=0; ii<sz; ++ii){
    symbolPtr& call = calls[ii];
    if(call == fun)
      continue;
    add_fun(lev, call, node);
  }
}


#include "Interpreter.h"
void cli_get_roots(symbolArr &);

static symbolArr ignore_list;
static int calls_ignore_setCmd(Interpreter*i, int argc, char const **argv)
{
    symbolArr arr;
    if(i->ParseArguments(1, argc, argv, arr) != 0)
	return TCL_ERROR;

    DI_object results;
    DI_object_create(&results);
    symbolArr & result_arr = *get_DI_symarr(results);
    
    ignore_list = arr;
    result_arr = ignore_list;
    i->SetDIResult(results);
    return TCL_OK;
}
static int calls_ignore_addCmd(Interpreter*i, int argc, char const **argv)
{
    symbolArr arr;
    if(i->ParseArguments(1, argc, argv, arr) != 0)
	return TCL_ERROR;

    DI_object results;
    DI_object_create(&results);
    symbolArr & result_arr = *get_DI_symarr(results);
    
    ignore_list.insert_last(arr);
    result_arr = ignore_list;
    i->SetDIResult(results);
    return TCL_OK;
}

static int callsCmd(Interpreter*i, int argc, char const **argv)
{
    symbolArr arr;
    if(i->ParseArguments(1, argc, argv, arr) != 0)
	return TCL_ERROR;

    DI_object results;
    DI_object_create(&results);
    symbolArr & result_arr = *get_DI_symarr(results);
    
    if(arr.size()==0)
      cli_get_roots(arr);
    if(arr[0].get_kind() == DD_FUNC_DECL){
      call_graph_closure(arr, result_arr);
    } else {
      symbolArr funs;
      get_functions(arr, funs);
      call_graph_closure(ignore_list, funs, result_arr);
    }
    i->SetDIResult(results);
    return TCL_OK;
}
projModule * symbol_get_module( symbolPtr& sym);
static int pmod_projCmd(Interpreter *i, int argc, char const *argv[])
{
  symbolArr inp;
  if(i->ParseArguments(1, argc, argv, inp) != 0)
    return TCL_ERROR;

  DI_object results;
  DI_object_create(&results);
  symbolArr & result_arr = *get_DI_symarr(results);
  
  int sz = inp.size();
  for(int ii=0; ii<sz; ++ii){
    projModule*mod = symbol_get_module(inp[ii]);
    if(!mod)
      continue;
    projNode*proj = mod->get_project();
    if(proj)
      result_arr.insert_last(proj);
  }
  result_arr.unsort();
  i->SetDIResult(results);
  return TCL_OK;
}

static int add_commands()
{
  new cliCommand("test_calls", callsCmd);
  new cliCommand("calls_ignore_set", calls_ignore_setCmd);
  new cliCommand("calls_ignore_add", calls_ignore_addCmd);
  new cliCommand("get_pmod_proj", pmod_projCmd);
  return 0;
}
static int dummy = add_commands();

