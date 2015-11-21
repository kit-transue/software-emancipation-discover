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
// CancelDialogDlg.cpp : implementation file
//

#include "stdafx.h"
#include "CancelDialog.h"
#include "CancelDialogDlg.h"


#define  WM_SETAMOUNT    WM_USER+1313
#define  WM_SETPROCESSED WM_USER+1314

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


static int msgProcessedID;

/////////////////////////////////////////////////////////////////////////////
// CCancelDialogDlg dialog

CCancelDialogDlg::CCancelDialogDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CCancelDialogDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CCancelDialogDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CCancelDialogDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCancelDialogDlg)
	DDX_Control(pDX, IDC_MESSAGE, m_MessageText);
	DDX_Control(pDX, IDC_AMOUNT, m_ElementsAmount);
	DDX_Control(pDX, IDC_READY, m_ElementsReady);
	DDX_Control(pDX, IDC_PROGRESS, m_ProgressBar);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CCancelDialogDlg, CDialog)
	//{{AFX_MSG_MAP(CCancelDialogDlg)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_REGISTERED_MESSAGE(msgProcessedID,OnSetProcessed)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCancelDialogDlg message handlers

BOOL CCancelDialogDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	// TODO: Add extra initialization here
    msgProcessedID = RegisterWindowMessage("WM_SETPROCESSED");

	SetWindowText(m_Title);
	CString amount;
	amount.Format("%d",m_Amount);
	m_ElementsAmount.SetWindowText(amount);
	m_ProgressBar.SetRange(0,m_Amount);
	m_MessageText.SetWindowText(m_Message);
	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CCancelDialogDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CCancelDialogDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

LRESULT CCancelDialogDlg::OnSetAmount(WPARAM rangeParam, LPARAM)  {
int range = rangeParam;
CString amount;
    amount.Format("%d",range);
	m_ElementsAmount.SetWindowText(amount);
	m_ElementsAmount.Invalidate();
	m_ProgressBar.SetRange(0,range);
	return 0;
}


LRESULT CCancelDialogDlg::OnSetProcessed(WPARAM processedParam, LPARAM str)  {
int processed = processedParam;
static char* message=(char *)str;
static CString mes;
CString proc;

    proc.Format("%d",processed);
	m_ElementsReady.SetWindowText(proc);

    CString amount;
    amount.Format("%d",m_Amount-processed);
	m_ElementsAmount.SetWindowText(amount);

	m_ProgressBar.SetPos(processed);
	mes=message;
	mes.TrimLeft();
	m_MessageText.SetWindowText(mes);
	ZeroMemory(message,1024);
	return 0;
}



