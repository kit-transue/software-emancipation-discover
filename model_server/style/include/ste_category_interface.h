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
//////////////////////////   FILE ste_category_interface.h  /////////////////
//
// -- Contains user inerface functions for category handler. 
//
#ifndef ste_category_interface_h
#define ste_category_interface_h

#include "steSlot.h"
#include "steView.h"

RelClass(steCategory);

extern void ste_remove_category_from_node();
extern void ste_remove_category_from_document();
extern void ste_turn_on_category(steSlotPtr, steViewPtr v = NULL);
extern void ste_turn_off_category(steSlotPtr, steViewPtr v = NULL);
extern void ste_turn_off_categories(steViewPtr v = NULL);
extern void ste_set_active_category(steSlotPtr);
extern void ste_unset_active_category();
extern void ste_check_active_category();
extern void ste_delete_category( steSlotPtr );
extern steSlotPtr ste_define_category(char *name,
            steSlotPtr style, char *gl_flag);
extern void ste_modify_category(steSlotPtr cat, steSlotPtr style, char *nm);

#endif

/*
   START-LOG-------------------------------------------

   $Log: ste_category_interface.h  $
   Revision 1.1 1993/07/28 18:35:24EDT builder 
   made from unix file
 * Revision 1.2  1992/12/18  19:10:48  glenn
 * Transfer from STE
 *
 * Revision 1.2.1.2  92/10/09  20:01:23  boris
 * Fix comment
 * 


   END-LOG---------------------------------------------

*/
