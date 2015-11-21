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
#if !defined(AFX_CMATTRIBUTESDLG_H__A2FA0CAE_44BE_4022_BF17_FCD035E4563B__INCLUDED_)
#define AFX_CMATTRIBUTESDLG_H__A2FA0CAE_44BE_4022_BF17_FCD035E4563B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// CMAttributesDlg.h : header file
//
#include "CMCommand.h"
/////////////////////////////////////////////////////////////////////////////
// CCMAttributesDlg dialog

class CWindowsList : public CTypedPtrList<CPtrList,CWnd*> {
};

class CCMAttributesDlg : public CDialog
{
// Construction
public:
	CCMAttributesDlg(CCMAttributesList* pAttrList,CWnd* pParent = NULL);   // standard constructor
	~CCMAttributesDlg();   // standard destructor

// Dialog Data
	//{{AFX_DATA(CCMAttributesDlg)
	enum { IDD = IDD_ATTRIBUTESDLG };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA
// Dialog Data support
	BOOL UpdateData(BOOL bSaveAndValidate = TRUE);


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCMAttributesDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CCMAttributesDlg)
	afx_msg void OnOkAll();
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	CWindowsList* m_pWndList;
	CCMAttributesList* m_pAttrsList;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CMATTRIBUTESDLG_H__A2FA0CAE_44BE_4022_BF17_FCD035E4563B__INCLUDED_)
