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
// DGPrintXREF.h
// -------------
//

#ifndef ISO_CPP_HEADERS
#include <stdio.h>
#else /* ISO_CPP_HEADERS */
#include <cstdio>
namespace std {};
using namespace std;
#endif /* ISO_CPP_HEADERS */
#include <listItem.h>

#include <symbolPtr.h>
#include <xrefSymbol.h>

#include <DGNode.h>
#include <DGPathHandler.h>
#include <DGPathXREF.h>
#include <DGPrintXREF.h>

//
// PUBLIC:
//

DGPrintXREF::DGPrintXREF(DGPathFilter * sym_flt, FILE * prt_file)
: DGPathXREF(sym_flt)
{
    output_file = prt_file;
}

DGPrintXREF::~DGPrintXREF()
{
}

int
DGPrintXREF::start(DGNode * root_vertex)
{
    int ret_val = -1;

    if (DGPathHandler::start(root_vertex) >= 0) {
	if (output_file) {
	    symbolPtr xref_sym;
	    if (!symbol(root_vertex, xref_sym)) {
		if (fprintf(output_file, "\n{") >= 0) {
		    if (writesym(xref_sym) >= 0)
			ret_val = 0;
		    (void) fprintf(output_file, "}\n");
		}
	    }
	}
    }

    return ret_val;
}

int
DGPrintXREF::process(listItem * path_in, int distance)
{
    int ret_val = -1;

    if (path_in!=0 && output_file!=0) {
	symbolPtr xref_sym;
	// -- write path header out
	ret_val = writehdr(distance);
	// -- iterate other symbols
	for (path_in=path_in->next(); path_in!=0 && ret_val>=0; path_in=path_in->next()) {
	    if (!symbol((DGNode *)(path_in->data()), xref_sym)) {
		if (writeln(xref_sym, ret_val) < 0)
		    ret_val = -1;
		else
		    ret_val++;
	    }
	}
    }

    return ret_val;
}

int
DGPrintXREF::finish(int no_paths, int stat_in)
{
    int ret_val = -1;

    if (DGPathHandler::finish(no_paths, stat_in) >= 0) {
	if (output_file) {
	    if (fprintf(output_file, "\n[%d]%s\n", no_paths, (stat_in<0)?"...":"") >= 0)
		ret_val = 0;
	}
    }

    return ret_val;
}

int
DGPrintXREF::hierarchy()
{
    return XREFPrint;
}

//
// PROTECTED:
//

//
// returns: -1: error, 0: the header is printed out
//
int
DGPrintXREF::writehdr(int distance)
{
    int ret_val = -1;

    if (output_file) {
	if (fprintf(output_file, "\n  |E| = %d\n", distance) >= 0)
	    ret_val = 0;
    }

    return ret_val;
}

//
// returns: -1: error, >=0: the number of printed symbols
//
int
DGPrintXREF::writeln(symbolPtr & xref_sym, int seq_no)
{
    int ret_val = -1;

    if (output_file) {
	ret_val = 0;

	if (fprintf(output_file, "  %5d. ", seq_no) >= 0) {
	    ret_val = writesym(xref_sym);
	    (void) fprintf(output_file, "\n");
	}
    }

    return ret_val;
}

//
// returns: -1: error, >=0: the number of output symbols
//
int
DGPrintXREF::writesym(symbolPtr & xref_sym)
{
    int ret_val = -1;

    if (output_file) {
	ret_val = 0;

	char * sym_name = xref_sym.get_name();
	if (sym_name) {
	    if (fprintf(output_file, "%s", sym_name) >= 0)
		ret_val =  1;
	    else
		ret_val = -1;
	}
    }

    return ret_val;
}
