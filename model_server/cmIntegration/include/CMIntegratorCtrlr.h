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
#ifndef __CMINTEGRATORCONTROLLER_HEADER_FILE
#define __CMINTEGRATORCONTROLLER_HEADER_FILE

#include "CMIntegrator.h"
#include "CMCommand.h"
#include "top_widgets.h"

class CCMIntegratorCtrlr {
 public:
  CCMIntegratorCtrlr();
  ~CCMIntegratorCtrlr();
  static CCMIntegratorCtrlr* getInstance();

  bool ConnectToCM();
  int SelectCM(Widget parent);
  int OnCMCommand(Widget parent,char* cmd,string** projnames,string** filenames);
  int OnCMCommand(Widget parent,CCMCommand *pCommand,TStringList &filesList);
  void setWait(bool state);
  bool AutoCO(string const &comment, string const &filename);
  //bool AutoCO(string const &comment, array<string const &filename>);  //someday.

  CCMIntegrator *m_CMIntegrator;
  string m_szCMName;
  CCMAttributes* m_pCMAttributes;
  CCMCommandsList* m_pCMCommands;
 private:
  void TurnOffSashTraversal(Widget pane);
  void CenterDialog(Widget dialog);

  static void OkPressed(Widget dialog,XtPointer client_data, XtPointer call_data);
  static void CancelPressed(Widget dialog,XtPointer client_data, XtPointer call_data);
  static void OnSelectCM(Widget list, XtPointer client_data, XtPointer call_data);
  
  tempCursor* m_pBusyCursor;    // busy cursor push/pop
  
  static Widget m_CMSelectDialog;
  static int m_IsOK;

  Widget   m_CMSelectionWidget;
  static CCMIntegratorCtrlr* theInstance;
};

#endif //__CMINTEGRATORCONTROLLER_HEADER_FILE
