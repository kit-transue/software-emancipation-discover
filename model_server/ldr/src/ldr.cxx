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
// ldr.h.C
//------------------------------------------
// classes :
//     ldr, ldrTree.
//
// description : Methods for the above classes.
//
#include <cLibraryFunctions.h>
#include <msg.h>
#include <machdep.h>
#include <objOperate.h>
#include <objCollection.h>
#include <objPropagate.h>

#include <reference.h>
#include <appMarker.h>

#include <assoc.h>
#include <RTL.h>
#include <symbolArr.h>
#include <OperPoint.h>

#include <symbolPtr.h>
#include <view.h>
#include <objAssoc.h>

//------------------------------------------
// Member functions for class ldr
//------------------------------------------

ldr::ldr()
{}

ldr::~ldr()
{}

objTreePtr ldr::find_ldr(objTreePtr source)
{
    Initialize(ldr::find_ldr);

    if (source == NULL)
	return NULL;

    ldrTreePtr ldr_root = checked_cast(ldrTree, get_root() );

    Obj * ldr_node_set = apptree_get_ldrtrees(checked_cast(appTree, source));
    if(ldr_node_set)
    {
	Obj * ldr_node;
	ForEach(ldr_node, *ldr_node_set)
	{
	    if((checked_cast(objTree, ldr_node))->subtree_of(ldr_root))
		return checked_cast(objTree, ldr_node);
	}
    }

    return NULL;
}

objTreePtr ldr::find_ldr_to_rebuild(objTreePtr source)
//
// Do your best to find an ldr under this header for the given 
// app node (source). -jef
// 
{
    Initialize(ldr::find_ldr_to_rebuild);

    ldrTreePtr ldr_root = checked_cast(ldrTree, get_root() );
    appTreePtr app_root = ldrtree_get_apptree( ldr_root );

    // If apptree of the root is a reference node continue with the
    // referenced node (H)
    if (is_ReferenceNode(app_root))
    {
	app_root = get_ReferenceRoot(app_root);

	// special check for root case because root does not have a direct
	// relation to the app tree (apptree_of_ldrtree points to
	// the reference node, not the reference root). -jef
	if (app_root == source)
	    return ldr_root;
    }

    // The app_root might be a sub-node of the source node if
    // we have opened a view on a sub-tree of a app.
    if (app_root == 0 || app_root->subtree_of(source))
	return ldr_root;  // Whole LDR should be regenerated.

    if( source->subtree_of(app_root) )
    {
	for( ; source; source = source->get_parent()) // Work up the tree
	{
	    Obj * ldr_node_set =
		apptree_get_ldrtrees(checked_cast(appTree, source));
	    if(ldr_node_set)
	    {
		Obj * ldr_node;
		ForEach(ldr_node, *ldr_node_set)
		{
		    if((checked_cast(objTree, ldr_node))->subtree_of(ldr_root))
			return checked_cast(objTree, ldr_node);
		}
	    }
	}
    }

    return NULL;
}

ldrPtr ldr::find_ldrHeader( appPtr app_header, int type )
{
    Initialize(ldr::find_ldrHeader);

    Obj * LdrHeader = app_get_ldrs(app_header);

    if (LdrHeader == NULL)
	return checked_cast(ldr, NULL );

    if ( !LdrHeader->collectionp() ) {
        if ( type == (checked_cast(ldr,LdrHeader))->get_type() )
	    return checked_cast(ldr,LdrHeader );
  	else
	    return checked_cast(ldr, NULL );
    }

    Obj *current;
    ForEach( current, *(objSet *)LdrHeader ) {
        if ( type == (checked_cast(ldr,current))->get_type() )
	    return checked_cast(ldr,current );
    }

    return NULL;
}

ldrPtr ldr::find_ldrHeader( appPtr app_header, int type, appTreePtr sub_root )
{
    Initialize(ldr::find_ldrHeader);

    ldrPtr my_ldr;

    Obj * LdrHeader = app_get_ldrs(app_header);

    if (LdrHeader == NULL)
	return NULL;

    Obj *current;
    ForEach( current, *(objSet *)LdrHeader ) {
        my_ldr = checked_cast(ldr,current);
        if ( type == my_ldr->get_type() &&
	    (my_ldr->get_root() == my_ldr->find_ldr(sub_root) ||
	     checked_cast(ldrTree,my_ldr->get_root())->get_appTree() == sub_root))
	    return(my_ldr);
    }

    return( NULL );
}

objMerger* ldr::merge_obj(objMerger *om, objMerger *)
{
    Initialize(ldr::merge_obj);

    appTreePtr parent_node = checked_cast(appTree, om->targ_obj->get_parent());
    if ( parent_node && parent_node->get_parent() )
	parent_node = checked_cast(appTree, parent_node->get_parent());

    IF(!parent_node)Error(ERR_INPUT);

    objInserter   ins_ptr(REPLACE, parent_node, parent_node, NULL);
    obj_modify(*this, &ins_ptr);

    return (objMerger *)NULL;
}

objSplitter* ldr::split_obj(objSplitter *oi, objSplitter *)
{
    Initialize(ldr::split_obj);

    appTreePtr parent_node = checked_cast(appTree, oi->top_obj->get_parent());
    IF(!parent_node)Error(ERR_INPUT);

    objInserter   ins_ptr(REPLACE, parent_node, parent_node, NULL);
    obj_modify(*this, &ins_ptr);

    return (objSplitter *)NULL;
}

appPtr ldr::get_appHeader() const
{
    return ldr_get_app((ldr *)this);
}

void ldr::hilite_obj(objHiliter* old_hiliter, objHiliter* new_hiliter)
{
    Initialize(ldr::hilite_obj);
 
    ldrTreePtr src_ldr = checked_cast(ldrTree,find_ldr(checked_cast(appTree,old_hiliter->src_obj)));
    new_hiliter->type = (src_ldr ? old_hiliter->type : 0);
    new_hiliter->src_obj = src_ldr;
 
    Return
}
 
void ldr::focus_obj(objFocuser* old_focuser, objFocuser* new_focuser)
{
    Initialize(ldr::focus_obj);
   
    ldrTreePtr src_ldr = checked_cast(ldrTree,find_ldr(checked_cast(appTree,old_focuser->src_obj)));
    new_focuser->type = (src_ldr ? old_focuser->type : 0);
    new_focuser->src_obj = src_ldr;
 
    Return
}
 
void ldr::assign_obj(objAssigner* old_assigner, objAssigner* new_assigner)
{
    Initialize(ldr::assign_obj);
 
    ldrTreePtr src_ldr = checked_cast(ldrTree,find_ldr(checked_cast(appTree,old_assigner->src_obj)));
    new_assigner->slot = (src_ldr ? old_assigner->slot : 0);
    new_assigner->src_obj = src_ldr;
 
    Return
}
 
ldrTree *ldr::generate_tree (appTree *, int) {return 0;}
 
//------------------------------------------
// Member functions of class ldrTree
//------------------------------------------

ldrTree::ldrTree()
{
    collapsed = 0;
    readonly = 0;
}

//boris: cludge to fix crash while double clicking in source view
//       should be done through Relations in ste_select.C
extern void ste_select_report_delete (ldrTree *);

ldrTree::~ldrTree()
{
    ste_select_report_delete (this);
}

void ldrTree::send_string( ostream& stream ) const
{
    appTreePtr app_nd = get_appTree();
    if ( app_nd )
	app_nd->send_string( stream );
}

appTreePtr ldrTree::get_appTree() const
{
    return ldrtree_get_apptree((ldrTree *)this);
}

void ldrTree::set_appTree(appTreePtr  at)
{
    ldrtree_put_apptree(this, at);
}

ldrTree::ldrTree(const ldrTree& oo)
   : commonTree(oo), collapsed(oo.collapsed), readonly(oo.readonly) {}

