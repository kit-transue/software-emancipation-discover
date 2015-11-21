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
//////////////////////////   FILE steSlotTable.h  ///////////////////////////
//
// -- Contains  related Class Definition                  
//
/*  SlotTable has two main purposes to exist. First STE needs array that
    can be achieved by Relation Mechanism, i.e. that inherits 
    from Relational. Second, SlotTable is array of slots. Each Slot is
    Relational object that either has relation to actual object or to
    another Slot. That gives flexibility to attach relations not to objects
    directly but to "their representatives". In this case removing of 
    a object from SlotTable should cause removing relation of a Slot to 
    the object and restoring relation to another Slot. All Slot's Relations 
    should not be reassigned. It is convinient espessially with Default
    Styles and Categories Tables.
*/
#ifndef steSlotTable_h
#define steSlotTable_h

#ifndef _objRelation_h
#include <objRelation.h>
#endif      

#ifndef _objArr_h
#include <objArr.h>
#endif
      
RelClass(steSlotTable);
class steSlot;

define_relation(table_of_slot,slot_of_table);


enum ste_table_type {
   Ste_Default,
   App_Level,
   Ste_Local
};

class steSlotTable : public Relational {
   int table_type;
   objArr *slot_array;
 public :
   
   define_relational(steSlotTable,Relational);

   steSlotTable() { };
#ifdef __GNUG__
   steSlotTable (const steSlotTable& oo){*this = oo; }
#endif
   declare_copy_member(steSlotTable);
   
   steSlotTable( int table_tp, objArr *cur = NULL );
   steSlotTable( int table_tp, steSlotTablePtr tbl);
   ~steSlotTable();

   int get_table_type() const {return table_type;}
   void set_table_type(int p) {table_type = p;}
   steSlot * get_slot( int index ) const;
   steSlot * get_slot(const char *st_name) const;
   Obj *get_obj(const char *st_name) const;
   Obj *get_obj( int index ) const;
   int get_obj_index( const RelationalPtr ) const;
   int get_size() const;
   objArr *get_array() const;
   void put_slot(const steSlot * s, int index);
   void put_slots(objArr *ar);
   void add_slot(const steSlot *);
   void add_slots(const steSlotTablePtr);
   void remove_slot(steSlot *);
   void remove_slot(const char *st_name);
   void remove_slot_obj(const char *st_name);
   void set_slot(steSlot *);
   int is_includes(const Obj *p) const {return slot_array->includes(p);}
   void remove_slots();
};
   generate_descriptor(steSlotTable,Relational);

#endif

/*
   START-LOG-------------------------------------------

   $Log: steSlotTable.h  $
   Revision 1.2 1995/08/26 09:38:34EDT wmm 
   New group/subsystem implementation
 * Revision 1.5  1993/07/14  01:57:37  boris
 * uninline rel_copy()
 *
 * Revision 1.4  1993/07/12  21:01:57  aharlap
 * *** empty log message ***
 *
 * Revision 1.3  1993/03/19  01:59:31  aharlap
 * cleanup for paraset compatibility`
 *
 * Revision 1.2  1992/12/18  19:10:48  glenn
 * Transfer from STE
 *
 * Revision 1.2.1.2  92/10/09  20:01:13  boris
 * Fix comment
 * 


   END-LOG---------------------------------------------

*/

