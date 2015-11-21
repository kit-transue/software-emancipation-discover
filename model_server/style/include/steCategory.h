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
//////////////////////////  FILE steCategory.h   ///////////////////////////
//
// -- Contains  related steCategory Class Definition
//
#ifndef steCategory_h
#define steCategory_h

#ifndef _objRelation_h
#include <objRelation.h>
#endif
#ifndef steSlot_h      
#include <steSlot.h>
#endif
#ifndef __steCategory_types_h__      
#include <steCategory_types.h>
#endif      

RelClass(steStyle);

define_relation(slotstyle_of_category,category_of_slotstyle);
define_relation(system_category_of_app,app_of_system_category);
define_relation(user_category_of_app,app_of_user_category);
define_relation(node_of_category,category_of_node);

enum ste_category_type {
   Ste_User,
   Ste_System
};

class steCategory : public Relational{

char *category_name;
int type;
steCategory_type  category_type; // user defined and system defined
static char *equate_categ_name;

  public :

  virtual void print(ostream&, int) const;
  define_relational(steCategory,Relational);

  steCategory (char *, steSlotPtr);
  steCategory (char *, steCategory_type category_type = USER_DEFINED_CATEGORY);
#ifdef __GNUG__  
  steCategory (const steCategory& oo){*this = oo;};
#endif
  declare_copy_member(steCategory);

  char const *get_name() const;
  const char *name() const;

  void put_name(char *);

  void set_type( int p ) {type = p;}
  int get_type() {return type;}

  steStylePtr get_category_style();
  steSlotPtr get_category_styleslot();

  static char * equate_category_name() { return equate_categ_name; }

  void set_category_type (steCategory_type);
  steCategory_type get_category_type () { return category_type; }
};

  generate_descriptor(steCategory,Relational);

#endif

/*
   START-LOG-------------------------------------------

   $Log: steCategory.h  $
   Revision 1.2 1995/08/26 09:38:28EDT wmm 
   New group/subsystem implementation
 * Revision 1.5  1993/07/14  01:57:37  boris
 * uninline rel_copy()
 *
 * Revision 1.4  1993/07/12  21:00:16  aharlap
 * *** empty log message ***
 *
 * Revision 1.3  1993/02/04  18:49:18  boris
 * fixed bug #1235 (memory leak)
 *
 * Revision 1.2  1992/12/18  19:10:48  glenn
 * Transfer from STE
 *
 * Revision 1.2.1.2  92/10/09  20:00:47  boris
 * Fix comment
 * 


   END-LOG---------------------------------------------

*/
