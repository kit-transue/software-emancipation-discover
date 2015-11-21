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
// disUIHandler.h
// --------------
//

#ifndef _disUIHandler_h
#define _disUIHandler_h

class appDGraph;
class linkTypes;
class symbolArr;

#ifndef NEW_UI
class gtRTL;
class gtDialogTemplate;
class gtStringEditor;
class gtPushButton;
#endif //NEW_UI

class DAGHandlerXREF;

class disUIHandler {
#ifndef NEW_UI
    gtDialogTemplate * shell;
    gtRTL *            src_syms;                  // source symbols
    gtRTL *            trg_syms;                  // target symbols
    gtStringEditor *   out_device;                // output filename
#else
    symbolArr*         src_arr;                  // source symbols
    symbolArr*         trg_arr;                  // target symbols
#endif
    int                out_destination;           // 0: default, >0: file

    DAGHandlerXREF *   xref_graph;                // graph handler

  protected:
#ifndef NEW_UI
    static void        create_CB(gtPushButton *, gtEventPtr, void *, gtReason);
    static void        apply_CB(gtPushButton *, gtEventPtr, void *, gtReason);
    static void        cancel_CB(gtPushButton *, gtEventPtr, void *, gtReason);
    static void        src2trg_CB(gtPushButton *, gtEventPtr, void *, gtReason);
    static void        trg2src_CB(gtPushButton *, gtEventPtr, void *, gtReason);
    static void        insert_CB(gtPushButton *, gtEventPtr, void *, gtReason);
    static void        remove_CB(gtPushButton *, gtEventPtr, void *, gtReason);
    static void        out_device_CB(gtToggleButton *, gtEventPtr, void *, gtReason);
#endif

    disUIHandler(linkTypes &, ddSelector &);

  public:

    virtual ~disUIHandler();

    int                gexists(int);
    int                restart(symbolArr &, symbolArr &);
#ifndef NEW_UI
    int                move_symbols(symbolArr &, RTLNode &, RTLNode &);
#endif
    int                filter_symbols(symbolArr &, symbolArr &);
    int                remove_symbols(symbolArr &);
    int                reduce_input(symbolArr &, symbolArr &);
    int                fetch_external(symbolArr &);
    int                generate(int);
    int                preview(char *);
    int                output(char *);

    static disUIHandler *   ui_handler(linkTypes *, ddSelector *);
#ifndef NEW_UI
    static int              update(gtRTL *);
    static int              extract(gtRTL *, symbolArr *&);
    static int              extract(gtRTL *, RTLNode *&);
#endif
    inline DAGHandlerXREF * graph();

};

DAGHandlerXREF *
disUIHandler::graph()
{
    return xref_graph;
}

#endif
