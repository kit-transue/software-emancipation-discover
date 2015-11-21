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
#ifndef steAppStyle_h
#define steAppStyle_h

#include "steStyle.h"
#include "steSlotTable.h"
#include "steSlot.h"

RelClass(app);
RelClass(steAppStyle);

define_relation(app_of_style,style_of_app);

class steAppStyle : public steSlotTable {

 public :
   
   define_relational(steAppStyle,steSlotTable);

   steAppStyle() { };
//   steAppStyle (const steAppStyle& oo){*this = oo; }
#ifdef __GNUG__
   steAppStyle (const steAppStyle& oo){*this = oo; }
#endif

   declare_copy_member(steAppStyle);
   
   steAppStyle(int tp, objArr *ar = NULL ) : steSlotTable(tp, ar) { }
   steAppStyle(int tp, steSlotTablePtr tbl): steSlotTable(tp, tbl) { }

   steSlotPtr get_styleslot(const char *st_name) const {
        return (steSlotPtr)get_slot(st_name);
   }
   steSlotPtr get_styleslot(int index) const{
        return (steSlotPtr)get_slot(index);
   }
   steSlotPtr get_styleslot(int level, ste_style_type) const;
   steSlotPtr get_default_styleslot() const {
       return (steSlotPtr)get_slot( 0 );
   }
   steStylePtr get_style(const char *st_name) const {
        return (steStylePtr)get_obj(st_name);
   }
   steStylePtr get_style(int index) const{
        return (steStylePtr)get_obj(index);
   }
   steStylePtr get_style(int level, ste_style_type) const;
   steStylePtr get_default_style() const {
       return (steStylePtr)get_obj( 0 );
   }
   void set_app( appPtr apl) { put_relation(app_of_style, this, apl); }
   appPtr get_app() const { return (appPtr)get_relation(app_of_style,this);}
   int get_app_type() const;
};
   generate_descriptor(steAppStyle,steSlotTable);


#endif

/*
   START-LOG-------------------------------------------

   $Log: steAppStyle.h  $
   Revision 1.1 1993/07/13 21:57:10EDT builder 
   made from unix file
 * Revision 1.3  1993/07/14  01:57:37  boris
 * uninline rel_copy()
 *
 * Revision 1.2  1992/12/18  19:10:48  glenn
 * Transfer from STE
 *
 * Revision 1.2.1.2  92/10/09  20:00:40  boris
 * Fix comment
 * 


   END-LOG---------------------------------------------

*/
