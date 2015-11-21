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
// Member functions for RTL ldr header
//
//---------------------------------------------
// synopsis:
//
// Member functions:
//
// History:  08/30/91   Smit            Initial coding.
//           11/09/91   S.Spivakovksy   Added set_appTree() to constructor.
//           11/14/91   S.Spivakovksy   Correction of insert_obj.
//
//---------------------------------------------
#include "objOperate.h"
#include <ldrRTLNode.h>
#include "ldrRTLHierarchy.h"
#include "lde-RTL.h"
#include "ldrError.h"
#include "ldrRTLDisplayType.h"
#include "browserRTL.h"

init_relational (ldrRTLHierarchy, ldrHierarchy);

ldrRTLHierarchy::ldrRTLHierarchy (RTLPtr rtl, int ,
		                  ldrRTLDisplayType d_type)
{
   Initialize(ldrRTLHierarchy::ldrRTLHierarchy);

   ldrRTLNodePtr root = NULL;
   RTLNodePtr app_root;

   if (rtl) {
      ldr_put_app(this, rtl);

      app_root = checked_cast(RTLNode, rtl->get_root());

      root = lde_extract_RTL_tree (app_root, d_type);
      root->set_appTree(app_root);
   }

   this->put_root (root);
}

int ldrRTLHierarchy::dummy_ldrRTLHierarchy ()
{
  Initialize (ldrRTLHierarchy::dummy_ldrRTLHierarchy);

  ReturnValue(0);
}

ldrRTLHierarchy::~ldrRTLHierarchy()
{
}

void ldrRTLHierarchy::build_selection(const ldrSelection& olds, OperPoint&    app_point)
{
   ldrRTLNodePtr ldr_node;

   Initialize (ldrRTLHierarchy::build_selection);

   ldr_node = checked_cast(ldrRTLNode, olds.ldr_symbol->get_parent());

   symbolPtr temp = ldr_node->get_obj();
   app_point.node = checked_cast(appTree, temp);

   if (!(int)app_point.node)
      Error(BAD_LDR_SELECTION);

   if (olds.ldr_connector == NULL)
     app_point.type = REPLACE;
   else if (olds.ldr_connector->is_input_p
            (checked_cast(ldrSymbolNode, olds.ldr_symbol)))
     app_point.type = FIRST;
   else if (olds.ldr_connector->is_output_p
            (checked_cast(ldrSymbolNode, olds.ldr_symbol)))
     app_point.type = AFTER;
   else Error(BAD_LDR_SELECTION);
}

void ldrRTLHierarchy::insert_obj (objInserter* old_inserter,
                               objInserter* new_inserter)
{
   ldrRTLNodePtr src_ldr, target_ldr;
   ldrRTLNodePtr root_ldr;
   ObjPtr src_obj, target_obj;


   Initialize (ldrRTLHierarchy::insert_obj);

   if ( old_inserter->type == CHANGE_NAME )
       return;

   src_obj = old_inserter->src_obj;
   target_obj = old_inserter->targ_obj;

// don't check names - obsolete

/*    if (!((RelationalPtr)src_obj)->get_name()) {
*               new_inserter->type = NULLOP;
*                return;
*    }
*/

   root_ldr = checked_cast(ldrRTLNode, this->get_root());

   // get target ldr
   target_ldr = lde_find_RTLldrNode (root_ldr, target_obj);

   if (target_ldr == NULL) {
      new_inserter->type = NULLOP;
      return;
   }
   
   // extract ldr tree
   if (target_obj == src_obj && is_RTLNode ( (RelationalPtr) src_obj) )
   {
       // means regenerating LDR
       src_ldr = checked_cast(ldrRTLNode, lde_extract_RTL_tree
	   ( checked_cast(RTLNode,src_obj), 
	    (ldrRTLDisplayType) root_ldr->get_display_flag()));
         	    
   }
   else {
       src_ldr = lde_find_RTLldrNode (root_ldr, src_obj);
       if (!src_ldr){
            symbolPtr sym_tmp;
            sym_tmp = src_obj;
            src_ldr = checked_cast(ldrRTLNode, lde_create_RTLldrNode 
	                   ( sym_tmp, (ldrRTLDisplayType) root_ldr->get_display_flag()));
            src_obj = (Obj *)sym_tmp;
       }

   }

   switch (old_inserter->type) {

     case (FIRST):
         lde_RTL_insert_first (target_ldr, src_ldr);
         break;

     case (AFTER):
         lde_RTL_insert_after (target_ldr, src_ldr);
         break;

     case (REPLACE) :

         if (target_ldr->get_parent()) {
	     if (target_ldr == src_ldr && !target_ldr->is_rename_occurred()) {
                 if ( is_view((Relational *)src_obj) ){    // RTL contains view_hdr
         		 new_inserter->type = NULLOP;
		         return;
                 }
	     } else if (target_ldr != src_ldr) {
		 lde_RTL_insert_after (target_ldr, src_ldr);
		 lde_detach (target_ldr);
	     }
         } 
         else {
           objDeleter exterminator;
	   obj_delete(*(RelationalPtr) target_ldr, &exterminator);
           this->put_root (src_ldr);
           src_ldr->set_appTree(checked_cast(appTree,src_obj));
         }

         break;

     default: 
         new_inserter->type = NULLOP;
         return;
   }
   
   // Set up new inserter

   if (target_obj == src_obj){
      new_inserter->type = REPLACE;
      new_inserter->src_obj = src_ldr;
      new_inserter->targ_obj = src_ldr;
   }
   else if (src_ldr->get_prev()) {
      new_inserter->type = AFTER;
      new_inserter->src_obj = src_ldr;
      new_inserter->targ_obj = src_ldr->get_prev();
   } else if (src_ldr->get_parent()) {
      new_inserter->type = FIRST;
      new_inserter->src_obj = src_ldr;
      new_inserter->targ_obj = src_ldr->get_parent();
   } else
      new_inserter->type = NULLOP;
}

void ldrRTLHierarchy::remove_obj(objRemover *old_remover,
				 objRemover *new_remover)
{
   ldrRTLNodePtr src_ldr, root_ldr;
   ObjPtr src_obj;

   Initialize(ldrRTLHierarchy::remove_obj);

   src_obj = old_remover->src_obj;

   // get root
   root_ldr = checked_cast(ldrRTLNode, this->get_root());

   // find ldr for this object
   src_ldr = lde_find_RTLldrNode (root_ldr, src_obj);

   // detach the ldr
   if (src_ldr)
      lde_detach( checked_cast(ldrNode, src_ldr ));

   // setup new remover
   new_remover->src_obj = src_ldr;
}

void ldrRTLHierarchy::hilite_obj(objHiliter *old_hiliter,
				 objHiliter *new_hiliter)
{
   ldrRTLNodePtr src_ldr, root_ldr;
   ObjPtr src_obj;

   Initialize(ldrRTLHierarchy::hilite_obj);

   src_obj = old_hiliter->src_obj;

   root_ldr = checked_cast(ldrRTLNode, this->get_root());

   // find ldr for this object
   src_ldr = lde_find_RTLldrNode (root_ldr, src_obj);

   if (src_ldr == NULL) Error(MISSING_LDR);

   new_hiliter->type    = old_hiliter->type;

   new_hiliter->src_obj = src_ldr;
}

void ldrRTLHierarchy::focus_obj(objFocuser *old_focuser,
				objFocuser *new_focuser)
{
   ldrRTLNodePtr src_ldr, root_ldr;
   ObjPtr src_obj;

   Initialize(ldrRTLHierarchy::focus_obj);

   src_obj = old_focuser->src_obj;

   root_ldr = checked_cast(ldrRTLNode, this->get_root());

   // find ldr for this object
   src_ldr = lde_find_RTLldrNode (root_ldr, src_obj);

   if (src_ldr == NULL) Error(MISSING_LDR);

   new_focuser->type = old_focuser->type;

   new_focuser->src_obj = src_ldr;
}

void ldrRTLHierarchy::send_string(ostream& stream) const
{
   ldrRTLNodePtr root_ldr;

   Initialize (ldrRTLHierarchy::send_string);

   // get root
   root_ldr = checked_cast(ldrRTLNode, this->get_root());

   root_ldr->send_string(stream);
}

/*
   START-LOG-------------------------------------------

   $Log: ldrRTLHierarchy.cxx  $
   Revision 1.6 1997/05/21 12:15:12EDT twight 
   Bug track: 14168
   Reviewed by: mb
   
   Changed parameter from 'ldrSelection& const' to 'const ldrSelection&'.
Revision 1.2.1.11  1993/06/14  18:04:27  sergey
More carefull filtering in REPLACE of inser_obj. Fixed bugs #3663 and 3398.

Revision 1.2.1.10  1993/06/09  22:25:57  boris
Fixed bug #3641 with viewerShell blinking

Revision 1.2.1.9  1993/04/20  16:09:58  sergey
Corrected argumnet passing in ldr_create_RTLldrNode. Part of bug #3376.

Revision 1.2.1.8  1993/04/16  19:49:10  sergey
Removed RTL name checking which stopped propagation. Part of bug #2818.

Revision 1.2.1.7  1993/03/18  20:24:39  aharlap
cleanup for paraset compatibility

Revision 1.2.1.6  1992/12/21  03:12:28  aharlap
selection use symbolPtr

Revision 1.2.1.5  1992/12/15  19:30:47  aharlap
changed for new xref

Revision 1.2.1.3  1992/11/21  22:54:33  builder
typesafe casts.

Revision 1.2.1.2  1992/10/09  19:43:56  jon
RCS History Marker Fixup



   END-LOG---------------------------------------------

*/
