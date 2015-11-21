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
// DAGHandlerXREF.C
// ----------------
//

#ifndef ISO_CPP_HEADERS
#include <stdio.h>
#else /* ISO_CPP_HEADERS */
#include <cstdio>
namespace std {};
using namespace std;
#endif /* ISO_CPP_HEADERS */
#include <pdutil.h>
#include <parray.h>

#include <linkTypes.h>
#include <ddSelector.h>
#include <xrefSymbol.h>
#include <symbolPtr.h>
#include <symbolArr.h>

#include <DGNode.h>
#include <DGNodeData.h>
#include <DGNodeDataXREF.h>
#include <DGDataAccess.h>
#include <DGDataAccessXREF.h>
#include <DGPathHandler.h>
#include <DGPathXREF.h>
#include <DGPrintXREF.h>
#include <DAGHandler.h>
#include <DAGHandlerXREF.h>

//
// PUBLIC-STATIC:
//

//
// returns: -1: error, 0: the symbol is extracted
//
int
DAGHandlerXREF::extract(DGNode * x_node, symbolPtr & x_sym)
{
    int ret_val = -1;

    if (x_node) {
        if (x_node->data.hierarchy() == DGNodeData::XREFsymbol) {
            x_sym = ((DGNodeDataXREF &)(x_node->data)).symbol();
            if (x_sym.is_xrefSymbol())
                ret_val = 0;
        }
    }

    return ret_val;
}

//
// PUBLIC:
//

DAGHandlerXREF::DAGHandlerXREF(linkTypes & sym_flt, ddSelector & knd_flt)
: DAGHandler(new DGDataAccessXREF(sym_flt, knd_flt)), xsym_types(knd_flt)
{
}

DAGHandlerXREF::~DAGHandlerXREF()
{
}

//
// returns: -1: error, >=0: ok
//
int
DAGHandlerXREF::xsym_print(char * dev_name)
{
    int ret_val = -1;

    if (graph() != 0) {
	// -- get the output device
	FILE * out_dev = (FILE *)0;
	if (!dev_name)
	    out_dev = stdout;
	else if (dev_name = pdexpandenv(dev_name)) {
	    out_dev = fopen(dev_name, "a");
	    FREE_MEMORY(dev_name);
	}
	// -- print the path(s) out
	if (out_dev) {
	    if (!pathfinderON(new DGPrintXREF(0, out_dev)))
		ret_val = traverse(0);
	    if (out_dev != stdout)
		(void) fclose(out_dev);
	}
    }

    return ret_val;
}

//
// returns: -1: error, >=0: ok
//
int
DAGHandlerXREF::xsym_build(symbolArr & source, symbolArr & target)
{
    int ret_val = -1;

    if (source.size()>0 && target.size()>0) {
	parray src_arr(0);
	parray trg_arr(0);

	// -- build the graph
        if (builddata(source, src_arr)>0 && builddata(target, trg_arr)>0)
	    ret_val = build(&src_arr, &trg_arr);
	// -- cleanup the input
	(void) DAGHandler::rmdata(&src_arr);
	(void) DAGHandler::rmdata(&trg_arr);
    }

    return ret_val;
}

//
// returns: (DAGHandler::exists())
//
int
DAGHandlerXREF::xsym_exists(symbolArr & source, symbolArr & target)
{
    int ret_val = 0;

    if (source.size()<=0 && target.size()<=0)
	ret_val = exists(0, 0);
    else if (source.size()>0 && target.size()>0) {
	parray src_arr(0);
	parray trg_arr(0);

	if (builddata(source, src_arr)>0 && builddata(target, trg_arr)>0)
	    ret_val = exists(&src_arr, &trg_arr);
	(void) DAGHandler::rmdata(&src_arr);
	(void) DAGHandler::rmdata(&trg_arr);
    }

    return ret_val;
}

//
// returns: -1: error, >=0: the number of built symbols
//
int
DAGHandlerXREF::builddata(symbolArr & sym_in, parray & data_out)
{
    int ret_val = 0;

    if (sym_in.size() > 0) {
	// -- generate data objects
	symbolPtr x_sym;
	ForEachS(x_sym,sym_in) {
	    if (x_sym.is_xrefSymbol()) {
		if (xsym_types.selects(x_sym.get_kind())) {
		    if (!data_out.insert((void *)(new DGNodeDataXREF(x_sym))))
			ret_val++;
		}
	    }
	}
    }

    return ret_val;
}

int
DAGHandlerXREF::hierarchy()
{
    return XREFHandler;
}
