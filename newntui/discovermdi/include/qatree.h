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
#if !defined(AFX_QATREE_H__447BA2A2_59FC_11D2_AF35_00A0C9B71DC4__INCLUDED_)
#define AFX_QATREE_H__447BA2A2_59FC_11D2_AF35_00A0C9B71DC4__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// QATree.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CQATree form view

#ifndef __AFXEXT_H__
#include <afxext.h>
#endif
#include "qaerrors.h"	// Added by ClassView

struct CTreeNodeInfo {
	CString m_NodeName;
	CString m_NodeType;
	BOOL    m_RootEnabled;
};

class CQATree : public CFormView
{
protected:
	CQATree();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CQATree)

// Form Data
public:
	//{{AFX_DATA(CQATree)
	enum { IDD = IDD_QATREE };
	CStatic	m_Title;
	CTreeCtrl	m_Tree;
	CDataSource	m_DataSource;
	//}}AFX_DATA

// Attributes
public:
	void OnReset();
	void OnRunAll();
	void OnRunSelected();

// Operations
public:
	void GenerateReport(CString& to);
	CString GetReportName();
	void FillNewTree();
	void Save(void);
	BOOL Open(void);
	void Update(void);
	CString* GetCurrentNodeName(void);
	void SetCurrentItemStatus(BOOL status);
	void AddChildren(HTREEITEM to, CString& path, BOOL rootStatus=TRUE);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CQATree)
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	int QueryChildren(CString& from,CString& list);
	void FillTree(void);
	virtual ~CQATree();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
	//{{AFX_MSG(CQATree)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnClickQatree(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSelchanged(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnKeydownQatree(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnClose();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	bool m_bLicensed;
	CImageList m_ImageList;
	void LayoutControls(int cx, int cy);
    void PropagateStatus(HTREEITEM node, BOOL newStatus);
	void Free(HTREEITEM root);
	CString DataSourceAccessSync(BSTR* command);
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_QATREE_H__447BA2A2_59FC_11D2_AF35_00A0C9B71DC4__INCLUDED_)
