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
#ifndef __IMPACT_COMPONENT_HEADER_FILE
#define  __IMPACT_COMPONENT_HEADER_FILE

#define Object XtObject
#include <Xm/MainW.h>
#include <Xm/Label.h>
#include <Xm/Form.h>
#include <Xm/CascadeB.h>
#include <Xm/PushB.h>
#include <Xm/RowColumn.h>
#include <Xm/PanedW.h>
#include <Xm/List.h>
#undef Object

#include "ImpactActionsSet.h"
#include "CheckList.h"
#include "ComboBox.h"
#include "symbolArr.h"
#include "IMAttachment.h"

typedef vector<CImpactActionsSet*> CEntityActions;

class CImpactComponent : public CIMAttachmentCallbackHandler{
 public:
  CImpactComponent(Widget topmost);
  virtual ~CImpactComponent();
  CImpactActionsSet* GetActions() { return m_pActions; };
  bool SetActions(CImpactActionsSet* actions);
  Widget GetTopWidget() { return XtParent(m_Component); };
  Widget GetComponent() { return m_Component; };
#ifndef __STANDALONE_TEST
  void SetSelectedEntities(symbolArr& entities);
  symbolArr* GetSelectedEntities() { return &m_Entities; };
#endif //__STANDALONE_TEST
  void SetVisible(bool state) { if(state) XtRealizeWidget(m_Component); else XtUnrealizeWidget(m_Component); };	 
  CImpactActionsSet* GetEntityActions(int idx) { return m_EntityActions[idx]; };
  int GetSymbolIdx() { return m_nSymbolIdx; };
  void SetSymbolIdx(int idx) { m_nSymbolIdx = idx; };
  Widget getToDoList() { return m_todoList; };
  Widget getChangesList() { return m_changesList; };
  CResultsTree* getResults() { return m_pResults; };
  void setResults(CResultsTree* pRes) { m_pResults = pRes; };
  void ShowResults(CResultsTree* pResults);
  void ReadyToAttach(int nIsOK);
  void GenerateReport(char* reportName);

  CIMAttachment* m_pAttachment;
 private:
  static void OnActionRequested(Widget list, XtPointer client_data, XtPointer call_data);
  static void OnChangeEntity(Widget list, XtPointer client_data, XtPointer call_data);
  static void OnShowChanges(Widget list, XtPointer client_data, XtPointer call_data);
  static void OnExitActionPerformed(Widget widget, XtPointer client_data, XtPointer call_data);
  static void OnReportActionPerformed(Widget widget, XtPointer client_data, XtPointer call_data);
  static void OnAttachActionPerformed(Widget widget, XtPointer client_data, XtPointer call_data);
  Widget   m_Component;
  Widget   mainMenuBar;
  Widget   statusBar;
  CCheckList*   m_pActionsList;
  CEntityActions m_EntityActions;
  
#ifndef __STANDALONE_TEST
  symbolArr m_Entities;
#endif //__STANDALONE_TEST
  Widget   m_SelectedEntities;
  CImpactActionsSet*  m_pActions;
  Widget   m_todoList;
  Widget   m_changesList;
  int m_nSymbolIdx;
  CResultsTree* m_pResults;
};

#endif //__IMPACT_COMPONENT_HEADER_FILE








