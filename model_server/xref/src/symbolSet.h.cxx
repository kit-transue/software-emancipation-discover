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
#include "symbolSet.h"
#include "xref.h"
#include "XrefTable.h"

inline bool symbol_is_special(const symbolPtr*sp)
{
  return sp->is_instance() || sp->is_ast() || sp->is_dataCell();
} 
unsigned int symbolIndSet::hash_element(const Object* obj) const
{
  const symbolPtr* sp = sym(obj);
  if (type==1 || symbol_is_special(sp))
    return (unsigned int)((unsigned int)sp->un.cp ^ sp->offset);

  if(!sp->is_xrefSymbol()) {
    unsigned int h1 = nameHash::hash_string(sp->get_name());
    return h1;
  }

  xrefSymbol * sym = sp->operator->();
  ddKind knd = sym->get_kind();

  genString key = sp->get_name();

  /* Add definition file to macro name so that macros with same name but
     different definition files will have different hash keys thus 
     reducing symbols with same hash value in the table. */
  if (knd == DD_MACRO) {
    symbolPtr deffile = sym->get_def_file();
    if (deffile.isnotnull()) {
      key += deffile.get_name();
    }
  }

  unsigned int h1 = nameHash::hash_string((char *)key);
  if(knd != DD_VAR_DECL)
    return h1;

  if(sym->get_has_def_file(sp->get_xref()->get_lxref()))
    if (sym->get_attribute(STAT_ATT,1) && ! sym->get_attribute(METHOD,1))
      h1 ^= sp->offset;

  return h1;
}  

bool symbolIndSet::isEqualObjects(const Object&o1, const Object&o2) const
{
  const symbolPtr*s1 = sym(&o1);
  const symbolPtr*s2 = sym(&o2);
  if(type==1 || symbol_is_special(s1)) {
    return *s1 == *s2;
  } else if (type == 2) {
    ddKind k1 = s1->get_kind();
    ddKind k2 = s2->get_kind();
    if(k1 != k2) return false;

    const char* nm1 = s1->get_name();
    if(!nm1) nm1 = "";
    const char* nm2 = s2->get_name();
    if(!nm2) nm2 = "";
    return (strcmp(nm1, nm2)==0);
  }
  int cmp =  s1->sym_compare(*s2);
  return (cmp < 2 && cmp > -2) ? 1 : 0;
}

const symbolPtr* symbolIndSet::lookup(const symbolPtr&s) const
{
 Initialize(symbolIndSet::lookup);
 if(s.isnull())
   return NULL;

 ((symbolIndSet*)this)->arr.insert_last(s);
  int ind = arr.size();
  Object* oo = findObjectWithKey(*((Object*)ind));
  ((symbolIndSet*)this)->arr.remove_index(ind-1);
  
  return (oo ? sym(oo) : 0);
}

int symbolSet::get_index(const symbolPtr& sym) const
{
  const symbolPtr*ps = lookup(sym);
  if(!ps)
    return -1;
  int ind = ps - & sis.arr[0];
  return ind;
}
symbolSet::symbolSet(int set_t, int sz) : sis(set_t), no_holes(0), cur_ind(0)
{
  if(sz > 0) {
    sis.reSize((unsigned ) 3*sz);
    sis.arr.provide(sz);
  }
}
symbolSet::symbolSet(int set_t) : sis(set_t), no_holes(0), cur_ind(0)
{
}
symbolSet::symbolSet() : sis(0), no_holes(0), cur_ind(0)
{
}

symbolSet::symbolSet(symbolSet& other) : sis(other.sis), 
   no_holes(other.no_holes), cur_ind(0)
{
}

symbolSet::~symbolSet()
{
}
const symbolPtr& symbolSet::get_first() const
{
  if(size() == 0)
    return NULL_symbolPtr;

  ((symbolSet*)this)->cur_ind = 0;
  return get_next();
}

const symbolPtr& symbolSet::get_next() const
{

  const symbolArr& sarr = sis.arr;
  int arrsz = sarr.size();
  while(cur_ind < arrsz){
    symbolPtr&sym = sarr[((symbolSet*)this)->cur_ind++];
    if(sym.isnotnull())
      return sym;
  }
  return NULL_symbolPtr;
}
symbolSet & symbolSet::operator = (const symbolArr & sa)
{
  Initialize(symbolSet::operator=);
  removeAll();
  insert(sa);
  return *this;
}

void symbolSet::removeAll()
{
  sis.Set::removeAll();
  sis.arr.removeAll();
  no_holes = 0;
  cur_ind = 0;
}

symbolSet & symbolSet::insert(const symbolSet& other) 
{
    Initialize(symbolSet::insert_set);
    symbolPtr sym;
    ForEachT(sym,other)
      insert(sym);
    return *this;
}

symbolSet & symbolSet::insert(const symbolArr& oarr) 
{
    Initialize(symbolSet::insert_arr);
    symbolPtr sym;
    ForEachS(sym,oarr)
      insert(sym);
    return *this;
}

bool symbolSet::insert(const symbolPtr& sym) 
{
  Initialize(symbolSet::insert);
  bool retval = 1;
  sis.arr.insert_last(sym);
  int ind = sis.arr.size();
  int old_ind = (int) sis.add(*((Object*)ind));
  if(old_ind != ind){
    if(sym.is_xrefSymbol() && sym->get_has_def_file(sym.get_xref()->get_lxref(), 1))
      sis.arr[old_ind-1] = sym;      
    sis.arr.remove_index(ind-1);
    retval = 0;
  }
  return retval;
}

int symbolSet::includes(const symbolPtr& sym) const
{
  return (int) sis.lookup(sym);
}

void symbolSet::remove(const symbolPtr& sym)
{
  Initialize(symbolSet::remove);
  const symbolPtr*sp = sis.lookup(sym);
  if(sp){
    int ind = sp - &(sis.arr[0]);
    Object* oo = (Object*) (ind + 1);
    sis.Set::remove(*oo);
    if((ind+1) != sis.arr.size()) {
      sis.arr[ind] = NULL_symbolPtr;
      ++no_holes;
    } else {
      sis.arr.remove_index(ind);
    }
  }
}

const char* stringSet::name(const Object* oo) const
  { return (const char*) oo;}

bool stringSet::isEqualObjects(const Object& o1, const Object&o2) const 
  {return &o1 == &o2;}

int symbolSet::biggest_chain(int num_of_syms)
{ 
  int last_ind = -1;
  int max =  sis.biggest_chain(&last_ind);
  
  if (max > 0) {
    if ((num_of_syms == -1) || max < num_of_syms)
      num_of_syms = max;
    msg("Name and kind of symbols in the biggest chain: \n") << eom;
    for (int i = 0; i < num_of_syms; i++) {
      Object* obj = sis.at(last_ind - i);
      if (obj) {
	const symbolPtr* sp = sis.sym(obj);
	msg("$1, $2\n") << sp->get_name() << eoarg << sp->get_kind() << eom;
      }
    }
  }
  
  msg("Length of biggest chain is: $1\n") << max << eom;
  return max;
}

