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
// ChangeFieldPermissionsCppAction.cpp: implementation of the CChangeFieldPermissionsCppAction class.
//
// This class works with AccessCppDlg and AccessCpp
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Impact.h"
#include "ImpactCtl.h"
#include "ChangeFieldPermissionsCppAction.h"
#include "AccessCppDlg.h"

#include "AttributesQueryResult.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNCREATE(CChangeFieldPermissionsCppAction, CImpactAction)
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CChangeFieldPermissionsCppAction::CChangeFieldPermissionsCppAction() :
	CImpactAction(CString("Change Declaration"),NULL)
{

}

CChangeFieldPermissionsCppAction::CChangeFieldPermissionsCppAction(CImpactCtrl* pCtrl) :
	CImpactAction(CString("Change Declaration"),pCtrl)
{

}

CChangeFieldPermissionsCppAction::~CChangeFieldPermissionsCppAction()
{

}

bool CChangeFieldPermissionsCppAction::ActionPerformed(CEntity* pEntity) {
	CString command;
	CString results;
	bool bRes = false;
	if(CImpactAction::ActionPerformed(pEntity)==true) {

		SetWait(true);

		// get current attributes of the entity
		CString id = PrepareQuery(pEntity->GetEtag());
		command = "source_dis impacts.dis; printformat \"%s\t%s\t%s\" public protected private;print " + id;
		results = m_pCtrl->ExecPrint(command);
		CAttributesQueryResult Attributes; 
		Attributes.parse(results); 	
		TRecord* pRecord = Attributes.getRecord(0);
		
		int nPermission = -1;
		int nPermission2 = -1;
		if (*(*pRecord)[0]=="1") nPermission = CAccessCpp::PUBLIC;
		else if (*(*pRecord)[1]=="1") nPermission = CAccessCpp::PROTECTED;
		else if (*(*pRecord)[2]=="1") nPermission = CAccessCpp::PRIVATE;

		
		bool bConst;
		bool bStatic;

		command = "IsConst " + id;
		results = m_pCtrl->ExecCommand(command);
		if ( results.Find("1") != -1 ) { 
			bConst = true;
		} else { 
			bConst = false;
		}
	
		command = "IsStatic " + id;
		results = m_pCtrl->ExecCommand(command);
		if ( results.Find("1") != -1 ) { 
			bStatic = true;
		} else { 
			bStatic = false;
		}
	

		SetWait(false);

		CAccessCppDlg dlg(m_pCtrl);
		dlg.SetConst(bConst);
		dlg.SetStatic(bStatic);
		
		if(nPermission!=-1) {
			dlg.SetAccess(nPermission);
		}
		
		int nRes = dlg.DoModal();

		if(nRes == IDOK) {
			CString szPermissions;
			szPermissions.Format("%d",dlg.GetAccess());
			CString szStatic = (dlg.IsStatic()?"1":"0");
			CString szConst = (dlg.IsConst()?"1":"0");
	
			CString description = GetName();
			description += " to ";
			if(dlg.IsStatic()) description += "static ";
			if(dlg.IsConst()) description += "const ";
			description += dlg.GetAccessString();
			SetDescription(description);

			command = "Impact:ChangeCPPFieldDeclaration " + 
						PrepareQuery(pEntity->GetEtag()) + " " + 
						szPermissions + " " + 
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
