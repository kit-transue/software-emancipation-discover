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
// ProjectModulesCtl.h : Declaration of the CProjectModulesCtrl OLE control class.

/////////////////////////////////////////////////////////////////////////////
// CProjectModulesCtrl : See ProjectModulesCtl.cpp for implementation.
#include <afxtempl.h>
#include <afxcmn.h>

#define IDC_PROJECTMODULES 1029

#define EVAL_OK               1
#define ERROR_DATA_SOURCE    -1
#define ERROR_ACCESS_SUPPORT -2
#define ERROR_ACCESS_COMMAND -3

#define TYPE_PROJECT   0
#define TYPE_C         1
#define TYPE_CPLUSPLUS 2
#define TYPE_CHEADER   3
#define TYPE_WORDDOC   4
#define TYPE_UNKNOWN   5

class CListEntry {
public :
	CString Name;
	unsigned int Type;
};

class CProjectModulesCtrl : public COleControl
{
	DECLARE_DYNCREATE(CProjectModulesCtrl)

// Constructor
public:
	CProjectModulesCtrl();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CProjectModulesCtrl)
	public:
	virtual void OnDraw(CDC* pdc, const CRect& rcBounds, const CRect& rcInvalid);
	virtual void DoPropExchange(CPropExchange* pPX);
	virtual void OnResetState();
	protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

// Implementation
protected:
	void ClearList(void);
	void FillFileTable(const char* projname);
	int  EvaluateSync(CString& command, CString* results=NULL);
	~CProjectModulesCtrl();

	DECLARE_OLECREATE_EX(CProjectModulesCtrl)    // Class factory and guid
	DECLARE_OLETYPELIB(CProjectModulesCtrl)      // GetTypeInfo
	DECLARE_PROPPAGEIDS(CProjectModulesCtrl)     // Property page IDs
	DECLARE_OLECTLTYPE(CProjectModulesCtrl)		// Type name and misc status

// Message maps
	//{{AFX_MSG(CProjectModulesCtrl)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSelectElement(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnExpandProject(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// Dispatch maps
	//{{AFX_DISPATCH(CProjectModulesCtrl)
	afx_msg LPDISPATCH GetDataSource();
	afx_msg void SetDataSource(LPDISPATCH newValue);
	afx_msg BSTR GetProject();
	afx_msg void SetProject(LPCTSTR lpszNewValue);
	//}}AFX_DISPATCH
	DECLARE_DISPATCH_MAP()

	afx_msg void AboutBox();

// Event maps
	//{{AFX_EVENT(CProjectModulesCtrl)
	void FireProjectChanged(LPCTSTR project)
		{FireEvent(eventidProjectChanged,EVENT_PARAM(VTS_BSTR), project);}
	void FireProjectDoubleclicked(LPCTSTR project)
		{FireEvent(eventidProjectDoubleclicked,EVENT_PARAM(VTS_BSTR), project);}
	void FireElementChanged(LPCTSTR element)
		{FireEvent(eventidElementchanged,EVENT_PARAM(VTS_BSTR), element);}
	void FireElementDoubleclicked(LPCTSTR element)
		{FireEvent(eventidElementDoubleclicked,EVENT_PARAM(VTS_BSTR), element);}
	//}}AFX_EVENT
	DECLARE_EVENT_MAP()

// Dispatch and event IDs
public:
	enum {
	//{{AFX_DISP_ID(CProjectModulesCtrl)
	dispidDataSource = 1L,
	dispidProject = 2L,
	eventidProjectChanged = 1L,
	eventidProjectDoubleclicked = 2L,
	eventidElementchanged = 3L,
	eventidElementDoubleclicked = 4L,
	//}}AFX_DISP_ID
	};
private:
	CString m_ProjectName;
	CImageList m_ImageList;    
	LPDISPATCH m_DataSource;
	CListCtrl  m_FileTable;
	CListEntry m_Element;
	CArray<CListEntry,CListEntry&> m_FullNameList; // Contains full logical paths to the projects
	                                         // and modules
};
