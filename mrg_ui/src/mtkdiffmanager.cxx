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
#include "difffile.h"
#include "mtkdiffmanager.h"
#include "framework.h"
#include "stdlib.h"

//============================================================================
DiffInfo *
MtkDiffManager::GetDiffFileAt(int row)
{
    DiffInfo *curr = *(DiffInfo **)current_diffs[row];
    return curr;
}

//============================================================================
int
MtkDiffManager::SelectAll()
{
	selected_files.reset();
	int count = 0;
        int total;
	if (framework.Tcl("%s index end", diff_list.str()) != TCL_OK) {
	    return 0;
	}
	if (Tcl_GetInt(framework.interp, framework.interp->result, &total) != TCL_OK) {
	    return 0;
	}
	while (count < total) {
		DiffInfo *f = GetDiffFileAt(count);
		selected_files.append(&f);
		count++;
	}
	return count;
}

//============================================================================
int
MtkDiffManager::BuildSelectionList(void)
{
	selected_files.reset();
	int count = 0;
	if (framework.Tcl("%s curselection", diff_list.str()) != TCL_OK) {
	    return 0;
	}
	// framework.interp->result is the list of integral selection indices.
	// Convert the TCL list to an argc/argv list.
	int listArgc;
	char const **listArgv;
	if (Tcl_SplitList(framework.interp, framework.interp->result, &listArgc, &listArgv) != TCL_OK) {
	    return 0;
	}
	for (int i = 0; i < listArgc; i += 1) {
	    int sel;
	    if (Tcl_GetInt(framework.interp, listArgv[i], &sel) == TCL_OK) {
		count++;
		DiffInfo *f = GetDiffFileAt(sel);
		selected_files.append(&f);
	    }
	}
	return count;
	Tcl_Free ((char *)listArgv);
}

//============================================================================
boolean
MtkDiffManager::SetupDialog(SrcInfoPtrArr& sources)
{
	if (   framework.Tcl("winfo exists %s", dialog.str()) == TCL_ERROR
            || strcmp(framework.interp->result, "0") != 0) {
            fprintf(stderr, "command '%s' failed or returned %s\n",
			dialog.str(), framework.interp->result);
	    return false;
	}

	framework.Tcl("createChooseDialog %s", dialog.str());

	selected_files.reset();

	// Populate the source list box.
	for(int i = 0; i < sources.size(); i++){
	    SrcInfo *si = *(SrcInfo **)sources[i];
            framework.Tcl("%s insert end %s",
                           (char *)src_list,
                           (char *)framework.quoteForTclString(si->name));
	}

	// Select the first source file.
	framework.Tcl("%s selection set 0 0", src_list.str());

	SourceSelectObserver();
	DiffSelectObserver();
	return true;
}


//============================================================================
// static member function
//
int
MtkDiffManager::SelectAllCmd(ClientData data, Tcl_Interp *interp, int argc, char const *argv[])
{
    MtkDiffManager *dialog = (MtkDiffManager *)data;
    if (argc != 1) {
        return TCL_ERROR;
    }
    int nsel = dialog->SelectAll();
    sprintf(interp->result, "%d", nsel);
    return TCL_OK;    
}

//============================================================================
// static member function
//
int
MtkDiffManager::BuildSelectionListCmd(ClientData data, Tcl_Interp *interp, int argc, char const *argv[])
{
    MtkDiffManager *dialog = (MtkDiffManager *)data;
    if (argc != 1) {
        return TCL_ERROR;
    }
    int nsel = dialog->BuildSelectionList();
    sprintf(interp->result, "%d", nsel);
    return TCL_OK;    
}

//============================================================================
// static member function (callback)
//
int
MtkDiffManager::SetDialogCommand(ClientData data, Tcl_Interp *interp, int argc, char const *argv[])
{
    DiffOperation operation = DIFF_NONE;
    if (argc != 2) {
        return TCL_ERROR;
    }
    if (strcmp(argv[1], "edit") == 0) {
	operation = DIFF_EDIT;
    }
    else if (strcmp(argv[1], "merge") == 0) {
	operation = DIFF_MERGE;
    }

    MtkDiffManager *dialog = (MtkDiffManager *)data;
    dialog->OpenMergeViewer(operation);
    return TCL_OK;    
}

DiffFile *
MtkDiffManager::ChooseOperation(DiffOperation &op)
{
	DiffFile *diff_file = NULL;

	if (selected_files.size() != 0) {
		char const * fname = (*(diffman.sources[current_src_selection]))->name;
		diff_file = new DiffFile(fname);
		for(int i = 0; i < selected_files.size(); ++i) {
			DiffInfo *diff = *(DiffInfo**)selected_files[i];
			diff_file->LoadChangeFile(diff->name);
		}
		diff_file->Sort();
		diff_file->RemoveSimilarChanges();
	}
	return diff_file;
}


void
MtkDiffManager::OpenMergeViewer(DiffOperation operation)
{
    // Used to be that setting the command was sufficient, but now we need
    // to create a new "main" dialog:
    MergeViewer mv(framework, ".main");
    DiffFile *df = ChooseOperation(operation);
    mv.OpenForOperation(df, operation);
    mv.Wait();
    // reset the source file (TCL lisboxes can only have one global selection):
    framework.Tcl("%s selection set %d", src_list.str(), current_src_selection);
}

//============================================================================
// static member function
//
int
MtkDiffManager::SourceObserverCmd(ClientData data, Tcl_Interp *interp, int argc, char const *argv[])
{
    MtkDiffManager *dialog = (MtkDiffManager *)data;
    if (argc != 1) {
        return TCL_ERROR;
    }
    dialog->SourceSelectObserver();
    return TCL_OK;    
}

//============================================================================
// static member function
//
int
MtkDiffManager::DiffObserverCmd(ClientData data, Tcl_Interp *interp, int argc, char const *argv[])
{
    MtkDiffManager *dialog = (MtkDiffManager *)data;
    if (argc != 1) {
        return TCL_ERROR;
    }
    dialog->DiffSelectObserver();
    return TCL_OK;    
}


//============================================================================
MtkDiffManager::MtkDiffManager(FrameWork &fw, DiffManager &dm)
    : framework(fw),
      dialog(".choose"),
      current_src_selection(-1),
      diffman(dm)
{
    change_info.printf("%s.info", dialog.str());
    diff_list.printf("%s.lists.changes.list", dialog.str());
    src_list.printf("%s.lists.source.list", dialog.str()),

    // Set up commands to support callback.
    Tcl_CreateCommand(framework.interp, "selectAll", SelectAllCmd, (ClientData)this, NULL);
    Tcl_CreateCommand(framework.interp, "buildSelectionList", BuildSelectionListCmd, (ClientData)this, NULL);
    Tcl_CreateCommand(framework.interp, "setDialogCommand", SetDialogCommand, (ClientData)this, NULL);
    Tcl_CreateCommand(framework.interp, "sourceSelectObserver", SourceObserverCmd, (ClientData)this, NULL);
    Tcl_CreateCommand(framework.interp, "diffSelectObserver", DiffObserverCmd, (ClientData)this, NULL);
}

//============================================================================
MtkDiffManager::~MtkDiffManager()
{
    // Remove commands to support callback.
    Tcl_DeleteCommand(framework.interp, "selectAll");
    Tcl_DeleteCommand(framework.interp, "buildSelectionList");
    Tcl_DeleteCommand(framework.interp, "setDialogCommand");
    Tcl_DeleteCommand(framework.interp, "sourceSelectObserver");
    Tcl_DeleteCommand(framework.interp, "diffSelectObserver");
}


//============================================================================
void
MtkDiffManager::OpenDiffManager()
{
    if (SetupDialog(diffman.sources)) {
	Wait();
    }
}

//============================================================================
void
MtkDiffManager::Wait()
{
    framework.Tcl("tkwait window %s", dialog.str());
}

//============================================================================
void
MtkDiffManager::SourceSelectObserver()
{
    // Track the selection, to avoid
    // blinking the diff list unnecessarily.
    // The source list is configured to only select one item at a time.
    if (framework.Tcl("%s curselection", src_list.str()) != TCL_OK) {
	return;
    }
    // framework.interp->result is the list of integral selection indices.
    // Convert the TCL list to an argc/argv list.
    int listArgc;
    char const **listArgv;
    if (Tcl_SplitList(framework.interp, framework.interp->result, &listArgc, &listArgv) != TCL_OK) {
	return;
    }
    int new_src_selection = -1;
    if (listArgc > 0) {
	int r = Tcl_GetInt(framework.interp, listArgv[0], &new_src_selection);
        Tcl_Free ((char *)listArgv);
        if (r != TCL_OK) {
	    return;
	}
    }
    if (new_src_selection == current_src_selection) {
        // The selection is unchanged.
	return;
    }

    // The source selection is changed; update the diff selection list.
    current_diffs.reset();

    // Remove all items from diff_list.
    framework.Tcl("%s delete 0 end", (char *)diff_list);

    SetChangeInfoText("");

    current_src_selection = new_src_selection;

    if (new_src_selection >= 0) {
	SrcInfo *src = *(diffman.sources[new_src_selection]);
	for(int i = 0; i < src->diffs.size(); i++){
	    DiffInfo *diff = *(DiffInfo **)src->diffs[i];
	    current_diffs.append(&diff);
	    framework.Tcl("%s insert end %s",
		           (char *)diff_list,
		           (char *)framework.quoteForTclString(diff->name));
	}
    }
}

//============================================================================
void
MtkDiffManager::DiffSelectObserver()
{
    if(current_diffs.size() == 0)
	return;

    if (framework.Tcl("%s curselection", diff_list.str()) != TCL_OK) {
	return;
    }
    // framework.interp->result is the list of integral selection indices.
    // Convert the TCL list to an argc/argv list.
    int listArgc;
    char const **listArgv;
    if (Tcl_SplitList(framework.interp, framework.interp->result, &listArgc, &listArgv) != TCL_OK) {
	return;
    }
    for (int i = 0; i < listArgc; i += 1) {
	int sel;
	if (Tcl_GetInt(framework.interp, listArgv[i], &sel) == TCL_OK) {
	    gString comment;
	    DiffInfo *diff = *(DiffInfo **)current_diffs[sel];
	    DiffFile::ReadComment((char *)diff->name, comment);
	    SetChangeInfoText(comment);
	    break;
	}
    }
    Tcl_Free ((char *)listArgv);
}

//============================================================================
void
MtkDiffManager::SetChangeInfoText(char const *txt)
{
    // check if info box is read only
    framework.Tcl("%s cget -state", change_info.str());
    boolean read_only = (strcmp(framework.interp->result, "disabled") == 0);
    if (read_only) { // if read only - set state to normal otherwise it'll not allow you to 
	             // change the text
      framework.Tcl("%s configure -state normal", change_info.str());
    }
    // Set the text of the change info box.
    framework.Tcl("%s delete 1.0 end", change_info.str());
    framework.Tcl("%s insert end %s",
                   change_info.str(),
                   framework.quoteForTclString(txt).str());
    if (read_only) { // restore info box state
      framework.Tcl("%s configure -state disabled", change_info.str());
    }
}
