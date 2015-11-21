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
// RemoveFieldAction.cpp: implementation of the CRemoveFieldAction class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Impact.h"
#include "ImpactCtl.h"
#include "RemoveFieldAction.h"

#include "SelectorDlg.h"
#include "AttributesQueryResult.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNCREATE(CRemoveFieldAction, CImpactAction)
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CRemoveFieldAction::CRemoveFieldAction() :
	CImpactAction(CString("Remove Field"),NULL,true)
{
}

CRemoveFieldAction::CRemoveFieldAction(CImpactCtrl* pCtrl,TAcomplishedHash* pRemovedFieldsHash) :
	CImpactAction(CString("Remove Field"),pCtrl,true)
{
	m_pHash = pRemovedFieldsHash;
}

CRemoveFieldAction::~CRemoveFieldAction()
{
}

bool CRemoveFieldAction::ActionPerformed(CEntity* pEntity) {
	CString command;
	CString results;
	bool bRes = false;
	if(CImpactAction::ActionPerformed(pEntity)==true) {

		CString id = PrepareQuery(pEntity->GetEtag());

		SetWait(true);
		command = "printformat \"%s\t%s\" cname etag; set CLASS_FIELDS [";

		// For Java we also need to retrieve static fields ("uses -var")!
		if(pEntity->GetLanguage().CollateNoCase("JAVA"))  
			command += "sort [set_subtract [set_union [get_member_data " + id + "] [uses -var " + id + 
				"]] [set_copy " + id + "]]";
		else 
			command += "sort [set_subtract [get_member_data " + id + "] [set_copy " + id + "]]";
		command += "]";
		
		results = m_pCtrl->ExecCommand(command);
		results = m_pCtrl->ExecPrint(CString("print $CLASS_FIELDS"));

		CAttributesQueryResult* pFields = new CAttributesQueryResult(); 
		pFields->parse(results); 	

		TActionDataList* pList = NULL;
		if(m_pHash!=NULL && 
		   m_pHash->Lookup(pEntity->GetEtag(),pList)) {
			POSITION ArgsPos = pFields->getStartPos();
			POSITION removedPos = pList->GetHeadPosition();
			while(removedPos!=NULL) {
				CActionData* pData = (CActionData*)pList->GetNext(removedPos);
				POSITION fieldPos = pFields->find(*pData->m_pszDataTag,1);
				if(fieldPos!=NULL) {
					TRecord* pRec = pFields->get(fieldPos);
					pFields->remove(pRec);
					delete pRec;
				}
			}
		}
		SetWait(false);
		if(pFields->getRecordsCount()>0) {	// if we have something to remove - display it
			CSelectorDlg dlg(CString("Remove Field"),CString("Field:"),m_pCtrl);
			dlg.SetQueryResults(pFields);

			int nRes = dlg.DoModal();

			if(nRes == IDOK) {
				CString szFieldName = dlg.GetSelection();
				CString szTag = dlg.GetSelectionTag();

				CString description = GetName();
				description += " " + szFieldName;
				SetDescription(description);
		
				command = "Impact:DeleteVariableFromClass " + PrepareQuery(szTag);

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
		delete pFields;
	}
 	SetExecuted(bRes);
	return bRes;
}

void CRemoveFieldAction::Undo() {
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
