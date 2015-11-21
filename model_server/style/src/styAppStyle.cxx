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
//////////////////////////   FILE steAppStyle.h  ///////////////////////////
//
// -- Contains  related Class Definition                  
//
#include "objOper.h"
#include "steAppStyle.h"
#include "steStyle.h"
#include "steSlot.h"
#include "representation.h"

init_relation(app_of_style,1,NULL,style_of_app,1,relationMode::D);
init_relational(steAppStyle,steSlotTable);

/***************************
*   steAppStyle::get_styleslot
***************************/
steSlotPtr steAppStyle::get_styleslot(int level, ste_style_type type) const {
 Initialize(steAppStyle::get_styleslot);
   steStylePtr max = NULL;
   steSlotPtr cur_slot;
   steSlotPtr max_slot = NULL;
   Obj* curr;
   steStylePtr p; 
   ForEach (curr,*(get_array())) {
      cur_slot = checked_cast(steSlot, curr);
      p = checked_cast(steStyle,cur_slot->get_slot_obj());
      IF (!p) ReturnValue(max_slot);
      if (p->style_type() == type) {
	 if (p->level() == level)
	    ReturnValue(cur_slot);
	 if ((!max || max->level() < level) && p->level() < level) {
	    max = p;
            max_slot = cur_slot;
         }
      }
   }
   if (!max_slot) max_slot = get_default_styleslot();
   ReturnValue(max_slot);
}

/***************************
*   steAppStyle::get_style
***************************/
steStylePtr steAppStyle::get_style(int level, ste_style_type type) const {
 Initialize(steAppStyle::get_style);
   steSlotPtr cur_slot = get_styleslot(level,type);
   steStylePtr p = checked_cast(steStyle,cur_slot->get_slot_obj());
 ReturnValue(p);
}
