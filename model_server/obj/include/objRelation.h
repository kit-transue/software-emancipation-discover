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
#ifndef _objRelation_h
#define _objRelation_h

//////////////////////   FILE objRelation.h  /////////////////////////////////
//

#include "Relational.h"

#include "objRelmacros.h"

#include "genError.h"

#undef NULL
#define NULL 0

#define MANY        100000

extern "C" void obj_prt_obj(void*);
extern "C" const char * obj_type(void*);

#define symbolPtr_is_relational(x) ((!((char*)x)[7]) && (x->relationalp()))
#define symbolPtr_is_symbol(x) (((char*)x)[7])

#define declare_copy_member(name)                                      \
virtual RelationalPtr rel_copy() const
    
#define implement_copy_member(Clas)                                   \
RelationalPtr Clas::rel_copy() const				      \
{								      \
  return (db_persistent() ? db_new(Clas,(*this)) : new Clas(*this));  \
}

#define copy_member(name)	                                      \
virtual RelationalPtr rel_copy() const				      \
{								      \
  return (db_persistent() ? db_new(name,(*this)) : new name(*this));  \
}

#define generate_descriptor(A,B)				      \
typedef A * paste(A,Ptr);					      \
extern int  paste(is_,A)(RelationalPtr)

#define define_relational(Clas,Bas)	        \
static class_descriptor _descrpt;		\
void *operator new(size_t);			\
void  operator delete(void*);			\
virtual descriptor descr() const;		\
virtual int subclass(descriptor)

#define init_set_descr(Clas,Bas)				      \
class_descriptor Clas::_descrpt(sizeof(Clas), quote(Clas), \
              &Bas::_descrpt);                             \
descriptor Clas::descr() const					      \
{								      \
    return (descriptor)&Clas::_descrpt;				      \
}								      \
int Clas::subclass(descriptor d)				      \
{								      \
    int lev;							      \
    if(Clas::descr() == d) lev = 1;				      \
    else if((lev=Bas::subclass(d)) > 0) ++lev;			      \
    return lev;							      \
}

int is_subclass(descriptor, descriptor);
void*obj_mem_alloc(descriptor,size_t);
void obj_mem_free(descriptor,void*);

#define implement_new(Clas,sz) \
  {return obj_mem_alloc((descriptor)&Clas::_descrpt,sz);}
#define implement_delete(Clas,ptr) \
 {obj_mem_free((descriptor)&Clas::_descrpt, ptr);}

#define init_relational(Clas,Bas)				      \
init_set_descr(Clas,Bas);					      \
int paste(is_,Clas)(RelationalPtr x)				      \
{return is_subclass(x->descr(), (descriptor)&Clas::_descrpt);}        \
void * Clas::operator new(size_t sz) implement_new(Clas,sz)           \
void Clas::operator delete(void*ptr) implement_delete(Clas,ptr)

inline RelationalPtr check_Relational_cast(int (*is_type)(RelationalPtr),
  const char*tp, RelationalPtr obj_ptr, const char* fcn_name, int line_no) {
   if (obj_ptr &&
    ((symbolPtr_is_symbol(obj_ptr)) ||
     (!obj_ptr->relationalp()) || (!is_type(obj_ptr))))
      gen_conv_error_handler_new(tp, fcn_name, line_no, obj_ptr);
   return obj_ptr;
}

#ifdef _ASET_OPTIMIZE
#define checked_cast(T,v) ((T*) RelationalPtr(v))
#else
#define checked_cast(T,v) ((T*) check_Relational_cast(::paste(is_,T), \
      quote(T), (RelationalPtr(v)), _my_name, __LINE__))
#endif

#define init_abstract_relational(Clas,Bas) init_relational(Clas, Bas)

enum objOperType
  {
  NULLOP = 0,
  FIRST,
  AFTER,
  REPLACE,
  REPLACE_RIGHT,
  CHANGE_NAME,
  EXT_MODIFY,
  NEW_TYPES,
  REMOVE = NEW_TYPES,
  REGENERATE,
  SPLIT,
  MERGE,
  BEFORE,
  PASTE,
  CUT,
  REPLACE_REGION
  };

enum notification_time
{
    AFTER_ACCEPT, AFTER_APPLY
};

class objTree;

class objPropagator;

class objWalker;

class objModifier;

class objCopier;

class objDeleter;

//class objPrinter;

void obj_delete(Obj*, objDeleter* = 0);
void obj_delete(Relational&, objDeleter*); // old style
void obj_real_delete(Obj*);
void obj_modify(Relational&, objModifier*);
void obj_unload(Obj*);

RelationalPtr obj_copy_shallow(Relational&, objCopier*);

extern "C" {
    void obj_prt(const RelationalPtr);
    void node_prt(const Obj*);
}

void gen_print_indent(ostream&, int level);

// ALL_LEVELS now is used only on obj_print
#define ALL_LEVELS  10


class relationMode
{
public:
	// U =unload S = Shallow Copy; D = Delete; M = Modify, C = Copy; W = Walk
	enum {
		USDC	= 58,
		UD	= 40,
		U	= 32,
		SDMCW	= 31,
		SDMC	= 30,
		SDMW	= 29,
		SDM	= 28,
		SDCW	= 27,
		SDC	= 26,
		SDW	= 25,
		SD	= 24,
		SMCW	= 23,
		SMC	= 22,
		SMW	= 21,
		SM	= 20,
		SCW	= 19,
		SC	= 18,
		SW	= 17,
		S	= 16,
		DMCW	= 15,
		DMC	= 14,
		DMW	= 13,
		DM	= 12,
		DCW	= 11,
		DC	= 10,
		DW	= 9,
		D	= 8,
		MCW	= 7,
		MC	= 6,
		MW	= 5,
		M	= 4,
		CW	= 3,
		C	= 2,
		W	= 1
	};
};


#endif  // _objRelation_
