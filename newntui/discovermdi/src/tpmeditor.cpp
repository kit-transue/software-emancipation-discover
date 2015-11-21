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
// TPMEditor.cpp : implementation file
//

#include "stdafx.h"
#include "discovermdi.h"
#include "TPMEditor.h"
#include "mainfrm.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTPMEditor dialog


CTPMEditor::CTPMEditor(CWnd* pParent /*=NULL*/)
	: CDialog(CTPMEditor::IDD, pParent)
{
	//{{AFX_DATA_INIT(CTPMEditor)
	m_CommentEdit = _T("");
	m_CurrentTPM = _T("");
	m_NameEdit = _T("");
	m_NewTPM = _T("");
	//}}AFX_DATA_INIT
}


void CTPMEditor::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTPMEditor)
	DDX_Control(pDX, IDC_NEWEDIT, m_NewTpmEditor);
	DDX_Control(pDX, IDC_CURRENTEDIT, m_CurrentTpmEditor);
	DDX_Control(pDX, IDC_CHANGE, m_ApplyButton);
	DDX_Text(pDX, IDC_COMMENTEDIT, m_CommentEdit);
	DDX_Text(pDX, IDC_CURRENTEDIT, m_CurrentTPM);
	DDX_Text(pDX, IDC_NAMEEDIT, m_NameEdit);
	DDX_Text(pDX, IDC_NEWEDIT, m_NewTPM);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CTPMEditor, CDialog)
	//{{AFX_MSG_MAP(CTPMEditor)
	ON_BN_CLICKED(IDC_CHANGE, OnChange)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTPMEditor message handlers


void CTPMEditor::OnChange()  {
	m_NewTpmEditor.GetWindowText(m_NewTPM);
	m_CurrentTPM = m_NewTPM;
	m_CurrentTpmEditor.SetWindowText(m_NewTPM);
	
}

void CTPMEditor::OnOK()  {
	CDialog::OnOK();
}

BOOL CTPMEditor::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	m_Img.LoadMappedBitmap(IDB_ARROWUP);
	m_ApplyButton.SetBitmap(m_Img);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
