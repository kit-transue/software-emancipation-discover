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
#ifndef __CHECKLIST_HEADER_FILE
#define __CHECKLIST_HEADER_FILE

#include <X11/Intrinsic.h>
#include <Xm/Xm.h>

#include "stl.h"

class CCallbackInfo {
 public:
  static const int START;
  static const int END;
  static const int MOVE_UP;
  static const int MOVE_DOWN;
  static const int FOCUS;

  CCallbackInfo(XtCallbackProc callback,XtPointer client_data) : m_Valid(true) {
    m_Callback=callback;
    m_ClientData=client_data;
  };
  XtCallbackProc GetCallback(){return m_Callback;};
  XtPointer GetData(){return m_ClientData;};
  int GetState() { return m_State; };
  void SetState(int state) { m_State = state; };
  Widget GetCallbackWidget() { return m_CallbackWidget; };
  void SetCallbackWidget(Widget widget) { m_CallbackWidget = widget; };
  void Invalidate() { m_Valid = false; };
  void Validate() { m_Valid = true; };
  bool IsValid() { return m_Valid; };
  
 private:
  XtCallbackProc m_Callback;
  XtPointer m_ClientData;
  int m_State;
  Widget m_CallbackWidget;
  bool m_Valid;
};

typedef list<CCallbackInfo*> TCallbacksList;
typedef TCallbacksList::iterator TCallbacksIterator;

class CCheckList {
  static int m_nDoubleClickInterval;
 public:
  CCheckList(Widget parent);
  virtual ~CCheckList();
  void Add(XmString item,bool selected = false);
  int GetCount();
  Widget GetWidget();
  void DeleteAllItems();
  void AddCallback(XtCallbackProc callback,XtPointer client_data);
  void RemoveCallback(XtCallbackProc callback);
  TCallbacksList* GetCallbacks() {return &m_Callbacks;};
  int GetItemNumber(Widget widget);
  void SelectWidget(Widget widget);
  static void FireCallback(Widget widget,CCheckList* pThis,int stat);
 private:
  static void ChangeFocus(Widget widget,XtPointer client_data,XEvent* event);
  static void OnCheckPressed(Widget widget, XtPointer client_data, XtPointer call_data); 
  static void ProcessCheck(XtPointer client_data, XtIntervalId id);
  Widget m_Scroll;
  Widget m_List;
  Widget m_SelectedWidget;
  int m_nWidth;
  TCallbacksList m_Callbacks;
};

#endif //__CHECKLIST_HEADER_FILE
