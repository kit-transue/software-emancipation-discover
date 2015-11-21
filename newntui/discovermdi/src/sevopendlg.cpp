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
// SevOpenDlg.cpp : implementation file
//

#include "stdafx.h"
#include "discovermdi.h"
#include "SevOpenDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSevOpenDlg dialog


CSevOpenDlg::CSevOpenDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSevOpenDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSevOpenDlg)
	//}}AFX_DATA_INIT
}


void CSevOpenDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSevOpenDlg)
	DDX_Control(pDX, IDC_SESSIONLIST, m_List);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSevOpenDlg, CDialog)
	//{{AFX_MSG_MAP(CSevOpenDlg)
	ON_CBN_SELCHANGE(IDC_SESSIONLIST, OnSelchangeSession)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSevOpenDlg message handlers

BOOL CSevOpenDlg::OnInitDialog()  {
register i=0;

	CDialog::OnInitDialog();
	m_List.Clear();
	POSITION pos;
	pos = m_NamesList.GetHeadPosition();
	while(pos) {
		m_List.InsertString(i++,m_NamesList.GetAt(pos));
		m_NamesList.GetNext(pos);
	}
	if(i) {
		m_List.SetCurSel(0);
        m_List.GetLBText(m_List.GetCurSel(),m_Name);	
	}
	return TRUE;  
}


void CSevOpenDlg::OnSelchangeSession()  {
	if(m_List.GetCurSel()>=0) 
	   m_List.GetLBText(m_List.GetCurSel(),m_Name);	
}
