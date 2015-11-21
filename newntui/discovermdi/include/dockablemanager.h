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
#if !defined(AFX_DOCKABLEMANAGER_H__715A43B1_2677_11D2_AEFD_00A0C9B71DC4__INCLUDED_)
#define AFX_DOCKABLEMANAGER_H__715A43B1_2677_11D2_AEFD_00A0C9B71DC4__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// DockableManager.h : header file
//
#include "sash.h"
#ifndef __PROJECTTREE__
   #define __PROJECTTREE__
   #include "projecttree.h"
#endif

#ifndef __PROJECTMODULES__
   #define __PROJECTMODULES__
   #include "projectmodules.h"
#endif

#ifndef __DATASOURCE_H
   #define __DATASOURCE_H
   #include "datasource.h"
#endif


#ifndef __BROWSERFRAME_H
   #define __BROWSERFRAME_H
   #include "cbrowserframe.h"
#endif
#define DOCK_LEFT   0
#define DOCK_RIGHT  1
#define DOCK_TOP    2
#define DOCK_BOTTOM 3
#define DOCK_FLOAT  4
/////////////////////////////////////////////////////////////////////////////
// CDockableManager window

class CDockableManager : public CDialogBar
{
// Construction
public:
	CDockableManager();

// Attributes
public:
	BOOL m_OnSize;

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDockableManager)
	//}}AFX_VIRTUAL

// Implementation
public:
	void BrowseGivenSet(CString& name, int col=0);
	CString ExecuteCommand(CString& command);
	void PositionSashBars(void);
	void CalculateFixedSize(void);
	void CalculateVariableSize(void);
	void LayoutControls(void);
	void UpdateSashes(BOOL initialize=FALSE);
	virtual ~CDockableManager();

	// Generated message map functions
protected:
	CSash m_BottomSash;
	CSash m_TopSash;
	CSash m_LeftSash;
	CSash m_RightSash;


	int GetDockSide(void);
	//{{AFX_MSG(CDockableManager)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg LRESULT OnSashMoved(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnSplitMoved(WPARAM wparam, LPARAM lparam);
	afx_msg void OnPaint();
    afx_msg void OnModeChanged(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnNodeChanged(LPCTSTR projectName);
    afx_msg void OnProjectChanged(LPCTSTR projectName);
    afx_msg void OnFileDoubleclicked(LPCTSTR fileName);
    afx_msg void OnRightSelectionChanged(LPCSTR info);
    afx_msg void OnRightGroupChanged(LPCTSTR gList);
    afx_msg void OnLeftSelectionChanged(LPCTSTR info);
    afx_msg void OnLeftGroupChanged(LPCTSTR gList);
    afx_msg void OnRightCategoryChanged(LPCTSTR list);
    afx_msg void OnLeftCategoryChanged(LPCTSTR list);
    afx_msg void OnElementdoubleclicked(LPCTSTR list);
    afx_msg void IntegratorCtrlClick(LPCTSTR lfile, long line,long col, long size);
    afx_msg void IntegratorQuery(LPCTSTR lfile, long line,long col, long size);
    afx_msg void IntegratorDefinition(LPCTSTR lfile, long line,long col, long size);
    afx_msg void IntegratorActivate(void);
    afx_msg void OnServiceChanged(void);
    afx_msg void OnServerNotification(LPCTSTR str);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
    afx_msg void SetBrowserVariable(void);

	DECLARE_MESSAGE_MAP()
private:
	CString m_CurrentProject;
	void SetSplitteredWindowsPos(void);
	int m_SplitterX;
	CSash m_DividerSash;
	CCBrowserFrame* m_RightPanel;
	CCBrowserFrame* m_LeftPanel;
	CDataSource* m_DataSource;
	CTabCtrl*        m_Selector;
	CProjectModules* m_Modules;
	CProjectTree*    m_Tree;
	int m_DockSide;

	// Data access stuff
	CString DataSourceAccessSync(CDataSource* DataSource, BSTR* command);

};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DOCKABLEMANAGER_H__715A43B1_2677_11D2_AEFD_00A0C9B71DC4__INCLUDED_)
