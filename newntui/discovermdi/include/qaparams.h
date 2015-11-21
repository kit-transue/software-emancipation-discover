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
#include "spreadsheet.h"
#include "webbrowser2.h"
//}}AFX_INCLUDES
#ifndef __DATASOURCE_H
   #define __DATASOURCE_H
   #include "datasource.h"
#endif

#if !defined(AFX_QAPARAMS_H__447BA2A3_59FC_11D2_AF35_00A0C9B71DC4__INCLUDED_)
#define AFX_QAPARAMS_H__447BA2A3_59FC_11D2_AF35_00A0C9B71DC4__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// QAParams.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CQAParams form view

#ifndef __AFXEXT_H__
#include <afxext.h>
#endif


class CQAParams : public CFormView
{
protected:
	CQAParams();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CQAParams)

// Form Data
public:
	//{{AFX_DATA(CQAParams)
	enum { IDD = IDD_QAPARAMS };
	CStatic	m_ScopeTitle;
	CStatic	m_PropertiesTitle;
	CTabCtrl	m_ModeSelector;
	CSpreadsheet	m_Properties;
	CWebBrowser2	m_Reporter;
	CComboBox	m_ScopeCombo;
	CDataSource	m_DataSource;
	//}}AFX_DATA

// Attributes
public:

// Operations
public:
	void EnableReport(bool bEnable=true);
	CString GetReportName();
	bool IsReportAvailable();
	void UpdateReport(void);
	void FreeCurrentSet(CString& name);
	BOOL GetCurrentSetEnable(void);
    void GetCurrentSet(CString& to);
	void Refresh(CString& node);
	void GetScope(CString& scope);
	void FillPropertiesOfNode(CString& node);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CQAParams)
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CQAParams();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
	//{{AFX_MSG(CQAParams)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnModeChanged(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSetChangedProperties(long row, long col, LPCTSTR val);
	afx_msg void OnStringChanged(long row, long col, LPCTSTR val);
	afx_msg void OnSelchangeScope();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	BOOL m_ShowReport;
	void ShowReportPage(BOOL show);
	void ShowSettingsPage(BOOL show);
	BOOL m_Updated;
	void LayoutControls(int cx, int cy);
    BOOL IsDigit(CString& token);
	CString DataSourceAccessSync(BSTR* command);
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_QAPARAMS_H__447BA2A3_59FC_11D2_AF35_00A0C9B71DC4__INCLUDED_)
