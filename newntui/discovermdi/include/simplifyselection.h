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
#if !defined(AFX_SIMPLIFYSELECTION_H__B7C99900_E891_11D2_AFB4_00A0C9B71DC4__INCLUDED_)
#define AFX_SIMPLIFYSELECTION_H__B7C99900_E891_11D2_AFB4_00A0C9B71DC4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SimplifySelection.h : header file
//
#ifndef __DATASOURCE_H
   #define __DATASOURCE_H
   #include "datasource.h"
#endif
#ifndef __QUERYRESULTS_H
   #define __QUERYRESULTS_H
   #include "queryresults.h"
#endif


/////////////////////////////////////////////////////////////////////////////
// CSimplifySelection form view

#ifndef __AFXEXT_H__
#include <afxext.h>
#endif

class CSimplifySelection : public CFormView
{
protected:
	CSimplifySelection();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CSimplifySelection)

// Form Data
public:
	//{{AFX_DATA(CSimplifySelection)
	enum { IDD = IDD_SIMPLIFY_SELECTION };
	CListCtrl	m_List;
	CDataSource	m_DataSource;
	//}}AFX_DATA

// Attributes
public:

// Operations
public:
	bool IsEmpty();
	void SetItemStatus(int item, int status);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSimplifySelection)
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CSimplifySelection();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
	//{{AFX_MSG(CSimplifySelection)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnItemChanged(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	int m_SelectedItem;
	CImageList m_Images;
	void FillSelectionList(void);
	CString DataSourceAccessSync(BSTR* command);
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SIMPLIFYSELECTION_H__B7C99900_E891_11D2_AFB4_00A0C9B71DC4__INCLUDED_)
