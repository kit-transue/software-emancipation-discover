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
// GroupsComboCtl.h : Declaration of the CGroupsComboCtrl OLE control class.
#include "NewSelectionList.h"

#define IDC_GROUPSLIST 100
#define IDC_GROUPTOOLS 101

#define EVAL_OK               1
#define ERROR_DATA_SOURCE    -1
#define ERROR_ACCESS_SUPPORT -2
#define ERROR_ACCESS_COMMAND -3

/////////////////////////////////////////////////////////////////////////////
// CGroupsComboCtrl : See GroupsComboCtl.cpp for implementation.

class CGroupsComboCtrl : public COleControl
{
	DECLARE_DYNCREATE(CGroupsComboCtrl)

// Constructor
public:
	CGroupsComboCtrl();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGroupsComboCtrl)
	public:
	virtual void OnDraw(CDC* pdc, const CRect& rcBounds, const CRect& rcInvalid);
	virtual void DoPropExchange(CPropExchange* pPX);
	virtual void OnResetState();
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual void FillList(void);
	~CGroupsComboCtrl();

	DECLARE_OLECREATE_EX(CGroupsComboCtrl)    // Class factory and guid
	DECLARE_OLETYPELIB(CGroupsComboCtrl)      // GetTypeInfo
	DECLARE_PROPPAGEIDS(CGroupsComboCtrl)     // Property page IDs
	DECLARE_OLECTLTYPE(CGroupsComboCtrl)		// Type name and misc status

// Message maps
	//{{AFX_MSG(CGroupsComboCtrl)
	afx_msg int  OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnGroupChanged(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg LRESULT OnUserSelection(WPARAM,LPARAM);
	afx_msg void OnShowAll(void);
	afx_msg void OnShowIntersection(void);
	afx_msg void OnShowDiff(void);
	afx_msg void OnNewGroup(void);
	afx_msg void OnRemoveGroup(void);
	afx_msg void OnAddSel(void);
	afx_msg void OnRemoveSel(void);
	afx_msg void OnItemEditEnd(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnItemEditStart(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnFindFirst(void);
	afx_msg void OnFindNext(void);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// Dispatch maps
	//{{AFX_DISPATCH(CGroupsComboCtrl)
	afx_msg BSTR GetSelectionVariable();
	afx_msg void SetSelectionVariable(LPCTSTR lpszNewValue);
	afx_msg LPDISPATCH GetDataSource();
	afx_msg void SetDataSource(LPDISPATCH newValue);
	afx_msg BSTR GetOutputVariable();
	afx_msg void SetOutputVariable(LPCTSTR lpszNewValue);
	afx_msg BOOL GetControlsEnable();
	afx_msg void SetControlsEnable(BOOL bNewValue);
	afx_msg void Update();
	afx_msg BSTR GetSelectedGroups();
	afx_msg void CreateGroup();
	afx_msg void AddElements();
	//}}AFX_DISPATCH
	DECLARE_DISPATCH_MAP()

	afx_msg void AboutBox();

// Event maps
	//{{AFX_EVENT(CGroupsComboCtrl)
	void FireGroupChanged(LPCTSTR newGroup)
		{FireEvent(eventidGroupChanged,EVENT_PARAM(VTS_BSTR), newGroup);}
	void FireGroupCreated(LPCTSTR name)
		{FireEvent(eventidGroupCreated,EVENT_PARAM(VTS_BSTR), name);}
	void FireGroupRemoved(short amount)
		{FireEvent(eventidGroupRemoved,EVENT_PARAM(VTS_I2), amount);}
	void FireGroupRenamed(LPCTSTR newName)
		{FireEvent(eventidGroupRenamed,EVENT_PARAM(VTS_BSTR), newName);}
	void FireSelectionInserted()
		{FireEvent(eventidSelectionInserted,EVENT_PARAM(VTS_NONE));}
	void FireSelectionRemoved()
		{FireEvent(eventidSelectionRemoved,EVENT_PARAM(VTS_NONE));}
	//}}AFX_EVENT
	DECLARE_EVENT_MAP()

// Dispatch and event IDs
public:
	BOOL m_ControlsEnable;
	enum {
	//{{AFX_DISP_ID(CGroupsComboCtrl)
	dispidSelectionVariable = 1L,
	dispidDataSource = 2L,
	dispidOutputVariable = 3L,
	dispidControlsEnable = 4L,
	dispidUpdate = 5L,
	dispidGetSelectedGroups = 6L,
	dispidCreateGroup = 7L,
	dispidAddElements = 8L,
	eventidGroupChanged = 1L,
	eventidGroupCreated = 2L,
	eventidGroupRemoved = 3L,
	eventidGroupRenamed = 4L,
	eventidSelectionInserted = 5L,
	eventidSelectionRemoved = 6L,
	//}}AFX_DISP_ID
	};
private:
	CString m_GroupsList;
	void MakeGroupsConcat(CString& name);
	void MakeGroupsIntersect(CString& name);
	void SetToolbarStatus(void);
	void FillElementsList(void);
	CString m_OutputVar;
	BOOL m_InSelection;
    int EvaluateSync(CString& command, CString* results=NULL);

private:
	int m_FirstFound;
	CString m_Search;
	LPDISPATCH m_DataSource;
	CString m_SelectionVar;
	CToolTipCtrl m_ToolTips;
	CImageList m_ListImageList;
	CToolBarCtrl m_OpTools;
	CNewSelectionList m_ElementsList;
};
