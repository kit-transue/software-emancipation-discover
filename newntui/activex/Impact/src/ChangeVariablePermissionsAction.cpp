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
// ChangeVariablePermissionsAction.cpp: implementation of the CChangeVariablePermissionsAction class.
//
// This class works with AccessCppDlg and AccessCpp
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Impact.h"
#include "ImpactCtl.h"
#include "ChangeVariablePermissionsAction.h"
#include "VariableAccessDlg.h"

#include "AttributesQueryResult.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNCREATE(CChangeVariablePermissionsAction, CImpactAction)
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CChangeVariablePermissionsAction::CChangeVariablePermissionsAction() :
	CImpactAction(CString("Change Declaration"),NULL)
{

}

CChangeVariablePermissionsAction::CChangeVariablePermissionsAction(CImpactCtrl* pCtrl) :
	CImpactAction(CString("Change Declaration"),pCtrl)
{

}

CChangeVariablePermissionsAction::~CChangeVariablePermissionsAction()
{

}

bool CChangeVariablePermissionsAction::ActionPerformed(CEntity* pEntity) {
	CString command;
	CString results;
	bool bRes = false;
	if(CImpactAction::ActionPerformed(pEntity)==true) {

		SetWait(true);

		// get current attributes of the entity
		CString id = PrepareQuery(pEntity->GetEtag());
		command = "source_dis impacts.dis";
		results = m_pCtrl->ExecPrint(command);
		
		bool bConst;
		bool bStatic;

		command = "IsConst " + id;
		results = m_pCtrl->ExecPrint(command);
		if ( results.Find("1") != -1 ) { 
			bConst = true;
		} else { 
			bConst = false;
		}
	
		command = "IsStatic " + id;
		results = m_pCtrl->ExecPrint(command);
		if ( results.Find("1") != -1 ) { 
			bStatic = true;
		} else { 
			bStatic = false;
		}
	

		SetWait(false);

		CVariableAccessDlg dlg(m_pCtrl);
		dlg.SetConst(bConst);
		dlg.SetStatic(bStatic);
		
		int nRes = dlg.DoModal();

		if(nRes == IDOK) {
			CString szPermissions;
			CString szStatic = (dlg.IsStatic()?"1":"0");
			CString szConst = (dlg.IsConst()?"1":"0");
	
			CString description = GetName();
			description += " to ";
			if(dlg.IsStatic()) description += "static ";
			if(dlg.IsConst()) description += "const ";
			SetDescription(description);

			command = "Impact:ChangeVariableDeclaration " + 
						PrepareQuery(pEntity->GetEtag()) + " " + 
						szConst + " " + 
						szStatic;

			results = m_pCtrl->ExecPrint(command);
			ParseResult(results);
			bRes = true;
			SetWait(false);
		} else {
			SetExecuted(false);
		}
	}
	SetExecuted(bRes);
	return bRes;
}
