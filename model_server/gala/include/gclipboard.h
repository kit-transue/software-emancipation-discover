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
// gxClipBoard
// -----------
//

#ifndef _gxClipBoard_h
#define _gxClipBoard_h

class gxClipBoard {

	    vclipboard *        clip_ctrl;
            vscrap *            clip_data;
	    rtlClient *         clip_data_rtl;

  protected:

    static  gxClipBoard *       app_clipboard;

    static  void                ClipRecvCB(vclipboard *, vscrap *);

  public:

            gxClipBoard(vbool);
    virtual ~gxClipBoard();

            int                 ReadClipData();

            int                 Cut(vtextitem *);

            int                 Copy(vtextitem *);
            int                 Copy(vlistitem *);
            int                 Copy(rtlClient *);

            int                 Paste(vtextitem *);
            int                 Paste(rtlClient *);

    // -- global interface
    static  int                 Cut(vobjectAttributed *);
    static  int                 Copy(vobjectAttributed *);
    static  int                 Paste(vobjectAttributed *);

    static  vobjectAttributed * GetClipSelection(vdialogItemList *);
    static  int                 Destroy();

};

#endif

