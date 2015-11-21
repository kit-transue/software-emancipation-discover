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
#ifndef __DATASOURCE_H
   #define __DATASOURCE_H
   #include "datasource.h"
#endif
#if !defined(AFX_TPMPROCESSOR_H__F44373E2_2D53_11D2_AF05_00A0C9B71DC4__INCLUDED_)
#define AFX_TPMPROCESSOR_H__F44373E2_2D53_11D2_AF05_00A0C9B71DC4__INCLUDED_

#ifndef __SPREADSHEET_H
   #define __SPREADSHEET_H
   #include "spreadsheet.h"
#endif

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// TpmProcessor.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CTpmProcessor form view

#ifndef __AFXEXT_H__
#include <afxext.h>
#endif

class CTpmProcessor : public CFormView
{
protected:
	CTpmProcessor();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CTpmProcessor)

// Form Data
public:
	//{{AFX_DATA(CTpmProcessor)
	enum { IDD = IDD_TPMFILTER };
	CButton	m_Inclusive;
	CButton	m_EditRow;
	CTabCtrl	  m_FilterType;
	CTabCtrl	  m_ModeSelector;
	CButton	      m_NewFilter;
	CComboBox	  m_FilterSelector;
	CButton	      m_DelRow;
	CButton	      m_AddRow;
	CSpreadsheet  m_TextSpreadsheet;
	CSpreadsheet  m_TypeSpreadsheet;
	CSpreadsheet	m_TpmSpreadsheet;
	CDataSource	m_DataSource;
	//}}AFX_DATA

// Attributes
public:

// Operations
public:
	void Update(void);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTpmProcessor)
	public:
	virtual void OnFinalRelease();
	virtual void OnInitialUpdate();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CTpmProcessor();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
	//{{AFX_MSG(CTpmProcessor)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnSelchangeFilterType(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSelchangeMode(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnNewRow();
	afx_msg void OnDelrow();
	afx_msg void OnNewFilter();
	afx_msg void OnClose();
	afx_msg void OnSelchangeFilter();
	afx_msg void OnEditRow();
	afx_msg void OnSelchangingMode(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnBoolChanged(long row, long col, LPCTSTR val);
	afx_msg void OnStringChanged(long row, long col, LPCTSTR val);
	afx_msg void OnBoolChangedTypespreadsheet(long row, long col, LPCTSTR val);
	afx_msg void OnStringChangedTypespreadsheet(long row, long col, LPCTSTR val);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	// Generated OLE dispatch map functions
	//{{AFX_DISPATCH(CTpmProcessor)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_DISPATCH
	DECLARE_DISPATCH_MAP()
	DECLARE_INTERFACE_MAP()
private:
	CString m_PatternPath;
	CString m_PatternName;
	CString m_FilterPath;
	CString m_FilterName;
	void SavePatternSpreadsheet(void);
	void SetButtonsStatus(void);
	void SaveFiltersSpreadsheets(void);
	void FillFiltersSpreadsheets(void);
    void FillPatternSpreadsheet(void);
	void FillFilterSelector();
	void LayoutTpmPage(int cx, int cy);
	void LayoutFilterPage(int cx, int cy);
	BOOL m_Updated;
	CFont m_InfoFont;
	CString DataSourceAccessSync(BSTR* command);
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TPMPROCESSOR_H__F44373E2_2D53_11D2_AF05_00A0C9B71DC4__INCLUDED_)
