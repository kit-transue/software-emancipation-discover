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
// AddBaseInterfaceAction.cpp: implementation of the CAddBaseInterfaceAction class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Impact.h"
#include "ImpactCtl.h"
#include "AddBaseInterfaceAction.h"

#include "SelectorDlg.h"
#include "AttributesQueryResult.h"


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNCREATE(CAddBaseInterfaceAction, CImpactAction)
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CAddBaseInterfaceAction::CAddBaseInterfaceAction() :
	CImpactAction(CString("Implement Interface"),NULL,true)
{
}

CAddBaseInterfaceAction::CAddBaseInterfaceAction(CImpactCtrl* pCtrl,TAcomplishedHash* pImplementedInterfacesHash) :
	CImpactAction(CString("Implement Interface"),pCtrl,true)
{
	m_pHash = pImplementedInterfacesHash;
}

CAddBaseInterfaceAction::~CAddBaseInterfaceAction()
{

}

bool CAddBaseInterfaceAction::ActionPerformed(CEntity* pEntity) 
{
	CString command;
	CString results;
	bool bRes = false;
	if(CImpactAction::ActionPerformed(pEntity)==true) {

		SetWait(true);

		// lets get the implemented interfaces first
		command = "printformat \"%s\t%s\" etag kind;source_dis closure.dis;set IMPLEMENTED_INTERFACES [query_closure 50 \"get_super_classes\" " + PrepareQuery(pEntity->GetEtag()) + "]";
		results=m_pCtrl->ExecCommand(command);
		results=m_pCtrl->ExecPrint(CString("print $IMPLEMENTED_INTERFACES"));
		CAttributesQueryResult* pImplemented = new CAttributesQueryResult(); 
		pImplemented->parse(results); 	

		TActionDataList* pList = NULL;
		if(m_pHash==NULL || 
		   !m_pHash->Lookup(pEntity->GetEtag(),pList))
			pList = NULL;

		// now get all available interfaces
		command = "printformat \"%s\t%s\" name etag;set DEFINED_INTERFACES [sort name [defines -interfaces /]]";
		results=m_pCtrl->ExecCommand(command);
		results=m_pCtrl->ExecPrint(CString("print $DEFINED_INTERFACES"));
		CAttributesQueryResult* pAvailable = new CAttributesQueryResult(); 
		pAvailable->parse(results); 	
		if(pAvailable->getRecordsCount()>0) {
			POSITION iter = pAvailable->getStartPos();
			CAttributesQueryResult DeletedRecords;
			while(iter!=NULL) {
				TRecord* pRecord = pAvailable->getNext(iter);
				TField* field = (*pRecord)[1];
				bool bImplemented = false;
				if(pList && pList->FindTag(*(*pRecord)[1]))
					bImplemented = true;
				if(pImplemented->contains(*field,0) || bImplemented) 
					DeletedRecords.add(pRecord);
			}
			iter = DeletedRecords.getStartPos();
			while(iter!=NULL) {
				TRecord* pRecord = DeletedRecords.getNext(iter);
				pAvailable->remove(pRecord);
			}
		} else {
			m_pCtrl->MessageBox("Nothing to implement.","Warning",MB_OK|MB_ICONWARNING);
			SetExecuted(false);
			bRes = false;
			return bRes;
		}
		delete pImplemented;

		SetWait(false);

		CSelectorDlg dlg(CString("Add Base Interface"),CString("Base Interface:"),m_pCtrl);
		dlg.SetQueryResults(pAvailable);
		
		int nRes = dlg.DoModal();

		if(nRes == IDOK) {
			CString szInterfaceName = dlg.GetSelection();
			CString szTag = dlg.GetSelectionTag();

			CString description = GetName();
			description += " " + szInterfaceName;
			SetDescription(description);

			command = "Impact:AddBaseInterface " + PrepareQuery(pEntity->GetEtag()) + " " + PrepareQuery(szTag);

			results = m_pCtrl->ExecPrint(command);
			ParseResult(results);

			if(m_pHash!=NULL) {
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
		delete pAvailable;
	}
	SetExecuted(bRes);
	return bRes;
}

void CAddBaseInterfaceAction::Undo()
{
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
