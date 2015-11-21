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
#include <afxcmn.h>
#include "NewSelectionList.h"

// CategoriesComboCtl.h : Declaration of the CCategoriesComboCtrl OLE control class.
#define IDC_CATEGORIESCOMBO   1246
#define IDC_CATEGORIESLIST    1247
#define IDC_CATEGORIESBUTTON  1248

#define EVAL_OK               1
#define ERROR_DATA_SOURCE    -1
#define ERROR_ACCESS_SUPPORT -2
#define ERROR_ACCESS_COMMAND -3


/////////////////////////////////////////////////////////////////////////////
// CCategoriesComboCtrl : See CategoriesComboCtl.cpp for implementation.

class CCategoriesComboCtrl : public COleControl
{
	DECLARE_DYNCREATE(CCategoriesComboCtrl)

// Constructor
public:
	CCategoriesComboCtrl();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCategoriesComboCtrl)
public:
	virtual void OnDraw(CDC* pdc, const CRect& rcBounds, const CRect& rcInvalid);
	virtual void DoPropExchange(CPropExchange* pPX);
	virtual void OnResetState();
	//}}AFX_VIRTUAL

// Implementation
protected:
	~CCategoriesComboCtrl();

	DECLARE_OLECREATE_EX(CCategoriesComboCtrl)    // Class factory and guid
	DECLARE_OLETYPELIB(CCategoriesComboCtrl)      // GetTypeInfo
	DECLARE_PROPPAGEIDS(CCategoriesComboCtrl)     // Property page IDs
	DECLARE_OLECTLTYPE(CCategoriesComboCtrl)		// Type name and misc status

// Message maps
	//{{AFX_MSG(CCategoriesComboCtrl)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg int  OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnCategoryChanged(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg LRESULT OnUserSelection(WPARAM,LPARAM);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// Dispatch maps
	//{{AFX_DISPATCH(CCategoriesComboCtrl)
	afx_msg LPDISPATCH GetDataSource();
	afx_msg void SetDataSource(LPDISPATCH newValue);
	afx_msg BSTR AccessFromName(LPCTSTR name);
	afx_msg BSTR GetCategoriesList();
	afx_msg void Update();
	afx_msg void Unselect();
	//}}AFX_DISPATCH
	DECLARE_DISPATCH_MAP()

	afx_msg void AboutBox();

// Event maps
	//{{AFX_EVENT(CCategoriesComboCtrl)
	void FireCategoryChanged(LPCTSTR categoriesList)
		{FireEvent(eventidCategoryChanged,EVENT_PARAM(VTS_BSTR), categoriesList);}
	//}}AFX_EVENT
	DECLARE_EVENT_MAP()

// Dispatch and event IDs
public:
	enum {
	//{{AFX_DISP_ID(CCategoriesComboCtrl)
	dispidDataSource = 1L,
	dispidAccessFromName = 2L,
	dispidGetCategoriesList = 3L,
	dispidUpdate = 4L,
	dispidUnselect = 5L,
	eventidCategoryChanged = 1L,
	//}}AFX_DISP_ID
	};
private:
	BOOL m_InSelection;
	void FillList(void);
	int  EvaluateSync(CString& command, CString* results=NULL);
private:
	int IconIndex(const CString& name);
	CString            m_Selection;
	CImageList         m_ImageList;
	CNewSelectionList  m_ListView;
	int                m_Items;
	LPDISPATCH         m_DataSource;
};
