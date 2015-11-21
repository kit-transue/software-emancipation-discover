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
// ChangeBodyAction.cpp: implementation of the CChangeBodyAction class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Impact.h"
#include "ImpactCtl.h"
#include "ChangeBodyAction.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNCREATE(CChangeBodyAction, CImpactAction)
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CChangeBodyAction::CChangeBodyAction() :
	CImpactAction(CString("Change Body"),NULL)
{

}

CChangeBodyAction::CChangeBodyAction(CImpactCtrl* pCtrl) :
	CImpactAction(CString("Change Body"),pCtrl)
{

}

CChangeBodyAction::~CChangeBodyAction()
{

}

bool CChangeBodyAction::ActionPerformed(CEntity* pEntity) {
	bool bRes = false;
	if(CImpactAction::ActionPerformed(pEntity)==true) {
			CString description = GetName();
			description += " of " + pEntity->GetKind();
			description += " " + pEntity->GetEntityName();
			SetDescription(description);

			CString command = GetCommand(pEntity);

			CString results = m_pCtrl->ExecPrint(command);
			ParseResult(results);

			bRes = true;
	}
	return bRes;
}

CString CChangeBodyAction::GetCommand(CEntity *pEntity)
{
	CString selectTag = PrepareQuery(pEntity->GetEtag());
	CString kind = pEntity->GetKind();
	if(kind.CompareNoCase("funct")==0) return "Impact:ChangeFunctionBody " + selectTag;
	else if(kind.CompareNoCase("macro")==0) return "Impact:ChangeMacroBody " + selectTag;
	else if(kind.CompareNoCase("templ")==0) return "Impact:ChangeTemplateBody " + selectTag;
	return ""; 
}
