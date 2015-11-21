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
// DAGraph.C
// ---------
//

#ifndef ISO_CPP_HEADERS
#include <stddef.h>
#else /* ISO_CPP_HEADERS */
#include <cstddef>
namespace std {};
using namespace std;
#endif /* ISO_CPP_HEADERS */

#include <pqueue.h>
#include <pstack.h>
#include <parray.h>
#include <listItem.h>

#include <DGNode.h>
#include <DGNodeAVL.h>
#include <DGNodeData.h>
#include <DGNodeDataAVL.h>

#include <DGDataAccess.h>
#include <DGPathHandler.h>

#include <DAGraph.h>

//
// PRIVATE:
//

//
// returns: (DGNode*)0: error, !=0: the newly created node
// NOTE: the symbol "new_sym" becomes responsibility of "DGNode"
//
DGNode *
DAGraph::newNode(DGNodeData * new_sym)
{
    DGNode * ret_val = (DGNode *)0;

    if (ordered!=0 && new_sym!=0) {
	if (!(ret_val=lookup(new_sym))) {
	    DGNode * x_node = new DGNode(*new_sym);
	    if (x_node) {
		DGNodeAVL * x_avl = new DGNodeAVL(ordered, *x_node);
		if (x_avl)
		    ret_val = x_node;
		else
		    delete x_node;
	    }
	}
    }

    return ret_val;
}

//
// returns: -1: error, 0: the node is deleted
//
// NOTE: this routine does not consider any context that a node may be in; it
//       is responsible for physical removal of the specified node
//
int
DAGraph::deleteNode(DGNode * node)
{
    int ret_val = -1;

    if (node) {
	if (ordered)
	    ordered->remove(node->data);
	delete node;

	ret_val = 0;
    }

    return ret_val;
}

//
// returns: -1: error, >=0: the number of deleted node(s)
//
int
DAGraph::deleteNode(pqueue * nodes)
{
    int ret_val = -1;

    if (nodes) {
	ret_val = 0;

	for (void * ref_ptr = (void *)0; !(nodes->remove(&ref_ptr)); ) {
	    if (ref_ptr) {
		if (!deleteNode((DGNode *)ref_ptr))
		    ret_val++;
	    }
	}
    }

    return ret_val;
}

//
// returns: -1: error, >=0: the number of removed references
//
int
DAGraph::disconnect(DGNode * node)
{
    int ret_val = -1;

    if (node) {
	ret_val = 0;

	// -- get references
	if (node->in_flow > 0) {
	    pqueue refs;
	    if (references(node, &refs) > 0) {
		for (void * ref_ptr = (void *)0; !refs.remove(&ref_ptr); ) {
		    if (ref_ptr) {
			if (((DGNode *)ref_ptr)->disconnect(node) >= 0)
			    ret_val++;
		    }
		}
	    }
	}
    }

    return ret_val;
}

//
// returns: -1: node is not deleted, 0: the node is deleted
//
int
DAGraph::remove0ref(DGNodeData * x_sym, int ref_val)
{
    int ret_val = -1;

    if (x_sym!=0 && ref_val==0) {
	if (!search(x_sym)) {
	    ret_val = 0;
	    delete x_sym;
	}
    }

    return ret_val;
}

//
// PROTECTED:
//

//
// returns: -1: error, >=0: the number of "reset"-ed nodes
//
int
DAGraph::algreset(int value)
{
    int ret_val = -1;

    if (ordered) {
	ret_val = 0;

	avl_iterator x_iter(ordered);
	for (avl_node * x_elem=x_iter.next(); x_elem!=0; x_elem=x_iter.next()) {
	    DGNode & dg_node = ((DGNodeAVL *)x_elem)->dgnode();

	    dg_node.alg_value  = value;
	    dg_node.alg_marked = value;
	    ret_val++;
	}
    }

    return ret_val;
}

//
// returns: -1: error, >=0: the number of reduced symbols
//
int
DAGraph::reduce(DGNode * rnode)
{
    int ret_val = -1;

    if (roots) {
	ret_val = 0;

	int rnode_ndx = -1;
	if (rnode) {
	    if (rnode->in_flow > 0) {
		rnode_ndx = roots->find((void *)rnode);
		if (rnode_ndx >= 0) {
		    if (roots->remove(rnode_ndx) >= 0)
			ret_val = 1;
		}
	    }
	} else {
	    for (int root_iter=roots->size()-1; root_iter>=0; root_iter--) {
		DGNode * dg_node = (DGNode *)((*roots)[root_iter]);
		if (dg_node) {
		    if (dg_node->in_flow > 0) {
			if (roots->remove(root_iter) >= 0)
			    ret_val++;
		    }
		}
	    }
	}
    }

    return ret_val;
}

//
// returns: <0: error, >=0: the graph is built (number of edges in the graph)
//
int
DAGraph::build(pqueue * x_edge, avl_treeptr target)
{
    int ret_val = -1;

    if (x_edge!=0 && target!=0 && data_access!=0) {
	pstack     traversal;                    // call stack
	avl_tree   x_path_set;                   // path in "progress"
	avl_tree   x_expanded;                   // all expanded nodes
	listItem * last_path = new listItem(0);  // the last node of a succ. build
	listItem *    x_path = last_path;        // path in "progress"

	ret_val = 0;
	// -- start computing
	int status = 0;
	for (void * elem = (void *)0; status>=0 && !(x_edge->remove(&elem)); elem = (void *)0) {
	    DGNodeData * symp = (DGNodeData *)elem;
	    // -- process one vertex
	    if (!x_path_set.find(*symp)) {
		DGNode * x_link = (DGNode *)0;
		if (x_link = lookup(symp)) {
		    if ((status=insert(last_path, x_link)) >= 0)
			ret_val += status;
		    last_path = x_path;
		} else if (target->find(*symp)) {
		    if ((status=insert(last_path, symp)) >= 0)
			ret_val += status;
		    last_path = x_path;
		} else if (!x_expanded.find(*symp)) {
		    if ((status=traversal.push((void *)x_edge)) >= 0) {
		        x_path = new listItem(elem, x_path);
		        (void) (new DGNodeDataAVL(&x_path_set, *symp));
		        (void) (new DGNodeDataAVL(&x_expanded, *(symp->clone())));
		        status = data_access->dependents(symp, (x_edge=new pqueue()));
		        symp   = (DGNodeData *)0;
		    }
		}
	    }
	    // -- delete the processed data: skip input symbols
	    if (symp)
		(void) remove0ref(symp, traversal.peekTop(0));
	    // -- adjust the context
	    while (status>=0 && x_edge->peekTop(0)<0 && traversal.peekTop(0)>=0) {
		if (!(status=traversal.pop(&elem))) {
		    symp   = (DGNodeData *)(x_path->data());
		    delete x_edge;
		    (void) x_path_set.remove(*symp); (void) remove0ref(symp, traversal.peekTop(0));
		    if (x_path == last_path)
			last_path = x_path->next();
		    x_path = x_path->next(); delete x_path->previous();
		    x_edge = (pqueue *)elem;
		}
	    }
	}
	// -- cleanup
	delete last_path;
	(void) reduce(0);
	if (status < 0)
	    ret_val = 0 - ret_val;
    }

    return ret_val;
}

//
// returns: <0: error, >=0: the nodes are inserted; the number of inserted edges
//
int
DAGraph::insert(listItem * path_in, DGNode * link_elem)
{
    int ret_val = -1;

    if (path_in!=0 && link_elem!=0) {
	ret_val = 0;

	DGNode * xa_elem = lookup((DGNodeData *)(path_in->data()));
	for (path_in=path_in->previous(); path_in!=0 && ret_val>=0; path_in=path_in->previous()) {
	    // -- insert one node ...
	    int      xb_stat = -1;
	    DGNode * xb_elem = newNode((DGNodeData *)(path_in->data()));
	    if (xb_elem) {
		if (!xa_elem)
		    xb_stat = roots->insert((void *)xb_elem);
		else if ((xb_stat=xa_elem->connect(xb_elem)) >= 0)
		    ret_val++;
		xa_elem = xb_elem;
	    }
	    // -- insertion was OK (xb_stat>=0) ?
	    if (xb_stat < 0)
		ret_val = -1 - ret_val;
	}
	// -- link with the port vertex
	if (xa_elem!=0 && ret_val>=0) {
	    if (xa_elem->connect(link_elem) >= 0)
		ret_val++;
	}
    }

    return ret_val;
}

//
// returns: <0: error, >=0: the node is created; the number of inserted edges
//
int
DAGraph::insert(listItem * path_in, DGNodeData * new_sym)
{
    int ret_val = -1;

    if (path_in!=0 && new_sym!=0) {
	DGNode * x_node = newNode(new_sym);
	if (x_node) {
	    if ((ret_val=insert(path_in, x_node)) < 0)
		(void) deleteNode(x_node);
	}
    }

    return ret_val;
}

//
// returns: -1: error, >=0: the node, and the nodes with removed references are removed
//
int
DAGraph::remove(DGNode * node)
{
    int ret_val = -1;

    if (node) {
	pqueue to_be_deleted;

	// -- remove references
	(void) to_be_deleted.insert((void *)node);
	(void) disconnect(node);
	// -- get dependent nodes that need to be deleted
	if (node->outflow() > 0) {
	    if (algreset(0) >= 0) {
		int      sval = 0;
		DGNode * iter = (DGNode *)0;

		// -- stage 1: compute the flow
		sval++; BFSEdges(node,iter,sval) {
		    if (_bfs_other)
			(_bfs_other->alg_value)++;
		} BFSEdgesEnd(sval);
		// -- stage 2: rectify stage 1
		sval++; BFSEdges(node,iter,sval) {
		    if (_bfs_other!=0 && (iter->in_flow-iter->alg_value)!=0)
			(_bfs_other->alg_value)--;
		} BFSEdgesEnd(sval);
		// -- stage 3: collect nodes for deletion
		sval++; BFSEdges(node,iter,sval) {
		    if (iter != node) {
			if ((iter->in_flow-iter->alg_value) == 0)
			    (void) to_be_deleted.insert((void *)iter);
			node = iter;
		    }
		} BFSEdgesEnd(sval);
	    }
	}
	// -- delete the collected nodes
	ret_val = deleteNode(&to_be_deleted);
    }

    return ret_val;
}

//
// PUBLIC:
//

DAGraph::DAGraph(DGDataAccess * sym_info)
{
    roots       = new parray(0);
    ordered     = new avl_tree();
    data_access = sym_info;
}

DAGraph::~DAGraph()
{
    if (roots) {
	for (int i=roots->size(); i>=0; i--) {
	    (void) remove((DGNode *)((*roots)[i]));
	}
	delete roots;
    }
    if (ordered)
	delete ordered;
}

//
// returns: <0: error,
//         >=0: the graph is built ( number of edges in the graph;
//              the upper bound is |V|*(|V|-1)/2 )
//
// NOTE: "dest_vrt" symbols are checked for duplicates. Any of the source
//       symbols found inside the destination array are filtered out.
//       Also, any duplicate symbols in the source array will not be
//       processed.
//
//       The symbols in "source", and "dest" do not change ownership; the
//       caller has to destroy them. There is an exception on the symbols
//       that might have been used to build the graph; these symbols will
//       be removed from the input arrays.
//
int
DAGraph::build(parray * src_vrt, parray * dest_vrt)
{
    int ret_val = -1;

    if (src_vrt!=0 && dest_vrt!=0) {
	if (data_access!=0 && roots!=0) {
	    // -- cannot build it twice
	    if (roots->size() <= 0) {
		pqueue   x_edge;
		avl_tree target;
		avl_tree source;
		int      count = 0;

		// -- cache the target symbols
		for (count=dest_vrt->size()-1; count>=0; count--) {
		    DGNodeData * x_dest = (DGNodeData *)((*dest_vrt)[count]);
		    if (x_dest) {
			if (!target.find(*x_dest))
			    (void) new DGNodeDataAVL(&target, *x_dest);
		    }
		}
		// -- copy the source symbols
		for (count=src_vrt->size()-1; count>=0; count--) {
		    DGNodeData * x_sym = (DGNodeData *)((*src_vrt)[count]);
		    if (x_sym) {
			if (!(target.find(*x_sym)!=0 || source.find(*x_sym)!=0)) {
			    (void) new DGNodeDataAVL(&source, *x_sym);
			    (void) x_edge.push((void *)x_sym);
			}
		    }
		}
		// -- start processing:
		ret_val = build(&x_edge, &target);
		// -- remove symbols that might have been used in the graph (the same objects)
		for (count=src_vrt->size()-1; count>=0; count--) {
		    if (search((DGNodeData *)((*src_vrt)[count])))
			(void) src_vrt->remove(count);
		}
	    }
	}
    }

    return ret_val;
}

//
// returns: (DGNode *)0: could not find the symbol, !=0: the specified graph node
//
DGNode *
DAGraph::lookup(DGNodeData * sym, int flow_val)
{
    DGNode * ret_val = (DGNode *)0;

    if (sym!=0 && ordered!=0) {
	DGNodeAVL * x_avl = (DGNodeAVL *)(ordered->find(*sym));
	if (x_avl)
	    ret_val = &(x_avl->dgnode());
	if (flow_val!=0 && ret_val!=0) {
	    if (flow_val < 0) {
		// -- terminal nodes
		if (ret_val->outflow() > 0)
		    ret_val = (DGNode *)0;
	    } else if (flow_val > 0) {
		// -- root nodes
		if (ret_val->in_flow > 0)
		    ret_val = (DGNode *)0;
	    }
	}
    }

    return ret_val;
}

//
// returns: (DGNode *)0: could not find the specified symbol object, !=0: the corresponding graph node
//
DGNode *
DAGraph::search(DGNodeData * sym_obj)
{
    DGNode * ret_val = (DGNode *)0;

    if (ret_val = lookup(sym_obj)) {
	if (sym_obj != &(ret_val->data))
	    ret_val = (DGNode *)0;
    }

    return ret_val;
}

//
// returns: -1: error, >=0: the number of references
//
int
DAGraph::references(DGNode * node, pqueue * refs)
{
    int ret_val = -1;

    if (node!=0 && ordered!=0) {
	ret_val = 0;

	avl_iterator x_iter(ordered);
	for (avl_node * x_elem=x_iter.next(); x_elem!=0; x_elem=x_iter.next()) {
	    DGNode & dg_node = ((DGNodeAVL *)x_elem)->dgnode();
	    if (dg_node.outflow() > 0) {
		if (dg_node.edges->find((void *)node) >= 0) {
		    if (refs)
			(void) refs->insert((void *)(&dg_node));
		    ret_val++;
		}
	    }
	}
    }

    return ret_val;
}

//
// returns: -1: error, >=0: the symbol/node(s) is/are removed from the graph
//
int
DAGraph::remove(DGNodeData * sym)
{
    int ret_val = -1;

    if (sym) {
	DGNode * x_node = lookup(sym);
	if (x_node)
	    ret_val = remove(x_node);
    }

    return ret_val;
}

//
// returns: -1: error, >=0: the number of succ. processed path(s)
//
int
DAGraph::pathfinder(DGPathHandler * handler, DGNode * snode)
{
    int ret_val = -1;

    if (handler) {
	ret_val = 0;

	// -- get the roots
	pqueue nodes;
	if (snode)
	    (void) nodes.insert((void *)snode);
	else if (roots) {
	    for (int i=roots->size()-1; i>=0; i--)
		(void) nodes.push((*roots)[i]);
	}
	// -- process all roots
	for (void * snode_ptr=(void *)0; !nodes.remove(&snode_ptr); ) {
	    if (snode = (DGNode *)snode_ptr) {
		listItem * path = (listItem *)0;
		if (!(handler->start(snode))) {
		    int proc_status = 0;
		    int path_icount = 0;
		    DFSPaths(snode,path) {
			if ((proc_status=handler->process(path, _dfs_distance)) >= 0)
			    path_icount++;
			else
			    _dfs_continue = 0; // -- stop processing
		    } DFSPathsEnd;
		    (void) handler->finish(path_icount, proc_status);
		    ret_val += path_icount;
		}
	    }
	}
    }

    return ret_val;
}

//
// returns: -1: error, >=0: the number of exported vertices
//
//   input: flow_val: 0: all vertices, >0: roots, <0: terminal nodes
//
int
DAGraph::vertexport(parray * vrt_out, int flow_val)
{
    int ret_val = -1;

    if (vrt_out!=0 && ordered!=0) {
        ret_val = 0;

	if (flow_val > 0) {
	    if (!(vrt_out->insert(roots)))
		ret_val = roots->size();
	} else {
	    avl_iterator x_iter(ordered);
	    for (avl_node * x_elem=x_iter.next(); x_elem!=0; x_elem=x_iter.next()) {
		int      dg_skip = flow_val;
		DGNode & dg_node = ((DGNodeAVL *)x_elem)->dgnode();
		if (flow_val < 0)
		    dg_skip = dg_node.outflow() > 0;
		if (!dg_skip) {
		    if (!(vrt_out->insert((void *)(&dg_node))))
			ret_val++;
		}
	    }
        }
    }

    return ret_val;
}

//
// returns: -1: error, >=0: the number of vertices in the graph
//
int
DAGraph::vertcount(int flow_val)
{
    int ret_val = -1;

    if (ordered) {
	if (flow_val != 0) {
	    parray terminals(0);
	    ret_val = vertexport(&terminals, flow_val);
	} else
	    ret_val = (int)(ordered->count());
    }

    return ret_val;
}
