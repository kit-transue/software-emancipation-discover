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
#include <msg.h>
#include <_Xref.h>
#include <xrefSymbol.h>
#include <machdep.h>
#include <proj.h>
#include <messages.h>
#include "_Xref.h"
#include "XrefTable.h"
#include "genError.h"

init_relational(MetricSet, Relational);
init_rel_or_ptr(MetricSet,set,NULL,projNode,proj,NULL);

MetricSet::MetricSet(projNode * pr)
{
    Initialize(MetricSet::MetricSet);
    
    proj_put_set(pr, this);
    load_table();
}

MetricSet::~MetricSet() 
{
    Initialize(MetricSet::~MetricSet);
}

bool MetricSet::load_table() {
    Initialize(MetricSet::load_table);

    // Read the table from file:

    projNode * pr = set_get_proj(this);
    file_name     = pr->sym_name->get_name();

    // remove the extension
    ((char*)file_name)[file_name.length()-3] = '\0';

    // and add our extension
    file_name += "met";
    
    FILE * file = NULL;
    struct OStype_stat buf;
    if ( OSapi_stat((char*)file_name, &buf) ) return false;
    int size = (int)buf.st_size / sizeof(MLink);
    
    file = OSapi_fopen((char*)file_name, "rb");
    
    if (!file)	return false;

    set.table.provide(size);
    set.table.reset();
    set.table.grow(size);

    if (OSapi_fread(set.table[0], sizeof(MLink), size, file) < size) {
	set.table.reset();
	OSapi_fclose(file);
	file = NULL;
	return false;
    }
    
    OSapi_fclose(file);
    file = NULL;
    // Now copy the content of the Table into the Hash table:
    for (int i = 0; i<size; i = set.table.next_index(i)) 
      if (set.table[i]->type() == MET_ELEMENT && set.table.next_index(i) <= size) {
	  MetricElement mel(&set.table, i);
	  add_element (mel);
       }

    return true;
}

void MetricSet::add_element(MetricElement & el, int old_offset) {
    Initialize(MetricSet::add_element);

    el.put(&set.table, old_offset);
    el.unget();


    MetricHash * mh = new MetricHash(el.get_offset());
    
    {
      static int metrichash_ok = 1;
      if (metrichash_ok && set.findObjectWithKey(*mh)) {
	msg("Attempt to reinsert a metric that already exists for this model.", catastrophe_sev) << eom;
	metrichash_ok = 0;
      }
    }
    set.add(*mh);
    
    // Now also add the children !

    MetricElement bmel;
    el.get_block(MET_CHILDREN, bmel);
    if (bmel.isnotnull())
      for (int i = 1; i < bmel.get_size(); i = bmel.next_index(i)) 
	if ((bmel[i].type() != MET_FREE)               && 
	    (bmel[i].type() != MET_UNDEFINED_ELEMENT)) { 
	  MetricHash * chmh = new MetricHash(bmel.get_offset()+i);

    DBG
    {
      //The error message probably means that the pmod symbol is already
      //in metrics hash. We'll need to debug this piece of code since the 
      //message appeared many times at customer sites and here with aset
      // model == mstarets

      if (set.findObjectWithKey(*chmh)) {
	msg("Inserting existing MetricHash! (ch)", error_sev) << eom;
      }
    }
    set.add ( *chmh );
  }
}

void MetricSet::add_element(symbolPtr& sym, int old_offset, MetricElement * mel) {
    Initialize(MetricSet::add_element);

    MetricElement el(sym); 
    el.put(&set.table, old_offset);
    el.unget();

    MetricHash * mh = new MetricHash(el.get_offset());
    set.add(*mh);
    if (mel) (*mel) = el;
}

void MetricSet::find_element (symbolPtr& sym, MetricElement & mel) {
    Initialize(MetricSet::find_element);
    mel.set();

    //boris: symbol must belongs to the project, that set corresponds to.
    MetricHash el(sym.get_offset(),1);
    MetricHash * new_el = (MetricHash*) set.findObjectWithKey(el);
    if (new_el)
	mel.set(&set.table, (new_el->offset) >> 1);
}

void MetricSet::remove_element(symbolPtr& sym, bool only_children) {
    Initialize(MetricSet::remove_element);

    MetricElement mel;
    find_element(sym, mel);
    if (mel.isnotnull()) {
	MetricElement bmel;
	mel.get_block(MET_CHILDREN, bmel);
	if (bmel.isnotnull()) {
	    for (int i = 1; i < bmel.get_size(); i = bmel.next_index(i)) 
	      if ((bmel[i].type() != MET_FREE) &&
		  (bmel[i].type() != MET_UNDEFINED_ELEMENT)){ 
		  MetricHash chmh(bmel[i+1].get_index(),1);
		  MetricHash * found = (MetricHash*)set.remove(chmh);
		  if (found) 
		    delete found;
	      }
	    bmel.set_free_block(0, bmel.get_size());
	}
	if (!only_children) {
	    MetricHash* mh = new MetricHash(sym.get_offset(),1);
	    Object* oo = set.remove(*mh);
	    delete mh;
	    MetricHash * found = (MetricHash*)oo;
	    if (found) delete found;
	    // declare this space as free
	    mel.set_free_block(0, mel.get_size());
	}
    }
}

void MetricElement::clear() {
    Initialize(MetricElement::clear);
    
    (*this)[0].set_index(2);
    set_free_block(2, get_size()-2);
}

int MetricTable::compress_priv(int offset_from, int len, int offset_to) {
    // compress a table segment and return the new length 
    Initialize(MetricTable::compress_priv);
    
    int to = offset_to;
    int new_len = 0;

    for(int from = offset_from; from < offset_from + len; ) 
      if (ptr[from].type() != MET_FREE && ptr[from].type() != MET_MOVED) {
	  if ( ! ptr[from].is_block() ) {
	      ptr[to++] = ptr[from++];
	      new_len ++;
	  } else {
	      // A block must be compressed and copied down the table
	      metricType type = ptr[from].type();
	      int old_block_len = ptr[from].get_index();
	      int block_len;
	      if (do_not_compress(type)) {
		  for (int i = 1; i < old_block_len; i++) ptr[to+i] = ptr[from+i];
		  block_len = old_block_len;
	      } else
		block_len = compress_priv(from+1, old_block_len-1, to+1) + 1;
	      ptr[to].assign(type, 1, block_len);
	      from += old_block_len;
	      to += block_len;
	      new_len += block_len;
	  }
      } else 
	from = next_index(from);
    
    return new_len;
}

void MetricSet::get_metrics_of_sym(symbolPtr& sym0, MetricElement & mel) {
    Initialize(get_metrics_of_sym);
    mel.set();

    symbolPtr sym = sym0.get_def_symbol();
    if (sym.isnull())
	return;

    ddKind knd = sym.get_kind();

    //MG 970924: definition must exist for this function
    symbolPtr def_file = sym->get_def_file();
    if (def_file.isnull())
	return;

    projNode *pr   = def_file.get_xref()->get_projNode();
    if (!pr)
	return;

    MetricSet *set = proj_get_set(pr);
    if (!set)
	set = new MetricSet(pr);
  
    if (needLoad::ask()) {
	// Check whether metrics exist for DD_MODULE def_file. 
	set->find_element(def_file, mel);
	    
	if (mel.isnull()) {

	    // No metrics found in the set, so compute them and write to the set.
	    // Load the file, compute metrics, save them, unload the file.
	    get_metrics_of_sym_aux(def_file, sym);

        }
    }
    
    if (mel.isnull() || knd != DD_MODULE)
	set->find_element(sym, mel);
}

void MetricSet::get_metrics_of_sym_nocompute(symbolPtr& sym0, MetricElement & mel) 
{
  Initialize(get_metrics_of_sym_nocompute);
  mel.set();

  symbolPtr sym = sym0.get_def_symbol();
  if (sym.isnull())
    return;

  projNode * pr = sym.get_xref()->get_projNode();
  if (pr) {
    MetricSet *set = proj_get_set(pr);
    if (!set) {
      set = new MetricSet(pr);
    }
    set->find_element(sym, mel);
  }
}


// Set-specific functions

MetricHash::~MetricHash() {
//    fprintf(stderr, "Deleting a MetricHash with offset %d\n", offset);
}

unsigned int MSet::hash_element (const Object * ob) const {
    Initialize(MetricSet::hash_element);

    if (((MetricHash *)ob)->offset & (unsigned int)0x1) // this is a KEY
      return ((MetricHash *)ob)->offset >> 1;
    else // this is an OFFSET
      return table[(((MetricHash *)ob)->offset >> 1) + 1]->get_index();
}
 
bool MSet::isEqualObjects(const Object& ob1, const Object& ob2) const {
    Initialize(MetricSet::isEqualObjects);
    
    return (hash_element(&ob1) == hash_element(&ob2));
}

unsigned MetricHash::hash() const { return 0; } // The real hash used is MSet::hash_element
bool MetricHash::isEqual(const Object &) const { return false; } // MSet::isEqualObjects is used
