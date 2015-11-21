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
#include <Xm/Frame.h>
#include <Xm/ToggleBG.h>

#include "ChangeCPPFieldPermissionsAction.h"
#include "QueryResult.h"

CChangeCPPFieldPermissionsAction::CChangeCPPFieldPermissionsAction():CImpactAction("Change Declaration"){
}

CChangeCPPFieldPermissionsAction::~CChangeCPPFieldPermissionsAction() {
}

Widget CChangeCPPFieldPermissionsAction::GetAttributesArea(Widget parent, symbolPtr* pSym) {
	XmString labelStr;
	
	string result;
	string command;

	setWait(true);

	// get current attributes of the entity
	string id = prepareQuery(CEntityInfo::etag(pSym));
	command = string("printformat \"1 %s\t%s\t%s\" public protected private;print ") + id;
	CEntityInfo::exec(command,result);
	CQueryResult Attributes; 
	Attributes.parse(result); 	
	TRecord* pRecord = *Attributes.getStartRecord();
	Boolean bPublic = *(*pRecord)[1]=="1";
	Boolean bProtected = *(*pRecord)[2]=="1";
	Boolean bPrivate = *(*pRecord)[3]=="1";
	Boolean bConst = false;
	Boolean bStatic = false;

	CEntityInfo::exec("source_dis impacts.dis", result);
	
	command = "IsConst " + id;
	CEntityInfo::exec(command, result);
	if (result.find("1") != -1 ) {
		bConst = true;
	}

	command = "IsStatic " + id;
	CEntityInfo::exec(command, result);
	if (result.find("1") != -1 ) {
		bStatic = true;
	}

	Widget fullArea = XtVaCreateWidget("area",
			xmRowColumnWidgetClass, parent,
			XmNwidth, 290,
			NULL);
	

	Widget border = XtVaCreateManagedWidget("etchedBorder",
			xmFrameWidgetClass, fullArea,
			XmNshadowType, XmSHADOW_ETCHED_IN,
			NULL);
	Widget permissionsArea = XmCreateRadioBox(border,"permisArea",NULL,0);
	m_publicInd = XtVaCreateManagedWidget("Public",
			xmToggleButtonGadgetClass,permissionsArea,
			NULL);
	XmToggleButtonGadgetSetState(m_publicInd,bPublic,true);
	XtAddCallback(m_publicInd,XmNvalueChangedCallback,OnPermissionChanged,this);
	m_protectedInd = XtVaCreateManagedWidget("Protected",
			xmToggleButtonGadgetClass,permissionsArea,
			NULL);
	XmToggleButtonGadgetSetState(m_protectedInd,bProtected,true);
	XtAddCallback(m_protectedInd,XmNvalueChangedCallback,OnPermissionChanged,this);
	m_privateInd = XtVaCreateManagedWidget("Private",
			xmToggleButtonGadgetClass,permissionsArea,
			NULL);
	XmToggleButtonGadgetSetState(m_privateInd,bPrivate,true);
	XtAddCallback(m_privateInd,XmNvalueChangedCallback,OnPermissionChanged,this);

	m_constInd = XtVaCreateManagedWidget("Const",
			xmToggleButtonGadgetClass,fullArea,
			NULL);
	XmToggleButtonGadgetSetState(m_constInd,bConst,true);


	m_staticInd = XtVaCreateManagedWidget("Static",
			xmToggleButtonGadgetClass,fullArea,
			NULL);
	XmToggleButtonGadgetSetState(m_staticInd,bStatic,true);

	XtManageChild(permissionsArea);

	setWait(false);
		
	return fullArea;	
}

void CChangeCPPFieldPermissionsAction::OnPermissionChanged(Widget widget,XtPointer client_data,XtPointer call_data) {
	CChangeCPPFieldPermissionsAction* pThis = (CChangeCPPFieldPermissionsAction*)client_data;
	XmToggleButtonCallbackStruct* state = (XmToggleButtonCallbackStruct*) call_data;
	if(state->set) {
		if(widget == pThis->m_publicInd) pThis->setPermission(PUBLIC);
		else if(widget == pThis->m_privateInd) pThis->setPermission(PRIVATE);
		else if(widget == pThis->m_protectedInd) pThis->setPermission(PROTECTED);
		else pThis->setPermission(PACKAGE);
	}
}

bool CChangeCPPFieldPermissionsAction::ActionPerformed(symbolPtr* pSym) {
  CImpactAction::ActionPerformed(pSym);
  
  Boolean bStatic = XmToggleButtonGadgetGetState(m_staticInd);
  string szStatic = (bStatic?"1":"0");
  Boolean bConst = XmToggleButtonGadgetGetState(m_constInd);
  string szConst = (bConst?"1":"0");

  char strPerm[10];
  sprintf(strPerm,"%d", m_nPermission);
  string command = "Impact:ChangeCPPFieldDeclaration " + prepareQuery(CEntityInfo::etag(pSym)) + " " + strPerm + " " + szConst + " " + szStatic;
  
  string szDesc = GetName()+" to ";
  if(bConst) szDesc += " const";
  if(bStatic) szDesc += " static";

  if(m_nPermission==PUBLIC) szDesc += " public";
  else if(m_nPermission==PRIVATE) szDesc += " private";
  else if(m_nPermission==PROTECTED) szDesc += " protected";

  SetDescription(szDesc);

  string results;
  CEntityInfo::exec(command,results);
  parseResult(results);
  return true;
}
