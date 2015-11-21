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
#include <msg.h>
#include <objOper.h>
#include <genError.h>
#include <steRefNode.h>
#include <steRefTextNode.h>
#include <steTextNode.h>
#include <steDocument.h>
#include <steDisplayHeader.h>
#include <smt.h>
#include <ldrSmtSte.h>
#include <steHeader.h>
#include <messages.h>
#include <xref.h>

#include <reference.h>

steTextNodePtr make_ste_reference(steTextNodePtr);
steTextNodePtr make_smt_reference(smtTreePtr);

extern Obj* dd_get_ifl_info(RelType*, const Relational*);
boolean dd_is_ifl_loaded(ddElement *);

boolean is_smtRefNode (const RelationalPtr node)
{
  if ((is_smtTree(node)) && (smtTreePtr(node)->get_extype () == SMTT_ref))
    return 1;
  else
    return 0;
}

boolean is_ReferenceNode(appTreePtr node)
//
// Determine if a given app node is a reference node
//
{
    Initialize(is_ReferenceNode);

    if (node)
    {
	if (is_steRefNode(node))
	{
	    return 1;
	}
	else if (is_smtRefNode(node)) // nonstandard, defined above
	{
	    return 1;
	}
    }

    return 0;
}

boolean is_ReferenceSource(appTreePtr node)
//
// Determine if a given app node is the source of a reference
//
{
    Initialize(is_ReferenceSource);

    if (node)
    {
	if (get_relation(reference_of_root, node))
	{
	    return 1;
	}
    }

    return 0;
}

boolean is_ReferenceTarget(appTreePtr node)
//
// Determine if a given app node is the target of a reference
//
{
    Initialize(is_ReferenceTarget);

    if (node)
    {
	if (get_relation(root_of_reference, node))
	{
	    return 1;
	}
    }

    return 0;
}

boolean is_ReferenceSourceLoaded(appTreePtr node)
{
    Initialize(is_ReferenceSourceLoaded);

    Obj* ddset = get_relation(root_of_reference, node);
    Obj* el;
    ForEach(el, *ddset) // we really should only have one element
    {
	if (dd_is_ifl_loaded((ddElement*)el))
	    return 1;
    }
    return 0;
}

char* get_ReferenceSourceFilename(appTreePtr node)
{
    Initialize(get_ReferenceSourceFilename);

    if (!is_ReferenceTarget(node))
	return NULL;

    Obj* info = dd_get_ifl_info(root_of_reference, node);
    if (!info) return NULL;

    // we are counting on the fact that we should only
    // have one element (not a colleaction) in info for
    // reference targets.
    fsymbolPtr xref_sym = (symbolPtr)info;

    fsymbolPtr file_sym = xref_sym->get_def_file();

    return file_sym->xrisnotnull() ? file_sym.get_name() : NULL;
}

appTreePtr get_ReferenceRoot(appTreePtr node)
//
// Get root appNode that a reference node references.
//
{
    Initialize(get_ReferenceRoot);

    appTreePtr ref_root = NULL;

    if (node)
    {
	if (is_steRefNode(node))
	{
	    ref_root = reference_get_root(node);
	}
	else if (is_smtRefNode(node)) // nonstandard, defined above
	{

// old code I want to keep around so I can refer to it. -jef
#if 0
	    smtTreePtr first;
   
	    if (node->get_node_type() != SMT_comment)
		return NULL;
   
	    if (!(first = checked_cast(smtTree, node->get_first())))
		return NULL;
   
	    if (first->get_node_type() != SMT_group)
		return NULL;
   
	    if (!(first = checked_cast(smtTree, first->get_first())))
		return NULL;
   
	    if (!is_smtRefNode(first))
		return NULL;

	    return reference_get_root(first);
#endif
	    ref_root = reference_get_root(node);

	}

    }

    if ((!ref_root) && (is_ReferenceNode(node)))
	msg("Error - get_ReferenceRoot: Reference node with no reference relation.\n") << eom;

    return ref_root;
}

// **************
// make_reference
// **************
// Create reference node to be inserted into an ste or smt app tree.

appTreePtr make_reference(appTreePtr node)
{
    Initialize(make_reference);

    appTreePtr ref = NULL;

    // This tries to do something about references of references. -jef
    while (is_ReferenceNode(node)) 
	node=get_ReferenceRoot(node);

    if (node)
    {
	if (is_steTextNode(node))
	    ref = make_ste_reference (checked_cast(steTextNode, node));
	else if (is_smtTree(node))
	    ref = make_smt_reference (checked_cast(smtTree, node));
    }

    return ref;
}

//
// make_ste_reference
//

steTextNodePtr make_ste_reference(steTextNodePtr node)
{
    Initialize(make_ste_reference);

    // Can not do references to a leaf. So, keeps the reference to the original
    // node, but push the text down.
    // what the heck does this do?  -jef
    if (!node->get_first()) {
	appPtr doc = checked_cast(app, node->get_header());
	obj_split (doc, NULL, node);
    }

    // Create the new reference node and set its type to be the same as the
    // node it is referencing
    steTextNodePtr root_node = db_new(steRefTextNode, ());
    root_node->set_node_type((steNodeType)node->get_node_type());

//    reference_put_root(root_node, node);

    return(root_node);
}

//
// make_smt_reference
//

steTextNodePtr make_smt_reference(smtTreePtr node)
{
    Initialize(make_smt_reference);

    // Can not reference smt leaf nodes, need push down function here.
    if (!node->get_first())
	return NULL; // Need to generate an error message here.

    // Create the new reference node and set its type to be STR.
    steTextNodePtr root_node = db_new(steRefTextNode, ());
    root_node->set_node_type(steSTR);

//    reference_put_root(root_node, node);

    return(root_node);
}




