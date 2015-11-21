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
#include "mtkchangeitem.h"
#include "framework.h"

//----------------------------------------------------------------------------------

changeItem::changeItem(char const *base_name, int n, FrameWork &fw,
		       char const *title, char const *text, int state,
                       ChangeData *data) :
		callback_data(data),	// caller must free
		// item_window_name,
		// checked_name,
		checked_value(state),
		id(n),
		framework(fw)
		// tmp
{
	// Set up the names.
	tmp.printf("%s.f%d", base_name, id);
	item_window_name = new char[tmp.length() + 1];
	strcpy(item_window_name, tmp.str());

	tmp.printf("whetherToApply%d", id);
	checked_name = new char[tmp.length() + 1];
	strcpy(checked_name, tmp.str());

	// Form the Tk window.
	framework.Tcl("addChangeInfoWindow %s", item_window_name);

	// Put text on label.
	framework.Tcl("%s configure -text %s", LabelName(),
	               framework.quoteForTclString(title).str());

	// Put text into text field.
        framework.Tcl("%s insert end %s", TextFieldName(),
		       framework.quoteForTclString(text).str());

	// Disable the text field.
	framework.Tcl("%s configure -state disabled", TextFieldName());

	// Create the checkbutton, with callback and associated variable.
	Tcl_LinkVar(framework.interp, checked_name, (char *)&checked_value, TCL_LINK_INT);
	genString cb_cmd;
	cb_cmd.printf("apply_cb %d", id);
	framework.Tcl("checkbutton %s -text \"Apply\" -variable %s -command %s",
	               Control1Name(),
	               checked_name,
	               framework.quoteForTclString(cb_cmd.str()).str());

	framework.Tcl("pack %s -side bottom -fill x", TextFieldName());
	framework.Tcl("pack %s -side left -anchor sw", LabelName());
	framework.Tcl("pack %s -side right -anchor se", Control1Name());
}

//----------------------------------------------------------------------------------

changeItem::~changeItem()
{
	// Remove the checkbutton's associated variable.
	Tcl_UnlinkVar(framework.interp, checked_name);

	framework.Tcl("removeChangeInfoWindow %s", item_window_name);

	delete[] checked_name;
	delete[] item_window_name;
}

//----------------------------------------------------------------------------------

void changeItem::EnableCheckBox(int enable)
{
	framework.Tcl("%s configure -state %s", Control1Name(),
	               enable ? "normal" : "disabled");
}

//----------------------------------------------------------------------------------

const char *changeItem::TextFieldName()
{
    (tmp = genString(item_window_name)) += ".changeText.textField";
    return tmp.str();
}

//----------------------------------------------------------------------------------

const char *changeItem::LabelName()
{
    (tmp = genString(item_window_name)) += ".editorLabel";
    return tmp.str();
}

//----------------------------------------------------------------------------------

const char *changeItem::Control1Name()
{
    (tmp = genString(item_window_name)) += ".control1";
    return tmp.str();
}

//----------------------------------------------------------------------------------

void changeItem::Open()
{
    framework.Tcl("pack %s -side top -fill x", item_window_name);
}

//----------------------------------------------------------------------------------

int changeItem::getStatus()
{
    return checked_value;
}

//----------------------------------------------------------------------------------

ChangeData *changeItem::getData()
{
    return callback_data;
}
