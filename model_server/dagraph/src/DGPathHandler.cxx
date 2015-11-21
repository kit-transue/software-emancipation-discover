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
// DGPathHandler.C
// ---------------
//

#include <DGPathHandler.h>

//
// PUBLIC:
//

DGPathHandler::DGPathHandler(DGPathFilter * pflt)
{
    proc_state  = INITIAL;
    proc_status = 0;
    path_filter = pflt;
}

DGPathHandler::~DGPathHandler()
{
    if (path_filter)
	delete path_filter;
}

//
// returns: -1: error/do not start processing, 0: the processing can start
//
int
DGPathHandler::start(DGNode * root_vertex)
{
    int ret_val = -1;

    if (root_vertex) {
	proc_state  = STARTED;
	proc_status = 0;
	ret_val     = 0;
    }

    return ret_val;
}

//
// returns: -1: error, >=0: the path is succ. processed
//
int
DGPathHandler::process(listItem *, int)
{
    return 0;
}

//
// returns: -1: error, 0: the processing is concluded succ.
//
int
DGPathHandler::finish(int, int stat_in)
{
    proc_state  = FINISHED;
    proc_status = stat_in;

    return 0;
}

int
DGPathHandler::hierarchy()
{
    return BASE;
}
