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
#include "msdevintegrator.h"
//}}AFX_INCLUDES
#ifndef __DATASOURCE_H
   #define __DATASOURCE_H
   #include "datasource.h"
#endif
#ifndef __BROWSERFRAME_H
   #define __BROWSERFRAME_H
   #include "cbrowserframe.h"
#endif
#include "DiscoverMDIDoc.h"

#ifndef __PROJECTTREE__
   #define __PROJECTTREE__
   #include "ProjectTree.h"
#endif

#ifndef __PROJECTMODULES__
   #define __PROJECTMODULES__
   #include "projectmodules.h"
#endif

#include "sash.h"

#if !defined(AFX_MINIBROWSER_H__220505F3_593F_11D2_AF31_00A0C9B71DC4__INCLUDED_)
#define AFX_MINIBROWSER_H__220505F3_593F_11D2_AF31_00A0C9B71DC4__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// MiniBrowser.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CMiniBrowser form view

#ifndef __AFXEXT_H__
#include <afxext.h>
#endif

class CDiscoverMDICntrItem;


class CMiniBrowser : public CFormView
{
protected:
	CMiniBrowser();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CMiniBrowser)

// Form Data
public:
	//{{AFX_DATA(CMiniBrowser)
	enum { IDD = IDD_MINIBROWSER };
	CTabCtrl	m_ModeSelector;
	CDataSource	m_DataSource;
	CCBrowserFrame	m_Left;
	CCBrowserFrame	m_Right;
	CProjectTree	m_Tree;
	CProjectModules m_Modules;
	CMSDEVIntegrator	m_MSDEVIntegrator;
	//}}AFX_DATA

// Attributes
public:
public:
	CDiscoverMDIDoc* GetDocument();
	// m_pSelection holds the selection to the current CDiscoverMDICntrItem.
	// For many applications, such a member variable isn't adequate to
	//  represent a selection, such as a multiple selection or a selection
	//  of objects that are not CDiscoverMDICntrItem objects.  This selection
	//  mechanism is provided just to help you get started.

	// TODO: replace this selection mechanism with one appropriate to your app.
	CDiscoverMDICntrItem* m_pSelection;
// Operations
public:
	CString m_CurrentProject;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMiniBrowser)
	public:
	virtual void OnFinalRelease();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void OnInitialUpdate(); // called first time after construct
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual BOOL IsSelected(const CObject* pDocItem) const;// Container support
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CMiniBrowser();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
	//{{AFX_MSG(CMiniBrowser)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnDestroy();
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnInsertObject();
	afx_msg void OnCancelEditCntr();
	afx_msg void OnCancelEditSrvr();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg LRESULT OnSplitMoved(WPARAM wparam, LPARAM lparam);
	afx_msg void OnNodeChanged(LPCTSTR projectName);
	afx_msg void OnModeChanged(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnLeftModeChanged(short newMode);
	afx_msg void OnLeftCategoryChanged(LPCTSTR categoriesList);
	afx_msg void OnLeftSelectionChanged(LPCTSTR varName);
	afx_msg void OnRightCategoryChanged(LPCTSTR categoriesList);
	afx_msg void OnRightSelectionChanged(LPCTSTR varName);
	afx_msg void IntegratorCtrlClick(LPCTSTR filename, long line, long column, long token);
	afx_msg void IntegratorQuery(LPCTSTR fname, long line, long col, long len);
	afx_msg void IntegratorDefinition(LPCTSTR filename, long line, long col, long len);
	afx_msg void IntegratorActivate();
	afx_msg void OnDoubleclick(LPCTSTR parms);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	// Generated OLE dispatch map functions
	//{{AFX_DISPATCH(CMiniBrowser)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_DISPATCH
	DECLARE_DISPATCH_MAP()
	DECLARE_INTERFACE_MAP()

	CSash m_DividerSash;
private:
	void LayoutWindows(int cx, int cy);
	int m_SplitterPos;
	BOOL m_Updated;
	CString DataSourceAccessSync(BSTR* command);
};

#ifndef _DEBUG  // debug version in DiscoverMDIView.cpp
inline CDiscoverMDIDoc* CMiniBrowser::GetDocument()
   { return (CDiscoverMDIDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MINIBROWSER_H__220505F3_593F_11D2_AF31_00A0C9B71DC4__INCLUDED_)
