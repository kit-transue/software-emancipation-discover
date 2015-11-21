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

#include "RemoveBaseInterfaceAction.h"
#include "ComboBox.h"

CRemoveBaseInterfaceAction::CRemoveBaseInterfaceAction():CImpactAction("Remove Base Interface"){
	m_InterfacesList = NULL;
	m_pInterfaces = NULL;
}

CRemoveBaseInterfaceAction::~CRemoveBaseInterfaceAction() {
	m_InterfacesList=NULL;
	if(m_pInterfaces != NULL) delete m_pInterfaces;
}

Widget CRemoveBaseInterfaceAction::GetAttributesArea(Widget parent, symbolPtr* pSym){
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
	CEntityInfo::exec("printformat \"%s\t%s\t%s\" etag kind name",result);

	command = "filter interface [ get_super_classes " + id + " ]";
	CEntityInfo::exec(command,result);
	m_pInterfaces = new CQueryResult(); 
	m_pInterfaces->parse(result); 	
	if(m_pInterfaces->getRecordsCount()>0 ) {
		labelStr = XmStringCreateLocalized("Interface:");
		Widget labelWidget = XtVaCreateManagedWidget("label_1",xmLabelGadgetClass, area,
			XmNlabelString, labelStr,
			XmNwidth, 100,
			XmNtopAttachment,XmATTACH_FORM, 
			XmNleftAttachment,XmATTACH_FORM,
			XmNbottomAttachment,XmATTACH_FORM, 
			NULL);
		XmStringFree(labelStr);
		m_InterfacesList = XtVaCreateManagedWidget("interfaceslist",
			        dtComboBoxWidgetClass, area,
				XmNtopAttachment,XmATTACH_FORM, 
				XmNleftAttachment,XmATTACH_WIDGET,
				XmNleftWidget, labelWidget,
				XmNrightAttachment,XmATTACH_FORM, 
				XmNbottomAttachment,XmATTACH_FORM, 
				NULL);

		TRecordSetIterator iter = m_pInterfaces->getStartRecord();
		TRecordSetIterator end = m_pInterfaces->getEndRecord();
		while(iter!=end) {
			TRecord* pRecord = *iter;
			labelStr = XmStringCreateLocalized((char*)(*pRecord)[3]->c_str());
			DtComboBoxAddItem(m_InterfacesList, labelStr, 0, false);
			XmStringFree(labelStr);
			iter++;
		}
	} else {
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

bool CRemoveBaseInterfaceAction::ActionPerformed(symbolPtr* pSym) {
  if(m_pInterfaces->getRecordsCount()>0) {
	CImpactAction::ActionPerformed(pSym);
  
	int selectedPosition;
	XtVaGetValues(m_InterfacesList, XmNselectedPosition, &selectedPosition, NULL);

	TRecord* pRecord = m_pInterfaces->getRecord(selectedPosition);
	string command = "Impact:RemoveBaseInterface " + prepareQuery(CEntityInfo::etag(pSym)) + " " + prepareQuery(*(*pRecord)[1]);

  	SetDescription(GetName()+string(" ")+*(*pRecord)[3]);

	string results;
	CEntityInfo::exec(command,results);
	parseResult(results);
	return true;
  } else 
	return false;
}







