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
// Entity.h
//
//------------------------------------------

#ifndef _Entity_h
#define _Entity_h

#include <objSet.h>
#include <ddict.h>
#include <autosubsys-macros.h>
#include <autosubsys-weights.h>

#ifndef _refs_and_weights_h
#include <refs_and_weights.h>
#endif
      
#ifndef _bit_array_h
#include <bit_array.h>
#endif      

#ifndef _objArr_h
#include <objArr.h>
#endif

//#include <stdio.h>
      
class entlist;
class allentity;
class decomposer;

//
// Entity is an abstracted container that encapsulates xrefSymbols
// and provides relations between them.
//

class Entity : public Obj {
  public:
    Entity();
    Entity(const Entity&);
   
    void  print(ostream&, int level = 0) const;

    int add2(const symbolPtr&, int alreadyused);

    int relate();

    virtual char *get_name() const;

    void setundead() { isundead=1; }
    int isdead();

    void find_live_code(weight *, int, int depth, FILE* dump_file);
   
    symbolPtr get_xrefSymbol() { return my_sym; }

    static void set_projects_to_search(const objSet& projs) {
       projects_to_search = projs;
    }

    void index(size_t idx) { my_idx = idx; }
    size_t index() const { return my_idx; }

    void add_ref(size_t targ_ref, unsigned char weight) {
       rw.add_ref(targ_ref, weight);
    }

    size_t calc_binding(const bit_array& mask);

    void tally_hits(size_t totals[]);

    void dump_refs_and_weights(FILE*, const objArr&);	// for debugging

    void relate_to_symbol(size_t, const symbolPtr&);
    symbolPtr relate_to_type(const symbolPtr& type, size_t rel);

    void normalize_refs();	// remove dups

    void set_nexte(Entity*);
    Entity* get_nexte() { return nexte; }
    void set_preve(Entity*);
    Entity* get_preve() { return preve; }

    static void set_allentities(allentity*);
    static allentity* get_allentities();

  protected:

    bool isundead;
    static allentity *allentities;
    symbolPtr my_sym;

    int newrelate();
   
private:
    void find_member_dtors(symbolArr&);
    void add_operator_delete(symbolArr&);
    void set_basetype();
    static objSet projects_to_search;

    size_t my_idx;
    refs_and_weights rw;

    Entity* nexte;
    Entity* preve;
    Entity* parent_class;
    Entity* basetype;	// for typedef

    friend int entity_sorter(const void *, const void *);
};

typedef Entity* EntityPtr;

#endif
