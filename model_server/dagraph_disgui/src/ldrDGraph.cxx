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
//
// ldrDGraph.C
// -----------
//

#include <parray.h>

#include <RTL.h>
#include <appDGraph.h>
#include <ldrHeader.h>
#include <ldrSelection.h>
#include <ddSelector.h>
#include <OperPoint.h>
#include <xrefSymbol.h>

#include <DGNode.h>
#include <DGNodeData.h>
#include <DGNodeDataXREF.h>
#include <DAGraph.h>
#include <DAGHandler.h>
#include <DAGHandlerXREF.h>

#include <ldrNode.h>
#include <ldrDGNode.h>
#include <ldrDGraph.h>

//========================================================= ldrDGNode

init_relational(ldrDGNode,ldrSymbolNode);
implement_copy_member(ldrDGNode);
 
// -- HELP: need to pass ldrDGNode to ldrSymbolNode(x)
 
ldrDGNode::ldrDGNode(symbolPtr & sym_in)
: ldrSymbolNode(ldrXREF_METHOD)
{
    x_sym = sym_in;
}
 
int
ldrDGNode::ldrIsA(ldrNodeTypes id) const
{
    return ldrSymbolNodeType == id; // -- HELP: ldrDGNodeType == id
}
 
void
ldrDGNode::send_string(ostream & stream) const
{
    if (x_sym.is_xrefSymbol()) {
	char * x_nm = x_sym.get_name();
	if (x_nm)
	    stream << x_nm;
    }
}
 
objArr *
ldrDGNode::get_tok_list(int mode)
{
    return 0;
}
 
int
ldrDGNode::get_symbolPtr(symbolPtr * sym_out) const
{
    int ret_val = 0;
 
    if (sym_out) {
        *sym_out = x_sym;
        ret_val  = 1;
    }
 
    return ret_val;
}
 
//========================================================= ldrDGraph

init_relational(ldrDGraph,ldrHeader);

// -- view cleanup related: delete ldr, and app whenever view gets deleted
init_relation(view_of_ldrDGraph,MANY,relationMode::D,ldrDGraph_of_view,1,relationMode::D);

//
// PUBLIC:
//

ldrDGraph::ldrDGraph(app * xg)
: ldrHeader()
{
    Initialize(ldrDGraph::ldrDGraph);

    if (xg) {
	if (is_appDGraph(xg)) {
	    ldr_put_app(this, xg);

	    symbolPtr  a_sym((Obj *)0);
	    ldrDGNode *a_one = new ldrDGNode(a_sym);
	    if (a_one) {
		put_root(a_one);
		put_relation(apptree_of_ldrtree,a_one,(xg->get_root()));

		DAGHandlerXREF * xg_handler = (checked_cast(appDGraph,xg))->graph();
		if (xg_handler) {
		    (void) build_name(*xg_handler);
		    (void) build_graph(*xg_handler);
		}
	    }
	}
    }
}

ldrDGraph::~ldrDGraph()
{
    Initialize(ldrDGraph::~ldrDGraph);

    RTL * rtl = (RTL *)get_appHeader();
    if (rtl)
	obj_delete(rtl);
}

void
ldrDGraph::build_selection(const ldrSelection& ldr_inp, OperPoint& app_point)
{
    Initialize(ldrDGraph::build_selection);
 
    ldrNodePtr ldr_node = ldr_inp.ldr_symbol;
    if (ldr_node) {
	if (ldr_node->get_symbolPtr(&app_point.node))
	    app_point.type = AFTER;
    }
}

void
ldrDGraph::insert_obj(objInserter * oi, objInserter * ni)
{
    Initialize(ldrDGraph::insert_obj);

    obj_delete(this);
}

int
ldrDGraph::regenerate_obj(objTree*)
{
    Initialize(ldrDGraph::regenerate_obj);

    obj_delete(this);

    return 0;
}

void
ldrDGraph::remove_obj(objRemover *, objRemover *)
{
    Initialize(ldrDGraph::remove_obj);

    obj_delete(this);
}

void
ldrDGraph::assign_obj(objAssigner * oc, objAssigner *nc)
{
    Initialize(ldrDGraph::assign_obj);

    obj_delete(this);
}

//
// PROTECTED:
//

//
// returns: directed edge between two nodes
//
ldrNode *
ldrDGraph::connect(ldrDGNode * ldr_src, ldrDGNode * ldr_trg)
{
    ldrNode * ret_val = (ldrNode *)0;

    if (ldr_src!=0 && ldr_trg!=0) {
	ldrConnectionNode * xe = new ldrConnectionNode(ldrcSEQUENCE);
	if (ret_val = xe) {
	    xe->add_input(ldr_src);
	    xe->add_output(ldr_trg);
	}
    }

    return ret_val;
}

//
// returns: -1: error, 0: the name is built
//
int
ldrDGraph::build_name(DAGHandlerXREF & xgraph)
{
    int ret_val = -1;

    if (xgraph.graph() != 0) {
	DAGraph * graph = xgraph.graph();
	if (graph) {
	    parray root_syms(0);
	    parray terminals(0);

	    if (graph->vertexport(&root_syms, 1)>0 &&
		graph->vertexport(&terminals, -1)>0) {
		symbolPtr sym_src;
		symbolPtr sym_trg;

		if (!DAGHandlerXREF::extract((DGNode *)(root_syms[0]), sym_src) &&
		    !DAGHandlerXREF::extract((DGNode *)(terminals[0]), sym_trg)) {
		    genString g_name;

		    g_name.printf("Graph {%s => %s}", sym_src.get_name(),
			sym_trg.get_name());
		    // -- set the name
		    set_name(g_name);
		    app * g_rtl = get_appHeader();
		    if (g_rtl) {
			g_rtl->set_name(g_name);
			ret_val = 0;
		    }
		}
	    }
	}
    }

    return ret_val;
}

//
// returns: -1: error, >=0: the number of created vertices
//
int
ldrDGraph::build_vertices(parray & dag_vert, parray & ldr_vert)
{
    Initialize(ldrDGraph::build_vertices);

    int ret_val = -1;

    if (dag_vert.size() > 0) {
	RTLNode *  g_arr = (RTLNode *)0;
	RTL *      g_rtl = (RTL *)get_appHeader();
	if (g_rtl) {
	    g_arr = checked_cast(RTLNode, g_rtl->get_root());
	    if (g_arr) {
		ret_val = 0;

		int x_ldr  = 0;
		symbolPtr  x_sym;

		ldrDGNode * ldr_prev = (ldrDGNode *)0;
		ldrDGNode * ldr_node = (ldrDGNode *)0;
		symbolArr & rtl_data = g_arr->rtl_contents();

		// -- build ldr vertices, and cache them
		int x_dag  = 0;
		int x_size = dag_vert.size();
		for (x_dag=0; x_dag<x_size; x_dag++) {
		    DGNode * dag_node = (DGNode *)(dag_vert[x_dag]);
		    if (!DAGHandlerXREF::extract(dag_node, x_sym)) {
			dag_node->alg_marked = -1;
			if (ldr_node = new ldrDGNode(x_sym)) {
			    if (!ldr_vert.insert((void *)ldr_node)) {
				dag_node->alg_marked = x_ldr++;
				rtl_data.insert_last(x_sym); ret_val++;
			    }
			}
		    }
		}
		// -- link all elements, and set relation to this header
		if (ret_val > 0) {
		    ldr_prev = (ldrDGNode *)ldr_vert[0];
		    ldr_node = checked_cast(ldrDGNode,get_root());
		    if (ldr_prev) {
			ldr_node->put_first(ldr_prev);
			for (x_ldr=1; x_ldr<ret_val; x_ldr++) {
			    ldr_node = (ldrDGNode *)ldr_vert[x_ldr];
			    ldr_prev->put_after(ldr_node);
			    ldr_prev = ldr_node;
			}
		    }
		}
	    }
	}
    }

    return ret_val;
}

//
// returns: -1: error, >=0: the number of edges
//
int
ldrDGraph::build_edges(parray & dag_vert, parray & ldr_vert)
{
    Initialize(ldrDGraph::build_edges);

    int ret_val = -1;

    if (ldr_vert.size()>0 && dag_vert.size()>0) {
	ldrNode *   ldr_node = (ldrNode *)0;
	ldrDGNode * ldr_root = (ldrDGNode *)get_root();
	if (ldr_root)
	    ldr_node = (ldrNode *)(ldr_root->get_last());
	// -- build edges
	if (ldr_node) {
	    ret_val = 0;

	    for (int x_dag=dag_vert.size()-1; x_dag>=0; x_dag--) {
		DGNode * dag_node = (DGNode *)(dag_vert[x_dag]);
		if (dag_node) {
		    ldrDGNode * ldr_src = (ldrDGNode *)(ldr_vert[dag_node->alg_marked]);
		    if (ldr_src) {
			ldrNode * ldr_edge = (ldrNode *)0;
			for (int x_trg=dag_node->outflow()-1; x_trg>=0; x_trg--) {
			    DGNode * trg_node = dag_node->outgoing(x_trg);
			    if (trg_node) {
				ldr_edge = connect(ldr_src, (ldrDGNode *)(ldr_vert[trg_node->alg_marked]));
				if (ldr_edge) {
				    ldr_node->put_after(ldr_edge);
				    ldr_node = ldr_edge;
				    ret_val++;
				}
			    }
			}
		    }
		}
	    }
	}
    }

    return ret_val;
}

//
// returns: -1: error, >=0: number of vertices in the graph
//
int
ldrDGraph::build_graph(DAGHandlerXREF & xgraph)
{
    int ret_val = -1;

    if (xgraph.graph() != 0) {
	DAGraph * graph = xgraph.graph();
	if (graph->vertcount() > 0) {
	    parray dag_vert(graph->vertcount()+1);
	    parray ldr_vert(graph->vertcount()+1); 

	    (void) graph->vertexport(&dag_vert);
	    if (build_vertices(dag_vert, ldr_vert) > 0)
		ret_val = build_edges(dag_vert, ldr_vert);
	}
    }

    return ret_val;
}
