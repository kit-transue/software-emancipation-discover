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
// DGDataAccessXREF.C
// ------------------
//

#include <pqueue.h>

#include <linkTypes.h>
#include <ddSelector.h>
#include <xrefSymbol.h>
#include <symbolPtr.h>
#include <symbolArr.h>

#include <DGNodeData.h>
#include <DGNodeDataXREF.h>
#include <DGDataAccess.h>
#include <DGDataAccessXREF.h>

//
// PUBLIC:
//

DGDataAccessXREF::DGDataAccessXREF(linkTypes & sym_flt, ddSelector & knd_flt)
: DGDataAccess(), link_types(sym_flt), xsym_types(knd_flt)
{
}

DGDataAccessXREF::~DGDataAccessXREF()
{
}

//
// returns: -1: error, >=0: the number of found symbols
//
int
DGDataAccessXREF::dependents(DGNodeData * x_sym, pqueue * q_out)
{
    int ret_val = -1;

    if (x_sym!=0 && q_out!=0) {
	ret_val = 0;

	if (x_sym->hierarchy() == DGNodeData::XREFsymbol) {
	    symbolArr q_symbols;
	    if (query_model(((DGNodeDataXREF *)x_sym)->symbol(), q_symbols) > 0) {
		// -- copy into the queue
		symbolPtr sym;
		ForEachS(sym,q_symbols) {
		    if (xsym_types.selects(sym.get_kind())) {
			x_sym = new DGNodeDataXREF(sym);
			if (x_sym) {
			    if (!(q_out->insert((void *)x_sym)))
				ret_val++;
			}
		    }
		}
	    }
	}
    }

    return ret_val;
}

int
DGDataAccessXREF::hierarchy()
{
    return DISModel;
}

//
// PROTECTED:
//

//
// returns: -1: error, >=0: the number of found symbols
//
int
DGDataAccessXREF::query_model(symbolPtr & x_sym, symbolArr & x_linked)
{
    int ret_val = -1;

    if (x_sym.is_xrefSymbol()) {
	int prev_size = x_linked.size();
	if (x_sym.get_link((linkType)(link_types), x_linked) >= 0)
	    ret_val = x_linked.size() - prev_size;
    }

    return ret_val;
}

