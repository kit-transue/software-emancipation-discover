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

#include <Xm/LabelG.h>
#include <Xm/Text.h>
#include <Xm/Frame.h>
#include <Xm/RowColumn.h>
#include <Xm/Form.h>
#include "DeleteEntityAction.h"
#include "EntityInfo.h"

CDeleteEntityAction::CDeleteEntityAction():CImpactAction("Delete"){
	m_FieldsList = NULL;
	m_pFields = NULL;
}

CDeleteEntityAction::~CDeleteEntityAction() {
	m_FieldsList = NULL;
	if(m_pFields != NULL) delete m_pFields;
}

Widget CDeleteEntityAction::GetAttributesArea(Widget parent, symbolPtr* pSym) {

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
	labelStr = XmStringCreateLocalized("Running impact queries on packages may take a long time.\nWould you like to proceed?");
	Widget label = XtVaCreateManagedWidget("label_1", xmLabelGadgetClass, oldArea, XmNlabelString, labelStr, NULL);
	XmStringFree(labelStr);

	return area;
}
	
bool CDeleteEntityAction::HasAttributes(Widget parent, symbolPtr* pSym) {
	if (CEntityInfo::kind(pSym) == "package") {
		return true;
	} else {
		return false;
	}
}



bool CDeleteEntityAction::ActionPerformed(symbolPtr* pSym) {

	CImpactAction::ActionPerformed(pSym);
	int selectedPosition;
	string id = prepareQuery(CEntityInfo::etag(pSym));
	string command;
	string results;

	
	if (CEntityInfo::kind(pSym) == "var") {
		command = "oo_member " + id;
		CEntityInfo::exec(command, results);
		if (results.find("1") != -1) {
			command = "Impact:DeleteVariableFromClass " + id;
		} else {
			command = "Impact:DeleteVariable " + id + " [where defined " + id + " ] " ;
		}		
	} else {
		command = "Impact:DeleteEntity " + id;
	}
  
  	SetDescription(GetName()+string(" ")+ id);
	CEntityInfo::exec(command,results);
	parseResult(results);
	return true;
}











