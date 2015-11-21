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
// ui-rowcolumn.h.C
//
//------------------------------------------

#ifndef ISO_CPP_HEADERS
#include <stdlib.h>
#include <string.h>
#else /* ISO_CPP_HEADERS */
#include <cstdlib>
using namespace std;
#include <cstring>
#endif /* ISO_CPP_HEADERS */

#include <genError.h>
#include <ui-rowcolumn.h>

#include <Xm/PushB.h>

#include <machdep.h>

ui_rowcolumn::ui_rowcolumn(Widget p, ui_rowcolumn_CB cb, void* data, int prim)
: strs(NULL),
  num(0),
  maxnum(0),
  callback(cb),
  client_data(data),
  parent(p),
  primary(prim)   // Shows whenever this controller controls first view in viewershell or additional(splitted) ones... 
                  // (Generally it\'s sux)
{}


ui_rowcolumn::~ui_rowcolumn()
{
    Initialize(ui_rowcolumn::~ui_rowcolumn);

    clear();
}


void ui_rowcolumn::clear()
{
    Initialize(ui_rowcolumn::clear);

    if(strs)
    {
	for(int i = 0; i < num; i++)
	    delete strs[i];
	delete strs;
	strs = NULL;		// Avoid double-delete.
    }
    num = maxnum = 0;
}


void ui_rowcolumn::grow(int count)
{
    Initialize(ui_rowcolumn::grow);

    if((num + count) > maxnum)
    {
	maxnum = (((num + count - 1) >> 4) + 1) << 4;
	char** new_strs = new char*[maxnum];
	OS_dependent::bcopy(strs, new_strs, num * sizeof(char*));
	delete strs;
	strs = new_strs;
    }
    num += count;
}


void ui_rowcolumn::add(int pos, char *str)
{
    Initialize(ui_rowcolumn::add);

    if(str)
    {
	char* new_str = new char[strlen(str) + 1];
	strcpy(new_str, str);

	if(pos < 0  ||  pos >= (num - 1))
	    pos = num;
	grow(1);
	for(int i = num - 1; i > pos; i--)
	    strs[i] = strs[i - 1];
	strs[pos] = new_str;
    }
}

// focus the view with "No View" title
int ui_viewer_focus_viewer (void *);

static const char MENU_ITEM_NAME[] = "name_of_view";

void ui_rowcolumn::button_callback(Widget w, ui_rowcolumn* ui_rc, void*)
{
    Initialize(ui_rowcolumn::button_callback);

    if(ui_rc)
    {
	int        count;
	WidgetList list;
	XtVaGetValues(ui_rc->parent, XmNnumChildren, &count, XmNchildren, &list, NULL);

	// Test for and mark dummy widget for destruction.
	// widget does not end up with the same address.
	Widget dummy;
	if(w == list[0])
	{
	    // ui_rc->client_data - pointer to viewer.
//	    printf("Switch to raw bugger  viewer : %x\n", ui_rc->client_data);
	    (void) ui_viewer_focus_viewer (ui_rc->client_data);
	} else {
	    int which = 0;
	    XtVaGetValues(w, XmNuserData, &which, NULL);
	    if(ui_rc->callback  &&  which >= 0  &&  which < ui_rc->num)
	    {
		char* str = ui_rc->strs ? ui_rc->strs[which] : NULL;
		(*ui_rc->callback)(which, str, ui_rc->client_data);
	    }
	}
    }
}

void ui_rowcolumn::regenerate ()
{
    Initialize(ui_rowcolumn::regenerate);

    int        count;
    WidgetList list;
    XtVaGetValues(parent, XmNnumChildren, &count, XmNchildren, &list, NULL);

    if(!strs){
	// Cleanup list
	for(int i = 1; i < count; i++)
	    XtUnmanageChild(list[i]);
	return;
    }

    // Test for and mark dummy widget for destruction.
    // Do not destroy the dummy right away so that a new (non-dummy)
    // widget does not end up with the same address.
    Widget dummy = NULL;
    if(count == 1  &&  strcmp(XtName(list[0]), MENU_ITEM_NAME))
    {
	dummy = list[0];
//	if(!primary)  // Do not overwrite "No View" dummy widget in secondary view.
//	    count = 0;
    }

    if(strcmp(XtName(list[0]), MENU_ITEM_NAME)){
	XtCallbackRec cb[2]; // This must be a NULL-terminated array.
	cb[0].callback = XtCallbackProc(button_callback);
	cb[0].closure  = XtPointer(this);
	cb[1].callback = NULL;
	cb[1].closure  = NULL;
	XtVaSetValues(list[0], XmNactivateCallback, &cb,
		      XmNuserData, 0, NULL);
    }

    for(int i = 1; i < num + 1; i++)
    {
	int n = 0;
	Arg args[3];

	XmString str = XmStringCreateSimple(strs[i - 1]);
	XtSetArg(args[n], XmNlabelString, str), n++;

	if(i < count)		// Re-use existing widget
	{
	    XtSetValues(list[i], args, n);
	    XtManageChild(list[i]);
	}
	else			// Make a new widget
	{
	    XtCallbackRec cb[2]; // This must be a NULL-terminated array.
	    cb[0].callback = XtCallbackProc(button_callback);
	    cb[0].closure  = XtPointer(this);
	    cb[1].callback = NULL;
	    cb[1].closure  = NULL;

	    XtSetArg(args[n], XmNactivateCallback, &cb), n++;
	    XtSetArg(args[n], XmNuserData, i - 1), n++;

	    XtManageChild(
		XmCreatePushButton(parent, (char*)MENU_ITEM_NAME, args, n));
	}
	XmStringFree(str);
    }

    // Hide unused widgets.
    if(count > num + 1)
	XtUnmanageChildren(list + num + 1, count - num - 1);
    // if(primary && dummy)
    // XtDestroyWidget(dummy);
}


/*
   START-LOG-------------------------------------------

   $Log: ui-rowcolumn.h.C  $
   Revision 1.7 2000/07/10 23:12:05EDT ktrans 
   mainline merge from Visual C++ 6/ISO (extensionless) standard header files
Revision 1.2.1.5  1993/07/20  18:57:28  harry
HP/Solaris Port

Revision 1.2.1.4  1993/02/20  23:56:24  glenn
Remove most C "glue" functions.
Simplify callback procedure.
Simplify string array maintenance.
Rewrite regenerate method to handle possible dummy buttons.
Use MENU_ITEM_NAME for name of buttons.
Change constructor to take callback function and data.

Revision 1.2.1.3  1993/01/06  14:51:32  smit
Fix yet another purify problem

Revision 1.2.1.2  1992/10/09  19:59:43  jon
RCS History Marker Fixup

   END-LOG---------------------------------------------
*/
