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
// DAGraph.h
// ---------
//

#ifndef _DAGraph_h
#define _DAGraph_h

class parray;
class pqueue;
class listItem;

class DGNode;
class DGNodeData;
class DGDataAccess;
class DGPathHandler;

class avl_tree;

class DAGraph {

    parray *       roots;                             // the list of disjoint (root) nodes
    avl_tree *     ordered;                           // the ordered list of nodes
    DGDataAccess * data_access;                       // access to vertex dependencies

    DGNode *       newNode(DGNodeData *);             // create new node in the graph
    int            deleteNode(DGNode *);              // delete the selected node
    int            deleteNode(pqueue *);              // delete the list of selected nodes
    int            disconnect(DGNode *);              // disconnect a node from the graph
    int            remove0ref(DGNodeData *, int = 0); // remove the data node if the data already exists

  protected:

    int            algreset(int);                     // reset algorithm reserved fields on DGNode(s)
    int            reduce(DGNode *);                  // reduce the roots
    int            build(pqueue *, avl_tree *);       // build the graph
    int            insert(listItem *, DGNode *);      // insert to existing; follow "prev"
    int            insert(listItem *, DGNodeData *);  // insert with new element
    int            remove(DGNode *);                  // remove the selected node, and its dependents from the graph

  public:

    DAGraph(DGDataAccess *);
    virtual ~DAGraph();

    int            build(parray *, parray *);         // build the graph
    DGNode *       lookup(DGNodeData *, int = 0);     // lookup symbol in the graph
    DGNode *       search(DGNodeData *);              // search for the specified symbol object
    int            references(DGNode *, pqueue *);    // the list of nodes which reference the specified node
    int            remove(DGNodeData *);              // remove vertex from the graph
    int            pathfinder(DGPathHandler *, DGNode * = 0); // traverse path(s)
    int            vertexport(parray *, int = 0);     // copy addresses of selected vertices (default == all)
    int            vertcount(int = 0);                // number of vertices in the graph

};

//
// -- performs breadth-first-search with coloring: opening clause
//    NOTE: the name "_bfs_other" should not be changed without adjusting references
//          to this reserved name (e.g., A->n1, A->n2:_bfs_other takes values {n1,n2})
//          _bfs_other == (DGNode *)0 for the nodes that do not have any outgoing
//          edges; terminal nodes
//

#define BFSEdges(root,node_iter,mark_value) \
        { \
	    pqueue _bfs_nodes; \
	    (void) _bfs_nodes.insert((void *)root); \
	    for (void * _bfs_iter=(void *)0; !_bfs_nodes.remove(&_bfs_iter); ) { \
		if (_bfs_iter) { \
		    node_iter = (DGNode *)_bfs_iter; \
		    if (node_iter->alg_marked != mark_value) { \
			node_iter->alg_marked = mark_value; \
			int _bfs_count = 0; \
			int _bfs_size  = node_iter->outflow(); \
			do { \
			    DGNode * _bfs_other = node_iter->outgoing(_bfs_count);

// -- BFS: closing clause

#define BFSEdgesEnd(mark_value) \
			    if (_bfs_other) { \
				if (_bfs_other->alg_marked != mark_value) \
				    (void *)_bfs_nodes.insert((void *)_bfs_other); \
			    } \
			    _bfs_count++; \
			} while (_bfs_count < _bfs_size); \
		    } \
		} \
	    } \
	}

//
// -- performs depth-first-search with modified coloring: the node could be revisited
//    NOTE: the name "_dfs_distance" should not be changed without adjusting references
//          to this reserved name (e.g., A->B->C->D: _dfs_distance=3; |E|)
//          Also, the name "_dfs_continue" controls the processing of DFS; if the variable
//          is set to non-positive value (<=0), the processing is interrupted, and the
//          appropriate cleanup is performed. This should be used to control the extent of
//          path processing; the upper bound for the number of paths is 2**(|V|-2)
//

#define DFSPaths(root,path) \
        { \
	    listItem   _dfs_begin(0); \
	    DGNode *   _dfs_iter = root; \
	    DGNode *   _dfs_next = (DGNode *)0; \
	    listItem * _dfs_context = new listItem((void *)root, &_dfs_begin, 'A'); \
            int        _dfs_distance = 0; \
	    int        _dfs_continue = 1; \
	    for (_dfs_iter->alg_value=0; _dfs_iter!=0 && _dfs_continue>0; _dfs_iter=(DGNode *)(_dfs_context->data())) { \
		if (_dfs_iter->alg_value < _dfs_iter->outflow()) { \
		    _dfs_next = _dfs_iter->outgoing(_dfs_iter->alg_value); \
		    _dfs_next->alg_value = 0; \
		    _dfs_context = new listItem((void *)_dfs_next, _dfs_context, 'A'); \
		    (_dfs_iter->alg_value)++; \
                    _dfs_distance++; \
		} else { \
		    if (_dfs_iter->alg_value == 0) { \
			path = _dfs_begin.next();

// -- DFS closing clause

#define DFSPathsEnd \
		    } \
		    while (_dfs_iter != 0) { \
                        _dfs_distance--; \
			_dfs_iter->alg_value = 0; \
			_dfs_context = _dfs_context->previous(); \
			delete _dfs_context->next(); \
			if (_dfs_iter = (DGNode *)(_dfs_context->data())) { \
			    if (_dfs_iter->alg_value < _dfs_iter->outflow()) \
				_dfs_iter = (DGNode *)0; \
			} \
		    } \
		} \
	    } \
	    if (_dfs_continue <= 0) { \
		while (_dfs_context->data() != 0) { \
		    _dfs_context = _dfs_context->previous(); \
		    delete _dfs_context->next(); \
		} \
	    } \
	}

#endif
