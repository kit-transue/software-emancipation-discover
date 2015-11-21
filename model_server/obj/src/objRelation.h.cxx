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
// objRelation.h.C

#include <msg.h>
#include "objArr.h"
#include "transaction.h"
#include "genError.h"
#include "objSet.h"
#include "objRelation.h"
#ifndef ISO_CPP_HEADERS
#include <iostream.h>
#include <stdlib.h>
#include <string.h>
#else /* ISO_CPP_HEADERS */
#include <iostream>
using namespace std;
#include <cstdlib>
#include <cstring>
#endif /* ISO_CPP_HEADERS */
#include "relArr.h"
#include "genArr.h"
#include <machdep.h>
#undef Initialize
#define Initialize(x) static char const *  _my_name = quote(x)
extern RelType* ref_smt_of_dd, *ref_dd_of_smt;

// uninlined virtual methods

 objPropagator* objPropagator::accept(RelationalPtr, int) { return this; }

 RelationalPtr Relational::rel_copy() const
  {
    Initialize(Relational::rel_copy);
    Error(NOT_IMPLEMENTED_YET);
    return (NULL);
  }

 descriptor	Relational::descr() const { return 0; }
 int		Relational::subclass(descriptor d) { return (d ? 0 : 1); }
 void	Relational::remove() {}
 char const *Relational::get_name() const { return "dummy";}
 void  Relational::set_name(char const *) {}
 void  Relational::notify(int, Relational*, objPropagator*, RelType*) {}

RelType* RelType::first;

///////

int is_subclass(descriptor desc1, descriptor desc2)
{
  if(desc1==desc2) return 1;

  register class_descriptor * d1 = (class_descriptor*) desc1;
  register class_descriptor * d2 = (class_descriptor*) desc2;

// boris: i am fixing crash during smtTree node physical deleting.
// it is relation on a node "def_smt_of_dd". Destractor on a Relational
// calls all_relation removing. The relation def_smt_of_dd calls hook
// "dd_rel_hook_prt" that calls is_ddElement() on a deleting node, that
// fails in is_subclass() because of a zero first class descriptor. 
// It is zero because descr() member on Relational returns 0;
//
// I assume that if desc1 == 0 it is Relational and it is not a subclass of
// anything but Relational. If desc2 == 0 it means that everything is subclass
// of it. If I am wrong, feel free to correct following code.

  if ( d1 == 0 )
     return 0;

  if ( d2 == 0 )
     return 1;

// end of boris's cludge.

  int depth1 = d1->depth;
  int depth2 = d2->depth;

  if(depth1 <= depth2) return 0;
  while(depth1--  > depth2) d1 = d1->base;
  return (d1==d2);
}

extern int not_call_hook(RelType* rt);

void ptr_prt(ostream&, const void * const);

static void call_rel_hook
       (int type, RelType * rt, RelType * irt, Relational * from,
                          Relational * to)
  {
    if (not_call_hook(rt))
      return;

  objRelHook hook = rt->get_hook();
  if(hook)
      (*hook)(type, from, to, rt);
  if(irt)
     {
     hook = irt->get_hook();
     if(hook)
         (*hook)(type, to, from, irt);
     }  
  }

inline int incl(Obj * set, Obj * el)
  {
  return set->collectionp() ? set->includes(el) : set == el;
  }

// The main constructor for "Relational"
Relational::Relational() : rel_list()
  {
  }

 int Relational::construct_symbolPtr(symbolPtr&) const { return 0; }
 ddKind Relational::get_symbolPtr_kind() const { return DD_UNKNOWN; }
 int Relational::is_symbolPtr_def () const { return 0; }   //analog to get_is_def()
 app* Relational::get_symbolPtr_def_app () { return 0; }

void Relational::Relational_stab1_virtual ()
{
    Initialize(Relational::Relational_stab1_virtual);
    Error(NOT_IMPLEMENTED_YET);
}

void Relational::Relational_stab2_virtual ()
{
    Initialize(Relational::Relational_stab2_virtual);
    Error(NOT_IMPLEMENTED_YET);
}

void Relational::Relational_stab3_virtual ()
{
    Initialize(Relational::Relational_stab3_virtual);
    Error(NOT_IMPLEMENTED_YET);
}

void Relational::Relational_stab4_virtual ()
{
    Initialize(Relational::Relational_stab4_virtual);
    Error(NOT_IMPLEMENTED_YET);
}

void Relational::Relational_stab5_virtual ()
{
    Initialize(Relational::Relational_stab5_virtual);
    Error(NOT_IMPLEMENTED_YET);
}

void Relational::Relational_stab6_virtual ()
{
    Initialize(Relational::Relational_stab6_virtual);
    Error(NOT_IMPLEMENTED_YET);
}

void Relational::Relational_stab7_virtual ()
{
    Initialize(Relational::Relational_stab7_virtual);
    Error(NOT_IMPLEMENTED_YET);
}

void Relational::Relational_stab8_virtual ()
{
    Initialize(Relational::Relational_stab8_virtual);
    Error(NOT_IMPLEMENTED_YET);
}


// Copy constructor for "Relational"
Relational::Relational (const Relational & ) : rel_list()
  {
  }

// Destructor for Relational
Relational::~Relational()
  {
  this->rem_rel_all();
  put_id(-1);
  } 

class_descriptor Relational::_descrpt
    (sizeof(Relational),"Relational",(class_descriptor *)0);

// Assign operator for "Relational"
Relational& Relational::operator=(const Relational & )
  {
    relArr tmp;
    rel_list = tmp;
  return *this;
  }

bool Relational::relationalp() const { return 1;}

// Print method for Relational
void Relational::print(ostream& st , int /*level*/) const
  {
  Relational * obj = (Relational*) this;
  st << '<' << obj_type(obj) << ' ' << obj << '>';
  }

char const * Relational::objname() const
{
  return  this->descr() ? this->descr()->name : "Relational";
}

// Prapagate method for Relational
void Relational::propagate(int flags, objArr* arr)
{
  for(int i = rel_list.size() - 1; i >= 0; i--){
    RelType * rt = rel_list[i]->get_rel_type();
    if(rt->get_flags() & flags){
       Obj * m = rel_list[i]->get_members();
       if((flags & relationMode::D) && rt->get_inv_type()->get_cardinality() > 1){
         Obj * el;
         ForEach(el,*m){
           Obj * that = get_relation(rt->get_inv_type(), RelationalPtr(el));
           if(that->size() == 1)
              arr->insert(el);
         }
       } else {
          arr->insert(m);
       }
    }
  }
}
// Is it needed ?
void Relational::send_string( ostream& stream ) const
  {
  stream << "object(" << (int) this << ")";
  }

objPropagator * Relational::accept(objPropagator *walker, RelType *)
  {
  return walker;
  }

int Relational::save(void * , int )
  {
  Initialize(Relational::save);
  Error(ERR_INPUT);
  return (0);
  }

//------------------  RelType class methods  ------------------

// - accept method for propagation on links; checks flags
//   rt if given. If returns 0 - propagation stopped.

objPropagator* RelType::accept(objPropagator *walker, int flag)
  {
  if ( (get_flags() & flag) == 0 )
    return 0;
  if ( walker->get_assoc_type() != this->get_assoc_type() )
    return 0;
//     if rt to be walked specified in the walker, check it
  else if ( walker->rel_type != NULL && 
            walker->rel_type != this )
    return 0;
  else
    return walker;
  }


objArr_Int * rel_classes;

class_descriptor::class_descriptor(int l, char const * n, class_descriptor *bs)
  : type_id(0xFF), length(l), name(n), rel(0), base(bs), depth(0), unused(0)
{
if(! rel_classes) rel_classes = new objArr_Int;
if(bs)
   rel_classes->insert_last((int) this);
}

struct rel_ptr {
  RelType * rt;
  class_descriptor * desc;
};

//----------------------------------------------------------------------------
// The contructor for the following genArr was called _AFTER_ another
// global class constructor (RelType::RelType())had inserted elements into it.  
// This would result in the zeroing of its size.
//    static genArr(rel_ptr) pointed_data;
// The solution is to make the genArr a pointer that is created at the 
// first insert.  This element seem to be only accessed by RelType.  Why
// it isn`t a member of RelType is a mystery.  Some day it should be moved into
// RelType.
//
genArr(rel_ptr);
static genArrOf(rel_ptr) *pointed_data;


extern "C" void rel_ptrs_prt()
{
  int sz;
  if( !pointed_data ) 
  { 
    msg("Static pointed_data refered to before set.", catastrophe_sev) << eom; 
    return; 
  } 
  
  sz = pointed_data->size();
  msg("rel_ptr $1", normal_sev) << sz << eom;

  for(int ii=0; ii<sz; ++ii){
     msg("$1", normal_sev) << (*pointed_data)[ii]->desc->name << eom;
     msg("$1", normal_sev) << (int) (*pointed_data)[ii]->desc->depth << eom;
     char const *rn = (*pointed_data)[ii]->rt->get_name();
     msg("$1", normal_sev) << rn << eom;
  }
}

static void init_ptr()
{
  int sz ;

  if( !pointed_data ) 
  { 
    msg("Static pointed_data refered to before set.", catastrophe_sev) << eom; 
    return; 
  } 
  
  sz = pointed_data->size();
  for(int ii=0; ii<sz; ++ii){
    class_descriptor * dd = (*pointed_data)[ii]->desc;
    RelType * rt = (*pointed_data)[ii]->rt;
    rt->next = dd->rel;
    dd->rel = rt;
  }
}

static void init_descr(class_descriptor * cd)
{
   class_descriptor * bd = cd->base;
   if(! bd) return;
   RelType * bt = bd->rel;
   if(! bt) return;

   RelType * ct = cd->rel;
   if(ct == NULL){
     cd->rel = bt;
     return;
   }
// put base.rel to the end

   while(ct->next) 
             ct = ct->next;
   ct->next = bt;
   return;
}

int obj_get_depth(class_descriptor * cd)
{
 if(cd->depth == 0)
    cd->depth = cd->base ? obj_get_depth(cd->base) + 1 : 1;

 return cd->depth;
}
 
void init_rel_ptrs()
{

  class_descriptor * dd;

  objArr_Int & darr = *rel_classes;
  int sz = darr.size();
  int ii, jj;

// initialize depth field

  for( ii=0; ii<sz; ++ii){
      dd = (class_descriptor *) darr[ii];
      obj_get_depth(dd);
  }

// init pointer fields

  init_ptr();

// insertion sort according to depth

  for( ii=0; ii<sz; ++ii){
    class_descriptor * di  = (class_descriptor *) darr[ii];
    for(jj = ii; jj>0; --jj){
      dd = (class_descriptor *) darr[jj-1];
      if(dd->depth > di->depth){
         darr[jj] = (int)dd;
      } else {
         break;
      }
    }
    darr[jj] = (int)di;
  }

  for( ii=0; ii<sz; ++ii){
    dd = (class_descriptor *) darr[ii];
    init_descr(dd);
  }
}

RelType::RelType (char const *nnn, int cc, int ff, 
    RelType* invr, int assoc, int off, descriptor desc) :
                  cardinality(cc), flags(ff), offset(off), hook(NULL),
	          hcs(0), filler (0), id (0), xref_id (0)  // A.H.
  {
  while(*nnn == ' ')		// Skip first spaces
    nnn++;                      // and
  strcpy(name, nnn);		// copy the name of relation
  assoc_type = (AssocType)assoc;
  inverse_type = invr;
  if(invr == (RelType *)1){
    inverse_type = this;
    forw = 0;
  } else if(invr != 0) {
    invr->inverse_type = this;
    forw = 0;
  } else {
    forw = 1;
  }

  ifl = (assoc==IFL_ASSOC);
  save = (assoc != 0);
  if(off)
    {
    if (!pointed_data)
	pointed_data = new genArrOf(rel_ptr);

    rel_ptr * rp = pointed_data->grow(1);
    rp->rt = this;
    rp->desc = (class_descriptor *) desc;
    
    this->next = 0;

    }
  else 
    {
    next = first;
    first = this;
    }
  if(ifl)
    {
      if(flags & relationMode::S){
        flags |= relationMode::C;
        flags ^= relationMode::S;
      }
    }  
put_id(2);			// id = 2 for relations type
  }

RelType::RelType (char const *nnn, int cc, int ff, RelType* invr, int assoc) :
                  cardinality(cc), flags(ff), offset(0), hook(NULL),
	          hcs(0), filler (0), id (0), xref_id (0)  // A.H.
  {
  while(*nnn == ' ')		// Skip first spaces
    nnn++;                      // and
  strcpy(name, nnn);		// copy the name of relation
  assoc_type = (AssocType)assoc;
  inverse_type = invr;
  if(invr == (RelType *)1) {
    inverse_type = this;
    forw = 0; // A.H.
  } else if(invr != 0) {
    invr->inverse_type = this;
    forw = 0;  // A.H.
  } else {
      forw = 1;  // A.H.
  }
  next = first;
  first = this;

  ifl = (assoc==IFL_ASSOC);  // A.H. 
  save = (assoc != 0);       // A.H.

  if(ifl)                    // A.H.
    {                        
      if(flags & relationMode::S){         
        flags |= relationMode::C;          
        flags ^= relationMode::S;          
      }                      
    }                        // A.H.
  put_id(2);			// id = 2 for relations type
  }

// Find relation type by name
RelType * RelType::find(char const *n)
  {
  RelType * r;
  for(r = first; r; r = r->next)
    if(strcmp(r->name, n) == 0)
      break;
  return r;
  }


// propagate returns value of level of target object.
// if return value is < 0; propagation is not done
int RelType::propagate(RelationalPtr  , int level,
                       RelationalPtr , int a_type)
  { 
  int new_level;
  new_level = level + 1;
  if ( (a_type != 0) && (a_type != get_assoc_type()) )
    new_level = -1;
  return new_level;         
  }          
          
//------------------  Propagator class methods  -----------------
void const *objPropagator::get_data() {return 0; }
objTree *objPropagator::get_target() {return 0; }
objOperType objPropagator::get_op_type() {return NULLOP; }
int objDeleter::apply(RelationalPtr  obj)  
  {
  delete obj;    
  return 1;      
  }

// sample objRelHook
static void obj_rel_hook_prt(int oper, Relational * src, 
                             Relational * targ, RelType * rel)
  {
  msg("obj_rel_hook_prt: $1$2", normal_sev) << (oper > 0 ? "put " : "rem ") << eoarg << rel->get_name() << eom;
  src->Relational::print();
  targ->Relational::print();
  }

extern "C" void rel_monitor(RelType *rel)
  {
  obj_rel_put_hook(rel, obj_rel_hook_prt);
  }

// Test relation
int  Relational::test_rel (Relational &m, const RelType *rt) const
  {
  Obj * members =  rel_list[rt];
  return members ? incl(members,&m) : 0;
  }

// Get relation
Obj* Relational::get_rel(const RelType* rt) const
  {
//     return rel_list[rt];
       return get_ptr(rt);
  }

// Put relation to specific object or set of objects
void  Relational::put_rel (Relational &m0, RelType *rt)
  {
  Initialize(Relational::put_rel); 
  RelType * irt = rt->get_inv_type();

  this->priv_add_one_rel (&m0, rt);
  if (irt)
      (&m0)->priv_add_one_rel(this, irt);

  call_rel_hook(1, rt, irt, this, &m0);     // Call rel_hook if present

  return;
  }

genArr(relStruct);

void  Relational::restore_rel (relStructArr& rel_arr)
{
    
  Initialize(Relational::restore_rel(relStructArr&));
    int no_rel = rel_arr.size();

    int new_sz = no_rel;
    int max_new_sz = 2*no_rel; // if no DB_NULL

    relStruct * new_ptr = rel_arr.grow(no_rel);
    relStruct * rel_ptr = rel_arr[0];

    int ii;
    for(ii=0; ii < no_rel; ++ii, ++rel_ptr) {
        if(rel_ptr->trg != DB_NULL){
           new_ptr->src = rel_ptr->trg;
           new_ptr->trg = rel_ptr->src;
           new_ptr->rel = rel_ptr->rel->get_inv_type();
           ++ new_sz;
           ++ new_ptr;
        }
    }
    if(new_sz < max_new_sz)               // DB_NULLs
      rel_arr.grow(new_sz - max_new_sz);  // shrink

// put relations
    new_ptr = rel_arr[0];
    for(ii=0; ii < new_sz;) {    // foreach set
       // collect set
       int count = 0;
       rel_ptr = new_ptr;
       RelType * rt = rel_ptr->rel;
       Relational * src = rel_ptr->src;
       do {
          ++ ii;
          ++ count;
          if(ii==new_sz)
                      break;
          ++ new_ptr;
       } while(new_ptr->src == src && new_ptr->rel == rt);
       
       // apply set

       relStruct * cur_ptr = rel_ptr;

       if(rt->offset)  {
          for(int jj=0; jj<count; ++jj, ++cur_ptr)
              src->priv_add_one_ptr(cur_ptr->trg, rt); 
          continue;
       }    
 
       if(count == 1){
          src->priv_add_one_rel(rel_ptr->trg, rt); 
          continue;
       }

       objSet * os = NULL;
       Relation* curr_rel = (src->rel_list)[(src->rel_list).index(rt)];

       if(!curr_rel){
          os = new objSet(count);
          curr_rel = (src->rel_list).grow(1);
          curr_rel->members = os;
          curr_rel->rel_type = rt;
       } else {
          Obj * mb = curr_rel->members;
          if(mb->collectionp()){
             os = (objSet*) mb;
          } else {
             os = new objSet(count+1);
             os->os_list.add(*mb);
             curr_rel->members = os;
          }
       }

       for(int jj=0; jj<count; ++jj, ++cur_ptr)
             os->os_list.add(*(cur_ptr->trg));
    } // for each

// call_hook

    rel_ptr = rel_arr[0];
    for(ii=0; ii < new_sz; ++ii, ++rel_ptr) {
       if(rel_ptr->trg != DB_NULL){
           RelType * rt = rel_ptr->rel;
           objRelHook hook = rt->get_hook();
           if(hook)
              (*hook)(0, rel_ptr->src, rel_ptr->trg, rt);
       }
    }
}

void  Relational::restore_rel (Relational &m0, RelType *rt)
  {
  Initialize(Relational::restore_rel); 
  RelType * irt = rt->get_inv_type();

  this->priv_add_one_rel (&m0, rt);
  if (irt)
      (&m0)->priv_add_one_rel(this, irt);

  call_rel_hook(0, rt, irt, this, &m0);     // Call rel_hook if present

  return;
  }

// trung: call_rem_rel_hooks() iterates through existing rel_list and
// calls relation hooks without actual relation touching. But hooks can
// remove relations too. It changes rel_list and causes crash. 
// get_next_rel() returns next relation for a specified one or NULL
// if rel_list does not have specified relation or specified relation is
// the last one.
//
Relation* get_next_rel(relArr& rel_list, RelType* rt)
{
  int size = rel_list.size();
  int i;
  for (i = 0; i < size; ++i) {
    Relation *rli = rel_list[i];
    if (rli && (rli->get_rel_type() == rt))
      break;
  }
  if (i < size - 1)
    return rel_list[i+1];
  return NULL;
}

// Remove relation hooks calling without touching relations itself
//

void  Relational::call_rel_hooks(int fla)
{
  Initialize(Relational::call_rel_hooks);  
  int sz = rel_list.size();
  if (sz == 0) return;
  RelType* rt = 0;
  Relation* rl;
  for (rl = rel_list[0] ;
       rl ;
       rl = get_next_rel(rel_list, rt))
  {
    rt = rl->get_rel_type();
    RelType *irt = rt->get_inv_type();
    Obj * m = get_ptr(rt);
    if (!m || !rt )
          continue;
    if((!rt->hook) && (!irt->hook))
          continue;
    int hcsf = rt->hcs || irt->hcs;
    if(m->collectionp() && !hcsf)
    {
      Obj * el;
      if (fla == 1) {
        ForEach(el, *m)
        {
	  call_rel_hook(fla, rt, irt, this, (Relational *)el);
	  if (rt == ref_smt_of_dd) break; // no need to do same action for all.
        }
      }
      else {
        objSet ms(m);
        ForEach(el, ms)
        {
	  call_rel_hook(fla, rt, irt, this, (Relational *)el);
        }
      }
    }
    else
      call_rel_hook(fla, rt, irt, this, (Relational *)m);
  }
}

// Remove one relation to specific object (or set of objects)
void  Relational::rem_rel(Relational & m0,  RelType *rt)
  {
  Initialize(Relational::rem_rel);  
  Assert(rt);
  Assert(&m0);
  RelType * irt = rt->get_inv_type();

  if ( get_id() > 0 )
      call_rel_hook(-1, rt, irt, this, &m0);

  this->priv_rem_one_rel (&m0, rt);
  if (irt)
      (&m0)->priv_rem_one_rel(this, irt);

  return;
  }

// Remove whole relation of specified type

void  Relational::rem_rel(RelType *rt)
{
    Initialize(Relational::rem_rel);  
    Assert(rt);
    Obj * m = get_raw_ptr(rt);
    if (! m)
	return;
    
  if(m->collectionp())
    {
     Obj * el;
     objSet ms(m);
     ForEach(el, ms)
        rem_rel(*(Relational *)el, rt);
    }
  else
     rem_rel(*(Relational *)m, rt);
}

// Remove all relations
void  Relational::rem_rel_all()
  {
  while(rel_list.size())
    rem_rel(rel_list[0]->get_rel_type());
  }


void  Relational::rem_rel_from(objSet & from)
  /* remove all relations except the ones in objset */
{
  Obj* ob;
  ForEach(ob, from) {
    RelType * rt = (RelType *) ob;
    rem_rel(rt);
  }
}

// Remove all pointers; must be called from obj_real_delete
void  Relational::rem_pointers()
  {
  if(descr())     // NULL for Relational
    for(RelType*rt=descr()->rel; rt; rt=rt->next)
      rem_rel(rt);

  }

// Remove one relation - internal function (doesn't care about inversed one)

void  Relational::priv_rem_one_ptr(Relational * m, RelType *rt)
  {

  Obj ** ptr = (Obj **) ((char *)this + rt->offset);
  if ( !ptr || ! *ptr)
    return;

  Obj * m1 = *ptr;

  if(m1->collectionp())
    {
    ((objSet *)m1)->remove(m);
    if(m1->size() == 0)
      {
      delete ((objSet *)m1);
      *ptr = 0;
      }
    }
  else
    {
    *ptr = 0;
    }  
  }


void Relational::priv_add_one_ptr(Relational *m, RelType *rt)
{
    Obj ** ptr = (Obj **) ((char *)this + rt->offset);

    if(! *ptr) {
	*ptr = m;
	return;
    }
    if(rt->get_cardinality() == 1) {	/* Cardinality = 1 */
        Relational * m1 = (Relational *)*ptr;
	if(m1 != m) {
	    RelType * irt = rt->get_inv_type();
	    call_rel_hook(-1, rt, irt, this, m1);
	    m1->priv_rem_one_rel(this, irt);
	    *ptr = m;
	}
    } else {					/* Cardinality > 1 */
        Obj * mb = *ptr;
	if(! mb->collectionp())
	    *ptr = mb = new objSet(mb);         
	((objSet *)mb)->os_list.add(*m);
    }
    return;
}

// Add one relation - internal function (doesn't care about inversed one)
void  Relational::priv_add_one_rel (Relational * m, RelType *rt)
  {
  Initialize(Relational::priv_add_one_rel);
  Assert(m);
  Assert(rt);

//  Assert(m != this);
 
  if(rt->offset)
    {
    priv_add_one_ptr((Relational*)m, rt); 
    return;
    }    

  Relation* curr_rel = rel_list[rel_list.index(rt)];// Find "Relation" object

  if(! curr_rel)
     {
      rel_list.append(rt, m);
      return;
     }

  if(rt->get_cardinality() == 1) {	// Cardinality = 1
      Relational * m1 = (Relational *)curr_rel->members;
      if(m1 != m) {
        RelType * irt = rt->get_inv_type();
        call_rel_hook(-1, rt, irt, this, m1);

        m1->priv_rem_one_rel(this, irt);

	curr_rel = rel_list[rel_list.index(rt)];// hook can change rel_list

        if(! curr_rel)
	    rel_list.append(rt, m);
	else 
	    curr_rel->members = m;
      }
    }
  else					// Cardinality > 1
    {
      Obj * mb = curr_rel->members;
      if (mb->includes (m))
	  return;
      if(! mb->collectionp()) {
          objSet * ns = new objSet(2);
          ns->os_list.add(*mb); 
          curr_rel->members = mb = ns;
      }	  
      ((objSet *)mb)->os_list.add(*m);
    }
  }

// Remove one relation - internal function (doesn't care about inversed one)

void  Relational::priv_rem_one_rel(Relational * m, RelType *rt)
  {
  Initialize(Relational::priv_rem_one_rel);
  Assert(rt);
  Assert(m);

  if(rt->offset)                                     // fixed type relation
    {
    priv_rem_one_ptr( m, rt);
    return;
    }
                                                     // dynamic relation
  Relation * curr_rel = rel_list[rel_list.index(rt)];// Find "Relation" object
  if (!curr_rel)
    return;

  Obj * m1 = curr_rel->get_members();
  if (!m1 || ! m1->includes(m))                     // already removed or never existed
     return;

  if(m1->collectionp())
    {
    ((objSet *)m1)->remove(m);
    if(m1->size() == 0)
      {
      delete ((objSet *)m1);
      rel_list.remove(curr_rel);
      }
    }
  else
    {
    rel_list.remove(curr_rel);
    }  
  return;
  }

char * obj_relname(char const * n1, char const * n2)
{
 Initialize(obj_relname);
 static char  name_buf[MAX_NAME_LEN]; 
 int len1 = strlen(n1);
 int len2 = strlen(n2);
 int total_len = len1 + len2 + 4;
 Assert(total_len < MAX_NAME_LEN);
 OS_dependent::bcopy(n1, name_buf, len1);
 OS_dependent::bcopy("_of_", name_buf+len1, 4);
 OS_dependent::bcopy(n2, name_buf+len1+4, len2);
 name_buf[total_len] = '\0';
 return(name_buf);
}

Obj* get_loaded_rel (const Relational* r, const RelType* rt)
{
    Obj* targ = r->rel_list[rt];

    if (targ == DB_NULL) 
	// don't complete restore if r is going to be deleted
	if (r->get_id () > 0 && db_restore_complete (r)) 
	    targ = r->rel_list[rt];
	else
	    targ = 0;

    return targ;
}

void  RelType::put_name(char const *nnn) { strcpy(name, nnn); }
