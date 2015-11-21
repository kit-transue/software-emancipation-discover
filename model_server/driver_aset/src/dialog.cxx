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
// C interface functions for dialog
//------------------------------------------
// synopsis:
// C interface functions to dialog class
//
// description:
// C interface to the following dialog methods:
//   get_title
//   untitled
//   create_window
//   resize_window
//   view_change
//------------------------------------------
// Restrictions:
// dialog_untitled is not yet implemented
//------------------------------------------

// include files

#include "driver.h"
#include "dialog.h"
#include <view.h>

//------------------------------------------
// dialog_name
//
// This function gets the title of the dialog
//------------------------------------------

char* dialog_name( void* dlg )
{
   Initialize(dialog_name);

   return checked_cast(dialog,dlg)->get_title ();
}


//------------------------------------------
// dialog_set_name
//
// This function sets the title of the dialog
//------------------------------------------

void dialog_set_name( void* dlg, char *name )
{
   Initialize (dialog_set_name);

   checked_cast(dialog,dlg)->set_title (name);

   Return;
}


//------------------------------------------

// dialog_view_name
//
// This function gets the title of the dialog
//------------------------------------------

char* dialog_view_name( void* dlg, int view_num )
{
   Initialize(dialog_view_name);

   return checked_cast(dialog,dlg)->get_view_name (view_num);
}

//------------------------------------------
// dialog_untitled
//
// Currently unimplimented
//------------------------------------------

int dialog_untitled( void* )
{
   Initialize (dialog_untitled);


   IF ( 1 ) ;

   ReturnValue (0);
}

//------------------------------------------
// dialog_create_window_instance
//
// Called from the expose callbacks to actually get an interviews window
// to be opened and parented
//------------------------------------------

void dialog_create_window_instance( int view_num, void* dlg, void* win )
{
   Initialize(dialog_create_window_instance);

   if (dlg != NULL)
      checked_cast(dialog,dlg)->create_window (view_num, win);
}

//------------------------------------------
// dialog_create_window
//
// Called from the expose callbacks to actually get an interviews window
// to be opened and parented
//------------------------------------------

void dialog_create_window( void* dlg, void* win  )
{
   Initialize(dialog_create_window);

   dialog_create_window_instance( 0, checked_cast(dialog,dlg), win );
}

//------------------------------------------
// dialog_resize_window
//
// Resize callback function called when the parent window was resized
//------------------------------------------

void dialog_resize_window( void* dlg, void* win )
{
   Initialize(dialog_resize_window);

   if (dlg != NULL)
      checked_cast(dialog,dlg)->resize_window (win);
}

//------------------------------------------
// dialog_view_change
//
// Called by the menus to change the view representation. 
// e.g. from text to diagram
//------------------------------------------

void dialog_view_change( void* dlg, void* win, char* , repType type )
{
   Initialize (dialog_view_change);


   dialog_change_view_type (dlg, win, type);

   Return;
}

// Routine to change view type
void dialog_change_view_type (void* dlg, void* win, repType type)
{
   Initialize (dialog_change_view_type);

   if (dlg != NULL)
      checked_cast(dialog,dlg)->change_view (win, 0, type);

   Return;
}

// opens a view
void dialog_open_view(
    void* dlg, int view_num, appTree *subroot, repType view_type)
{
    Initialize (dialog_open_view);

    if (dlg && subroot) {

	// get header
	app *header = checked_cast(app, subroot->get_header());

	// call open view method
	if (header)
	    checked_cast(dialog,dlg)->open_view (view_num, header, subroot, view_type);
    }

    Return;
}

    
// Closes the view to open a new view
void dialog_close_view (void* dlg, int view_num)
{
   Initialize (dialog_close_view);

   checked_cast(dialog,dlg)->close_view(view_num);

   Return;
}

void dialog_manipulate_perspective(void* dlg, int view_num, int op, 
				   int i1, int i2, int i3, int i4 )
{
    Initialize(dialog_manipulate_perspective);
    if (dlg != NULL)
	checked_cast(dialog,dlg)->manipulate_perspective(
	    view_num, op, i1, i2, i3, i4 );
    Return;
}

int dialog_get_slider_style(void* dlg, int view_num)
{
    Initialize(dialog_get_slider_style);
    if (dlg != NULL)
	ReturnValue(checked_cast(dialog,dlg)->get_slider_style(view_num));
    else
	ReturnValue(0);
}

void dialog_set_slider_style( void* dlg, int view_num, int style)
{
    Initialize(dialog_set_slider_style);
    if (dlg != NULL)
	checked_cast(dialog,dlg)->set_slider_style(view_num, style);
    Return;
}

void dialog_send_view_to_ui (void* dlg, int view_num, void *data)
{
    Initialize(dialog_send_view_to_ui);
    if (dlg != NULL)
      checked_cast(dialog,dlg)->send_view_to_ui (view_num, data);
    Return;
}

int dialog_get_view_type (void* dlg, int view_num)
{
    Initialize(dialog_get_view_type);

    if (dlg != NULL) {
	viewPtr viewp = checked_cast(dialog,dlg)->get_view_ptr(view_num);

	if (viewp)
	    ReturnValue(viewp->get_type());
    }

    ReturnValue(0);
}

void *dialog_get_view_ptr (void* dlg, int view_num)
{
   Initialize (dialog_get_view_ptr);

   if (dlg != NULL)
       ReturnValue (checked_cast(dialog,dlg)->get_view_ptr(view_num));
   else
       ReturnValue (NULL);
}

void dialog_register_scrollbars(
    void* dlg, int view_num, void *hscroll, void *vscroll)
{
   Initialize (dialog_register_scrollbars);

   if (dlg)
      checked_cast(dialog,dlg)->register_scrollbars (view_num, hscroll, vscroll);

   Return;
}

/*
   START-LOG-------------------------------------------

   $Log: dialog.cxx  $
   Revision 1.3 1996/04/01 12:51:28EST mg 
   cleanup
Revision 1.2.1.3  1992/11/21  21:04:38  builder
typesafe casts.

Revision 1.2.1.2  1992/10/09  18:43:45  smit
*** empty log message ***


   END-LOG---------------------------------------------

*/
