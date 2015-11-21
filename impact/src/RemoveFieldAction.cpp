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
#include <Xm/RowColumn.h>
#include <Xm/Form.h>

#include "RemoveFieldAction.h"
#include "EntityInfo.h"

CRemoveFieldAction::CRemoveFieldAction():CImpactAction("Remove Field"){
	m_FieldsList = NULL;
	m_pFields = NULL;
}

CRemoveFieldAction::~CRemoveFieldAction() {
	m_FieldsList = NULL;
	if(m_pFields != NULL) delete m_pFields;
}

Widget CRemoveFieldAction::GetAttributesArea(Widget parent, symbolPtr* pSym){
	XmString labelStr;
	Widget area = XtVaCreateWidget("area", 
		xmFormWidgetClass, parent,
		XmNwidth, 300,
		XmNheight, 30,
		XmNnumColumns, 2,
		XmNpacking, XmPACK_COLUMN,
		XmNorientation, XmVERTICAL,
		NULL);
	string result;
	string command;
	string id = prepareQuery(CEntityInfo::etag(pSym));

	setWait(true);
	CEntityInfo::exec("printformat \"%s\t%s\" cname etag",result);

	if(CEntityInfo::language(pSym).compare("JAVA"))  // For Java we also need to retrieve static fields ("uses -var")!
		command = string("sort [set_subtract [set_union [get_member_data ") + id + string("] [uses -var ") + id + 
				string("]] [set_copy ") + id + string("]]");
	else 
		command = string("sort [set_subtract [get_member_data ") + id + string("] [set_copy ") + id + string("]]");
	CEntityInfo::exec(command,result);
	m_pFields = new CQueryResult(); 
	m_pFields->parse(result); 	
	if(m_pFields->getRecordsCount()>0) {	// if we have something to remove - display it
		labelStr = XmStringCreateLocalized("Field:");
		Widget labelWidget = XtVaCreateManagedWidget("label_1",xmLabelGadgetClass, area,
			XmNlabelString, labelStr,
			XmNwidth, 100,
			XmNtopAttachment,XmATTACH_FORM, 
			XmNleftAttachment,XmATTACH_FORM,
			XmNbottomAttachment,XmATTACH_FORM, 
			NULL);
		XmStringFree(labelStr);
		m_FieldsList = XtVaCreateManagedWidget("fieldslist",
			        dtComboBoxWidgetClass, area,
				XmNtopAttachment,XmATTACH_FORM, 
				XmNleftAttachment,XmATTACH_WIDGET,
				XmNleftWidget, labelWidget,
				XmNrightAttachment,XmATTACH_FORM, 
				XmNbottomAttachment,XmATTACH_FORM, 
				NULL);

		TRecordSetIterator iter = m_pFields->getStartRecord();
		TRecordSetIterator end = m_pFields->getEndRecord();
		while(iter!=end) {
			TRecord* pRecord = *iter;
			labelStr = XmStringCreateLocalized((char*)(*pRecord)[1]->c_str());
			DtComboBoxAddItem(m_FieldsList, labelStr, 0, false);
			XmStringFree(labelStr);
			iter++;
		}
	} else { // otherwise display a message
		labelStr = XmStringCreateLocalized("Nothing to remove.");
		Widget labelWidget = XtVaCreateManagedWidget("label_1",xmLabelGadgetClass, area,
			XmNlabelString, labelStr,
			XmNwidth, 100,
			XmNtopAttachment,XmATTACH_FORM, 
			XmNleftAttachment,XmATTACH_FORM,
			XmNbottomAttachment,XmATTACH_FORM, 
			XmNrightAttachment,XmATTACH_FORM, 
			NULL);
		XmStringFree(labelStr);
	}

	setWait(false);
	
	return area;
}

bool CRemoveFieldAction::ActionPerformed(symbolPtr* pSym) {
  if(m_pFields->getRecordsCount()>0) {	// if we have something to remove - display it
	CImpactAction::ActionPerformed(pSym);
  
	int selectedPosition;
	XtVaGetValues(m_FieldsList, XmNselectedPosition, &selectedPosition, NULL);

	TRecord* pRecord = m_pFields->getRecord(selectedPosition);
	string command = "Impact:DeleteVariableFromClass " + prepareQuery(*(*pRecord)[2]);

  	SetDescription(GetName()+string(" ")+*(*pRecord)[1]);

	string results;
	CEntityInfo::exec(command,results);
	parseResult(results);
	return true;
  } else 
	return false;
}











