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
#if !defined(AFX_ERRORSBROWSER_H__BD7BA724_FCD1_11D2_AFB8_00A0C9B71DC4__INCLUDED_)
#define AFX_ERRORSBROWSER_H__BD7BA724_FCD1_11D2_AFB8_00A0C9B71DC4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ErrorsBrowser.h : header file
//
#include "sash.h"
#define DOCK_LEFT   0
#define DOCK_RIGHT  1
#define DOCK_TOP    2
#define DOCK_BOTTOM 3
#define DOCK_FLOAT  4

/////////////////////////////////////////////////////////////////////////////
// CErrorsBrowser dialog

class CErrorsBrowser : public CDialogBar
{
// Construction
public:
	BOOL    m_SpyRunning;
    BOOL    m_OnSize;

	void StopFileLog(void);
	void StartFileLog(const CString& filename);
	void SetErrorLog(const CString& log);
	CErrorsBrowser();   // standard constructor
	void CalculateFixedSize(void);
	void CalculateVariableSize(void);
	void LayoutControls(void);
    void UpdateFile(void);
	void UpdateSashes(BOOL initialize=FALSE);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CErrorsBrowser)
	//}}AFX_VIRTUAL

// Implementation
protected:
	CSash m_BottomSash;
	CSash m_TopSash;
	CSash m_LeftSash;
	CSash m_RightSash;

	int m_DockSide;
	// Generated message map functions
	//{{AFX_MSG(CErrorsBrowser)
		// NOTE: the ClassWizard will add member functions here
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnPaint();
	afx_msg LRESULT OnSashMoved(WPARAM wParam, LPARAM lParam);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	int m_OldFileLength;
    int  GetDockSide(void);
	void SetControlsPos(void);
	void PositionSashBars(void);
	HANDLE m_SpyThread;

	CString m_LogFile;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ERRORSBROWSER_H__BD7BA724_FCD1_11D2_AFB8_00A0C9B71DC4__INCLUDED_)
