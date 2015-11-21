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
// DGPathXREF.h
// ------------
//

#include <listItem.h>

#include <symbolPtr.h>
#include <symbolArr.h>

#include <DGNode.h>
#include <DGNodeData.h>
#include <DGNodeDataXREF.h>
#include <DGPathHandler.h>
#include <DGPathXREF.h>

//
// PUBLIC:
//

DGPathXREF::DGPathXREF(DGPathFilter * sym_flt)
: DGPathHandler(sym_flt)
{
}

DGPathXREF::~DGPathXREF()
{
}

int
DGPathXREF::hierarchy()
{
    return XREFBase;
}

//
// PROTECTED:
//

//
// returns: -1: error, 0: the XREF symbol is extracted
//
int
DGPathXREF::symbol(DGNode * x_node, symbolPtr & x_sym)
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
// returns: -1: error, >=0: the number of copied nodes
//
int
DGPathXREF::copy_path(listItem * path_in, symbolArr & path_arr)
{
    int ret_val = -1;

    if (path_in) {
	symbolPtr x_sym;
	for (ret_val=0; path_in!=0; path_in=path_in->next()) {
	    if (!symbol((DGNode *)(path_in->data()), x_sym)) {
		path_arr.insert_last(x_sym);
		ret_val++;
	    }
	}
    }

    return ret_val;
}
