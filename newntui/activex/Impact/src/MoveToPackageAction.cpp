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
// MoveToPackageAction.cpp: implementation of the CMoveToPackageAction class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Impact.h"
#include "ImpactCtl.h"
#include "MoveToPackageAction.h"

#include "SelectorDlg.h"
#include "AttributesQueryResult.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNCREATE(CMoveToPackageAction, CImpactAction)
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMoveToPackageAction::CMoveToPackageAction() :
	CImpactAction(CString("Move To Package"),NULL,false)
{
}

CMoveToPackageAction::CMoveToPackageAction(CImpactCtrl* pCtrl,TAcomplishedHash* pPackagesHash) :
	CImpactAction(CString("Move To Package"),pCtrl,false)
{
	m_pHash = pPackagesHash;
}

CMoveToPackageAction::~CMoveToPackageAction()
{

}

bool CMoveToPackageAction::ActionPerformed(CEntity* pEntity) {
	CString command;
	CString results;
	bool bRes = false;
	if(CImpactAction::ActionPerformed(pEntity)==true) {

        CString id = PrepareQuery(pEntity->GetEtag());

        SetWait(true);
	    command = "set PACKAGES [ packages / ] ";
		results = m_pCtrl->ExecCommand(command);
		results = m_pCtrl->ExecPrint(CString("print $PACKAGES"));

        CAttributesQueryResult* pPackages = new CAttributesQueryResult(); 
        pPackages->parse(results);      
		TActionDataList* pList = NULL;

		if(m_pHash!=NULL && 
		   m_pHash->Lookup(pEntity->GetEtag(),pList)) {
			POSITION ArgsPos = pPackages->getStartPos();
			POSITION removedPos = pList->GetHeadPosition();
			while(removedPos!=NULL) {
				CActionData* pData = (CActionData*)pList->GetNext(removedPos);
				POSITION fieldPos = pPackages->find(*pData->m_pszDataTag,1);
				if(fieldPos!=NULL) {
					TRecord* pRec = pPackages->get(fieldPos);
					pPackages->remove(pRec);
					delete pRec;
				}
			}
		}
        SetWait(false);

		// Add the "Any Package" package to the list
		TRecord *anyRec = new TRecord();
		TField *anyField = new TField("Any Package");
		anyRec->Add(anyField);
		pPackages->addHead(anyRec);

        if(pPackages->getRecordsCount()>0) {
			CSelectorDlg dlg(CString("Move To Package"),CString("Package:"),m_pCtrl);
			dlg.SetQueryResults(pPackages);
			
			int nRes = dlg.DoModal();

			if(nRes == IDOK) {
				CString szPackageName = dlg.GetSelection();
				//CString szTag = dlg.GetSelectionTag();

				CString description = GetName();
				description += " " + szPackageName;
				SetDescription(description);
		
				command = "Impact:MoveToPackage " + id + " " + PrepareQuery(szPackageName);

				results = m_pCtrl->ExecPrint(command);
				ParseResult(results);
				
				if(m_pHash!=NULL) {
					if(pList == NULL) {
						pList = new TActionDataList();
						(*m_pHash)[pEntity->GetEtag()] = pList;
					}
					CActionData* pData = new CActionData(new CString(pEntity->GetEtag()),new CString(szPackageName));
					pList->AddTail(pData);
					SetData(pData);

					bRes = true;
				} else
					bRes = false;
			} 
			
		} else
			m_pCtrl->MessageBox("There are no packages to move to.","Warning",MB_OK|MB_ICONWARNING);

		delete pPackages;
		
	}
	SetExecuted(bRes);
	return bRes;
}
