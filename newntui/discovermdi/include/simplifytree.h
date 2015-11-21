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
#if !defined(AFX_SIMPLIFYTREE_H__B7C998F6_E891_11D2_AFB4_00A0C9B71DC4__INCLUDED_)
#define AFX_SIMPLIFYTREE_H__B7C998F6_E891_11D2_AFB4_00A0C9B71DC4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SimplifyTree.h : header file
//

#ifndef __DATASOURCE_H
   #define __DATASOURCE_H
   #include "datasource.h"
#endif

struct CIncludeTreeNode {
	int     m_Level;
	int     m_Line;
	int     m_Marked;
	int     m_ID;
	CString m_FileName;
	BOOL    m_Status;
};


/////////////////////////////////////////////////////////////////////////////
// CSimplifyTree form view

#ifndef __AFXEXT_H__
#include <afxext.h>
#endif

class CSimplifyTree : public CFormView
{
protected:
	CSimplifyTree();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CSimplifyTree)

// Form Data
public:
	//{{AFX_DATA(CSimplifyTree)
	enum { IDD = IDD_SIMPLIFY_TREE };
	CTreeCtrl	m_Tree;
	CDataSource	m_DataSource;
	//}}AFX_DATA

// Attributes
public:

// Operations
public:
	bool IsLicensed();
	BOOL m_SimplifyRunning;
	BOOL       m_Substitute;
	int m_FilesToExclude;
	CString m_FileName;
	void Done(void);
	BOOL GetInclusionMode(void);
	void EnableInclusionMode(BOOL enable);
	void SetNewElement(int element, const CString& filename);
	CString GetShortName(const CString& longName);
    int FindNodeImage(CIncludeTreeNode* node);
	void Close();
   	CString DataSourceAccessSync(BSTR* command);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSimplifyTree)
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CSimplifyTree();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
	//{{AFX_MSG(CSimplifyTree)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnClickTree(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSimplifyReady();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	bool m_bLicensed;
	CString m_SavedFileName;
	int m_SavedSelection;
	void EndSimplify(void);
	BOOL m_FilesUpdated;
	int m_CurrentItem;
	CImageList m_Images;
	int        m_PropagateLevel;
	void ParseAndFillTree(int element,const CString& filename);
    void ChildPropagateStatus(HTREEITEM item, BOOL newStatus);
    HTREEITEM GetFirstIncludedItem(HTREEITEM item);
    HTREEITEM GetSourceFileItem(HTREEITEM item);
	void DisableAllExclusions(HTREEITEM item);
    void DisableMarkedExclusions(HTREEITEM item);
    void ClearTree(HTREEITEM item);

};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SIMPLIFYTREE_H__B7C998F6_E891_11D2_AFB4_00A0C9B71DC4__INCLUDED_)
