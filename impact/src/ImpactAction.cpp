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
#include "ImpactAction.h"

#include <Xm/DialogS.h>
#include <Xm/MessageB.h>
#include <Xm/MessageBP.h>
#include <Xm/BulletinBP.h>
#include <Xm/PanedW.h>
#include <Xm/PushBG.h>
#include <Xm/Form.h>
#include <Xm/SashP.h>

Widget CImpactAction::m_Dialog = NULL;
int CImpactAction::m_IsOK = -1;

CImpactAction::CImpactAction(string name) {
	m_szName=name;
	m_bExecuted = false;
	m_pResults = NULL;
	m_pBusyCursor=NULL;
}

CImpactAction::~CImpactAction(){
	if(m_Dialog)  XtDestroyWidget(m_Dialog);
	TCallbacksIterator i;	
	TCallbacksIterator end = m_Callbacks.end();	
	for(i=m_Callbacks.begin();i!=end;i++) {
		delete *i;
	}	
}

bool CImpactAction::HasAttributes(Widget parent, symbolPtr* pSym) {
	return true;
}

string& CImpactAction::GetName() {
	return m_szName;
}

string CImpactAction::prepareQuery(const string& query) {
	return CEntityInfo::prepareQuery(query); 
}

void CImpactAction::FireEvent(CImpactAction* pThis,int nState) {
	TCallbacksList* pCallbacks = pThis->GetCallbacks();
	TCallbacksIterator i;	
	for(i=pCallbacks->begin();i!=pCallbacks->end();) {
		CCallbackInfo* info = (CCallbackInfo*)*i;
		XtCallbackProc callback = info->GetCallback();
		if(callback) {
			CCallbackInfo* pUserInfo = (CCallbackInfo*)info->GetData();
			pUserInfo->SetState(nState);
			callback(pUserInfo->GetCallbackWidget(),pUserInfo->GetData(),pUserInfo);
			if(!info->IsValid()) {
				delete *i;
				pCallbacks->erase(i++);
			} else {
				++i;
			}
		}
	}
}

#include "top_widgets.h"

void CImpactAction::setWait(bool state) {
	if(state) {
		printf("in Wait state.....\n");
		m_pBusyCursor = new tempCursor();
	} else {
		delete m_pBusyCursor;
		m_pBusyCursor=NULL;
		printf("in Normal state.....\n");
	}
}

void CImpactAction::OkPressed(Widget dialog,XtPointer client_data, XtPointer call_data) {
	m_IsOK = 1;
	CImpactAction* pThis = (CImpactAction*)client_data;

	pThis->setWait(true);

	FireEvent(pThis,CCallbackInfo::END);

	pThis->setWait(false);

	rem_top_widget(m_Dialog);
	XtDestroyWidget(m_Dialog);
	m_Dialog = NULL;
}

void CImpactAction::CancelPressed(Widget dialog,XtPointer client_data, XtPointer call_data) {
	m_IsOK = 0;
	CImpactAction* pThis = (CImpactAction*)client_data;
	
	pThis->setWait(true);

	FireEvent(pThis,CCallbackInfo::END);
	
	pThis->setWait(false);

	rem_top_widget(m_Dialog);
	XtDestroyWidget(m_Dialog);
	m_Dialog = NULL;
}

void CImpactAction::TurnOffSashTraversal(Widget pane) {
	Widget *children;
	int	nChildren;
	
	XtVaGetValues(pane,
		XmNchildren,&children,
		XmNnumChildren, &nChildren,
		NULL);
	while(nChildren-- > 0) {
		if(XmIsSash(children[nChildren]))
			XtVaSetValues(children[nChildren],
				XmNtraversalOn, False,
				NULL);
	}
}

Widget GetTopMost(Widget wnd) {
	Widget parent = wnd;
	while(XtParent(parent)) parent = XtParent(parent);
	return parent;
}

void CImpactAction::CenterDialog(Widget dialog) {
	Position sX, sY;
	Dimension sWidth, sHeight;
	Position dX, dY;
	Dimension dWidth, dHeight;

	Widget shell = GetTopMost(dialog);

	XtVaGetValues(shell, 
		      XtNwidth, &sWidth, 
		      XtNheight, &sHeight,
		      XtNx, &sX, 
		      XtNy, &sY,
		      NULL);  

	XtVaGetValues(dialog, 
		      XtNwidth, &dWidth, 
		      XtNheight, &dHeight,
		      NULL);
	dX = (sWidth - dWidth)/2;  
	dY = (sHeight - dHeight)/2;  

	// ...translate coordinates to center of the root window
	XtTranslateCoords(shell, dX, dY, &dX, &dY);
 
	// ...move popup shell to this position (it's not visible yet)... 
	XtVaSetValues(dialog, XtNx, dX, XtNy, dY, NULL);
}

void CImpactAction::PopupErrorDialog(char* text)
{
  Widget dialog;
  XmString xm_string;
  Arg args[3];

  /* set the lavel for the dialog */
  xm_string = XmStringCreateSimple (text);

  XtSetArg(args[0], XmNmessageString, xm_string);
  XtSetArg(args[1], XmNcancelLabelString, NULL);
  XtSetArg(args[2], XmNhelpLabelString, NULL);

  /* Create the InformationDialog as child of push_button */
  dialog = XmCreateInformationDialog (m_Dialog, "info", args, 3);

  XmMessageBoxWidget mb_dialog = (XmMessageBoxWidget) dialog;
  XtUnmanageChild( mb_dialog->bulletin_board.cancel_button );
  XtUnmanageChild( mb_dialog->message_box.help_button );

  /* No longer need the compound string, free it */
  XmStringFree(xm_string);

  /* add the callback routine */
  //XtAddCallback(dialog, XmNokCallback, activate, NULL );

  /* manage the dialog */  
  XtManageChild(dialog);
  
  /* Curt - original XtPopup with XtGrabNone */
  XtPopup(XtParent(dialog), XtGrabExclusive);
}

void CImpactAction::AddCallback(XtCallbackProc callback,XtPointer client_data) {
	m_Callbacks.push_back(new CCallbackInfo(callback,client_data));
}

void CImpactAction::RemoveCallback(XtCallbackProc callbackToRemove) {	
	TCallbacksIterator i;	
	for(i=m_Callbacks.begin();i!=m_Callbacks.end();++i) {
		CCallbackInfo* info = (CCallbackInfo*)*i;
		XtCallbackProc callback = info->GetCallback();
		if(callback == callbackToRemove) {
			info->Invalidate();
			break;
		}
	}
}

bool CImpactAction::AskAttributes(Widget parent, const char* title, symbolPtr* pSym) {
	if(m_Dialog)	XtDestroyWidget(m_Dialog);

	m_Dialog = XtVaCreatePopupShell(title,
			xmDialogShellWidgetClass,parent,
			XmNdeleteResponse,XmDESTROY,
			NULL);
	add_top_widget(m_Dialog);
	Widget dialogForm = XtVaCreateWidget("dialogForm",
				xmFormWidgetClass,m_Dialog,
				XmNdialogStyle, XmDIALOG_FULL_APPLICATION_MODAL,
				NULL);
	Widget pane = XtVaCreateManagedWidget("pane",xmPanedWindowWidgetClass,dialogForm,
			XmNsashWidth,1,
			XmNsashHeight,1,
			XmNleftAttachment,XmATTACH_FORM,
			XmNtopAttachment,XmATTACH_FORM,
			XmNrightAttachment,XmATTACH_FORM,
			XmNbottomAttachment,XmATTACH_FORM,
			NULL);
	// action specific area
	Widget attrsArea = GetAttributesArea(pane,pSym);
	
	if(attrsArea!=NULL) {
		XtVaSetValues(attrsArea,
			XmNleftAttachment, XmATTACH_FORM,
			XmNtopAttachment, XmATTACH_FORM,
			XmNrightAttachment, XmATTACH_FORM,
			XmNbottomAttachment, XmATTACH_FORM,
			NULL);
		XtManageChild(attrsArea);
	} else {
		OkPressed(m_Dialog,this,NULL);
		return true;
	}
	
	// common area
	Widget form = XtVaCreateManagedWidget("form",xmFormWidgetClass, pane,
			XmNfractionBase, 5,
			NULL);
	Widget okButton = XtVaCreateManagedWidget("OK",
		xmPushButtonGadgetClass, form,
		XmNtopAttachment, XmATTACH_FORM,
		XmNbottomAttachment, XmATTACH_FORM,
		XmNleftAttachment, XmATTACH_POSITION,
		XmNleftPosition, 1,
		XmNrightAttachment, XmATTACH_POSITION,
		XmNrightPosition, 2,
		XmNshowAsDefault, True,
		XmNdefaultButtonShadowThickness, 1,
		NULL);
	XtAddCallback(okButton,XmNactivateCallback,OkPressed,this);
	Widget cancelButton = XtVaCreateManagedWidget("Cancel",
		xmPushButtonGadgetClass, form,
		XmNtopAttachment, XmATTACH_FORM,
		XmNbottomAttachment, XmATTACH_FORM,
		XmNleftAttachment, XmATTACH_POSITION,
		XmNleftPosition, 3,
		XmNrightAttachment, XmATTACH_POSITION,
		XmNrightPosition, 4,
		XmNshowAsDefault, False,
		XmNdefaultButtonShadowThickness, 1,
		NULL);
	XtAddCallback(cancelButton,XmNactivateCallback,CancelPressed,this);
		
	Dimension h;
	XtVaGetValues(cancelButton,XmNheight, &h, NULL);
	XtVaSetValues(form,XmNpaneMaximum, h,XmNpaneMinimum, h, NULL);
	XtManageChild(dialogForm);		
	
	TurnOffSashTraversal(pane);
	CenterDialog(m_Dialog);
	Dimension w;
	XtVaGetValues(m_Dialog,XmNheight, &h, XmNwidth, &w, NULL);
	XtVaSetValues(m_Dialog,
		XmNminHeight, h,
		XmNmaxHeight, h,
		NULL);
	SetModal();
	return true;
}



