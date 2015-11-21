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
// ProjectTreeCtl.h : Declaration of the CProjectTreeCtrl OLE control class.
#include <afxtempl.h>
#include "TreeWindow.h"
#define IDC_PROJECTTREE       1000
#define EVAL_OK               1
#define ERROR_DATA_SOURCE    -1
#define ERROR_ACCESS_SUPPORT -2
#define ERROR_ACCESS_COMMAND -3
/////////////////////////////////////////////////////////////////////////////
// CProjectTreeCtrl : See ProjectTreeCtl.cpp for implementation.

class CProjectPath {
public:
	CString   Name;
	HTREEITEM Id;
};


class CProjectTreeCtrl : public COleControl
{
	DECLARE_DYNCREATE(CProjectTreeCtrl)

// Constructor
public:
	int EvaluateSync(CString& command,CString* results=NULL);
	CProjectTreeCtrl();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CProjectTreeCtrl)
	public:
	virtual void OnDraw(CDC* pdc, const CRect& rcBounds, const CRect& rcInvalid);
	virtual void DoPropExchange(CPropExchange* pPX);
	virtual void OnResetState();
	virtual BOOL OnDoVerb(LONG iVerb, LPMSG lpMsg, HWND hWndParent, LPCRECT lpRect);
	//}}AFX_VIRTUAL

// Implementation
protected:
	void FillChilds(CString varname,int index,HTREEITEM attach_to=TVI_ROOT);
	virtual void FillProjectTree();
	~CProjectTreeCtrl();

	DECLARE_OLECREATE_EX(CProjectTreeCtrl)    // Class factory and guid
	DECLARE_OLETYPELIB(CProjectTreeCtrl)      // GetTypeInfo
	DECLARE_PROPPAGEIDS(CProjectTreeCtrl)     // Property page IDs
	DECLARE_OLECTLTYPE(CProjectTreeCtrl)		// Type name and misc status

// Message maps
	//{{AFX_MSG(CProjectTreeCtrl)
	afx_msg int  OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnSelchangedProjectTree(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnDoubleclickProjectTree(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnExpandProjectTree(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// Dispatch maps
	//{{AFX_DISPATCH(CProjectTreeCtrl)
	afx_msg LPDISPATCH GetDataSource();
	afx_msg void SetDataSource(LPDISPATCH newValue);
	afx_msg BSTR GetSelectedNode();
	afx_msg void SetSelectedNode(LPCTSTR lpszNewValue);
	afx_msg void Query();
	afx_msg void Clean();
	//}}AFX_DISPATCH
	DECLARE_DISPATCH_MAP()

	afx_msg void AboutBox();

// Event maps
	//{{AFX_EVENT(CProjectTreeCtrl)
	void FireNodeChanged(LPCTSTR projectName)
		{FireEvent(eventidNodeChanged,EVENT_PARAM(VTS_BSTR), projectName);}
	void FireNodeDoubleclicked(LPCTSTR project)
		{FireEvent(eventidNodeDoubleclicked,EVENT_PARAM(VTS_BSTR), project);}
	void FireNodeExpanded(LPCTSTR project)
		{FireEvent(eventidNodeExpanded,EVENT_PARAM(VTS_BSTR), project);}
	void FireNodeCollapsed(LPCTSTR project)
		{FireEvent(eventidNodeCollapsed,EVENT_PARAM(VTS_BSTR), project);}
	//}}AFX_EVENT
	DECLARE_EVENT_MAP()

// Dispatch and event IDs
public:
	enum {
	//{{AFX_DISP_ID(CProjectTreeCtrl)
	dispidDataSource = 1L,
	dispidSelectedNode = 2L,
	dispidQuery = 3L,
	dispidClean = 4L,
	eventidNodeChanged = 1L,
	eventidNodeDoubleclicked = 2L,
	eventidNodeExpanded = 3L,
	eventidNodeCollapsed = 4L,
	//}}AFX_DISP_ID
	};
private:
	CString m_SelectedNode;
	CImageList                        m_ImageList;
	CTreeWindow                       m_Tree;
	CList<CProjectPath,CProjectPath&> m_FullNameList;
	LPDISPATCH                        m_DataSource;
};
