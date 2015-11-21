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
#ifndef _objRelmacros_h
#define _objRelmacros_h

#ifndef _general_h
#include "general.h"
#endif

//-----------------  MACRO DEFINITIONS FOR HANDLING RELATIONS  ------------


#define define_unirel(rel_name)                                             \
       extern RelType* rel_name

#define define_relation(rel_name,inv_rel_name)                              \
       extern RelType* rel_name; extern RelType* inv_rel_name

#define init_unirel(name, cardinality, flag)                                 \
       RelType* name = new RelType(quote(name), cardinality, flag)

#define init_symrel(name, cardinality, flag)                                 \
       RelType* name = new RelType(quote(name), cardinality, flag, 1)


#define __rn(n1,n2) paste3(n1,_of_,n2)

#define init_relation(name1, cardinality1, flag1,name2, cardinality2, flag2) \
       RelType* name1 = new RelType(quote(name1), cardinality1, flag1);       \
       RelType* name2 = new RelType(quote(name2), cardinality2, flag2, name1)

//  -  macros for methods :

#define put_relation(name, obj1, obj2) \
  do \
    if ( !((obj1) == NULL || (obj2) == NULL) ) \
      (obj1)->put_rel(*(obj2), name); \
    while(0)

#define get_relation(name, obj1)                                             \
                          (((obj1) == NULL) ? ((Obj *)obj1) : (obj1)->get_ptr(name))

#define try_relation(name, obj1)                                             \
                          (((obj1) == NULL) ? (obj1) : (obj1)->get_raw_ptr(name))

#define test_relation(name, obj1, obj2) (((obj1) == NULL || (obj2) == NULL) ?\
                           0 : (obj1)->test_rel(*(obj2), name)) 


inline void rem_relation(RelType* name, Relational *obj1, 
                                        Relational *obj2 = NULL) 
     {
	if (obj1) 
	   if (obj2) 
	       obj1->rem_rel(*obj2, name);
	   else
	       obj1->rem_rel(name);
     }

#define define_hard_assoc(name,inv_rel_name)                      \
       extern paste(HardAssoc,name)* name;                        \
       extern paste(HardAssoc,inv_rel_name)* inv_rel_name

// create an instance of HardAssocEquate rel type (like  qute/unqute)

#define define_hard_equate(name, inv_rel_name)                  \
       extern HardAssocEquate * name;                           \
       extern HardAssocEquate * inv_rel_name

#define define_unirel_hard(name)                                \
       extern paste(HardAssoc,name)* name

#define init_unirel_hard(name, crdn, flag, type)          \
  paste(HardAssoc,name)* name = new paste(HardAssoc,name)(quote(name),\
                                    crdn, flag, (RelType *)1, type)    

#define init_hard_assoc(name1, card1, flag1,name2, card2, flag2)             \
  paste(HardAssoc,name1)* name1 = new paste(HardAssoc,name1)(quote(name1),   \
                                  card1, flag1);                             \
  paste(HardAssoc,name2)* name2 = new paste(HardAssoc,name2)(quote(name2),   \
                                  card2, flag2, name1)


#define init_hard_assoc_eq(n1, card1, flag1, e_type1,n2, card2, flag2, e_type2)\
 HardAssocEquate * n1 = new HardAssocEquate (quote(n1), card1, flag1, 0, e_type1);\
 HardAssocEquate * n2 = new HardAssocEquate (quote(n2), card2, flag2, n1, e_type2)

 void dd_put_ifl(RelType* rel, Relational* o1, Relational*o2);
 Obj* dd_get_ifl(RelType* rel, const Relational* ob);
 void dd_rem_ifl(RelType* rel, Relational* o1, Relational*o2);


// class 1 is one , e.g root of leaf

#define  __defrel(class1,name1,class2,name2)                              \
 extern  RelType * paste3(name1,_of_,name2);                              \
 inline Obj *    paste3(name2,_getit_,name1) (const Relational * name2)   \
  {return  get_relation(paste3(name1,_of_,name2),name2);}                 \
 inline    int    paste4(atp_,name1,_of_,name2)(Relational*){return 0;}   \
 inline    int    paste4(off_,name1,_of_,name2)(Relational*){return 0;}

#define  defrel_one_to(class1,name1,class2,name2)                         \
__defrel(class1,name1,class2,name2)                                       \
 inline    int   paste4(card_,name1,_of_,name2)(){return 1;}              \
 inline class1 * paste3(name2,_get_,name1) (const class2 * name2)         \
  {return (class1*) paste3(name2,_getit_,name1)(name2);}                  \
 inline void     paste3(name2,_put_,name1) (class2 * name2,class1 * name1)\
  {put_relation(paste3(name1,_of_,name2),name2,name1);}                   \
 inline void     paste3(name2,_rem_,name1) (class2 * name2)               \
  {rem_relation(paste3(name1,_of_,name2),name2,NULL);}

#define  defifl_one_to(class1,name1,class2,name2)                         \
__defrel(class1,name1,class2,name2)                                       \
 inline    int   paste4(atp_,name1,_of_,name2)(class2*){return IFL_ASSOC;}\
 inline    int   paste4(card_,name1,_of_,name2)(){return 1;}              \
 inline class1 * paste3(name2,_get_,name1) (const class2 * name2)         \
  {return (class1*) dd_get_ifl(paste3(name1,_of_,name2),name2);}         \
 inline void     paste3(name2,_put_,name1) (class2 * name2,class1 * name1)\
  {dd_put_ifl(paste3(name1,_of_,name2),name2,name1);}                     \
 inline void     paste3(name2,_rem_,name1) (class2 * name2)               \
  {dd_rem_ifl(paste3(name1,_of_,name2),name2,NULL);}

// class 1 is many , e.g leaf of root

#define  defrel_many_to(class1,name1,class2,name2)                        \
__defrel(class1,name1,class2,name2)                              \
 inline    int   paste4(card_,name1,_of_,name2)(){return MANY;}           \
 inline Obj *    paste4(name2,_get_,name1,s) (const class2 * name2)       \
  {return  paste3(name2,_getit_,name1)(name2);}                           \
 inline void     paste3(name2,_put_,name1) (class2 * name2,class1 * name1)\
  {put_relation(paste3(name1,_of_,name2),name2,name1);}                   \
 inline void     paste3(name2,_rem_,name1) (class2 * name2,class1 * name1)\
  {rem_relation(paste3(name1,_of_,name2),name2,name1);}                   \
 inline void     paste4(name2,_rem_,name1,s) (class2 * name2)             \
  {rem_relation(paste3(name1,_of_,name2),name2,NULL);}

#define  defifl_many_to(class1,name1,class2,name2)                        \
__defrel(class1,name1,class2,name2)                              \
 inline    int   paste4(atp_,name1,_of_,name2)(class2*){return IFL_ASSOC;}\
 inline    int   paste4(card_,name1,_of_,name2)(){return MANY;}           \
 inline Obj *    paste4(name2,_get_,name1,s) (const class2 * name2)       \
  {return dd_get_ifl(paste3(name1,_of_,name2),name2);}                    \
 inline void     paste3(name2,_put_,name1) (class2 * name2,class1 * name1)\
  {dd_put_ifl(paste3(name1,_of_,name2),name2,name1);}                     \
 inline void     paste3(name2,_rem_,name1) (class2 * name2,class1 * name1)\
  {dd_rem_ifl(paste3(name1,_of_,name2),name2,name1);}                     \
 inline void     paste4(name2,_rem_,name1,s) (class2 * name2)             \
  {dd_rem_ifl(paste3(name1,_of_,name2),name2,NULL);}


#define defrel_to_one(c1,n1,c2,n2) defrel_one_to(c2,n2,c1,n1)
#define defrel_to_many(c1,n1,c2,n2) defrel_many_to(c2,n2,c1,n1)

#define defifl_to_one(c1,n1,c2,n2) defifl_one_to(c2,n2,c1,n1)
#define defifl_to_many(c1,n1,c2,n2) defifl_many_to(c2,n2,c1,n1)

#define defrel_one_to_one(class1,name1,class2,name2)                    \
   defrel_to_one(class1,name1,class2,name2);                            \
   defrel_to_one(class2,name2,class1,name1)                      

#define defrel_one_to_many(class1,name1,class2,name2)                   \
   defrel_to_many(class1,name1,class2,name2);                           \
   defrel_to_one(class2,name2,class1,name1)                      

#define defrel_many_to_one(class1,name1,class2,name2)                   \
   defrel_to_one(class1,name1,class2,name2);                            \
   defrel_to_many(class2,name2,class1,name1)                      

#define defrel_many_to_many(class1,name1,class2,name2)                  \
   defrel_to_many(class1,name1,class2,name2);                           \
   defrel_to_many(class2,name2,class1,name1)                    

#define defifl_one_to_one(class1,name1,class2,name2)                    \
   defifl_to_one(class1,name1,class2,name2);                            \
   defifl_to_one(class2,name2,class1,name1)                      

#define defifl_one_to_many(class1,name1,class2,name2)                   \
   defifl_to_many(class1,name1,class2,name2);                           \
   defifl_to_one(class2,name2,class1,name1)                      

#define defifl_many_to_one(class1,name1,class2,name2)                   \
   defifl_to_one(class1,name1,class2,name2);                            \
   defifl_to_many(class2,name2,class1,name1)                      

#define defifl_many_to_many(class1,name1,class2,name2)                  \
   defifl_to_many(class1,name1,class2,name2);                           \
   defifl_to_many(class2,name2,class1,name1)                    

#define declare_rel(c1,n1,c2,n2)                   \
  extern  RelType * __rn(n1,n2);  \
  extern  RelType * __rn(n2,n1)

#define __rp (Relational *)

inline void obj_rem_ptr(RelType * rt, Obj ** op)
{
   Relational *xx = ((Relational*) ((unsigned)op - (unsigned)rt->offset));
   xx->rem_rel(rt);
}

extern char * obj_relname(const char*n1,const char*n2); // n1_of_n2
#define __rlnm(x,y) obj_relname(quote(x),quote(y))

#define define_ptr(class1,name1,class2,name2)                   \
friend Obj * paste3(name1,_getit_,name2)(const class1 * name1)  \
  {return name1->paste4(_,name2,_of_,name1).ptr;}               \
friend    int paste4(off_,name2,_of_,name1)(class1 *)           \
 {return (int)&((class1 * )0)->paste4(_,name2,_of_,name1).ptr;} \
struct paste4(st_,name2,_of_,name1)  {          \
  Obj * ptr;                                    \
  paste4(st_,name2,_of_,name1)() : ptr(0) {}                    \
  paste4(st_,name2,_of_,name1)                                  \
   (const paste4(st_,name2,_of_,name1) &) : ptr(0){}            \
  paste4(st_,name2,_of_,name1) & operator =                     \
   (const paste4(st_,name2,_of_,name1) &) {ptr=0;return *this;} \
 ~paste4(st_,name2,_of_,name1)()                                \
  {if(ptr) obj_rem_ptr((RelType*) paste3(name2,_of_,name1),&ptr);}    \
} paste4(_,name2,_of_,name1)


#define init_one_ptr(class1,name1,f1,class2,name2,f2)       \
  RelType * paste3(name1,_of_,name2)   =   \
  new RelType (__rlnm(name1,name2),       \
  paste4(card_,name1,_of_,name2)(), f1,   \
  paste3(name2,_of_,name1),                        \
  paste4(atp_,name1,_of_,name2)((class2 *)0),      \
  paste4(off_,name1,_of_,name2)((class2 *)0),      \
  &class2::_descrpt)

#define init_rel_or_ptr(class1,name1,f1,class2,name2,f2)    \
 init_one_ptr(class1,name1,f1,class2,name2,f2);             \
 init_one_ptr(class2,name2,f2,class1,name1,f1)

#endif // _objRelmacros_h

/*
   START-LOG-------------------------------------------

   $Log: objRelmacros.h  $
   Revision 1.5 2000/07/07 08:11:56EDT sschmidt 
   Port to SUNpro 5 compiler
 * Revision 1.2.1.6  1993/10/12  15:23:30  kws
 * Port (unsigned pointer arithmetic (for both hp and sun)
 *
 * Revision 1.2.1.5  1993/07/15  23:29:22  mg
 * operator new
 *
 * Revision 1.2.1.4  1993/01/28  23:12:25  aharlap
 * added macro try_relation
 *
 * Revision 1.2.1.3  1992/12/25  16:44:36  mg
 * new ifl
 *
 * Revision 1.2.1.2  1992/10/09  18:58:17  boris
 * Fix comment
 *


   END-LOG---------------------------------------------

*/
