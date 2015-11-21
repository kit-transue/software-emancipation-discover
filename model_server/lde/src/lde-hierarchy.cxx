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
#include "genError.h"
#include "ldrNode.h"
#include "ldrError.h"
#include "lde-decorate.h"

static ldrConnectionNodePtr find_connection(ldrNodePtr);
static ldrSymbolNodePtr     root_symbol(ldrNodePtr);
static void clear_connection_from_parent(ldrNodePtr);

// This file contains functions which modify
// ldr hierarchy tree  in a  consistent  way
// consistent with.  It would be  a nicer to
// define  hierarchy tree as a special class
// with  modification functions  defined  as
// methods.

void lde_insert_after(ldrNodePtr insertion_point, ldrNodePtr new_tree)
{
   Initialize(lde_insert_after);
   clear_connection_from_parent (new_tree);
   ldrNodePtr the_parent = checked_cast(ldrNode, insertion_point->get_parent());
   ldrConnectionNodePtr connection = find_connection(the_parent);
   ldrSymbolNodePtr insertion_point_symbol;
   int level;
   if (connection == NULL) Error(NO_CONNECTION);
   connection->add_output(root_symbol(new_tree));
   insertion_point->put_after(new_tree);
   insertion_point_symbol = root_symbol(insertion_point);
   level = insertion_point_symbol->get_graphlevel();
   adjust_hierarchy_levels(checked_cast(ldrHierarchyNode,new_tree), level);
}

void lde_insert_first(ldrNodePtr insertion_point, ldrNodePtr new_tree)
{
   ldrConnectionNodePtr connection;
   ldrSymbolNodePtr root_sym = root_symbol(insertion_point);
   int level;
   
   Initialize(lde_insert_first);
   clear_connection_from_parent (new_tree);
   root_sym = root_symbol(insertion_point);
   level = root_sym->get_graphlevel();
   connection = find_connection(insertion_point);
   root_sym->put_after(new_tree);
   if (connection == NULL) {
      ldrSymbolNodePtr root_sym = root_symbol(insertion_point);
      connection = db_new(ldrConnectionNode,(ldrcHIERARCHY));
      connection->set_graphlevel(level + 1);
      connection->add_input(root_sym);
      new_tree->put_after(connection);
   }
   connection->add_output(root_symbol(new_tree));
   adjust_hierarchy_levels(checked_cast(ldrHierarchyNode, new_tree), level + 1);
}

void lde_detach(ldrNodePtr root_node)
{
   Initialize(lde_detach);
   
   ldrNodePtr the_parent = checked_cast(ldrNode, root_node->get_parent());
   if (the_parent)     //  not the root
   {
       ldrConnectionNodePtr connection = find_connection(the_parent);
       if (connection == NULL) 
           Error(NO_CONNECTION);
       connection->remove_output(root_symbol(root_node));
       if (connection->get_output_symbols() == NULL)
           connection->remove_from_tree();
   }
   root_node->remove_from_tree();
}


///////////////// Local Stuff ////////////////////////////////////////////

// get_upper_connection      returns     the
// connector in which "sym" is an output.

static
ldrConnectionNodePtr get_upper_connection (ldrSymbolNodePtr sym)
{
   Initialize(get_upper_connection);
   ldrNodePtr parent = checked_cast(ldrNode, sym->get_parent());
   ldrNodePtr grand_parent = checked_cast(ldrNode, parent->get_parent());
   ldrConnectionNodePtr the_connection = checked_cast(ldrConnectionNode, NULL);
   for(ldrNodePtr child = checked_cast(ldrNode, grand_parent->get_first());
       child != NULL;
       child = checked_cast(ldrNode, child->get_next())) {
      if (child->ldrIsA(ldrConnectionNodeType)) {
	 the_connection = checked_cast(ldrConnectionNode, child);
	 IF (! the_connection->is_output_p (sym)) {
	    Error(NOT_CHILD);
	 }

      }
   }
   return(the_connection);
}

// find_connection finds a  connection  node
// among children of root.   There  are must
// be exactly one connection.

static
ldrConnectionNodePtr find_connection(ldrNodePtr root)
{
   Initialize(find_connection);

   ldrConnectionNodePtr connection = NULL;
   for(ldrNodePtr child = checked_cast(ldrNode, root->get_first());
       child != NULL;
       child = checked_cast(ldrNode, child->get_next())) {
      if (child->ldrIsA(ldrConnectionNodeType)) {
	 if (connection != NULL)
	   Error(TOO_MANY_CONNECTIONS);
	 else connection = checked_cast(ldrConnectionNode, child);
      }
   }
   return(connection);
}

// root_symbol  returns  the  first child of
// parent_node. This  returned node  must be
// an ldrSymbolNode

static
ldrSymbolNodePtr root_symbol (ldrNodePtr parent_node)
{
   Initialize(root_symbol);
   ldrNodePtr the_symbol = checked_cast(ldrNode, parent_node->get_first());
   if (! the_symbol->ldrIsA(ldrSymbolNodeType)) {
      Error(NOT_SYMBOL);
   }
   return(checked_cast(ldrSymbolNode, the_symbol));
}

static void clear_connection_from_parent (ldrNodePtr node)
{
    Initialize(clear_connection_from_parent);

    if (!node) return;

    // get to the parent
    ldrNodePtr parent = checked_cast(ldrNode, node->get_parent());
    if (!parent) return;

    // find connection among children of parent
    ldrConnectionNodePtr conn = find_connection (parent);
    if (!conn) return;

    // remove connection to this node
    ldrSymbolNodePtr snode = root_symbol (node);
    if (snode) {
	conn->remove_output (snode);
	conn->remove_input (snode);
    }
}

/*
   START-LOG-------------------------------------------

   $Log: lde-hierarchy.cxx  $
   Revision 1.1 1993/06/07 15:13:13EDT builder 
   made from unix file
Revision 1.2.1.6  1993/06/07  19:17:10  sergey
More changes in lde_detach(). Part of bug #2174.

Revision 1.2.1.5  1993/06/06  21:39:21  sergey
Correction in lde_detach() for root case. Fixed bug #3582.

Revision 1.2.1.4  1992/12/09  19:28:32  smit
fix bug # 2069

Revision 1.2.1.3  1992/11/21  22:16:34  builder
typesafe casts.

Revision 1.2.1.2  1992/10/09  19:18:55  jon
RCS History Marker Fixup



   END-LOG---------------------------------------------

*/
