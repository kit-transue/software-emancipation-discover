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
// ChangeFunctionPermissionsJavaAction.cpp: implementation of the CChangeFunctionPermissionsJavaAction class.
//
// This class works with FunctionAccessJavaDlg and FunctionAccessJava.  
// This should be renamed to ChangeFunctionPermissionsJavaAction
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Impact.h"
#include "ImpactCtl.h"
#include "ChangeFunctionPermissionsJavaAction.h"
#include "FunctionAccessJavaDlg.h"

#include "AttributesQueryResult.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNCREATE(CChangeFunctionPermissionsJavaAction, CImpactAction)
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CChangeFunctionPermissionsJavaAction::CChangeFunctionPermissionsJavaAction() :
	CImpactAction(CString("Change Declaration"),NULL)
{

}

CChangeFunctionPermissionsJavaAction::CChangeFunctionPermissionsJavaAction(CImpactCtrl* pCtrl) :
	CImpactAction(CString("Change Declaration"),pCtrl)
{

}

CChangeFunctionPermissionsJavaAction::~CChangeFunctionPermissionsJavaAction()
{

}

bool CChangeFunctionPermissionsJavaAction::ActionPerformed(CEntity* pEntity) {
	CString command;
	CString results;
	bool bRes = false;
	if(CImpactAction::ActionPerformed(pEntity)==true) {

		SetWait(true);

		// get current attributes of the entity
		CString id = PrepareQuery(pEntity->GetEtag());
		command = "printformat \"%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\" public protected package_prot private abstract final static native;print " + id;
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

CString CChangeFunctionPermissionsJavaAction::GetUserInput(CEntity* pEntity, TRecord* pRecord, CString id)
{

	// Check to see if the class is final.  If so we need to prohibit 
	// the user from deselecting the final button
	m_pCtrl->ExecCommand(CString("source_dis impacts.dis"));
	CString results = m_pCtrl->ExecCommand(CString("final [ GetClassOf " + id + " ] "));
	boolean classIsFinal = false;
	if (results.Compare("1") == 0) {
		classIsFinal = true;
	}

	int nPermission = -1;
	if (*(*pRecord)[0]=="1") nPermission = CFunctionAccessJava::PUBLIC;
	else if (*(*pRecord)[1]=="1") nPermission = CFunctionAccessJava::PROTECTED;
	else if (*(*pRecord)[2]=="1") nPermission = CFunctionAccessJava::PACKAGE;
	else if (*(*pRecord)[3]=="1") nPermission = CFunctionAccessJava::PRIVATE;
	else nPermission = CFunctionAccessJava::PACKAGE;
	bool bAbstract = *(*pRecord)[4]=="1"; 
	bool bFinal = *(*pRecord)[5]=="1";
	if (classIsFinal) {
		bFinal = true;
	}
	bool bStatic = *(*pRecord)[6]=="1";
	bool bNative = *(*pRecord)[7]=="1";
	
	CFunctionAccessJavaDlg dlg(m_pCtrl);
	dlg.SetAbstract(bAbstract);
	dlg.SetFinal(bFinal);		
	dlg.SetStatic(bStatic);	
	dlg.SetNative(bNative);
	
	if(nPermission!=-1)
		dlg.SetAccess(nPermission);
	
	int nRes = dlg.DoModal();

	if(nRes == IDOK) {
		if (dlg.IsAbstract() && dlg.IsFinal()) {
			m_pCtrl->MessageBox("A function cannot be both abstract and final.","Warning",MB_OK|MB_ICONWARNING);
			return GetUserInput(pEntity, pRecord, id);
		}
		if (dlg.IsAbstract() && dlg.IsStatic()) {
			m_pCtrl->MessageBox("A function cannot be both abstract and static.","Warning",MB_OK|MB_ICONWARNING);
			return GetUserInput(pEntity, pRecord, id);
		}
		if (dlg.IsAbstract() && dlg.IsNative()) {
			m_pCtrl->MessageBox("A function cannot be both abstract and native.","Warning",MB_OK|MB_ICONWARNING);
			return GetUserInput(pEntity, pRecord, id);
		}
		if (dlg.IsAbstract() && (dlg.GetAccess() == CFunctionAccessJava::PRIVATE)) {
			m_pCtrl->MessageBox("A function cannot be both abstract and private.","Warning",MB_OK|MB_ICONWARNING);
			return GetUserInput(pEntity, pRecord, id);
		}
		if (classIsFinal && !dlg.IsFinal()) {
			m_pCtrl->MessageBox("The class that this function belongs to is final.  You cannot make this function non-final.","Warning",MB_OK|MB_ICONWARNING);
			return GetUserInput(pEntity, pRecord, id);
		}

		CString szPermissions;
		szPermissions.Format("%d",dlg.GetAccess());
		CString szFinal = (dlg.IsFinal()?"1":"0");
		CString szAbstract = (dlg.IsAbstract()?"1":"0");
		CString szNative = (dlg.IsNative()?"1":"0");
		CString szStatic = (dlg.IsStatic()?"1":"0");
	
		CString description = GetName();
		description += " to ";
		if(dlg.IsFinal()) description += "final ";
		if(dlg.IsAbstract()) description += "abstract ";
		if(dlg.IsNative()) description += "native ";
		if(dlg.IsStatic()) description += "static ";
		description += dlg.GetAccessString();
		description += " ";
		SetDescription(description);

		return ("Impact:ChangeJavaFunctionDeclaration " + 
					PrepareQuery(pEntity->GetEtag()) + " " + 
					szPermissions + " " + 
					szAbstract + " " + 
					szFinal + " " + 
					szStatic + " " + 
					szNative);

	} else {
		SetExecuted(false);
		return "";
	}


}
