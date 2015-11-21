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
#include <Xm/DialogS.h>
#include <Xm/PanedW.h>
#include <Xm/PushBG.h>
#include <Xm/Form.h>
#include <Xm/Text.h>
#include <Xm/SashP.h>
#include <Xm/RowColumn.h>
#include <Xm/ScrolledW.h>
#include <Xm/Label.h>
#include <Xm/ToggleB.h>

#include "CMAttributesDlg.h"
#include "../../impact/include/ComboBox.h"

const int CCMAttributesDlg::IDOK = 1;
const int CCMAttributesDlg::IDCANCEL = 0;
const int CCMAttributesDlg::IDC_OKALL = 2;

int CCMAttributesDlg::m_nIsOK = -1;
Widget CCMAttributesDlg::m_Dialog;

CCMAttributesDlg::CCMAttributesDlg(string& header,CCMAttributesList* pAttrList,Widget parent) {
	m_pAttrs = pAttrList;
	m_Parent = parent;
	m_WidgetsArr = NULL;
	m_szHeader = header;
}

CCMAttributesDlg::~CCMAttributesDlg() {
	if(m_WidgetsArr!=NULL)	delete m_WidgetsArr;
}

void CCMAttributesDlg::TurnOffSashTraversal(Widget pane) {
	Widget *children;
	int	nChildren;
	
	XtVaGetValues(pane,
		XmNchildren,&children,
		XmNnumChildren, &nChildren,
		NULL);
	while(nChildren-- > 0) {
		if(XmIsSash(children[nChildren]))
			XtVaSetValues(children[nChildren],
				XmNtraversalOn, False,
				NULL);
	}
}

int CCMAttributesDlg::DoModal() {
	m_Dialog = XtVaCreatePopupShell((char*)m_szHeader.c_str(),
			xmDialogShellWidgetClass,m_Parent,
			XmNdeleteResponse,XmDESTROY,
			NULL);
	add_top_widget(m_Dialog);
	Widget dialogForm = XtVaCreateWidget("dialogForm",
				xmFormWidgetClass,m_Dialog,
				XmNdialogStyle, XmDIALOG_FULL_APPLICATION_MODAL,
				XmNwidth, 370,
			NULL);
	Widget pane = XtVaCreateManagedWidget("pane",xmPanedWindowWidgetClass,dialogForm,
			XmNsashWidth,1,
			XmNsashHeight,1,
			XmNleftAttachment,XmATTACH_FORM,
			XmNtopAttachment,XmATTACH_FORM,
			XmNrightAttachment,XmATTACH_FORM,
			XmNbottomAttachment,XmATTACH_FORM,
			NULL);
	Widget rowColumn_w = XtVaCreateManagedWidget("rowcol",xmRowColumnWidgetClass,pane,NULL);
	CreateDlgPanel(rowColumn_w);

	// common area
	Widget form = XtVaCreateManagedWidget("form",xmFormWidgetClass, pane,
			XmNfractionBase, 13,
			NULL);
	Widget okButton = XtVaCreateManagedWidget("OK",
		xmPushButtonGadgetClass, form,
		XmNtopAttachment, XmATTACH_FORM,
		XmNbottomAttachment, XmATTACH_FORM,
		XmNleftAttachment, XmATTACH_POSITION,
		XmNleftPosition, 1,
		XmNrightAttachment, XmATTACH_POSITION,
		XmNrightPosition, 4,
		XmNshowAsDefault, True,
		XmNdefaultButtonShadowThickness, 1,
		NULL);
	XtAddCallback(okButton,XmNactivateCallback,OnOK,this);
	
	Widget okAllButton = XtVaCreateManagedWidget("OK for All",
		xmPushButtonGadgetClass, form,
		XmNtopAttachment, XmATTACH_FORM,
		XmNbottomAttachment, XmATTACH_FORM,
		XmNleftAttachment, XmATTACH_POSITION,
		XmNleftPosition, 5,
		XmNrightAttachment, XmATTACH_POSITION,
		XmNrightPosition, 8,
		XmNshowAsDefault, False,
		XmNdefaultButtonShadowThickness, 1,
		NULL);
	XtAddCallback(okAllButton,XmNactivateCallback,OnOKAll,this);

	Widget cancelButton = XtVaCreateManagedWidget("Cancel",
		xmPushButtonGadgetClass, form,
		XmNtopAttachment, XmATTACH_FORM,
		XmNbottomAttachment, XmATTACH_FORM,
		XmNleftAttachment, XmATTACH_POSITION,
		XmNleftPosition, 9,
		XmNrightAttachment, XmATTACH_POSITION,
		XmNrightPosition, 12,
		XmNshowAsDefault, False,
		XmNdefaultButtonShadowThickness, 1,
		NULL);
	XtAddCallback(cancelButton,XmNactivateCallback,OnCancel,this);
		
	Dimension h;
	XtVaGetValues(cancelButton,XmNheight, &h, NULL);
	XtVaSetValues(form,XmNpaneMaximum, h,XmNpaneMinimum, h, NULL);
	XtManageChild(dialogForm);		
	
	TurnOffSashTraversal(pane);
	Dimension w;
	XtVaGetValues(m_Dialog,XmNheight, &h, XmNwidth, &w, NULL);
	XtVaSetValues(m_Dialog,
		XmNminHeight, h,
		XmNmaxHeight, h,
		NULL);

	m_nIsOK = -1;

	XtAppContext appContext = XtWidgetToApplicationContext(m_Dialog);
	while(m_nIsOK==-1) {
		XtInputMask mask = XtAppPending(appContext);
		if(mask != 0) XtAppProcessEvent(appContext, mask);
	}
	return m_nIsOK;	
}

void CCMAttributesDlg::CreateDlgPanel(Widget parent) {
	if(m_pAttrs!=NULL) {
		int nCount = m_pAttrs->size();
		if(nCount > 0) {
			m_WidgetsArr = new Widget[m_pAttrs->size()];
			CCMAttributesList::iterator iPos = m_pAttrs->begin();
			CCMAttributesList::iterator iEnd = m_pAttrs->end();
			int nPos=0;
			while(iPos!=iEnd) {
				CCMAttribute* pAttr = *iPos;
				if(pAttr!=NULL) {
					int nType = pAttr->GetType();
					if(!pAttr->IsBoolean()) {
						Widget attrWidget = XtVaCreateManagedWidget("attrform", 
							xmFormWidgetClass, parent,
							NULL);
						XtVaCreateManagedWidget(pAttr->GetName().c_str(),
							xmLabelWidgetClass, attrWidget,
							XmNtopAttachment, XmATTACH_FORM,
							XmNbottomAttachment, XmATTACH_FORM,
							XmNleftAttachment, XmATTACH_FORM,
							XmNrightAttachment, XmATTACH_POSITION,
							XmNrightPosition, 30,
							NULL);
						if(nType==CCMAttribute::TEXT) {
							m_WidgetsArr[nPos] = XtVaCreateManagedWidget("text",
									xmTextWidgetClass,attrWidget,
									XmNeditMode, XmSINGLE_LINE_EDIT,
									XmNtopAttachment, XmATTACH_FORM,
									XmNbottomAttachment, XmATTACH_FORM,
									XmNleftAttachment, XmATTACH_POSITION,
									XmNrightAttachment, XmATTACH_FORM,
									XmNleftPosition, 30,
									XmNvalue,pAttr->GetValue().c_str(),
									NULL);
						} else { if(nType==CCMAttribute::MULTILINE_TEXT) {
							Widget scroll_w = XtVaCreateManagedWidget("scrolled_w",
									xmScrolledWindowWidgetClass, attrWidget,
									XmNscrollingPolicy,XmAPPLICATION_DEFINED,
									XmNvisualPolicy,XmVARIABLE,
									XmNscrollBarDisplayPolicy,XmSTATIC,
									XmNtopAttachment, XmATTACH_FORM,
									XmNbottomAttachment, XmATTACH_FORM,
									XmNleftAttachment, XmATTACH_POSITION,
									XmNrightAttachment, XmATTACH_FORM,
									XmNleftPosition, 30,
									XmNshadowThickness,0,
									NULL); 
							m_WidgetsArr[nPos] = XtVaCreateManagedWidget("text",
									xmTextWidgetClass,scroll_w,
									XmNvalue,pAttr->GetValue().c_str(),
									XmNeditMode, XmMULTI_LINE_EDIT,
									NULL);	
						} else { if(nType==CCMAttribute::LIST) {
							m_WidgetsArr[nPos] = XtVaCreateManagedWidget( "cList", 
			        					dtComboBoxWidgetClass, attrWidget,
									XmNtopAttachment, XmATTACH_FORM,
									XmNbottomAttachment, XmATTACH_FORM,
									XmNleftAttachment, XmATTACH_POSITION,
									XmNrightAttachment, XmATTACH_FORM,
									XmNleftPosition, 30,
									NULL);
							XmString labelStr;
							int nElsCount = pAttr->GetListElementsCount();
							string szValue = pAttr->GetValue();
							for(int i=0;i<nElsCount;i++) {
								string szEl = pAttr->GetListElementValue(i);
	 							labelStr = XmStringCreateLocalized((char*)szEl.c_str());
    								DtComboBoxAddItem(m_WidgetsArr[nPos], labelStr, 0, false);
								XmStringFree(labelStr);
							}
							labelStr = XmStringCreateLocalized((char*)szValue.c_str());
							DtComboBoxSelectItem(m_WidgetsArr[nPos], labelStr);
							XmStringFree(labelStr);
						}}}
					} else {
						m_WidgetsArr[nPos] = XtVaCreateManagedWidget(pAttr->GetName().c_str(), 
			        			xmToggleButtonWidgetClass, parent,
							NULL);
						Boolean bSelected = pAttr->GetValue().compare("true")==0;
						XmToggleButtonSetState(m_WidgetsArr[nPos],bSelected,true);
					}
				}
				iPos++;
-				nPos++;				
			}
		}
	}
}

void CCMAttributesDlg::UpdateData() {
	if(m_pAttrs!=NULL) {
		CCMAttributesList::iterator pos = m_pAttrs->begin();
		CCMAttributesList::iterator iEnd = m_pAttrs->end();
		int nPos = 0;
		while(pos!=iEnd) {
			CCMAttribute* pAttr = *pos;
			Widget widget = m_WidgetsArr[nPos];
			int nType = pAttr->GetType();
			if(nType == CCMAttribute::TEXT || nType == CCMAttribute::MULTILINE_TEXT) {
				char* text = XmTextGetString(widget);
				string szValue(text);
				XtFree(text);
				pAttr->SetValue(szValue);
			} else {
				if(pAttr->IsBoolean()) {
					Boolean bSelected = XmToggleButtonGetState(widget);
					string szValue = (bSelected?"true":"false");
					string szKey = pAttr->GetListElementKey(szValue);
					pAttr->SetValue(szKey);
				} else {
					int nItemCount = pAttr->GetListElementsCount();
					if(nItemCount>0) {
						int selPos;
						XtVaGetValues(widget,XmNselectedPosition,&selPos,NULL);
						string szKey = pAttr->GetListElementKey(selPos);
						pAttr->SetValue(szKey);
					}
				}
			}
			nPos++;
			pos++;
		}
	}
}

void CCMAttributesDlg::OnOK(Widget dialog,XtPointer client_data, XtPointer call_data) {
	m_nIsOK = IDOK;
	CCMAttributesDlg* pThis = (CCMAttributesDlg*) client_data;
	
	pThis->UpdateData();

	rem_top_widget(m_Dialog);
	XtDestroyWidget(m_Dialog);
	m_Dialog = NULL;
	delete pThis->m_WidgetsArr;
}

void CCMAttributesDlg::OnOKAll(Widget dialog,XtPointer client_data, XtPointer call_data) {
	m_nIsOK = IDC_OKALL;
	CCMAttributesDlg* pThis = (CCMAttributesDlg*) client_data;

	pThis->UpdateData();

	rem_top_widget(m_Dialog);
	XtDestroyWidget(m_Dialog);
	m_Dialog = NULL;
	delete pThis->m_WidgetsArr;
}

void CCMAttributesDlg::OnCancel(Widget dialog,XtPointer client_data, XtPointer call_data) {
	m_nIsOK = IDCANCEL;
	CCMAttributesDlg* pThis = (CCMAttributesDlg*) client_data;

	rem_top_widget(m_Dialog);
	XtDestroyWidget(m_Dialog);
	m_Dialog = NULL;
	delete pThis->m_WidgetsArr;
}






