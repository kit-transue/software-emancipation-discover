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
// FunctionAccessJavaDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Impact.h"
#include "FunctionAccessJavaDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CFunctionAccessJavaDlg dialog


CFunctionAccessJavaDlg::CFunctionAccessJavaDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CFunctionAccessJavaDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CFunctionAccessJavaDlg)
	m_bStatic = FALSE;
	m_bAbstract = FALSE;
	m_bNative = FALSE;
	m_bFinal = FALSE;
	//}}AFX_DATA_INIT
	m_tmpAccess = -1;
}


void CFunctionAccessJavaDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CFunctionAccessJavaDlg)
	DDX_Check(pDX, IDC_ABSTRACT, m_bAbstract);
	DDX_Check(pDX, IDC_FINAL, m_bFinal);
	DDX_Check(pDX, IDC_STATIC2, m_bStatic);
	DDX_Check(pDX, IDC_NATIVE, m_bNative);	
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CFunctionAccessJavaDlg, CDialog)
	//{{AFX_MSG_MAP(CFunctionAccessJavaDlg)
	ON_BN_CLICKED(IDC_PRIVATE, OnPrivate)
	ON_BN_CLICKED(IDC_PROTECTED, OnProtected)
	ON_BN_CLICKED(IDC_PACKAGE, OnPackage)
	ON_BN_CLICKED(IDC_PUBLIC, OnPublic)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFunctionAccessJavaDlg message handlers

void CFunctionAccessJavaDlg::OnPrivate() 
{
	m_nPermission = PRIVATE;
}

void CFunctionAccessJavaDlg::OnProtected() 
{
	m_nPermission = PROTECTED;
}

void CFunctionAccessJavaDlg::OnPublic() 
{
	m_nPermission = PUBLIC;
}

void CFunctionAccessJavaDlg::OnPackage() 
{
	m_nPermission = PACKAGE;
}

bool CFunctionAccessJavaDlg::IsFinal()
{
	return m_bFinal;
}

bool CFunctionAccessJavaDlg::IsAbstract()
{
	return m_bAbstract;
}

bool CFunctionAccessJavaDlg::IsNative()
{
	return m_bNative;
}

bool CFunctionAccessJavaDlg::IsStatic()
{
	return m_bStatic;
}


void CFunctionAccessJavaDlg::SetAccess(int nScope)
{
	m_tmpAccess = nScope;
}


void CFunctionAccessJavaDlg::SetFinal(bool state)
{
	m_bFinal = state;
}

void CFunctionAccessJavaDlg::SetAbstract(bool state)
{
	m_bAbstract = state;
}

void CFunctionAccessJavaDlg::SetNative(bool state)
{
	m_bNative = state;
}

void CFunctionAccessJavaDlg::SetStatic(bool state)
{
	m_bStatic = state;
}

BOOL CFunctionAccessJavaDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// set the radio buttons for public/protected/private
	if(m_tmpAccess!=-1) {
		int nId = -1;
		m_nPermission = m_tmpAccess;
		if(m_nPermission == PUBLIC) nId = IDC_PUBLIC;
		else if(m_nPermission == PRIVATE) nId = IDC_PRIVATE;
		else if(m_nPermission == PACKAGE) nId = IDC_PACKAGE;
		else if(m_nPermission == PROTECTED) nId = IDC_PROTECTED;
		
		if(nId!=-1) {
			CButton* pButton = (CButton*)GetDlgItem(nId);
			if(pButton!=NULL) pButton->SetCheck(1);
		}
	}
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
