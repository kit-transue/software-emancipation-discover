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
#if !defined(AFX_CMINTEGRATORCTL_H__F9A12339_9255_4E79_AFCA_BAC1D28AD5A0__INCLUDED_)
#define AFX_CMINTEGRATORCTL_H__F9A12339_9255_4E79_AFCA_BAC1D28AD5A0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <atlbase.h>

#include <IntegratorCtrl.h>

#define LOOP_COUNT 5

// CMIntegratorCtl.h : Declaration of the CCMIntegratorCtrl ActiveX Control class.

/////////////////////////////////////////////////////////////////////////////
// CCMIntegratorCtrl : See CMIntegratorCtl.cpp for implementation.

class CCMIntegratorCtrl : public COleControl, public CIntegratorCtrl
{
	DECLARE_DYNCREATE(CCMIntegratorCtrl)

// Constructor
public:
	CCMIntegratorCtrl();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCMIntegratorCtrl)
	public:
	virtual void OnDraw(CDC* pdc, const CRect& rcBounds, const CRect& rcInvalid);
	virtual void DoPropExchange(CPropExchange* pPX);
	virtual void OnResetState();
	//}}AFX_VIRTUAL

	virtual void OnIntegratorEvent(void);
// Implementation
protected:
	~CCMIntegratorCtrl();
	virtual CString GetIntegratorType();
	virtual CString GetIntegratorName();
	virtual bool RunDriver();

	CString m_szCMName;
	CString m_szIntegratorName;
	bool m_bConnected;

	DECLARE_OLECREATE_EX(CCMIntegratorCtrl)    // Class factory and guid
	DECLARE_OLETYPELIB(CCMIntegratorCtrl)	   // GetTypeInfo
	DECLARE_PROPPAGEIDS(CCMIntegratorCtrl)	   // Property page IDs
	DECLARE_OLECTLTYPE(CCMIntegratorCtrl)		// Type name and misc status

// Message maps
	//{{AFX_MSG(CCMIntegratorCtrl)
		// NOTE - ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// Dispatch maps
	//{{AFX_DISPATCH(CCMIntegratorCtrl)
	afx_msg BSTR GetCMName();
	afx_msg void SetCMName(LPCTSTR lpszNewValue);
	afx_msg BSTR GetCMs();
	afx_msg BSTR GetCommands();
	afx_msg BOOL Connect(LPCTSTR szName);
	afx_msg BOOL IsConnected();
	afx_msg BOOL IsAvailable();
	afx_msg BSTR GetAttributes();
	afx_msg BSTR ExecCommand(LPCTSTR szCommand);
	afx_msg BOOL Reconnect();
	//}}AFX_DISPATCH
	DECLARE_DISPATCH_MAP()

// Event maps
	//{{AFX_EVENT(CCMIntegratorCtrl)
	//}}AFX_EVENT
	DECLARE_EVENT_MAP()

// Dispatch and event IDs
public:
	enum {
	//{{AFX_DISP_ID(CCMIntegratorCtrl)
	dispidCMName = 1L,
	dispidGetCMs = 2L,
	dispidGetCommands = 3L,
	dispidConnect = 4L,
	dispidIsConnected = 5L,
	dispidIsAvailable = 6L,
	dispidGetAttributes = 7L,
	dispidExecCommand = 8L,
	dispidReconnect = 9L,
	//}}AFX_DISP_ID
	};
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CMINTEGRATORCTL_H__F9A12339_9255_4E79_AFCA_BAC1D28AD5A0__INCLUDED)
