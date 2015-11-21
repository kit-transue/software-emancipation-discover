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
//////////////////////////   FILE steSlotTable_h.C  ///////////////////////////
//
// -- Contains  related Class Definition                  
//
#include "objOper.h"
#include "objRelation.h"
#include "steSlotTable.h"
#include "steSlot.h"

init_relational(steSlotTable,Relational);
init_relation(table_of_slot,1,NULL,slot_of_table,MANY,NULL);

steSlotTable::~steSlotTable() {
   remove_slots();
   delete slot_array;
}


/***************************
*   steSlotTable::steSlotTable
***************************/
steSlotTable::steSlotTable(int tp, objArr *ar) {
    Initialize(steSlotTable::steSlotTable);

    table_type = tp;
    slot_array = db_new(objArr,());
    Obj *curr;
    steSlotPtr cur_sl;

    if (ar == NULL) return;

    ForEach(curr, *ar){
        if ( is_steSlot((RelationalPtr)curr) ) 
           cur_sl = db_new(steSlot,(checked_cast(steSlot,curr)));        
        else
           cur_sl = db_new(steSlot,(curr));        
        slot_array->insert_last(cur_sl);
        set_slot(cur_sl);
    }
}


/***************************
*   steSlotTable::steSlotTable
***************************/
steSlotTable::steSlotTable(int tp, steSlotTablePtr tbl) {
    table_type = tp;
    slot_array = db_new(objArr,());
    steSlotPtr curr;
    steSlotPtr cur_sl;

    if (tbl == NULL) return;

    int sz = tbl->get_size();
    for ( int ii = 0 ; ii < sz ; ii++ ) {
        curr = tbl->get_slot(ii);
        cur_sl = db_new(steSlot,(curr));        
        slot_array->insert_last(cur_sl);
        set_slot(cur_sl);
    }
}

/***************************
*   steSlotTable::get_slot
***************************/
steSlotPtr steSlotTable::get_slot(const char *st_name) const {
    Initialize(steSlotTable::get_slot);

    steSlotPtr answer = NULL;
    Obj *curr;
    steSlotPtr cur_slot;
    char *cur_name;

    if ( !st_name || slot_array->size() == 0 )
           return answer;

    ForEach(curr,*slot_array){
       cur_slot = checked_cast(steSlot,curr);
       cur_name = cur_slot->get_name();
       if ( !strcmp(st_name,cur_name) ) {
           answer = cur_slot;
           break;
       }
    }
 return answer;
}

/***************************
*   steSlotTable::get_slot
***************************/
steSlotPtr steSlotTable::get_slot(int index) const {
    Initialize(steSlotTable::get_slot);

    steSlotPtr answer = NULL;

    if( slot_array->size() > index )
          answer = checked_cast(steSlot,(*slot_array)[index]);

    return answer;
}

/***************************
*   steSlotTable::get_obj
***************************/
Obj *steSlotTable::get_obj(const char *st_name) const {
    Initialize(steSlotTable::get_obj);

    Obj *answer = NULL;
    Obj *curr;
    steSlotPtr cur_slot;
    char *cur_name;

    if ( !st_name || slot_array->size() == 0 )
           return answer;

    ForEach(curr,*slot_array){
       cur_slot = checked_cast(steSlot,curr);
       cur_name = cur_slot->get_name();
       if ( !strcmp(st_name,cur_name) ) {
           answer = cur_slot->get_slot_obj();
           break;
       }
    }
 return answer;
}

/***************************
*   steSlotTable::get_obj
***************************/
Obj *steSlotTable::get_obj(int index) const {
    Initialize(steSlotTable::get_obj);

    Obj *answer = NULL;

    if( slot_array->size() > index )
          answer = (checked_cast(steSlot,(*slot_array)[index]))->get_slot_obj();

    return answer;
}

/***************************
*   steSlotTable::get_size
***************************/
int  steSlotTable::get_size() const {
    return slot_array->size();
}

/***************************
*   steSlotTable::get_array
***************************/
objArr *steSlotTable::get_array() const {
    return slot_array;
}

/***************************
*   steSlotTable::set_slot
***************************/
void steSlotTable::set_slot(steSlotPtr s) {
    put_relation(slot_of_table,this,s);
}

/***************************
*   steSlotTable::put_slot
***************************/
void steSlotTable::put_slot(const steSlot* s, int index) { // GNU
    Initialize(steSlotTable::put_slot);

    if ( slot_array->size() > index )
      {
       (*slot_array)[index] = (Obj *)s;
       set_slot(checked_cast(steSlot,s));
      }
}
/***************************
*   steSlotTable::put_slots
***************************/
void steSlotTable::put_slots(objArr *ar) {
    Initialize(steSlotTable::put_slots);

    remove_slots();
    delete slot_array;
    slot_array = ar;
// set slot-table relations
    Obj *curr;
    ForEach(curr, *ar) {
       set_slot(checked_cast(steSlot,curr));
    }
}
/***************************
*   steSlotTable::add_slot
***************************/
void steSlotTable::add_slot(const steSlot* s) { // GNU
    Initialize(steSlotTable::add_slot);

    slot_array->insert_last(s);
    set_slot(checked_cast(steSlot,s));
}
/***************************
*   steSlotTable::add_slots
***************************/
void steSlotTable::add_slots(const steSlotTablePtr s) {
    steSlotPtr curr;
    Obj *cur_obj;
 Initialize(steSlotTable::add_slots);
    int i_max = s->get_size();
    for (int ii = 0 ; ii < i_max ; ii++)
       {
        cur_obj = s->get_obj(ii);
        curr = db_new(steSlot,(cur_obj));
        add_slot(curr);
       }
 Return;
}

/***************************
*   steSlotTable::remove_slot
***************************/
void steSlotTable::remove_slot(steSlot* s) { // GNU
    slot_array->remove(s);
    delete s;
}

/***************************
*   steSlotTable::remove_slots
***************************/
void steSlotTable::remove_slots() {
 Initialize(steSlotTable::remove_slots);
    Obj *ob = get_relation(slot_of_table,this);
    if ( !ob ) Return;
    objSet obj_set = ob;

    objArr *new_arr = new objArr;
    objArr *cur = slot_array;
    slot_array = new_arr;

    delete cur;


    Obj *my_obj;
    steSlotPtr sl;
    ForEach(my_obj,obj_set){
       sl = checked_cast(steSlot,my_obj);
       delete sl;
    }
 Return;
}

/*
   START-LOG-------------------------------------------

   $Log: stySlotTable.cxx  $
   Revision 1.2 2002/01/23 09:57:59EST ktrans 
   Merge from branch: mainly dormant code removal
 * Revision 1.3  1993/03/19  02:02:34  aharlap
 * cleanup for paraset compatibility
 *
 * Revision 1.2  1992/12/18  19:05:09  glenn
 * Transferred from STE
 *
Revision 1.2.1.4  1992/11/22  03:15:45  builder
typesafe casts.

Revision 1.2.1.3  1992/11/20  19:36:32  boris
Added Decorate Source facilities

Revision 1.2.1.2  1992/10/09  19:48:32  boris
Fix comments



   END-LOG---------------------------------------------

*/
