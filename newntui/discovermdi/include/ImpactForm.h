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
//{{AFX_INCLUDES()
//}}AFX_INCLUDES
#if !defined(AFX_IMPACTFORM_H__E3FFCAAB_4B15_4BA5_B41E_D65780B061C3__INCLUDED_)
#define AFX_IMPACTFORM_H__E3FFCAAB_4B15_4BA5_B41E_D65780B061C3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ImpactForm.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CImpactForm form view

#ifndef __AFXEXT_H__
#include <afxext.h>
#endif

#include "CodeRoverIntegrator.h"

class CImpactForm : public CFormView
{
protected:
	CImpactForm();           // protected constructor used by dynamic creation
	virtual ~CImpactForm();
	DECLARE_DYNCREATE(CImpactForm)

// Form Data
public:
	//{{AFX_DATA(CImpactForm)
	enum { IDD = IDD_IMPACT };
	CDataSource	m_DataSource;
	//}}AFX_DATA

// Attributes
public:

// Operations
public:
	void SetEditorCtrl(CCodeRoverIntegrator* pEditorCtrl);
	CString GetReportName();
	bool SaveReportTo(CString& szFileName);
	bool IsReportView();
	bool Update();
	bool Create(CWnd* pParent,CRect& rect);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CImpactForm)
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
	//{{AFX_MSG(CImpactForm)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_IMPACTFORM_H__E3FFCAAB_4B15_4BA5_B41E_D65780B061C3__INCLUDED_)
