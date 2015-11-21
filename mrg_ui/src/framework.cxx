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
#include <cstring>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <ctype.h>
#include "framework.h"
#include "mtktextbuffer.h"
#include "diffmanager.h"
#include "difffile.h"
#include "mtkargs.h"
#include "genTmpfile.h"
#include "servercomm.h"
#include "machdep.h"
#include "stdlib.h"
#include "mtkchangeitem.h"
#include "general.h"
#include "mtkargs.h"


//----------------------------------------------------------------------------------

ChangeRegion::~ChangeRegion()
{
	if(undo_change)
		delete undo_change;
	if(new_region)
		delete new_region;
	if(original_region)
		delete original_region;
}

//----------------------------------------------------------------------------------

ChangeData::ChangeData(ChangeRegion *reg, int ch_num) :
	region(reg),
	change_number(ch_num)
{
}
	
//----------------------------------------------------------------------------------

void
MergeViewer::ProcessCommand(FrameWorkCommands cmd)
{
    switch(cmd){
    case CMD_NEXT_CHANGE:
	ShowNextChange();
	break;
    case CMD_PREV_CHANGE:
	ShowPrevChange();
	break;
    case CMD_NEXT_CONFLICT:
	ShowNextConflict();
	break;
    case CMD_PREV_CONFLICT:
	ShowPrevConflict();
	break;
    case CMD_CURR_CHANGE:
	ShowCurrentChange();
	break;
    case CMD_ACCEPT:
	AcceptChanges();
	break;
    case CMD_EDITOR:
	InvokeEditor();
	break;
    case CMD_CANCEL_MERGE:
	CancelMerge();
	break;
    }
}

//----------------------------------------------------------------------------------

void
MergeViewer::CancelMerge()
{
    RemoveChangeInfo(current_change_region);
    for(int i = 0; i < change_regions.size(); i++){
	ChangeRegion *chreg = *(ChangeRegion **)change_regions[i];
	delete chreg;
    }
    original_text.SetText("");
    modified_text.SetText("");
    delete diff_file;
    diff_file = NULL;
    Close();
}



//----------------------------------------------------------------------------------

int vsystem(char const*);	/* system() call using vfork. */

void
MergeViewer::AcceptChanges()
{
    gString new_file_name;
    gString new_file_command;
    genTmpfile tmp_file("mrgui");

    modified_text.SaveText((char *)tmp_file.name());
    if (diff_file != NULL) {
	if(diff_file->GetDeltaFileCount() == 1){
	    new_file_name = diff_file->GetDeltaFileName(0);
	} else {
	    diff_file->RemoveDeltaFiles();
	    new_file_command.printf("diff_new {%s}", (char *)diff_file->source_name);
	    int result = SendSyncCommand((char *)new_file_command, new_file_name);
	}
	gString diff_util = "mrg_diff ";
	if(strchr(diff_file->source_name.str(),' ')) diff_util += "\"";
	diff_util        += diff_file->source_name;
	if(strchr(diff_file->source_name.str(),' ')) diff_util += "\"";
	diff_util        += " ";
	if(strchr(tmp_file.name(),' ')) diff_util += "\"";
	diff_util        += tmp_file.name();
	if(strchr(tmp_file.name(),' ')) diff_util += "\"";
	diff_util        += " ";
	if(strchr(new_file_name.str(),' ')) diff_util += "\"";
	diff_util        += new_file_name;
	if(strchr(new_file_name.str(),' ')) diff_util += "\"";
	vsystem((char *)diff_util);
	RemoveChangeInfo(current_change_region);
	for(int i = 0; i < change_regions.size(); i++){
	    ChangeRegion *chreg = *(ChangeRegion **)change_regions[i];
	    delete chreg;
	}
	original_text.SetText("");
	modified_text.SetText("");
	delete diff_file;
	diff_file = NULL;
    }
    Close();
}

//----------------------------------------------------------------------------------

void
MergeViewer::InvokeEditor()
{
    gString editor = OSapi_getenv("EDITOR");
    if(editor.length() != 0){
	genTmpfile tmp_original("mrgorg");
	genTmpfile tmp_file("mrgui");
	genTmpfile change_file("diff");
	modified_text.SaveText((char *)tmp_original.name());
	modified_text.SaveText((char *)tmp_file.name());
	gString cmd;
	cmd.printf("%s %s", (char *)editor, tmp_file.name());
	vsystem((char *)cmd);
	
	// Execute diff between edited and saved version
	gString diff_util = "mrg_diff ";
	if(strchr(tmp_original.name(),' ')) diff_util += "\"";
	diff_util        += tmp_original.name();
	if(strchr(tmp_original.name(),' ')) diff_util += "\"";
	diff_util        += " ";
	if(strchr(tmp_file.name(),' ')) diff_util += "\"";
	diff_util        += tmp_file.name();
	if(strchr(tmp_file.name(),' ')) diff_util += "\"";
	diff_util        += " ";
	if(strchr(change_file.name(),' ')) diff_util += "\"";
	diff_util        += change_file.name();
	if(strchr(change_file.name(),' ')) diff_util += "\"";
	vsystem((char *)diff_util);

	// Load and apply changes
	diff_file = new DiffFile("tmp");
	diff_file->LoadChangeFile((char *)change_file.name());
	for(int i = diff_file->GetNumOfChanges() - 1; i >= 0; i--){
	    Change *ch = diff_file->GetChange(i);
	    int new_start, new_end;
	    int original_start, original_end;
	    modified_text.ApplyChange(ch, NULL, new_start, new_end, 
				       original_start, original_end);
	}
	delete diff_file;
	diff_file = NULL;
    }
}

//----------------------------------------------------------------------------------

void
MergeViewer::ShowSingleChange(Change *ch, ChangeData *cd)
{
	char const *operation_txt = "";
	switch(ch->operation){
	case DELETE:
		operation_txt = "Deleted text";
		break;
	case REPLACE:
		operation_txt = "Replacement text:";
		break;
	case INSERT:
		operation_txt = "Inserted text:";
		break;
	}
	AddChangeInfoWindow(operation_txt, (char *)ch->new_text, ch->applied, cd);
}

//----------------------------------------------------------------------------------

void
MergeViewer::RemoveChangeInfo(int change_num)
{
    if (   0 <= change_num && change_num < change_regions.size()) {
	ChangeRegion *chreg = *(ChangeRegion **)change_regions[change_num];
	original_text.UnHighliteRegion(chreg->original_region);
	modified_text.UnHighliteRegion(chreg->new_region);
	RemoveAllChangeInfoWindows();
    }
}

//----------------------------------------------------------------------------------

void
MergeViewer::ShowChangeInfo(int change_num)
{
	if (diff_file == NULL) {
		return;
	}
	ChangeRegion *chreg = *(ChangeRegion **)change_regions[change_num];
	original_text.HighliteRegion(chreg->original_region);
	original_text.FocusOnRegion(chreg->original_region);
	if(chreg->change->applied)
		modified_text.UnderlineRegion(chreg->new_region);
	modified_text.FocusOnRegion(chreg->new_region);

	gString info;
	if(chreg->conflict == NULL){
		ChangeData *cd = new ChangeData(chreg, chreg->change_number);
		ShowSingleChange(chreg->change, cd);
		info.printf("%d change(s). %d conflict(s). current change is %d.", diff_file->GetNumOfChanges(), diff_file->GetNumOfConflicts(), 
			     chreg->change_number + 1);
	} else {
		Conflict *cf = chreg->conflict;
		for(int i = cf->first_change; i < cf->first_change + cf->no_changes; i++){
			Change *ch     = diff_file->GetChange(i);
			ChangeData *cd = new ChangeData(chreg, i);
			ShowSingleChange(ch, cd);
		}
		DisableConflictedChanges(cf);
		info.printf("%d change(s). %d conflict(s). current conflict is %d.", diff_file->GetNumOfChanges(), diff_file->GetNumOfConflicts(), 
			     chreg->conflict_number);
	}
	SetInfoLabel((char *)info);
}

//----------------------------------------------------------------------------------

void
MergeViewer::ChangeSelected(ChangeData *cd, int state)
{
	ChangeRegion *region    = cd->region;
	Region *original_region = region->original_region;
	Region *new_region      = region->new_region;
	int original_offset     = original_region->GetOffset();
	int modified_offset     = new_region->GetOffset();
	int offset_diff         = modified_offset - original_offset;
	fprintf(stderr, "ChangeSelected called with state %d, original offset %d, modified offset %d, and offset diff %d\n", state, original_offset, modified_offset, offset_diff);

	if(state == 0){
		if(new_region->IsValid() == 0)
		    return;
		if(region->undo_change){
			Change *ch  = region->change;
			ch->applied = 0;
			ch          = region->undo_change;
			int new_start, new_end;
			int original_start, original_end;
			delete region->new_region;
			modified_text.ApplyChange(ch, NULL, new_start, new_end, 
				                       original_start, original_end, offset_diff);
			Region *new_reg    = new Region(new_start, new_end);
			modified_text.TrackRegion(new_reg);
			region->new_region = new_reg;
			if(region->conflict){
				DisableConflictedChanges(region->conflict);
				delete region->undo_change;
				region->undo_change = NULL;
			} 
		}
	} else {
	        if(new_region->IsValid() == 0)
		    return;
		Change *ch = diff_file->GetChange(cd->change_number);
		int new_start, new_end;
		int original_start, original_end;
		Change *undo = new Change;
		modified_text.ApplyChange(ch, undo, new_start, new_end, 
					   original_start, original_end, offset_diff);
		ch->applied = 1;
		if(region->conflict){
			DisableConflictedChanges(region->conflict);
			if(region->undo_change == NULL){
				delete region->new_region;
				Region *new_reg = new Region(new_start, new_end);
				modified_text.TrackRegion(new_reg);
				modified_text.UnderlineRegion(new_reg);
				region->new_region  = new_reg;
				region->undo_change = undo;
				region->change      = ch;
			}			
		} else {
			delete region->new_region;
			Region *new_reg = new Region(new_start, new_end);
			modified_text.TrackRegion(new_reg);
			modified_text.UnderlineRegion(new_reg);
			region->new_region = new_reg;
		}
	}
}

//----------------------------------------------------------------------------------

void
MergeViewer::DisableConflictedChanges(Conflict *cf)
{	
	int any_applied = 0;
	int i;
	for(i = 0; i < cf->no_changes && any_applied == 0; i++){
		Change *ch   = diff_file->GetChange(cf->first_change + i);
		any_applied |= ch->applied;
	}
	for(i = 0; i < cf->no_changes; i++){
		Change *ch = diff_file->GetChange(cf->first_change + i);
		EnableChangeInfoWindow(i, (ch->applied != 0) || !any_applied);
	}
}


//----------------------------------------------------------------------------------

void
MergeViewer::ShowCurrentChange()
{
	if(change_regions.size() != 0){
		ChangeRegion *chreg = *(ChangeRegion **)change_regions[current_change_region];
		original_text.FocusOnRegion(chreg->original_region);
		modified_text.FocusOnRegion(chreg->new_region);
	}
}

//----------------------------------------------------------------------------------

void
MergeViewer::FocusOnConflict(int number)
{
    int change_info_num = 0;
    while(change_info_num < change_regions.size()){
	ChangeRegion *chreg = *(ChangeRegion **)change_regions[change_info_num];
	if(chreg->conflict && chreg->conflict_number == number){
	    RemoveChangeInfo(current_change_region);
	    current_change_region = change_info_num;
	    ShowChangeInfo(current_change_region);
	    break;
	}
	change_info_num++;
    }
}

//----------------------------------------------------------------------------------

void
MergeViewer::ShowNextConflict()
{
	if(change_regions.size() != 0){
		int change_info_num = current_change_region + 1;
		while(change_info_num < change_regions.size()){
			ChangeRegion *chreg = *(ChangeRegion **)change_regions[change_info_num];
			if(chreg->conflict){
				RemoveChangeInfo(current_change_region);
				current_change_region = change_info_num;
				ShowChangeInfo(current_change_region);
				break;
			}
			change_info_num++;
		}
	}
}

//----------------------------------------------------------------------------------

void
MergeViewer::ShowPrevConflict()
{
	if(change_regions.size() != 0){
		int change_info_num = current_change_region - 1;
		while(change_info_num >= 0){
			ChangeRegion *chreg = *(ChangeRegion **)change_regions[change_info_num];
			if(chreg->conflict){
				RemoveChangeInfo(current_change_region);
				current_change_region = change_info_num;
				ShowChangeInfo(current_change_region);
				break;
			}
			change_info_num--;
		}
	}
}

//----------------------------------------------------------------------------------

void
MergeViewer::ShowNextChange()
{
	if(current_change_region < change_regions.size() - 1 && change_regions.size() != 0){
		RemoveChangeInfo(current_change_region);
		current_change_region++;
		ShowChangeInfo(current_change_region);
	}
}

//----------------------------------------------------------------------------------

void
MergeViewer::ShowPrevChange()
{
	if(current_change_region > 0 && change_regions.size() != 0){
		RemoveChangeInfo(current_change_region);
		current_change_region--;
		ShowChangeInfo(current_change_region);
	}
}

//----------------------------------------------------------------------------------

void
MergeViewer::ExitSelected()
{
    RemoveChangeInfo(current_change_region);
    for(int i = 0; i < change_regions.size(); i++){
	ChangeRegion *chreg = *(ChangeRegion **)change_regions[i];
	delete chreg;
    }
    original_text.SetText("");
    modified_text.SetText("");
    delete diff_file;
    diff_file = NULL;
}

//----------------------------------------------------------------------------------


static int showPrevConflict(ClientData data, Tcl_Interp *interp, int argc, char const *argv[])
{
    MergeViewer *viewer = (MergeViewer *)data;
    viewer->ShowPrevConflict();
    return TCL_OK;    
}
static int showPrevChange(ClientData data, Tcl_Interp *interp, int argc, char const *argv[])
{
    MergeViewer *viewer = (MergeViewer *)data;
    viewer->ShowPrevChange();
    return TCL_OK;    
}
static int showNextConflict(ClientData data, Tcl_Interp *interp, int argc, char const *argv[])
{
    MergeViewer *viewer = (MergeViewer *)data;
    viewer->ShowNextConflict();
    return TCL_OK;    
}
static int showNextChange(ClientData data, Tcl_Interp *interp, int argc, char const *argv[])
{
    MergeViewer *viewer = (MergeViewer *)data;
    viewer->ShowNextChange();
    return TCL_OK;    
}
static int showCurrentChange(ClientData data, Tcl_Interp *interp, int argc, char const *argv[])
{
    MergeViewer *viewer = (MergeViewer *)data;
    viewer->ShowCurrentChange();
    return TCL_OK;    
}
static int editor_cb(ClientData data, Tcl_Interp *interp, int argc, char const *argv[])
{
    MergeViewer *viewer = (MergeViewer *)data;
    viewer->InvokeEditor();
    return TCL_OK;    
}
static int accept_cb(ClientData data, Tcl_Interp *interp, int argc, char const *argv[])
{
    MergeViewer *viewer = (MergeViewer *)data;
    viewer->AcceptChanges();
    return TCL_OK;    
}

static int cancel_cb(ClientData data, Tcl_Interp *interp, int argc, char const *argv[])
{
    MergeViewer *viewer = (MergeViewer *)data;
    viewer->CancelMerge();
    return TCL_OK;    
}

static int apply_cb(ClientData data, Tcl_Interp *interp, int argc, char const *argv[])
{
    MergeViewer *viewer = (MergeViewer *)data;
    // Parse the argument list, which must have 1 integer.
    if (argc != 2) {
	interp->result = "wrong # args";
	return TCL_ERROR;
    }
    int item_id;
    if (Tcl_GetInt(interp, argv[1], &item_id) != TCL_OK) {
        return TCL_ERROR;
    }
    changeItem *item = *viewer->container[item_id];
    if (item == NULL) {
	return TCL_ERROR;
    }
    viewer->ChangeSelected(item->getData(), item->getStatus());
    return TCL_OK;    
}

static int focusOnConflict(ClientData data, Tcl_Interp *interp, int argc, char const *argv[])
{
    MergeViewer *viewer = (MergeViewer *)data;
    // Parse the argument list, which must have 1 integer.
    if (argc != 2) {
	interp->result = "wrong # args";
	return TCL_ERROR;
    }
    int num1;
    if (Tcl_GetInt(interp, argv[1], &num1) != TCL_OK) {
        return TCL_ERROR;
    }
    viewer->FocusOnConflict(num1);
    return TCL_OK;    
}


FrameWork::FrameWork(Tcl_Interp *i) :
	interp(i)
{
	trace_tcl = startup_args->isPresent("trace");
}

void
FrameWork::DisplayNotice(char const *text)
{
	genString cmd("tk_messageBox -icon info -type ok -title Notice -message ");
	cmd += quoteForTclString(text);
	Tcl_Eval(interp, (char *)cmd.str());
}

//----------------------------------------------------------------------------------
// Initialize Tcl commands and variables that reference C,
// most of which are callbacks.
//
void
MergeViewer::RegisterCallbacks() {
    Tcl_CreateCommand(framework.interp, "showPrevConflict", showPrevConflict, (ClientData)this, NULL);
    Tcl_CreateCommand(framework.interp, "showPrevChange", showPrevChange, (ClientData)this, NULL);
    Tcl_CreateCommand(framework.interp, "showNextConflict", showNextConflict, (ClientData)this, NULL);
    Tcl_CreateCommand(framework.interp, "showNextChange", showNextChange, (ClientData)this, NULL);
    Tcl_CreateCommand(framework.interp, "showCurrentChange", showCurrentChange, (ClientData)this, NULL);
    Tcl_CreateCommand(framework.interp, "invokeEditor", editor_cb, (ClientData)this, NULL);
    Tcl_CreateCommand(framework.interp, "acceptChanges", accept_cb, (ClientData)this, NULL);
    Tcl_CreateCommand(framework.interp, "cancelMerge", cancel_cb, (ClientData)this, NULL);
    Tcl_CreateCommand(framework.interp, "apply_cb", apply_cb, (ClientData)this, NULL);
    Tcl_CreateCommand(framework.interp, "focusOnConflict", focusOnConflict, (ClientData)this, NULL);
    Tcl_LinkVar(framework.interp, "conflictsOnly", (char *)&conflicts_only, TCL_LINK_BOOLEAN);
}

//----------------------------------------------------------------------------------
// The MktFrameWork is already constructed, and here we make it visible.
// Actually, this is where all of the Tk objects are constructed as well.
// Possibly, the Tk objects should be constructed earlier, and only
// pack them here for display.
//
void
MergeViewer::FabricateMainDialog()
{
    framework.Tcl("createMainDialog");

    (container_name       = dialogBase) += ".container";
    (original_text_driver = dialogBase) += ".original.text";
    (modified_text_driver = dialogBase) += ".modified.text";
    (source_file_label    = dialogBase) += ".buttonBar.sourceFileLabel";

    // original_text
    original_text.AttachDriver(original_text_driver);

    // modified_text
    modified_text.AttachDriver(modified_text_driver);

    (conflicts_only_toggle = dialogBase) += ".buttonBar.conflictsOnlyToggle";
    (info_label            = dialogBase) += ".infoBar.infoLabel";
}

//----------------------------------------------------------------------------------

MergeViewer::MergeViewer(FrameWork &fw, char const *toplevBaseName) :
	dialogBase(toplevBaseName),
	original_text(fw),
	modified_text(fw),
	diff_file(NULL),
	current_change_region(-1),
	framework(fw),
	conflicts_only(0)
{
	RegisterCallbacks();
	FabricateMainDialog();
}

MergeViewer::~MergeViewer()
{
    Tcl_DeleteCommand(framework.interp, "showPrevConflict");
    Tcl_DeleteCommand(framework.interp, "showPrevChange");
    Tcl_DeleteCommand(framework.interp, "showNextConflict");
    Tcl_DeleteCommand(framework.interp, "showNextChange");
    Tcl_DeleteCommand(framework.interp, "showCurrentChange");
    Tcl_DeleteCommand(framework.interp, "invokeEditor");
    Tcl_DeleteCommand(framework.interp, "acceptChanges");
    Tcl_DeleteCommand(framework.interp, "cancelMerge");
    Tcl_DeleteCommand(framework.interp, "apply_cb");
    Tcl_DeleteCommand(framework.interp, "focusOnConflict");
    Tcl_UnlinkVar(framework.interp, "conflictsOnly");
}


void
MergeViewer::Wait()
{
	// Wait until Close() called:
	// (Enter the domain of the TCL window event loop/callbacks/etc.)
	framework.Tcl("tkwait window %s", dialogBase.str());
}

void
MergeViewer::Close()
{
	// This will cause Wait() to return.
	// Destruction probably follows shortly.
	framework.Tcl("destroy %s", dialogBase.str());
}

//----------------------------------------------------------------------------------
//
// Quote the string so that it can be used in a Tcl list.
// The string may contain white space, backslash, etc.
//
genString
FrameWork::quoteForTclString(char const *txt)
{
	return quoteForTclString(interp, txt);
}

// static member function
genString
FrameWork::quoteForTclString(Tcl_Interp *interp, char const *txt)
{
    Tcl_ResetResult(interp);
    Tcl_AppendElement(interp, (char *)txt);
    genString tmp(interp->result);
    Tcl_ResetResult(interp);
    return tmp;
}



//----------------------------------------------------------------------------------

void
MergeViewer::SetSourceFileLabel(char const *txt)
{
    framework.Tcl("%s configure -text %s", source_file_label.str(),
                                 framework.quoteForTclString(txt).str());
}

//----------------------------------------------------------------------------------

void
MergeViewer::AddChangeInfoWindow(char const *operation_txt, char const *text, int state, ChangeData *cd)
{
	changeItem *item  = new changeItem(container_name,
					   container.size(), framework,
					   operation_txt, text, state,
					   cd);
	container.append(&item);
	item->Open();
}

//----------------------------------------------------------------------------------

void
MergeViewer::RemoveAllChangeInfoWindows(void)
{
    for(int i = container.size() - 1; i >= 0; i--){
	changeItem *item = *container[i];
	ChangeData *cd   = item->getData();
	if(cd)
	    delete cd;
	delete item;
    }
    container.reset();
}

//----------------------------------------------------------------------------------

void
MergeViewer::EnableChangeInfoWindow(int window, int flag)
{	
    changeItem *item = *container[window];
    item->EnableCheckBox(flag);
}

//----------------------------------------------------------------------------------

void
MergeViewer::SetInfoLabel(char const *txt)
{
    framework.Tcl("%s configure -text %s", info_label.str(),
                                 framework.quoteForTclString(txt).str());
}


//============================================================================
// Compose a command printf-like, and send it to the Tcl interpreter
// for evaluation.  Currently only %s and %d are supported, but others
// may work.
//
int
FrameWork::Tcl(char const *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    char const *p = fmt;
    long size = 0;
    while (*p != '\0') {
	if (*p == '%') {
	    if (p[1] == 'd') {
	        size += 20;
		int i = va_arg(args, int);
	    }
	    else if (p[1] == 's') {
		char *p = va_arg(args, char *);
		if (p != NULL) {
		    size += strlen(p);
		}
	    }
	    else {
		size += 20;
	    }
	    if (p[1] != '\0') {
		p += 1;
            }
	}
	else {
	    size += 1;
	}
	p += 1;
    }
    va_end(args);
    va_start(args, fmt);
    char *str = new char[size + 1];
    int written = vsprintf(str, fmt, args);
    va_end(args);
    if (written > size) {
        fprintf(stderr, "FrameWork::Tcl unexpected format \"%s\"\n", fmt);
	exit(1);
    }
    if (trace_tcl) {
	fprintf(stderr, "Tcl: %s\n", str);
    }
    int status = Tcl_Eval(interp, str);
    delete [] str;

    if (trace_tcl) {
	if (status != TCL_OK) {
	    fprintf(stderr, "    status: %d\n", status);
	}
	if (interp->result != NULL && strlen(interp->result) > 0) {
	    fprintf(stderr, "    result: %s\n", interp->result);
	}
    }
    else if (status != TCL_OK) {
	if (interp->result != NULL && strlen(interp->result) > 0) {
	    fprintf(stderr, "internal error calling Tcl: %s\n", interp->result);
	}
    }	
    return status;
}

//----------------------------------------------------------------------------------


void
MergeViewer::OpenForOperation(DiffFile *df, DiffOperation operation)
{
	diff_file = df;
	if (diff_file) {
		if(operation == DIFF_EDIT || operation == DIFF_VIEW || operation == DIFF_MERGE){
		        SetSourceFileLabel((char *)diff_file->source_name);
			original_text.LoadFile((char *)diff_file->source_name);
			modified_text.LoadFile((char *)diff_file->source_name);
			framework.Tcl("%s configure -state %s", conflicts_only_toggle.str(),
            			diff_file->TestConflicts() ? "normal" : "disabled");
			framework.Tcl("set %s %d", "conflictsOnly", diff_file->TestConflicts());
			diff_file->BuildConflictsList();
			change_regions.reset();
			ChangeRegionPtrArr tmp_change_regions;
			int conflict_number = 0;
			int i;
			for(i = diff_file->GetNumOfChanges(); i > 0; i--){
				Change *ch          = diff_file->GetChange(i - 1);
				ChangeRegion *chreg = new ChangeRegion;
				Conflict *cf;
				if((cf = diff_file->FindConflict(ch)) != NULL){
					i = cf->first_change + 1;
					int start, end;
					diff_file->GetConflictRange(cf, start, end);
					Region *original_reg = new Region(start, end);
					Region *new_reg      = new Region(start, end);
					modified_text.TrackRegion(new_reg);
					ChangeRegion *ch       = new ChangeRegion;
					chreg->change          = diff_file->GetChange(cf->first_change);
					chreg->change_number   = cf->first_change;
					chreg->conflict        = cf;
					chreg->conflict_number = ++conflict_number;

					chreg->new_region      = new_reg;
					chreg->undo_change     = NULL;
					chreg->original_region = original_reg;
				} else {
					int new_start, new_end;
					int original_start, original_end;
					Change *undo = new Change;
					modified_text.ApplyChange(ch, undo, new_start, new_end, 
					   	                       original_start, original_end);
					ch->applied          = 1;
					Region *original_reg = new Region(original_start, original_end);
					Region *new_reg      = new Region(new_start, new_end);
					modified_text.TrackRegion(new_reg);
//					modified_text.HighliteRegion(new_reg);
					chreg->change          = ch;
					chreg->change_number   = i - 1;
					chreg->conflict_number = 0;
					chreg->new_region      = new_reg;
					chreg->original_region = original_reg;
					chreg->undo_change     = undo;
					chreg->conflict        = NULL;
				}
				tmp_change_regions.append(&chreg);
			}
			current_change_region = 0;
			for(i = tmp_change_regions.size() - 1; i >= 0; i--)
				change_regions.append(tmp_change_regions[i]);
			if(change_regions.size() != 0){
				ShowChangeInfo(0);
			}
		}
		else if (operation == DIFF_NONE) {
			// User declined to choose an operation.
			exit(0);
		}
	}
}

