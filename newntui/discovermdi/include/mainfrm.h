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
// MainFrm.h : interface of the CMainFrame class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_MAINFRM_H__64FA0D60_261E_11D2_AEFB_00A0C9B71DC4__INCLUDED_)
#define AFX_MAINFRM_H__64FA0D60_261E_11D2_AEFB_00A0C9B71DC4__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "DockableBrowser.h"
#include "ErrorsBrowser.h"
#include "DormantChildFrame.h"
#include "QAChildFrame.h"
#include "AccessChildFrame.h"

#include "TreeViewChildFrame.h"
#include "InheritanceViewChildFrame.h"
#include "ERDViewChildFrame.h"
#include "DataChartViewChildFrame.h"
#include "OutlineViewChildFrame.h"
#include "FlowChartViewChildFrame.h"
#include "UpdateWaitDlg.h"
#include "SimplifyChildFrame.h"
#include "ImpactChildFrame.h"

#include <afxtempl.h>

#define LIC_ACCESS            0
#define LIC_AUTODOC           1
#define LIC_DORMANT           4
#define LIC_QA                6
#define LIC_QABROWSE          7
#define LIC_QAREPORT          8
#define LIC_SIMPLIFY          9
#define LIC_TASKFLOW          11
#define LIC_TPM               12
#define LIC_PACKAGE           16

#define UM_REFRESH_FILE_LIST WM_USER + 773

class CAccessMenuItem {
public:
	void SetItemName(CString& name);
	CString& GetItemName(void);

	void SetItemAction(CString& action);
	CString& GetItemAction(void);

	void SetItemID(int id);
	int GetItemID(void);
private:
	CString m_Name;
	CString m_Action;
	int     m_ID;
};



class CAccessMenu {
public:
	CAccessMenu();
	~CAccessMenu();

    void     SetMenuName(CString& name);
    CString& GetMenuName(void);
    BOOL     AddMenu(CString& menuName,CString& defSelection);
    BOOL     RemoveMenu(CString& menuName);
    BOOL     AddMenuItem(CString& itemName,CString& action);
    BOOL     RemoveMenuItem(CString& itemName);
	BOOL     CommandFromID(int id, CString& command);
	CString& GetDefSelection(void);
	void     SetDefSelection(CString& selection);
	CAccessMenu* FindMenu(CString& menuName);
	CList<CAccessMenu*,CAccessMenu*>         m_MenusList;
	CList<CAccessMenuItem*,CAccessMenuItem*> m_ItemsList;
private:
	CString m_Name;
	CString m_DefSelection;
	static BOOL m_IDsInit;
	static BOOL m_UsedIDs[100];
};




class CMainFrame : public CMDIFrameWnd
{
	DECLARE_DYNAMIC(CMainFrame)
public:
	CMainFrame();

// Attributes
public:
    // This function will add new pop-up menu to the Access sub-menu.
	// The new pop-up menu will receve "menuName" name.
	BOOL AddAccessMenu(CString& menuName,CString& defSelection);
	// This function will try to remove menu with "menuName" name. Returns TRUE if OK.
	BOOL RemoveAccessMenu(CString& menuName);

	// This function will add new menu item to the Access sub-menu. The "menuName"
	// parameter specifies the popup menu to which we are goint to add new item or,
	// if empty, specifies that we are going to add the item to the Access menu itself.
	// "itemName" parameter specifies the name of the item, "action" parameter 
	// specifies the Access procedure call when menu item selected.
	// Will return TRUE  if menu item was added successfully or FALSE if no menu
	// with the name passed was detected.
	BOOL AddAccessMenuItem(CString& menuName,CString& itemName, CString& action);
	// This function will try to remove menu item with "itemName" name from the menu
	// "menuName". Returns TRUE if OK.
	BOOL RemoveAccessMenuItem(CString& menuName,CString& itemName);

	// This function will set the new menu to the main window. The difference between
	// this function and MDISetMenu function is that this function will add to the
	// mentioned menu all menu items added by Access commands grouped in the Access
	// sub-menu (lockated before "Windows" and "Help" sub-menus).
	void SetNewMenu(UINT nResourceId);

	UINT GetBaseMenuResource(void);

    void DockControlBarLeftOf(CToolBar* Bar,CToolBar* LeftOf);

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMainFrame)
	public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual void RecalcLayout(BOOL bNotify = TRUE);
	//}}AFX_VIRTUAL

// Implementation
public:
	void DestroyCMMenu();
	void CreateCMMenu();
	void SetDiscoverName(void);
	BOOL m_Updating;
	int m_SelectionSize;
	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	CDormantChildFrame*      m_wndDormant;
	CQAChildFrame*           m_wndQA;
	CAccessChildFrame*       m_wndAccess;
	CSimplifyChildFrame*     m_wndSimplify;
	CImpactChildFrame*		 m_wndImpact;

	CDockableBrowser       m_wndBrowser;
	CErrorsBrowser         m_wndErrors;
	CStatusBar             m_wndStatusBar;
	CAccessMenu            m_AccessMenu;
	BOOL m_Waiting;
	int                    m_Lic[20];
protected:  // control bar embedded members
	CToolBar          m_wndBaseToolbar;
	//CToolBar          m_wndViewsToolbar;

	// This array represents complete set of the pop-uo menus added to the server 
	// using add_menu access command. When setting
	CMenu         m_MainMenu;
	UINT          m_BaseMenuResource;

// Generated message map functions
protected:
    void RecursiveMenuAdd(CAccessMenu& from, CMenu& to);
	//{{AFX_MSG(CMainFrame)
	afx_msg LRESULT OnUpdateGroups(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnUpdateErrors(WPARAM wParam, LPARAM lParam);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnPaint();
	afx_msg void OnDormant();
	afx_msg void OnFlowchart();
	afx_msg void OnDatachart();
	afx_msg void OnErd();
	afx_msg void OnInheritance();
	afx_msg void OnOutline();
	afx_msg void OnTreediagram();
	afx_msg void OnUpdateDormant(CCmdUI* pCmdUI);
	afx_msg void OnUpdateErd(CCmdUI* pCmdUI);
	afx_msg void OnUpdateDatachart(CCmdUI* pCmdUI);
	afx_msg void OnUpdateFlowchart(CCmdUI* pCmdUI);
	afx_msg void OnUpdateInheritance(CCmdUI* pCmdUI);
	afx_msg void OnUpdateOutline(CCmdUI* pCmdUI);
	afx_msg void OnUpdateTreediagram(CCmdUI* pCmdUI);
	afx_msg void OnQa();
    afx_msg void OnServiceChanged();
	afx_msg LRESULT OnServiceDisconnect(WPARAM wparam, LPARAM lparam);
	afx_msg void OnServiceSelect();
	afx_msg void OnAccess();
	afx_msg void OnOpenDataChart();
	afx_msg void OnOpenERD();
	afx_msg void OnOpenFlowChart();
	afx_msg void OnOpenInheritance();
	afx_msg void OnOpenOutline();
	afx_msg void OnOpenTree();
	afx_msg void OnClose();
	afx_msg void OnUpdate();
	afx_msg void OnUpdateUpdate(CCmdUI* pCmdUI);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnSimplify();
	afx_msg void OnUpdateSimplify(CCmdUI* pCmdUI);
	afx_msg void OnImpact();
	afx_msg void OnUpdateImpact(CCmdUI* pCmdUI);
	afx_msg void OnViewBrowser();
	afx_msg void OnViewToolbar();
	afx_msg void OnUpdateViewBrowser(CCmdUI* pCmdUI);
	afx_msg void OnUpdateViewToolbar(CCmdUI* pCmdUI);
	afx_msg void OnUpdateQa(CCmdUI* pCmdUI);
	afx_msg void OnUpdateAccess(CCmdUI* pCmdUI);
	afx_msg void OnUpdateXmlreport(CCmdUI* pCmdUI);
	afx_msg void OnCallgraph();
	afx_msg void OnUpdateCallgraph(CCmdUI* pCmdUI);
	afx_msg void OnViewErrors();
	afx_msg void OnUpdateViewErrors(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditorSetup(CCmdUI* pCmdUI);
	afx_msg void OnEditorSetup();
	afx_msg void OnUpdateCMSetup(CCmdUI* pCmdUI);
	afx_msg void OnCMSetup();
	afx_msg void OnDestroy();
	//}}AFX_MSG
    afx_msg void OnUserAction(int id);
    afx_msg void OnCMAction(int id);
	DECLARE_MESSAGE_MAP()
private:
	BOOL m_ServerConnected;
	CUpdateWaitDlg m_UpdateDlg;
	HCURSOR m_SavedCursor;
	HANDLE m_UpdateThread;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MAINFRM_H__64FA0D60_261E_11D2_AEFB_00A0C9B71DC4__INCLUDED_)
