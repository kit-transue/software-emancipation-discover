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
#include <Xm/Form.h>
#include <Xm/LabelG.h>

#include "DeleteVariableAction.h"
#include "ImpactAction.h"
#include "ComboBox.h"


CDeleteVariableAction::CDeleteVariableAction():CImpactAction("Delete"){
	m_ClassesList = NULL;
	m_pClasses = NULL;
}

CDeleteVariableAction::~CDeleteVariableAction() {
	m_ClassesList=NULL;
	if(m_pClasses != NULL) delete m_pClasses;
}

Widget CDeleteVariableAction::GetAttributesArea(Widget parent, symbolPtr* pSym){

	XmString labelStr;
	Widget area = XtVaCreateWidget("area", 
		xmFormWidgetClass, parent,
		XmNwidth, 450,
		XmNheight, 30,
		XmNnumColumns, 2,
		XmNpacking, XmPACK_COLUMN,
		XmNorientation, XmVERTICAL,
		NULL);
	string result;
	string command;
	string id = prepareQuery(CEntityInfo::etag(pSym));
	setWait(true);

	CEntityInfo::exec("printformat \"%s\t%s\" name etag;", result);
	command = string("GetFunctionsUsingLocal " + id);
       
	CEntityInfo::exec("source_dis impacts.dis", result);
	CEntityInfo::exec(command,result);
	m_pClasses = new CQueryResult(); 

	string cleanResult;
	cleanResult = m_pClasses->cleanUpResult(result, "(funct)");

	if (cleanResult.size() >= 1) {
		m_pClasses->parse(cleanResult); 	
		if(m_pClasses->getRecordsCount()>0 ) {
			labelStr = XmStringCreateLocalized("Functions:");
			Widget labelWidget = XtVaCreateManagedWidget("label_1",xmLabelGadgetClass, area,
				XmNlabelString, labelStr,
				XmNwidth, 100,
				XmNtopAttachment,XmATTACH_FORM, 
				XmNleftAttachment,XmATTACH_FORM,
				XmNbottomAttachment,XmATTACH_FORM, 
				NULL);
			XmStringFree(labelStr);
			m_ClassesList = XtVaCreateManagedWidget("Functions",
				        dtComboBoxWidgetClass, area,
					XmNtopAttachment,XmATTACH_FORM, 
					XmNleftAttachment,XmATTACH_WIDGET,
					XmNleftWidget, labelWidget,
					XmNrightAttachment,XmATTACH_FORM, 
					XmNbottomAttachment,XmATTACH_FORM, 
					NULL);

			TRecordSetIterator iter = m_pClasses->getStartRecord();
			TRecordSetIterator end = m_pClasses->getEndRecord();
			while(iter!=end) {
				TRecord* pRecord = *iter;
				labelStr = XmStringCreateLocalized((char*)(*pRecord)[1]->c_str());
				DtComboBoxAddItem(m_ClassesList, labelStr, 0, false);
				XmStringFree(labelStr);
				iter++;
			} //while
			setWait(false);
			return area;
		}
		setWait(false);
	}  else {
		XmString labelStr;
	 	labelStr = XmStringCreateLocalized("Deleting this variable requires changing the the functions arguments.\nPlease run Change Arguments instead.");
		Widget labelWidget = 
			XtVaCreateManagedWidget("label_1", xmLabelGadgetClass, area, 		
			XmNlabelString, labelStr,
			XmNwidth, 150,
			XmNtopAttachment, XmATTACH_FORM,
			XmNleftAttachment, XmATTACH_FORM,
			XmNbottomAttachment, XmATTACH_FORM,
			XmNrightAttachment, XmATTACH_FORM,
			NULL);
		XmStringFree(labelStr);
	}

	setWait(false);
	return area;
}
	
bool CDeleteVariableAction::ActionPerformed(symbolPtr* pSym) {
	if(m_pClasses->getRecordsCount()>0) {
		CImpactAction::ActionPerformed(pSym);
		int selectedPosition;
		XtVaGetValues(m_ClassesList, XmNselectedPosition, &selectedPosition, NULL);
	
		TRecord* pRecord = m_pClasses->getRecord(selectedPosition);
		string command;
		string result;
		command = "Impact:DeleteLocalVariable " + prepareQuery(CEntityInfo::etag(pSym)) + " {" + (*pRecord)[2]->c_str() + "} " ;
		CEntityInfo::exec(command,result);
		
		
  		SetDescription(GetName()+string(" "));
		parseResult(result);
 		return true;
	
	}
	return false;
}
