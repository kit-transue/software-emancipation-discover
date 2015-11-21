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
// ChangeTypeAction.cpp: implementation of the CChangeTypeAction class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Impact.h"
#include "ImpactCtl.h"
#include "ChangeTypeAction.h"
#include "NewTypeDlg.h"
#include "AttributesQueryResult.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNCREATE(CChangeTypeAction, CImpactAction)
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CChangeTypeAction::CChangeTypeAction() :
	CImpactAction(CString("Change Type"),NULL)
{

}

CChangeTypeAction::CChangeTypeAction(CImpactCtrl* pCtrl) :
	CImpactAction(CString("Change Type"),pCtrl)
{

}

CChangeTypeAction::~CChangeTypeAction()
{

}

bool CChangeTypeAction::ActionPerformed(CEntity* pEntity) {
	CString command;
	CString results;
	bool bRes = false;
	if(CImpactAction::ActionPerformed(pEntity)==true) {
		CString type;
		command = "set CURRENT_TYPE [type " + PrepareQuery(pEntity->GetEtag()) + "]";
		results = m_pCtrl->ExecCommand(command);

		CAttributesQueryResult typeResult;
		typeResult.parse(results);
		if(typeResult.getRecordsCount()>0) {
			POSITION iter = typeResult.getStartPos();
			if(iter!=NULL) {
				TRecord* pRecord = typeResult.getNext(iter);
				type = *(*pRecord)[0];
			}
		}

		CNewTypeDlg dlg(m_pCtrl);
		dlg.SetType(type);
		
		int nRes = dlg.DoModal();

		if(nRes == IDOK) {
			CString szType = dlg.GetType();
			CString szNewType = dlg.GetNewType();

			CString description = GetName();
			description += " from " + szType;
			description += " to " + szNewType;
			SetDescription(description);

			command = GetCommand(pEntity,szNewType);

			results = m_pCtrl->ExecPrint(command);
			ParseResult(results);

			bRes = true;
		} else {
			SetExecuted(false);
		}
	}
	return bRes;
}

CString CChangeTypeAction::GetCommand(CEntity *pEntity, CString& newtype)
{
	CString selectTag = PrepareQuery(pEntity->GetEtag());
	CString kind = pEntity->GetKind();

	if(kind.CompareNoCase("field")==0) return "Impact:ChangeVariableType " + selectTag + " " + "{" + newtype + "}";
	else if(kind.CompareNoCase("funct")==0) return "Impact:ChangeFunctionType " + selectTag + " " + "{" + newtype + "}";
	else if(kind.CompareNoCase("var")==0) return "Impact:ChangeVariableType " + selectTag + " " + "{" + newtype + "}";
	return "";
}
