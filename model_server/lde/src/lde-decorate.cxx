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
// Adds symbols and connections to ldr.
//
//---------------------------------------------
// synopsis: Adds symbols and connections to ldr
//           tree.
//
//---------------------------------------------


#include "genError.h"
#include "objRelation.h"
#include "ldrNode.h"

// Returns a symbol node if it exists, else returns NULL
inline static ldrSymbolNodePtr get_symbol_node(ldrNodePtr lnode)
{
   Initialize(get_symbol_node);

   // get first child
   ldrNodePtr first_child = checked_cast(ldrNode, lnode->get_first());

   // Verify first child to be a symbol.
   if (first_child && (first_child->ldrIsA(ldrSymbolNodeType)))
      ReturnValue (checked_cast(ldrSymbolNode, first_child));
   else
      ReturnValue (NULL);
}

// Returns a connection node if it exists, else returns NULL
inline static ldrConnectionNodePtr get_connection_node(ldrNodePtr lnode)
{
   Initialize(get_connection_node);

   // get to the last child
   ldrNodePtr last_child = checked_cast(ldrNode, lnode->get_first());
   while (last_child->get_next() != NULL)
      last_child = checked_cast(ldrNode, last_child->get_next());

   // Verify last child to be a connection.
   if (last_child && (last_child->ldrIsA(ldrConnectionNodeType)))
      ReturnValue (checked_cast(ldrConnectionNode, last_child));
   else
      ReturnValue (NULL);
}

void _lde_decorate(ldrHierarchyNodePtr, int);

void lde_decorate (ldrNodePtr root)
{
   Initialize(lde_decorate);
   _lde_decorate (checked_cast(ldrHierarchyNode,root), 0);
}

void _lde_decorate (ldrHierarchyNodePtr root, int level)
{
   ldrSymbolNodePtr root_symbol;
   ldrNodePtr last_child;
   ldrConnectionNodePtr connection;
   int with_children_p = 0;
   
   Initialize (lde_decorate);

   // create root symbol if it doesnot exist
   if ( (root_symbol = get_symbol_node(root)) == NULL ) {
      root_symbol = db_new(ldrSymbolNode, (root->get_symbol_type()));
      root->put_first(root_symbol);
   }

   root_symbol->set_graphlevel(level);

   // create connection node if it doesnot exist
   if ( (connection = get_connection_node (root)) == NULL)
      connection = db_new(ldrConnectionNode,(ldrcHIERARCHY));
   else
      connection->remove_from_tree();

   // Go through brothers and sisters of root recursively
   last_child = checked_cast(ldrNode, root_symbol);
   for(ldrNodePtr child = checked_cast(ldrNode, root_symbol->get_next());
       child != NULL;
       child = checked_cast(ldrNode, child->get_next())) {

      with_children_p= 1;

      _lde_decorate(checked_cast(ldrHierarchyNode,child), level+1);

      ldrSymbolNodePtr child_symbol = get_symbol_node(child);
      connection->add_output(child_symbol);

      last_child = child;
   }

   if (with_children_p) {
      connection->add_input(root_symbol);
      connection->set_graphlevel(level+1);
      last_child->put_after(connection);
   }
   else delete(connection);
   Return;
}


//  adjust_hierarchy_level does not create any new nodes, but sets
//  levels on each of the symbols and connections of the subtree.

void adjust_hierarchy_levels (ldrHierarchyNodePtr root, int level)
{
   ldrSymbolNodePtr root_symbol;
   Initialize (lde_decorate);
   root_symbol = checked_cast(ldrSymbolNode, root->get_first());
   root_symbol->set_graphlevel(level);
   for(ldrNodePtr child = checked_cast(ldrNode, root_symbol->get_next());
       child != NULL;
       child = checked_cast(ldrNode, child->get_next())) {
      int new_level = level + 1;
      if (child->ldrIsA(ldrConnectionNodeType))
	child->set_graphlevel(new_level);
      else adjust_hierarchy_levels(checked_cast(ldrHierarchyNode,child), new_level);
   }
   Return;
}

/*
   START-LOG-------------------------------------------

   $Log: lde-decorate.cxx  $
   Revision 1.1 1992/11/21 17:17:14EST builder 
   made from unix file
Revision 1.2.1.3  1992/11/21  22:16:34  builder
typesafe casts.

Revision 1.2.1.2  1992/10/09  19:18:53  jon
RCS History Marker Fixup



   END-LOG---------------------------------------------

*/
