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
#if !defined(AFX_IMPACTPAGESELECTOR_H__4544B7DD_5880_4C4F_9607_93DE950EBF2A__INCLUDED_)
#define AFX_IMPACTPAGESELECTOR_H__4544B7DD_5880_4C4F_9607_93DE950EBF2A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ImpactPageSelector.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CImpactPageSelector window

class CImpactPageSelector : public CTabCtrl
{
// Construction
public:
	CImpactPageSelector();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CImpactPageSelector)
	//}}AFX_VIRTUAL

// Implementation
public:
	void AddPage(CWnd *pWnd,CString& szTitle,int nImage=-1);
	virtual ~CImpactPageSelector();

	// Generated message map functions
protected:
	//{{AFX_MSG(CImpactPageSelector)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnSelchange(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	//}}AFX_MSG
	afx_msg LRESULT OnItemStateChanged(WPARAM wparm, LPARAM lparm);
	afx_msg LRESULT OnOpenInstance(WPARAM wparm, LPARAM lparm);
	DECLARE_MESSAGE_MAP()
private:
	CFont m_ItemFont;
	CList<CWnd*,CWnd*> m_Pages;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_IMPACTPAGESELECTOR_H__4544B7DD_5880_4C4F_9607_93DE950EBF2A__INCLUDED_)
