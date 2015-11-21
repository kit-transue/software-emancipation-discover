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
#if !defined(AFX_DOCKABLEBrowser_H__715A43B1_2677_11D2_AEFD_00A0C9B71DC4__INCLUDED_)
#define AFX_DOCKABLEBrowser_H__715A43B1_2677_11D2_AEFD_00A0C9B71DC4__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// DockableBrowser.h : header file
//

#include <afxtempl.h>

#include "sash.h"
#ifndef __PROJECTTREE__
   #define __PROJECTTREE__
   #include "projecttree.h"
#endif

#ifndef __CATEGORIESCOMBO__
   #define __CATEGORIESCOMBO__
   #include "categoriescombo.h"
#endif

#ifndef __QUERIESCOMBO__
   #define __QUERIESCOMBO__
   #include "queriescombo.h"
#endif

#ifndef __GROUPSCOMBO__
   #define __GROUPSCOMBO__
   #include "groupscombo.h"
#endif


#ifndef __DATASOURCE_H
   #define __DATASOURCE_H
   #include "datasource.h"
#endif


#ifndef __QUERYRESULTS_H
   #define __QUERYRESULTS_H
   #include "queryresults.h"
#endif

#include "CMCommand.h"

#define DOCK_LEFT   0
#define DOCK_RIGHT  1
#define DOCK_TOP    2
#define DOCK_BOTTOM 3
#define DOCK_FLOAT  4
/////////////////////////////////////////////////////////////////////////////
// CDockableBrowser window

//----------------------------------------------------------------------------------------
// This class designed to represent query results on the client. We will use it to
// support history and history buttons.
//----------------------------------------------------------------------------------------
class CHistoryEntry {
public:
	int    m_Mode;		    // Current browser mode;


	CString m_ServerVariable;   // Contains the server variable name in wich results
				    // are stored.
	CString m_ProjectName;	    // The project value for this results;
	CString m_CategoryName;     // The category value for this results
	CString m_QueryName;	    // The query name for this results
	CString m_GroupName;
	CString m_Command;

	CHistoryEntry& operator = (CHistoryEntry& other) {
		m_ServerVariable=other.m_ServerVariable;
		m_ProjectName=other.m_ProjectName;
		m_CategoryName=other.m_CategoryName;
		m_QueryName=other.m_QueryName;
		m_Command = other.m_Command;
		return *this;
	};
};
//----------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------
// This class designed to contain links between queries and command ID's. The browser
// contains the query map which will help to determine which query must be called when
// query popup menu item selected (this menu is build dynamically)
//----------------------------------------------------------------------------------------
class CMenuMapEntry {
public:
	int	m_ID;		   // command ID;
	CString m_Query;	   // Query string
	CMenuMapEntry& operator = (CMenuMapEntry& other) {
		m_ID=other.m_ID;
		m_Query=other.m_Query;
		return *this;
	};
};



//----------------------------------------------------------------------------------------


class CDockableBrowser : public CDialogBar
{
// Construction
public:
	CDockableBrowser();

// Attributes
public:
	BOOL m_OnSize;

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDockableBrowser)
	//}}AFX_VIRTUAL

// Implementation
public:
	CString GetEditor();
	void SetEditor(CString& szName);
	CString GetCM();
	BOOL SetCM(CString& szName);
	void ViewFile(CString& params);
	void ViewTextFile(CString& filename);
	void PdfChanged();
	void BrowseGivenSet(CString& name, int col=0);
	CString ExecuteCommand(CString& command);
	BOOL SelectionHasKind(CString kind);
	void PositionSashBars(void);
	void CalculateFixedSize(void);
	void CalculateVariableSize(void);
	void LayoutControls(void);
	void UpdateSashes(BOOL initialize=FALSE);
	virtual ~CDockableBrowser();
	void OnEditorSetup();
	void CreateCMMenu(CMenu** pMenu);
	afx_msg void OnCMAction(int id);
	afx_msg void OnCMSetup();
	afx_msg void OnUpdateCMSetup(CCmdUI* pCmdUI);

	// Generated message map functions
protected:
	CSash m_BottomSash;
	CSash m_TopSash;
	CSash m_LeftSash;
	CSash m_RightSash;
	CSash m_ProjectDividerSash;
	CSash m_QueryDividerSash;


	int GetDockSide(void);
	//{{AFX_MSG(CDockableBrowser)
	afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnQueryModeChanged(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg LRESULT OnSashMoved(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnProjectSplitMoved(WPARAM wparam, LPARAM lparam);
    afx_msg LRESULT OnRefreshFileList(WPARAM wparam, LPARAM lparam);
	afx_msg void OnPaint();
    afx_msg void OnNodeChanged(LPCTSTR projectName);
    afx_msg void OnCMEvent(LPCTSTR lfile);
    afx_msg void IntegratorCtrlClick(LPCTSTR lfile, long line,long col, long size);
    afx_msg void IntegratorQuery(LPCTSTR lfile, long line,long col, long size);
    afx_msg void IntegratorDefinition(LPCTSTR lfile, long line,long col, long size);
    afx_msg void IntegratorActivate(void);
	afx_msg LRESULT OnServiceDisconnect(WPARAM wparam, LPARAM lparam);
    afx_msg void OnServiceChanged(void);
    afx_msg void OnServerNotification(LPCTSTR str);
	afx_msg void OnQueriesEnable(void);
    afx_msg void OnUpdateQueriesBtn(CCmdUI* pCmdUI);
    afx_msg void OnCategoryChanged(LPCTSTR categoriesList);
    afx_msg void OnQueryChanged(LPCTSTR queriesList);
    afx_msg void OnSelectionChanged(LPCTSTR pstrParams);
	afx_msg void OnElementDoubleclicked(LPCTSTR parmsList);
    afx_msg void OnRButtonEvent(short x,short y);
	afx_msg void PrevHistory(void);
	afx_msg void NextHistory(void);
    afx_msg void OnUpdatePrev(CCmdUI* pCmdUI);
    afx_msg void OnUpdateNext(CCmdUI* pCmdUI);
	afx_msg void OnUpdateFilter(CCmdUI* pCmdUI);
	afx_msg void OnSelectSort(void);
	afx_msg void OnSetAttributes(void);
	afx_msg void OnSetFilter(void);
	afx_msg void OnResetFilter(void);
    afx_msg void OnAddElements(void);
    afx_msg void OnSaveElements(void);
    afx_msg void OnGroupChanged(LPCSTR catList);
    afx_msg void OnGroupCreated(LPCSTR name);
    afx_msg void OnGroupRemoved(short amount);
    afx_msg void OnGroupInsSel(void);
    afx_msg void OnGroupDelSel(void);
	afx_msg void OnUpdateEditorSetup(CCmdUI* pCmdUI);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	afx_msg void OnClosure(void);
    afx_msg void OnUpdateClosure(CCmdUI* pCmdUI);
	afx_msg void OnShortNames(void);
    afx_msg void OnUpdateShortNames(CCmdUI* pCmdUI);
	afx_msg void OnMenuQuery(int id);
    afx_msg void SetBrowserVariable(void);
    LRESULT OnQuerySplitMoved(WPARAM wparam, LPARAM lparam);
    void OnFileDoubleclicked(LPCTSTR element);

	DECLARE_MESSAGE_MAP()
private:
	// CM Integration stuff
	CMenu* m_pCMMenu;
	CCMCommandsList* m_pCMCommands;
	CCMAttributes* m_pCMAttributes;
	
	BOOL m_Closure;
	CImageList m_ModeImages;
	CBitmap m_NextBmp;
	CBitmap m_PrevBmp;
	CBitmap m_SplitBmp;
	BOOL m_ResultWasEmpty;
	CString m_CurrentGroup;
	CString m_CurrentQuery;
	CString m_CurrentCategory;
	int m_CurrentMode;

	// History related members
	int				    m_InHistory;
	POSITION			    m_CurrentHistoryEntry;
	CList<CHistoryEntry,CHistoryEntry&> m_History;
	void SaveHistory(CString command = "");
    void ClearHistory(void);
	int m_VarIndex;

	// Menu map 
	CList<CMenuMapEntry,CMenuMapEntry&> m_MenuMap;
	void MakeMenuMap();

	// Set information string as a filter button title
    void DisplayInfoString(void);

	// Kind list can be used to determine the kinds in the current selection
	CList<CString,CString&> m_KindList;
	void FillKindList(void);

	CString m_QueryRequest;
    void FormCategoryRequest(CString& categoriesList);
    void FormQueryRequest(CString& queriesList);
	CString m_CategoryRequest;
	CString m_CurrentProject;
	void SetSplitteredWindowsPos(void);
	int m_ProjectSplitterX;
	int m_QuerySplitterX;
	int m_DockSide;
	BOOL m_QueriesEnabled;
	BOOL m_Updated;

	// For executing CM commands
	void DoCMAction(CString &cmdKey, CStringList &filesList);
	void DoCMAction(CString &cmdKey, int argNum, CString *argVal);

	// Data access stuff
	CString DataSourceAccessSync(CDataSource* DataSource, BSTR* command);
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DOCKABLEBrowser_H__715A43B1_2677_11D2_AEFD_00A0C9B71DC4__INCLUDED_)
