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
//////////////////////////   FILE steSlot.h  ///////////////////////////
//
// -- Contains  related Class Definition                  
//
#ifndef steSlot_h
#define steSlot_h

#ifndef _objOper_h
#include <objOper.h>
#endif
#ifndef _objString_h      
#include <objString.h>
#endif      

define_relation(slot_of_obj,obj_of_slot);
define_relation(prev_slot,next_slot);

RelClass(steSlotTable);

class objCopier;

class steSlot : public appTree {

   int delete_flag;
   objString *name_string;
 public :
   
   define_relational(steSlot, appTree);

   steSlot() { };
#ifdef ___GNUG___
   steSlot(const steSlot& oo){*this = oo; }
#endif
   declare_copy_member(steSlot);
   
   steSlot( Obj *cur);
   steSlot( steSlot * );
   ~steSlot();

   Obj *get_own_obj() const;
   Obj *get_slot_obj() const;
   steSlot * get_slot_prev() const;
   Obj *get_slot_next() const;
   steSlotTablePtr get_slot_table() const;
   int get_slot_table_type() const;
   void set_slot_prev(steSlot * s);
   void remove_slot_obj();
   void set_slot_obj(Obj *);   
   int get_index();
   virtual char const *get_name() const;
   int is_category();
   void set_delete_flag( int p = 1 );
   int get_delete_flag();
   objString *get_name_string();
   void set_name_string ( objString *str_obj = NULL );

   steSlot( objString *str );

   virtual appTreePtr rel_copy_for_header (appPtr, objCopier *);

};
   generate_descriptor(steSlot, appTree);

#endif

/*
   START-LOG-------------------------------------------

   $Log: steSlot.h  $
   Revision 1.2 1995/08/26 09:38:32EDT wmm 
   New group/subsystem implementation
 * Revision 1.4  1993/07/14  01:57:37  boris
 * uninline rel_copy()
 *
 * Revision 1.3  1993/03/18  17:04:09  boris
 * Fixed compiling bug with illegal indirection after CC compiler
 *
 * Revision 1.2  1992/12/18  19:10:48  glenn
 * Transfer from STE
 *
 * Revision 1.2.1.2  92/10/09  20:01:12  boris
 * Fix comment
 * 


   END-LOG---------------------------------------------

*/
