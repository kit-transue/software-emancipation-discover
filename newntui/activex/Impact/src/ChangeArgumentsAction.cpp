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
// ChangeArgumentsAction.cpp: implementation of the CChangeArgumentsAction class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Impact.h"
#include "ImpactCtl.h"
#include "ChangeArgumentsAction.h"

#include "ArgumentsInfoDlg.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNCREATE(CChangeArgumentsAction, CImpactAction)
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CChangeArgumentsAction::CChangeArgumentsAction() :
	CImpactAction(CString("Change Arguments"),NULL)
{

}

CChangeArgumentsAction::CChangeArgumentsAction(CImpactCtrl* pCtrl) :
	CImpactAction(CString("Change Arguments"),pCtrl)
{

}

CChangeArgumentsAction::~CChangeArgumentsAction()
{

}

bool CChangeArgumentsAction::ActionPerformed(CEntity* pEntity) {
	CString command;
	CString results;
	bool bRes = false;
	if(CImpactAction::ActionPerformed(pEntity)==true) {
		CString command = "args " + PrepareQuery(pEntity->GetEtag());
		results = m_pCtrl->ExecCommand(command);
		CArguments Args;

		Args.Parse(results);

		CArgumentsInfoDlg dlg(m_pCtrl);
		dlg.SetArguments(&Args);
		
		int nRes = dlg.DoModal();

		if(nRes == IDOK) {
			CString szArguments = "(";
			CArguments* pArgs = dlg.GetArguments();
			POSITION nPos = pArgs->GetHeadPosition();
			while(nPos!=NULL) {
				CArgument* pArg = pArgs->GetNext(nPos);
				szArguments += pArg->GetType();
				szArguments += " ";
				szArguments += pArg->GetName();
				if(nPos!=NULL)
					szArguments += ",";
			}
			szArguments += ")";

			CString description = GetName();
			description += " to " + szArguments;
			SetDescription(description);

			command = "Impact:ChangeFunctionArguments " + PrepareQuery(pEntity->GetEtag()) + " " + PrepareQuery(szArguments);

			results = m_pCtrl->ExecPrint(command);
			ParseResult(results);

			bRes = true;
		} else {
			SetExecuted(false);
		}
	}
	return bRes;
}
