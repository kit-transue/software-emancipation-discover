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
//////////////////////////   FILE steSlot_h.C  ///////////////////////////
//
// -- Contains  related Class Definition                  
//
#include "steSlot.h"
#include "steSlotTable.h"

#include "objRelation.h"
#include "steCategory.h"
#include "steStyle.h"
#include "objString.h"
#include "ste_category_handler.h"
#include "ste_style_handler.h"

init_relation(slot_of_obj,1,NULL,obj_of_slot,1,NULL);
init_relation(prev_slot,1,NULL,next_slot,MANY,NULL);
init_relational(steSlot, appTree);

steSlot::steSlot(Obj *cur){
  delete_flag = 0;
  name_string = (objString *)NULL;
  set_slot_obj(cur);
}

steSlot::steSlot(steSlotPtr s){
  delete_flag = 0;
  name_string = (objString *)NULL;
  set_slot_prev(s);
}

steSlot::steSlot(objString *s){
  delete_flag = 0;
  name_string = s;
}

steSlot::~steSlot() {
  if ( name_string )
      delete name_string;
  else{
      Obj *cur = get_own_obj();
      if(cur)
         delete cur;
  }
}


/************************
*  get_slot_prev
************************/
steSlotPtr steSlot::get_slot_prev() const
{
     return (steSlot *)get_relation(prev_slot,this);
}

Obj *steSlot::get_slot_next() const {
     return get_relation(next_slot,this);
}

steSlotTablePtr steSlot::get_slot_table() const 
{
     return (steSlotTable *)get_relation(table_of_slot,this);
}

int steSlot::get_slot_table_type() const {
     return ( (get_slot_table())->get_table_type() );
}

void steSlot::set_slot_prev(steSlotPtr s){ put_relation(prev_slot,this,s);}

Obj *steSlot::get_own_obj() const {
  return (get_relation(obj_of_slot,this));
}

Obj *steSlot::get_slot_obj() const {
     steSlotPtr prev;
     Obj *answer;
     answer = get_own_obj();
     if ( !answer ) {
         prev = get_slot_prev();
         answer = prev->get_slot_obj();
     }
     return answer;
}    

void steSlot::remove_slot_obj() {
    Obj *cur = get_own_obj();
    if ( cur ) {
        delete cur;
    }
}

void steSlot::set_slot_obj(Obj *cur) {
    remove_slot_obj();
    put_relation(obj_of_slot,this,(RelationalPtr)cur);
}


/***********************
* get_name();
***********************/
char *steSlot::get_name() const {
     char *nm;
     if ( name_string ){
          nm = (char *)*name_string;
          return nm;
     }
     RelationalPtr ob = (RelationalPtr)get_slot_obj();
     return ob->get_name();
}

/***********************
* is_category()
***********************/
int steSlot::is_category() {
 Initialize(steSlot::is_category);
   int ii;
   Obj * ob = get_slot_obj();
   if (is_steCategory((RelationalPtr)ob))
      ii = 1;
   else
      ii = 0;
 
 ReturnValue(ii);
}

int steSlot::get_delete_flag() { return delete_flag; }

/*********
* get_name_string()
*********/
objString *steSlot::get_name_string(){ return name_string; }

/********
* set_name_string()
********/
void steSlot::set_name_string( objString *str ) { name_string = str; }

// copies slot for another app
appTreePtr steSlot::rel_copy_for_header (appPtr appp, objCopier *)
{
    Initialize (steSlot::rel_copy_for_header);

    if ( appp == NULL )
       return 0;

    steSlotPtr r_val = NULL;

    steSlotTablePtr table;

    // get category or style table.
    if (is_category ())
	table = get_create_local_user_category (appp);
    else
	table = get_create_app_style (appp);

    char *name = this->get_name();

    r_val = table->get_slot (name);

    if (!r_val) {
	// Get local object
	RelationalPtr o = (RelationalPtr) get_own_obj();

	// make copy if local object; else set up link with prev slot
	if (o) {
	    o = o->rel_copy();
	    r_val = db_new (steSlot, (o));
	} else {
	    r_val = get_slot_prev();
	    r_val = db_new (steSlot, (r_val));
	}

	// put slot in app's slot table.
	if (r_val)
	    table->set_slot (r_val);
    }

    ReturnValue( checked_cast(appTree, r_val));
}

/*
   START-LOG-------------------------------------------

   $Log: stySlot.cxx  $
   Revision 1.3 2002/01/23 09:57:58EST ktrans 
   Merge from branch: mainly dormant code removal
 * Revision 1.5  1994/08/05  19:09:45  boris
 * Bug track: 8020, 7962
 * Fixed memory problems on big (huge) report generation
 *
 * Revision 1.4  1993/03/03  20:04:27  boris
 * Fixed crash in copy
 *
 * Revision 1.3  1993/03/03  00:48:03  boris
 * Fixed crash while copy error note category
 *
 * Revision 1.2  1992/12/18  19:05:09  glenn
 * Transferred from STE
 *
Revision 1.2.1.4  1992/11/22  03:15:45  builder
typesafe casts.

Revision 1.2.1.3  1992/11/20  19:36:32  boris
Added Decorate Source facilities

Revision 1.2.1.2  1992/10/09  19:48:33  boris
Fix comments



   END-LOG---------------------------------------------

*/
