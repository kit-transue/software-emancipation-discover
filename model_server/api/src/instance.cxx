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
#include <Relational.h>
#include <symbolPtr.h>
#include <symbolArr.h>
#include <smtRegion.h>
#include <metric.h>
#include "_Xref.h"
#include "symbolSet.h"
#include "xref_queries.h"
#include <xrefSymbol.h>
#include <driver_mode.h>

bool module_get_instances(symbolPtr& mod, symbolArr& instances)
{
  symbolPtr sym = mod->get_def_file();
  if(sym.isnull())
    return false;

  // MetricElement mel = Metric::get_mel_of_undefined_sym(sym, sym);
  MetricElement mel;
  if(is_model_server())
    MetricSet::get_metrics_of_sym_nocompute(sym, mel);
  else
    MetricSet::get_metrics_of_sym(sym, mel);
  if(mel.isnull())
    return false;
  mel.find_inst_between_offsets(sym, 0, -1, instances);

  return true;
}


void cli_defines_or_uses(symbolArr&src, symbolArr&res, ddSelector&sel, int def_flag);

void instances_in_mod(symbolPtr&sym, symbolPtr&mod, symbolArr&res)
{
  symbolArr sarr;
  Metric::get_instance_list(mod, sym, sarr);
  res.insert_last(sarr);
}
static void symbol_instances(const symbolPtr& s, symbolArr& res)
{
  symbolPtr sym = s.get_xrefSymbol();
  if(sym.isnull())
    return;
      
  symbolArr query_result;
  sym.get_link(ref_file, query_result);
  sym.get_link(is_defined_in, query_result);
  query_result.unsort();
  
  symbolPtr ref;
  ForEachS(ref, query_result) {
    instances_in_mod(sym, ref, res);
  }
}

static void instance_instances(symbolPtr& inst, symbolArr& res)
{
  symbolPtr xsym = inst.get_xrefSymbol();
  if(xsym.isnull())
    return;

  ddKind kind = xsym.get_kind();
  if(kind != DD_LOCAL){
    symbol_instances(xsym, res);
  } else {
    symbolPtr sc = inst.scope();
    if(sc.isnull())
      return;    

    MetricElement mel  = inst.get_MetricElement();
    MetricElement decl = mel.inst_get_decl();
    if(decl.isnull())
      return;

    int offset = decl.get_offset();

    symbolArr def_arr;
    sc.get_link(is_defined_in, def_arr);
    if(def_arr.size() == 0)
      return;

    symbolArr inst_arr;
    instances_in_mod(xsym, def_arr[0], inst_arr);
    int sz = inst_arr.size();
    for(int ii=0; ii<sz; ++ii){
      symbolPtr& s = inst_arr[ii];
      MetricElement m = s.get_MetricElement();
      MetricElement d = m.inst_get_decl();
      int o = d.get_offset();
      if(o == offset)
	res.insert_last(s);
    }
  }
}

int (*ast_instances)(const symbolPtr&ast, symbolArr&results);
static void api_instances_intern(symbolPtr& s, symbolArr& res)
{
  ddKind kind = s.get_kind();
  switch(kind){
  case DD_SMT:
    get_instances_from_smt(s, res); /* should be stubbed for pmod server */
    break;
  case DD_REGION:
    get_instances_from_region(s, res); /* should be stubbed for pmod server */
    break;
  case DD_MODULE:
    module_get_instances(s, res);
    break;
  case DD_INSTANCE:
    instance_instances(s, res);
    break;
  case DD_AST:
  case DD_DATA:
    if(ast_instances)
      (*ast_instances)(s, res);
    break;
  case DD_PROJECT:
  case DD_SCOPE:
    {
      symbolArr scopes;
      scopes.insert_last(s);
      ddSelector sel;
      sel.add(DD_MODULE);
      symbolArr mods;
      cli_defines_or_uses(scopes, mods, sel, 1);
      int sz = mods.size();
      for(int ii=0; ii<sz; ++ii)
	module_get_instances(mods[ii], res);
    }
    break;

  default:
    symbol_instances(s, res);
    break;
  }
}

void module_instances(symbolPtr& sym, symbolArr *res)
{
  symbolPtr mod = sym.get_xrefSymbol();
  if(mod.isnull())
    return;

  symbolArr mods;
  sym.get_link(included_by, mods);
  int sz = mods.size();
  for(int ii=0; ii<sz; ++ii){
     instances_in_mod(mod, mods[ii], *res);
  }
}

void declarations_in_mod(const symbolArr& arr, symbolPtr&mod, symbolArr&res)
{
  MetricSet* set = NULL;
  projNode * pr = mod.get_xref()->get_projNode();
  if(!pr)
    return;

  int sz = arr.size();
  for(int ii=0; ii<sz; ++ii){
    symbolPtr& sym = arr[ii];
    MetricElement mel = Metric::get_mel_of_undefined_sym(mod, sym);
    if(mel.isnotnull()) {
      if(!set){
	set = proj_get_set(pr);
	if(!set)
	  return;
      }
      symbolArr sarr;
      Metric::get_decls(mel, set, sarr);
      res.insert_last(sarr);
    }
  }
}

//static bool showing_instances = 0;
//bool is_showing_instances() { return showing_instances;}

void find_instances(symbolPtr& s, symbolArr *res)
{
    //  showing_instances = 1;
  api_instances_intern(s, *res);
  //  showing_instances = 0;
}
void api_instances(symbolPtr& s, symbolArr &res)
{
    //  showing_instances = 1;
  api_instances_intern(s, res);
  //  showing_instances = 0;
}
