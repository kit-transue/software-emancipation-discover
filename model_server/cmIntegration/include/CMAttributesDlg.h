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
#ifndef __CMATTRIBUTESDLG_HEADER_FILE
#define __CMATTRIBUTESDLG_HEADER_FILE

#include "CMCommand.h"
#include "top_widgets.h"
#include "stl.h"

class CCMAttributesDlg {
 public:
  static const int IDOK;
  static const int IDCANCEL;
  static const int IDC_OKALL;

  CCMAttributesDlg(string& header,CCMAttributesList* pAttrList,Widget parent);
  ~CCMAttributesDlg();

  int DoModal();

  Widget* m_WidgetsArr;
 private:
  static void OnOK(Widget dialog,XtPointer client_data, XtPointer call_data);
  static void OnCancel(Widget dialog,XtPointer client_data, XtPointer call_data);
  static void OnOKAll(Widget dialog,XtPointer client_data, XtPointer call_data);
  static int m_nIsOK;
  static Widget m_Dialog;

  void CreateDlgPanel(Widget parent);
  void UpdateData();
  void TurnOffSashTraversal(Widget pane);

  CCMAttributesList* m_pAttrs;
  Widget m_Parent;
  string m_szHeader;
};

#endif //__CMATTRIBUTESDLG_HEADER_FILE
