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
// DeleteVariableAction.cpp: implementation of the CDeleteVariableAction class.
// This clas is only used for Deleting Local variables
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Impact.h"
#include "ImpactCtl.h"
#include "DeleteVariableAction.h"

#include "SelectorDlg.h"
#include "AttributesQueryResult.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNCREATE(CDeleteVariableAction, CImpactAction)
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


CDeleteVariableAction::CDeleteVariableAction() :
	CImpactAction(CString("Delete"),NULL,false)
{

}

CDeleteVariableAction::CDeleteVariableAction(CImpactCtrl* pCtrl,TAcomplishedHash* pEntitiesHash) :
	CImpactAction(CString("Delete"),pCtrl,false)
{
	m_pHash = pEntitiesHash;
}

CDeleteVariableAction::~CDeleteVariableAction()
{
	
}

bool CDeleteVariableAction::ActionPerformed(CEntity* pEntity) {
	CString command;
	CString results;
	bool isStatic = false;
	bool bRes = false;
	if(CImpactAction::ActionPerformed(pEntity)==true) {
			CString id = PrepareQuery(pEntity->GetEtag());
			SetWait(true);        
			command = "printformat \"%s\t%s\" name etag;";
			command += "set FUNCTIONS [ GetFunctionsUsingLocal " + id + " ] ";
			results = m_pCtrl->ExecCommand(command);
			results = m_pCtrl->ExecPrint(CString("print $FUNCTIONS"));		        
		
			CAttributesQueryResult* pFunctions;
			TActionDataList* pList;
			pFunctions = new CAttributesQueryResult();
			pFunctions->parse(results);      
			pList = NULL;
			
			
			if( m_pHash!=NULL && 
				m_pHash->Lookup(pEntity->GetEtag(),pList)){
				POSITION ArgsPos = pFunctions->getStartPos();
				POSITION removedPos = pList->GetHeadPosition();
				while(removedPos!=NULL) {
					CActionData* pData = (CActionData*)pList->GetNext(removedPos);
					POSITION fieldPos = pFunctions->find(*pData->m_pszDataTag,1);
					if(fieldPos!=NULL) {
						TRecord* pRec = pFunctions->get(fieldPos);
						pFunctions->remove(pRec);
						delete pRec;
					}
				}
			}
	
			if(pFunctions->getRecordsCount()>0 ){				
				CSelectorDlg dlg(CString("Delete Local Variable"),CString("Functions:"),m_pCtrl);
				dlg.SetQueryResults(pFunctions);
				
				int nRes = dlg.DoModal();
				if(nRes == IDOK) {
					CString szFunctionName = dlg.GetSelection();
					CString szTag = dlg.GetSelectionTag();
					CString description = GetName();
					description += " " + pEntity->GetName() + " from " + szTag;
					SetDescription(description);				
					command = "Impact:DeleteLocalVariable " + id + " " + PrepareQuery(szTag);
		
				} else {
					command = "";
				}
			} else {
				m_pCtrl->MessageBox("Deleting this variable requires changing the functions arguments.  Please run Change Arguments instead.","Error",MB_OK|MB_ICONWARNING);
				SetWait(false);
				bRes = false;
				SetExecuted(false);
				return bRes;
			}
		}
		if ( command != "" ) { 
	
			results = m_pCtrl->ExecPrint(command);
			ParseResult(results);
			bRes = true;
			SetWait(false);
		} else {
			bRes = false;
			SetWait(false);
		}
	
	SetExecuted(bRes);
	return bRes;
}

