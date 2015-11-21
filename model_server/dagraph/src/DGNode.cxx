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
// DGNode.C
// --------
//

#ifndef ISO_CPP_HEADERS
#include <stddef.h>
#else /* ISO_CPP_HEADERS */
#include <cstddef>
namespace std {};
using namespace std;
#endif /* ISO_CPP_HEADERS */

#include <parray.h>

#include <DGNode.h>
#include <DGNodeAVL.h>
#include <DGNodeData.h>

//
// PUBLIC:
//

DGNode::DGNode(DGNodeData & node_data)
: data(node_data)
{
    edges   = (parray *)0;
    in_flow = 0;
}

DGNode::~DGNode()
{
    if (edges)
	delete edges;
    delete &data;
}

//
// returns: -1: error, 0: the edge is set
//
int
DGNode::connect(DGNode * dest)
{
    int ret_val = -1;

    if (!edges)
	edges = new parray(8);
    if (dest!=0 && edges!=0) {
#ifdef DEBUG
	if (edges->find((void *)dest) < 0) {
	    if (!(ret_val=edges->insert((void *)dest)))
		(dest->in_flow)++;
	} else
	    ret_val = 0;
#else
	if (!(ret_val=edges->insert((void *)dest)))
	    (dest->in_flow)++;
#endif
    }

    return ret_val;
}

//
// returns: -1: error, >=0: the inflow of the disconnected node
//
int
DGNode::disconnect(DGNode * dest)
{
    int ret_val = -1;

    if (dest!=0 && edges!=0) {
	int ndx = edges->find((void *)dest);
	if (ndx >= 0) {
	    if (!(edges->remove(ndx)))
		ret_val = --(dest->in_flow);
	}
    }

    return ret_val;
}

//
// returns: the specified "outgoing" node
//
DGNode *
DGNode::outgoing(int ndx)
{
    DGNode * ret_val = (DGNode *)0;

    if (edges)
	ret_val = (DGNode *)((*edges)[ndx]);

    return ret_val;
}

//
// returns: >=0: the degree of the node with respect to the outgoing edges
//
int
DGNode::outflow()
{
    int ret_val = 0;

    if (edges)
	ret_val = edges->size();

    return ret_val;
}

// ---- DGNodeAVL ----

//
// PUBLIC:
//

DGNodeAVL::DGNodeAVL(avl_treeptr x_tree, DGNode& x_node)
: graph_node(x_node), avl_node(x_tree, x_node.data)
{
}

//
// returns: 1: this < other, 0: this == other, -1: this > other
//
int
DGNodeAVL::compare(const avl_key & other) const
{
    int ret_val = 0;

    ret_val = 0 - dgnode().data.compare((DGNodeData &)other);

    return ret_val;
}
