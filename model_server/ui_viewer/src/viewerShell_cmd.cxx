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
// viewerShell_cmd.C
//------------------------------------------
// synopsis:
// C functions to be called from command journals
//------------------------------------------

// include files

#include "objOper.h"
#include "viewerShell.h"


// function definitions

extern "C" viewerShell *new_viewerShell()
{
    return new viewerShell(CREATE_MINIBROWSER);
}

extern "C" void viewerShell_open_view(
    app* app_head, repType rep_type, appTree* app_tree)
{
    if(app_head == NULL && app_tree == NULL)
	return;

    if(app_head == NULL)
	app_head = app_tree->get_header();
    else if(app_tree == NULL)
	app_tree = appTreePtr(app_head->get_root());
    
    viewerShell::get_current_vs()->open_view(app_head, rep_type, app_tree);
}

extern "C" void viewerShell_map()
{
    viewerShell::get_current_vs()->map();
}

/*
   START-LOG-------------------------------------------

   $Log: viewerShell_cmd.C  $
   Revision 1.2 1995/04/20 10:45:29EDT azaparov 
   Bug track: N/A
   Fixes
Revision 1.2.1.3  1993/01/07  02:06:23  smit
made new_viewerShell return the created viewerShell.

Revision 1.2.1.2  1992/10/09  20:21:06  jon
RCS History Marker Fixup



   END-LOG---------------------------------------------

*/
