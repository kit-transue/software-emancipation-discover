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
#ifndef _MTKDIFFMANAGER_H
#define _MTKDIFFMANAGER_H

#include "diffmanager.h"
#include "tcl.h"
class FrameWork;


class MtkDiffManager {
public:
    static int SelectAllCmd(ClientData, Tcl_Interp *, int, char const *[]);
    static int BuildSelectionListCmd(ClientData, Tcl_Interp *, int, char const *[]);
    static int SetDialogCommand(ClientData, Tcl_Interp *, int, char const *[]);
    static int SourceObserverCmd(ClientData, Tcl_Interp *, int, char const *[]);
    static int DiffObserverCmd(ClientData, Tcl_Interp *, int, char const *[]);

public:
			MtkDiffManager(FrameWork &fw, DiffManager &dm);
	virtual		~MtkDiffManager();
	void            AddToSelected(DiffInfo *f);
	DiffInfo        *GetDiffFileAt(int row);
	int             SelectAll();
	int             BuildSelectionList(void);
	boolean         SetupDialog(SrcInfoPtrArr& sources);
	void		Wait();
	void		SetChangeInfoText(const char *);

	void OpenDiffManager();

	void SourceSelectObserver();
	void DiffSelectObserver();
	DiffFile *ChooseOperation(DiffOperation &op);

	void OpenMergeViewer(DiffOperation operation);

// DATA
private:
	genString       diff_list;//    SetupDialog, is the DiffList widget
	genString       src_list; //    SetupDialog, is the SrcList widget
        DiffInfoPtrArr  current_diffs; // tracked in SourceSelectObserver
	DiffInfoPtrArr	selected_files;
	int		current_src_selection;

	genString       change_info;//  the ChangeInfoText widget
	genString       dialog;

	FrameWork &framework;
	DiffManager &diffman;
};

#endif
