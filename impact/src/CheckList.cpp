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
#include <Xm/DrawingA.h>
#include <Xm/RowColumn.h>
#include <Xm/ToggleB.h>
#include <Xm/LabelG.h>
#include <Xm/Form.h>

#define XK_MISCELLANY
#include <X11/keysymdef.h>

#include <stdio.h>

#include "CheckList.h"

const int CCallbackInfo::START = 0;
const int CCallbackInfo::END = 1;
const int CCallbackInfo::MOVE_UP = 2;
const int CCallbackInfo::MOVE_DOWN = 3;
const int CCallbackInfo::FOCUS = 4;

int CCheckList::m_nDoubleClickInterval = -1;

CCheckList::CCheckList(Widget parent) {
	if(m_nDoubleClickInterval < 0)
		m_nDoubleClickInterval = XtGetMultiClickTime(XtDisplay(parent));

	m_Scroll = XtVaCreateWidget("scroll",
		xmScrolledWindowWidgetClass, parent,
 		XmNscrollingPolicy, XmAUTOMATIC,
		NULL);
	m_List = XtVaCreateManagedWidget("list", 
		xmRowColumnWidgetClass, m_Scroll,
		XmNnumColumns, 1, 
		XmNtraversalOn, True,
		NULL);
}

CCheckList::~CCheckList() {
	TCallbacksIterator i;
	for(i=m_Callbacks.begin();i!=m_Callbacks.end();i++) {
		delete *i;
	}
}

int CCheckList::GetCount() {
	Cardinal nChildren = 0;
	XtVaGetValues(m_List, XmNnumChildren, &nChildren, NULL);
	return (int)nChildren;
}

Widget CCheckList::GetWidget() {
	return m_Scroll;
}

void CCheckList::AddCallback(XtCallbackProc callback,XtPointer client_data) {
	m_Callbacks.push_back(new CCallbackInfo(callback,client_data));
}

void CCheckList::RemoveCallback(XtCallbackProc callbackToRemove) {	
	TCallbacksIterator i;	
	for(i=m_Callbacks.begin();i!=m_Callbacks.end();i++) {
		CCallbackInfo* info = (CCallbackInfo*)*i;
		XtCallbackProc callback = info->GetCallback();
		if(callback == callbackToRemove) {
			m_Callbacks.remove(info);
		}
	}
}

void CCheckList::FireCallback(Widget widget,CCheckList* pThis,int stat) {
	TCallbacksList* pCallbacks = pThis->GetCallbacks();
	TCallbacksIterator i;	
	for(i=pCallbacks->begin();i!=pCallbacks->end();i++) {
		CCallbackInfo* info = (CCallbackInfo*)*i;
		XtCallbackProc callback = info->GetCallback();
		if(callback) {
			info->SetState(stat);
			callback(widget,info->GetData(),info);
		}
	}
}

static int click_count = 0;

class CClickData {
public:
	CClickData(Widget w,CCheckList* list,int d) : widget(w),
			pThis(list),doubleClick(d){};
	Widget widget;
	CCheckList* pThis;
	int doubleClick;
};

void CCheckList::ProcessCheck(XtPointer client_data, XtIntervalId id) {
	CClickData* pData = (CClickData*)client_data;
	
	if(pData->doubleClick == 1) 
		FireCallback(pData->widget,pData->pThis,CCallbackInfo::START);	
	delete pData;
	click_count=0;
}

void CCheckList::OnCheckPressed(Widget widget, XtPointer client_data, XtPointer call_data) {
	static XtIntervalId id;
	XmToggleButtonCallbackStruct* state = (XmToggleButtonCallbackStruct*)call_data;
	CCheckList* pThis = (CCheckList*)client_data;
	XtVaSetValues(widget,XmNset,!state->set,NULL);
	if(click_count == 0) {
		FireCallback(widget,pThis,CCallbackInfo::FOCUS);	
		id = XtAppAddTimeOut(XtWidgetToApplicationContext(widget),m_nDoubleClickInterval,
					(XtTimerCallbackProc)ProcessCheck,new CClickData(widget,pThis,0));
		click_count++;
	} else {
		if(click_count == 1) {
			XtRemoveTimeOut(id);
			ProcessCheck(new CClickData(widget,pThis,1),id);
		}
	}
}

int CCheckList::GetItemNumber(Widget widget) {
	Widget parent = XtParent(widget);
	char* szRow = XtName(parent);
	return atoi(szRow);
}

void CCheckList::DeleteAllItems() {
	if(m_List!=NULL)
		XtDestroyWidget(m_List);
	m_List = XtVaCreateManagedWidget("list", 
		xmRowColumnWidgetClass, m_Scroll,
		XmNnumColumns, 1, 
		XmNtraversalOn, True,
		NULL);
}

static char *xm_string_to_string(XmString cs) {
	XmStringContext context;
	XmStringCharSet charset;
	XmStringDirection direction;
	Boolean separator;
	static char *primitive_string;

	XmStringInitContext (&context,cs);
	XmStringGetNextSegment (context,&primitive_string,&charset,&direction,&separator);
	XmStringFreeContext (context);
	return ((char *) primitive_string);
}

void CCheckList::ChangeFocus(Widget widget,XtPointer client_data,XEvent* event) {
	Modifiers modifs;
	KeySym keysym = XtGetActionKeysym(event,&modifs);
	bool shift = modifs&ShiftMask;
	
	if (event->type == KeyPress) {
		CCheckList* pThis = (CCheckList*)client_data;
 	
		if(/*keysym == XK_Down || */(keysym==XK_Tab && !shift)) {
			FireCallback(widget,pThis,CCallbackInfo::MOVE_DOWN);	
		} else 
			if(/*keysym == XK_Up || */(keysym==XK_Tab && shift)) {
				FireCallback(widget,pThis,CCallbackInfo::MOVE_UP);	
			}
	}
}

void CCheckList::Add(XmString item, bool selected) {
	int nNumRows = GetCount();
	char szRow[30];
	
	sprintf(szRow,"%d",nNumRows);
	Widget form = XtVaCreateWidget(szRow,
		xmFormWidgetClass, m_List,
		XmNtraversalOn, True,
		NULL);
	Widget checkWidget = XtVaCreateManagedWidget(xm_string_to_string(item),
		xmToggleButtonWidgetClass, form,
		XmNtopAttachment, XmATTACH_FORM,
		XmNleftAttachment, XmATTACH_FORM,
		XmNrightAttachment, XmATTACH_FORM,
		XmNbottomAttachment, XmATTACH_FORM,
		XmNalignment, XmALIGNMENT_BEGINNING,
		XmNset, (selected?1:0),
		NULL);
	XtAddCallback(checkWidget,XmNvalueChangedCallback,OnCheckPressed,(XtPointer)this);
	XtAddEventHandler(checkWidget,KeyPressMask|KeyReleaseMask,False,(XtEventHandler)ChangeFocus,(XtPointer)this);
	XtManageChild(form);
}


