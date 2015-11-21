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
#include <smt.h>
#ifndef ISO_CPP_HEADERS
#include <iostream.h>
#else /* ISO_CPP_HEADERS */
#include <iostream>
using namespace std;
#endif /* ISO_CPP_HEADERS */
#include <systemMessages.h>
#include <msg.h>
#include <symbolPtr.h>
#include <ddict.h>
#include <general.h>
#include <metric.h>
#ifndef ISO_CPP_HEADERS
#include <stdio.h>
#else /* ISO_CPP_HEADERS */
#include <cstdio>
#endif /* ISO_CPP_HEADERS */
#include <xrefSymbol.h>
#include <messages.h>
#include <_Xref.h>
#include "cLibraryFunctions.h"

genArr(int);

/* The format of the following definitions:
 * { TYPE, name, abbreviation, whether is an integer, function }
 * function must have a prototype
 *
 * void f(symbolPtr, smtTree *& t, Relational * m, void * res).
 * 
 * res points to the result (int or double).
 * m might point to already computed values. node might point to an smtTree for
 * the symbol. If you need to use smtTree and smtHeader, please use
 * get_smt_of_sym(symbolPtr, smtTree *&, [ smtHeader *&]).
 * To use values of already computed metrics, use
 * compute_one_metric(type, sym, t, m, &tmp_res).
 * 
 * Abbreviations given here are in capital letters without spaces (used in REPORT/sw).
 */

struct xrefEntry {  // quick way to be-friend class Xref
  static symbolPtr local_symbol(Xref*xr, symbolPtr& sym) ;
};
symbolPtr xrefEntry::local_symbol(Xref*xr, symbolPtr& sym)
{
// find  representative of sym in xr

  Xref*sxr = sym.get_xref();
  if(sxr == xr) {
    return sym;
  } else {
    symbolArr loc_syms;

    ddKind kind = sym.get_kind();
    char const *name = sym.get_name();
  
    return
      xr->lookup_local(loc_syms, kind, name, 0);
  }
}

metric_definition Metric::definitions[NUM_OF_METRICS] = {
    { MET_CC, "Cyclomatic Complexity", "CC", true , Metric::cyclomatic_complexity },
    { MET_MYERS, "Myer's Complexity", "MYERS", true, Metric::myers },
    { MET_SRC_LINE_NUM, "Lines Of Code", "LOC", true, Metric::number_of_lines },
    { MET_BYTELEN, "Size in Bytes", "BYTES", true, Metric::bytelen },
    { MET_FANIN, "Base Classes (fan-in)", "FANIN", true, Metric::fanin },
    { MET_FANOUT, "Derived Classes (fan-out)", "FANOUT", true, Metric::fanout },
    { MET_DEPTH, "Nested Ctrl Structs", "DEPTH", true, Metric::depth },
    { MET_DANG_ELSEIF, "Dangling Else-Ifs", "DANG_EI", true, Metric::dang_elseif },
    { MET_DANG_SWITCH, "Defaultless Switches", "DANG_SW", true, Metric::dang_switch },
    { MET_RETURNS, "Return Statements", "RETURNS", true, Metric::returns },
    { MET_START_LINE, "Starting Line Number", "START_LN", true, Metric::starting_line },
    { MET_ULOC, "Uncommented Lines", "ULOC", true, Metric::uncom_lines },
    { MET_XLOC, "Executable Lines", "XLOC", true, Metric::exec_lines },
    { MET_IFS, "Number of Ifs", "IFS", true, Metric::ifs },
    { MET_DEFAULTS, "Number of Defaults", "DEFAULTS", true, Metric::defaults },
    { MET_LOOPS, "Number of Loops", "LOOPS", true, Metric::loops },
    { MET_SWITCHES, "Number of Switches", "SWITCHES", true, Metric::switches }
};

// Each of the above functions can use other metric types by calling
// Metric::compute_one_metric(type, sym, (void*)&tmp, node, m)
// where tmp is int or double containing the result. 

// Metrics computed for each entity kind. MET_END_ARRAY must terminate each of them.

metricType Metric::file_sequence[] =  { MET_CC, 
					MET_MYERS, 
					MET_SRC_LINE_NUM, 
					MET_BYTELEN,
					MET_DEPTH,
					MET_DANG_ELSEIF,
					MET_DANG_SWITCH,
					MET_RETURNS,
					MET_ULOC,
					MET_XLOC,
					MET_IFS,
					MET_SWITCHES,
					MET_LOOPS,
					MET_DEFAULTS,
					MET_END_ARRAY };

metricType Metric::class_sequence[] = { MET_FANIN, 
					MET_FANOUT,
					MET_START_LINE,
					MET_END_ARRAY  };

metricType Metric::func_sequence[] =  { MET_CC, 
					MET_MYERS,
					MET_SRC_LINE_NUM, 
					MET_BYTELEN, 
					MET_DEPTH,
					MET_DANG_ELSEIF,
					MET_DANG_SWITCH,
					MET_RETURNS,
					MET_START_LINE,
					MET_ULOC,
					MET_XLOC,
					MET_IFS,
					MET_SWITCHES,
					MET_LOOPS,
					MET_DEFAULTS,
					MET_END_ARRAY };

init_relational(Metric, Relational);

bool MetricElement::stored[MI_num][NUM_OF_METRICS];
metric_definition * Metric::def_array[NUM_OF_METRICS];
bool Metric::need_compute_static[MI_num][NUM_OF_METRICS];
bool Metric::need_compute_def[MI_num][NUM_OF_METRICS];

const char * C_METRICS = "";

static void metric_error(symbolPtr & sym, int type) {
    Initialize(metric_error);
}

int Metric::init_statics() {
    Initialize(Metric::init_statics);
    
    static int initialized = 0;

    if (initialized) return 1;
    initialized = 1;

    int i, idx;

    for (i=0; i<NUM_OF_METRICS; i++) def_array[definitions[i].type] = &definitions[i];

    for (idx = 0; idx < MI_num; idx++) 
      for (i=0; i<NUM_OF_METRICS; i++) need_compute_static[idx][i] = false;

    for (i=0; file_sequence[i] != MET_END_ARRAY; i++) 
      need_compute_static[MI_file][file_sequence[i]] = true;

    for (i=0; class_sequence[i] != MET_END_ARRAY; i++) 
      need_compute_static[MI_class][class_sequence[i]] = true;

    for (i=0; func_sequence[i] != MET_END_ARRAY; i++) 
      need_compute_static[MI_func][func_sequence[i]] = true;
    
    for (idx = 0; idx < MI_num; idx++) 
      for (i=0; i<NUM_OF_METRICS; i++) need_compute_def[idx][i] = need_compute_static[idx][i];
    return 1;
}

bool needLoad::need_load_files = true;


int Metric::compute_int_metric(metricType type, symbolPtr& sym) {
    // Compute an INTEGER-VALUED metric of a symbol
    Initialize(compute_metric);
    
    int value = -1;
    smtTree * node = NULL;
    Metric::def_array[type]->fcn(sym, node, NULL, (void*)&value);

    return value;
}

void Metric::need_to_compute(metricType type, bool flag) {
    Initialize(Metric::need_to_compute);

    if (uses_defaults) {
	uses_defaults = false;
	my_need_compute = new bool[NUM_OF_LINKS];
	for (int i = 0; i < NUM_OF_LINKS; i ++) my_need_compute[i] = need_compute[i];
    }
    my_need_compute[type] = need_compute[type] && flag;
}

int Metric::compute_one_metric_int(metricType type, symbolPtr& sym, smtTree *& node, 
				Relational* met) {
    Initialize(Metric::compute_one_metric_int);

    int res;
    compute_one_metric(type, sym, node, met, &res);
    return res;
}

void Metric::compute_one_metric(metricType type, symbolPtr& sym, smtTree *& node, 
				Relational* met, void * res) {
    Initialize(Metric::compute_one_metric);

    bool is_metric = (met && is_Metric(met));
    if (def_array[type]->type_is_int) {
	// Compute an integer metric.
	// First check if it has been computed already during model build.
	int val = -1;
	if (met) {
	    if (is_metric)
	      val = ((Metric*)met)->result[type].int_value;
	    else if (is_MetricElement(met))
	      val = ((MetricElement*)met)->get_value(type);
	}
	if (val<0)
	  // Ok, have to actually compute it:
	  def_array[type]->fcn(sym, node, met, (void*)&val);
	if (is_metric) ((Metric*)met)->result[type].int_value = val;
	if (res) *(int*)res = val;
	if (val<0) metric_error(sym, type);
    } else {
	// Double-valued metric.
	double val = -1;
	if (is_metric) val = ((Metric*)met)->result[type].dbl_value;
	
	if (val < 0)
	  def_array[type]->fcn(sym, node, met, (void*)&val);
	if (is_metric) ((Metric*)met)->result[type].dbl_value = val;
 	if (res) *(double*)res = val;
	if (val<0) metric_error(sym, type);
   }
}

void Metric::find_one_metric(metricType type, symbolPtr& sym, smtTree *& node, 
                              Relational* met, void * res) {
// Same as compute_one_metrics, but it only looks in the pmod
    Initialize(Metric::compute_one_metric);

    bool is_metric = (met && is_Metric(met));
    if (def_array[type]->type_is_int) {
      // Compute an integer metric.
      // First check if it has been computed already during model build.
      int val = -1;
      if (met) {
          if (is_metric)
            val = ((Metric*)met)->result[type].int_value;
          else if (is_MetricElement(met))
            val = ((MetricElement*)met)->get_value(type);
      }
      if (is_metric) ((Metric*)met)->result[type].int_value = val;
      if (res) *(int*)res = val;
      if (val<0) metric_error(sym, type);
    } else {
      // Double-valued metric.
      double val = -1;
      if (is_metric) val = ((Metric*)met)->result[type].dbl_value;

      if (is_metric) ((Metric*)met)->result[type].dbl_value = val;
      if (res) *(double*)res = val;
      if (val<0) metric_error(sym, type);
   }
}


void Metric::get_metric(metricType type, symbolPtr& sym0, void * res) {
    Initialize(Metric::get_metric);
    
    symbolPtr sym = sym0.find_xrefSymbol();
    
    smtTree * node = NULL;
    
    met_ent_kind kind;
    switch (sym.get_kind()) {
      case DD_MODULE:
	kind = MI_file;
	break;
      case DD_FUNC_DECL:
	kind = MI_func;
	break;
      case DD_CLASS:
	kind = MI_class;
	break;
      default:
	kind = MI_num;
    }

    //omazor: return -1 if we do not compute metrics
    if ( kind == MI_num || ! Metric::need_to_compute_def(kind, type) ) {
      if (type != MET_START_LINE && type != MET_SRC_LINE_NUM) {
	if (res) 
	  *(int*)res = -1;
	return;
      }
    }

    MetricElement el;
    if (def_array[type]->type_is_int) {

	MetricSet::get_metrics_of_sym(sym, el);

//	if (el.isnull()) 
//	    dis_message(C_METRICS, MSG_STDERR, "M_MET_NOT_STORED", sym.get_name());
//	    //Metrics for %s not in .met file or outdated. Recomputing.

    }
    compute_one_metric(type, sym, node, &el, res);
}

void Metric::get_metric_nocompute(metricType type, symbolPtr& sym0, void * res) {
    Initialize(Metric::get_metric);
    
    symbolPtr sym = sym0.find_xrefSymbol();
    
    smtTree * node = NULL;
    
    met_ent_kind kind;
    switch (sym.get_kind()) {
      case DD_MODULE:
      kind = MI_file;
      break;
      case DD_FUNC_DECL:
      kind = MI_func;
      break;
      case DD_CLASS:
      kind = MI_class;
      break;
      default:
      kind = MI_num;
    }

    //omazor: return -1 if we do not compute metrics
    if ( kind == MI_num || ! Metric::need_to_compute_def(kind, type) ) {
      if (type != MET_START_LINE && type != MET_SRC_LINE_NUM) {
	if (res) 
	  *(int*)res = -1;
	return;
      }
    }

    MetricElement el;
    if (def_array[type]->type_is_int) {
      MetricSet::get_metrics_of_sym_nocompute(sym, el);
    }
    find_one_metric(type, sym, node, &el, res);
}

void Metric::compute_all_metrics() {
    Initialize(Metric::compute_all_metrics);

    // compute all metrics of a symbol

    MetricElement el;
    MetricSet::get_metrics_of_sym(sym, el);

    smtTree * node = NULL;
    
    // Set all metrics as uncomputed
    
    int i;
    
    for (i = 0; i < NUM_OF_METRICS; i++) {
	if (def_array[i]->type_is_int) result[i].int_value = -1;
	else result[i].dbl_value = -1;
    }
    
    // Now set whatever values possible from the metric element:
    
    if (el.isnotnull())
      for (i=1; i < el.get_size(); i = el.next_index(i)) 
	if (el[i].type() < NUM_OF_METRICS) // This is an actual metric, not something internal
	  result[el[i].type()].int_value = el[i].get_index();
//      else
//        dis_message(C_METRICS, MSG_STDERR, "M_MET_NOT_STORED", sym.get_name());
//	    //Metrics for %s not in .met file or outdated. Recomputing.

    for (i = 0 ; seq[i] != MET_END_ARRAY; i++) {
	metricType type = seq[i];

	if (need_to_compute(type))
	  compute_one_metric(type, sym, node, this, NULL);
    }
}

Metric::Metric(symbolPtr& sm) : uses_defaults(true) {
    Initialize(Metric::Metric);

    sym = sm.find_xrefSymbol();
    switch(sym.get_kind()) {
      case DD_MODULE:
	need_compute = need_compute_static[MI_file];
	seq = file_sequence;
	break;
      case DD_CLASS:
	need_compute = need_compute_static[MI_class];
	seq = class_sequence;
	break;
      case DD_FUNC_DECL:
	need_compute = need_compute_static[MI_func];
	seq = func_sequence;
	break;
      default:
	msg("Illegal kind for Metric: $1 ") << sym.get_kind() << eom;
    }
    my_need_compute = need_compute;
    
}   

bool Metric::is_int(int index) { 
    Initialize(Metric::is_int);

    return def_array[index]->type_is_int; 
}

void Metric::set_value (metricType type, int val) {
    Initialize(Metric::set_value);

    result[type].int_value = val; 
}

void Metric::set_value (metricType type, double val) {
    Initialize(Metric::set_value);

    result[type].dbl_value = val; 
}

int Metric::get_i_value(metricType type) { 
    Initialize(Metric::get_i_value);

    return result[type].int_value; 
}


void extract_instances (MetricElement& iBlock, symbolArr& results, MetricSet * set) 
{
  int size = iBlock.get_size();
  for (int i=1; i<size; i=iBlock.next_index(i)) {
    metricType t = iBlock[i].type();
    if ((t == MET_INSTANCE) || (t == MET_DECL_INSTANCE)) {
      MetricElement imel;
      iBlock.extract(i,imel);
      symbolPtr s(&imel,set);
      results.insert_last(s);
      MetricElement moreInstances;
      imel.get_block(MET_INSTANCE_BLOCK, moreInstances); 
      if (!moreInstances.isnull()) {
	extract_instances(moreInstances, results, set);
      }
    }
  }
}

void extract_declarations (MetricElement& iBlock, symbolArr& results, MetricSet * set) 
{
  int size = iBlock.get_size();
  for (int i=1; i<size; i=iBlock.next_index(i)) {
    if (iBlock[i].type() == MET_DECL_INSTANCE) {
      MetricElement imel;
      iBlock.extract(i,imel);
      symbolPtr s(&imel,set);
      results.insert_last(s);
    }
  }
}

void Metric::get_instance_list(symbolPtr& mod, symbolPtr& sym, symbolArr& results)
{
  Initialize(Metric::get_instance_list);
    
  results.removeAll();
  symbolPtr module = mod.get_def_symbol();
  if(module.isnull())
    return;
  
  Xref*xr = mod.get_xref();
  if(!xr)
    return;
  
  symbolPtr xsym = xrefEntry::local_symbol(xr, sym);
  if(xsym.isnull())
    return;
  
  MetricElement mel;
  MetricSet::get_metrics_of_sym(module, mel);    
  if(mel.isnull())
    return;
  
  int offset = xsym.get_offset();
  
  int size = mel.get_size();
  bool flag = 0;
  int i=0;
  if (mel.get_value(MET_OBJECT) == offset) 
      flag = 1;
  else {
    MetricElement temp;
    mel.get_block(MET_CHILDREN, temp);
    mel = temp;
    size = mel.get_size();
    i=1;
    while (i<size && !flag) {
      MetricElement testmel;
      mel.extract(i,testmel);      
      if (testmel.get_value(MET_OBJECT) == offset)
	flag = 1;
      else
	i=mel.next_index(i);
    }
  }
  
  if (flag) {
    metricType t = mel[i].type();
    if ((t != MET_UNDEFINED_ELEMENT) && (t != MET_ELEMENT)) return;
    
    projNode * pr = module.get_xref()->get_projNode();
    MetricSet * set = proj_get_set(pr);
    
    if (set) {
      MetricElement myMel;
      mel.extract(i,myMel);
      size = myMel.get_size();
      for (i=1; (i<size) && (myMel[i].type() != MET_INSTANCE_BLOCK) && (myMel[i].type() != MET_LOCAL_DEC_BLOCK); i=myMel.next_index(i)) {};
      
      if (i<size) {
	MetricElement iBlock;
	myMel.extract(i,iBlock);
	extract_instances(iBlock,results,set);
      }
    }
  }
}


MetricElement Metric::get_mel_of_undefined_sym(symbolPtr& module, symbolPtr& sym)
{
    Initialize(Metric::get_mel_of_undefined_sym);
    
    MetricElement result;

    if (sym.is_instance()) 
	result = sym.get_MetricElement();
    else {
	symbolPtr xsym = sym.get_xrefSymbol();
	MetricElement mel;
	MetricSet::get_metrics_of_sym(module, mel);
	
      if (!xsym.xrisnull() && !mel.isnull()) {
	  int offset = xsym.get_offset();
	  int size   = mel.get_size();
	  bool flag  = 0;
	  int i      = 0;
	  if (mel.get_value(MET_OBJECT) == offset) 
	      flag = 1;
	  else {
	      MetricElement temp;
	      mel.get_block(MET_CHILDREN, temp);
	      mel = temp;
	      size = mel.get_size();
	      i=1;
	      while (i<size && !flag) {
		  MetricElement testmel;
		  mel.extract(i,testmel);      
		  if (testmel.get_value(MET_OBJECT) == offset)
		      flag = 1;
		  else
		      i=mel.next_index(i);
	      }
	  }
	  
	  if (flag) {
	      metricType t = mel[i].type();
	      if ((t == MET_UNDEFINED_ELEMENT) || (t == MET_ELEMENT))
		  mel.extract(i,result);
	  }
      }
  }
    return result;
}

void Metric::get_instances(MetricElement mel, MetricSet* set, symbolArr& results)
{
  results.removeAll();
  metricType t = mel[0].type();
  if ((t == MET_UNDEFINED_ELEMENT) || (t == MET_ELEMENT) || (t == MET_DECL_INSTANCE)) {
    if (set) {
      int size = mel.get_size();
      int i;
      for (i=1; (i<size) && (mel[i].type() != MET_INSTANCE_BLOCK) && (mel[i].type() != MET_LOCAL_DEC_BLOCK); i=mel.next_index(i)) {};
      if (i<size) {
	MetricElement iBlock;
	mel.extract(i,iBlock);
	extract_instances(iBlock,results,set);
      }
    }
  }
  if (t == MET_DECL_INSTANCE || t == MET_INSTANCE) {
      symbolPtr s(&mel,set);
      results.insert_last(s);
  }
}

void Metric::get_decls(MetricElement mel, MetricSet* set, symbolArr& results)
{
  results.removeAll();
  metricType t = mel[0].type();
  if ((t == MET_UNDEFINED_ELEMENT) || (t == MET_ELEMENT)) {
    if (set) {
      int size = mel.get_size();
      int i;
      for (i=1; (i<size) && (mel[i].type() != MET_LOCAL_DEC_BLOCK); i=mel.next_index(i)) {};
      if (i<size) {
	MetricElement iBlock;
	mel.extract(i,iBlock);
	extract_declarations(iBlock,results,set);
      }
    }
  }
}


char* Metric::get_instance_info(symbolPtr& msym)
{
  static genString info;
  info = "\0";
  symbolPtr xsym = msym.get_xrefSymbol();
  if (xsym.xrisnotnull())
    info += xsym.get_name();
  if (msym.is_instance()) {
    info += " [";
    symbolPtr scope = msym.scope();
    symbolPtr mod;
    if (scope.xrisnotnull())
      mod = scope->get_def_file();
    int line = msym.line_number();
    char const *mname = mod.get_name();
    if (mname) {
      info += mname;
      info += " : ";
    }       
    char buf[1024];
    OSapi_sprintf(buf,"%d\0",line);
    info += buf;
    info += "]\0";
  }
  return (char*) info;
}

double Metric::get_f_value(metricType type) { 
    Initialize(Metric::get_f_value);

    return result[type].dbl_value; 
}

bool Metric::need_to_compute(metricType type) { 
    Initialize(Metric::need_to_compute);

    return my_need_compute[type]; 
}

void Metric::need_to_compute_static(int idx, int type, bool flag) {
    Initialize(Metric::need_to_compute_static);

    if (type < NUM_OF_METRICS)
      need_compute_static[idx][type] = need_compute_def[idx][type] && flag; 
    else {
	for (int i = 0; i < NUM_OF_METRICS; i++)
	  need_compute_static[idx][i] = need_compute_def[idx][i] && flag; 
    }
}

bool Metric::need_to_compute_static(int idx, int type) { 
    Initialize(Metric::need_to_compute_static);

    return need_compute_static[idx][type]; 
}

bool Metric::need_to_compute_def(int idx, int type) { 
    Initialize(Metric::need_to_compute_def);

    return need_compute_def[idx][type]; 
}

const char * Metric::name() { 
    Initialize(Metric::name);

    return def_array[seq[cur_idx]]->name; 
}

const char * Metric::abbrev() { 
    Initialize(Metric::abbrev);

    return def_array[seq[cur_idx]]->abbrev; 
}

int Metric::int_value() { 
    Initialize(Metric::int_value);

    return result[seq[cur_idx]].int_value; 
}

double Metric::dbl_value() { 
    Initialize(Metric::dbl_value);

    return result[seq[cur_idx]].dbl_value; 
}

bool Metric::is_int() { 
    Initialize(Metric::is_int);

    return def_array[seq[cur_idx]]->type_is_int; 
}

bool Metric::is_computed() { 
    Initialize(Metric::is_computed);

    if (def_array[seq[cur_idx]]->type_is_int)  
	return (result[seq[cur_idx]].int_value>=0);
    else
      return (result[seq[cur_idx]].dbl_value>=0); 
} 

Metric::~Metric() { 
    if ( !uses_defaults) delete my_need_compute; 
}
