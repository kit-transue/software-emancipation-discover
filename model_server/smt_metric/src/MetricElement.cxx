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
#include <metric.h>
#ifndef ISO_CPP_HEADERS
#include <stdio.h>
#else /* ISO_CPP_HEADERS */
#include <cstdio>
using namespace std;
#endif /* ISO_CPP_HEADERS */
#include <fileEntry.h>
#include <cLibraryFunctions.h>
#include <_Xref.h>
#include <xrefSymbol.h>
#include <ddict.h>

// The list of metrics that are to be computed during model build for 
// classes, functions, and files. 

metricType MetricElement::class_array[] = {
    MET_START_LINE,
    MET_SRC_LINE_NUM,
    MET_FANIN, 
    MET_FANOUT,
    MET_END_ARRAY
  };
metricType MetricElement::func_array[] = {
    MET_CC,
    MET_SRC_LINE_NUM,
    MET_MYERS,
    MET_START_LINE,
    MET_DEPTH,
    MET_DANG_ELSEIF,
    MET_RETURNS,
    MET_BYTELEN, 
    MET_DANG_SWITCH,
    MET_ULOC,
    MET_XLOC,
    MET_IFS,
    MET_SWITCHES,
    MET_LOOPS,
    MET_DEFAULTS,    
    MET_END_ARRAY
  };
metricType MetricElement::file_array[] = {
    MET_CC,
    MET_SRC_LINE_NUM,
    MET_MYERS,
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
    MET_END_ARRAY
  };

init_relational(MetricElement, Relational);

// Variables defined in xrefSymbol.h.C. The following two functions 
// stolen from the same file.
static const int datemask=0xffffff;    // mask for low part of date field
static const int dateshift=24;  	  // number of bits to shift for high part

void MetricElement::set_last_mod_date(time_t date) {
    Initialize(MetricElement::set_last_mod_date);

    set_value(MET_lmd,     (unsigned int)date & datemask);
    set_value(MET_lmdhigh, ((unsigned int)date) >> dateshift);
}

#define BAD_PRIV_TABLE(name, t)  fprintf(stderr, "Metric WARNING: %s called with priv_table containing %d elements.\n", quote(name), t->size())

time_t MetricElement::get_last_mod_date() {
    Initialize(MetricElement::get_last_mod_date);

    unsigned int high = get_value(MET_lmdhigh);
    unsigned int low = get_value(MET_lmd);
    if (high==-1) return 0;
    if (!(high|low)) low=1;
    time_t result =  (low & datemask) | (high << dateshift);
    return result;
}

int MetricElement::init_statics() {
    Initialize(MetricElement::init_statics);
    
    static int initialized = 0;
    
    if (initialized) return 1;
    initialized = 1;

    int i, j;
    for (i = 0; i < MI_num; i++)
      for (j=0; j < NUM_OF_METRICS; j++) 
	stored[i][j] = false;
    
    for (i=0; func_array[i] != MET_END_ARRAY; i++) stored[MI_func][func_array[i]] = true;
    for (i=0; file_array[i] != MET_END_ARRAY; i++) stored[MI_file][file_array[i]] = true;
    for (i=0; class_array[i] != MET_END_ARRAY; i++) stored[MI_class][class_array[i]] = true;

    return 1;
}

void MetricElement::set_value(metricType type, int value, bool overwrite) {
//    Initialize(MetricElement::set_value);
    
    // Don't store '-1's for normal metrics.
    if ((value<0) && (type < NUM_OF_METRICS)) return;  

    int size = get_size();
    int i = size;
    
    if (overwrite)
      for (i=1; i<size && (*this)[i].type() != type; i=next_index(i));
    
    if (i >= size) {
	// Add new type
	// look for an empty spot
	for (i=1; i<size && (*this)[i].type() != MET_FREE; i=next_index(i));
	if (i>=size) make_room(size+1);
	if ((*this)[i].is_block() && (*this)[i].get_index()>1) 
	  // This is a start of empty block; adjust.
	  set_free_block(i+1, (*this)[i].get_index()-1);
	(*this)[i].assign(type, 0);
    }
    (*this)[i].set_index(value);
}

void MetricElement::compute_set_values(symbolPtr& sym) {
    Initialize(MetricElement::compute_set_value);
    
    metricType * array = NULL;
    switch (sym.get_kind()) {
      case DD_MODULE:
	array = file_array;
	break;
      case DD_FUNC_DECL:
	array = func_array;
	break;
      case DD_CLASS:
	array = class_array;
	break;
      default:
	compute_set_val(MET_START_LINE, sym);
	compute_set_val(MET_SRC_LINE_NUM, sym);
	return;
    }
    for (int i = 0; array[i] != MET_END_ARRAY; i++) 
      compute_set_val(array[i], sym);
}

int MetricElement::get_value(metricType type) {
    Initialize(MetricElement::get_value);
    
    if (isnull()) return -1;
    int i;
    int size = get_size();
    for (i=1; i<size && (*this)[i].type() != type; i=next_index(i));
    
    if (i<size) return (*this)[i].get_index();
    else return -1; // value not found
}

void MetricElement::extract(int idx, MetricElement & mel) {
    Initialize(MetricElement::extract);

    if (!(*this)[idx].is_block()) mel.set();    

    else {
	if (priv_table) {
	    BAD_PRIV_TABLE(MetricElement::extract, priv_table);
	    mel.set((*this)[idx].type());
	    int size = (*this)[idx].get_index();
	    mel.make_room(size);
	    for (int i = 1; i<size; i++) mel[i] = (*this)[i+idx];
	}  else  {
	    mel.set(table, offset + idx);
	}
    }
}

void MetricElement::get_block(metricType type, MetricElement & mel) {
    Initialize(MetricElement::get_block);
    
    int i;
    int size = get_size();
    for (i=1; i<size && (*this)[i].type() != type; i=next_index(i));
    
    if (i<size) extract(i, mel);

    else mel.set();
}

bool MetricElement::value_exists(metricType type) {
    Initialize(MetricElement::value_exists);
    
    int i;
    int size = get_size();
    for (i=1; i<size && (*this)[i].type() != type; i=next_index(i));
    return (i<size);
}


void MetricElement::get() {
    Initialize(MetricElement::get);

    if (!table) return;
    if (priv_table) delete priv_table;
    int size = get_size();
    priv_table = new MetricTable(size);
    for (int i=0; i<size; i++) *(*priv_table)[i] = *(*table)[offset+i];
}

void MetricElement::make_room_in_table(int sz, bool look_for_space) {
    Initialize(MetricElement::make_room_in_table);

    int new_sz, old_sz=0;
    if (offset >= 0 && (*table)[offset]->is_block()) {
	old_sz = get_size_in_table();
	if (sz <= old_sz) {
	  if (sz < old_sz)
	    table->set_free_block(offset+sz, old_sz-sz);
	  return;
	}
	int i;
	for (i=old_sz; 
	     i < sz && i+offset<table->size() && (*table)[offset+i]->type() == MET_FREE; 
	     i = next_index(i));
	if (i>=sz) {
	    (*table)[offset]->set_index(sz);
	    table->set_free_block(offset+sz, i-sz);
	    return;
	}
	new_sz = new_size(i, sz);
	if (i+offset == table->size()) {
	    table->grow(new_sz - i);
	    table->set_free_block(offset+sz, new_sz-sz);
	    table->set_free_block(offset+old_sz, sz);
	    (*table)[offset]->set_index(sz);
	    return;
	}
    } else new_sz = sz;
    int new_offset;
    if (!look_for_space) {
	new_offset = table->size();
	table->grow(new_sz);
    } else {
	// Otherwise, search the entire table to find a free block of size new_sz.
	int max_sz = 0;
	int cur_idx;
	for ( int i=0; i<table->size() && max_sz < new_sz; i = next_index(i)) {
	    if ((*table)[i]->type() != MET_FREE) max_sz = 0; 
	    else {
		if (!max_sz) cur_idx = i;
		max_sz += (*table)[i]->get_index();
	    }
	}
	if (max_sz >= new_sz) {
	    new_offset = cur_idx;
	    new_sz = max_sz;
	} else {
	    new_offset = table->size();
	    table->grow(new_sz);
	}
    }
    if (offset >= 0) {
	// Copy old onto new, making sure not to copy the content of empty blocks.
	for (int i=0; 
	     i<old_sz; 
	     i = ((*table)[new_offset+i]->type() == MET_FREE) ? next_index(i) : i+1) 
	  *(*table)[new_offset+i] = *(*table)[offset+i];
	
	table->set_free_block(new_offset+old_sz, new_sz-old_sz);
	if (table->notify) {
	    table->set_free_block(offset+1, old_sz-1);
	    (*table)[offset]->assign(MET_MOVED, 0, new_offset);
	} else
	  table->set_free_block(offset, old_sz);
	(*table)[new_offset]->set_index(sz);
    } else {
	table->set_free_block(new_offset+1, new_sz-1);
	(*table)[new_offset]->assign(MET_ELEMENT, 1, sz);
    }
    offset = new_offset;
}

void MetricElement::make_room(int sz) {
//    Initialize(MetricElement::make_room);

    if (!priv_table)
	make_room_in_table(sz);
    else {
	int old_sz = priv_table->size();
	if (sz > old_sz) {
	    int new_sz = new_size(old_sz, sz);	
	    priv_table->grow(new_sz-old_sz);
	    set_free_block(old_sz, sz-old_sz);
	    set_free_block(sz, new_sz-sz);
	    (*priv_table)[0]->set_index(sz);
	}
    }
}

void MetricElement::put(MetricTable * tbl, int off) {
    Initialize(MetricElement::put);
    
    if (!priv_table) return;
    
    if (tbl) {
	table = tbl;
	offset = off;  // off == -1 means "find room for a NEW element"
    }
    if (!table) return;
    int size = get_size();
    make_room_in_table(size);   // now offset is positive
    for (int i = 0; i < size; i++) *(*table)[offset+i] = *(*priv_table)[i];
}

void MetricElement::append(MetricElement & mel) {
    Initialize(MetricElement::append);

    int my_size = get_size();
    int add_size = mel.get_size();
    make_room(my_size + add_size);
    for (int i = 0; i<add_size; i++) operator[](my_size+i) = mel[i];
    operator[](0).set_index(my_size + add_size);
}

void MetricElement::set() {
    Initialize(MetricElement::set());
    table = NULL;
    offset = 0;
    priv_table = NULL;
}
  
// create an empty block of a certain type
void MetricElement::set(metricType block_type) {
    Initialize(MetricElement::set(metricType));

    table = NULL;
    offset = 0;
    priv_table = new MetricTable(1); 
    (*priv_table)[0]->assign(block_type, 1, 1);
}

void MetricElement::set(MetricTable * tbl, int off) {
    Initialize(MetricElement::set(MetricTable*,int));

    table = tbl;
    offset = off;
    priv_table = NULL;
}

// create an empty block of a certain type
MetricElement::MetricElement(metricType block_type) {
    Initialize(MetricElement::MetricElement);

    set(block_type);
}

MetricElement::MetricElement(symbolPtr& sym) {
    Initialize(MetricElement::MetricElement);

    table = NULL;
    offset = 0;
    priv_table = new MetricTable(2); 
    (*priv_table)[0]->assign(MET_ELEMENT, 1, 2);
    symbolPtr xsym = sym.get_xrefSymbol();
    (*priv_table)[1]->assign(MET_OBJECT, 0, xsym.get_offset()); 
}

MetricElement::MetricElement(const MetricElement & ob) {
    Initialize(MetricElement::MetricElement);

    table = ob.table;
    offset = ob.offset;
    if (ob.priv_table) {
	BAD_PRIV_TABLE("MetricElement::MetricElement(MetricElement&)", ob.priv_table);
	int size = ob.priv_table->size();
	priv_table = new MetricTable(size);
	for (int i = 0; i<size; i++) *(*priv_table)[i] = *(*ob.priv_table)[i];
    } else priv_table = NULL;
}

int MetricTable::update_index(int & i) { 
    Initialize(MetricTable::update_index);

    while (operator[](i)->type() == MET_MOVED) 
      i = operator[](i)->get_index(); 
    return i; 
}

int MetricTable::compress() { 
    Initialize(MetricTable::compress);

    return compress_priv(0, size(), 0); 
}   

int MetricTable::next_index(int i) {
    Initialize(MetricTable::next_index);
    
    if (operator[](i)->is_block())
      return i+operator[](i)->get_index();
    else return i+1; 
}

void MetricTable::set_free_block(int from, int len) { 
//    Initialize(MetricTable::set_free_block);

    if (len>0) ptr[from].assign(MET_FREE, 1, len); 
}

MLink & MetricElement::operator[](int idx) { 
    return priv_table ? *(*priv_table)[idx] : *(*table)[offset+idx]; 
}

void MetricElement::set_int_at(int i, int val) { 
    Initialize(MetricElement::set_int_at);

    operator[](i).set_index(val); 
}

int  MetricElement::get_int_at(int i) { 
    return operator[](i).get_index(); 
}

void MetricElement::set_char_at(int i, int d, uchar val) { 
    *((uchar *)(&operator[](i))+d) = val; 
}

uchar  MetricElement::get_char_at(int i, int d) { 
    return *((uchar *)(&operator[](i))+d); 
}

int MetricElement::next_index(int i) {
    return operator[](i).is_block() ? i+operator[](i).get_index() : i+1; 
}

int MetricElement::get_size() { 
   return operator[](0).get_index();
}

void MetricElement::compute_set_val(metricType type, symbolPtr& sym) {
//    Initialize(MetricElement::compute_set_val);

    set_value(type, Metric::compute_int_metric(type, sym));
}

void MetricElement::set_free_block(int from, int len) { 
//    Initialize(MetricElement::set_free_block);

    if (len>0) (*this)[from].assign(MET_FREE, 1, len); 
}

MetricElement::~MetricElement() { if (priv_table) delete priv_table; }

MetricElement MetricElement::FindParentElement()
{
  Initialize(MetricElement::FindParentElement());
  MetricElement result;
  int count =1;
  
  // go backwards to find first block containing this
  while ((offset-count >= 0) && 
	 !(operator[](-count).is_block() && (operator[](-count).get_index() > count)))
    count ++;

  if (offset-count >= 0) {
    result = *this;
    result.offset = offset-count;
  }
  return result;
}

MetricElement MetricElement::inst_get_decl()
{
  Initialize(MetricElement::inst_get_decl);
  MetricElement result;
  if (isnotnull() && (operator[](0).is_block())) {
    if (operator[](0).type() == MET_DECL_INSTANCE)
      result = *this;
    else {
      MetricElement dec = FindParentElement();
      if (dec.isnotnull() && (dec[0].type() == MET_INSTANCE_BLOCK))
	dec = dec.FindParentElement();
      if (dec.isnotnull() && (dec[0].type() == MET_DECL_INSTANCE))
	result = dec;
    }
  }
  return result;
}

extern symbolPtr api_lookup_xrefSymbol(ddKind kind, char const *name);

symbolPtr MetricElement::get_instance(char const *file, int off)
{
  symbolPtr result;
  symbolPtr mod = api_lookup_xrefSymbol(DD_MODULE, file); // this will get us any xref symbol for specified  module 
  // we are interested in actual definition to pickup correct pmod
  MetricElement mel;
  if (mod.isnotnull()) {
      mod = mod.get_def_symbol();
      MetricSet::get_metrics_of_sym(mod, mel);
  }
  MetricElement inst;
  if (mel.isnotnull())
    inst = mel.find_inst_by_offset( mod , off); //NEW arg mod

  if (inst.isnotnull()) {
    projNode* pr = mod.get_projNode();
    
    MetricSet * set = NULL;
    if (pr)
      set = proj_get_set(pr);
    if (set)    
      result = symbolPtr(&inst,set);
  }
  return result; 
}

symbolPtr MetricElement::get_instance(char const *file, int line, int line_offset)
{
  symbolPtr result = NULL_symbolPtr;

  symbolPtr any_mod = api_lookup_xrefSymbol(DD_MODULE, file); // this will get us any xref symbol for specified  module 
  // we are interested in actual definition to pickup correct pmod
  symbolPtr mod;
  if(any_mod.isnotnull())
      mod = any_mod.get_def_symbol();
  MetricElement mel;
  if (mod.isnotnull())  
    MetricSet::get_metrics_of_sym(mod, mel);

  MetricElement inst;
  if (mel.isnotnull())
    inst = mel.find_inst_by_line_offset( mod , line , line_offset); //NEW arg mod

  if (inst.isnotnull()) {
    projNode* pr = mod.get_projNode();
    
    MetricSet * set = NULL;
    if (pr)
      set = proj_get_set(pr);
    if (set)    
      result = symbolPtr(&inst,set);
  }
  return result; 
}
//NEW BLOCK BEGIN

int MetricElement::get_inst_symbol_length( symbolPtr& mod ) const
{
  int result = 0;

   projNode* pr = mod.get_projNode();
    
   MetricSet * set = NULL;
   if (pr)
     set = proj_get_set(pr);
   if (set){    
     symbolPtr sym = symbolPtr( ( MetricElement * ) this, set );
     if( sym.is_instance() ){
       for( char const *ptr = sym.get_name() ; *ptr ; ptr++ ){
          switch( *ptr ){
             case '(':
               return result;
              case ':':
                result = 0;
                break;
	      default:
		result++;
	  }
       } 
     }
   }

  return result;
}

void MetricElement::get_instances(char const *file, int offStart , int offEnd , symbolArr& arr)
{
  symbolPtr mod = api_lookup_xrefSymbol(DD_MODULE, file);

  MetricElement mel;
  if (mod.isnotnull())  
    MetricSet::get_metrics_of_sym(mod, mel);

  if( mel.isnotnull() ){
    mel.find_inst_between_offsets( mod , offStart , offEnd , arr);  
  }
}


void MetricElement::find_inst_between_offsets( symbolPtr& mod , int startOffset , int endOffset , symbolArr& arr)
{
    metricType t = operator[](0).type();

    if ((t == MET_INSTANCE) || (t == MET_DECL_INSTANCE)) {
	int off = get_value(MET_INSTANCE_OFFSET); 
	int length = get_value(MET_INSTANCE_LENGTH);
	
	if( length < 0 )  
	    length = get_inst_symbol_length( mod );
	
	if (endOffset == -1 || (( startOffset < off + length )  && off <= endOffset)){
	    projNode* pr = mod.get_projNode();
	    
	    MetricSet * set = NULL;
	    if (pr)
		set = proj_get_set(pr);
	    if (set){    
		symbolPtr  result = symbolPtr(this,set);
		arr.insert_last( result );
	    }
	}
    }
    
    // recurse
    int size = get_size();
    for (int i = 1; i < size ; i=next_index(i)) {
	if (operator[](i).is_block()) {
	    MetricElement imel;
	    extract(i,imel);
	    imel.find_inst_between_offsets(mod, startOffset, endOffset, arr);
	}
    }
}
//NEW BLOCK END

MetricElement MetricElement::find_inst_by_offset(symbolPtr& mod , int off)
{
  MetricElement result;
  metricType t = operator[](0).type();
  if ((t == MET_INSTANCE) || (t == MET_DECL_INSTANCE)) {
    int offBeg = get_value(MET_INSTANCE_OFFSET); 
    int length = get_value(MET_INSTANCE_LENGTH);

    if( length < 0 )  
	length = get_inst_symbol_length( mod );

    if( offBeg <= off &&  ( off < offBeg + length ) )
      result =  *this;
  }

  // recurse
  int size = get_size();
  for (int i=1; i<size && result.isnull() ; i=next_index(i)) {
    if (operator[](i).is_block()) {
      MetricElement imel;
      extract(i,imel);
      result = imel.find_inst_by_offset(mod, off);
    }
  }
  return result;
}

MetricElement MetricElement::find_inst_by_line_offset(symbolPtr& mod , int line , int line_offset)
{
  MetricElement result;
  metricType t = operator[](0).type();
  if ((t == MET_INSTANCE) || (t == MET_DECL_INSTANCE)) {
    int ln = get_value(MET_INSTANCE_LINE);

    if (line == ln) {
      int loff = get_value(MET_INSTANCE_LN_OFFSET); 
      if (line_offset == loff)
        result = *this;
      else if (line_offset > loff) {
        int length = get_value(MET_INSTANCE_LENGTH);
        if (length < 0)
          length = get_inst_symbol_length(mod);
        if (line_offset < loff + length)
          result = *this;
      }
    }
  }

  // recurse
  int size = get_size();
  for (int i=1; i<size && result.isnull() ; i=next_index(i)) {
    if (operator[](i).is_block()) {
      MetricElement imel;
      extract(i,imel);
      result = imel.find_inst_by_line_offset(mod, line, line_offset);
    }
  }
  return result;
}

void MetricElement::set_ast_attribute(uint value, uint bitnum, uint numbits)
{
    Initialize(MetricElement::set_ast_attribute);
    IF (numbits==0 || bitnum+numbits>24 || value >= (1<<numbits)) {
        // this indicates a coding error in calling function
        return;
    }
    
    int old = get_value(MET_AST_CATEGORY);  // get link locally
    uint oldvalue = 0;
    if (old != -1)
      oldvalue = (uint) old;

    xrefSymbol::set_attribute(oldvalue, value, bitnum, numbits);
    set_value(MET_AST_CATEGORY,oldvalue);
}

uint MetricElement::get_ast_attribute(uint bitnum, uint numbits)
{
    Initialize(MetricElement::get_ast_attribute);
    IF (bitnum+numbits>24)                 // this indicates a coding error in the calling function
        return 0;
    int cur_attribute = get_value(MET_AST_CATEGORY);
    if (cur_attribute == -1)
	return 0;
    uint xr_attribute = (uint)cur_attribute;
    return xrefSymbol::get_attribute(xr_attribute, bitnum, numbits);
}




