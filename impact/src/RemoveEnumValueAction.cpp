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
#include <Xm/Form.h>

#include "RemoveEnumValueAction.h"
#include "ComboBox.h"

CRemoveEnumValueAction::CRemoveEnumValueAction():CImpactAction("Remove Enum Value"){
	m_ValuesList = NULL;
	m_pValues = NULL;
}

CRemoveEnumValueAction::~CRemoveEnumValueAction(){
	m_ValuesList=NULL;
	if(m_pValues!=NULL) delete m_pValues;
}

Widget CRemoveEnumValueAction::GetAttributesArea(Widget parent, symbolPtr* pSym){
	XmString labelStr;
	Widget area = XtVaCreateWidget("area", 
		xmRowColumnWidgetClass, parent,
		XmNwidth, 300,
		XmNnumColumns, 2,
		XmNpacking, XmPACK_COLUMN,
		XmNorientation, XmVERTICAL,
		NULL);
	string result;
	string command;
	string id = prepareQuery(CEntityInfo::etag(pSym));

	setWait(true);
	CEntityInfo::exec("printformat \"%s\t%s\" cname etag",result);

	command = "uses -evalues " + id;
	CEntityInfo::exec(command,result);
	m_pValues = new CQueryResult(); 
	m_pValues->parse(result); 	
	if(m_pValues->getRecordsCount()>0) {
		labelStr = XmStringCreateLocalized("Value:");
		Widget labelWidget = XtVaCreateManagedWidget("label_1",xmLabelGadgetClass, area,
			XmNwidth, 100,
			XmNlabelString, labelStr,
			XmNtopAttachment,XmATTACH_FORM, 
			XmNleftAttachment,XmATTACH_FORM,
			XmNbottomAttachment,XmATTACH_FORM, 
			NULL);
		XmStringFree(labelStr);
		Widget comboArea = XtVaCreateManagedWidget("formArea",
			xmFormWidgetClass, area,
			XmNtopAttachment,XmATTACH_FORM, 
			XmNleftAttachment,XmATTACH_WIDGET,
			XmNleftWidget, labelWidget,
			XmNrightAttachment,XmATTACH_FORM, 
			XmNbottomAttachment,XmATTACH_FORM, 
			NULL);
		m_ValuesList = XtVaCreateManagedWidget("valueslist",
			        dtComboBoxWidgetClass, comboArea,
				XmNtopAttachment,XmATTACH_FORM, 
				XmNleftAttachment,XmATTACH_FORM, 
				XmNrightAttachment,XmATTACH_FORM, 
				XmNbottomAttachment,XmATTACH_FORM, 
				NULL);
		TRecordSetIterator iter = m_pValues->getStartRecord();
		TRecordSetIterator end = m_pValues->getEndRecord();
		while(iter!=end) {
			TRecord* pRecord = *iter;
			labelStr = XmStringCreateLocalized((char*)(*pRecord)[1]->c_str());
			DtComboBoxAddItem(m_ValuesList, labelStr, 0, false);
			XmStringFree(labelStr);
			iter++;
		}
	} else {
		labelStr = XmStringCreateLocalized("Nothing to remove.");
		XtVaCreateManagedWidget("label_1",xmLabelGadgetClass, area,
			XmNlabelString, labelStr,
			NULL);
		XmStringFree(labelStr);
	}

	setWait(false);
	
	return area;
}

bool CRemoveEnumValueAction::ActionPerformed(symbolPtr* pSym) {
  if(m_pValues->getRecordsCount()>0) {
	CImpactAction::ActionPerformed(pSym);
  
	int selectedPosition;
	XtVaGetValues(m_ValuesList, XmNselectedPosition, &selectedPosition, NULL);

	TRecord* pRecord = m_pValues->getRecord(selectedPosition);
	string command = "Impact:DeleteUnionEnumStructMember " + *(*pRecord)[2];

  	SetDescription(GetName()+string(" ")+*(*pRecord)[1]);

	string results;
	CEntityInfo::exec(command,results);
	parseResult(results);
	return true;
  } else 
	return false;
}
