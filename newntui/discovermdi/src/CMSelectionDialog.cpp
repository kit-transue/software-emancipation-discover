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
// CMSelectionDialog.cpp : implementation file
//

#include "stdafx.h"
#include "discovermdi.h"
#include "CMSelectionDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CCMSelectionDialog dialog


CCMSelectionDialog::CCMSelectionDialog(CCMIntegrator* pIntegrator,CWnd* pParent /*=NULL*/)
	: CDialog(CCMSelectionDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CCMSelectionDialog)
	m_CMName = _T("");
	//}}AFX_DATA_INIT
	m_pIntegrator = pIntegrator;
}


void CCMSelectionDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCMSelectionDialog)
	DDX_Control(pDX, IDC_CMSELECTOR, m_CMSelector);
	DDX_CBString(pDX, IDC_CMSELECTOR, m_CMName);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CCMSelectionDialog, CDialog)
	//{{AFX_MSG_MAP(CCMSelectionDialog)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCMSelectionDialog message handlers

BOOL CCMSelectionDialog::OnInitDialog() 
{
	CDialog::OnInitDialog();
	m_CMSelector.EnableWindow(false);
	
	if(m_pIntegrator!=NULL) {
		CString szCMs = m_pIntegrator->GetCMs();
		if(!szCMs.IsEmpty()) {
			int nStart = 0;
			int nEnd = 0;
			while((nEnd = szCMs.Find('\n',nStart)) > 0) {
				m_CMSelector.AddString(szCMs.Mid(nStart,nEnd-nStart));
				nStart = nEnd+1;
			}
			m_CMSelector.AddString(szCMs.Mid(nStart));
			m_CMSelector.EnableWindow(true);
			CString szSelectedCM = m_pIntegrator->GetCMName();
			if(!szSelectedCM.IsEmpty()) m_CMSelector.SelectString(0,szSelectedCM);
		} 	
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

CString& CCMSelectionDialog::GetCMName()
{
	return m_CMName;
}
