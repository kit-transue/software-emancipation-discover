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
// ListSelDlg.cpp : implementation file
//

#include "stdafx.h"
#include "discovermdi.h"
#include "ListSelDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CListSelDlg dialog


CListSelDlg::CListSelDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CListSelDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CListSelDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CListSelDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CListSelDlg)
	DDX_Control(pDX, IDOK, m_OK);
	DDX_Control(pDX, IDC_LIST, m_List);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CListSelDlg, CDialog)
	//{{AFX_MSG_MAP(CListSelDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CListSelDlg message handlers

void CListSelDlg::SetTitle(const CString &title) {
	m_Title = title;
}

void CListSelDlg::SetList(const CString &list) {
     m_Elements= list;


}

void CListSelDlg::OnOK()  {
	int sel = m_List.GetCurSel();
	if(sel<0) 	m_Selection = "";
	else m_List.GetText(sel,m_Selection);
	CDialog::OnOK();
}

BOOL CListSelDlg::OnInitDialog()  {
register i;

	 CDialog::OnInitDialog();
	 SetWindowText(m_Title);
     m_List.ResetContent();
     int in_element = 0;
	 CString tag;
	 for(i=0;i<m_Elements.GetLength();i++) {
		 switch(m_Elements[i]) {
		     case ' ' : if(in_element==0) {
				            m_List.AddString(tag);
						    tag="";
						} else tag+=m_Elements[i];
						break;
			 case '{' : in_element++;
				        break;
			 case '}' : if(in_element) {
				            in_element--;
							if(in_element==0) {
				                m_List.AddString(tag);
						        tag="";
							}
						} else tag+=m_Elements[i];
				        break;
			 default:   tag+=m_Elements[i];
				        break;
		 }
	 }
	 if(tag.GetLength()>0) m_List.AddString(tag);
	 if(m_List.GetCount()>0) m_List.SetCurSel(0);
	 return TRUE;  
}
