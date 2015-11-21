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
#ifndef __DATASOURCE__
   #define __DATASOURCE__
   #include "datasource.h"
#endif

#ifndef __PROJECTTREE__
   #define __PROJECTTREE__
   #include "projecttree.h"
#endif

#ifndef __PROJECTMODULES__
   #define __PROJECTMODULES__
   #include "projectmodules.h"
#endif

#if !defined(AFX_PROJECTBROWSER_H__110ADEF2_2244_11D2_AEF9_00A0C9B71DC4__INCLUDED_)
#define AFX_PROJECTBROWSER_H__110ADEF2_2244_11D2_AEF9_00A0C9B71DC4__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// ProjectBrowser.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CProjectBrowser form view

#ifndef __AFXEXT_H__
#include <afxext.h>
#endif

class CProjectBrowser : public CFormView 
{
protected:
	CProjectBrowser();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CProjectBrowser)

// Form Data
public:
	//{{AFX_DATA(CProjectBrowser)
	enum { IDD = IDD_PROJECTBROWSER };
	CTabCtrl	m_ModeSelector;
	CDataSource	m_DataSource;
	CProjectModules	m_ProjectModules;
	CProjectTree	m_ProjectTree;
	//}}AFX_DATA

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CProjectBrowser)
	public:
	virtual void OnFinalRelease();
	virtual void OnInitialUpdate();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	CString m_CurrentProject;
	virtual ~CProjectBrowser();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
	//{{AFX_MSG(CProjectBrowser)
	afx_msg void OnNodeChanged(LPCTSTR projectName);
	afx_msg void OnProjectChanged(LPCTSTR Project);
	afx_msg void OnModeChanged(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	// Generated OLE dispatch map functions
	//{{AFX_DISPATCH(CProjectBrowser)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_DISPATCH
	DECLARE_DISPATCH_MAP()
	DECLARE_INTERFACE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PROJECTBROWSER_H__110ADEF2_2244_11D2_AEF9_00A0C9B71DC4__INCLUDED_)
