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
// AccessCppDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Impact.h"
#include "AccessCppDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAccessCppDlg dialog


CAccessCppDlg::CAccessCppDlg(CWnd* pParent /*=NULL*/)
: CDialog(CAccessCppDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CAccessCppDlg)
	m_bConst = FALSE;
	m_bStatic = FALSE;
	//}}AFX_DATA_INIT
	m_tmpAccess = -1;
}


void CAccessCppDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAccessCppDlg)
	DDX_Check(pDX, IDC_STATIC2, m_bStatic);
	DDX_Check(pDX, IDC_CONST, m_bConst);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CAccessCppDlg, CDialog)
	//{{AFX_MSG_MAP(CAccessCppDlg)
	ON_BN_CLICKED(IDC_PRIVATE, OnPrivate)
	ON_BN_CLICKED(IDC_PROTECTED, OnProtected)
	ON_BN_CLICKED(IDC_PUBLIC, OnPublic)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAccessCppDlg message handlers

void CAccessCppDlg::OnPrivate() 
{
	m_nPermission = PRIVATE;
}

void CAccessCppDlg::OnProtected() 
{
	m_nPermission = PROTECTED;
}

void CAccessCppDlg::OnPublic() 
{
	m_nPermission = PUBLIC;
}

bool CAccessCppDlg::IsConst()
{
	return m_bConst;
}

bool CAccessCppDlg::IsStatic()
{
	return m_bStatic;
}

void CAccessCppDlg::SetAccess(int nScope)
{
	m_tmpAccess = nScope;
}

void CAccessCppDlg::SetConst(bool state)
{
	m_bConst = state;
}

void CAccessCppDlg::SetStatic(bool state)
{
	m_bStatic = state;
}

BOOL CAccessCppDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// set the radio buttons for public/protected/private
	if(m_tmpAccess!=-1) {
		int nId = -1;
		m_nPermission = m_tmpAccess;
		if(m_nPermission == PUBLIC) nId = IDC_PUBLIC;
		else if(m_nPermission == PRIVATE) nId = IDC_PRIVATE;
		else if(m_nPermission == PROTECTED) nId = IDC_PROTECTED;
		
		if(nId!=-1) {
			CButton* pButton = (CButton*)GetDlgItem(nId);
			if(pButton!=NULL) pButton->SetCheck(1);
		}
	}
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
