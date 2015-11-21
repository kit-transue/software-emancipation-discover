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

#include "ChangeBaseClassAction.h"
#include "ComboBox.h"

CChangeBaseClassAction::CChangeBaseClassAction():CImpactAction("Change Base Class"){
	m_ClassesList = NULL;
	m_pClasses = NULL;
}

CChangeBaseClassAction::~CChangeBaseClassAction() {
	m_ClassesList=NULL;
	if(m_pClasses!=NULL) delete m_pClasses;
}

Widget CChangeBaseClassAction::GetAttributesArea(Widget parent, symbolPtr* pSym){
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
	string id = prepareQuery(CEntityInfo::etag(pSym));

	setWait(true);
	
	CEntityInfo::exec("printformat \"%s\t%s\t%s\" name etag final",result);
	command = "source_dis closure.dis; sort name [set_subtract [set_subtract [defines -classes /] [set_copy " + id + "]]" +
                  " [query_closure 50 \"get_sub_classes\" " + id + "]]";
	CEntityInfo::exec(command,result);
	m_pClasses= new CQueryResult();
	m_pClasses->parse(result);
	if(m_pClasses->getRecordsCount()>0) {
		TRecordSetIterator iter = m_pClasses->getStartRecord();
		TRecordSetIterator end = m_pClasses->getEndRecord();
		CQueryResult DeletedRecords;
		while(iter!=end) {
			TRecord* pRecord = *iter;
			TField* pName = (*pRecord)[1]; 
			bool bFinal = (*pRecord)[3]->compare("1")==0;
			bool bAnon = pName->find("<anonymous")!=string::npos;
			bool bUnnamed = pName->find("<unnamed")!=string::npos;
			if(!bFinal && pName->length()!=0 && 
			   (*pRecord)[2]->length()!=0 && !bAnon && !bUnnamed) {
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
			m_pClasses->remove(pRecord);
			iter++;
		}
	}

	setWait(false);
	
	return area;
}

bool CChangeBaseClassAction::ActionPerformed(symbolPtr* pSym) {
  CImpactAction::ActionPerformed(pSym);
  
  int selectedPosition;
  XtVaGetValues(m_ClassesList, XmNselectedPosition, &selectedPosition, NULL);

  TRecord* pRecord = m_pClasses->getRecord(selectedPosition);
  string command = "Impact:ChangeBaseClass " + prepareQuery(CEntityInfo::etag(pSym)) + " " + prepareQuery(*(*pRecord)[2]);
  SetDescription(GetName()+" to " + *(*pRecord)[1]);

  string results;
  CEntityInfo::exec(command,results);
  parseResult(results);
  return true;
}

