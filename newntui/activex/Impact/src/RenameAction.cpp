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
// RenameAction.cpp: implementation of the CRenameAction class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Impact.h"
#include "RenameAction.h"

#include "ImpactCtl.h"
#include "RenameDlg.h"

#include "stdio.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNCREATE(CRenameAction, CImpactAction)
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CRenameAction::CRenameAction() : CImpactAction(CString("Rename"),NULL)
{
}

CRenameAction::CRenameAction(CImpactCtrl* pCtrl) : CImpactAction(CString("Rename"),pCtrl)
{
}

CRenameAction::~CRenameAction()
{

}

bool CRenameAction::ActionPerformed(CEntity* pEntity) {
	bool bRes = false;
	if(CImpactAction::ActionPerformed(pEntity)==true) {
	
		CRenameDlg dlg(m_pCtrl);
		CString kind = pEntity->GetKind();
		kind.MakeLower();

		if ( kind == "package" ) {
			dlg.SetOriginalName( pEntity->GetName() );

			if(MessageBox(NULL,"Running impact queries on packages may take a long time.\nWould you like to proceed?",
				          "QA Warning",
						  MB_ICONQUESTION|MB_OKCANCEL)!=IDOK) {

				SetExecuted(false);
				bRes = false;
				return false;
			}
		} else {
			dlg.SetOriginalName( pEntity->GetEntityName() );
		}
		int nRes = dlg.DoModal();

		if(nRes == IDOK) {
			CString newname = dlg.GetNewName();

			CString description = GetName();
			description += " " + pEntity->GetKind();
			description += " from " + pEntity->GetEntityName();
			description += " to " + newname;
			SetDescription(description);

			CString command = GetRenameCommand(pEntity, PrepareQuery(newname));

			CString results = m_pCtrl->ExecPrint(command);
			ParseResult(results);

			bRes = true;
		} else {
			SetExecuted(false);
		}
	}
	return bRes;
}

CString CRenameAction::GetRenameCommand(CEntity *pEntity, CString& szNewName)
{
	CString command;
	CString selectTag = PrepareQuery(pEntity->GetEtag());
	CString permissionCode = "1";
  
	if(pEntity->IsPublic()) permissionCode = "0";
	else if(pEntity->IsProtected()) permissionCode = "2";
	else if(pEntity->IsPrivate())   permissionCode = "3";
	
	CString kind = pEntity->GetKind();
	kind.MakeLower();
	if(kind=="module") {
		command+="Impact:ChangeFilename " + selectTag + " " + szNewName;
	} else if((kind=="struct" || kind=="interface") && 
		      (pEntity->GetLanguage().CompareNoCase("CPP")==0 || 
			   pEntity->GetLanguage().CompareNoCase("JAVA")==0 ||
			   pEntity->GetLanguage().CompareNoCase("C")==0)) {
	    command+="Impact:ChangeClassname " + selectTag + " " + szNewName;
	} else if(kind=="union" || kind=="enum") {
		command+="Impact:ChangeUnionEnumStructName " + selectTag + " " + szNewName;
	} else if(kind=="field") {
		command+="Impact:ChangeFieldName " + selectTag + " " + szNewName + " " + permissionCode;
	} else if(kind=="funct") {
		command+="Impact:ChangeFunctionName " + selectTag + " " + szNewName;
	} else if(kind=="typedef") {
		command+="Impact:ChangeTypedefName " + selectTag + " " + szNewName;
	} else if(kind=="macro") {
		command+="Impact:ChangeMacroName " + selectTag + " " + szNewName;
	} else if(kind=="templ") {
		command+="Impact:ChangeTemplateName " + selectTag + " " + szNewName;
	} else if(kind=="var") {
		command+="Impact:ChangeVariableName " + selectTag + " " + szNewName;
	} else if(kind=="package") {
		command+="Impact:RenamePackage " + selectTag + " " + szNewName;
	}
	return command; 
}
