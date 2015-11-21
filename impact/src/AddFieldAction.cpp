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
#include <Xm/Frame.h>
#include <Xm/ToggleBG.h>
#include <Xm/Form.h>

#include "AddFieldAction.h"

Widget CAddFieldAction::GetAttributesArea(Widget parent, symbolPtr* pSym) {
	XmString labelStr;
	Widget fullArea = XtVaCreateWidget("area",
			xmRowColumnWidgetClass, parent,
			NULL);
	
	Widget border = XtVaCreateManagedWidget("etchedBorder",
			xmFrameWidgetClass, fullArea,
			XmNshadowType, XmSHADOW_ETCHED_IN,
			NULL);
 
	Widget area = XtVaCreateManagedWidget("infoArea", 
		xmRowColumnWidgetClass, border,
		NULL);
	
	Widget typeArea = XtVaCreateManagedWidget("typeArea", 
		xmRowColumnWidgetClass, area,
		XmNnumColumns, 2,
		XmNpacking, XmPACK_COLUMN,
		XmNorientation, XmVERTICAL,
		NULL);
	labelStr = XmStringCreateLocalized("Type:");
	Widget label = XtVaCreateManagedWidget("label_1",xmLabelGadgetClass, typeArea,
			XmNlabelString, labelStr,
			NULL);
	XmStringFree(labelStr);
	m_typeField = XtVaCreateManagedWidget("type",
		xmTextWidgetClass, typeArea,
		NULL);

	Widget nameArea = XtVaCreateManagedWidget("nameArea", 
		xmRowColumnWidgetClass, area,
		XmNnumColumns, 2,
		XmNpacking, XmPACK_COLUMN,
		XmNorientation, XmVERTICAL,
		NULL);
	labelStr = XmStringCreateLocalized("Name:");
	label = XtVaCreateManagedWidget("label_2",xmLabelGadgetClass, nameArea,
			XmNlabelString, labelStr,
			NULL);
	XmStringFree(labelStr);
	m_nameField = XtVaCreateManagedWidget("name",
		xmTextWidgetClass, nameArea,
		NULL);

	border = XtVaCreateManagedWidget("etchedBorder1",
			xmFrameWidgetClass, fullArea,
			XmNshadowType, XmSHADOW_ETCHED_IN,
			NULL);
	Widget permissionsArea = XmCreateRadioBox(border,"permisArea",NULL,0);
	publicInd = XtVaCreateManagedWidget("Public",
			xmToggleButtonGadgetClass,permissionsArea,
			NULL);
	XtAddCallback(publicInd,XmNvalueChangedCallback,OnPermissionChanged,this);
	privateInd = XtVaCreateManagedWidget("Private",
			xmToggleButtonGadgetClass,permissionsArea,
			NULL);
	XtAddCallback(privateInd,XmNvalueChangedCallback,OnPermissionChanged,this);
	protectedInd = XtVaCreateManagedWidget("Protected",
			xmToggleButtonGadgetClass,permissionsArea,
			NULL);
	XtAddCallback(protectedInd,XmNvalueChangedCallback,OnPermissionChanged,this);
	XtManageChild(permissionsArea);
		
	return fullArea;	
}

void CAddFieldAction::OnPermissionChanged(Widget widget,XtPointer client_data,XtPointer call_data) {
	CAddFieldAction* pThis = (CAddFieldAction*)client_data;
	XmToggleButtonCallbackStruct* state = (XmToggleButtonCallbackStruct*) call_data;
	if(state->set) {
		if(widget == pThis-> publicInd) pThis->setPermission(PUBLIC);
		else if(widget == pThis->privateInd) pThis->setPermission(PRIVATE);
		else if(widget == pThis->protectedInd) pThis->setPermission(PROTECTED);
		else pThis->setPermission(NONE);
	}
}

bool CAddFieldAction::ActionPerformed(symbolPtr* pSym) {
  CImpactAction::ActionPerformed(pSym);
  
  string name;
  string type;
  char* text = XmTextGetString(m_nameField);
  name = text;
  XtFree(text);
  text = XmTextGetString(m_typeField);
  type = text;
  XtFree(text);

  string szDesc = GetName();
  if(m_nPermission==PUBLIC) szDesc += " public";
  else if(m_nPermission==PRIVATE) szDesc += " private";
  else if(m_nPermission==PROTECTED) szDesc += " protected";

  szDesc = " " + type + " " + name;
  SetDescription(szDesc);
  string command = "Impact:AddVariable " + prepareQuery(CEntityInfo::etag(pSym)) + " " +
                           "{" + type + "}" + " " + name + " ";
  char strPerm[10];
  sprintf(strPerm,"%d", m_nPermission);
  command += strPerm;

  string results;
  CEntityInfo::exec(command,results);
  parseResult(results);
  return true;
}
