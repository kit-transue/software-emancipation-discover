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
#ifndef _FRAMEWORK_H
#define _FRAMEWORK_H

#include "tcl.h"
#include "genArr.h"
#include "difffile.h"
#include "mtktextbuffer.h"
#include "diffmanager.h"

class MergeViewer;

enum FrameWorkCommands {
	CMD_CURR_CHANGE,
	CMD_NEXT_CHANGE,
	CMD_PREV_CHANGE,
	CMD_NEXT_CONFLICT,
	CMD_PREV_CONFLICT,
	CMD_ACCEPT,
	CMD_EDITOR,
	CMD_CUT_MENU,
	CMD_COPY_MENU,
	CMD_PASTE_MENU,
	CMD_CANCEL_MERGE,
	CMD_COUNT
};

struct ChangeRegion {
	~ChangeRegion();

	Conflict *conflict;
	Change   *change;
	Change   *undo_change;
	Region   *new_region;
	Region   *original_region;
	int      change_number;
        int      conflict_number;
};

typedef ChangeRegion *ChangeRegionPtr;

genArr(ChangeRegionPtr);

class ChangeData {
public:
	ChangeData(ChangeRegion *reg, int ch_num);

	ChangeRegion *region;
	int          change_number;
};


class changeItem;

typedef changeItem *changeItemPtr;
genArr(changeItemPtr);

class FrameWork {
public:
	FrameWork(Tcl_Interp *interp);
	void DisplayNotice(char const *);
	int Tcl(char const *, ...);
	static genString quoteForTclString(Tcl_Interp *interp, char const *txt);
	genString	 quoteForTclString(char const *txt);
	Tcl_Interp       *interp;
private:
	bool		 trace_tcl;
};


class MergeViewer {
public:
	MergeViewer(FrameWork &framework, char const *dialog_base_name);
	virtual ~MergeViewer();

	virtual void ProcessCommand(FrameWorkCommands cmd);
	virtual void FabricateMainDialog();
        virtual void ExitSelected();
        virtual void CancelMerge();
	virtual void AcceptChanges();
        virtual void InvokeEditor();
        virtual void FocusOnConflict(int number);
	virtual void ShowCurrentChange();
	virtual void ShowPrevChange();
	virtual void ShowNextChange();
	virtual void ShowNextConflict();
	virtual void ShowPrevConflict();
	virtual void ShowChangeInfo(int change_num);
	virtual void ShowSingleChange(Change *ch, ChangeData *cd);
	virtual void RemoveChangeInfo(int change_num);
	virtual void ChangeSelected(ChangeData *cd, int state);
	virtual void AddChangeInfoWindow(char const *operation_txt, char const *text, int state, ChangeData *cd);
	virtual void RemoveAllChangeInfoWindows();
	virtual void DisableConflictedChanges(Conflict *cf);
	virtual void EnableChangeInfoWindow(int window, int enable);
        virtual void SetSourceFileLabel(char const *txt);
        virtual void SetInfoLabel(char const *txt);
	virtual void OpenForOperation(DiffFile *diff_file, DiffOperation operation);

	void RegisterCallbacks();
	void Wait();
	void Close();

// DATA:
public:
	genArrOf(changeItemPtr) container;
private:
	genString        dialogBase;
	genString        original_text_driver;
	genString        modified_text_driver;
	genString        source_file_label;
	genString        container_name;
	genString        conflicts_only_toggle;
	genString        info_label;
	int              conflicts_only;
  
	ChangeRegionPtrArr change_regions;
	textBuffer         original_text;
	textBuffer         modified_text;
	DiffFile           *diff_file;
	int                current_change_region;
	FrameWork 	   &framework;
};

#endif
