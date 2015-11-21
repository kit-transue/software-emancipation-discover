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

#ifndef _symbolSet_h
#define _symbolSet_h
class symbolSet;
#include <symbolArr.h>

#include <symbolPtr.h>

#include <general.h>
#include <Hash.h>

// note: the symbolSet has one of two types:
//    type 0 - all symbols must be xrefSymbol, and are compared by name
//    type 1 - symbols may be of any kind, and are compared using teh
//         operator ==, which considers only address and offset
// stores symbolArr and hash table (Set) of indeces
// store ind+1  (cannot store just ind: 0 is treated as NULL

class symbolIndSet : public Set {
public:
  symbolArr arr;
  symbolIndSet(int tp) : type(tp), tmp_sym(0) {}
  symbolIndSet(symbolIndSet&other) : 
          Set(other), arr(other.arr), type(other.type), tmp_sym(0) {}
  unsigned int  type;
  const symbolPtr*tmp_sym;
  const symbolPtr* lookup(const symbolPtr&) const;
 
  const symbolPtr* sym(const Object* oo) const {return &arr[((int) oo)-1];}
  virtual unsigned hash_element(const Object*) const;
  virtual bool isEqualObjects(const Object&, const Object&) const;
  int biggest_chain(int * ind) { return contents.biggest_chain(ind); }
};


class symbolSet {
  symbolIndSet sis;
  int no_holes;
  int cur_ind;          // for get_first(), get_next();
public:
  symbolSet(int set_t); // normal constructor
  symbolSet();	        // default ctor -- like set_t = 0
  symbolSet(int set_t, int init_size);  // for perfomance
  symbolSet(symbolSet&);
  ~symbolSet();
  unsigned capacity() { return sis.capacity(); }
  int biggest_chain(int);
  int size() const {return sis.size();}
  symbolSet & insert(const symbolSet& as) ;
  symbolSet & insert(const symbolArr& arr) ;
  bool insert(const symbolPtr&);
  const symbolPtr* lookup(const symbolPtr&s) const {return sis.lookup(s);}
  symbolSet & operator = (const symbolArr &);
    
  void insert_last(const symbolPtr& sym) { insert(sym); }

  int includes(const symbolPtr& sym) const; 
  void remove(const symbolPtr& sym); 
  void removeAll();

  void set_type(unsigned int ii) { sis.type = ii;}
  symbolArr&arr() {return sis.arr;}
  int get_index(const symbolPtr& sym) const; // -1 if not included
  const symbolPtr& get_first() const; 
  const symbolPtr& get_next() const; 
};


#define ForEachT(ee,coll) \
for( ee=coll.get_first(); ee.isnotnull(); ee=coll.get_next())


class stringSet : public nameHash {
  virtual const char* name(const Object* oo) const;
  virtual bool isEqualObjects(const Object& o1, const Object&o2) const;
};

#endif
