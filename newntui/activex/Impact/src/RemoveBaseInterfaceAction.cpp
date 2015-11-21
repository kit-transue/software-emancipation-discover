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
// RemoveBaseInterfaceAction.cpp: implementation of the CRemoveBaseInterfaceAction class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Impact.h"
#include "ImpactCtl.h"
#include "RemoveBaseInterfaceAction.h"

#include "SelectorDlg.h"
#include "AttributesQueryResult.h"


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNCREATE(CRemoveBaseInterfaceAction, CImpactAction)
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CRemoveBaseInterfaceAction::CRemoveBaseInterfaceAction() :
	CImpactAction(CString("Remove Base Interface"),NULL,true)
{
}

CRemoveBaseInterfaceAction::CRemoveBaseInterfaceAction(CImpactCtrl* pCtrl,TAcomplishedHash* pRemovedInterfaceHash) :
	CImpactAction(CString("Remove Base Interface"),pCtrl,true)
{
	m_pHash = pRemovedInterfaceHash;
}

CRemoveBaseInterfaceAction::~CRemoveBaseInterfaceAction()
{

}

bool CRemoveBaseInterfaceAction::ActionPerformed(CEntity* pEntity) {
	CString command;
	CString results;
	bool bRes = false;
	if(CImpactAction::ActionPerformed(pEntity)==true) {

		CString id = PrepareQuery(pEntity->GetEtag());

		SetWait(true);

		command = "printformat \"%s\t%s\t%s\" name etag kind;set BASE_INTERFACES [ filter interface [get_super_classes " + id + "]]";
		results = m_pCtrl->ExecCommand(command);
		results = m_pCtrl->ExecPrint(CString("print $BASE_INTERFACES"));
		
		CAttributesQueryResult* pInterfaces = new CAttributesQueryResult(); 
		pInterfaces->parse(results); 	

		
		TActionDataList* pList = NULL;
		if(m_pHash==NULL ||
		   !m_pHash->Lookup(pEntity->GetEtag(),pList))
			pList = NULL;
		if(pInterfaces->getRecordsCount()>0) {
			POSITION iter = pInterfaces->getStartPos();
			while(iter!=NULL) {
				TRecord* pRecord = pInterfaces->getNext(iter);
				bool bImplemented = false;
				if(pList && pList->FindTag(*(*pRecord)[1]))
					bImplemented = true;
			}
		} 
		SetWait(false);

		if(pInterfaces->getRecordsCount()>0) {
			CSelectorDlg dlg(CString("Remove Base Interface"),CString("Base Class"),m_pCtrl);
			dlg.SetQueryResults(pInterfaces);
			
			int nRes = dlg.DoModal();

			if(nRes == IDOK) {
				CString szInterfaceName = dlg.GetSelection();
				CString szInterfaceTag = dlg.GetSelectionTag();

				CString description = GetName();
				description += " " + szInterfaceName;
				SetDescription(description);

				command = "Impact:RemoveBaseInterface " + PrepareQuery(pEntity->GetEtag()) + " " + PrepareQuery(szInterfaceTag);

				results = m_pCtrl->ExecPrint(command);
				ParseResult(results);

				if(m_pHash != NULL) {
					if(pList == NULL) {
						pList = new TActionDataList();
						(*m_pHash)[pEntity->GetEtag()] = pList;
					}	
					CActionData* pData = new CActionData(new CString(pEntity->GetEtag()),new CString(szInterfaceTag));
					pList->AddTail(pData);
					SetData(pData);

					bRes = true;
				} else
					bRes = false;
			}
		} else
			m_pCtrl->MessageBox("Nothing to remove.","Warning",MB_OK|MB_ICONWARNING);
		delete pInterfaces;
	}
	SetExecuted(bRes);
	return bRes;
}

void CRemoveBaseInterfaceAction::Undo() {
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
