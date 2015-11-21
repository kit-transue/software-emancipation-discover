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
// viewerShell_btn_bar.C
//-----------------------------------------------------------------
// synopsis:
// Implementation of viewerShell methods relating to the button bar
//-----------------------------------------------------------------

#ifndef ISO_CPP_HEADERS
#include <stdlib.h>
#else /* ISO_CPP_HEADERS */
#include <cstdlib>
using namespace std;
#endif /* ISO_CPP_HEADERS */

#include <msg.h>
#include <genString.h>
#include <systemMessages.h>
#include <customize.h>
#include <viewerShell.h>

#include <Xm/PushB.h>
#include <Xm/ToggleB.h>

extern "C"
{
    Widget create_buttonbar_rowcol(Widget);
    void   vs_set_buttonbar(Widget, int);
    int    vs_get_buttonbar(Widget);
}


static void bb_btn_destroy_CB(Widget UxWidget, XtPointer, XtPointer)
//
// Free the memory used for the search string.
//
{
    Initialize(bb_btn_destroy_CB);

    genString* search_path = NULL;
    XtVaGetValues(UxWidget, XmNuserData, &search_path, NULL);
    delete search_path;
}

static void bb_btn_callback(Widget UxWidget, XtPointer client, XtPointer call_arg)
//
// What to do when a button in the button-bar is pressed.
//
{
    Initialize(bb_btn_callback);

    viewerShell* vs = (viewerShell*)client;
    genString* search_path = NULL;

    switch (vs->is_customizing())
    {
      case VC_customize:	// Do nothing.
	break;

      case VC_remove:		// Destroy this button.
	XtDestroyWidget(UxWidget);
	break;

      case VC_normal:		// Imitate a press on the original button.
	{
	XtVaGetValues(UxWidget, XmNuserData, &search_path, NULL);
	Assert(search_path);

	Widget w = XtNameToWidget(vs->get_widget(), search_path->str());
	if(w){
	    if(XmIsToggleButton(w))
		XtCallCallbacks(w, XmNvalueChangedCallback, call_arg);
	    else
		XtCallCallbacks(w, XmNactivateCallback, NULL);
	}
	else
	    msg("Button-bar could not find widget: \n\"$1\"", error_sev) << search_path->str() << eom;
        }
    }
} 


void viewerShell::add_buttonbar_item(Widget w)
{
    Initialize(viewerShell::add_buttonbar_item);

    add_buttonbar_item(current_type, w);
}


void viewerShell::add_buttonbar_item(int tp, Widget original)
//
// Create a new button in the button bar.  The button will imitate <original>,
// and will be visible only when the type of view matches <tp>.
//
{
    Initialize(viewerShell::add_buttonbar_item);

    if(original == NULL)
	return;

    // Get labelString of original widget.
    XmString label;
    XtVaGetValues(original, XmNlabelString, &label, NULL);

    const char* name = XtName(original);
    const char* parent_name = XtName(XtParent(original));

    // Path for the button to search for the original menu item.
    genString* search_path = new genString;
    search_path->printf("*%s.%s", parent_name, name);

    // Create the button-bar if necessary.
    if(buttonBar[tp] == NULL)
    {
	buttonBar[tp] = create_buttonbar_rowcol(buttonBarSlot);
	if(tp != current_type)
	    XtUnmanageChild(buttonBar[tp]);
    }

    // Create the pushButton
    genString BB_name;
    BB_name.printf("_BB_%s_%d", name,tp);
    Widget ViewShellWidget = this->get_widget();
    genString old_name("*");
    old_name += BB_name;
    Widget old_button = XtNameToWidget(ViewShellWidget, (char *)old_name);
    if(old_button){
	msg("Button already exists.", error_sev) << eom;
	return;
    }

    Widget button;
    if(XmIsToggleButton(original)){
	button = XmCreateToggleButton(buttonBar[tp], (char *)BB_name, NULL, 0);
	XtVaSetValues(button,

		      // Copy the original labelString
		      XmNlabelString, label, 

		      // Store the search-path to the original widget in
		      // userData.  (using name instead of Widget allows
		      // save/restore button bar to work.)
		      XmNuserData, search_path,
		      
		      // This button uses 1/100th mm as its unit type.
		      XmNmarginHeight, 20,
		      XmNmarginWidth, 60,
		      XmNshadowThickness, 60,
		      
		      NULL);
	XtAddCallback(button, XmNvalueChangedCallback, bb_btn_callback, XtPointer(this));
    }
    else{
	button = XmCreatePushButton(buttonBar[tp], (char*)BB_name, NULL, 0);
	XtVaSetValues(button,

		      // Copy the original labelString
		      XmNlabelString, label, 

		      // Store the search-path to the original widget in
		      // userData.  (using name instead of Widget allows
		      // save/restore button bar to work.)
		      XmNuserData, search_path,
		      
		      // This button uses 1/100th mm as its unit type.
		      XmNmarginHeight, 60,
		      XmNmarginWidth, 60,
		      XmNshadowThickness, 60,
		      
		      NULL);
	XtAddCallback(button, XmNactivateCallback, bb_btn_callback, XtPointer(this));
    }
    XtAddCallback(button, XmNdestroyCallback, bb_btn_destroy_CB, XtPointer(this));

    XtManageChild(button);
}

void viewerShell::add_buttonbar_item(int tp, char* search_string)
{
    Initialize(viewerShell::add_buttonbar_item);

    Widget original = XtNameToWidget(wij, search_string);
    if(original)
	add_buttonbar_item(tp, original);
    else
	msg("viewerShell: Could not restore widget.", error_sev) << eom;

    // DO NOT print out search_string on failure, since it could contain
    // garbage characters that will confuse Epoch.
}

extern "C" void viewerShell_save_buttonbars(viewerShell* vs)
{ vs->save_buttonbars(); }

void viewerShell::save_buttonbars()		
{
    Initialize(viewerShell::save_buttonbars);

    genString buffer[view_Last];

    for(int i = 0; i < view_Last; i++)
    {
	if(buttonBar[i])
	{
	    WidgetList bbar_items;
	    Cardinal num_buttons;

	    XtVaGetValues(buttonBar[i],
			  XmNchildren, &bbar_items,
			  XmNnumChildren, &num_buttons,
			  NULL);

	    for(int j = 0; j < num_buttons; j++)
	    {
		Widget btn = bbar_items[j];
		genString* search_path = NULL;
		XtVaGetValues(btn, XmNuserData, &search_path, NULL);
		Assert(search_path);

		buffer[i] += *search_path;
		if(j < num_buttons - 1)
		    buffer[i] += '\t';
	    }
	}
	else
	    buffer[i] = "";
    }

    customize::ste_buttonbar(buffer[view_STE]);
    customize::smt_buttonbar(buffer[view_SMT]);
    customize::calltree_buttonbar(buffer[view_CallTree]);
    customize::class_buttonbar(buffer[view_Class]);
    customize::erd_buttonbar(buffer[view_ERD]);
    customize::subsysBrowser_buttonbar(buffer[view_SubsysBrowser]);
    customize::subsysMap_buttonbar(buffer[view_SubsysMap]);
    customize::dc_buttonbar(buffer[view_DC]);

    // Save the button bar visibility state.
    if(wij != NULL)		// wij is the topLevelShell
	customize::button_bar_visible(vs_get_buttonbar(wij) == TRUE);

    customize::save_buttonbar(NULL);

    // load this buttonbar configuration into other viewer shells
    Obj* el;
    ForEach (el, vsh_array)
    {
	viewerShell* vsh = (viewerShell*)el;
	if(vsh != this)
	    vsh->load_buttonbars();
    }
}


void viewerShell::load_buttonbars()
{
    Initialize(viewerShell::load_buttonbars);

    genString buffer[view_Last];

    // Load in the saved button bars, each saved as a string of 
    // widget paths separated by tabs.
    buffer[view_STE] = customize::ste_buttonbar();
    buffer[view_SMT] = customize::smt_buttonbar();
    buffer[view_CallTree] = customize::calltree_buttonbar();
    buffer[view_Class] = customize::class_buttonbar();
    buffer[view_ERD] = customize::erd_buttonbar();
    buffer[view_SubsysBrowser] = customize::subsysBrowser_buttonbar();
    buffer[view_SubsysMap] = customize::subsysMap_buttonbar();
    buffer[view_DC] = customize::dc_buttonbar();

    for(int i = 0; i < view_Last; i++)
    {
	// destroy all existing buttonbar items
	if(buttonBar[i])
	{
	    WidgetList bbar_items;
	    Cardinal num_buttons;
	    XtVaGetValues(buttonBar[i],
			  XmNchildren, &bbar_items,
			  XmNnumChildren, &num_buttons,
			  NULL);

	    for(int j = 0; j < num_buttons; j++)
		XtDestroyWidget(bbar_items[j]);
	}

	// load in the new items for this buttonbar
	if(buffer[i].length())
	{
	    for(char* btn_string = strtok((char*)buffer[i], "\t");
		btn_string; btn_string = strtok(NULL, "\t"))
	    {
		add_buttonbar_item(i, btn_string);
	    }
	}
    }

    // If visible, display the button bar.
    if(wij)		// wij is the topLevelShell
	vs_set_buttonbar(wij, customize::button_bar_visible() == TRUE);
}


void viewerShell::sensitize_buttonbar()
{
    Initialize(viewerShell::sensitize_buttonbar);

    if(current_type < 0 || current_type > view_Last)
	return;

    Widget current_button_bar = buttonBar[current_type];
    if(current_button_bar)
    {
	WidgetList buttons;
	Cardinal num_buttons;
	XtVaGetValues(current_button_bar,
		      XmNchildren, &buttons,
		      XmNnumChildren, &num_buttons,
		      NULL);

	for(int i = 0; i < num_buttons; i++)
	{
	    genString* search_path = NULL;
	    XtVaGetValues(buttons[i], XmNuserData, &search_path, NULL);
	    Assert(search_path);

	    Widget original = XtNameToWidget(wij, search_path->str());
	    if(original)
	    {
		Widget parent = XtParent(original);
		XtCallCallbacks(parent, XmNmapCallback, NULL);
		XtSetSensitive(buttons[i], XtIsSensitive(original));
	    }
	    else
		XtSetSensitive(buttons[i], 0);
	}
    }
}

void viewerShell::manage_buttonbar_buttons()
{
    Initialize(viewerShell::sensitize_buttonbar);

    if(current_type < 0 || current_type > view_Last)
	return;

    Widget current_button_bar = buttonBar[current_type];
    if(current_button_bar)
    {
	WidgetList buttons;
	Cardinal num_buttons;
	XtVaGetValues(current_button_bar,
		      XmNchildren, &buttons,
		      XmNnumChildren, &num_buttons,
		      NULL);

	for(int i = 0; i < num_buttons; i++)
	{
	    genString* search_path = NULL;
	    XtVaGetValues(buttons[i], XmNuserData, &search_path, NULL);
	    Assert(search_path);

	    Widget original = XtNameToWidget(wij, search_path->str());
	    if(original  &&  buttons[i])
		XtManageChild(buttons[i]);
	}
    }
}


/*
   START-LOG-------------------------------------------

   $Log: viewerShell_btn_bar.C  $
   Revision 1.6 2000/11/28 08:39:58EST sschmidt 
   Fix for bugs 20106, 20107: clean up messages, make some more popups, implement message database.
Revision 1.2.1.18  1993/11/26  20:26:28  azaparov
Bug 5180 fixed

Revision 1.2.1.17  1993/11/23  21:54:42  azaparov
Fixed bug 4145

Revision 1.2.1.15  1993/06/24  16:43:25  bakshi
added brackets fot c++3.0.1 port

Revision 1.2.1.14  1993/05/11  15:23:06  glenn
Renamed create_btn_bar1 to create_buttonbar_rowcol.
Do not pass viewerShell* to create_buttonbar_rowcol.

Revision 1.2.1.13  1993/04/02  16:03:33  glenn
Fix bug #3090 by suppressing output of bogus search_path.
Use array of genStrings instead of array of fixed length char arrays
when saving button-bars.
Fix improper conversion of XmNuserData to char* (should be genString*).

Revision 1.2.1.12  1993/03/30  20:23:13  glenn
Fix fnh (Free Non-Heap) error due to mis-use of genString in XmNuserData.

Revision 1.2.1.11  1993/02/18  14:16:03  oak
Changed to save the button bar to the psetPrefs file.

Revision 1.2.1.10  1993/02/12  02:08:18  glenn
Rename bbar.sav file to ~/.psetbuttons
Show error dialog when opening button bar file for write fails.
v_shell_array renamed to vsh_array.

Revision 1.2.1.9  1993/02/08  22:17:30  oak
Fixed crash with using buttons from the button bar.
Fixes bug #2386.

Revision 1.2.1.8  1993/02/08  21:47:45  oak
Fixed problem with button bar save.
Fixes bug #2304

Revision 1.2.1.7  1993/02/04  16:10:29  oak
Fixed button bar save.  Button bar load is
still broken.

Revision 1.2.1.6  1992/12/22  16:16:24  glenn
Call is_customizing instead of get_mode.

Revision 1.2.1.5  1992/12/14  22:22:07  glenn
Use genStrings for widget names and search_paths.
Add prefix to widget names in buttonbar.

Add sensitize_buttonbar.

Revision 1.2.1.4  1992/12/12  07:07:06  glenn
New view creation interface.

Revision 1.2.1.3  1992/10/12  16:28:04  oak
Saved the state of the button bar.
This fixes bug #1252.

Revision 1.2.1.2  92/10/09  20:21:05  jon
RCS History Marker Fixup

   END-LOG---------------------------------------------
*/
