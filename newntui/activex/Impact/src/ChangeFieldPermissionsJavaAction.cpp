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
// ChangeFieldPermissionsJavaAction.cpp: implementation of the CChangeFieldPermissionsJavaAction class.
//
// This class works with AccessJavaDlg and AccessJava.  
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Impact.h"
#include "ImpactCtl.h"
#include "ChangeFieldPermissionsJavaAction.h"
#include "AccessJavaDlg.h"



#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNCREATE(CChangeFieldPermissionsJavaAction, CImpactAction)
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CChangeFieldPermissionsJavaAction::CChangeFieldPermissionsJavaAction() :
	CImpactAction(CString("Change Declaration"),NULL)
{

}

CChangeFieldPermissionsJavaAction::CChangeFieldPermissionsJavaAction(CImpactCtrl* pCtrl) :
	CImpactAction(CString("Change Declaration"),pCtrl)
{

}

CChangeFieldPermissionsJavaAction::~CChangeFieldPermissionsJavaAction()
{

}

bool CChangeFieldPermissionsJavaAction::ActionPerformed(CEntity* pEntity) {
	CString command;
	CString results;
	bool bRes = false;
	if(CImpactAction::ActionPerformed(pEntity)==true) {
		SetWait(true);
		// get current attributes of the entity
		CString id = PrepareQuery(pEntity->GetEtag());
		command = "printformat \"%s\t%s\t%s\t%s\t%s\t%s\" public protected package_prot private final static;print " + id;
		results = m_pCtrl->ExecPrint(command);
		CAttributesQueryResult Attributes; 
		Attributes.parse(results); 	
		TRecord* pRecord = Attributes.getRecord(0);
		
		command = GetUserInput(pEntity, pRecord, id);
		if (command.Compare("")!=0) {
			results = m_pCtrl->ExecPrint(command);
			ParseResult(results);
			bRes = true;
		} else {
			bRes = false;
		}
	}
	SetWait(false);
	SetExecuted(bRes);
	return bRes;
}

CString CChangeFieldPermissionsJavaAction::GetUserInput(CEntity* pEntity, TRecord* pRecord, CString id) 
{

	int nPermission = -1;
	if (*(*pRecord)[0]=="1") nPermission = CAccessJava::PUBLIC;
	else if (*(*pRecord)[1]=="1") nPermission = CAccessJava::PROTECTED;
	else if (*(*pRecord)[2]=="1") nPermission = CAccessJava::PACKAGE;
	else if (*(*pRecord)[3]=="1") nPermission = CAccessJava::PRIVATE;
	else nPermission = CAccessJava::PACKAGE;
	bool bFinal = *(*pRecord)[4]=="1";
	bool bStatic = *(*pRecord)[5]=="1";
		
	CAccessJavaDlg dlg(m_pCtrl);
	dlg.SetFinal(bFinal);
	dlg.SetStatic(bStatic);
	if(nPermission!=-1) {
		dlg.SetAccess(nPermission);
	}

	CString tempCommand = "source_dis impacts.dis";
	bool isDeclaredInInnerClass = false;
	m_pCtrl->ExecCommand(tempCommand);
	
	tempCommand = "IsInnerClass [GetClassOf " + id + " ]" ;
	CString declaredInInner = m_pCtrl->ExecCommand(tempCommand);

	if (declaredInInner.Find("1") != - 1) {
		isDeclaredInInnerClass = true;
	}

	
	int nRes = dlg.DoModal();
	if(nRes == IDOK) {
				
		if (dlg.IsStatic() && isDeclaredInInnerClass && pEntity->GetLanguage().CompareNoCase("JAVA")==0) {
			m_pCtrl->MessageBox("You cannot make a field declared in an inner class static.","Warning",MB_OK|MB_ICONWARNING);
			return GetUserInput(pEntity, pRecord, id);
		}


		CString szPermissions;
		szPermissions.Format("%d",dlg.GetAccess());
		CString szFinal = (dlg.IsFinal()?"1":"0");			
		CString szStatic = (dlg.IsStatic()?"1":"0");			
	
		CString description = GetName();
		description += " to ";
		if(dlg.IsFinal()) description += "final ";
		if(dlg.IsStatic()) description += "static ";
		description += dlg.GetAccessString();
		SetDescription(description);
		
		return ("Impact:ChangeJavaFieldDeclaration " + 
						id + " " + 
						szPermissions + " " + 
						szFinal + " " + 
						szStatic);
		
	} else {
		SetExecuted(false);
		return "";
	}
}
