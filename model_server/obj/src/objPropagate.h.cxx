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
// objPropagate.h.C
//  
//   Contains propagation functions.
//  
//
#include "objPropagate.h"

#include "genError.h"
#include "objOper.h"
#include "objRelation.h"
#include "transaction.h"

extern void obj_notify(int, Relational*);

#undef implement_new
#define implement_new(Clas,sz) \
  {return transactional::operator new(sz);}

#undef implement_delete
#define implement_delete(Clas,ptr) \
 {transactional::operator delete(ptr);}

// -------- this class requires special hash method

init_relational(objRelWalkSet,Relational);


void objRelWalkSet::print(ostream& os, int level
) const
{
  obj_prt_obj(targ_obj); 
  os << " wp " << walk_ptr << endl; 
}


// -------- this class requires special hash method

bool objRelWalkSet::isEqual(const Object& x) const
{
  Initialize(objRelWalkSet::isEqual);
  
  bool tmp; 
  tmp =  ( targ_obj == (checked_cast(objRelWalkSet,&x))->targ_obj ) ; 
  return tmp; }

unsigned objRelWalkSet::hash() const { 
  unsigned tmp ;
  tmp = (unsigned)targ_obj;
  return tmp;
}
objRelWalkSet::~objRelWalkSet()
{
  this->rem_rel_all();
}

// trnsf_type.C 
// Description: transfers operation type along relation; 
//              for ste->ast pair changes REPLACE type to REPLACE_RIGHT,
//              for ast->not-ast changes REPLACE_RIGHT to REPLACE;
//
//           -- This call should depend on RELATION TYPE
#ifdef XXX_trnsf_type
objOperType  trnsf_type(RelationalPtr, RelationalPtr, objOperType)
{
   return (REPLACE);
}
#endif
// obj_modify.C
//
// Description: Modifies the object and all its relations propagating the 
//              changes. For each relation type the call finds all member 
//              objects and applies modify action function to them. The call
//              invoked recursively untill the entire tree is processed. The
//              objects are not modified immediately but collected in a 
//              temporary set (one occurences per object). When all objects 
//              collected the actual modification takes place. A special test
//              can be included into objModifier to filter the objects to be
//              modified.
//              obj_modify starts a transaction accumulating in array all 
//              the objects that should be deleted. Application code moves
//              the objects into this array. NOte, that obj_modify can be 
//              nested and only the end of all transactions starts deletion.
//
//



void obj_apply(objSet*walk)
{
  Initialize(obj_apply);
   start_transaction(){
     Obj*el;
     ForEach( el, *walk){
        objRelWalkSet *ws = checked_cast(objRelWalkSet,el);
        Relational * node = ws->targ_obj;
        objModifier * pp = (objModifier*) ws->walk_ptr;

        pp->apply(node);
        
        if (pp->get_notify_flag()==AFTER_APPLY){
            obj_notify(relationMode::M, node);
        }
      }
   } end_transaction();
}

extern "C" void obj_modify (Relational &root, objModifier *mod_ptr, 
                int apply_flag, objSet*walk)
{
 Initialize(obj_modify);
   int        level = -100;     //should not metter
   int flag = relationMode::M;
   rel_walk( &root, flag, mod_ptr, *walk, level);

   if(apply_flag) 
      obj_apply(walk);
}

void obj_modify (Relational &root, objModifier *mod_ptr)
 {
  
   objSet     mod_set;
   obj_transaction_start();
   obj_modify(root, mod_ptr, 1, &mod_set);
   obj_transaction_end();
 }


// obj_walk.C
//
// Description: Process the object and all related objects of the relational 
//              tree. For each relation type the call finds all member objects
//              and applies the action function to them. The function is 
//              defined in the objWalker as apply function. The call invoked
//              recursively untill the maximum asked level reached. The 
//              objects are not processed immediately but collected in a 
//              temporary set (one occurences per object). When all objects 
//              collected the actual processing takes place. A special test 
//              can be included into objWalker to filter the objects to be
//              processed.
//
//


void obj_walk (Relational &root, objWalker *walk_ptr, const int & max_level)
{
        int        level;
        int        flag;
        Obj        *el;
        objSet     walk_set;

//-------- Executable Code ----------------------------------------------

 Initialize(obj_walk);

  start_transaction(){
//      make a set to collect all nodes for applying certain functions

        level = max_level;                          // max depth

//      walk the relational tree starting from the root up to max level

        flag = relationMode::W;
        rel_walk( &root, flag, walk_ptr, walk_set, level);

//      do actual walk thru the tree processing the nodes

        ForEach( el, walk_set)
                 walk_ptr->apply( (checked_cast(objRelWalkSet,el))->targ_obj );

        walk_set.purge();

    } end_transaction();
}

