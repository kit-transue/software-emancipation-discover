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
#if !defined(AFX_IMPACTCHILDFRAME_H__BB31AE31_6917_4B65_8FAA_F8294366BAD2__INCLUDED_)
#define AFX_IMPACTCHILDFRAME_H__BB31AE31_6917_4B65_8FAA_F8294366BAD2__INCLUDED_

#include "ImpactForm.h"	// Added by ClassView
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ImpactChildFrame.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CImpactChildFrame frame

class CImpactChildFrame : public CMDIChildWnd
{
	DECLARE_DYNCREATE(CImpactChildFrame)
protected:
	CImpactChildFrame();           // protected constructor used by dynamic creation

// Attributes
public:

// Operations
public:
	void SetEditorCtrl(CCodeRoverIntegrator* pEditorCtrl);
	bool Update();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CImpactChildFrame)
	protected:
	virtual BOOL OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext);
	//}}AFX_VIRTUAL

// Implementation
protected:
	CToolBar m_ImpactTools;
	virtual ~CImpactChildFrame();

	// Generated message map functions
	//{{AFX_MSG(CImpactChildFrame)
	afx_msg void OnMDIActivate(BOOL bActivate, CWnd* pActivateWnd, CWnd* pDeactivateWnd);
	afx_msg void OnClose();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg LRESULT OnServiceDisconnect(WPARAM wparam, LPARAM lparam);
	afx_msg void OnServiceChanged(void);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnUpdateSaveReport(CCmdUI* pCmdUI);
	afx_msg void OnSaveReport();
	afx_msg void OnIMAttach();
	afx_msg void OnUpdateIMAttach(CCmdUI* pCmdUI);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	CImpactForm* m_pwndImpact;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_IMPACTCHILDFRAME_H__BB31AE31_6917_4B65_8FAA_F8294366BAD2__INCLUDED_)
