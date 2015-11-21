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
/////////////////////////   FILE ste_default_tables.C   //////////////////////
//
// -- Contains static default_app_table handling;
//
#include "ste.h"
#include "objRelation.h"
#include "representation.h"
#include "steSlotTable.h"
#include "objOper.h"
#include "ste_category_handler.h"
#include "ste_style_handler.h"
#include "steSlotTable.h"
#include "steAppStyle.h"
#include "steAppCategory.h"
#include "steCategory.h"
#include "steStyle.h"
#include "steHeader.h"

static objArr default_app_table;

/***********************
* ste_init_fill_default_table;
***********************/
// Inserts tree array items for each application type. 
//
//
void ste_init_fill_default_table() {
 Initialize(ste_init_fill_default_table);
   for(int ii = 0 ; ii < App_LAST ; ii++)
      for (int jj = 0; jj < Ste_Type_Last ; jj++) 
          default_app_table.insert_last( (Obj *)NULL );

}

/************************
*  set_app_table();
************************/
void set_app_table( int app_tp, int table_tp, steSlotTablePtr tbl ) {
 Initialize(set_app_table);
      if ( (table_tp < Ste_Type_Last) && (app_tp < App_LAST) ){
          int index = app_tp * Ste_Type_Last + table_tp;
          default_app_table[ index ] = (Obj *)tbl;
      }else{
          steHeader(Message)
          << "set_app_table - unknown table type."
          << steEnd;
      }
 return;
}

/************************
*  get_app_table( int app_tp, int table_tp );
************************/
steSlotTablePtr get_app_table( int app_tp, int table_tp ) {
 Initialize(get_app_table);
   steSlotTablePtr tbl;
      table_tp = ( table_tp >= Ste_Type_Last ) ? 0 : table_tp;
      app_tp = ( app_tp >= App_LAST ) ? 0 : app_tp;
      int index = app_tp * Ste_Type_Last + table_tp;
      tbl = checked_cast(steSlotTable,default_app_table[ index ]);
      if ( !tbl ) tbl = ste_main->get_default(table_tp);      
 return (tbl);
}

/************************
* get_create_default_tables
************************/
void get_create_default_tables( appPtr appl) {
    Initialize(get_create_default_tables);

   int ap_tp = appl->get_type();
   steSlotTablePtr def_st = get_app_table(ap_tp, Ste_Style);
   steSlotTablePtr def_cat_s = get_app_table(ap_tp, System_Category);
   steSlotTablePtr def_cat_u = get_app_table(ap_tp, User_Category);

   steAppStylePtr tbl1 = 
        checked_cast(steAppStyle,get_relation(style_of_app,appl));
   if (!tbl1) {
      tbl1 = db_new(steAppStyle,(Ste_Local,def_st));
      (checked_cast(steAppStyle,tbl1))->set_app(appl);
   } 

   steAppCategoryPtr tbl2 = checked_cast(steAppCategory,
         get_relation(system_category_of_app,appl));
   if (!tbl2) {
      tbl2 = db_new(steAppCategory,(Ste_Local,def_cat_s));
      tbl2->set_app_system(appl);
   } 
   steAppCategoryPtr tbl3 = checked_cast(steAppCategory,
        get_relation(user_category_of_app,appl));
   if (!tbl3) {
      tbl3 = db_new(steAppCategory,(Ste_Local,def_cat_u));
      tbl3->set_app_user(appl);
   } 
}

/*
   START-LOG-------------------------------------------

   $Log: sty_default_tables.cxx  $
   Revision 1.1 1993/02/05 15:28:20EST builder 
   made from unix file
 * Revision 1.2  1992/12/18  19:05:09  glenn
 * Transferred from STE
 *
Revision 1.2.1.4  1992/11/25  14:59:54  boris
Added Raw/Smode print facilities

Revision 1.2.1.3  1992/11/22  03:17:38  builder
atypesafe casts.

Revision 1.2.1.2  1992/10/09  19:49:02  boris
Fix comments



   END-LOG---------------------------------------------

*/
