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
#include <Xm/LabelG.h>
#include <Xm/Text.h>

#include "AddEnumValueAction.h"

CAddEnumValueAction::CAddEnumValueAction():CImpactAction("Add Enum Value"){
}

CAddEnumValueAction::~CAddEnumValueAction() {
}

Widget CAddEnumValueAction::GetAttributesArea(Widget parent, symbolPtr* pSym) {
	XmString labelStr;
	Widget fullArea = XtVaCreateWidget("area",
			xmRowColumnWidgetClass, parent,
			NULL);
	
	Widget border = XtVaCreateManagedWidget("etchedBorder",
			xmFrameWidgetClass, fullArea,
			XmNshadowType, XmSHADOW_ETCHED_IN,
			NULL);
 
	Widget nameArea = XtVaCreateManagedWidget("nameArea", 
		xmRowColumnWidgetClass, border,
		XmNnumColumns, 2,
		XmNpacking, XmPACK_COLUMN,
		XmNorientation, XmVERTICAL,
		NULL);
	labelStr = XmStringCreateLocalized("Name:");
	Widget label = XtVaCreateManagedWidget("label_1",xmLabelGadgetClass, nameArea,
			XmNlabelString, labelStr,
			NULL);
	XmStringFree(labelStr);
	m_nameField = XtVaCreateManagedWidget("name",
		xmTextWidgetClass, nameArea,
		NULL);
		
	return fullArea;	
}

bool CAddEnumValueAction::ActionPerformed(symbolPtr* pSym) {
  CImpactAction::ActionPerformed(pSym);
  
  string newname;
  char* text = XmTextGetString(m_nameField);
  newname = text;
  XtFree(text);

  SetDescription(GetName()+string(" ")+newname);
  string command = "Impact:AddUnionEnumStructMember " + prepareQuery(CEntityInfo::etag(pSym)) +
                        " " + newname;
  string results;
  CEntityInfo::exec(command,results);
  parseResult(results);
  return true;
}
