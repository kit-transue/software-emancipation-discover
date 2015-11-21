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
#include <Xm/DialogS.h>
#include <Xm/PanedW.h>
#include <Xm/PushBG.h>
#include <Xm/Form.h>
#include <Xm/SashP.h>
#include <Xm/RowColumn.h>
#include <Xm/Frame.h>
#include <Xm/ToggleBG.h>
#include <Xm/LabelG.h>
#include <Xm/Text.h>
#include <Xm/MessageB.h>

#include "top_widgets.h"

#include "IMAttachment.h"

string CIMAttachment::m_szHostName="";
string CIMAttachment::m_szPort="";
string CIMAttachment::m_szUserName="";
string CIMAttachment::m_szPassword="";
string CIMAttachment::m_szIssueID="";

CIMAttachment::CIMAttachment() : m_Dialog(NULL),m_IsOK(-1)
{
}

CIMAttachment::~CIMAttachment(){
	if(m_Dialog)  XtDestroyWidget(m_Dialog);
}

void CIMAttachment::AddCallback(CIMAttachmentCallbackHandler* pHandler) {
        m_Handlers.push_back(pHandler);
}
 
void CIMAttachment::RemoveCallback(CIMAttachmentCallbackHandler* pHandler) {
        TAttachmentCallbacksIterator i;
        for(i=m_Handlers.begin();i!=m_Handlers.end();i++) {
                CIMAttachmentCallbackHandler* handler = (CIMAttachmentCallbackHandler*)*i;
                if(handler == pHandler) {
                        m_Handlers.remove(pHandler);
                }
        }
}
 
void CIMAttachment::FireCallback(int nIsOK) {
        TAttachmentCallbacksIterator i;
        for(i=m_Handlers.begin();i!=m_Handlers.end();i++) {
                CIMAttachmentCallbackHandler* handler = (CIMAttachmentCallbackHandler*)*i;
                if(handler) {
                        handler->ReadyToAttach(nIsOK);
                }
        }
}

void CIMAttachment::OkPressed(Widget dialog,XtPointer client_data,XtPointer call_data) {
	CIMAttachment* pThis = (CIMAttachment*)client_data;
	pThis->m_IsOK = 1;
	
	m_szHostName=XmTextGetString(pThis->m_HostNameField);
	m_szPort=XmTextGetString(pThis->m_PortField);
	m_szUserName=XmTextGetString(pThis->m_UserNameField);
	m_szIssueID=XmTextGetString(pThis->m_IssueIDField);

	pThis->FireCallback(pThis->m_IsOK);

	rem_top_widget(pThis->m_Dialog);
	XtDestroyWidget(pThis->m_Dialog);
	pThis->m_Dialog = NULL;
}

void CIMAttachment::CancelPressed(Widget dialog,XtPointer client_data,XtPointer call_data) {
	CIMAttachment* pThis = (CIMAttachment*)client_data;
	pThis->m_IsOK = 0;

	rem_top_widget(pThis->m_Dialog);
	XtDestroyWidget(pThis->m_Dialog);
	pThis->m_Dialog = NULL;

	pThis->FireCallback(pThis->m_IsOK);
}

// CheckPassword() -- handle the input of a password
void CIMAttachment::CheckPassword(Widget text_w,XtPointer client_data,XtPointer call_data) {
	char* passwd = NULL;
	char* new_pwd = NULL;
	int nLen = 0;
	bool bDelete = false;

	XmTextVerifyCallbackStruct* cbs = (XmTextVerifyCallbackStruct*) call_data;

	if((nLen=m_szPassword.length()) > 0) {
		passwd = XtMalloc(nLen+1);
		strcpy(passwd,m_szPassword.c_str());
	}

	if( cbs->startPos < cbs->endPos ) { // shrink the password by moving endPos... characters forward to
					    // the point of deletion
		memmove( &(passwd[cbs->startPos]),&(passwd[cbs->endPos]), strlen(passwd) - cbs->endPos + 1 );
		if( cbs->text->length == 0 ) { // then just a delete, not a replace
			bDelete = true;
		}
	}

	new_pwd = XtMalloc( cbs->text->length + 1 );
	if(!bDelete) {
		strncpy( new_pwd, cbs->text->ptr, cbs->text->length ); // just the new chars
		new_pwd[cbs->text->length]=0;

		if( passwd ) { // open a hole for the new characters, and insert them
			       // in the proper place
			passwd = XtRealloc( passwd, strlen(passwd) + cbs->text->length + 1 );
			memmove( &(passwd[cbs->startPos + cbs->text->length]), &(passwd[cbs->startPos]), strlen(passwd) - cbs->startPos + 1 );
			memcpy( &(passwd[cbs->startPos]), new_pwd, cbs->text->length );
		} else {
			passwd = new_pwd;
		}
	}
	m_szPassword = passwd;
	XtFree(new_pwd);
	XtFree(passwd);

	memset( cbs->text->ptr, '*', cbs->text->length ); // makes it all stars
}


Widget CIMAttachment::CreateParametersArea(Widget parent) {
	XmString labelStr;
	Widget area = XtVaCreateManagedWidget("infoArea", 
		xmRowColumnWidgetClass, parent,
		XmNwidth, 300,
		NULL);
	Widget hostNameArea = XtVaCreateManagedWidget("hostNameArea", 
		xmRowColumnWidgetClass, area,
		XmNnumColumns, 2,
		XmNpacking, XmPACK_COLUMN,
		XmNorientation, XmVERTICAL,
		NULL);
	labelStr = XmStringCreateLocalized("Host Name:");
	Widget label = XtVaCreateManagedWidget("label_host_name",xmLabelGadgetClass, hostNameArea,
			XmNlabelString, labelStr,
			NULL);
	XmStringFree(labelStr);
	Widget hostNameInputArea = XtVaCreateManagedWidget("hostNameInputArea",
			xmFormWidgetClass, hostNameArea,
			NULL);
	m_HostNameField = XtVaCreateManagedWidget("hostNameInputField",
			xmTextWidgetClass, hostNameInputArea,
			XmNvalue, m_szHostName.c_str(),
			XmNeditable, True,
			XmNtopAttachment,XmATTACH_FORM, 
			XmNleftAttachment,XmATTACH_FORM, 
			XmNrightAttachment,XmATTACH_FORM, 
			XmNbottomAttachment,XmATTACH_FORM,
			XmNtraversalOn, True, 
			NULL);

	Widget hostPortArea = XtVaCreateManagedWidget("hostPortArea", 
		xmRowColumnWidgetClass, area,
		XmNnumColumns, 2,
		XmNpacking, XmPACK_COLUMN,
		XmNorientation, XmVERTICAL,
		NULL);
	labelStr = XmStringCreateLocalized("Port:");
	label = XtVaCreateManagedWidget("label_host_port",
			xmLabelGadgetClass, hostPortArea,
			XmNlabelString, labelStr,
			NULL);
	XmStringFree(labelStr);
	Widget hostPortInputArea = XtVaCreateManagedWidget("hostPortInputArea",
			xmFormWidgetClass, hostPortArea,
			NULL);
	m_PortField = XtVaCreateManagedWidget("hostPortInputField",
		xmTextWidgetClass, hostPortInputArea,
		XmNvalue, m_szPort.c_str(),
		XmNtopAttachment,XmATTACH_FORM, 
		XmNleftAttachment,XmATTACH_FORM,
		XmNrightAttachment,XmATTACH_FORM, 
		XmNbottomAttachment,XmATTACH_FORM, 
		NULL);

	Widget userNameArea = XtVaCreateManagedWidget("userNameArea", 
		xmRowColumnWidgetClass, area,
		XmNnumColumns, 2,
		XmNpacking, XmPACK_COLUMN,
		XmNorientation, XmVERTICAL,
		NULL);
	labelStr = XmStringCreateLocalized("User Name:");
	label = XtVaCreateManagedWidget("label_user_name",
			xmLabelGadgetClass, userNameArea,
			XmNlabelString, labelStr,
			NULL);
	XmStringFree(labelStr);
	Widget  userNameInputArea = XtVaCreateManagedWidget("userNameInputArea",
			xmFormWidgetClass, userNameArea,
			NULL);
	m_UserNameField = XtVaCreateManagedWidget("userNameInputField",
		xmTextWidgetClass, userNameInputArea,
		XmNvalue, m_szUserName.c_str(),
		XmNtopAttachment,XmATTACH_FORM, 
		XmNleftAttachment,XmATTACH_FORM,
		XmNrightAttachment,XmATTACH_FORM, 
		XmNbottomAttachment,XmATTACH_FORM, 
		NULL);

	Widget passwordArea = XtVaCreateManagedWidget("passwordArea", 
		xmRowColumnWidgetClass, area,
		XmNnumColumns, 2,
		XmNpacking, XmPACK_COLUMN,
		XmNorientation, XmVERTICAL,
		NULL);
	labelStr = XmStringCreateLocalized("Password:");
	label = XtVaCreateManagedWidget("label_password",
			xmLabelGadgetClass, passwordArea,
			XmNlabelString, labelStr,
			NULL);
	XmStringFree(labelStr);
	Widget passwordInputArea = XtVaCreateManagedWidget("passwordInputArea",
			xmFormWidgetClass, passwordArea,
			NULL);
	m_PasswordField = XtVaCreateManagedWidget("passwordInputField",
		xmTextWidgetClass, passwordInputArea,
		XmNtopAttachment,XmATTACH_FORM, 
		XmNleftAttachment,XmATTACH_FORM,
		XmNrightAttachment,XmATTACH_FORM, 
		XmNbottomAttachment,XmATTACH_FORM, 
		NULL);
	XtAddCallback(m_PasswordField, XmNmodifyVerifyCallback, CheckPassword, this);
	// we need to save current password
	string tmpPassword = m_szPassword;
	m_szPassword = ""; // and empty password field if it will not be empty
			   // when we set the widget value the password field will be duplicated 
	XmTextSetString(m_PasswordField,(char*)tmpPassword.c_str()); // now set the widget value and the
							     // field value will be set as well 
							     // through a callback function 
	
	Widget issueIDArea = XtVaCreateManagedWidget("issueIDArea", 
		xmRowColumnWidgetClass, area,
		XmNnumColumns, 2,
		XmNpacking, XmPACK_COLUMN,
		XmNorientation, XmVERTICAL,
		NULL);
	labelStr = XmStringCreateLocalized("Issue ID:");
	label = XtVaCreateManagedWidget("label_issue_ID",
			xmLabelGadgetClass, issueIDArea,
			XmNlabelString, labelStr,
			NULL);
	XmStringFree(labelStr);
	Widget issueIDInputArea = XtVaCreateManagedWidget("issueIDInputArea",
			xmFormWidgetClass, issueIDArea,
			NULL);
	m_IssueIDField = XtVaCreateManagedWidget("issueIDInputField",
		xmTextWidgetClass, issueIDInputArea,
		XmNvalue, m_szIssueID.c_str(),
		XmNtopAttachment,XmATTACH_FORM, 
		XmNleftAttachment,XmATTACH_FORM,
		XmNrightAttachment,XmATTACH_FORM, 
		XmNbottomAttachment,XmATTACH_FORM, 
		NULL);

	return area;
}

bool CIMAttachment::AskForParameters(Widget parent){
	string title = "Attach to IM issue...";

	if(m_Dialog)	XtDestroyWidget(m_Dialog);

	m_Dialog = XtVaCreatePopupShell(title.c_str(),
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
	// action parameters area
	Widget attrsArea = CreateParametersArea(pane);
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
	
	Dimension w;
	XtVaGetValues(m_Dialog,XmNheight, &h, XmNwidth, &w, NULL);
	XtVaSetValues(m_Dialog,
		XmNminHeight, h,
		XmNmaxHeight, h,
		NULL);
	return true;
}

bool CIMAttachment::Attach(string szFile,Widget wParent){
	bool bRetCode = true;
	push_busy_cursor();

	string szCommand = "im editissue --addAttachment="+szFile+" --hostname="+m_szHostName+" --port="+m_szPort+" --password="+m_szPassword+" --user="+m_szUserName+" "+m_szIssueID;

	int nRet = system(szCommand.c_str());
	
	if(nRet == -1) {
		XmString errorStr = XmStringCreateLocalized("Can't excute an IM client.");
		Arg args[1];
		int nCount=0;

		XtSetArg(args[nCount],XmNmessageString,errorStr);nCount++;
		XmCreateErrorDialog(wParent,"Error",args,nCount);
		XmStringFree(errorStr);
		bRetCode = false;			
	} else {
		if(nRet != 0) {
			XmString errorStr = XmStringCreateLocalized("Can't attach the report to an IM issue.");
			Arg args[1];
			int nCount=0;
			XtSetArg(args[nCount],XmNmessageString,errorStr);nCount++;
			XtManageChild(XmCreateErrorDialog(wParent,"Error",args,nCount));
			XmStringFree(errorStr);
			bRetCode = false;
		}
	}
	pop_cursor();		
	return bRetCode;
}
