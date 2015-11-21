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
// viewShellData.C
//------------------------------------------
// synopsis:
// class-static data for viewerShell
//------------------------------------------

// INCLUDE FILES

#include <viewerShell.h>
#include <RTL_externs.h>


// EXTERNAL FUNCTIONS

Widget create_style_menu(Widget, void*);
Widget create_categ_menu(Widget, void*);
Widget create_OODT_menu(Widget, void*);
Widget create_TREE_menu(Widget, void*);
Widget create_ERD_menu(Widget, void*);
Widget create_DC_menu(Widget, void*);


// VARIABLE DEFINITIONS

objArr        viewerShell::vsh_array;	// Array of all existing viewerShells

viewerShell*  viewerShell::hook_shell;	// Shell that invoked current command.

viewer*       viewerShell::the_focused_viewer;
viewPtr       viewerShell::the_focused_view_header;


// *_menu_creators arrays must be NULL-terminated.

static slotfill_func SMT_menu_creators[] =
{
//    create_categ_menu,
//    create_style_menu,
    NULL
};

static slotfill_func TREE_menu_creators[] =
{
    create_TREE_menu,
    NULL
};


static slotfill_func OODT_menu_creators[] =  // used for oodt browser
{
    create_OODT_menu,
    NULL
};

static slotfill_func ERD_menu_creators[] =  // used for ERD
{
    create_ERD_menu,
    NULL
};

static slotfill_func DC_menu_creators[] =  // used for data chart
{
    create_DC_menu,
    NULL
};



// viewerShell::create_pullMenu[] is an array of pointers
// to NULL-terminated arrays.  This array must be kept in sync
// with enum viewType.

slotfill_func* viewerShell::create_pullMenu[view_Last + 1] =
{
    NULL,			// view_STE
    SMT_menu_creators,		// view_SMT
    TREE_menu_creators,		// view_CallTree
    OODT_menu_creators,		// view_Class
    ERD_menu_creators,		// view_ERD
    NULL,	// view_SubsysBrowser
    NULL,	// view_SubsysMap
    DC_menu_creators,		// view_DC
    NULL,			// view_Raw

    NULL			// view_Last
};
