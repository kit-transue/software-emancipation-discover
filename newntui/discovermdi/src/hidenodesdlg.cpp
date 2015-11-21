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
// HideNodesDlg.cpp : implementation file
//

#include "stdafx.h"
#include "discovermdi.h"
#include "HideNodesDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CHideNodesDlg dialog


CHideNodesDlg::CHideNodesDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CHideNodesDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CHideNodesDlg)
	//}}AFX_DATA_INIT
}


void CHideNodesDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CHideNodesDlg)
	DDX_Control(pDX, IDC_NAMEMATCH, m_NameCombo);
	DDX_Control(pDX, IDC_FILEMATCH, m_FileCombo);
	DDX_Control(pDX, IDC_FILEEDIT, m_FileEdit);
	DDX_Control(pDX, IDC_NAMEEDIT, m_NameEdit);
	DDX_Control(pDX, IDOK, m_OkBtn);
	DDX_Control(pDX, IDAPPLY, m_ApplyBtn);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CHideNodesDlg, CDialog)
	//{{AFX_MSG_MAP(CHideNodesDlg)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CHideNodesDlg message handlers
