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
#include <Xm/ScrolledW.h>
#include <Xm/Form.h>
#include <Xm/RowColumn.h>
#include <Xm/Text.h>
#include <Xm/Label.h>

#include "ArgumentsTable.h"

const char* CArgumentsTable::NAME_FIELD = "NameField";
const char* CArgumentsTable::TYPE_FIELD = "TypeField";
   
CArgumentsTable::CArgumentsTable(Widget parent,int width,int height) {
	m_nWidth = width-10;
	Widget formWidget = XtVaCreateWidget("form", 
		xmFormWidgetClass, parent,
		XmNwidth, m_nWidth + 10,
		XmNheight, height,
		NULL);
	m_Form = formWidget;

	Widget headerWidget = XtVaCreateManagedWidget("Argument Type",
		xmLabelWidgetClass, formWidget,
		XmNtopAttachment, XmATTACH_FORM,
		XmNleftAttachment, XmATTACH_FORM,
		XmNrightAttachment, XmATTACH_POSITION,
		XmNrightPosition, 50,
		NULL);
	XtVaCreateManagedWidget("Argument Name",
		xmLabelWidgetClass, formWidget,
		XmNtopAttachment, XmATTACH_FORM,
		XmNrightAttachment, XmATTACH_FORM,
		XmNleftAttachment, XmATTACH_POSITION,
		XmNleftPosition, 50,
		NULL);
	m_Scroll = XtVaCreateManagedWidget("scroll",
		xmScrolledWindowWidgetClass, formWidget,
 		XmNscrollingPolicy, XmAUTOMATIC,
		XmNtopAttachment,XmATTACH_WIDGET,
		XmNtopWidget, headerWidget, 
		XmNleftAttachment,XmATTACH_FORM,
		XmNrightAttachment,XmATTACH_FORM, 
		XmNbottomAttachment,XmATTACH_FORM, 
		NULL);
	m_Table = XtVaCreateManagedWidget("area", 
		xmRowColumnWidgetClass, m_Scroll,
		NULL);
}

CArgumentsTable::~CArgumentsTable() {
}

void CArgumentsTable::OnCellChanged(Widget textW,XtPointer client_data, XtPointer call_data) {
	CArgumentsTable* table = (CArgumentsTable*) client_data;
	char* fieldType = XtName(textW);
	if(!strcmp(fieldType,TYPE_FIELD)) {		
    		int numchild = table->GetRowsCount();
		char* szRow = XtName(XtParent(textW));
		int nRow = atoi(szRow);
		char* text = XmTextGetString(textW);
		if(nRow==(numchild-1) && strlen(text))
			table->Add(NULL);
		XtFree(text);
	} else {
	}
	XmProcessTraversal( textW, XmTRAVERSE_NEXT_TAB_GROUP);
} 

int CArgumentsTable::GetRowsCount() {
	Cardinal nChildren = 0;
	XtVaGetValues(m_Table, XmNnumChildren, &nChildren, NULL);
	return (int)nChildren-1;
}

CArgument CArgumentsTable::Get(int idx) {
	CArgumentFields* fields = m_Arguments[idx];
	Widget nameWidget = fields->m_name;
	Widget typeWidget = fields->m_type;

  	char* name = XmTextGetString(nameWidget);
        char* type = XmTextGetString(typeWidget);
	CArgument arg(type,name);
	XtFree(name);
	XtFree(type);
	return arg;
}

void CArgumentsTable::Add(CArgument* arg) {
	string szName;
	string szType;
	if(arg!=NULL) {
		szName = arg->GetName();
		szType = arg->GetType();
	}
	int nNumRows = GetRowsCount();
	char szRow[10];
	sprintf(szRow,"%d",nNumRows);
	Widget form = XtVaCreateWidget(szRow,
		xmFormWidgetClass, m_Table,
		XmNwidth, m_nWidth,
		NULL);
	Widget typeWidget = XtVaCreateManagedWidget(TYPE_FIELD,
		xmTextWidgetClass, form,
		XmNtopAttachment, XmATTACH_FORM,
		XmNleftAttachment, XmATTACH_FORM,
		XmNbottomAttachment, XmATTACH_FORM,
		XmNrightAttachment, XmATTACH_POSITION,
		XmNrightPosition, 50,
		XmNvalue, szType.c_str(),
		NULL);
	XtAddCallback(typeWidget,XmNactivateCallback, OnCellChanged,(XtPointer)this);
	Widget nameWidget = XtVaCreateManagedWidget(NAME_FIELD,
		xmTextWidgetClass, form,
		XmNtopAttachment, XmATTACH_FORM,
		XmNrightAttachment, XmATTACH_FORM,
		XmNbottomAttachment, XmATTACH_FORM,
		XmNleftAttachment, XmATTACH_POSITION,
		XmNleftPosition, 50,
		XmNvalue, szName.c_str(),
		NULL);
	XtAddCallback(nameWidget,XmNactivateCallback, OnCellChanged,(XtPointer)this);
	m_Arguments.push_back(new CArgumentFields(typeWidget,nameWidget));
	XtManageChild(form);
}

Widget CArgumentsTable::GetWidget() {
	return m_Form;
}

