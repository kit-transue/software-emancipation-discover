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
#ifndef __IMPACT_ACTION_HEADER_FILE
#define __IMPACT_ACTION_HEADER_FILE

class symbolPtr;

#include "ResultsTree.h"
#include "CheckList.h"
#include "EntityInfo.h"
#include "top_widgets.h"

class CImpactAction {
  public :
    CImpactAction(string name);
    virtual ~CImpactAction();
    virtual bool HasAttributes(Widget parent, symbolPtr* pSym);
    string& GetName();
    string& GetDescription() { return m_szDescription; };
    void SetDescription(string& desc) { m_szDescription = desc; };

    void AddCallback(XtCallbackProc callback,XtPointer client_data);
    void RemoveCallback(XtCallbackProc callback);
    TCallbacksList* GetCallbacks() {return &m_Callbacks;};
    bool AskAttributes(Widget parent,const char* title, symbolPtr* pSym);

    virtual Widget GetAttributesArea(Widget parent, symbolPtr* pSym) = 0;
    virtual bool ActionPerformed(symbolPtr* pSym){
      string result;
      CEntityInfo::exec("source_dis impacts.dis",result);
      SetExecuted(true);
      SetDescription(GetName());
      return true; 
    };
  
    void setWait(bool state);
    bool IsWait() { return m_pBusyCursor!=NULL; };

    bool IsModal() { return m_IsOK==-1;};
    void SetModal() { m_IsOK = -1;};
    bool IsOK() { return m_IsOK==1; };
    bool IsExecuted() { return m_bExecuted; };
    void SetExecuted(bool state) { 
      m_bExecuted = state; 
      if(!m_bExecuted) {
	if(m_pResults!=NULL) {
	  delete m_pResults;
	  m_pResults = NULL;
	}
      }
    };
    CResultsTree* getResults() { return m_pResults; };
    static string prepareQuery(const string& query);

 protected:
    void parseResult(string result) { m_pResults = CResultsTree::parse(result); };
    static void PopupErrorDialog(char* text);
    static Widget m_Dialog;
    void TurnOffSashTraversal(Widget pane);
    void CenterDialog(Widget dialog);
 private:
    static void FireEvent(CImpactAction* pThis,int nState);
    static void OkPressed(Widget dialog,XtPointer client_data, XtPointer call_data);
    static void CancelPressed(Widget dialog,XtPointer client_data, XtPointer call_data);
    static int m_IsOK;

    tempCursor* m_pBusyCursor;    // busy cursor push/pop
    string m_szName;
    string m_szDescription;
    bool   m_bExecuted;
    TCallbacksList m_Callbacks;
    CResultsTree* m_pResults;
};
#endif // __IMPACT_ACTION_HEADER_FILE

