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
#if !defined(AFX_IMPACTCTL_H__66DE19FE_CC07_4996_A166_9EF617757CBF__INCLUDED_)
#define AFX_IMPACTCTL_H__66DE19FE_CC07_4996_A166_9EF617757CBF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// ImpactCtl.h : Declaration of the CImpactCtrl ActiveX Control class.

#include "ImpactActionsSet.h"
#include "ImpactItem.h"
#include "CheckedTree.h"
#include "ImpactPageSelector.h"
#include "ReportViewer.h"

/////////////////////////////////////////////////////////////////////////////
// CImpactCtrl : See ImpactCtl.cpp for implementation.

class CImpactCtrl : public COleControl
{
	DECLARE_DYNCREATE(CImpactCtrl)

// Constructor
public:
	CImpactCtrl();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CImpactCtrl)
	public:
	virtual void OnDraw(CDC* pdc, const CRect& rcBounds, const CRect& rcInvalid);
	virtual void DoPropExchange(CPropExchange* pPX);
	virtual void OnResetState();
	virtual BOOL DestroyWindow();
	//}}AFX_VIRTUAL

// Implementation
protected:
	~CImpactCtrl();

	DECLARE_OLECREATE_EX(CImpactCtrl)    // Class factory and guid
	DECLARE_OLETYPELIB(CImpactCtrl)      // GetTypeInfo
	DECLARE_PROPPAGEIDS(CImpactCtrl)     // Property page IDs
	DECLARE_OLECTLTYPE(CImpactCtrl)		// Type name and misc status

// Message maps
	//{{AFX_MSG(CImpactCtrl)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
	afx_msg LRESULT OnItemStateChanged(WPARAM wparm, LPARAM lparm);
	afx_msg LRESULT OnOpenInstance(WPARAM wparm, LPARAM lparm);
	DECLARE_MESSAGE_MAP()

// Dispatch maps
	//{{AFX_DISPATCH(CImpactCtrl)
	afx_msg LPDISPATCH GetDataSource();
	afx_msg void SetDataSource(LPDISPATCH newValue);
	afx_msg LPDISPATCH GetEditorCtrl();
	afx_msg void SetEditorCtrl(LPDISPATCH newValue);
	afx_msg BOOL Update();
	afx_msg BOOL SaveReportTo(LPCTSTR szFileName);
	afx_msg BOOL IsReportView();
	afx_msg BSTR GetReportName();
	//}}AFX_DISPATCH
	DECLARE_DISPATCH_MAP()

// Event maps
	//{{AFX_EVENT(CImpactCtrl)
	//}}AFX_EVENT
	DECLARE_EVENT_MAP()

// Dispatch and event IDs
public:
	int GetIconIdx(int iconID);
	CImpactActionsSet* CreateActions(CEntity* pEntity);
	void Clean();
	int GetIconIdx(CString kind);
	CString ExecCommand(CString& command);
	CString ExecPrint(CString& command);
	enum {
	//{{AFX_DISP_ID(CImpactCtrl)
	dispidDataSource = 1L,
	dispidEditorCtrl = 2L,
	dispidUpdate = 3L,
	dispidSaveReportTo = 4L,
	dispidIsReportView = 5L,
	dispidGetReportName = 6L,
	//}}AFX_DISP_ID
	};
private:
	BOOL AddAction(HTREEITEM parent,HTREEITEM after,CImpactAction* pAction);
	CImageList m_PagesIcons;
	CReportViewer m_wndReport;
	CImpactPageSelector m_wndPages;
	void DeleteResults(HTREEITEM iRoot);
	void DisplayActionResults(HTREEITEM hItem,CResultsTree* pRoot);
	void AddActions(HTREEITEM item, CImpactItem* pItem);
	const CString m_cszError;

	static const int IconsIdxs[];

	LPDISPATCH m_DataSource;
	LPDISPATCH m_EditorCtrl;
	CImageList m_TypeIcons;
	CCheckedTree m_ImpactTree;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_IMPACTCTL_H__66DE19FE_CC07_4996_A166_9EF617757CBF__INCLUDED)
