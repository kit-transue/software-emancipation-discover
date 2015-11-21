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
#if !defined(AFX_GRAPHLAYOUTPROPPAGE_H__A3EC5B92_C1F1_11D2_AF88_00A0C9B71DC4__INCLUDED_)
#define AFX_GRAPHLAYOUTPROPPAGE_H__A3EC5B92_C1F1_11D2_AF88_00A0C9B71DC4__INCLUDED_

#include "GraphView.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// GraphLayoutPropPage.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CGraphLayoutPropPage dialog

class CGraphLayoutPropPage : public CPropertyPage
{
	DECLARE_DYNCREATE(CGraphLayoutPropPage)

// Construction
public:
	void SetGraphSettings(CGraphSettings* settings);
	int m_Justification;
	int m_Orientation;
	CGraphLayoutPropPage();
	~CGraphLayoutPropPage();

// Dialog Data
	//{{AFX_DATA(CGraphLayoutPropPage)
	enum { IDD = IDD_GRAPH_LAYOUT };
	CButton	m_VariableSpace;
	CStatic	m_SpaceNodeName;
	CEdit	m_SpaceNode;
	CStatic	m_SpaceLevelName;
	CEdit	m_SpaceLevel;
	CButton	m_Orthogonal;
	CButton	m_MergeRows;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CGraphLayoutPropPage)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CGraphLayoutPropPage)
	afx_msg void OnRightToLeft();
	afx_msg void OnLeftToRight();
	afx_msg void OnTopToBottom();
	afx_msg void OnBottomToTop();
	afx_msg void OnJustifyCenter();
	afx_msg void OnJustifyEnd();
	afx_msg void OnJustifyStart();
	afx_msg void OnOrthogonal();
	afx_msg void OnMerge();
	afx_msg void OnVarSpace();
	virtual BOOL OnInitDialog();
	afx_msg void OnChangeSpacenode();
	afx_msg void OnChangeSpacelevel();
	afx_msg void OnChangeSpacerow();
	afx_msg void OnChangeSpacecolumn();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	CGraphSettings* m_Settings;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GRAPHLAYOUTPROPPAGE_H__A3EC5B92_C1F1_11D2_AF88_00A0C9B71DC4__INCLUDED_)
