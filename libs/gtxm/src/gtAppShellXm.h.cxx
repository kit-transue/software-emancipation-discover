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
// gtAppShellXm.h.C
//------------------------------------------
// synopsis:
// Member functions of the gtApplicationShell class OSF Motif implementation
//------------------------------------------

// INCLUDE FILES

#include <gtAppShellXm.h>


// EXTERNAL VARIABLES

extern Widget UxTopLevel;
extern XtAppContext UxAppContext;


// VARIABLES DEFINITIONS

XtAppContext global_app_context;
gtRep* gtApplicationShell::global_app_rep;



// FUNCTION DEFINITIONS

gtApplicationShell* gtApplicationShell::create(
    String name, int argc, char* argv[])
{
    return new gtApplicationShellXm(name, argc, argv);
}

gtApplicationShell::~gtApplicationShell () {}


gtApplicationShellXm::gtApplicationShellXm(
    const char* name, int argc, char* argv[])
{
    global_app_rep = new gtRep;
    global_app_rep->widget(

    XtVaAppInitialize(
	    &global_app_context, name, NULL, 0, &argc, argv,
	    NULL, NULL));

    r->widget(global_app_rep->widget());
    destroy_init();
}


gtApplicationShellXm::~gtApplicationShellXm()
{
    delete global_app_rep;
    global_app_rep = NULL;
}


void gtApplicationShellXm::event_loop()
{
    XtAppMainLoop(global_app_context);
}


gtBase* gtApplicationShellXm::container()
{
    return this;
}


Widget gtApplicationShellXm::find_existing()
{
    if (UxTopLevel)
	return UxTopLevel;
    else
	return global_app_rep->widget();
}

	
XtAppContext gtApplicationShellXm::get_context()
{
    return UxAppContext ? UxAppContext : global_app_context;
}


/*
   START-LOG-------------------------------------------

   $Log: gtAppShellXm.h.C  $
   Revision 1.6 2000/07/07 08:10:28EDT sschmidt 
   Port to SUNpro 5 compiler
Revision 1.2.1.9  1994/06/23  18:21:06  builder
sun5 prot

Revision 1.2.1.8  1994/03/29  01:20:34  builder
Port

Revision 1.2.1.7  1994/02/11  03:51:16  builder
Port

Revision 1.2.1.5  1993/10/04  14:03:01  kws
Port

Revision 1.2.1.4  1993/10/01  17:31:22  kws
Port

Revision 1.2.1.3  1993/01/26  04:32:20  glenn
set global_app_rep to NULL in dtor.

Revision 1.2.1.2  1992/10/09  19:03:21  jon
RCS History Marker Fixup

   END-LOG---------------------------------------------
*/
