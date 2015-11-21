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
// DAGHandler.h
// ------------
//

#ifndef ISO_CPP_HEADERS
#include <stddef.h>
#else /* ISO_CPP_HEADERS */
#include <cstddef>
namespace std {};
using namespace std;
#endif /* ISO_CPP_HEADERS */
#include <parray.h>

#include <DGNodeData.h>
#include <DGDataAccess.h>
#include <DGPathFilter.h>
#include <DGPathHandler.h>
#include <DAGraph.h>
#include <DAGHandler.h>

//
// PUBLIC-STATIC:
//

//
// returns: -1: error, >=0: the number of deleted symbols
//
int
DAGHandler::rmdata(parray * data_arr)
{
    int ret_val = -1;

    if (data_arr) {
	ret_val = 0;

	DGNodeData * key = (DGNodeData *)0;
	for (int x_iter=data_arr->size()-1; x_iter>=0; x_iter--) {
	    if (key = (DGNodeData *)((*data_arr)[x_iter])) {
		delete key;
		ret_val++;
	    }
	}
    }

    return ret_val;
}

//
// PUBLIC:
//
DAGHandler::DAGHandler(DGDataAccess * sym_dbase)
{
    data_access  = sym_dbase;
    path_filter  = (DGPathFilter *)0;
    path_handler = (DGPathHandler *)0;
    graph_proc   = (DAGraph *)0;
}

DAGHandler::~DAGHandler()
{
    if (graph_proc)
	delete graph_proc;
    if (path_handler)
	delete path_handler;
    else if (path_filter)
	delete path_filter;
    if (data_access)
	delete data_access;
}

//
// returns: -1: error, 0: the filter is set
//
int
DAGHandler::filterON(DGPathFilter * sym_flt)
{
    int ret_val = -1;

    if (sym_flt) {
	if (path_handler) {
	    delete path_handler;
	    path_handler = (DGPathHandler *)0;
	} else if (path_filter)
	    delete path_filter;
	path_filter = sym_flt;
	ret_val = 0;
    }

    return ret_val;
}

//
// returns: -1: error, 0: 
//
int
DAGHandler::pathfinderON(DGPathHandler * handle_in)
{
    int ret_val = -1;

    if (handle_in) {
	if (path_handler)
	    delete path_handler;
	else if (path_filter)
	    delete path_filter;
	path_handler = handle_in;
	path_filter  = handle_in->filter();
	ret_val = 0;
    }

    return ret_val;
}

//
// returns: -1: error, >=0: ok
//
int
DAGHandler::build(parray * source, parray * target)
{
    int ret_val = -1;

    if (source!=0 && target!=0 && data_access!=0) {
	// -- no incremental builds on a graph
	if (graph_proc) {
	    delete graph_proc;
	    graph_proc = (DAGraph *)0;
	}
	if (!graph_proc)
	    graph_proc = new DAGraph(data_access);
	// -- build a graph
	if (graph_proc)
	    ret_val = graph_proc->build(source, target);
    }

    return ret_val;
}

//
// returns: -1: error, >=0: ok
//
int
DAGHandler::traverse(DGNode * root_vertex)
{
    int ret_val = -1;

    if (graph_proc)
	ret_val = graph_proc->pathfinder(path_handler, root_vertex);

    return ret_val;
}

//
// returns: 0: the graph does not exist, !=0: the graph exists (number of nodes)
//
int
DAGHandler::exists(parray * source, parray * target)
{
    int ret_val = 0;

    if (graph_proc) {
	int count = 0;
	if (source!=0 && target!=0) {
	    count = graph_proc->vertcount();
	    // -- search for roots, terminals
	    int x_iter = 0;
	    for (x_iter=source->size()-1; count>0 && x_iter>=0; x_iter--) {
		if (!(graph_proc->lookup((DGNodeData*)((*source)[x_iter]), 1)))
		    count = 0;
	    }
	    for (x_iter=target->size()-1; count>0 && x_iter>=0; x_iter--) {
		if (!(graph_proc->lookup((DGNodeData*)((*target)[x_iter]), -1)))
		    count = 0;
	    }
	} else if (!source && !target)
	    count = graph_proc->vertcount();
	if (count > 0)
	    ret_val = count;
    }

    return ret_val;
}

int
DAGHandler::hierarchy()
{
    return BASE;
}
