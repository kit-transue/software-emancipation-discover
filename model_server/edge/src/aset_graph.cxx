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
// aset_graph::aset_graph
//------------------------------------------
// synopsis: aset_graph constructor.
//
// description: createcos aset_graph object from the input view_node.
//              makes the lists of nodes and edges of graph using
//              the input view tree. For each node sets back pointers
//              to the view source node.
//              For Call Tree filters up/down tree nodes processing
//              only one tree at a time.
//
// restrictions: negative level of aset_node is used to distinguish
//               the upper tree in the Call Trees. It is assumed
//               that in all other cases levels are >= 0.
//
//------------------------------------------
// Restrictions:
// ...
//------------------------------------------

// include files

#include "viewNode.h"
#include "viewERDClassNode.h"
#include "all.h"
#include "globdef.h"
#include "graph.h"

#include "EDGEAset.h"
#include "viewEdge.h"

#include "aset_graph.h"
#include "aset_node.h"
#include "aset_edge.h"

#include "genError.h"
#include "objTree.h"
#include "objOper.h"
#include "ldrNode.h"
 

#include "viewGraHeader.h"
#include "viewNode.h"
#include "driver.h"
#include "dialog.h"

/* declarations of external C functions */

#define DEFAULT_WIND_WIDTH   700
#define DEFAULT_WIND_HEIGHT  700

extern void  edge_build_root(aset_graph *, viewSymbolNodePtr );

extern void  edge_build_root_arr(aset_graph *, objArrPtr );

extern aset_node * edge_create_node(aset_graph *, viewSymbolNodePtr );

extern aset_node  *edge_get_view_ptr(aset_graph *,
                                     viewSymbolNodePtr);

extern void  edge_create_connector(aset_graph *,viewConnectionNodePtr);
                                     
extern void  edge_simple_connector(aset_graph *,
                  viewConnectionNodePtr, 
                  aset_node *src, aset_node *targ,
                  int node_index =0);

extern "C" void ui_get_window_size(void *, int *, int *);

RelClass(objArr);

static bool viewnode_is_hidden (viewSymbolNode* vsn)
{
    Initialize (viewnode_is_hidden);

    viewFuncCallNode* vfcn;

    if (vsn->get_collapsed ())
	return true;

    if (vsn->viewIsKindOf (viewFuncCallType)) {
	vfcn = checked_cast (viewFuncCallNode, vsn);
	return vfcn->prune_level > 0;
    }

    return false;
}

//------------  aset_graph constructor ----------------------

//  input view node comes either as a single root or as an array
//  of view nodes

aset_graph::aset_graph( viewNodePtr root, int x_space, int y_space,
                        objArrPtr root_arr, int uu)
                      : graph((char *) NULL)
{
    Initialize(aset_graph::aset_graph);

    viewSymbolNodePtr top_view;
    viewGraHeaderPtr local_view;
    int        screen_width, screen_height;
    dialogPtr  dlg = NULL;

//--------- Executable Code ------------------------------

    up_tree     = uu;
    int no_view = 0;
    if (root_arr == NULL)
        top_view = checked_cast(viewSymbolNode,root->get_first());
    else
        top_view = checked_cast(viewSymbolNode, (*root_arr)[0]);

//  set window size

    local_view = checked_cast(viewGraHeader,top_view->get_header ());
    if (local_view == NULL)
          no_view = 1;
    else{
          dlg = driver_instance->find_dialog_for_view(local_view);
          if (dlg == NULL)
              no_view = 1;          
    }

    if (no_view == 0){
       void * local_window = dlg->window_of_view(local_view);
       ui_get_window_size(local_window, &screen_width, &screen_height);
    }
    else{
       screen_width = DEFAULT_WIND_WIDTH;
       screen_height = DEFAULT_WIND_HEIGHT;
    }
    this->setxmax(screen_width);
    this->setymax(screen_height);
    this->setorientation(bottom_to_top);
    this->setxspace(x_space);
    this->setyspace(y_space);
    this->setzspace(NODE_SPACE);

//  build EDGE nodes and edges

    if (root_arr == NULL)
        edge_build_root(this, top_view);

    else
        edge_build_root_arr(this, root_arr);
}




//  build the list of nodes and edges; set headnode
//  edgenode, tailnode, tailedge of the graph

    void  edge_build_root(aset_graph *graph_node,
                          viewSymbolNodePtr curr_view)
{

    aset_node *tmp_node;

//--------- Executable Code ------------------------------

    Initialize(edge_build_root);

//  create an EDGE node out of view_node; insert it into 
//  all graph lists.

    while ( curr_view ) {
        if (!curr_view->ignore_for_routing() && 
            !viewnode_is_hidden (curr_view)){
	    if (curr_view->viewIsKindOf(viewConnectorType) )
	         edge_create_connector(graph_node, 
		     checked_cast(viewConnectionNode, curr_view));

	    // regular viewSymbol node
	    else if (curr_view->viewIsA(viewERDClassType) ||
		     curr_view->viewIsA(viewERDBoxType) ||
		     curr_view->viewIsKindOf(viewMultiType) ||
		     curr_view->viewIsA(viewGroupType) ||
		     curr_view->viewIsA(viewSymbolType))
		 tmp_node = edge_create_node(graph_node, curr_view);
        }

//      get the next view node if not collapsed

        curr_view = checked_cast(viewSymbolNode, curr_view->get_next_symbol());
    }

    return;
}



//  same as above but takes an input array  of view nodes.

    void  edge_build_root_arr(aset_graph *graph_node,
                          objArrPtr root_arr)
{
    aset_node *tmp_node;
    Obj        *el;
    viewSymbolNodePtr  curr_view;

//--------- Executable Code ------------------------------

    Initialize(edge_build_root_arr);

//  create an EDGE node out of view_node; insert it into 
//  all graph lists.

    ForEach(el, *root_arr){
        curr_view = checked_cast(viewSymbolNode, el);
        if (!curr_view->ignore_for_routing())
	    if ( curr_view->viewIsKindOf(viewConnectorType) )
		       edge_create_connector(graph_node,
			   checked_cast(viewConnectionNode, curr_view));

	    // regular viewSymbol node
	    else if (curr_view->viewIsA(viewERDClassType) ||
		     curr_view->viewIsA(viewERDBoxType) ||
		     curr_view->viewIsKindOf(viewMultiType) ||
		     curr_view->viewIsA(viewGroupType))
		 tmp_node = edge_create_node(graph_node, curr_view);
    }

    return;
}



 aset_node *  edge_create_node(aset_graph *graph_node, 
                               viewSymbolNodePtr curr_view)
{
    aset_node *root_node;

//--------- Executable Code ------------------------------

    Initialize(edge_create_node);

//  filter up/down tree nodes for Call Tree, but not the ROOT

    if (curr_view){
        int lev = curr_view->get_graphlevel();
        if ( lev != 0 ){
             if ( curr_view->viewIsA(viewFuncCallType) ){
	          ldrFuncCallNodePtr l_node = checked_cast(ldrFuncCallNode,
                        get_relation(ldrtree_of_viewtree, curr_view));
                  if (l_node->upper_tree != graph_node->up_tree)
                        ReturnValue (NULL);
             }

             else if ( curr_view->viewIsKindOf(viewClassType) &&
                  checked_cast(viewClassNode,curr_view)->is_above_root() != 
	          graph_node->up_tree)

                  ReturnValue(NULL);
        }
     }

    node *head_node = graph_node->getheadnode();
    node *tail_node = graph_node->gettailnode();

//  make GraSymbolNode (by get_symbol_size in aset_node 
//  constructor) and create new EDGE node 

    root_node = new aset_node(curr_view);

    if (head_node == NULL){               // empty list
         graph_node->setheadnode(root_node);
         head_node = root_node;
         root_node->setprevnode(NULL);
    }
    else{                                 // list is not empty
         tail_node->setnextnode(root_node);
         root_node->setprevnode(tail_node);
    }

    graph_node->settailnode(root_node);    // add new node to the tail
    root_node->setbackgraph(graph_node);   // back ptr to graph

//    root_node->setssucclist(NULL);
    root_node->setnextnode(NULL);

    ReturnValue (root_node);
}



// Creates EDGE edge node(s) out of view connector. Multiple
// node view connectors are split into simple two node
// EDGE connectors. Each EDGE connector gets an index corresponding
// to the view input[] or output[] node. If view connector has both
// multiple inputs and outputs then an EDGE dummy node of a special
// DUMMYCONNECT_POINT type is created and connected with all 
// inputs and outputs. 


 void edge_create_connector(aset_graph *graph_node,
                            viewConnectionNodePtr view_conn)
{
    aset_node        *dummy_node;   // created for multiple node
                                    // connector
    aset_node        *src_node;
    aset_node        *targ_node;
    objArrPtr        view_inputs;           
    objArrPtr        view_outputs;           
    Obj              *el1;
    Obj              *el2;
    int              ind;
    int              v_level;
    int              x_space;

//--------- Executable Code ------------------------------

    Initialize(edge_create_connector);

//  get input and output nodes of connector

    view_inputs = view_conn->get_inputs();
    view_outputs = view_conn->get_outputs();
    if (view_inputs->size() == 0 || view_outputs->size() == 0)
       return;

    dummy_node = NULL;
    if ( view_inputs->size() > 1 && view_outputs->size() > 1 ){

//      both inputs and outputs are multiple nodes; create a dummy
//      but not EDGE dummy kind

        dummy_node = edge_create_node(graph_node, 
                                      checked_cast(viewSymbolNode, NULL));
        dummy_node->dummy_type = DUMMY_CONNECT_POINT;
        v_level = (checked_cast(viewSymbolNode, (*view_outputs)[0]))->get_graphlevel();
        dummy_node->setlevel(v_level);

//      dummy nodes have NEGATIVE length not to take space

        x_space = - graph_node->getxspace(); 
        dummy_node->setwidth(x_space);
    }

//  process multiple input nodes

    if ( view_inputs->size() > 1 ){
        if (dummy_node == NULL){
            if (viewnode_is_hidden (checked_cast(viewSymbolNode, 
                 (*view_outputs)[0])))
                  return;             // target collapsed

            targ_node = edge_get_view_ptr(graph_node, 
                        checked_cast(viewSymbolNode, (*view_outputs)[0]));
        }
        else
            targ_node = dummy_node;

        ind = 0;
        ForEach(el1, *view_inputs){
            if (viewnode_is_hidden (checked_cast(viewSymbolNode, el1)))
                  continue;             // destination collapsed
            src_node = edge_get_view_ptr(graph_node,
                                       checked_cast(viewSymbolNode, el1));
            edge_simple_connector(graph_node, view_conn, src_node,
                                  targ_node, ind);
            ind ++;
        }
        if ( view_outputs->size() == 1 )
            return;                       // all done
    }

//  process output nodes (simple edge case included)

    if (dummy_node == NULL){
        if (viewnode_is_hidden (checked_cast(viewSymbolNode,
              (*view_inputs)[0])))
            return;
        src_node = edge_get_view_ptr(graph_node, 
                        checked_cast(viewSymbolNode, (*view_inputs)[0] ));
    }
    else
        src_node = dummy_node;

    if ( view_outputs->size() == 1 &&
         ! view_conn->viewIsA(viewERDConnectorType) )
        ind = -1;               // simple connector
    else
        ind = 0;

    ForEach(el2, *view_outputs){
        if (viewnode_is_hidden (checked_cast(viewSymbolNode, el2)))
                  continue;           
        targ_node = edge_get_view_ptr(graph_node,
                                        checked_cast(viewSymbolNode, el2));
        edge_simple_connector(graph_node, view_conn, src_node,
                              targ_node, ind);
/*
      switch direction of the connector for upper tree
   
        if ( targ_node->getlevel() >= 0 )
            edge_simple_connector(graph_node, view_conn, src_node,
                              targ_node, ind);
        else
            edge_simple_connector(graph_node, view_conn, targ_node,
                              src_node, ind);
*/
        ind ++;
    }

    Return
}



 void  edge_simple_connector(aset_graph *graph_node,
                             viewConnectionNodePtr view_conn,
                             aset_node *src_node,
                             aset_node *targ_node,
                             int node_index)
{
    aset_edge        *edge_obj;

//--------- Executable Code ------------------------------

    Initialize(edge_simple_connector);

    if (src_node == NULL || targ_node == NULL)   // no routing
            return;

    edge *head_edge = graph_node->getheadedge();
    edge *tail_edge = graph_node->gettailedge();

//  create new EDGE's edge object

    edge_obj = new aset_edge(view_conn);
    edge_obj->mult_index = node_index;

    if (head_edge == NULL){               // empty list
         graph_node->setheadedge(edge_obj);
         head_edge = edge_obj;
         edge_obj->setprevedge(NULL);
    }
    else{                                 // list is not empty
         tail_edge->setnextedge(edge_obj);
         edge_obj->setprevedge(tail_edge);
    }

    edge_obj->setnextedge(NULL);
    graph_node->settailedge(edge_obj);    // add new edge to the tail
    edge_obj->setbackgraph(graph_node);   // back ptr to graph

//  set src and target node pointers;
//  Note, both nodes appear before connector in the view list , so
//        they have to be already processed at this moment;

    targ_node->addpred(src_node);
    src_node->addsucc(targ_node);

    edge_obj->setsource(src_node);
    edge_obj->settarget(targ_node);

    return;
}





//  Get view_node back pointer; This code can be removed
//  if add a field edge_back_ptr to viewFuncCallNode class.

 aset_node * edge_get_view_ptr(aset_graph *graph_node, 
                               viewSymbolNodePtr view_ptr)
 {
    Initialize(edge_get_view_ptr);

    aset_node *tail_node = (aset_node *)graph_node->gettailnode();

    while (view_ptr->ignore_for_routing() &&
            view_ptr->viewIsKindOf(viewERDBoxableType) &&
            checked_cast(viewERDBoxableNode,view_ptr)->containing_box())
        view_ptr = checked_cast(viewERDBoxableNode,view_ptr)->containing_box();

    while (tail_node){
           if (tail_node->view_node == view_ptr)
               break;
           tail_node = (aset_node *) tail_node->getprevnode();
    }
    return tail_node;
 }

/*
   START-LOG-------------------------------------------

   $Log: aset_graph.cxx  $
   Revision 1.4 2000/11/01 07:54:10EST sschmidt 
   General cleanup
Revision 1.2.1.6  1993/05/13  21:22:59  sergey
Commented out root_node->setssucclist(NULL) to prevent memory leak. Part of fix of bug #3463.

Revision 1.2.1.5  1993/02/17  20:39:54  sergey
Skipped collapsed nodes. Part of bug #2498.

// Revision 1.1  1993/02/17  20:34:43  sergey
// Initial revision
//
Revision 1.2.1.4  1992/11/21  21:26:40  builder
typesafe casts.

Revision 1.2.1.3  1992/10/27  17:04:17  sergey
Added check for curr_view = 0; fixed bug # 1763.

Revision 1.2.1.2  92/10/09  18:57:31  sergey.
   Initial coding.


   END-LOG---------------------------------------------

*/
