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
// RTL_interface.C
//------------------------------------------
// synopsis:
// C++ Interface support for RTL
//
// description:
//------------------------------------------
//
//
//------------------------------------------
// Restrictions:
// ...
//------------------------------------------


#include "genError.h"
#include "RTL.h"
#include "BooleanTree.h"
#include "RTL_externs.h"
#include <objOperate.h>


void rtl_add_obj (RTLNode *rtl, symbolPtr symbol)
{
   Initialize (rtl_add_obj);

   if (rtl) {
       if (symbol.is_xrefSymbol()) {
       	   if (! rtl->rtl_includes (symbol))
	   	rtl->rtl_insert (symbol);
       } else {
       	   Obj* obj = symbol;  //no load !!
	   RTLPtr head = (RTLPtr) rtl->get_header();
	   head->obj_insert (FIRST, (commonTree*)obj, rtl, NULL);
       }
   }
}

void rtl_delete_obj (RTLNode *rtl, Obj *obj)
{
   Initialize (rtl_delete_obj);

   if (rtl) {
      objRemover ro(checked_cast(objTree,obj));
      obj_modify(*(rtl->get_header()), &ro);
   }

}
