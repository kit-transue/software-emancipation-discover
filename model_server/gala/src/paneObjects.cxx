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
#include <vport.h>
#include vlistitemHEADER
#include vdialogHEADER
#include vdragHEADER
#include vcontainerHEADER
#include vstdioHEADER
#include vstdlibHEADER
#include vapplicationHEADER
#include vliststrHEADER
#include vmenubarHEADER
#include vmenuHEADER
#include vapplicationHEADER
#include vcomboHEADER
#include vmemHEADER
#include vtimerHEADER
#include vtimestampHEADER


#include "gpaneObjects.h"
#include "gview.h"
#ifndef _generic_h
   #include "ggeneric.h"
#endif
#ifndef _gglobalFuncs_h
   #include "gglobalFuncs.h"
#endif
#ifndef _gString_h
   #include "gString.h"
#endif
#include "grtlClient.h"

class Viewer;


// definitions for overrides from vloadable
vkindTYPED_FULL_DEFINITION(glistItem, vlistitem, "glistItem");

int glistItem::UniqId_ = 0;  // Set the unique id system to zero

const int STRING_SIZE = 1024;
const unsigned long SECONDS = 1;
const unsigned long NANOSECONDS = 100000000; //  0; //100000000;   // half a second
const unsigned long FAST_SECONDS = 1;
const unsigned long FAST_NANOSECONDS = 0; //50000000;   // half a second
const unsigned long WAIT_SECONDS = 1; //1;
const unsigned long WAIT_NANOSECONDS = 0;   // half a second

const vchar* LINE_BREAK = (vchar*) "\n";
const vchar* UP = (vchar*) "up";
const vchar* DOWN = (vchar*) "down";
const vchar* UP_BUTTON = (vchar*) "up_button";
const vchar* DOWN_BUTTON = (vchar*) "down_button";

/*
This method initializes the item.  It tells the tcl interpreter to keep a variable of the
format [TAGNAME]_range to the id of the item, and then copies the tcl command to execute
upon selection.  It also sets itself to allow for multiple selection.
*/

void glistItem::Initialize(vchar* selectCmd, vchar* activateCmd, View* owner)
{
    id_ = UniqId_++;
    char buffer[1024];
    sprintf(buffer,"set %s_range %d",GetTag(), id_);
    gala_eval(owner->interp(),buffer);
    selectCmd_ = new vchar[ vcharLength(selectCmd) + 1];
    vcharCopy (selectCmd, selectCmd_);
    activateCmd_ = new vchar[ vcharLength(activateCmd) +1];
    vcharCopy (activateCmd, activateCmd_);
    owner_ = owner;
    SetSelectMethod(vlistFREE_SELECTION);
    SetClickFocusable(vFALSE);
}

/*
This method handles the actual selection of an object.  It gets the selection list
and creates an iterator to generate a list of numbers of the lines that have been
selected.  It tacks it onto the tcl command and then sends it to the interpreter
*/


void glistItem::ObserveSelect(int state)
{
    gInit (glistItem::ObserveSelect);

    if (state == vlistviewSELECTION_HAS_CHANGED) {

        int                     first = 1, total = 0;
        gString                 str;
        vlistSelection*         sel = GetSelection ();
        vlistIterator           iter;
	Tcl_Interp*		intr = 0;

	if (owner_)
	    intr = owner_->interp ();

        iter.StartWithSelectionRange (sel);

        while (iter.Next()) {

            int rows = iter.GetRangeRowCount ();
            if (rows < 1)
                continue;

            total += rows;

            if (!first)
                str += (vchar) ',';
            else first = 0;

            vrectLong   my_rect;
            gString     segment;

            iter.GetRange (&my_rect);
            if (rows == 1)
                segment.sprintf_scribed (vcharScribeLiteral("%d"), my_rect.y);
            else segment.sprintf_scribed (vcharScribeLiteral("%d-%d"),
                                            my_rect.y, my_rect.y + my_rect.h - 1);
            str += (vchar*) segment;
        }

        iter.Finish ();

	if (intr) {

	    Tcl_SetVar (intr, (char*) GetTag(),
				(char*) str.str(), TCL_GLOBAL_ONLY);

	    gString cmd;
	    gString base;
	    if (GetLastPointerClickCount() > 1)
	        base = activateCmd_;
	    else base = selectCmd_;
	    cmd.sprintf_scribed (vcharScribeLiteral("%s %s 0"), base.str(), str.str());

	    gala_eval (intr, (char*) cmd.str());
        }
    }
}


// definitions for overrides from vloadable
vkindTYPED_FULL_DEFINITION(gbutton, vbutton, "gbutton");

void gbutton::Initialize(vchar* activateCmd, View* owner)
{
    activateCmd_ = NULL;

    if (activateCmd) {
        activateCmd_ = new vchar[ vcharLength(activateCmd) +1];
        vcharCopy (activateCmd, activateCmd_);
	}
    owner_ = owner;
}


int gbutton::HandleButtonUp(vevent* event)
{
    int result = vbutton::HandleButtonUp(event);

    if (activateCmd_ && owner_)
	gala_eval(owner_->interp(), (char*)activateCmd_);

    return result;
}


// definitions for overrides from vloadable
vkindTYPED_FULL_DEFINITION(filterButton, gdButton, "filterButton");


void filterButton::SetDisplay(int select, int subset, int total)
{
    vchar buffer[STRING_SIZE];
    if (subset != -1)
        sprintf((char*)buffer,"%d of %d from %d", select, subset, total);
    else
        sprintf((char*)buffer,"%d of %d", select, total);
    SetTitle(buffer);
}

int filterButton::HandleButtonUp(vevent *event)
{
    rtlClient* lc = rtlClient::find (rtlclientid);
	if (lc) {
		char command[100];
        sprintf (command, "dis_setvar -global FilterRTLid %d;dis_launch_dialog {FilterList} -modal", lc->getServerId());                                                                                               
        SetTclCmd((unsigned char *)command);
	}

    return gdButton::HandleButtonUp(event);
}


vkindTYPED_FULL_DEFINITION(filterResetButton, gdButton, "filterResetButton");

int filterResetButton::HandleButtonUp(vevent *event)
{
    rtlClient* lc = rtlClient::find (rtlclientid);
	if (lc) {
		char command[100];
		sprintf (command, "reval_async {DISrtlFilter} [concat { dis_filterReset} %d]", lc->getServerId());                                                                                            
        SetTclCmd((unsigned char *)command);
	}

    return gdButton::HandleButtonUp(event);
}


vkindTYPED_FULL_DEFINITION(gmessageBox, vtextitem, "gmessageBox");

void gmessageBox::Open()
{
    vtextitem::Open();

    inQHelp_ = vFALSE;
    first_ = vTRUE;
    tclCmd_ = NULL;
    owner_ = NULL;
    text_ = GetTextData();
    selection_ = GetTextData()->CreateSelection();
    SetSelection(selection_);

    vcontainer* owner = vcontainer::CastDown(GetParentObject());
    if (owner) {
	gscrollButton* button = gscrollButton::CastDown(owner->FindItem(vnameInternGlobal((vchar*) UP_BUTTON)));
	if (button)
	    button->SetCommand((vchar*)UP, this);

	button = gscrollButton::CastDown(owner->FindItem(vnameInternGlobal((vchar*) DOWN_BUTTON)));
	if (button)
	    button->SetCommand((vchar*)DOWN, this);
    }
}


void gmessageBox::Initialize(vchar* tclCmd, View* owner)
{
    tclCmd_ = new vchar[vcharLength(tclCmd) + 1];
    vcharCopy(tclCmd, tclCmd_);

    owner_ = owner;
}



void gmessageBox::DisplayQuickHelp(vchar* message)
{
    SetModifyable(vTRUE);
    SetVertLocked(vFALSE);
    if (!inQHelp_)  {
	inQHelp_ = vTRUE;
	AddNewLine();
    }

    text_->SelectLines(selection_, text_->GetLineCount(), text_->GetLineCount(), vtextLINE_START);
    text_->InsertText(selection_, (const vchar*) message, vcharLength(message));
    Scroll(text_->GetLineCount(), 0);
    SetModifyable(vFALSE);
    SetVertLocked(vTRUE);
}


void gmessageBox::DisplayMessage(vchar* message)
{
    SetModifyable(vTRUE);
    SetVertLocked(vFALSE);
    if (*message == (vchar)'\001') {
	inQHelp_ = vTRUE;
        message = message + 1;
    }
    if (!inQHelp_) {
	AddNewLine();
    }
	inQHelp_ = vFALSE;
    text_->SelectLines(selection_, text_->GetLineCount(), text_->GetLineCount(), vtextLINE_START);
    text_->InsertText(selection_, (const vchar*) message, vcharLength(message));
    Scroll(text_->GetLineCount(), 0);
    SetModifyable(vFALSE);
    SetVertLocked(vTRUE);
}

void gmessageBox::AddNewLine()
{
    if (!first_) {
	text_->SelectRange(selection_, text_->GetLength(), text_->GetLength());
	text_->InsertText(selection_, LINE_BREAK, vcharLength(LINE_BREAK));
    }
    else first_ = vFALSE;
}		


void gmessageBox::ScrollMessageBox(vchar* direction)
{
    SetVertLocked(vFALSE);
    if (vcharCompare(direction, UP) == 0)
	PageScroll(vtextviewUP);
    else if (vcharCompare(direction, DOWN) == 0)
	PageScroll(vtextviewDOWN);
    SetVertLocked(vTRUE);
}


int gmessageBox::HandleButtonDown(vevent* event)
{
    int result = vtextitem::HandleButtonDown(event);

    if (event->GetClickCount() > 2)
	if (tclCmd_ && owner_)
	    gala_eval(owner_->interp(), (char*)tclCmd_);
    return result;
}
    





// definitions for overrides from vloadable
vkindTYPED_FULL_DEFINITION(gscrollButton, vbutton, "gscrollButton");


void gscrollButton::Open()
{
    vbutton::Open();

    command_ = NULL;
    owner_ = NULL;
    timer_ = NULL;
}


void gscrollButton::Destroy()
{
    delete command_;
}


void gscrollButton::SetCommand(vchar* command, gmessageBox* owner)
{
    command_ = new vchar [vcharLength(command)+1];
    vcharCopy(command, command_);

    owner_ = owner;
}


int gscrollButton::HandleButtonDown(vevent* event)
{
    int result = vbutton::HandleButtonDown(event);

    if (timer_)
	delete timer_;

    timer_ = new gscrollButtonTimer;
    timer_->SetOwner(this);
    timer_->Start();

    return result;
}


int gscrollButton::HandleButtonUp(vevent* event)
{
    int result = vbutton::HandleButtonUp(event);

    if (timer_) {
	timer_->Stop();
	delete timer_;
	timer_ = NULL;
    }

    return result;
}


void gscrollButton::gActivate()
{
    owner_->ScrollMessageBox(command_);
}




void gscrollButtonTimer::SetOwner(gscrollButton* owner)
{
    SetRecurrent();
    SetPeriod(SECONDS, NANOSECONDS);
    owner_ = owner;
    startTime_ = vtimestampGetNow();
    speedUpTime_ = vtimestampAdd(startTime_, WAIT_SECONDS, WAIT_NANOSECONDS);
    hasAccelerated_ = vFALSE;
}


void gscrollButtonTimer::ObserveTimer()
{
    if (!hasAccelerated_)
	if (vtimestampCompare(vtimestampGetNow(), speedUpTime_) >= 0)
	    SetPeriod(FAST_SECONDS, FAST_NANOSECONDS);
    owner_->gActivate();
}




