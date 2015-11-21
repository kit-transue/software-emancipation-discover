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
#if !defined(AFX_MSDEVINTEGRATORCTL_H__C5267922_392E_11D2_AF11_00A0C9B71DC4__INCLUDED_)
#define AFX_MSDEVINTEGRATORCTL_H__C5267922_392E_11D2_AF11_00A0C9B71DC4__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
#include <afxsock.h>
// MSDEVIntegratorCtl.h : Declaration of the CMSDEVIntegratorCtrl ActiveX Control class.
#include "./include/IDE_editor.h"

/////////////////////////////////////////////////////////////////////////////
// CMSDEVIntegratorCtrl : See MSDEVIntegratorCtl.cpp for implementation.
class CMSDEVIntegratorCtrl;
class CIntegratorSocket:public CSocket {
public:
	void SetParent(CMSDEVIntegratorCtrl* p);
	virtual void OnReceive( int nErrorCode );
protected :
    CMSDEVIntegratorCtrl* parent;
};


class CMSDEVIntegratorCtrl : public COleControl
{
	DECLARE_DYNCREATE(CMSDEVIntegratorCtrl)

// Constructor
public:
	CMSDEVIntegratorCtrl();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMSDEVIntegratorCtrl)
	public:
	virtual void OnDraw(CDC* pdc, const CRect& rcBounds, const CRect& rcInvalid);
	virtual void DoPropExchange(CPropExchange* pPX);
	virtual void OnResetState();
	//}}AFX_VIRTUAL

// Implementation
protected:
	~CMSDEVIntegratorCtrl();

	DECLARE_OLECREATE_EX(CMSDEVIntegratorCtrl)    // Class factory and guid
	DECLARE_OLETYPELIB(CMSDEVIntegratorCtrl)      // GetTypeInfo
	DECLARE_PROPPAGEIDS(CMSDEVIntegratorCtrl)     // Property page IDs
	DECLARE_OLECTLTYPE(CMSDEVIntegratorCtrl)		// Type name and misc status

// Message maps
	//{{AFX_MSG(CMSDEVIntegratorCtrl)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnClose();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// Dispatch maps
	//{{AFX_DISPATCH(CMSDEVIntegratorCtrl)
	afx_msg void OpenFile(LPCTSTR filename, long line);
	afx_msg void MakeSelection(LPCTSTR filename, long line, long column, LPCTSTR token);
	afx_msg short GetTabSize();
	afx_msg void SetInfo(LPCTSTR info);
	afx_msg void GetSelection(BSTR FAR* filename, BSTR FAR* token, long FAR* line, long FAR* column);
	afx_msg void Exit();
	afx_msg void Command(LPCTSTR string);
	//}}AFX_DISPATCH
	DECLARE_DISPATCH_MAP()

	afx_msg void AboutBox();

// Event maps
	//{{AFX_EVENT(CMSDEVIntegratorCtrl)
	void FireCtrlClick(LPCTSTR filename, long line, long column, long token)
		{FireEvent(eventidCtrlClick,EVENT_PARAM(VTS_BSTR  VTS_I4  VTS_I4  VTS_I4), filename, line, column, token);}
	void FireQuery(LPCTSTR fname, long line, long col, long len)
		{FireEvent(eventidQuery,EVENT_PARAM(VTS_BSTR  VTS_I4  VTS_I4  VTS_I4), fname, line, col, len);}
	void FireDefinition(LPCTSTR filename, long line, long col, long len)
		{FireEvent(eventidDefinition,EVENT_PARAM(VTS_BSTR  VTS_I4  VTS_I4  VTS_I4), filename, line, col, len);}
	void FireActivate()
		{FireEvent(eventidActivate,EVENT_PARAM(VTS_NONE));}
	//}}AFX_EVENT
	DECLARE_EVENT_MAP()



// Dispatch and event IDs
public:
	virtual void CtrlClickProcessor(void);
	virtual void ActivateProcessor(void);
	virtual void QueryProcessor(void);
	virtual void DefinitionProcessor(void);
	int  ReceveIntegrationService(void);
	enum {
	//{{AFX_DISP_ID(CMSDEVIntegratorCtrl)
	dispidOpenFile = 1L,
	dispidMakeSelection = 2L,
	dispidGetTabSize = 3L,
	dispidSetInfo = 4L,
	dispidGetSelection = 5L,
	dispidExit = 6L,
	dispidCommand = 7L,
	eventidCtrlClick = 1L,
	eventidQuery = 2L,
	eventidDefinition = 3L,
	eventidActivate = 4L,
	//}}AFX_DISP_ID
	};
private:
	SOCKADDR_IN m_WhereIntegrator;
	CIntegratorSocket m_IntegratorServerConnection;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MSDEVINTEGRATORCTL_H__C5267922_392E_11D2_AF11_00A0C9B71DC4__INCLUDED)
