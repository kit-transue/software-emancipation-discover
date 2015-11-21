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
#if !defined(AFX_EDGEPROPPAGE_H__406F8FCF_C2A4_11D2_AF89_00A0C9B71DC4__INCLUDED_)
#define AFX_EDGEPROPPAGE_H__406F8FCF_C2A4_11D2_AF89_00A0C9B71DC4__INCLUDED_
#include "GraphView.h"
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EdgePropPage.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CEdgePropPage dialog

class CEdgePropPage : public CPropertyPage
{
	DECLARE_DYNCREATE(CEdgePropPage)

// Construction
public:
	void SetSettings(CGraphSettings* settings);
	CEdgePropPage();
	~CEdgePropPage();

// Dialog Data
	//{{AFX_DATA(CEdgePropPage)
	enum { IDD = IDD_GRAPH_EDGE };
	CStatic	m_Color;
	CComboBox	m_Thickness;
	CSliderCtrl	m_Red;
	CSliderCtrl	m_Green;
	CSliderCtrl	m_Blue;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CEdgePropPage)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CEdgePropPage)
	afx_msg void OnEdgeBlue(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnEdgeGreen(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnEdgeRed(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnEdgeSelected();
	afx_msg void OnEdgeUnselected();
	afx_msg void OnEdgeFocused();
	afx_msg void OnSelchangeThickness();
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	void UpdateColorRect(void);
	void SetEdgeSizeControl(void);
	void SetSlider(void);
	int m_CurType;
	CGraphSettings* m_Settings;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EDGEPROPPAGE_H__406F8FCF_C2A4_11D2_AF89_00A0C9B71DC4__INCLUDED_)
