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
// IMAttachDlg.cpp : implementation file
//

#include "stdafx.h"
#include "discovermdi.h"
#include "IMAttachDlg.h"
#include "WaitDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CIMAttachDlg dialog

CString CIMAttachDlg::m_szHostName = _T("");
CString CIMAttachDlg::m_szPort = _T("");
CString CIMAttachDlg::m_szIssueID = _T("");
CString CIMAttachDlg::m_szPassword = _T("");
CString CIMAttachDlg::m_szUserName = _T("");

CIMAttachDlg::CIMAttachDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CIMAttachDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CIMAttachDlg)
	//}}AFX_DATA_INIT
}


void CIMAttachDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CIMAttachDlg)
	DDX_Text(pDX, IDC_HOSTNAME, m_szHostName);
	DDX_Text(pDX, IDC_HOSTPORT, m_szPort);
	DDX_Text(pDX, IDC_ISSUEID, m_szIssueID);
	DDX_Text(pDX, IDC_PASSWORD, m_szPassword);
	DDX_Text(pDX, IDC_USERNAME, m_szUserName);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CIMAttachDlg, CDialog)
	//{{AFX_MSG_MAP(CIMAttachDlg)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CIMAttachDlg message handlers

bool CIMAttachDlg::AttachFileToIMIssue(CString szPathToFile)
{
	CString szCmd;
	szCmd.Format("im editissue --addAttachment=\"%s\" --hostname=%s --port=%s --password=%s --user=%s %s",szPathToFile,m_szHostName,m_szPort,m_szPassword,m_szUserName,m_szIssueID);

    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    ZeroMemory( &si, sizeof(si) );
    si.cb = sizeof(si);
    ZeroMemory( &pi, sizeof(pi) );

    // Start the child process. 
    if(!CreateProcess( NULL, // No module name (use command line). 
       (char*)(LPCTSTR)szCmd,	 // Command line. 
       NULL,             // Process handle not inheritable. 
       NULL,             // Thread handle not inheritable. 
       FALSE,            // Set handle inheritance to FALSE. 
       CREATE_NO_WINDOW, // Do not show the console window
       NULL,             // Use parent's environment block. 
       NULL,             // Use parent's starting directory. 
       &si,              // Pointer to STARTUPINFO structure.
       &pi )             // Pointer to PROCESS_INFORMATION structure.
	){
		LPVOID lpMsgBuf;
		FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | 
					  FORMAT_MESSAGE_FROM_SYSTEM | 
					  FORMAT_MESSAGE_IGNORE_INSERTS,
					  NULL,
					  GetLastError(),
					  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
					  (LPTSTR) &lpMsgBuf,
					  0,
					  NULL);
		// Display the error.
		::MessageBox( NULL, (LPCTSTR)lpMsgBuf, "Can't attach file to an IM issue", MB_OK | MB_ICONINFORMATION);
		// Free the buffer.
		LocalFree( lpMsgBuf );
		return false;
    }

	CWaitDlg waitDlg(pi.hProcess);
	waitDlg.SetMessage(CString("Attaching ") + szPathToFile + CString("..."));
	waitDlg.Wait();
    
    // Close process and thread handles. 
    CloseHandle( pi.hProcess );
    CloseHandle( pi.hThread );
	return true;
}
