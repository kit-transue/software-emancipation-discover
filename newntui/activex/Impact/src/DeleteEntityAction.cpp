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
// DeleteEntityAction.cpp: implementation of the CDeleteEntityAction class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Impact.h"
#include "ImpactCtl.h"
#include "DeleteEntityAction.h"

#include "SelectorDlg.h"
#include "AttributesQueryResult.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNCREATE(CDeleteEntityAction, CImpactAction)
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDeleteEntityAction::CDeleteEntityAction() :
	CImpactAction(CString("Delete"),NULL,false)
{

}

CDeleteEntityAction::CDeleteEntityAction(CImpactCtrl* pCtrl) :
	CImpactAction(CString("Delete"),pCtrl,false)
{

}

CDeleteEntityAction::~CDeleteEntityAction()
{

}

bool CDeleteEntityAction::ActionPerformed(CEntity* pEntity) {
	CString command;
	CString results;
	bool bRes = false;
	if(CImpactAction::ActionPerformed(pEntity)==true) {

			CString kind = pEntity->GetKind();
			kind.MakeLower();
			CString id = PrepareQuery(pEntity->GetEtag());
			SetWait(true);        
	
			if ( kind == "package" ) {
				if(MessageBox(NULL,"Running impact queries on packages may take a long time.\nWould you like to proceed?",
				          "QA Warning",
						  MB_ICONQUESTION|MB_OKCANCEL)!=IDOK) {

					SetExecuted(false);
					bRes = false;
					return false;
				}
				command = "Impact:DeleteEntity " + id;
			} else if ( kind == "var" ) { 
				command = "oo_member " + id;
				results = m_pCtrl->ExecCommand(command);
				if (results == "1") {
					command = "Impact:DeleteVariableFromClass " + id;
				}  else {
					command = "Impact:DeleteVariable " + id + " [ where defined " + id + " ] ";
				}
			} else { 
				command = "Impact:DeleteEntity " + id;
			}

			results = m_pCtrl->ExecPrint(command);
			ParseResult(results);
			bRes = true;
			SetWait(false);		
	}
	SetExecuted(bRes);
	return bRes;
}

