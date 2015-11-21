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
// CheckInPromptDlg.cpp : implementation file
//

#include "stdafx.h"
#include "discovermdi.h"
#include "CheckInPromptDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CCheckInPromptDlg dialog


CCheckInPromptDlg::CCheckInPromptDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CCheckInPromptDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CCheckInPromptDlg)
	//}}AFX_DATA_INIT
}


void CCheckInPromptDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCheckInPromptDlg)
	DDX_Control(pDX, IDC_OPTIONS, m_Options);
	DDX_Control(pDX, IDC_COMMENTS, m_Comments);
	DDX_Control(pDX, IDC_FILELIST, m_FileList);
	DDX_Control(pDX, IDC_DATASOURCE, m_DataSource);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CCheckInPromptDlg, CDialog)
	//{{AFX_MSG_MAP(CCheckInPromptDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCheckInPromptDlg message handlers

BOOL CCheckInPromptDlg::OnInitDialog()  {
CString res;
CString cmd;
BSTR sysCmd;
int elements;

	CDialog::OnInitDialog();

	// Forming file list  for submission
	cmd="set __cm_files__ [filter {kind==\"module\" && writable==1} [where defined $DiscoverSelection]]";
	sysCmd=cmd.AllocSysString();
	DataSourceAccessSync(&sysCmd);
	::SysFreeString(sysCmd);

	cmd="size $__cm_files__";
	sysCmd=cmd.AllocSysString();
	res=DataSourceAccessSync(&sysCmd);
	elements=atoi(res);
	::SysFreeString(sysCmd);
	
	cmd = "get_pref_value SubmissionCheck.Enabled";
	sysCmd=cmd.AllocSysString();
	res=DataSourceAccessSync(&sysCmd);
	::SysFreeString(sysCmd);
	/*if(res=="1") m_SubCheck.EnableWindow(TRUE);
	else         m_SubCheck.EnableWindow(FALSE);*/


	for(int i=0;i<elements;i++) {

		// Extracting file name
	    cmd.Format("fname $__cm_files__ %d",i+1);
	    sysCmd=cmd.AllocSysString();
	    res=DataSourceAccessSync(&sysCmd);
	    ::SysFreeString(sysCmd);

		m_FileList.AddString(res);

	}
	cmd="unset __cm_files__";
	sysCmd=cmd.AllocSysString();
	res=DataSourceAccessSync(&sysCmd);
	elements=atoi(res);
	::SysFreeString(sysCmd);
	
	return TRUE;  
}

void CCheckInPromptDlg::OnOK()  {
CString cmd;
BSTR sysCmd;

    CString options;
	m_Options.GetWindowText(options);
    CString comments;
	m_Comments.GetWindowText(comments);

	cmd="set __cm_files__ [filter {kind==\"module\" && writable==1} [where defined $DiscoverSelection]]";
	sysCmd=cmd.AllocSysString();
	DataSourceAccessSync(&sysCmd);
	::SysFreeString(sysCmd);

	cmd.Format("cm_put \"%s\" \"%s\" $__cm_files__",options,comments);
    sysCmd=cmd.AllocSysString();
    DataSourceAccessSync(&sysCmd);
    ::SysFreeString(sysCmd);

	cmd="unset __cm_files__";
	sysCmd=cmd.AllocSysString();
	DataSourceAccessSync(&sysCmd);
    ::SysFreeString(sysCmd);

	CDialog::OnOK();
}

CString CCheckInPromptDlg::DataSourceAccessSync(BSTR* command) {
	CString res;
	static BOOL bCommDlgShown = FALSE;
	res = m_DataSource.AccessSync(command);
	if (m_DataSource.IsConnectionLost() == TRUE &&
		bCommDlgShown != TRUE) {
		::MessageBox(m_hWnd, 
			   _T("Connection with server is lost."
			   " Make sure the server is running."),
			   _T("Server Communication Error."), 
			   MB_OK | MB_ICONINFORMATION); 
		bCommDlgShown = TRUE;
	} else if (m_DataSource.IsConnectionLost() != TRUE) {
		bCommDlgShown = FALSE;
	}	
	return res;
}
