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
// RemoveMethodAction.cpp: implementation of the CRemoveMethodAction class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Impact.h"
#include "ImpactCtl.h"
#include "RemoveMethodAction.h"

#include "SelectorDlg.h"
#include "AttributesQueryResult.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNCREATE(CRemoveMethodAction, CImpactAction)
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CRemoveMethodAction::CRemoveMethodAction() :
	CImpactAction(CString("Remove Method"),NULL,true)
{
}

CRemoveMethodAction::CRemoveMethodAction(CImpactCtrl* pCtrl,TAcomplishedHash* pRemovedMethodsHash) :
	CImpactAction(CString("Remove Method"),pCtrl,true)
{
	m_pHash = pRemovedMethodsHash;
}

CRemoveMethodAction::~CRemoveMethodAction()
{

}

bool CRemoveMethodAction::ActionPerformed(CEntity* pEntity) {
	CString command;
	CString results;
	bool bRes = false;
	if(CImpactAction::ActionPerformed(pEntity)==true) {

        CString id = PrepareQuery(pEntity->GetEtag());

        SetWait(true);
        command = "printformat \"%s\t%s\" cname etag; set CLASS_METHODS [";

        command += "sort [set_subtract [get_member_functions " + id + "] [set_copy " + id + "]]";
		command += "]";
		results = m_pCtrl->ExecCommand(command);
		results = m_pCtrl->ExecPrint(CString("print $CLASS_METHODS"));

        CAttributesQueryResult* pMethods = new CAttributesQueryResult(); 
        pMethods->parse(results);      
		TActionDataList* pList = NULL;
		if(m_pHash!=NULL && 
		   m_pHash->Lookup(pEntity->GetEtag(),pList)) {
			POSITION ArgsPos = pMethods->getStartPos();
			POSITION removedPos = pList->GetHeadPosition();
			while(removedPos!=NULL) {
				CActionData* pData = (CActionData*)pList->GetNext(removedPos);
				POSITION fieldPos = pMethods->find(*pData->m_pszDataTag,1);
				if(fieldPos!=NULL) {
					TRecord* pRec = pMethods->get(fieldPos);
					pMethods->remove(pRec);
					delete pRec;
				}
			}
		}
        SetWait(false);
        if(pMethods->getRecordsCount()>0) {
			CSelectorDlg dlg(CString("Remove Method"),CString("Method:"),m_pCtrl);
			dlg.SetQueryResults(pMethods);
			
			int nRes = dlg.DoModal();

			if(nRes == IDOK) {
				CString szMethodName = dlg.GetSelection();
				CString szTag = dlg.GetSelectionTag();

				CString description = GetName();
				description += " " + szMethodName;
				SetDescription(description);
		
				command = "Impact:DeleteMemberFunction " + PrepareQuery(szTag);

				results = m_pCtrl->ExecPrint(command);
				ParseResult(results);

				if(m_pHash != NULL) {
					if(pList == NULL) {
						pList = new TActionDataList();
						(*m_pHash)[pEntity->GetEtag()] = pList;
					}	
					CActionData* pData = new CActionData(new CString(pEntity->GetEtag()),new CString(szTag));
					pList->AddTail(pData);
					SetData(pData);

					bRes = true;
				} else 
					bRes = false;
			} 
		} else
			m_pCtrl->MessageBox("Nothing to remove.","Warning",MB_OK|MB_ICONWARNING);
		delete pMethods;
	}
	SetExecuted(bRes);
	return bRes;
}

void CRemoveMethodAction::Undo() {
	CActionData* pData = GetData();
	SetData(NULL);

	TActionDataList* pList = NULL;
	if(m_pHash!=NULL && 
	   m_pHash->Lookup(*pData->m_pszEntityTag,pList)) {
		POSITION pos = pList->Find(pData);
		if(pos != NULL)
			pList->RemoveAt(pos);
	}
	delete pData;
}
