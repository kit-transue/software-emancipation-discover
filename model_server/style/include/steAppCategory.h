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
//////////////////////////   FILE steAppCategory.h  ///////////////////////////
//
// -- Contains  related Class Definition                  
//
#ifndef steAppCategory_h
#define steAppCategory_h

#include "steCategory.h"
#include "steSlotTable.h"
#include "steSlot.h"

RelClass(app);
RelClass(steAppCategory);

define_relation(app_of_category,category_of_app);

class steAppCategory : public steSlotTable {

 public :
   
   define_relational(steAppCategory,steSlotTable);

   steAppCategory() { };
//   steAppCategory (const steAppCategory& oo){*this = oo; }
#ifdef __GNUG__
   steAppCategory (const steAppCategory& oo){*this = oo; }
#endif

   declare_copy_member(steAppCategory);
   
   steAppCategory(int tp, objArr *ar = NULL ) : steSlotTable(tp, ar) { }
   steAppCategory(int tp, steSlotTablePtr tbl): steSlotTable(tp, tbl) { }

   steCategoryPtr get_category(const char *st_name) const {
        return (steCategoryPtr)get_obj(st_name);
   }
   steCategoryPtr get_category(int index) const{
        return (steCategoryPtr)get_obj(index);
   }

   int get_app_type() const ;
   void set_app_system( appPtr apl) ;
   appPtr get_app_system();
   void set_app_user( appPtr apl);
   appPtr get_app_user();

   int save_styles( boolean glob = 0 );
   int restore_styles( steSlotTablePtr stl_tbl, boolean glob = 0 );
};
   generate_descriptor(steAppCategory,steSlotTable);


#endif

/*
   START-LOG-------------------------------------------

   $Log: steAppCategory.h  $
   Revision 1.1 1993/07/13 21:57:10EDT builder 
   made from unix file
 * Revision 1.4  1993/07/14  01:57:37  boris
 * uninline rel_copy()
 *
 * Revision 1.3  1993/01/19  23:40:26  boris
 * y
 * Added Category Save
 *
 * Revision 1.2  1992/12/18  19:10:48  glenn
 * Transfer from STE
 *
 * Revision 1.2.1.2  92/10/09  20:00:39  boris
 * Fix comment
 * 


   END-LOG---------------------------------------------

*/
