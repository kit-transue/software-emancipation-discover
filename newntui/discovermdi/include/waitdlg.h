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
#if !defined(AFX_WAITDLG_H__856C19E2_60BE_4D2C_99AB_46D8303CEE4B__INCLUDED_)
#define AFX_WAITDLG_H__856C19E2_60BE_4D2C_99AB_46D8303CEE4B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// WaitDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CWaitDlg dialog

class CCompletionProvider {
public:
	virtual bool IsWaitComplete()=0;
};

class CWaitDlg : public CDialog
{
// Construction
public:
	void SetMessage(CString& szMsg);
	void SetCompletionProvider(CCompletionProvider* pProvider);
	int Wait(int nTimeout=0);
	CWaitDlg(HANDLE hProcess,CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CWaitDlg)
	enum { IDD = IDD_WAITDLG };
	CStatic	m_MessageCtrl;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CWaitDlg)
	public:
	virtual int DoModal();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	CCompletionProvider* m_pCompletionProvider;

	// Generated message map functions
	//{{AFX_MSG(CWaitDlg)
	afx_msg void OnTimer(UINT nIDEvent);
	virtual void OnOK();
	virtual void OnCancel();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
			 CString GetTextWithEllipsis(CDC* pDC, CString& szString, CRect& Rect);

			 CString m_szMessage;
			 HANDLE m_hProcess;
			 int	m_nTimeout;
			 int	m_bTimeoutSet;
static const int	GRANULARITY;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_WAITDLG_H__856C19E2_60BE_4D2C_99AB_46D8303CEE4B__INCLUDED_)
