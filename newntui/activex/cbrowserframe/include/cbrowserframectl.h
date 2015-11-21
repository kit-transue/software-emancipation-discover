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
// CBrowserFrameCtl.h : Declaration of the CBrowserFrameCtrl OLE control class.
#include "TabSelector.h"
#ifndef __QUERYRESULTS_H
   #define __QUERYRESULTS_H
   #include "queryresults.h"
#endif

/////////////////////////////////////////////////////////////////////////////
// CBrowserFrameCtrl : See CBrowserFrameCtl.cpp for implementation.

#define CATEGORIES_MODE 0
#define QUERIES_MODE    1
#define GROUPS_MODE     2

#define IDC_MODESELECTOR  100
#define IDC_RESULTS       101
#define IDC_STATUSBAR     102


class CBrowserFrameCtrl : public COleControl
{
	DECLARE_DYNCREATE(CBrowserFrameCtrl)

// Constructor
public:
	CBrowserFrameCtrl();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBrowserFrameCtrl)
	public:
	virtual void OnDraw(CDC* pdc, const CRect& rcBounds, const CRect& rcInvalid);
	virtual void DoPropExchange(CPropExchange* pPX);
	virtual void OnResetState();
	//}}AFX_VIRTUAL

// Implementation
protected:
	~CBrowserFrameCtrl();

	DECLARE_OLECREATE_EX(CBrowserFrameCtrl)    // Class factory and guid
	DECLARE_OLETYPELIB(CBrowserFrameCtrl)      // GetTypeInfo
	DECLARE_PROPPAGEIDS(CBrowserFrameCtrl)     // Property page IDs
	DECLARE_OLECTLTYPE(CBrowserFrameCtrl)		// Type name and misc status

// Message maps
	//{{AFX_MSG(CBrowserFrameCtrl)
	afx_msg int  OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnModeChanged(short mode);
    afx_msg void OnCategoryChanged(LPCTSTR categoriesList);
    afx_msg void OnQueryChanged(LPCTSTR queriesList);
    afx_msg void OnGroupChanged(LPCTSTR groupsList);
    afx_msg void OnSelectionChanged(LPCTSTR pstrParams);
	afx_msg void OnElementDoubleclicked(LPCTSTR parmsList);
    afx_msg void OnRButtonEvent(short x,short y);
	afx_msg void OnSelectSort(void);
	afx_msg void OnSetAttributes(void);
	afx_msg void OnSetFilter(void);
	afx_msg void OnResetFilter(void);
    afx_msg void OnAddElements(void);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
// Dispatch maps
	//{{AFX_DISPATCH(CBrowserFrameCtrl)
	afx_msg BSTR GetArguments();
	afx_msg void SetArguments(LPCTSTR lpszNewValue);
	afx_msg BSTR GetResultsFromCategory();
	afx_msg void SetResultsFromCategory(LPCTSTR lpszNewValue);
	afx_msg BSTR GetResultsFromQuery();
	afx_msg void SetResultsFromQuery(LPCTSTR lpszNewValue);
	afx_msg BSTR GetResultsFromGroup();
	afx_msg void SetResultsFromGroup(LPCTSTR lpszNewValue);
	afx_msg BSTR GetSelectionFromQuery();
	afx_msg void SetSelectionFromQuery(LPCTSTR lpszNewValue);
	afx_msg BSTR GetSelectionFromCategory();
	afx_msg void SetSelectionFromCategory(LPCTSTR lpszNewValue);
	afx_msg BSTR GetSelectionFromGroup();
	afx_msg void SetSelectionFromGroup(LPCTSTR lpszNewValue);
	afx_msg BSTR GetProjects();
	afx_msg void SetProjects(LPCTSTR lpszNewValue);
	afx_msg short GetMode();
	afx_msg void SetMode(short nNewValue);
	afx_msg LPDISPATCH GetDataSource();
	afx_msg void SetDataSource(LPDISPATCH newValue);
	afx_msg BSTR GetSelectedCategories();
	afx_msg void SetSelectedCategories(LPCTSTR lpszNewValue);
	afx_msg void Update();
	afx_msg void SetSet(LPCTSTR varname);
	//}}AFX_DISPATCH
	DECLARE_DISPATCH_MAP()

	afx_msg void AboutBox();

// Event maps
	//{{AFX_EVENT(CBrowserFrameCtrl)
	void FireModeChanged(short modeID)
		{FireEvent(eventidModeChanged,EVENT_PARAM(VTS_I2), modeID);}
	void FireCategoryChanged(LPCTSTR list)
		{FireEvent(eventidCategoryChanged,EVENT_PARAM(VTS_BSTR), list);}
	void FireQueryChanged(LPCTSTR list)
		{FireEvent(eventidQueryChanged,EVENT_PARAM(VTS_BSTR), list);}
	void FireGroupChanged(LPCTSTR list)
		{FireEvent(eventidGroupChanged,EVENT_PARAM(VTS_BSTR), list);}
	void FireElementDoubleclicked(LPCTSTR parmstr)
		{FireEvent(eventidElementDoubleclicked,EVENT_PARAM(VTS_BSTR), parmstr);}
	void FireSelectionChanged(LPCTSTR describe)
		{FireEvent(eventidSelectionChanged,EVENT_PARAM(VTS_BSTR), describe);}
	void FireSubsystemsNeedUpdate()
		{FireEvent(eventidSubsystemsNeedUpdate,EVENT_PARAM(VTS_NONE));}
	//}}AFX_EVENT
	DECLARE_EVENT_MAP()

// Dispatch and event IDs
public:
	void UpdateSubsystems(void);
	enum {
	//{{AFX_DISP_ID(CCBrowserFrameCtrl)
	dispidArguments  =1,
	dispidResultsFromCategory =2,
	dispidResultsFromQuery =3,
	dispidResultsFromGroup =4,
	dispidSelectionFromQuery =5,
	dispidSelectionFromCategory =6,
	dispidSelectionFromGroup =7,
	dispidProjects =8,
	dispidMode =9,
	dispidDataSource =10,
	dispidSelectedCategories =11,
	dispidUpdate = 12L,
	dispidSetSet = 13L,
	eventidModeChanged=1,
	eventidCategoryChanged=2,
	eventidQueryChanged=3,
	eventidSelectionChanged=4,
    eventidGroupChanged=5,
    eventidElementDoubleclicked=6,
    eventidSubsystemsNeedUpdate=7
	//}}AFX_DISP_ID
	};
	/*dispidArguments  =1,
	dispidResultsFromCategory =2,
	dispidResultsFromQuery =3,
	dispidResultsFromGroup =4,
	dispidSelectionFromQuery =5,
	dispidSelectionFromCategory =6,
	dispidSelectionFromGroup =7,
	dispidProjects =8,
	dispidMode =9,
	dispidDataSource =10,
	dispidSelectedCategories =11,
	dispidCategoriesEnable = 12L,
	dispidUpdate = 13L,
	eventidModeChanged=1,
	eventidCategoryChanged=2,
	eventidQueryChanged=3,
	eventidSelectionChanged=4,
    eventidGroupChanged=5*/
	CTabSelector  m_ModeSelector;
	CQueryResults m_QueryResults;
	short         m_ModeID;
	CString       m_SelectionFromGroup;
	CString       m_SelectionFromQuery;
	CString       m_SelectionFromCategory;
private:
	BOOL m_DurtyFlag;
	void FormCategoryRequest(CString& categoriesList);
	void FormQueryRequest(CString& queriesList);
    void DisplayInfoString(void);

	LPDISPATCH    m_DataSource;
	CString       m_SelectedCategories;
	CString       m_SavedCategoryRequest;
	CString       m_SavedQueryRequest;
	CString       m_Projects;
	CString       m_ResultsFromGroup;
	CString       m_ResultsFromQuery;
	CString       m_ResultsFromCategory;
	CString       m_Arguments;
	CStatic       m_InfoBar;
	CFont         m_InfoFont;
};
