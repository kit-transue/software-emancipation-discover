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
// synopsis : lde for RTL
//
//----------------------------------------------------------
 
#include <cLibraryFunctions.h>
#include "steDisplayNode.h"
#include "steTextNode.h"
#include "ldrTypes.h"
#include "ldrRTLNode.h"
#include "lde-hierarchy.h"
#include "lde-RTL.h"
#include "steSlot.h"
#include "RTL_externs.h"
#include "ParaCancel.h"

 
static ldrRTLNodePtr build_header_node (symbolPtr& obj, ldrRTLDisplayType d_type)
{
   ldrRTLNodePtr ldr_node;
 
   Initialize (build_header_node);
 
   ldr_node = db_new (ldrRTLNode, ());
   ldr_node->set_display_flag (d_type);
   ldr_node->set_obj (obj);
   ldr_node->set_ascending (0);
 
   ldr_node->set_role(ldrCONTAINMENT);
   ldr_node->set_symbol_type (ldrLIST);
   ldr_node->set_old_name();
   ReturnValue (ldr_node);
}
 
static ldrRTLNodePtr build_application_node (symbolPtr& obj)
{
     Initialize (build_application_node);
     ldrRTLNodePtr node = build_header_node (obj, ldrRTLDisplayType(0));
     return node;
}
 
// Creates ldr node for element of an RTL
static ldrRTLNodePtr build_rtl_element_node
        (symbolPtr& obj, ldrRTLDisplayType d_type)
{
   ldrRTLNodePtr    rtl_node, child_node, first_node = NULL, last_node = NULL;
   int              i;
 
   Initialize (build_rtl_element_node);
 
   if ( (d_type > 0) &&
        (d_type != rtlDisplayUnknown) ) {
 
      // create a parent node
      rtl_node = build_header_node (obj, d_type);
   } else
      rtl_node = NULL;
 
   if (d_type == 0) {
      first_node = build_application_node (obj);
      first_node->set_display_flag (d_type);
      first_node->set_ascending (0);
   }
   else
      for (i = 0; i < 4; i++) {
 
        if (d_type & (1 << i)) {
 
          child_node = build_application_node (obj);
          child_node->set_ascending (0);
          child_node->set_display_flag ((1 << i));
 
          // insert in the hierarchy
          if (last_node) {
             last_node->put_after (child_node);
             last_node = child_node;
          }
          else {
             last_node = first_node = child_node;
 
             if (rtl_node)
                rtl_node->put_first(child_node);
          }
       }
     }
 
   if (rtl_node)
      ReturnValue (rtl_node);
   else
      ReturnValue (first_node);
}
 
 
// create an ldr node for an object
ldrRTLNodePtr lde_create_RTLldrNode (symbolPtr& obj, ldrRTLDisplayType d_type)
{
   ldrRTLNodePtr ldr_node;
 
   Initialize (lde_create_RTLldrNode);
 
   ldr_node = build_rtl_element_node (obj, d_type);
 
   ReturnValue (ldr_node);
}
 
ldrRTLNodePtr lde_find_RTLldrNode (ldrRTLNodePtr first, symbolPtr obj)
{
   ldrRTLNodePtr r_node = NULL;
   ldrNodePtr node;
 
   Initialize (lde_find_RTLldrNode);
 
   if (!is_ldrRTLNode (first)) return NULL;

   if (first) {
 
      // match with self
      if (first->ldrIsA(ldrNodeType) &&
	  ldrRTLNodePtr(first)->get_obj() == obj)
         return (first);
 
      node = checked_cast(ldrNode,first->get_first());
 
      while (node) {
 
	  if (is_ldrRTLNode (node) && 
	      (r_node = lde_find_RTLldrNode (ldrRTLNodePtr(node), obj)))
	      ReturnValue (r_node);
 
	  node = checked_cast(ldrNode, node->get_next());
      }
   }
 
   ReturnValue (NULL);
}
 
static int compare (ldrRTLNodePtr node1, ldrRTLNodePtr node2,char *sort_spec)
{
   Initialize(compare);
   commonTreePtr obj1, obj2;

// used to be RelationalPtrs
   symbolPtr temp1 = node1->get_obj();
   symbolPtr temp2 = node2->get_obj();
   obj1 = checked_cast(commonTree, temp1);
   obj2 = checked_cast(commonTree, temp2);
 
   if (obj1 && obj2) {
      return rtl_compare_elems(obj1, obj2, sort_spec);
   }
 
   return 0;
}
 
// inserts the node.
ldrRTLNodePtr lde_insert_RTLldrNode (ldrRTLNodePtr first,
                        ldrRTLNodePtr ldr_node)
{
   ldrNodePtr prev_node;
 
   Initialize (lde_insert_RTLldrNode);
 
   // get first ldr node
   if (first->get_first()) {
      prev_node = checked_cast(ldrNode, first->get_last());
 
      if (prev_node->ldrIsA (ldrConnectionNodeType))
	  prev_node->put_before (ldr_node);
      else
	  prev_node->put_after (ldr_node);
 
   } else {
     first->put_first(ldr_node);
   }
 
   // return root
   ReturnValue (first);
}
 
// builds ldr tree without decoration
static ldrRTLNodePtr lde_build_RTL_tree(
    RTLNodePtr rtl, ldrRTLDisplayType d_type)
{
    Initialize (lde_build_RTL_tree);
 
    symbolArr& rtl_contents = rtl->rtl_contents();
 
    // create the root node first
    symbolPtr rrr(rtl);
    ldrRTLNodePtr root = build_header_node(rrr, d_type);
 
    ldrRTLNodePtr prev = NULL;
    for(int i = 0; i < rtl_contents.size(); ++i)
    {
        if (ParaCancel::is_cancelled()) break;

	symbolPtr& obj = rtl_contents[i];
	if(obj.get_name())
	{
	    ldrRTLNodePtr node =
		lde_create_RTLldrNode(obj, d_type);
 
	    if(prev)
		prev->put_after(node);
	    else
		root->put_first(node);
 
	    prev = node;
	}
    }
 
    ReturnValue(root);
}
 
 
ldrRTLNodePtr lde_extract_RTL_tree(
    RTLNodePtr rtl, ldrRTLDisplayType d_type)
{
   Initialize (lde_extract_RTL_tree);
 
   ldrRTLNodePtr new_ldr = lde_build_RTL_tree(rtl, d_type);
   lde_decorate(new_ldr);
 
   ReturnValue(new_ldr);
}
 
 
void lde_RTL_insert_first(ldrRTLNodePtr target_ldr, ldrRTLNodePtr src_ldr)
{
   Initialize (lde_RTL_insert_first);
 
      lde_decorate (src_ldr);
      lde_insert_first (target_ldr, src_ldr);
 
if(0) lde_insert_RTLldrNode(target_ldr,src_ldr);
 
 
   Return
}
 
void lde_RTL_insert_after (ldrRTLNodePtr target_ldr,
                           ldrRTLNodePtr src_ldr)
{
   Initialize (lde_RTL_insert_after);
 
     // insert at the specified place
     lde_decorate (src_ldr);
     lde_insert_after (target_ldr, src_ldr);
 
 
   Return
}
 
/*
   START-LOG-------------------------------------------

   $Log: lde-RTL.cxx  $
   Revision 1.5 2001/07/25 20:42:02EDT Sudha Kallem (sudha) 
   Changes to support model_server.
Revision 1.2.1.10  1994/02/01  15:55:39  builder
Port

Revision 1.2.1.9  1993/11/30  00:07:41  kws
Bug track: 4162
Allow ability to cancel RTL extraction

Revision 1.2.1.8  1993/06/09  22:28:09  boris
Fixed bug #3641 with viewerShell blinking

Revision 1.2.1.7  1992/12/15  19:27:12  aharlap
changed for new xref

Revision 1.2.1.6  1992/12/09  21:08:30  aharlap
added interface for new xref

Revision 1.2.1.5  1992/11/24  20:18:02  smit
Fix all kinds of casting problems.

Revision 1.2.1.4  1992/11/21  22:16:34  builder
typesafe casts.

Revision 1.2.1.3  1992/10/19  16:53:12  wmm
Change if (0) to #if 0 to comment
out code; otherwise, can't import
into alphaSET.

Revision 1.2.1.2  92/10/09  19:16:57  jon
RCS History Marker Fixup



   END-LOG---------------------------------------------

*/
