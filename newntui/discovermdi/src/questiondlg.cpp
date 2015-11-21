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
// QuestionDlg.cpp : implementation file
//

#include "stdafx.h"
#include "discovermdi.h"
#include "QuestionDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CQuestionDlg dialog


CQuestionDlg::CQuestionDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CQuestionDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CQuestionDlg)
	m_Message = _T("");
	//}}AFX_DATA_INIT
	m_Accepted=FALSE;
}


void CQuestionDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CQuestionDlg)
	DDX_Control(pDX, ID_BTN3, m_Btn3);
	DDX_Control(pDX, ID_BTN2, m_Btn2);
	DDX_Control(pDX, ID_BTN1, m_Btn1);
	DDX_Text(pDX, IDC_MESSAGE, m_Message);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CQuestionDlg, CDialog)
	//{{AFX_MSG_MAP(CQuestionDlg)
	ON_BN_CLICKED(ID_BTN1, OnBtn1)
	ON_BN_CLICKED(ID_BTN2, OnBtn2)
	ON_BN_CLICKED(ID_BTN3, OnBtn3)
	ON_WM_CLOSE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CQuestionDlg message handlers

void CQuestionDlg::OnBtn1()  {
	m_Btn1.GetWindowText(m_Result);
    m_DataSource->AcceptNotification(m_Result);
	m_Accepted=TRUE;
	CDialog::OnOK();
}

void CQuestionDlg::OnBtn2()  {
	m_Btn2.GetWindowText(m_Result);
    m_DataSource->AcceptNotification(m_Result);
	m_Accepted=TRUE;
	CDialog::OnOK();
}

void CQuestionDlg::OnBtn3()  {
	m_Btn3.GetWindowText(m_Result);
    m_DataSource->AcceptNotification(m_Result);
	m_Accepted=TRUE;
	CDialog::OnOK();
}

BOOL CQuestionDlg::OnInitDialog()  {
	CDialog::OnInitDialog();
	if(m_Title.GetLength()>0) SetWindowText(m_Title);
	if(m_BtnTitle1.GetLength()>0) {
		m_Btn1.SetWindowText(m_BtnTitle1);
		m_Btn1.ShowWindow(SW_SHOW);
		if(m_BtnTitle2.GetLength()>0) {
			if(m_BtnTitle3.GetLength()>0) {
		        m_Btn2.SetWindowText(m_BtnTitle2);
		        m_Btn2.ShowWindow(SW_SHOW);
		        m_Btn3.SetWindowText(m_BtnTitle3);
		        m_Btn3.ShowWindow(SW_SHOW);
			} else {
		        m_Btn3.SetWindowText(m_BtnTitle2);
		        m_Btn3.ShowWindow(SW_SHOW);
			}
		} else{
		   m_Btn3.SetWindowText("Cancel");
		   m_Btn3.ShowWindow(SW_SHOW);
		}
	} else {
		m_Btn2.SetWindowText("Cancel");
		m_Btn2.ShowWindow(SW_SHOW);
	}		
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CQuestionDlg::OnCalcel(void) {
    m_DataSource->AcceptNotification("");
	m_Accepted=TRUE;
	CDialog::OnCancel();
}


void CQuestionDlg::OnClose(void) {
	if(m_Accepted==FALSE)
         m_DataSource->AcceptNotification("");
	CDialog::OnClose();
}
