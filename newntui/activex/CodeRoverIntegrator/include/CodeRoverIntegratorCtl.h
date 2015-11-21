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
#if !defined(AFX_CODEROVERINTEGRATORCTL_H__8DBAE795_DB45_11D4_8E41_000102390321__INCLUDED_)
#define AFX_CODEROVERINTEGRATORCTL_H__8DBAE795_DB45_11D4_8E41_000102390321__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "IntegratorCtrl.h"

// CodeRoverIntegratorCtl.h : Declaration of the CCodeRoverIntegratorCtrl ActiveX Control class.

/////////////////////////////////////////////////////////////////////////////
// CCodeRoverIntegratorCtrl : See CodeRoverIntegratorCtl.cpp for implementation.

class StringPair {
public:
	StringPair(CString key, CString value) : szKey(key),szValue(value){};
	CString szKey;
	CString szValue;
};

class CCodeRoverIntegratorCtrl : public COleControl, public CIntegratorCtrl
{
	DECLARE_DYNCREATE(CCodeRoverIntegratorCtrl)

// Constructor
public:
	CCodeRoverIntegratorCtrl();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCodeRoverIntegratorCtrl)
	public:
	virtual void OnDraw(CDC* pdc, const CRect& rcBounds, const CRect& rcInvalid);
	virtual void DoPropExchange(CPropExchange* pPX);
	virtual void OnResetState();
	//}}AFX_VIRTUAL

// Implementation
protected:
	CString m_EditorName;
	CMapStringToPtr m_EditorsList;
	~CCodeRoverIntegratorCtrl();
	virtual CString GetIntegratorName();
	virtual CString GetIntegratorType();
	virtual bool RunDriver();
	virtual void AllIntegratedEditorsList();
	virtual void AddUserIntegratedEditors();

	DECLARE_OLECREATE_EX(CCodeRoverIntegratorCtrl)    // Class factory and guid
	DECLARE_OLETYPELIB(CCodeRoverIntegratorCtrl)      // GetTypeInfo
	DECLARE_PROPPAGEIDS(CCodeRoverIntegratorCtrl)     // Property page IDs
	DECLARE_OLECTLTYPE(CCodeRoverIntegratorCtrl)		// Type name and misc status

// Message maps
	//{{AFX_MSG(CCodeRoverIntegratorCtrl)
		// NOTE - ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// Dispatch maps
	//{{AFX_DISPATCH(CCodeRoverIntegratorCtrl)
	afx_msg BSTR GetEditorName();
	afx_msg void SetEditorName(LPCTSTR lpszNewValue);
	afx_msg BSTR GetEditorsList();
	afx_msg void SetEditorsList(LPCTSTR lpszNewValue);
	afx_msg void MakeSelection(LPCTSTR filename, long line, long column, long len);
	afx_msg void OpenFile(LPCTSTR filename, long line);
	afx_msg void SetInfo(LPCTSTR text);
	afx_msg BOOL Reconnect();
	afx_msg long GetModelLine(LPCTSTR pszLocalName, LPCTSTR pszBaseName, long nLine);
	//}}AFX_DISPATCH
	DECLARE_DISPATCH_MAP()

// Event maps
	//{{AFX_EVENT(CCodeRoverIntegratorCtrl)
	void FireCtrlClick(LPCTSTR filename, long line, long column, long token)
		{FireEvent(eventidCtrlClick,EVENT_PARAM(VTS_BSTR  VTS_I4  VTS_I4  VTS_I4), filename, line, column, token);}
	void FireQuery(LPCTSTR filename, long line, long column, long len)
		{FireEvent(eventidQuery,EVENT_PARAM(VTS_BSTR  VTS_I4  VTS_I4  VTS_I4), filename, line, column, len);}
	void FireDefinition(LPCTSTR filename, long line, long column, long len)
		{FireEvent(eventidDefinition,EVENT_PARAM(VTS_BSTR  VTS_I4  VTS_I4  VTS_I4), filename, line, column, len);}
	void FireActivate()
		{FireEvent(eventidActivate,EVENT_PARAM(VTS_NONE));}
	//}}AFX_EVENT
	DECLARE_EVENT_MAP()

// Dispatch and event IDs
public:
	virtual void OnIntegratorEvent(void);
	enum {
	//{{AFX_DISP_ID(CCodeRoverIntegratorCtrl)
	dispidEditorName = 1L,
	dispidEditorsList = 2L,
	dispidMakeSelection = 3L,
	dispidOpenFile = 4L,
	dispidSetInfo = 5L,
	dispidReconnect = 6L,
	dispidGetModelLine = 7L,
	eventidCtrlClick = 1L,
	eventidQuery = 2L,
	eventidDefinition = 3L,
	eventidActivate = 4L,
	//}}AFX_DISP_ID
	};
private:
	CString GetReply(void);
	bool SendCommand(CString& command);
	CString SendRequestGetReply(CString request);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CODEROVERINTEGRATORCTL_H__8DBAE795_DB45_11D4_8E41_000102390321__INCLUDED)
