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
#include <Xm/RowColumn.h>
#include <Xm/LabelG.h>
#include <Xm/Text.h>
#include <Xm/Form.h>
#include <Xm/Frame.h>
#include <Xm/DialogS.h>
#include <Xm/PushBG.h>
#include <Xm/Form.h>
#include <Xm/SashP.h>
#include <Xm/PanedW.h>
#include <Xm/MessageB.h>
#include <unistd.h>


#include "RenameAction.h"
#include "EntityInfo.h"

bool CRenameAction::canProceed = false;
bool CRenameAction::gotUserInput = false;
Widget CRenameAction::mainDialog = NULL;

CRenameAction::CRenameAction() : CImpactAction("Rename") {
}

CRenameAction::~CRenameAction() {
}

bool CRenameAction::HasAttributes(Widget parent, symbolPtr* pSym) {
		
	if ( CEntityInfo::kind(pSym) == "package" ) {

		// label to display	
       		XmString labelStr = XmStringCreateLocalized("Running impact queries on packages may take a long time.\nWould you like to proceed?");
		Arg args[3];
		
		XtSetArg(args[0], XmNmessageString, labelStr);
		XtSetArg(args[1], XmNcancelLabelString, NULL);
		XtSetArg(args[2], XmNokLabelString, NULL);
	
		// main dialog with message and buttons	
	 	mainDialog = XmCreateQuestionDialog(parent, "warning", args, 3);
		XmMessageBoxWidget mb_dialog = (XmMessageBoxWidget) mainDialog;
		// remove help button
		XtUnmanageChild(XmMessageBoxGetChild(mainDialog, XmDIALOG_HELP_BUTTON));
		XmStringFree(labelStr);

		// this will force us to wait for the user inpout
		XtVaSetValues(mainDialog, 
			XmNdialogStyle, XmDIALOG_FULL_APPLICATION_MODAL,
			NULL);

		// the callbacks will set and store the answer from the user
		XtAddCallback(mainDialog, XmNokCallback, OkToProceed,this);	
		XtAddCallback(mainDialog, XmNcancelCallback, NotOkToProceed, this);	

		// make it pop up
		XtManageChild(mainDialog);
		XtPopup(XtParent(mainDialog), XtGrabExclusive);			
		// wait for user response
		XtAppContext appContext = XtWidgetToApplicationContext(mainDialog);
		while (!gotUserInput) {
			XtInputMask mask = XtAppPending(appContext);
			if (mask!=0) XtAppProcessEvent(appContext, mask);
		}
		// set this for the next time around
		gotUserInput = false;
		return canProceed;
		
	} else {		
		return true;
	}
}

Widget CRenameAction::GetAttributesArea(Widget parent, symbolPtr* pSym) {
	XmString labelStr;
	Widget area = XtVaCreateManagedWidget("infoArea", 
		xmRowColumnWidgetClass, parent,
		XmNwidth, 300,
		NULL);
	Widget oldArea = XtVaCreateManagedWidget("oldArea", 
		xmRowColumnWidgetClass, area,
		XmNnumColumns, 2,
		XmNpacking, XmPACK_COLUMN,
		XmNorientation, XmVERTICAL,
		NULL);
	labelStr = XmStringCreateLocalized("Original Name:");
	Widget label = XtVaCreateManagedWidget("label_1",xmLabelGadgetClass, 
			oldArea,
			XmNlabelString, labelStr,
			NULL);
	XmStringFree(labelStr);
	Widget typeArea = XtVaCreateManagedWidget("formArea",
			xmFormWidgetClass, oldArea,
			NULL);

    if ( CEntityInfo::kind(pSym) == "package" )
        XtVaCreateManagedWidget("type",
                xmTextWidgetClass, typeArea,
                XmNvalue, CEntityInfo::entityName(pSym).c_str(),
                XmNeditable, False,
                XmNtopAttachment,XmATTACH_FORM, 
                XmNleftAttachment,XmATTACH_FORM, 
                XmNrightAttachment,XmATTACH_FORM, 
                XmNbottomAttachment,XmATTACH_FORM,
                XmNtraversalOn, False, 
                NULL);
	else
        XtVaCreateManagedWidget("type",
                xmTextWidgetClass, typeArea,
                XmNvalue, CEntityInfo::name(pSym).c_str(),
                XmNeditable, False,
                XmNtopAttachment,XmATTACH_FORM, 
                XmNleftAttachment,XmATTACH_FORM, 
                XmNrightAttachment,XmATTACH_FORM, 
                XmNbottomAttachment,XmATTACH_FORM,
                XmNtraversalOn, False, 
                NULL);

	Widget newArea = XtVaCreateManagedWidget("newArea", 
		xmRowColumnWidgetClass, area,
		XmNnumColumns, 2,
		XmNpacking, XmPACK_COLUMN,
		XmNorientation, XmVERTICAL,
		NULL);
	labelStr = XmStringCreateLocalized("New Name:");
	label = XtVaCreateManagedWidget("label_2",
			xmLabelGadgetClass, newArea,
			XmNlabelString, labelStr,
			NULL);
	XmStringFree(labelStr);
	Widget nameArea = XtVaCreateManagedWidget("nameArea",
			xmFormWidgetClass, newArea,
			NULL);
	nameField = XtVaCreateManagedWidget("name",
		xmTextWidgetClass, nameArea,
		XmNtopAttachment,XmATTACH_FORM, 
		XmNleftAttachment,XmATTACH_FORM,
		XmNrightAttachment,XmATTACH_FORM, 
		XmNbottomAttachment,XmATTACH_FORM, 
		NULL);

	return area;
}

// call back for ok button
void CRenameAction::OkToProceed(Widget dialog, XtPointer client_data, XtPointer call_data) {

	if (mainDialog != NULL) {
		rem_top_widget(mainDialog);
		XtDestroyWidget(mainDialog);
		mainDialog = NULL;
	}
	gotUserInput = true;
	canProceed = true;
}


// call back for cancel button
void CRenameAction::NotOkToProceed(Widget dialog, XtPointer client_data, XtPointer call_data) {
	if (mainDialog != NULL) {
		rem_top_widget(mainDialog);
		XtDestroyWidget(mainDialog);
		mainDialog = NULL;
	}
	gotUserInput = true;
	canProceed = false;
}

bool CRenameAction::ActionPerformed(symbolPtr* pSym) {
  CImpactAction::ActionPerformed(pSym);
  
  string newname;
  char* text = XmTextGetString(nameField);
  newname = text;
  XtFree(text);

  SetDescription(GetName()+ " " + CEntityInfo::kind(pSym) + " from " + CEntityInfo::name(pSym) + " to " + newname);

  string command = CEntityInfo::getRenameCommand(pSym, prepareQuery(newname));

  string results;
  CEntityInfo::exec(command,results);
  parseResult(results);
  return true;
}








