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
// AddMethodAction.cpp: implementation of the CAddMethodAction class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Impact.h"
#include "ImpactCtl.h"
#include "AddMethodAction.h"

#include "MethodInfoDlg.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNCREATE(CAddMethodAction, CImpactAction)
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CAddMethodAction::CAddMethodAction() :
	CImpactAction(CString("Add Method"),NULL,true)
{

}

CAddMethodAction::CAddMethodAction(CImpactCtrl* pCtrl) :
	CImpactAction(CString("Add Method"),pCtrl,true)
{

}

CAddMethodAction::~CAddMethodAction()
{

}

bool CAddMethodAction::ActionPerformed(CEntity* pEntity) 
{
	bool bRes = false;
	if(CImpactAction::ActionPerformed(pEntity)==true) {

		CArguments Args;

		CMethodInfoDlg dlg(m_pCtrl);
		dlg.SetArguments(&Args);

		int nRes = dlg.DoModal();

		if(nRes == IDOK) {
			CString szName = dlg.GetName();
			CString szType = dlg.GetType();
			CString szPermission;
			szPermission.Format("%d",dlg.GetPermission());
			CString szVirtual = (dlg.IsVirtual()?"1":"0");

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
			description += " " + szType;
			description += " " + szName;
			description += szArguments;
			SetDescription(description);

			CString command = "Impact:AddFunction " + PrepareQuery(pEntity->GetEtag()) + " " +
                        "{" + szType + "}" + " " + szName +
                        " " + szPermission + " " + szVirtual +
                        " " + PrepareQuery(szArguments) + "";

			CString results = m_pCtrl->ExecPrint(command);
			ParseResult(results);

			bRes = true;
		} else {
			SetExecuted(false);
		}
	}
	return bRes;
}
