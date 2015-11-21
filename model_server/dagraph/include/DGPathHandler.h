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
// DGPathHandler.h
// ---------------
//

#ifndef _DGPathHandler_h
#define _DGPathHandler_h

class listItem;
class DGNode;
class DGPathFilter;

class DGPathHandler {

  protected:

    enum PROCSTATE {
	INITIAL  = 0,
	STARTED  = 1,
	FINISHED
    };

    PROCSTATE      proc_state;                   // current processing state of the handler
    int            proc_status;                  // the last processing status
    DGPathFilter * path_filter;                  // value of the path filter

  public:

    enum HIERARCHY {
	BASE     = 1,
	XREFBase,
	XREFPrint
    };

    DGPathHandler(DGPathFilter *);
    virtual ~DGPathHandler();

    virtual int start(DGNode *);                 // path extraction is ready to start
    virtual int process(listItem *, int);        // list of nodes that constitute a path
    virtual int finish(int, int);                // path extraction is completed

    virtual int hierarchy();

    inline  DGPathFilter * filter();

};

DGPathFilter *
DGPathHandler::filter()
{
    return path_filter;
}

#endif
