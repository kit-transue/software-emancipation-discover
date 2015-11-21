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
// set_constraints
//------------------------------------------
// synopsis: accept constraints on nodes from the caller.
//
// description: imposes constraints specified by the caller on
//              nodes. The file contains 2 interface routines to set
//              and to remove constraints of given type. The 
//              acceptable types are :
//
//                  - _equal_row
//                  - _equal_column
//                  - _equal_position.
//
//              Priorities are used in set_constr in case of conflicts of
//              the constraints. EDGE router should invoke Constraints
//              layout, i.e. call view_edge_init(root, constraints).
//              All other layout algorithms ignore constraints.
//
//------------------------------------------
// Restrictions:
// ...
//------------------------------------------

// include files

#include "all.h"
#include "constr.h"
#include "globdef.h"
#include "graph.h"

#include "EDGEAset.h"
#include "viewEdge.h"

#include "aset_graph.h"
#include "aset_node.h"

#include "genError.h"
//#include "objOper.h"
#include "viewNode.h"

/* declarations of external C functions */

RelClass(objArr);
RelClass(viewSymbolNode);

extern aset_node  *edge_get_view_ptr(aset_graph *, viewSymbolNodePtr);

static int change_constr( aset_graph *graph_node, mode  c_mode, 
                          constraint_type  c_type, objArrPtr v_node_arr,
                          int  c_priority);


// ------------  remove constraints --------------------

int edge_remove_constr( aset_graph *graph_node,
                        constraint_type  c_type, 
                        objArrPtr v_node_arr)
{
   int c_stat = change_constr( graph_node, UNDO,
                               c_type,  v_node_arr, 0);
   return c_stat;
}


// ------------  insert constraints --------------------

int edge_set_constr( aset_graph *graph_node,
                     constraint_type c_type,   // can be EQUAL_ROW
                                           //        EQUAL_COLUMN
                                           //        EQUAL_POSITION
                     objArrPtr v_node_arr,
                     int  c_priority)  
{

// DO is in conflict with ASet DO
//   int c_stat =  change_constr( graph_node, DO, c_type,

   int c_stat =  change_constr( graph_node, (mode) 0, c_type,
                                v_node_arr, c_priority);
   return c_stat;
}



// ------------  actual work is done here

static int change_constr( aset_graph *graph_node,
                     mode  c_mode,  // DO = 0 or UNDO = 1
                     constraint_type c_type,   // can be EQUAL_ROW
                                           //        EQUAL_COLUMN
                                           //        EQUAL_POSITION
                     objArrPtr v_node_arr,
                     int  c_priority)  
{
    nlist             node_list;
    node              *curr_node;
    ObjPtr            elem;
    int               c_stat;

//--------- Executable Code ------------------------------

    Initialize (change_constr);

//  get edge nodes corresponding view nodes and make a node list

    ForEach (elem, *v_node_arr){
       curr_node = edge_get_view_ptr(graph_node, checked_cast(viewSymbolNode,elem));
       if (curr_node == NULL)
           Error(ERR_FAIL);

       node_list.insert(curr_node);
    }

//  call appropriate function to set constraints

    switch (c_type) {

        case EQUAL_ROW :
          c_stat = graph_node->constraints->equal_line(USER, &node_list,
                                          c_mode, c_priority);
          break;

        case EQUAL_COLUMN :
          c_stat = graph_node->constraints->equal_column(USER, &node_list,
                                          c_mode, c_priority);
          break;

        case EQUAL_POSITION :
          c_stat = graph_node->constraints->equal_position(USER, 
                             &node_list, c_mode, c_priority);
          break;    

        default:
           Error(ERR_FAIL);
    }
    ReturnValue(c_stat);
}
/*
   START-LOG-------------------------------------------

   $Log: set_constraints.cxx  $
   Revision 1.1 1993/04/19 13:41:03EDT builder 
   made from unix file
Revision 1.2.1.4  1993/04/19  17:43:38  sergey
Minor correction to clean the warning. Part of bug #3420.

Revision 1.2.1.3  1992/11/21  21:26:40  builder
typesafe casts.

Revision 1.2.1.2  1992/10/09  18:57:49  smit
*** empty log message ***


   END-LOG---------------------------------------------

*/
