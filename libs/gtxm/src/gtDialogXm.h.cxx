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
// gtDialogXm.h.C
//------------------------------------------

// INCLUDE FILES

#include <gtDialog.h>
#include <gtBaseXm.h>

static gtDialog* last_popup = NULL;

// FUNCTION DEFINITIONS
void bring_modal_dialog_to_top()
{

    if (last_popup && !last_popup->ChildHasFocus())
      last_popup->bring_to_top();
}

void clear_modal_dialog(gtDialog* x)
{
    if (x == last_popup)
        last_popup = NULL;
}

void clear_modal_dialog()
{
    last_popup = NULL;
}


int gtDialog::ChildHasFocus()
{
// This function address OpenWin filter dialog bug.
// It will detect if one of the dialog's children has
// input focus. bring_modal_dialog_to_top() uses this function to 
// decide whether a dialog has to activate itself (it doesn't make
// sense to activate a dialog if its child has focus)

    int nReturn = 0;
    Widget w; 
    Window wnd;
    int rev;
    XGetInputFocus( XtDisplay(r->widget()), &wnd, &rev );
    w = XtWindowToWidget( XtDisplay(r->widget()), wnd ); 
    if( w )
    {
      while( w=XtParent( w ) )
        if( XtWindow( w ) == XtWindow(r->widget()) )
        {
	  nReturn = 1;
          break;  
        }
    }
    return nReturn;
}


void gtDialog::popup(int modal)
{
    Arg args[1];
    WidgetList wlist;
    XtSetArg(args[0], XmNchildren, &wlist);

    XtGetValues(r->widget(), args, 1);

    int style;
    switch(modal)
    {
      default:
      case 0: style = XmDIALOG_MODELESS; break;

      case 1: style = XmDIALOG_FULL_APPLICATION_MODAL; 
	      last_popup = this;
	      dflt_parent = this;
	      break;

      case 2: style = XmDIALOG_SYSTEM_MODAL; 
	      last_popup = this;
	      dflt_parent = this;
	      break;

      case 3: style = XmDIALOG_FULL_APPLICATION_MODAL; 
	      dflt_parent = this;
	      break;
    }

    XtSetArg(args[0], XmNdialogStyle, style);
    XtSetValues(wlist[0], args, 1);

    XtManageChild(wlist[0]);

    gtShell::popup();
}

/*
   START-LOG-------------------------------------------

   $Log: gtDialogXm.h.C  $
   Revision 1.4 1996/08/26 18:49:13EDT mstarets 
   OpenWin Filter Dialog Bug
Revision 1.2.1.4  1993/08/04  15:48:59  swu
part of the fix for bug 4111

Revision 1.2.1.3  1992/10/23  17:37:58  glenn
Call gtShell::popup instead of XtPopup to insure window is drawn
after being mapped.

Revision 1.2.1.2  92/10/09  19:03:28  jon
RCS History Marker Fixup

   END-LOG---------------------------------------------
*/
