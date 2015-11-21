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
// lde_reference.h.C
//
#include <msg.h>
#include <objOper.h>
#include <genError.h>
#include <steRefNode.h>
#include <steRefTextNode.h>
#include <steTextNode.h>
#include <steDisplayNode.h>
#include <steDocument.h>
#include <steDisplayHeader.h>
#include <ldrNode.h>
#include <ldrSmtHierarchy.h>

#include <smt.h>
#include <ldrSmtSte.h>
#include <steHeader.h>
#include <reference.h>
#include <messages.h>

#include "lde_smt_text.h"
#include "lde_ste_hierarchy.h"

#include "lde_reference.h"


steDisplayNodePtr extract_reference_ldr_with_header(appTreePtr ref_node)
//
// Build the ldr sub-tree for a ste or smt reference node for ste or s-mode.
// The tree is built by calling the constructor for the needed sub-header.
// Returns the root of the new ldr tree.
//
// This is called by the ldr extractors for ldrSmtSte and steDisplayHeader
// when they run across a ref node in their app tree.
//
{
    Initialize(extract_reference_ldr_with_header);

    if (!ref_node) return (NULL);

    if (!is_ReferenceSourceLoaded(ref_node))
    {
	// The referenced document is not loaded so we build a placeholder.
	steDisplayNodePtr disp_root = db_new(steDisplayNode,());
	disp_root->set_node_type(steSTR);
	steDisplayNodePtr disp_node1 = db_new(steDisplayNode,());
	disp_node1->set_node_type(stePAR);
	steDisplayNodePtr disp_node2 = db_new(steDisplayNode,());
	disp_node2->set_node_type(steREG);

	disp_root ->put_first(disp_node1);
	disp_node1->put_first(disp_node2);

	disp_node2->set_appTree(ref_node);
        disp_node2->set_read_only( 1 );

	return(disp_root);
    }
   
    appTreePtr ref_root = get_ReferenceRoot(ref_node);
    if (!ref_root)
    {
	steHeader(Message)
	    << "extract_reference_lde_with_header - No reference relation"
	    << steEnd;
	return(NULL);
    }

    appPtr app_header = ref_root->get_header();
    if (!app_header) return (NULL);

    int type = app_header->get_type();
    
    ldrPtr ldr_header;
    switch(type)
    {
      case App_STE:
      {
        steDocument* ste_header = checked_cast(steDocument, app_header);
        steTextNode* ste_node = checked_cast(steTextNode,  ref_root);
        ldr_header = db_new(steDisplayHeader, (ste_header,ste_node));
        put_relation(ldr_of_app, ste_header, ldr_header);
        break;
      }
      case App_SMT:
      {
        smtHeader* smt_header = checked_cast(smtHeader, app_header);
        smtTree *  smt_node = checked_cast(smtTree,  ref_root);
        ldr_header = db_new(ldrSmtSte, (smt_header,smt_node));
        put_relation(ldr_of_app, smt_header, ldr_header);
        break;
      }
      default:
        ldr_header = NULL;
	msg("ERROR: Not Implemented Yet") << eom;
        break;
    }

    if (!ldr_header) return(NULL);

    ldrTreePtr ldr_root = checked_cast(ldrTree, ldr_header->get_root());
    if (!ldr_root) return(NULL);

// change in ldr struct. -jef
//    ldr_root->set_appTree(ref_node);

    ldr_root->set_read_only( 1 );

    return( checked_cast(steDisplayNode, ldr_root) );
}

steDisplayNodePtr rebuild_reference_ldr(ldrPtr ldr_header)
//
// Given a reference ldr sub-header, rebuild the tree for that header,
// detach the old tree and paste in the new tree.  Returns the root of
// the old tree.
//
// This is called when ldrSmtSte::insert_obj or steDisplayHeader::insert_obj
// are asked to rebuild a referenced node.
//
{
    Initialize(extract_reference);

    if (!ldr_header) return (NULL);

    ldrNodePtr old_ldr_root = checked_cast(ldrNode, ldr_header->get_root());
    if (!old_ldr_root) return (NULL);

    appTreePtr ref_node = ldrtree_get_apptree(old_ldr_root);
    if (!ref_node) return(NULL);

    appTreePtr ref_root = get_ReferenceRoot(ref_node);
    if (!ref_root)
    {
	steHeader(Message)
	    << "rebuild_reference_lde - No reference relation"
	    << steEnd;
	return(NULL);
    }

    steDisplayNodePtr new_ldr_root = NULL;
    if (is_smtTree(ref_root))
    {
	new_ldr_root =
	    lde_extract_smt_text(checked_cast(smtTree, ref_root), steSTR);
    }
    else if (is_steTextNode(ref_root))
    {
	new_ldr_root = lde_extract_ste_text(checked_cast(steTextNode,ref_root));
    }

    if (!new_ldr_root) return(NULL);

    new_ldr_root->set_appTree(ref_node);

    old_ldr_root->remove_from_tree();

    ldr_header->put_root(new_ldr_root);

    return( checked_cast(steDisplayNode, old_ldr_root) );
}

