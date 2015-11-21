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
/*
   C support for lex category restore
*/

#include "objOper.h"
#include "steAppCategory.h"
#include "steSlotTable.h"
#include "steSlot.h"
#include "steCategory.h"
#include "steHeader.h"

static steAppCategoryPtr categ = 0;
static steSlotTablePtr   stl   = 0;

void sty_categ( char *cat_nm, char *stl_nm )
{
  Initialize(sty_categ);
   if ( !categ || !stl || !cat_nm || !stl_nm ){
      steHeader(Message)
      << "sty_categ: wrong category/style name"
      << steEnd;
      return;
   }
   steSlotPtr st_sl = stl->get_slot( stl_nm );

   if ( !st_sl ){
      steHeader(Message)
      << "sty_categ: no style"
      << steEnd;
      return;
   }

   steSlotPtr cat_sl = categ->get_slot( cat_nm );
   if ( !cat_sl ){
      steHeader(Message)
      << "sty_categ: no style"
      << steEnd;
      return;
   }

   steCategoryPtr cat_obj = checked_cast(steCategory,cat_sl->get_slot_obj());
   steSlotPtr st_sl_orig = cat_obj->get_category_styleslot();
   if ( st_sl != st_sl_orig )
      put_relation(slotstyle_of_category,cat_obj,st_sl);

   return;
}

void sty_restore(steAppCategoryPtr cat_tbl,steSlotTablePtr stl_tbl,char *buf )
{
  Initialize(sty_restore);
    categ = cat_tbl;
    stl   = stl_tbl;

    int len = strlen(buf);

    int par_ind = 0;
    int quote_ind = 0;

    char *cat_nm;
    char *sty_nm;

    for ( int ii = 0 ; ii < len ; ii++ ){
        if ( buf[ii] == '\"' ){
           if ( quote_ind == 0 ){
              quote_ind = 1;
              if ( par_ind == 0 )
                 cat_nm = &buf[ii + 1];
              else
                 sty_nm = &buf[ii + 1];
           }else{
              quote_ind = 0;
              buf[ ii ] = 0;
              if ( par_ind == 0 )
                 par_ind = 1;
              else{
                 par_ind = 0;
                 sty_categ( cat_nm, sty_nm );
              }
           }
        }
    }

    categ = 0;
    stl   = 0;
}


