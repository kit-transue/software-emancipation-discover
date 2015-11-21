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

#include "ChangeVariablePermissionsAction.h"
#include "QueryResult.h"

CChangeVariablePermissionsAction::CChangeVariablePermissionsAction():CImpactAction("Change Declaration"){
}

CChangeVariablePermissionsAction::~CChangeVariablePermissionsAction() {
}

Widget CChangeVariablePermissionsAction::GetAttributesArea(Widget parent, symbolPtr* pSym) {
	XmString labelStr;
	
	string result;
	string command;

	setWait(true);

	// get current attributes of the entity
	string id = prepareQuery(CEntityInfo::etag(pSym));
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
	

	m_constInd = XtVaCreateManagedWidget("Const",
			xmToggleButtonGadgetClass,fullArea,
			NULL);
	XmToggleButtonGadgetSetState(m_constInd,bConst,true);


	m_staticInd = XtVaCreateManagedWidget("Static",
			xmToggleButtonGadgetClass,fullArea,
			NULL);
	XmToggleButtonGadgetSetState(m_staticInd,bStatic,true);

	setWait(false);
		
	return fullArea;	
}

bool CChangeVariablePermissionsAction::ActionPerformed(symbolPtr* pSym) {
  CImpactAction::ActionPerformed(pSym);
  
  Boolean bStatic = XmToggleButtonGadgetGetState(m_staticInd);
  string szStatic = (bStatic?"1":"0");
  Boolean bConst = XmToggleButtonGadgetGetState(m_constInd);
  string szConst = (bConst?"1":"0");

  string command = "Impact:ChangeVariableDeclaration " + prepareQuery(CEntityInfo::etag(pSym)) + " " +  szConst + " " + szStatic;
  
  string szDesc = GetName()+" to ";
  if(bConst) szDesc += " const";
  if(bStatic) szDesc += " static";

  SetDescription(szDesc);

  string results;
  CEntityInfo::exec(command,results);
  parseResult(results);
  return true;
}
