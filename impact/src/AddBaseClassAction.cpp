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

#include "AddBaseClassAction.h"
#include "ComboBox.h"

CAddBaseClassAction::CAddBaseClassAction():CImpactAction("Inherit Class"){
	m_ClassesList = NULL;
  	m_pAvailable = NULL;
}

CAddBaseClassAction::~CAddBaseClassAction() {
	m_ClassesList=NULL;
	if(m_pAvailable != NULL) delete m_pAvailable;
}

Widget CAddBaseClassAction::GetAttributesArea(Widget parent, symbolPtr* pSym){
	XmString labelStr;
	Widget area = XtVaCreateWidget("area", 
		xmFormWidgetClass, parent,
		XmNwidth, 300,
		XmNheight, 30,
		XmNnumColumns, 2,
		XmNpacking, XmPACK_COLUMN,
		XmNorientation, XmVERTICAL,
		NULL);
	labelStr = XmStringCreateLocalized("Class:");
	Widget labelWidget = XtVaCreateManagedWidget("label_1",xmLabelGadgetClass, area,
			XmNlabelString, labelStr,
			XmNwidth, 100,
			XmNtopAttachment,XmATTACH_FORM, 
			XmNleftAttachment,XmATTACH_FORM,
			XmNbottomAttachment,XmATTACH_FORM, 
			NULL);
	XmStringFree(labelStr);
	m_ClassesList = XtVaCreateManagedWidget("classeslist",
			        dtComboBoxWidgetClass, area,
				XmNtopAttachment,XmATTACH_FORM, 
				XmNleftAttachment,XmATTACH_WIDGET,
				XmNleftWidget, labelWidget,
				XmNrightAttachment,XmATTACH_FORM, 
				XmNbottomAttachment,XmATTACH_FORM, 
				NULL);
	string result;
	string command;

	setWait(true);

	// now get all available class except inherited
	string id = prepareQuery(CEntityInfo::etag(pSym));
	command = string("printformat \"%s\t%s\" name etag;source_dis closure.dis;") +
		  string("sort name [set_subtract") +
                    string(" [set_subtract") +
                      string(" [set_subtract") +
                        string(" [defines -classes /] [set_copy " + id + "]]") +
                      string(" [query_closure 50 \"get_super_classes\" " + id + " ]]") +
                    string(" [query_closure 50 \"get_sub_classes\" " + id + " ]]");
	CEntityInfo::exec(command,result);
	m_pAvailable = new CQueryResult(); 
	m_pAvailable->parse(result); 	
	if(m_pAvailable->getRecordsCount()>0) {
		TRecordSetIterator iter = m_pAvailable->getStartRecord();
		TRecordSetIterator end = m_pAvailable->getEndRecord();
		CQueryResult DeletedRecords;
		while(iter!=end) {
			TRecord* pRecord = *iter;
			TField* pName = (*pRecord)[1];
			bool bAnon = pName->find("<anonymous")!=string::npos;
			bool bUnnamed = pName->find("<unnamed")!=string::npos;
			if(pName->length()!=0 && !bAnon && !bUnnamed) {
				labelStr = XmStringCreateLocalized((char*)pName->c_str());
				DtComboBoxAddItem(m_ClassesList, labelStr, 0, false);
				XmStringFree(labelStr);
			} else
				DeletedRecords.add(pRecord);
			iter++;
		}
		iter = DeletedRecords.getStartRecord();
		end = DeletedRecords.getEndRecord();
		while(iter!=end) {
			TRecord* pRecord = *iter;
			m_pAvailable->remove(pRecord);
			iter++;
		}
	}

	setWait(false);

	return area;
}

bool CAddBaseClassAction::ActionPerformed(symbolPtr* pSym) {
  CImpactAction::ActionPerformed(pSym);
  

  int selectedPosition;
  XtVaGetValues(m_ClassesList, XmNselectedPosition, &selectedPosition, NULL);

  TRecord* pRecord = m_pAvailable->getRecord(selectedPosition);

  SetDescription(GetName()+string(" ")+*(*pRecord)[1]);

  string command = "Impact:AddBaseClass " + prepareQuery(CEntityInfo::etag(pSym)) + " " + prepareQuery(*(*pRecord)[2]);

  string results;
  CEntityInfo::exec(command,results);
  parseResult(results);
  return true;
}
