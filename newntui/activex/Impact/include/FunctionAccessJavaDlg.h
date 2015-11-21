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
#if !defined(AFX_FUNCTIONACCESSJAVADLG_H_)
#define AFX_FUNCTIONACCESSJAVADLG_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// FunctionAccessJavaDlg.h : header file
//

#include "FunctionAccessJava.h"

/////////////////////////////////////////////////////////////////////////////
// CFunctionAccessJavaDlg dialog

class CFunctionAccessJavaDlg : public CDialog, public CFunctionAccessJava
{
// Construction
public:
	void SetFinal(bool state);
	void SetAbstract(bool state);
	void SetStatic(bool state);
	void SetNative(bool state);
	void SetAccess(int nScope);
	bool IsAbstract();
	bool IsStatic();
	bool IsNative();
	bool IsFinal();
	CFunctionAccessJavaDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CFunctionAccessJavaDlg)
	enum { IDD = IDD_METHOD_PERMISSIONS_JAVA };
	BOOL	m_bAbstract;
	BOOL	m_bFinal;
	BOOL	m_bStatic;
	BOOL	m_bNative;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFunctionAccessJavaDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CAccessDlg)
	afx_msg void OnPrivate();
	afx_msg void OnProtected();
	afx_msg void OnPackage();
	afx_msg void OnPublic();	
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	int m_tmpAccess;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FUNCTIONACCESSJAVADLG_H_)
