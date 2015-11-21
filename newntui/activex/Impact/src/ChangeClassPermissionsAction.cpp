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
// ChangeClassPermissionsAction.cpp: implementation of the CChangeClassPermissionsAction class.
//
// This class works with ClassAccessDlg and ClassAccess.  
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Impact.h"
#include "ImpactCtl.h"
#include "ChangeClassPermissionsAction.h"
#include "ClassAccessDlg.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNCREATE(CChangeClassPermissionsAction, CImpactAction)
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CChangeClassPermissionsAction::CChangeClassPermissionsAction() :
	CImpactAction(CString("Change Declaration"),NULL)
{

}

CChangeClassPermissionsAction::CChangeClassPermissionsAction(CImpactCtrl* pCtrl) :
	CImpactAction(CString("Change Declaration"),pCtrl)
{

}

CChangeClassPermissionsAction::~CChangeClassPermissionsAction()
{

}

bool CChangeClassPermissionsAction::ActionPerformed(CEntity* pEntity) {
	CString command;
	CString results;
	bool bRes = false;
	if(CImpactAction::ActionPerformed(pEntity)==true) {

		SetWait(true);

		// get current attributes of the entity
		CString id = PrepareQuery(pEntity->GetEtag());
		command = "printformat \"%s\t%s\t%s\t%s\t%s\t%s\t%s\" public protected package_prot private abstract final static;print " + id;
		results = m_pCtrl->ExecPrint(command);
		CAttributesQueryResult Attributes; 
		Attributes.parse(results); 	
		TRecord* pRecord = Attributes.getRecord(0);
		results = GetUserInput(pEntity, pRecord, id);
		if (results.Compare("")!=0){		
			ParseResult(results);
			bRes = true;
			SetWait(false);
		} else {
			bRes = false;
		}		
	}
	SetExecuted(bRes);
	return bRes;
}


CString CChangeClassPermissionsAction::GetUserInput(CEntity* pEntity, TRecord* pRecord, CString id) {

	int nPermission = -1;
	if (*(*pRecord)[0]=="1") nPermission = CClassAccess::PUBLIC;
	else if (*(*pRecord)[1]=="1") nPermission = CClassAccess::PROTECTED;
	else if (*(*pRecord)[2]=="1") nPermission = CClassAccess::PACKAGE;
	else if (*(*pRecord)[3]=="1") nPermission = CClassAccess::PRIVATE;
	else nPermission = CClassAccess::PACKAGE;
	bool bAbstract = *(*pRecord)[4]=="1";
	bool bFinal = *(*pRecord)[5]=="1";
	bool bStatic = *(*pRecord)[6]=="1";
	SetWait(false);

	CClassAccessDlg dlg(m_pCtrl);
	dlg.SetFinal(bFinal);
	dlg.SetAbstract(bAbstract);
	dlg.SetStatic(bStatic);
	if(nPermission!=-1)
		dlg.SetAccess(nPermission);
	
	int nRes = dlg.DoModal();
	if(nRes == IDOK) {
		CString szPermissions;
		szPermissions.Format("%d",dlg.GetAccess());
		CString szFinal = (dlg.IsFinal()?"1":"0");
		CString szAbstract = (dlg.IsAbstract()?"1":"0");
		CString szStatic = (dlg.IsStatic()?"1":"0");
	
		if ( dlg.IsFinal() && dlg.IsAbstract() ) {
			m_pCtrl->MessageBox("A class cannot be both abstract and final.","Error",MB_OK|MB_ICONWARNING);
			return GetUserInput(pEntity, pRecord, id);
		} else {
			CString description = GetName();
			description += " to ";
			if(dlg.IsFinal()) description += "final ";
			if(dlg.IsAbstract()) description += "abstract ";
			if(dlg.IsStatic()) description += "static ";
			description += dlg.GetAccessString();
			description += " ";
			SetDescription(description);
			CString command =  "Impact:ChangeClassDeclaration " + 
						PrepareQuery(pEntity->GetEtag()) + " " + 
						szPermissions + " " + 
						szAbstract + " " + 
						szFinal + " " + 
						szStatic;
			
			CString results = m_pCtrl->ExecPrint(command);
			
			if (results.Compare("")==0) {
				m_pCtrl->MessageBox("This class cannot be made Protected, Private, or Static.","Error",MB_OK|MB_ICONWARNING);
				return GetUserInput(pEntity, pRecord, id);
			} else {
				return results;
			}
		}
	} else {
		return "";
	}
}
