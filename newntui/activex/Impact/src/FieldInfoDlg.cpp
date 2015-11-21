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
// FieldInfoDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Impact.h"
#include "FieldInfoDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CFieldInfoDlg dialog

CFieldInfoDlg::CFieldInfoDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CFieldInfoDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CFieldInfoDlg)
	m_szName = _T("");
	m_szType = _T("");
	//}}AFX_DATA_INIT
}


void CFieldInfoDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CFieldInfoDlg)
	DDX_Text(pDX, IDC_NAME, m_szName);
	DDX_Text(pDX, IDC_TYPE, m_szType);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CFieldInfoDlg, CDialog)
	//{{AFX_MSG_MAP(CFieldInfoDlg)
	ON_BN_CLICKED(IDC_PUBLIC, OnPublic)
	ON_BN_CLICKED(IDC_PROTECTED, OnProtected)
	ON_BN_CLICKED(IDC_PRIVATE, OnPrivate)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFieldInfoDlg message handlers

void CFieldInfoDlg::OnPublic() 
{
	m_nPermission = PUBLIC;
}

void CFieldInfoDlg::OnProtected() 
{
	m_nPermission = PROTECTED;
}

void CFieldInfoDlg::OnPrivate() 
{
	m_nPermission = PRIVATE;
}

CString& CFieldInfoDlg::GetName()
{
	return m_szName;
}

CString& CFieldInfoDlg::GetType()
{
	return m_szType;
}

int CFieldInfoDlg::GetPermission()
{
	return m_nPermission;
}
