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
// EditorSelectionDialog.cpp : implementation file
//

#include "stdafx.h"
#include "discovermdi.h"
#include "EditorSelectionDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CEditorSelectionDialog dialog


CEditorSelectionDialog::CEditorSelectionDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CEditorSelectionDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CEditorSelectionDialog)
	m_EditorName = _T("");
	//}}AFX_DATA_INIT
}


void CEditorSelectionDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEditorSelectionDialog)
	DDX_Control(pDX, IDC_EDITORSELECTOR, m_EditorSelector);
	DDX_CBString(pDX, IDC_EDITORSELECTOR, m_EditorName);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CEditorSelectionDialog, CDialog)
	//{{AFX_MSG_MAP(CEditorSelectionDialog)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEditorSelectionDialog message handlers


BOOL CEditorSelectionDialog::OnInitDialog() 
{
	CDialog::OnInitDialog();
	register curSel = 0;
	//parse the comma separated editor list in m_AllEditorsStr
	if (!m_AllEditorsStr.IsEmpty()) {
	    register i = 0;
	    register stop = 0;
	    register begin_ind = 0;
	    register end_ind = 0;
	    while (!stop) {
		end_ind = m_AllEditorsStr.Find(",", begin_ind);
		if (end_ind == -1) {
		    end_ind = m_AllEditorsStr.GetLength();
		    stop = 1;
		}
		CString singleEditor = m_AllEditorsStr.Mid(begin_ind, end_ind - begin_ind);
		//add editor name to combobox
		m_EditorSelector.AddString(singleEditor);
		//select the saved editor name
		if (m_EditorName == singleEditor) {
		    curSel = i;
		}
		i++;
		begin_ind = end_ind + 1;
	    }
        }
	else {
	    m_EditorSelector.AddString("Error: no editors!");
	}
	m_EditorSelector.SetCurSel(curSel);
 
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
