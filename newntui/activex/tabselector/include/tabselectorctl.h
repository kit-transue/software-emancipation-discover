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
// TabSelectorCtl.h : Declaration of the CTabSelectorCtrl OLE control class.
#include <afxcmn.h>
#include "CategoriesCombo.h"
#include "QueriesCombo.h"
#include "GroupsCombo.h"

#define IDC_TAB        100
#define IDC_CATEGORIES 101
#define IDC_QUERIES    102
#define IDC_GROUPS     103

/////////////////////////////////////////////////////////////////////////////
// CTabSelectorCtrl : See TabSelectorCtl.cpp for implementation.

class CTabSelectorCtrl : public COleControl
{
	DECLARE_DYNCREATE(CTabSelectorCtrl)

// Constructor
public:
	CTabSelectorCtrl();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTabSelectorCtrl)
	public:
	virtual void OnDraw(CDC* pdc, const CRect& rcBounds, const CRect& rcInvalid);
	virtual void DoPropExchange(CPropExchange* pPX);
	virtual void OnResetState();
	//}}AFX_VIRTUAL

// Implementation
protected:
	~CTabSelectorCtrl();

	DECLARE_OLECREATE_EX(CTabSelectorCtrl)    // Class factory and guid
	DECLARE_OLETYPELIB(CTabSelectorCtrl)      // GetTypeInfo
	DECLARE_PROPPAGEIDS(CTabSelectorCtrl)     // Property page IDs
	DECLARE_OLECTLTYPE(CTabSelectorCtrl)		// Type name and misc status

// Message maps
	//{{AFX_MSG(CTabSelectorCtrl)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnSelchangeTab(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnCategoryChanged(LPCSTR catList);
    afx_msg void OnGroupChanged(LPCSTR catList);
    afx_msg void OnQueryChanged(LPCSTR queriesList);
    afx_msg void OnGroupCreated(LPCSTR name);
    afx_msg void OnGroupRemoved(short amount);
    afx_msg void OnGroupInsSel(void);
    afx_msg void OnGroupDelSel(void);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// Dispatch maps
	//{{AFX_DISPATCH(CTabSelectorCtrl)
	afx_msg LPDISPATCH GetDataSource();
	afx_msg void SetDataSource(LPDISPATCH newValue);
	afx_msg BSTR GetOutputVariable();
	afx_msg void SetOutputVariable(LPCTSTR lpszNewValue);
	afx_msg BSTR GetSelectionVariable();
	afx_msg void SetSelectionVariable(LPCTSTR lpszNewValue);
	afx_msg BSTR GetSelectedCategories();
	afx_msg void SetSelectedCategories(LPCTSTR lpszNewValue);
	afx_msg BSTR AccessFromName(LPCTSTR name);
	afx_msg void Update();
	afx_msg BSTR AccessFromCategory(LPCTSTR category);
	afx_msg BSTR AccessFromQuery(LPCTSTR query);
	afx_msg void Unselect();
	//}}AFX_DISPATCH
	DECLARE_DISPATCH_MAP()

	afx_msg void AboutBox();

// Event maps
	//{{AFX_EVENT(CTabSelectorCtrl)
	void FireModeChanged(short modeID)
		{FireEvent(eventidModeChanged,EVENT_PARAM(VTS_I2), modeID);}
	void FireCategoryChanged(LPCTSTR catList)
		{FireEvent(eventidCategoryChanged,EVENT_PARAM(VTS_BSTR), catList);}
	void FireQueryChanged(LPCTSTR name)
		{FireEvent(eventidQueryChanged,EVENT_PARAM(VTS_BSTR), name);}
	void FireGroupChanged(LPCTSTR varName)
		{FireEvent(eventidGroupChanged,EVENT_PARAM(VTS_BSTR), varName);}
	//}}AFX_EVENT
	DECLARE_EVENT_MAP()

// Dispatch and event IDs
public:
	enum {
	//{{AFX_DISP_ID(CTabSelectorCtrl)
	dispidDataSource = 1L,
	dispidOutputVariable = 2L,
	dispidSelectionVariable = 3L,
	dispidSelectedCategories = 4L,
	dispidAccessFromName = 5L,
	dispidUpdate = 6L,
	dispidAccessFromCategory = 7L,
	dispidAccessFromQuery = 8L,
	dispidUnselect = 9L,
	eventidModeChanged = 1L,
	eventidCategoryChanged = 2L,
	eventidQueryChanged = 3L,
	eventidGroupChanged = 4L,
	//}}AFX_DISP_ID
	};
private:
	BOOL m_EnableCategories;
	CString m_SelectedCategories;
	CFont infoFont;
	CString m_Projects;
	CString m_SelectionVar;
	LPDISPATCH m_DataSource;
	CTabCtrl m_Tab;
	CGroupsCombo m_Groups;
	CQueriesCombo m_Queries;
	CCategoriesCombo m_Categories;
};
