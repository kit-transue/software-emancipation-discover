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
#ifndef _gtShell_h
#include "gtShell.h"
#endif
#ifndef _gtBaseXm_h
#include <gtBaseXm.h>
#endif

#include <Xm/AtomMgr.h>
#include <Xm/Protocols.h>

extern void override_handler (Widget, void *client_data,
			      XmAnyCallbackStruct *)
{
    int destroy_me = (*((gtShell *)client_data)->override_CB)(client_data);
    if (destroy_me)
	XtDestroyWidget(((gtShell *)client_data)->rep()->widget());
}

void gtShell::override_WM_destroy(int (*callback)(void *cd))
{
    Arg		args[1];
    Atom	WM_DELETE_WINDOW;

    override_CB = callback;
    XtSetArg(args[0], XmNdeleteResponse, XmDO_NOTHING);
    XtSetValues(r->widget(), args, 1);
    
    WM_DELETE_WINDOW = XmInternAtom(XtDisplay(r->widget()),
				    "WM_DELETE_WINDOW", False);
    XmAddWMProtocolCallback(r->widget(), WM_DELETE_WINDOW,
			    (XtCallbackProc)override_handler, (caddr_t)this);
}

/*
   START-LOG-------------------------------------------

   $Log: WM_Override.C  $
   Revision 1.1 1993/10/01 15:52:48EDT builder 
   made from unix file
Revision 1.2.1.4  1993/10/01  17:31:22  kws
Port

Revision 1.2.1.3  1993/04/22  21:22:50  davea
bug 3461 - within override_WM_destroy(), do explicit cast
to satisfy ANSI C++ restrictions

Revision 1.2.1.2  1992/10/09  19:01:39  jon
RCS History Marker Fixup



   END-LOG---------------------------------------------

*/
