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
// AddBaseClassAction.cpp: implementation of the CAddBaseClassAction class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Impact.h"
#include "AddBaseClassAction.h"
#include "ImpactCtl.h"

#include "SelectorDlg.h"
#include "AttributesQueryResult.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNCREATE(CAddBaseClassAction, CImpactAction)
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CAddBaseClassAction::CAddBaseClassAction() : 
	CImpactAction(CString("Inherit Class"),NULL,true)
{
}

CAddBaseClassAction::CAddBaseClassAction(CImpactCtrl* pCtrl,TAcomplishedHash* pInheritedClassesHash) : 
	CImpactAction(CString("Inherit Class"),pCtrl,true)
{
	m_pHash = pInheritedClassesHash;
}

CAddBaseClassAction::~CAddBaseClassAction()
{

}

bool CAddBaseClassAction::ActionPerformed(CEntity* pEntity) {
	CString results;
	CString command;
	bool bRes = false;
	if(CImpactAction::ActionPerformed(pEntity)==true) {

		SetWait(true);

		// now get all available class except inherited
		CString id = PrepareQuery(pEntity->GetEtag());
		command = CString("source_dis closure.dis;") + 
			      CString("set BASE_CLASSES [sort name [set_subtract") +
					CString(" [set_subtract") +
					  CString(" [set_subtract") +
						CString(" [defines -classes /] [set_copy " + id + "]]") +
					  CString(" [query_closure 50 \"get_super_classes\" " + id + " ]]") +
					CString(" [query_closure 50 \"get_sub_classes\" " + id + " ]]]");
		results = m_pCtrl->ExecCommand(command);
		results = m_pCtrl->ExecPrint(CString("printformat \"%s\t%s\" name etag;print $BASE_CLASSES"));
		CAttributesQueryResult* pAvailable = new CAttributesQueryResult(); 
		pAvailable->parse(results);
		
		TActionDataList* pList = NULL;
		if(m_pHash==NULL || 
		   !m_pHash->Lookup(pEntity->GetEtag(),pList))
			pList = NULL;

		int nCount = pAvailable->getRecordsCount();
		if(nCount>0) {
			POSITION iter = pAvailable->getStartPos();
			CAttributesQueryResult DeletedRecords;
			while(iter!=NULL) {
				TRecord* pRecord = pAvailable->getNext(iter);
				TField* pName = (*pRecord)[0];
				bool bAnon = pName->Find("<anonymous")!=-1;
				bool bUnnamed = pName->Find("<unnamed")!=-1;
				bool bInherited = false;
				if(pList && pList->FindTag(*(*pRecord)[1]))
					bInherited = true;
				if(pName->GetLength()==0 || bAnon || bUnnamed || bInherited)
					DeletedRecords.add(pRecord);
			}
			iter = DeletedRecords.getStartPos();
			while(iter!=NULL) {
				TRecord* pRecord = DeletedRecords.getNext(iter);
				pAvailable->remove(pRecord);
			}
		}

		SetWait(false);

		CSelectorDlg dlg(CString("Inherit Class"),CString("Base Class:"),m_pCtrl);
		dlg.SetQueryResults(pAvailable);
		
		int nRes = dlg.DoModal();

		if(nRes == IDOK) {
			CString szClassName = dlg.GetSelection();
			CString szTag = dlg.GetSelectionTag();

			CString description = GetName();
			description += " " + szClassName;
			SetDescription(description);

			command = "Impact:AddBaseClass " + PrepareQuery(pEntity->GetEtag()) + " " + PrepareQuery(szTag);

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

void CAddBaseClassAction::Undo()
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
