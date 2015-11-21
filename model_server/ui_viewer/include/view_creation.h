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
#ifndef _view_creation_h
#define _view_creation_h

// view_creation.h
//------------------------------------------
// synopsis:
// 
// Global utilities for creating and displaying views.
//------------------------------------------

#ifndef _representation_h
#include <representation.h>
#endif

#ifndef _Relational_h
#include <Relational.h>
#endif

#ifndef _objOper_h
#include <objOper.h>
#endif

class view;
class viewer;
class symbolPtr;
class app;
class appTree;

viewer* view_set_target_viewer(viewer*);
void    view_unset_target_viewer(viewer*);
viewer* view_target_viewer();
view*   view_create(symbolPtr, repType = Rep_UNKNOWN, int enforce = 0);
view*   view_create(const symbolArr&, repType);
view*   view_create(const char *filenm);
view*   view_create(app *);
view*   view_create_noselect(const char *filenm);
void    view_create_flush();
void    view_set_help_focus(boolean);
int     view_displays_app(view *, app *, appTree * root = NULL);
extern appPtr get_create_app_by_name (const char* f_name);
extern int has_view(app *h);

/*
   START-LOG-------------------------------------------

   $Log: view_creation.h  $
   Revision 1.1 1993/08/20 09:49:38EDT builder 
   made from unix file
// Revision 1.12  1993/08/20  12:34:55  davea
// Added #include objOper.h, so appPtr would be defined
//
// Revision 1.11  1993/08/04  00:06:46  so
// declare two external functions.
//
// Revision 1.10  1993/06/22  13:17:01  boris
// Fixed compilation problem
//
// Revision 1.8  1993/06/15  22:26:55  boris
// moved function view_displays_app() to non static
//
// Revision 1.7  1993/06/08  23:18:29  boris
// Added view_create_noselect() function
//
// Revision 1.6  1993/04/23  22:07:12  jon
// Added view_set_help_focus() to indicated views that are intended
// for the Helper Shell so the target button will be ignored. (bug 3373)
//
// Revision 1.5  1993/04/20  01:35:48  glenn
// Add view_target_viewer.
//
// Revision 1.4  1993/04/19  12:58:55  boris
// Fixed bug #3315.
//
// Revision 1.3  1993/01/22  22:54:15  wmm
// Support XREF-based ERDs and DCs.
//
// Revision 1.2  1992/12/15  20:26:22  aharlap
// changed for new xref
//
// Revision 1.1  1992/12/12  07:14:19  glenn
// Initial revision
//
   END-LOG---------------------------------------------
*/

#endif // _view_creation_h
