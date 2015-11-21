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
#if !defined(AFX_GRAPHINHERITANCEPROPPAGE_H__98B1EE40_C784_11D2_AF8C_00A0C9B71DC4__INCLUDED_)
#define AFX_GRAPHINHERITANCEPROPPAGE_H__98B1EE40_C784_11D2_AF8C_00A0C9B71DC4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "GraphView.h"

// GraphInheritancePropPage.h : header file
//
/////////////////////////////////////////////////////////////////////////////
// CGraphInheritancePropPage dialog

class CGraphInheritancePropPage : public CPropertyPage
{
	DECLARE_DYNCREATE(CGraphInheritancePropPage)

// Construction
public:
	void SetGraphSettings(CGraphSettings *settings);
	CGraphInheritancePropPage();
	~CGraphInheritancePropPage();

// Dialog Data
	//{{AFX_DATA(CGraphInheritancePropPage)
	enum { IDD = IDD_GRAPH_INHERITANCE };
	CButton	m_Merge;
	CButton	m_Public;
	CButton	m_Protected;
	CButton	m_Private;
	CButton	m_PackageAccess;
	CButton	m_ClassProperties;
	CButton	m_ClassMethods;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CGraphInheritancePropPage)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CGraphInheritancePropPage)
	virtual BOOL OnInitDialog();
	afx_msg void OnEnableClassMehtods();
	afx_msg void OnEnableClassProperties();
	afx_msg void OnLongNames();
	afx_msg void OnPackageAccess();
	afx_msg void OnPrivate();
	afx_msg void OnProtected();
	afx_msg void OnPublic();
	afx_msg void OnShortNames();
	afx_msg void OnSortByAccess();
	afx_msg void OnSortByName();
	afx_msg void OnMergeClasses();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	CGraphSettings* m_Settings;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GRAPHINHERITANCEPROPPAGE_H__98B1EE40_C784_11D2_AF8C_00A0C9B71DC4__INCLUDED_)
