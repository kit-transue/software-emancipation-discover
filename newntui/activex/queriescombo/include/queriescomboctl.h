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
// QueriesComboCtl.h : Declaration of the CQueriesComboCtrl OLE control class.
#include <afxcmn.h>
#include "NewSelectionList.h"
#define IDC_QUERIESCOMBO   1256
#define IDC_QUERIESLIST    1257
#define IDC_QUERIESBUTTON  1258

#define EVAL_OK               1
#define ERROR_DATA_SOURCE    -1
#define ERROR_ACCESS_SUPPORT -2
#define ERROR_ACCESS_COMMAND -3

/////////////////////////////////////////////////////////////////////////////
// CQueriesComboCtrl : See QueriesComboCtl.cpp for implementation.

class CQueriesComboCtrl : public COleControl
{
	DECLARE_DYNCREATE(CQueriesComboCtrl)

// Constructor
public:
	CString m_Categories;
	CQueriesComboCtrl();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CQueriesComboCtrl)
	public:
	virtual void OnDraw(CDC* pdc, const CRect& rcBounds, const CRect& rcInvalid);
	virtual void DoPropExchange(CPropExchange* pPX);
	virtual void OnResetState();
	//}}AFX_VIRTUAL

// Implementation
protected:
	~CQueriesComboCtrl();

	DECLARE_OLECREATE_EX(CQueriesComboCtrl)    // Class factory and guid
	DECLARE_OLETYPELIB(CQueriesComboCtrl)      // GetTypeInfo
	DECLARE_PROPPAGEIDS(CQueriesComboCtrl)     // Property page IDs
	DECLARE_OLECTLTYPE(CQueriesComboCtrl)		// Type name and misc status

// Message maps
	//{{AFX_MSG(CQueriesComboCtrl)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnQueryChanged(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg LRESULT OnUserSelection(WPARAM,LPARAM);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// Dispatch maps
	//{{AFX_DISPATCH(CQueriesComboCtrl)
	afx_msg LPDISPATCH GetDataSource();
	afx_msg void SetDataSource(LPDISPATCH newValue);
	afx_msg BSTR GetSelection();
	afx_msg void SetSelection(LPCTSTR lpszNewValue);
	afx_msg BSTR GetCategories();
	afx_msg void SetCategories(LPCTSTR lpszNewValue);
	afx_msg BSTR AccessFromName(LPCTSTR name);
	afx_msg void Update();
	afx_msg BSTR GetQueriesList();
	afx_msg void Clear();
	afx_msg BSTR GetSelectedQueries();
	//}}AFX_DISPATCH
	DECLARE_DISPATCH_MAP()

	afx_msg void AboutBox();

// Event maps
	//{{AFX_EVENT(CQueriesComboCtrl)
	void FireQueryChanged(LPCTSTR queryList)
		{FireEvent(eventidQueryChanged,EVENT_PARAM(VTS_BSTR), queryList);}
	//}}AFX_EVENT
	DECLARE_EVENT_MAP()

// Dispatch and event IDs
public:
	enum {
	//{{AFX_DISP_ID(CQueriesComboCtrl)
	dispidDataSource = 1L,
	dispidSelection = 2L,
	dispidCategories = 3L,
	dispidAccessFromName = 4L,
	dispidUpdate = 5L,
	dispidGetQueriesList = 6L,
	dispidClear = 7L,
	dispidGetSelectedQueries = 8L,
	eventidQueryChanged = 1L,
	//}}AFX_DISP_ID
	};
private:
	CString m_SelectedQueriesList;
	void FormQueriesList(void);
	CString m_QueriesList;
	void FillList();
    int EvaluateSync(CString& command, CString* results=NULL);
private:
	BOOL               m_InSelection;
	CString            m_Selection;
	CNewSelectionList  m_ListView;
	CImageList         m_ImageList;
	LPDISPATCH         m_DataSource;
	int                m_Items;
};
