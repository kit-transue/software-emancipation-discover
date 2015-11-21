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
// ChangeBaseClassAction.cpp: implementation of the CChangeBaseClassAction class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Impact.h"
#include "ImpactCtl.h"
#include "ChangeBaseClassAction.h"

#include "SelectorDlg.h"
#include "AttributesQueryResult.h"


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNCREATE(CChangeBaseClassAction, CImpactAction)
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CChangeBaseClassAction::CChangeBaseClassAction() :
	CImpactAction(CString("Change Base Class"),NULL)
{

}

CChangeBaseClassAction::CChangeBaseClassAction(CImpactCtrl* pCtrl) :
	CImpactAction(CString("Change Base Class"),pCtrl)
{

}

CChangeBaseClassAction::~CChangeBaseClassAction()
{

}

bool CChangeBaseClassAction::ActionPerformed(CEntity* pEntity) {
	CString command;
	CString results;
	bool bRes = false;
	if(CImpactAction::ActionPerformed(pEntity)==true) {

		SetWait(true);
		
		CString id = PrepareQuery(pEntity->GetEtag());
		command = "printformat \"%s\t%s\t%s\" name etag final;source_dis closure.dis; set BASE_CLASSES [sort name [set_subtract [set_subtract [defines -classes /] [set_copy " + id + "]]" +
					  " [query_closure 50 \"get_sub_classes\" " + id + "]]]";
		results = m_pCtrl->ExecPrint(command);
		results = m_pCtrl->ExecPrint(CString("print $BASE_CLASSES"));
		CAttributesQueryResult* pClasses= new CAttributesQueryResult();
		pClasses->parse(results);
		if(pClasses->getRecordsCount()>0) {
			POSITION iter = pClasses->getStartPos();
			CAttributesQueryResult DeletedRecords;
			while(iter!=NULL) {
				TRecord* pRecord = pClasses->getNext(iter);
				TField* pName = (*pRecord)[0]; 
				bool bFinal = (*pRecord)[2]->CompareNoCase("1")==0;
				bool bAnon = pName->Find("<anonymous")!=-1;
				bool bUnnamed = pName->Find("<unnamed")!=-1;
				if(bFinal || pName->GetLength()==0 ||
				   (*pRecord)[1]->GetLength()==0 || bAnon || bUnnamed) 
					DeletedRecords.add(pRecord);
			}
			iter = DeletedRecords.getStartPos();
			while(iter!=NULL) {
				TRecord* pRecord = DeletedRecords.getNext(iter);
				pClasses->remove(pRecord);
			}
		}

		SetWait(false);

		CSelectorDlg dlg(CString("Change Base Class"),CString("Base Class"),m_pCtrl);
		dlg.SetQueryResults(pClasses);
		
		int nRes = dlg.DoModal();

		if(nRes == IDOK) {
			CString szClassName = dlg.GetSelection();
			CString szTag = dlg.GetSelectionTag();

			CString description = GetName();
			description += " to " + szClassName;
			SetDescription(description);

			command = "Impact:ChangeBaseClass " + PrepareQuery(pEntity->GetEtag()) + " " + PrepareQuery(szTag);

			results = m_pCtrl->ExecPrint(command);
			ParseResult(results);

			bRes = true;
		} else {
			SetExecuted(false);
		}
		delete pClasses;
	}
	return bRes;
}
