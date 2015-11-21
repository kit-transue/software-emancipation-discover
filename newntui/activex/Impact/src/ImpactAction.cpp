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
// ImpactAction.cpp: implementation of the CImpactAction class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Impact.h"
#include "ImpactAction.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#include "ImpactCtl.h"

IMPLEMENT_DYNCREATE(CImpactAction, CObject)

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CImpactAction::CImpactAction() {
	m_szName="";
	m_bExecuted = false;
	m_pResults = NULL;
	m_pCtrl = NULL;
	m_bRepeatAction = false;
	m_bDuplicated = false;
	
	m_pData = NULL;
	m_bWaitState = false;

	m_pHash =  NULL;
}

CImpactAction::CImpactAction(CString& name,CImpactCtrl* pCtrl,bool repeat) {
	m_szName=name;
	m_bExecuted = false;
	m_pResults = NULL;
	m_pCtrl = pCtrl;
	m_bRepeatAction = repeat;
	m_bDuplicated = false;

	m_pData = NULL;
	m_bWaitState = false;

	m_pHash =  NULL;
}

CImpactAction::~CImpactAction(){
	if(m_pResults!=NULL) delete m_pResults;
	if(m_pData!=NULL) delete m_pData;
}

CString& CImpactAction::GetName() {
	return m_szName;
}

CString CImpactAction::PrepareQuery(const CString& query) {
	int nLen = query.GetLength();
	if(nLen==0) return "";
	CString result="{";
	for(int j=0;j<nLen;j++) {
		switch(query[j]) {
//			case '{' : result += "\\{";break;
//			case '}' : result += "\\}";break;
			default: result+=query[j];break;
		}
	}
	result += "}";
	return result; 
}

bool CImpactAction::ActionPerformed(CEntity* pEntity)
{
	if(m_pCtrl!=NULL) {
		CString result = m_pCtrl->ExecCommand(CString("source_dis impacts.dis"));
		SetExecuted(true);
		SetDescription(GetName());
		return true;
	}
	return false; 
}

void CImpactAction::SetWait(bool state)
{
    if(state==true) {
		AfxGetApp()->BeginWaitCursor();
	} else {
		AfxGetApp()->EndWaitCursor();
	}
	m_bWaitState = state;
}

bool CImpactAction::IsWait()
{
	return m_bWaitState;
}

bool CImpactAction::DoRepeat()
{
	return m_bRepeatAction;
}

CImpactCtrl* CImpactAction::GetCtrl()
{
	return m_pCtrl;
}

void CImpactAction::SetCtrl(CImpactCtrl* pCtrl)
{
	m_pCtrl = pCtrl;
}

void CImpactAction::SetDuplicated(bool state)
{
	m_bDuplicated = state;
}

bool CImpactAction::IsDuplicated()
{
	return m_bDuplicated;
}

void CImpactAction::Undo()
{

}

void CImpactAction::SetData(CActionData* pData)
{
	m_pData = pData;
}

CActionData* CImpactAction::GetData()
{
	return m_pData;
}
