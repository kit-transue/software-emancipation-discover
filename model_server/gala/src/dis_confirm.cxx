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
/*********
*
* dis_confirm.cxx - dis/tcl commands for generic confirm dialogs.
*
* dis_confirm1 - One button confirm dialog.
* dis_confirm2 - Two button confirm dialog. Returns LeftButton or RightButton.
* dis_confirm3 - Three button confirm dialog.
*
* dis_confirm_list - Two button confirm dialog with a list of text items.
*
* dis_prompt_file - Brings up the file chooser.
* dis_prompt - Obsolete - Use dis_confirm[123] instead.
* dis_prompt_string1 - Brings up a one button dialog and waits for a string.
* dis_prompt_string2 - Brings up a two button dialog and waits for a string.
* dis_prompt_string3 - Brings up a three button dialog and waits for a string.
* 
* dis_prompt_bool1 - Brings up a one button dialog and waits for a yes-no choice
* dis_prompt_bool2 - Brings up a two button dialog and waits for a yes-no choice
* dis_prompt_bool3 - Brings up a three button dialog and waits for a yes-no choice
* dis_prompt_int[1-3] --brings up a [1-3]button dialog with a spinner and waits for a number.
**********/

#include <vport.h>
#include vcharHEADER
#include vstdioHEADER
#include vstdlibHEADER

#include <machdep.h>

#include <dis_confirm.h>
#include <gapl_menu.h>    // This has the TCL function externs!
#include <gfileChooser.h>
#include <ggeneric.h>
#include <gglobalFuncs.h>
#include <gString.h>
#include <gcontrolObjects.h>



// File static constants.

static const vchar* PROCEDURES             = (vchar*)".Procedures";



//-----------------------------------------------------------------------------
// dis_confirm1 Title BtnTitle Label
int dis_confirm1 (ClientData, Tcl_Interp* interp, int argc, char* argv[])
{
    gInit (dis_confirm1);
	
    if (argc != 4) {
		Tcl_SetResult (interp, "dis_confirm1 requires 3 arguments", TCL_STATIC);
		return TCL_ERROR;
    }
	
    vresource res;
    gString dialogName = "GenericConfirmDialogs.OneButtonConfirm";
    if (getResource((const vchar*) dialogName, &res))  {
		GConfirmDialog* myDialog = new GConfirmDialog(res);
		
		// Give the dialog box access to the view which created it.
		char* vr = Tcl_GetVar (interp, "vr", TCL_GLOBAL_ONLY);
		myDialog->SetInterpVariable((vchar*)"vr", (vchar*)vr);
		
		char* v  = Tcl_GetVar (interp, "v",  TCL_GLOBAL_ONLY);
		myDialog->SetInterpVariable((vchar*)"v", (vchar*)v);
		
        // Eval the procedures script for the dialog.
        gString procName = dialogName;
        procName += PROCEDURES;
        eval_StringResource(myDialog->GetInterpreter(), procName);
		
        // Parse the label into lines.
        gString labels[MAX_LABELS];
        int numLabels = get_labels((const vchar*) argv[3], labels);
		
        // Place the labels on the dialog.
        place_labels(myDialog, labels, numLabels);
		
        // Set the Dialog title.
        myDialog->SetTitle ((vchar *)argv[1]);
		
        // Set the Button title.
        Tcl_Interp* myInterp = myDialog->GetInterpreter();
        gdButton* myButton = (gdButton*) dis_findItem(myInterp,
			(vchar*) "ConfirmButton");
        if (myButton)
            myButton->SetTitle((const vchar*) argv[2]);
        else
            TCL_EXECERR1 ("button not found", "ConfirmButton")
			
#if defined(UNIX_OPEN_BUG_FIX)
			// We open and close the dialog here to force all of the
			// resize events. I don't know why it needs to be opened
			// to be resized and I don't know how to resize without
			// opening it.
			// Note - We use vwindow::Close because the default Close
			//        will also call vevent::StopProcessing.
			myDialog->Open();
		myDialog->vwindow::Close();
#endif
		
        // Move the Dialog in the middle of the screen and open it again.
		vwindow* display = vwindow::GetRoot();
		myDialog->Place(display, vrectPLACE_CENTER, vrectPLACE_CENTER);
		int nCurStyle=myDialog->GetStyle();
		nCurStyle=nCurStyle&~vwindowSTYLE_RESIZABLE;
		myDialog->SetStyle(nCurStyle);
		myDialog->Open();
		
		
        //  Go to the event loop and process events until a 
        //  vevent::StopProcessing call is made (dis_close_confirmDialog)
        //  This is what causes processing to wait for an answer.
        vevent::Process();
		
        char* returnValue = Tcl_GetVar (g_global_interp, "ConDStatus", TCL_GLOBAL_ONLY);
        Tcl_SetResult (interp, returnValue, TCL_VOLATILE);
		return TCL_OK;
    }
	
    printf("Error: '%s' folder not found in dictionary.\n", (char*) dialogName);
    return TCL_ERROR;
}


//-----------------------------------------------------------------------------
// dis_confirm2 Title LeftBtnTitle RightBtnTitle Label
int dis_confirm2 (ClientData, Tcl_Interp* interp, int argc, char* argv[])
{
    gInit (dis_confirm2);
	
    if (argc != 5) {
		Tcl_SetResult (interp, "dis_confirm2 requires 4 arguments", TCL_STATIC);
		return TCL_ERROR;
    }
	
    vresource res;
    gString dialogName = "GenericConfirmDialogs.TwoButtonConfirm";
    if (getResource((const vchar*) dialogName, &res))  {
		GConfirmDialog* myDialog = new GConfirmDialog(res);
		
		// Give the dialog box access to the view which created it.
		char* vr = Tcl_GetVar (interp, "vr", TCL_GLOBAL_ONLY);
		myDialog->SetInterpVariable((vchar*)"vr", (vchar*)vr);
		
		char* v  = Tcl_GetVar (interp, "v",  TCL_GLOBAL_ONLY);
		myDialog->SetInterpVariable((vchar*)"v", (vchar*)v);
		
        // Eval the procedures script for the dialog.
        gString procName = dialogName;
        procName += PROCEDURES;
        eval_StringResource(myDialog->GetInterpreter(), procName);
		
        // Parse the label into lines.
        gString labels[MAX_LABELS];
        int numLabels = get_labels((const vchar*) argv[4], labels);
		
        // Place the labels on the dialog.
        place_labels(myDialog, labels, numLabels);
		
        // Set the Dialog title.
        myDialog->SetTitle ((vchar *)argv[1]);
		
        // Set the Left button title.
        Tcl_Interp* myInterp = myDialog->GetInterpreter();
        gdButton* myButton = (gdButton*) dis_findItem(myInterp,
			(vchar*) "LeftButton");
        if (myButton)
            myButton->SetTitle((const vchar*) argv[2]);
        else
            TCL_EXECERR1 ("button not found", "LeftButton")
			
			// Set the Right button title.
			myButton = (gdButton*) dis_findItem(myInterp, (vchar*) "RightButton");
        if (myButton)
            myButton->SetTitle((const vchar*) argv[3]);
        else
            TCL_EXECERR1 ("button not found", "RightButton")
			
#if defined(UNIX_OPEN_BUG_FIX)
			// We open and close the dialog here to force all of the
			// resize events. I don't know why it needs to be opened
			// to be resized and I don't know how to resize without
			// opening it.
			// Note - We use vwindow::Close because the default Close
			//        will also call vevent::StopProcessing.
			myDialog->Open();
		myDialog->vwindow::Close();
#endif
		
        // Move the Dialog in the middle of the screen and open it again.
		vwindow* display = vwindow::GetRoot();
		myDialog->Place(display, vrectPLACE_CENTER, vrectPLACE_CENTER);
		int nCurStyle=myDialog->GetStyle();
		nCurStyle=nCurStyle&~vwindowSTYLE_RESIZABLE;
		myDialog->SetStyle(nCurStyle);
		myDialog->Open();
		
		
        //  Go to the event loop and process events until a 
        //  vevent::StopProcessing call is made (dis_close_confirmDialog)
        //  This is what causes processing to wait for an answer.
        vevent::Process();
		
        char* returnValue = Tcl_GetVar (g_global_interp, "ConDStatus", TCL_GLOBAL_ONLY);
        Tcl_SetResult (interp, returnValue, TCL_VOLATILE);
		return TCL_OK;
    }
	
    printf("Error: '%s' folder not found in dictionary.\n", (char*) dialogName);
    return TCL_ERROR;
}


//-----------------------------------------------------------------------------
// dis_confirm3 Title LeftBtnTitle MiddleBtnTitle RightBtnTitle Label
int dis_confirm3 (ClientData, Tcl_Interp* interp, int argc, char* argv[])
{
    gInit (dis_confirm3);
	
    if (argc != 6) {
		Tcl_SetResult (interp, "dis_confirm3 requires 5 arguments", TCL_STATIC);
		return TCL_ERROR;
    }
	
    vresource res;
    gString dialogName = "GenericConfirmDialogs.ThreeButtonConfirm";
    if (getResource((const vchar*) dialogName, &res))  {
		GConfirmDialog* myDialog = new GConfirmDialog(res);
		
		// Give the dialog box access to the view which created it.
		char* vr = Tcl_GetVar (interp, "vr", TCL_GLOBAL_ONLY);
		myDialog->SetInterpVariable((vchar*)"vr", (vchar*)vr);
		
		char* v  = Tcl_GetVar (interp, "v",  TCL_GLOBAL_ONLY);
		myDialog->SetInterpVariable((vchar*)"v", (vchar*)v);
		
        // Eval the procedures script for the dialog.
        gString procName = dialogName;
        procName += PROCEDURES;
        eval_StringResource(myDialog->GetInterpreter(), procName);
		
        // Parse the label into lines.
        gString labels[MAX_LABELS];
        int numLabels = get_labels((const vchar*) argv[5], labels);
		
        // Place the labels on the dialog.
        place_labels(myDialog, labels, numLabels);
		
        // Set the Dialog title.
        myDialog->SetTitle ((vchar *)argv[1]);
		
        // Set the Left button title.
        Tcl_Interp* myInterp = myDialog->GetInterpreter();
        gdButton* myButton = (gdButton*) dis_findItem (myInterp,
			(vchar*) "LeftButton");
        if (myButton)
            myButton->SetTitle((const vchar*) argv[2]);
        else
            TCL_EXECERR1 ("button not found", "LeftButton")
			
			// Set the Middle button title.
			myButton = (gdButton*) dis_findItem(myInterp, (vchar*) "MiddleButton");
        if (myButton)
            myButton->SetTitle((const vchar*) argv[3]);
        else
            TCL_EXECERR1 ("button not found", "MiddleButton")
			
			// Set the Right button title.
			myButton = (gdButton*) dis_findItem(myInterp, (vchar*) "RightButton");
        if (myButton)
            myButton->SetTitle((const vchar*) argv[4]);
        else
            TCL_EXECERR1 ("button not found", "RightButton")
			
#if defined(UNIX_OPEN_BUG_FIX)
			// We open and close the dialog here to force all of the
			// resize events. I don't know why it needs to be opened
			// to be resized and I don't know how to resize without
			// opening it.
			// Note - We use vwindow::Close because the default Close
			//        will also call vevent::StopProcessing.
			myDialog->Open();
		myDialog->vwindow::Close();
#endif
		
        // Move the Dialog in the middle of the screen and open it again.
		vwindow* display = vwindow::GetRoot();
		myDialog->Place(display, vrectPLACE_CENTER, vrectPLACE_CENTER);
		int nCurStyle=myDialog->GetStyle();
		nCurStyle=nCurStyle&~vwindowSTYLE_RESIZABLE;
		myDialog->SetStyle(nCurStyle);
		myDialog->Open();
		
		
        //  Go to the event loop and process events until a 
        //  vevent::StopProcessing call is made (dis_close_confirmDialog)
        //  This is what causes processing to wait for an answer.
        vevent::Process();
		
        char* returnValue = Tcl_GetVar (g_global_interp, "ConDStatus", TCL_GLOBAL_ONLY);
        Tcl_SetResult (interp, returnValue, TCL_VOLATILE);
		return TCL_OK;
    }
	
    printf("Error: '%s' folder not found in dictionary.\n", (char*) dialogName);
    return TCL_ERROR;
}

//-----------------------------------------------------------------------------
// dis_confirm_list Title LeftBtnTitle RightBtnTitle Label {List}
int dis_confirm_list (ClientData, Tcl_Interp* interp, int argc, char* argv[])
{
    gInit (dis_confirm_list);
	
    if (argc != 6) {
		Tcl_SetResult (interp, "dis_confirm_list requires 5 arguments", TCL_STATIC);
		return TCL_ERROR;
    }
	
    vresource res;
    gString dialogName = "GenericConfirmDialogs.ConfirmList";
    if (getResource((const vchar*) dialogName, &res))  {
		GConfirmDialog* myDialog = new GConfirmDialog(res);
		
		// Give the dialog box access to the view which created it.
		char* vr = Tcl_GetVar (interp, "vr", TCL_GLOBAL_ONLY);
		myDialog->SetInterpVariable((vchar*)"vr", (vchar*)vr);
		
		char* v  = Tcl_GetVar (interp, "v",  TCL_GLOBAL_ONLY);
		myDialog->SetInterpVariable((vchar*)"v", (vchar*)v);
		
#if defined(UNIX_OPEN_BUG_FIX)
        // Note - We need to open the dialog before we initialize it or else
        //        the list items will not be processed properly.
        // We open and close the dialog here to force all of the
        // resize events. I don't know why it needs to be opened
        // to be resized and I don't know how to resize without
        // opening it.
        // Note - We use vwindow::Close because the default Close
        //        will also call vevent::StopProcessing.
		myDialog->Open();
		myDialog->vwindow::Close();
#endif
		
        // Move the Dialog in the middle of the screen and open it again.
		vwindow* display = vwindow::GetRoot();
		myDialog->Place(display, vrectPLACE_CENTER, vrectPLACE_CENTER);
		int nCurStyle=myDialog->GetStyle();
		nCurStyle=nCurStyle&~vwindowSTYLE_RESIZABLE;
		myDialog->SetStyle(nCurStyle);
		myDialog->Open();
		
        // Parse the label into lines.
        gString labels[MAX_LABELS];
        int numLabels = get_labels((const vchar*) argv[4], labels);
		
        // Place the labels on the dialog.
        place_labels(myDialog, labels, numLabels);
		
        // Set the Dialog title.
        myDialog->SetTitle ((vchar *)argv[1]);
		
        // Set the Left button title.
        Tcl_Interp* myInterp = myDialog->GetInterpreter();
        gdButton* myButton = (gdButton*) dis_findItem(myInterp,
			(vchar*) "LeftButton");
        if (myButton)
            myButton->SetTitle((const vchar*) argv[2]);
        else
            TCL_EXECERR1 ("button not found", "LeftButton")
			
			// Set the Right button title.
			myButton = (gdButton*) dis_findItem(myInterp, (vchar*) "RightButton");
        if (myButton)
            myButton->SetTitle((const vchar*) argv[3]);
        else
            TCL_EXECERR1 ("button not found", "RightButton")
			
			// Eval the procedures script for the dialog.
			gString procName = dialogName;
        procName += PROCEDURES;
        eval_StringResource(myDialog->GetInterpreter(), procName);
		
        // Set the data for the list item.
        myDialog->SetInterpVariable((vchar*)"ConfirmListData", (vchar*)argv[5]);
		
		
        //  Go to the event loop and process events until a 
        //  vevent::StopProcessing call is made (dis_close_confirmDialog)
        //  This is what causes processing to wait for an answer.
        vevent::Process();
		
        char* returnValue = Tcl_GetVar (g_global_interp, "ConDStatus", TCL_GLOBAL_ONLY);
		
        Tcl_SetResult (interp, returnValue, TCL_VOLATILE);
        return TCL_OK;
    }
	
    printf("Error: '%s' folder not found in dictionary.\n", (char*) dialogName);
    return TCL_ERROR;
}



//-----------------------------------------------------------------------------

int dis_close_confirmDialog (ClientData, Tcl_Interp* interp, int, char**)
{
    gInit (dis_close_confirmDialog);
	
    char* gd = Tcl_GetVar (interp, "gd", TCL_GLOBAL_ONLY);
    if (gd) {
        int gdialog_id = atoi (gd);
		
        GDialog* gd = GDialog::FindGDialog(gdialog_id);
        if (!gd)
			return TCL_ERROR;
		else  {
			gd->CloseDialog();
			gd->DeleteLater();
		}
    }
    return TCL_OK;
}



// Parse the input string into 1-3 label lines.
int get_labels(const vchar* inStr, gString labels[MAX_LABELS])
{
    int numFound = 0;
	
    vstr* allLabels = vstrClone(inStr);
	
    const vchar* seperators = vnameInternGlobalLiteral("\n");
	
    vchar* nextLine = vcharGetFirstToken(allLabels, seperators);
    while (nextLine && (numFound < MAX_LABELS) ) {
        labels[numFound] = nextLine;
        numFound++;
        nextLine = vcharGetNextToken(seperators);
    }
	
    vstrDestroy(allLabels);
	
    return(numFound);
}



// Place the labels on the dialog.
vbool place_labels(GConfirmDialog* myDialog, gString labels[MAX_LABELS], int numLabels)
{
    vbool status = vTRUE;
	
    // Place up to 3 labels.
    switch (numLabels) {
	case 1:
		myDialog->SetInterpVariable((vchar*)"Label2", labels[0]);
		break;
	case 2:
		myDialog->SetInterpVariable((vchar*)"Label1", labels[0]);
		myDialog->SetInterpVariable((vchar*)"Label2", labels[1]);
		break;
	case 3:
		myDialog->SetInterpVariable((vchar*)"Label1", labels[0]);
		myDialog->SetInterpVariable((vchar*)"Label2", labels[1]);
		myDialog->SetInterpVariable((vchar*)"Label3", labels[2]);
		break;
	default:
		printf("Trying to place %d labels when only 1-%d are allowed!\n",
			numLabels, MAX_LABELS);
		break;
    };
	
    return(status);
}




//-----------------------------------------------------------------------------

int dis_prompt_file (ClientData, Tcl_Interp* interp, int argc, char* argv[])
{
	int error = 0;
	
    if (argc < 2)
		TCL_EXECERR("wrong number of arguments")
		
		char* vr = Tcl_GetVar (interp, "vr", TCL_GLOBAL_ONLY);
    char* v  = Tcl_GetVar (interp, "v", TCL_GLOBAL_ONLY);
	
    if (v && vr) {
        int viewer_id = atoi (vr);
        int view_id = atoi (v);
		vchar defaultPath[1024], newFilter[1024], defaultFilter[1024];
        vchar *newTitle, *newPrompt;
		
		vcharCopy((const vchar*)argv[1], defaultPath);
		vcharCopy((const vchar*)argv[1], defaultFilter);
        vcharCopy((const vchar*)argv[1], newFilter);
		vcharAppend(defaultPath, (const vchar*)"(path)");
        vcharAppend(newFilter, (const vchar*)"(filter)");
		vcharAppend(defaultFilter, (const vchar*)"(defaultFilter)");
		
        fileChooser* newChooser = new fileChooser;
		
        if (argc > 3) newTitle = (vchar *)argv[3];
        else newTitle = NULL;
		
        if (argc > 4) newPrompt = (vchar *)argv[4];
        else newPrompt = NULL;
		
		int multipleSelections = 0;
		
		if (vcharCompare((vchar*)argv[2],"-new") == 0)
			newChooser->SetType(vfilechsrPUT);
		else if (vcharCompare((vchar*)argv[2],"-new0") == 0)
		{
			newChooser->SetType(vfilechsrPUT);
			newChooser->SetConfirmOverwrite(0);
		}
		else {
			newChooser->SetType(vfilechsrGET);
			if (vcharCompare((vchar*)argv[2],"-multiple") == 0)
				multipleSelections = 1;
		}
		
		newChooser->SetCallers(viewer_id, 
			view_id, 
			newTitle, 
			newPrompt, 
			defaultPath, 
			newFilter,
			defaultFilter,
			multipleSelections);
		
		
		(void)newChooser->Process();
		
		
        if (newChooser->GetResult()) {
            Tcl_SetResult (interp, (char *)newChooser->GetResult(), TCL_VOLATILE);
			
            vchar *newPath = newChooser->GetDirectory()->CloneString ();
			Tcl_SetVar (g_global_interp, 
				(char *)defaultPath, 
				(char*)newPath, 
				TCL_GLOBAL_ONLY);
            if (newPath) vstrDestroy (newPath);
			
#if 0
            const vchar *temp = (const vchar*)newChooser->GetActiveFilter();
			if (temp) {
				printf ("%s\n", temp);
				Tcl_SetVar (g_global_interp, 
					(char*)defaultFilter, 
					(char*)temp, 
					TCL_GLOBAL_ONLY);
			}
#endif
		} else error = 1;
		
        delete newChooser;
    }
    
    if (error) return TCL_ERROR;
	
	return TCL_OK;
}


//-----------------------------------------------------------------------------
// This function should be obsoleted in the future. Use dis_confirm instead.

int dis_prompt (ClientData, Tcl_Interp* interp, int argc, char* argv[])
{
    gInit (dis_prompt);
	
    if (argc < 3 || argc > 6) {
		Tcl_SetResult (interp, "wrong number of arguments", TCL_STATIC);
		return TCL_ERROR;
    }
	
    vresource res;
    gString dialogName = "GenericConfirmDialogs.";
    dialogName += argv[1];
    if (getResource((const vchar*) dialogName, &res))  {
		GConfirmDialog* myDialog = new GConfirmDialog(res);
		
		// Give the dialog box access to the view which created it.
		char* vr = Tcl_GetVar (interp, "vr", TCL_GLOBAL_ONLY);
		myDialog->SetInterpVariable((vchar*)"vr", (vchar*)vr);
		
		char* v  = Tcl_GetVar (interp, "v",  TCL_GLOBAL_ONLY);
		myDialog->SetInterpVariable((vchar*)"v", (vchar*)v);
		
		Tcl_Interp *myInterp = myDialog->GetInterpreter();
		
		if (argc > 3) 
		{
			
			vtextitem *label = vtextitem::CastDown((vdialogItem *)dis_findItem(myInterp,
				(vchar*) "prompt"));
			if( label )
				label->SetText((vchar *)argv[3]);	
			
		}
		
        // Move the Dialog in the middle of the screen and open it.
		vwindow* display = vwindow::GetRoot();
		myDialog->Place(display, vrectPLACE_CENTER, vrectPLACE_CENTER);
		myDialog->Open();
		
        myDialog->SetTitle ((vchar *)argv[2]);
		
		
		
		if (argc > 4) {
            gdButton *myButton = (gdButton*) dis_findItem(myInterp,
				(vchar*) "button1");
            if (myButton)
                myButton->SetTitle((const vchar*) argv[4]);
            else
                TCL_EXECERR1 ("button not found", "button1")
        }
		
		if (argc > 5) {
            gdButton* myButton = (gdButton*) dis_findItem(myInterp,
				(vchar*) "button2");
            if (myButton)
                myButton->SetTitle((const vchar*) argv[5]);
            else
                TCL_EXECERR1 ("button not found", "button2")
		}
		
		
        // Eval the procedures script for the dialog.
        gString procName = dialogName;
        procName += PROCEDURES;
        eval_StringResource(myDialog->GetInterpreter(), procName);
		
        //  Go to the event loop and process events until a 
        //  vevent::StopProcessing call is made (dis_close_confirmDialog)
        //  This is what causes processing to wait for an answer.
        vwindow::FlushRequests();
        vevent::Process ();
		
        char* returnValue = Tcl_GetVar (g_global_interp, "ConDStatus", TCL_GLOBAL_ONLY);
        Tcl_SetResult (interp, returnValue, TCL_VOLATILE);
		return TCL_OK;
    }
	
    printf("Error: dialog %s not found in dictionary.\n", (char*) dialogName);
    return TCL_ERROR;
}




//-----------------------------------------------------------------------------
// dis_prompt_string1 Title OK_Button Value Prompt
int dis_prompt_string1 (ClientData, Tcl_Interp* interp, int argc, char* argv[])
{
    gInit (dis_prompt_string1);
	
    if (argc < 4) {
		Tcl_SetResult(interp, "dis_prompt_string1 requires 3 arguments", TCL_STATIC);
		return TCL_ERROR;
    }
	
    vresource res;
    gString dialogName = "GenericConfirmDialogs.OneButtonStringPrompt";
    if (getResource((const vchar*) dialogName, &res))  {
		GConfirmDialog* myDialog = new GConfirmDialog(res);
		
		// Give the dialog box access to the view which created it.
		char* vr = Tcl_GetVar (interp, "vr", TCL_GLOBAL_ONLY);
		myDialog->SetInterpVariable((vchar*)"vr", (vchar*)vr);
		
		char* v  = Tcl_GetVar (interp, "v",  TCL_GLOBAL_ONLY);
		myDialog->SetInterpVariable((vchar*)"v", (vchar*)v);
		
        // Eval the procedures script for the dialog.
        gString procName = dialogName;
        procName += PROCEDURES;
        eval_StringResource(myDialog->GetInterpreter(), procName);
		
        // Parse the prompt into lines.
        gString labels[MAX_LABELS];
        int numLabels = get_labels((const vchar*) argv[3], labels);
		
        // Place the labels on the dialog.
        place_labels(myDialog, labels, numLabels);
		
        // Set the Dialog title.
        myDialog->SetTitle ((vchar*) argv[1]);
		
        // Set the Button title.
		gString btn_title = argv[2];
		if (btn_title.length() == 0)
			btn_title = "OK";
        Tcl_Interp* myInterp = myDialog->GetInterpreter();
        gdButton* myButton = (gdButton*) dis_findItem(myInterp,
			(vchar*) "OKButton");
        if (myButton)
            myButton->SetTitle(btn_title);
        else
            TCL_EXECERR1 ("button not found", "OKButton")
			
			// Set the inital value of the text, if given.
			if (argc > 4)
			myDialog->SetInterpVariable((vchar*)"InputString",
			(vchar*) argv[4]);
		
#if defined(UNIX_OPEN_BUG_FIX)
        // We open and close the dialog here to force all of the
        // resize events. I don't know why it needs to be opened
        // to be resized and I don't know how to resize without
        // opening it.
        // Note - We use vwindow::Close because the default Close
        //        will also call vevent::StopProcessing.
		myDialog->Open();
		myDialog->vwindow::Close();
#endif
		
        // Move the Dialog in the middle of the screen and open it again.
		vwindow* display = vwindow::GetRoot();
		myDialog->Place(display, vrectPLACE_CENTER, vrectPLACE_CENTER);
		int nCurStyle=myDialog->GetStyle();
		nCurStyle=nCurStyle&~vwindowSTYLE_RESIZABLE;
		myDialog->SetStyle(nCurStyle);
		myDialog->Open();
		
		
        //  Go to the event loop and process events until a 
        //  vevent::StopProcessing call is made (dis_close_confirmDialog)
        //  This is what causes processing to wait for an answer.
        vevent::Process();
		
        char* returnValue = Tcl_GetVar(g_global_interp, "PromptResult", TCL_GLOBAL_ONLY);
        Tcl_SetResult (interp, returnValue, TCL_VOLATILE);
		return TCL_OK;
    }
	
    printf("Error: '%s' folder not found in dictionary.\n", (char*) dialogName);
    return TCL_ERROR;
}


//-----------------------------------------------------------------------------
// dis_prompt_string2 Title Left_Button Right_Button Value Prompt
int dis_prompt_string2 (ClientData, Tcl_Interp* interp, int argc, char* argv[])
{
    gInit (dis_prompt_string2);
	
    if (argc < 5) {
		Tcl_SetResult(interp, "dis_prompt_string2 requires 4 arguments", TCL_STATIC);
		return TCL_ERROR;
    }
	
    vresource res;
    gString dialogName = "GenericConfirmDialogs.TwoButtonStringPrompt";
    if (getResource((const vchar*) dialogName, &res))  {
		GConfirmDialog* myDialog = new GConfirmDialog(res);
		
		// Give the dialog box access to the view which created it.
		char* vr = Tcl_GetVar (interp, "vr", TCL_GLOBAL_ONLY);
		myDialog->SetInterpVariable((vchar*)"vr", (vchar*)vr);
		
		char* v  = Tcl_GetVar (interp, "v",  TCL_GLOBAL_ONLY);
		myDialog->SetInterpVariable((vchar*)"v", (vchar*)v);
		
        // Eval the procedures script for the dialog.
        gString procName = dialogName;
        procName += PROCEDURES;
        eval_StringResource(myDialog->GetInterpreter(), procName);
		
        // Parse the prompt into lines.
        gString labels[MAX_LABELS];
        int numLabels = get_labels((const vchar*) argv[4], labels);
		
        // Place the labels on the dialog.
        place_labels(myDialog, labels, numLabels);
		
        // Set the Dialog title.
        myDialog->SetTitle ((vchar*) argv[1]);
		
        // Set the Left Button title.
        Tcl_Interp* myInterp = myDialog->GetInterpreter();
        gdButton* myButton = (gdButton*) dis_findItem(myInterp,
			(vchar*) "LeftButton");
        if (myButton) {
			gString btn_title = argv[2];
			if (btn_title.length() == 0)
				btn_title = "OK";
			myButton->SetTitle(btn_title);
        } else
            TCL_EXECERR1 ("button not found", "LeftButton")
			
			// Set the Right Button title.
			myButton = (gdButton*) dis_findItem(myInterp,
			(vchar*) "RightButton");
        if (myButton) {
			gString btn_title = argv[3];
			if (btn_title.length() == 0)
				btn_title = "Cancel";
			myButton->SetTitle(btn_title);
        } else
            TCL_EXECERR1 ("button not found", "RightButton")
			
			// Set the inital value of the text, if given.
			if (argc > 5) {
			myDialog->SetInterpVariable((vchar*)"InputString",
				(vchar*) argv[5]);
		} 
		
#if defined(UNIX_OPEN_BUG_FIX)
        // We open and close the dialog here to force all of the
        // resize events. I don't know why it needs to be opened
        // to be resized and I don't know how to resize without
        // opening it.
        // Note - We use vwindow::Close because the default Close
        //        will also call vevent::StopProcessing.
		myDialog->Open();
		myDialog->vwindow::Close();
#endif
		
        // Move the Dialog in the middle of the screen and open it again.
		vwindow* display = vwindow::GetRoot();
		myDialog->Place(display, vrectPLACE_CENTER, vrectPLACE_CENTER);
		int nCurStyle=myDialog->GetStyle();
		nCurStyle=nCurStyle&~vwindowSTYLE_RESIZABLE;
		myDialog->SetStyle(nCurStyle);
		myDialog->Open();
		
		
        //  Go to the event loop and process events until a 
        //  vevent::StopProcessing call is made (dis_close_confirmDialog)
        //  This is what causes processing to wait for an answer.
        vevent::Process();
		
		// Note - The first word of PromptResult is the button number (0 or 1).
		//	The remainder is the string result.
        char* returnValue = Tcl_GetVar(g_global_interp, "PromptResult", TCL_GLOBAL_ONLY);
        Tcl_SetResult (interp, returnValue, TCL_VOLATILE);
		return TCL_OK;
    }
	
    printf("Error: '%s' folder not found in dictionary.\n", (char*) dialogName);
    return TCL_ERROR;
}




//-----------------------------------------------------------------------------
// dis_prompt_string3 Title Left_Button Middle_Button Right_Button Prompt
int dis_prompt_string3 (ClientData, Tcl_Interp* interp, int argc, char* argv[])
{
    gInit (dis_prompt_string3);
	
    if (argc < 6) {
		Tcl_SetResult(interp, "dis_prompt_string3 requires 5 arguments", TCL_STATIC);
		return TCL_ERROR;
    }
	
    vresource res;
    gString dialogName = "GenericConfirmDialogs.ThreeButtonStringPrompt";
    if (getResource((const vchar*) dialogName, &res))  {
		GConfirmDialog* myDialog = new GConfirmDialog(res);
		
		// Give the dialog box access to the view which created it.
		char* vr = Tcl_GetVar (interp, "vr", TCL_GLOBAL_ONLY);
		myDialog->SetInterpVariable((vchar*)"vr", (vchar*)vr);
		
		char* v  = Tcl_GetVar (interp, "v",  TCL_GLOBAL_ONLY);
		myDialog->SetInterpVariable((vchar*)"v", (vchar*)v);
		
        // Eval the procedures script for the dialog.
        gString procName = dialogName;
        procName += PROCEDURES;
        eval_StringResource(myDialog->GetInterpreter(), procName);
		
        // Parse the prompt into lines.
        gString labels[MAX_LABELS];
        int numLabels = get_labels((const vchar*) argv[5], labels);
		
        // Place the labels on the dialog.
        place_labels(myDialog, labels, numLabels);
		
        // Set the Dialog title.
        myDialog->SetTitle ((vchar*) argv[1]);
		
        // Set the Left Button title.
        Tcl_Interp* myInterp = myDialog->GetInterpreter();
        gdButton* myButton = (gdButton*) dis_findItem(myInterp,
			(vchar*) "LeftButton");
        if (myButton) {
			gString btn_title = argv[2];
			if (btn_title.length() == 0)
				btn_title = "OK";
			myButton->SetTitle(btn_title);
        } else
            TCL_EXECERR1 ("button not found", "LeftButton")
			
			// Set the Middle Button title.
		myButton = (gdButton*) dis_findItem(myInterp,
			(vchar*) "MiddleButton");
        if (myButton) {
			gString btn_title = argv[3];
			if (btn_title.length() == 0)
				btn_title = "OK For All";
			myButton->SetTitle(btn_title);
        } else
            TCL_EXECERR1 ("button not found", "MiddleButton")
			
			// Set the Right Button title.
	    myButton = (gdButton*) dis_findItem(myInterp,
			(vchar*) "RightButton");
        if (myButton) {
			gString btn_title = argv[4];
			if (btn_title.length() == 0)
				btn_title = "Cancel";
			myButton->SetTitle(btn_title);
        } else
            TCL_EXECERR1 ("button not found", "RightButton")
			
			// Set the inital value of the text, if given.
			if (argc > 6)
			myDialog->SetInterpVariable((vchar*)"InputString",
			(vchar*) argv[6]);
		
#if defined(UNIX_OPEN_BUG_FIX)
        // We open and close the dialog here to force all of the
        // resize events. I don't know why it needs to be opened
        // to be resized and I don't know how to resize without
        // opening it.
        // Note - We use vwindow::Close because the default Close
        //        will also call vevent::StopProcessing.
		myDialog->Open();
		myDialog->vwindow::Close();
#endif
		
        // Move the Dialog in the middle of the screen and open it again.
		vwindow* display = vwindow::GetRoot();
		myDialog->Place(display, vrectPLACE_CENTER, vrectPLACE_CENTER);
		int nCurStyle=myDialog->GetStyle();
		nCurStyle=nCurStyle&~vwindowSTYLE_RESIZABLE;
		myDialog->SetStyle(nCurStyle);
		myDialog->Open();
		
		
        //  Go to the event loop and process events until a 
        //  vevent::StopProcessing call is made (dis_close_confirmDialog)
        //  This is what causes processing to wait for an answer.
        vevent::Process();
		
		// Note - The first word of the button number (0, 1, or 2).
		//	The remainder is the string result.
        char* returnValue = Tcl_GetVar(g_global_interp, "PromptResult", TCL_GLOBAL_ONLY);
        Tcl_SetResult (interp, returnValue, TCL_VOLATILE);
		return TCL_OK;
    }
	
    printf("Error: '%s' folder not found in dictionary.\n", (char*) dialogName);
    return TCL_ERROR;
}


int dis_display_string_OK_cancel( char *title, char *prompt_str,
                                 char *err_msg, vstr * &result )
{
	int nRet = -1;
	vresource res;
	result = NULL;
	gString resource_name = (const vchar *)"GenericConfirmDialogs.TwoButtonStringConfirm";
	if (getResource((const vchar*) resource_name, &res))  
	{
		
		GConfirmDialog* pDialog = new GConfirmDialog(res);
		// Eval the procedures script for the dialog.
		resource_name += ".Procedures";
		Tcl_Interp* interp = pDialog->GetInterpreter();
		
		if( prompt_str && strlen(prompt_str) )
		{
			vtextitem *label = vtextitem::CastDown((vdialogItem *)dis_findItem(interp,
				(vchar*) "tagLabel"));
			if( label )
				label->SetText((vchar *)prompt_str);
		}
		
		if( err_msg &&  strlen(err_msg) )
			Tcl_SetVar( interp, "ERR_MSG", err_msg, TCL_GLOBAL_ONLY );
		eval_StringResource(interp, resource_name);
		
		// Set the Dialog title.
		if( title )
			pDialog->SetTitle ((vchar*) title);
		
		
		// Move the Dialog in the middle of the screen and open it again.
		vwindow* display = vwindow::GetRoot();
		pDialog->Place(display, vrectPLACE_CENTER, vrectPLACE_CENTER);
		pDialog->Open();
		
		
		Tcl_SetVar(interp, "ENTERED_STRING", "", TCL_GLOBAL_ONLY);
		Tcl_SetVar(interp, "CLOSE_STATUS", "-1", TCL_GLOBAL_ONLY);
		
		//  Go to the event loop and process events until a 
		//  vevent::StopProcessing call is made (dis_close_confirmDialog)
		//  This is what causes processing to wait for an answer.
		vevent::Process();
		
		char* returnValue = Tcl_GetVar(interp, "CLOSE_STATUS", TCL_GLOBAL_ONLY);
		if( returnValue )
		{
			int nResult = atoi( returnValue );
			if( nResult > 0 )
			{
				char *string = Tcl_GetVar(interp, "ENTERED_STRING", TCL_GLOBAL_ONLY);
				if( string )
				{
					nRet = 1;
					result = vstrClone( (const vchar *)string );
				}
			}
		}
	}
	
	return nRet;
}

//dis_prompt_bool functions:
//
//how to use them:
//
// dis_prompt_bool<n> "title" "LeftButton" [MiddleButton|RightButton] [RightButton] "Message (use \n to separate
// into at most 3 lines)"  [initial value (must be "yes" or "no")]


//-----------------------------------------------------------------------------
// dis_prompt_bool1 Title Button message defaultvalue
int dis_prompt_bool1 (ClientData, Tcl_Interp* interp, int argc, char* argv[])
{
    gInit (dis_prompt_bool1);
	
    if (argc < 4) {
		Tcl_SetResult(interp, "dis_prompt_string1 requires 3 arguments", TCL_STATIC);
		return TCL_ERROR;
    }
	
    vresource res;
    gString dialogName = "GenericConfirmDialogs.OneButtonBoolPrompt";
    if (getResource((const vchar*) dialogName, &res))  {
		GConfirmDialog* myDialog = new GConfirmDialog(res);
		
		// Give the dialog box access to the view which created it.
		char* vr = Tcl_GetVar (interp, "vr", TCL_GLOBAL_ONLY);
		myDialog->SetInterpVariable((vchar*)"vr", (vchar*)vr);
		
		char* v  = Tcl_GetVar (interp, "v",  TCL_GLOBAL_ONLY);
		myDialog->SetInterpVariable((vchar*)"v", (vchar*)v);
		
        // Eval the procedures script for the dialog.
        gString procName = dialogName;
        procName += PROCEDURES;
        eval_StringResource(myDialog->GetInterpreter(), procName);
		
        // Parse the prompt into lines.
        gString labels[MAX_LABELS];
        int numLabels = get_labels((const vchar*) argv[3], labels);
		
        // Place the labels on the dialog.
        place_labels(myDialog, labels, numLabels);
		
        // Set the Dialog title.
        myDialog->SetTitle ((vchar*) argv[1]);
		
        // Set the Button title.
		gString btn_title = argv[2];
		if (btn_title.length() == 0)
			btn_title = "OK";
        Tcl_Interp* myInterp = myDialog->GetInterpreter();
        gdButton* myButton = (gdButton*) dis_findItem(myInterp,
			(vchar*) "LeftButton");
        if (myButton)
            myButton->SetTitle(btn_title);
        else
            TCL_EXECERR1 ("button not found", "LeftButton")
			
#if defined(UNIX_OPEN_BUG_FIX)
			// We open and close the dialog here to force all of the
			// resize events. I don't know why it needs to be opened
			// to be resized and I don't know how to resize without
			// opening it.
			// Note - We use vwindow::Close because the default Close
			//        will also call vevent::StopProcessing.
			myDialog->Open();
		myDialog->vwindow::Close();
#endif
		
        // Move the Dialog in the middle of the screen and open it again.
		vwindow* display = vwindow::GetRoot();
		myDialog->Place(display, vrectPLACE_CENTER, vrectPLACE_CENTER);
		int nCurStyle=myDialog->GetStyle();
		nCurStyle=nCurStyle&~vwindowSTYLE_RESIZABLE;
		myDialog->SetStyle(nCurStyle);
		myDialog->Open();
		
		// Set the inital value of the yesnobox, if given.
		if (argc > 4)
		{
			myDialog->SetInterpVariable((vchar*)"tagYesNo",
			(vchar*) argv[4]);
			//now update the prompt result.
			char buffer[10];
			sprintf(buffer, "0 %s\0", argv[4]);
			Tcl_SetVar (g_global_interp, "PromptResult", &buffer[0], TCL_GLOBAL_ONLY);
		}

        //  Go to the event loop and process events until a 
        //  vevent::StopProcessing call is made (dis_close_confirmDialog)
        //  This is what causes processing to wait for an answer.
        vevent::Process();
		
        char* returnValue = Tcl_GetVar(g_global_interp, "PromptResult", TCL_GLOBAL_ONLY);
        Tcl_SetResult (interp, returnValue, TCL_VOLATILE);
		return TCL_OK;
    }
	
    printf("Error: '%s' folder not found in dictionary.\n", (char*) dialogName);
    return TCL_ERROR;
}


//-----------------------------------------------------------------------------
// dis_prompt_bool2 Title Left_Button Right_Button Value Prompt initialvalue
int dis_prompt_bool2 (ClientData, Tcl_Interp* interp, int argc, char* argv[])
{
    gInit (dis_prompt_bool2);
	
    if (argc < 5) {
		Tcl_SetResult(interp, "dis_prompt_string2 requires 4 arguments", TCL_STATIC);
		return TCL_ERROR;
    }
	
    vresource res;
    gString dialogName = "GenericConfirmDialogs.TwoButtonBoolPrompt";
    if (getResource((const vchar*) dialogName, &res))  {
		GConfirmDialog* myDialog = new GConfirmDialog(res);
		
		// Give the dialog box access to the view which created it.
		char* vr = Tcl_GetVar (interp, "vr", TCL_GLOBAL_ONLY);
		myDialog->SetInterpVariable((vchar*)"vr", (vchar*)vr);
		
		char* v  = Tcl_GetVar (interp, "v",  TCL_GLOBAL_ONLY);
		myDialog->SetInterpVariable((vchar*)"v", (vchar*)v);
		
        // Eval the procedures script for the dialog.
        gString procName = dialogName;
        procName += PROCEDURES;
        eval_StringResource(myDialog->GetInterpreter(), procName);
		
        // Parse the prompt into lines.
        gString labels[MAX_LABELS];
        int numLabels = get_labels((const vchar*) argv[4], labels);
		
        // Place the labels on the dialog.
        place_labels(myDialog, labels, numLabels);
		
        // Set the Dialog title.
        myDialog->SetTitle ((vchar*) argv[1]);
		
        // Set the Left Button title.
        Tcl_Interp* myInterp = myDialog->GetInterpreter();
        gdButton* myButton = (gdButton*) dis_findItem(myInterp,
			(vchar*) "LeftButton");
        if (myButton) {
			gString btn_title = argv[2];
			if (btn_title.length() == 0)
				btn_title = "OK";
			myButton->SetTitle(btn_title);
        } else
            TCL_EXECERR1 ("button not found", "LeftButton")
			
			// Set the Right Button title.
			myButton = (gdButton*) dis_findItem(myInterp,
			(vchar*) "RightButton");
        if (myButton) {
			gString btn_title = argv[3];
			if (btn_title.length() == 0)
				btn_title = "Cancel";
			myButton->SetTitle(btn_title);
        } else
            TCL_EXECERR1 ("button not found", "RightButton")
			
#if defined(UNIX_OPEN_BUG_FIX)
			// We open and close the dialog here to force all of the
			// resize events. I don't know why it needs to be opened
			// to be resized and I don't know how to resize without
			// opening it.
			// Note - We use vwindow::Close because the default Close
			//        will also call vevent::StopProcessing.
			myDialog->Open();
		myDialog->vwindow::Close();
#endif
		
        // Move the Dialog in the middle of the screen and open it again.
		vwindow* display = vwindow::GetRoot();
		myDialog->Place(display, vrectPLACE_CENTER, vrectPLACE_CENTER);
		int nCurStyle=myDialog->GetStyle();
		nCurStyle=nCurStyle&~vwindowSTYLE_RESIZABLE;
		myDialog->SetStyle(nCurStyle);
		myDialog->Open();
		
		
		// Set the inital value of the yesnobox, if given.
		if (argc > 5) {
			myDialog->SetInterpVariable((vchar*)"tagYesNo",
				(vchar*) argv[5]);
			//now update the prompt result.
			char buffer[10];
			sprintf(buffer, "1 %s\0", argv[5]);
			Tcl_SetVar (g_global_interp, "PromptResult", &buffer[0], TCL_GLOBAL_ONLY);

		
		
		} 
		
		
        //  Go to the event loop and process events until a 
        //  vevent::StopProcessing call is made (dis_close_confirmDialog)
        //  This is what causes processing to wait for an answer.
        vevent::Process();
		
		// Note - The first word of PromptResult is the button number (0 or 1).
		//	The remainder is the string result.
        char* returnValue = Tcl_GetVar(g_global_interp, "PromptResult", TCL_GLOBAL_ONLY);
        Tcl_SetResult (interp, returnValue, TCL_VOLATILE);
		return TCL_OK;
    }
	
    printf("Error: '%s' folder not found in dictionary.\n", (char*) dialogName);
    return TCL_ERROR;
}



//-----------------------------------------------------------------------------
// dis_prompt_bool3 Title Left_Button Middle_Button Right_Button Prompt initialvalue
int dis_prompt_bool3 (ClientData, Tcl_Interp* interp, int argc, char* argv[])
{
    gInit (dis_prompt_bool3);
	
    if (argc < 6) {
		Tcl_SetResult(interp, "dis_prompt_bool3 requires 5 arguments", TCL_STATIC);
		return TCL_ERROR;
    }
	
    vresource res;
    gString dialogName = "GenericConfirmDialogs.ThreeButtonBoolPrompt";
    if (getResource((const vchar*) dialogName, &res))  {
		GConfirmDialog* myDialog = new GConfirmDialog(res);
		
		// Give the dialog box access to the view which created it.
		char* vr = Tcl_GetVar (interp, "vr", TCL_GLOBAL_ONLY);
		myDialog->SetInterpVariable((vchar*)"vr", (vchar*)vr);
		
		char* v  = Tcl_GetVar (interp, "v",  TCL_GLOBAL_ONLY);
		myDialog->SetInterpVariable((vchar*)"v", (vchar*)v);
		
        // Eval the procedures script for the dialog.
        gString procName = dialogName;
        procName += PROCEDURES;
        eval_StringResource(myDialog->GetInterpreter(), procName);
		
        // Parse the prompt into lines.
        gString labels[MAX_LABELS];
        int numLabels = get_labels((const vchar*) argv[5], labels);
		
        // Place the labels on the dialog.
        place_labels(myDialog, labels, numLabels);
		
        // Set the Dialog title.
        myDialog->SetTitle ((vchar*) argv[1]);
		
        // Set the Left Button title.
        Tcl_Interp* myInterp = myDialog->GetInterpreter();
        gdButton* myButton = (gdButton*) dis_findItem(myInterp,
			(vchar*) "LeftButton");
        if (myButton) {
			gString btn_title = argv[2];
			if (btn_title.length() == 0)
				btn_title = "OK";
			myButton->SetTitle(btn_title);
        } else
            TCL_EXECERR1 ("button not found", "LeftButton")
			
			// Set the Middle Button title.
        myButton = (gdButton*) dis_findItem(myInterp,
			(vchar*) "MiddleButton");
        if (myButton) {
			gString btn_title = argv[3];
			if (btn_title.length() == 0)
				btn_title = "OK For All";
			myButton->SetTitle(btn_title);
        } else
            TCL_EXECERR1 ("button not found", "MiddleButton")
			
			// Set the Right Button title.
        myButton = (gdButton*) dis_findItem(myInterp,
			(vchar*) "RightButton");
        if (myButton) {
			gString btn_title = argv[4];
			if (btn_title.length() == 0)
				btn_title = "Cancel";
			myButton->SetTitle(btn_title);
        } else
            TCL_EXECERR1 ("button not found", "RightButton")
			
			
#if defined(UNIX_OPEN_BUG_FIX)
			// We open and close the dialog here to force all of the
			// resize events. I don't know why it needs to be opened
			// to be resized and I don't know how to resize without
			// opening it.
			// Note - We use vwindow::Close because the default Close
			//        will also call vevent::StopProcessing.
			myDialog->Open();
		myDialog->vwindow::Close();
#endif
		
        // Move the Dialog in the middle of the screen and open it again.
		vwindow* display = vwindow::GetRoot();
		myDialog->Place(display, vrectPLACE_CENTER, vrectPLACE_CENTER);
		int nCurStyle=myDialog->GetStyle();
		nCurStyle=nCurStyle&~vwindowSTYLE_RESIZABLE;
		myDialog->SetStyle(nCurStyle);
		myDialog->Open();
		// Set the inital value of the text, if given.
		if (argc > 6){

			myDialog->SetInterpVariable((vchar*)"tagYesNo",
			(vchar*) argv[6]);
			//now update the prompt result.
			char buffer[10];
			sprintf(buffer, "2 %s\0", argv[6]);
			Tcl_SetVar (g_global_interp, "PromptResult", &buffer[0], TCL_GLOBAL_ONLY);
		}
		
        //  Go to the event loop and process events until a 
        //  vevent::StopProcessing call is made (dis_close_confirmDialog)
        //  This is what causes processing to wait for an answer.
        vevent::Process();
		
		// Note - The first word of the button number (0, 1, or 2).
		//	The remainder is the string result.
        char* returnValue = Tcl_GetVar(g_global_interp, "PromptResult", TCL_GLOBAL_ONLY);
        Tcl_SetResult (interp, returnValue, TCL_VOLATILE);
		return TCL_OK;
    }
	
    printf("Error: '%s' folder not found in dictionary.\n", (char*) dialogName);
    return TCL_ERROR;
}


//-----------------------------------------------------------------------------



//-----------------------------------------------------------------------------
// dis_prompt_int1 Title Left_Button  Prompt <initialvalue default:0> <min default:0> <max default:100> 
int dis_prompt_int1 (ClientData, Tcl_Interp* interp, int argc, char* argv[])
{
    gInit (dis_prompt_int1);
	
    if (argc < 4) {
		Tcl_SetResult(interp, "dis_prompt_int1 requires 3 arguments", TCL_STATIC);
		return TCL_ERROR;
    }
	
    vresource res;
    gString dialogName = "GenericConfirmDialogs.OneButtonIntPrompt";
    if (getResource((const vchar*) dialogName, &res))  {
		GConfirmDialog* myDialog = new GConfirmDialog(res);

		// Give the dialog box access to the view which created it.
		char* vr = Tcl_GetVar (interp, "vr", TCL_GLOBAL_ONLY);
		myDialog->SetInterpVariable((vchar*)"vr", (vchar*)vr);
		
		char* v  = Tcl_GetVar (interp, "v",  TCL_GLOBAL_ONLY);
		myDialog->SetInterpVariable((vchar*)"v", (vchar*)v);
		
        // Eval the procedures script for the dialog.
        gString procName = dialogName;
        procName += PROCEDURES;
        eval_StringResource(myDialog->GetInterpreter(), procName);
		
        // Parse the prompt into lines.
        gString labels[MAX_LABELS];
        int numLabels = get_labels((const vchar*) argv[3], labels);
		
        // Place the labels on the dialog.
        place_labels(myDialog, labels, numLabels);
		
        // Set the Dialog title.
        myDialog->SetTitle ((vchar*) argv[1]);
		
        // Set the Left Button title.
        Tcl_Interp* myInterp = myDialog->GetInterpreter();
        gdButton* myButton = (gdButton*) dis_findItem(myInterp,
			(vchar*) "LeftButton");
        if (myButton) {
			gString btn_title = argv[2];
			if (btn_title.length() == 0)
				btn_title = "OK";
			myButton->SetTitle(btn_title);
        } else
            TCL_EXECERR1 ("button not found", "LeftButton")
			
		gdNumberSpinner * mySpinner = (gdNumberSpinner *) dis_findItem(myInterp, (vchar *) "tagSpinner");
		if (!mySpinner)
			TCL_EXECERR1 ("Spinner not found", "tagSpinner");
		// now set the min, max for the dialog
		if (argc>5)
		{
			if (argv[5] &&strcmp(argv[5], "") )
			{
				mySpinner->SetMin(atoi(argv[5]));
			}
			if (argc>6)
			{
				mySpinner->SetMax(atoi(argv[6]));
			}
		}

#if defined(UNIX_OPEN_BUG_FIX)
			// We open and close the dialog here to force all of the
			// resize events. I don't know why it needs to be opened
			// to be resized and I don't know how to resize without
			// opening it.
			// Note - We use vwindow::Close because the default Close
			//        will also call vevent::StopProcessing.
			myDialog->Open();
		myDialog->vwindow::Close();
#endif
		
        // Move the Dialog in the middle of the screen and open it again.
		vwindow* display = vwindow::GetRoot();
		myDialog->Place(display, vrectPLACE_CENTER, vrectPLACE_CENTER);			
		int nCurStyle=myDialog->GetStyle();
		nCurStyle=nCurStyle&~vwindowSTYLE_RESIZABLE;
		myDialog->SetStyle(nCurStyle);

		myDialog->Open();
		
		// Set the inital value of the text, if given.
		if (argc > 4&&argv[4]&&strcmp(argv[4], "") )
		{

			myDialog->SetInterpVariable((vchar*)"tagSpinner",
			(vchar*) argv[4]);
			//now update the prompt result.
			char buffer[10];
			sprintf(buffer, "0 %s\0", argv[4]);
			Tcl_SetVar (g_global_interp, "PromptResult", &buffer[0], TCL_GLOBAL_ONLY);
		}

		//myDialog->SetStyle(vwindowSTYLE_BACKGROUND|vwindowSTYLE_BORDER|vwindowSTYLE_MOVABLE);

        //  Go to the event loop and process events until a 
        //  vevent::StopProcessing call is made (dis_close_confirmDialog)
        //  This is what causes processing to wait for an answer.
        vevent::Process();
		
		// Note - The first word of the button number (0)
		//second number is the value of the selection
		
		char* returnValue = Tcl_GetVar(g_global_interp, "PromptResult", TCL_GLOBAL_ONLY);
        Tcl_SetResult (interp, returnValue, TCL_VOLATILE);
		return TCL_OK;
    }
	
    printf("Error: '%s' folder not found in dictionary.\n", (char*) dialogName);
    return TCL_ERROR;
}



//-----------------------------------------------------------------------------
// dis_prompt_int2 Title Left_Button Right_Button Prompt <initialvalue default:0> <min default:0> <max default:100> 
int dis_prompt_int2 (ClientData, Tcl_Interp* interp, int argc, char* argv[])
{
    gInit (dis_prompt_int2);
	
    if (argc < 5) {
		Tcl_SetResult(interp, "dis_prompt_int3 requires 5 arguments", TCL_STATIC);
		return TCL_ERROR;
    }
	
    vresource res;
    gString dialogName = "GenericConfirmDialogs.TwoButtonIntPrompt";
    if (getResource((const vchar*) dialogName, &res))  {
		GConfirmDialog* myDialog = new GConfirmDialog(res);
		
		// Give the dialog box access to the view which created it.
		char* vr = Tcl_GetVar (interp, "vr", TCL_GLOBAL_ONLY);
		myDialog->SetInterpVariable((vchar*)"vr", (vchar*)vr);
		
		char* v  = Tcl_GetVar (interp, "v",  TCL_GLOBAL_ONLY);
		myDialog->SetInterpVariable((vchar*)"v", (vchar*)v);
		
        // Eval the procedures script for the dialog.
        gString procName = dialogName;
        procName += PROCEDURES;
        eval_StringResource(myDialog->GetInterpreter(), procName);
		
        // Parse the prompt into lines.
        gString labels[MAX_LABELS];
        int numLabels = get_labels((const vchar*) argv[4], labels);
		
        // Place the labels on the dialog.
        place_labels(myDialog, labels, numLabels);
		
        // Set the Dialog title.
        myDialog->SetTitle ((vchar*) argv[1]);
		
        // Set the Left Button title.
        Tcl_Interp* myInterp = myDialog->GetInterpreter();
        gdButton* myButton = (gdButton*) dis_findItem(myInterp,
			(vchar*) "LeftButton");
        if (myButton) {
			gString btn_title = argv[2];
			if (btn_title.length() == 0)
				btn_title = "OK";
			myButton->SetTitle(btn_title);
        } else
            TCL_EXECERR1 ("button not found", "LeftButton");

		// Set the Right Button title.
        myButton = (gdButton*) dis_findItem(myInterp,
			(vchar*) "RightButton");
        if (myButton) {
			gString btn_title = argv[3];
			if (btn_title.length() == 0)
				btn_title = "Cancel";
			myButton->SetTitle(btn_title);
        } else
            TCL_EXECERR1 ("button not found", "RightButton");
			
		gdNumberSpinner * mySpinner = (gdNumberSpinner *) dis_findItem(myInterp, (vchar *) "tagSpinner");
		if (!mySpinner)
			TCL_EXECERR1 ("Spinner not found", "tagSpinner");
		// now set the min, max for the dialog
		if (argc>6)
		{
			if (argv[6] &&strcmp(argv[6], "") )
			{
				mySpinner->SetMin(atoi(argv[6]));
			}
			if (argc>7)
			{
				mySpinner->SetMax(atoi(argv[7]));
			}
		}

#if defined(UNIX_OPEN_BUG_FIX)
			// We open and close the dialog here to force all of the
			// resize events. I don't know why it needs to be opened
			// to be resized and I don't know how to resize without
			// opening it.
			// Note - We use vwindow::Close because the default Close
			//        will also call vevent::StopProcessing.
			myDialog->Open();
		myDialog->vwindow::Close();
#endif
		
        // Move the Dialog in the middle of the screen and open it again.
		vwindow* display = vwindow::GetRoot();
		myDialog->Place(display, vrectPLACE_CENTER, vrectPLACE_CENTER);
		int nCurStyle=myDialog->GetStyle();
		nCurStyle=nCurStyle&~vwindowSTYLE_RESIZABLE;
		myDialog->SetStyle(nCurStyle);
		myDialog->Open();
		
		// Set the inital value of the text, if given.
		if (argc > 5&&argv[5]&&strcmp(argv[5], "") )
		{

			myDialog->SetInterpVariable((vchar*)"tagSpinner",
			(vchar*) argv[5]);
			//now update the prompt result.
			char buffer[10];
			sprintf(buffer, "1 %s\0", argv[5]);
			Tcl_SetVar (g_global_interp, "PromptResult", &buffer[0], TCL_GLOBAL_ONLY);
		}

		
        //  Go to the event loop and process events until a 
        //  vevent::StopProcessing call is made (dis_close_confirmDialog)
        //  This is what causes processing to wait for an answer.
        vevent::Process();
		
		// Note - The first word of the button number (0, 1, or 2).
		//	The remainder is the string result.
        char* returnValue = Tcl_GetVar(g_global_interp, "PromptResult", TCL_GLOBAL_ONLY);
        Tcl_SetResult (interp, returnValue, TCL_VOLATILE);
		return TCL_OK;
    }
	
    printf("Error: '%s' folder not found in dictionary.\n", (char*) dialogName);
    return TCL_ERROR;
}


// dis_prompt_int3 Title Left_Button Middle_Button Right_Button Prompt <initialvalue default:0> <min default:0> <max default:100> 
int dis_prompt_int3 (ClientData, Tcl_Interp* interp, int argc, char* argv[])
{
    gInit (dis_prompt_int3);
	
    if (argc < 6) {
		Tcl_SetResult(interp, "dis_prompt_int3 requires 6 arguments", TCL_STATIC);
		return TCL_ERROR;
    }
	
    vresource res;
    gString dialogName = "GenericConfirmDialogs.ThreeButtonIntPrompt";
    if (getResource((const vchar*) dialogName, &res))  {
		GConfirmDialog* myDialog = new GConfirmDialog(res);
		
		// Give the dialog box access to the view which created it.
		char* vr = Tcl_GetVar (interp, "vr", TCL_GLOBAL_ONLY);
		myDialog->SetInterpVariable((vchar*)"vr", (vchar*)vr);
		
		char* v  = Tcl_GetVar (interp, "v",  TCL_GLOBAL_ONLY);
		myDialog->SetInterpVariable((vchar*)"v", (vchar*)v);
		
        // Eval the procedures script for the dialog.
        gString procName = dialogName;
        procName += PROCEDURES;
        eval_StringResource(myDialog->GetInterpreter(), procName);
		
        // Parse the prompt into lines.
        gString labels[MAX_LABELS];
        int numLabels = get_labels((const vchar*) argv[5], labels);
		
        // Place the labels on the dialog.
        place_labels(myDialog, labels, numLabels);
		
        // Set the Dialog title.
        myDialog->SetTitle ((vchar*) argv[1]);
		
        // Set the Left Button title.
        Tcl_Interp* myInterp = myDialog->GetInterpreter();
        gdButton* myButton = (gdButton*) dis_findItem(myInterp,
			(vchar*) "LeftButton");
        if (myButton) {
			gString btn_title = argv[2];
			if (btn_title.length() == 0)
				btn_title = "OK";
			myButton->SetTitle(btn_title);
        } else
            TCL_EXECERR1 ("button not found", "LeftButton")
			
			// Set the Middle Button title.
        myButton = (gdButton*) dis_findItem(myInterp,
			(vchar*) "MiddleButton");
        if (myButton) {
			gString btn_title = argv[3];
			if (btn_title.length() == 0)
				btn_title = "OK For All";
			myButton->SetTitle(btn_title);
        } else
            TCL_EXECERR1 ("button not found", "MiddleButton")
			
			// Set the Right Button title.
        myButton = (gdButton*) dis_findItem(myInterp,
			(vchar*) "RightButton");
        if (myButton) {
			gString btn_title = argv[4];
			if (btn_title.length() == 0)
				btn_title = "Cancel";
			myButton->SetTitle(btn_title);
        } else
            TCL_EXECERR1 ("button not found", "RightButton");
			
		gdNumberSpinner * mySpinner = (gdNumberSpinner *) dis_findItem(myInterp, (vchar *) "tagSpinner");
		if (!mySpinner)
			TCL_EXECERR1 ("Spinner not found", "tagSpinner");
		// now set the min, max for the dialog
		if (argc>7)
		{
			if (argv[7] &&strcmp(argv[7], "") )
			{
				mySpinner->SetMin(atoi(argv[7]));
			}
			if (argc>8)
			{
				mySpinner->SetMax(atoi(argv[8]));
			}
		}

#if defined(UNIX_OPEN_BUG_FIX)
			// We open and close the dialog here to force all of the
			// resize events. I don't know why it needs to be opened
			// to be resized and I don't know how to resize without
			// opening it.
			// Note - We use vwindow::Close because the default Close
			//        will also call vevent::StopProcessing.
			myDialog->Open();
		myDialog->vwindow::Close();
#endif
		
        // Move the Dialog in the middle of the screen and open it again.
		vwindow* display = vwindow::GetRoot();
		myDialog->Place(display, vrectPLACE_CENTER, vrectPLACE_CENTER);
		int nCurStyle=myDialog->GetStyle();
		nCurStyle=nCurStyle&~vwindowSTYLE_RESIZABLE;
		myDialog->SetStyle(nCurStyle);
		myDialog->Open();
		
		// Set the inital value of the text, if given.
		if (argc > 6&&argv[6]&&strcmp(argv[6], "") )
		{

			myDialog->SetInterpVariable((vchar*)"tagSpinner",
			(vchar*) argv[6]);
			//now update the prompt result.
			char buffer[10];
			sprintf(buffer, "2 %s\0", argv[6]);
			Tcl_SetVar (g_global_interp, "PromptResult", &buffer[0], TCL_GLOBAL_ONLY);
		}
		
        //  Go to the event loop and process events until a 
        //  vevent::StopProcessing call is made (dis_close_confirmDialog)
        //  This is what causes processing to wait for an answer.
        vevent::Process();
		
		// Note - The first word of the button number (0, 1, or 2).
		//	The remainder is the string result.
        char* returnValue = Tcl_GetVar(g_global_interp, "PromptResult", TCL_GLOBAL_ONLY);
        Tcl_SetResult (interp, returnValue, TCL_VOLATILE);
		return TCL_OK;
    }
	
    printf("Error: '%s' folder not found in dictionary.\n", (char*) dialogName);
    return TCL_ERROR;
}

