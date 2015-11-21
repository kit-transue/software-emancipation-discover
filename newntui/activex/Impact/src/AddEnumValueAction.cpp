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
// AddEnumValueAction.cpp: implementation of the CAddEnumValueAction class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Impact.h"
#include "ImpactCtl.h"
#include "AddEnumValueAction.h"

#include "EnumValueDlg.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNCREATE(CAddEnumValueAction, CImpactAction)
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CAddEnumValueAction::CAddEnumValueAction() :
	CImpactAction(CString("Add Enum Value"),NULL,true)
{

}

CAddEnumValueAction::CAddEnumValueAction(CImpactCtrl* pCtrl) :
	CImpactAction(CString("Add Enum Value"),pCtrl,true)
{

}

CAddEnumValueAction::~CAddEnumValueAction()
{

}

bool CAddEnumValueAction::ActionPerformed(CEntity* pEntity) 
{
	bool bRes = false;
	if(CImpactAction::ActionPerformed(pEntity)==true) {

		CEnumValueDlg dlg(m_pCtrl);
		
		int nRes = dlg.DoModal();

		if(nRes == IDOK) {
			CString szValName = dlg.GetValueName();

			CString description = GetName();
			description += " " + szValName;
			SetDescription(description);

			CString command = "Impact:AddUnionEnumStructMember " + PrepareQuery(pEntity->GetEtag()) +
                        " " + PrepareQuery(szValName);

			CString results = m_pCtrl->ExecPrint(command);
			ParseResult(results);

			bRes = true;
		} else {
			SetExecuted(false);
		}
	}
	return bRes;
}
