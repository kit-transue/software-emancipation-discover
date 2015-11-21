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
#if !defined(AFX_QUESTIONDLG_H__018476B4_D83E_11D2_AF95_00A0C9B71DC4__INCLUDED_)
#define AFX_QUESTIONDLG_H__018476B4_D83E_11D2_AF95_00A0C9B71DC4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// QuestionDlg.h : header file
//
#ifndef __DATASOURCE_H
   #define __DATASOURCE_H
   #include "datasource.h"
#endif

/////////////////////////////////////////////////////////////////////////////
// CQuestionDlg dialog

class CQuestionDlg : public CDialog
{
// Construction
public:
	CDataSource* m_DataSource;
	CString m_Title;
	CString m_BtnTitle3;
	CString m_BtnTitle2;
	CString m_BtnTitle1;
	CString m_Result;
	CQuestionDlg(CWnd* pParent = NULL);   // standard constructor
	virtual void OnCalcel(void);

// Dialog Data
	//{{AFX_DATA(CQuestionDlg)
	enum { IDD = IDD_QUESTIONDLG };
	CButton	m_Btn3;
	CButton	m_Btn2;
	CButton	m_Btn1;
	CString	m_Message;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CQuestionDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CQuestionDlg)
	afx_msg void OnBtn1();
	afx_msg void OnBtn2();
	afx_msg void OnBtn3();
	virtual BOOL OnInitDialog();
    afx_msg void OnClose( );
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	BOOL m_Accepted;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_QUESTIONDLG_H__018476B4_D83E_11D2_AF95_00A0C9B71DC4__INCLUDED_)
