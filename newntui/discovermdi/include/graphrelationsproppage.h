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
#if !defined(AFX_GRAPHRELATIONSPROPPAGE_H__C62C4655_CC3F_11D2_AF8F_00A0C9B71DC4__INCLUDED_)
#define AFX_GRAPHRELATIONSPROPPAGE_H__C62C4655_CC3F_11D2_AF8F_00A0C9B71DC4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// GraphRelationsPropPage.h : header file
//
#include "GraphView.h"
/////////////////////////////////////////////////////////////////////////////
// CGraphRelationsPropPage dialog

class CGraphRelationsPropPage : public CPropertyPage
{
	DECLARE_DYNCREATE(CGraphRelationsPropPage)

// Construction
public:
	void SetGraphSettings(CGraphSettings* settings);
	CGraphRelationsPropPage();
	~CGraphRelationsPropPage();

// Dialog Data
	//{{AFX_DATA(CGraphRelationsPropPage)
	enum { IDD = IDD_GRAPH_ERD };
	CButton	m_Visible;
	CComboBox	m_ThicknessSelector;
	CComboBox	m_StyleSelector;
	CComboBox	m_RelationSelector;
	CSliderCtrl	m_Red;
	CSliderCtrl	m_Green;
	CStatic	m_Color;
	CSliderCtrl	m_Blue;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CGraphRelationsPropPage)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CGraphRelationsPropPage)
	afx_msg void OnRelationChanged();
	afx_msg void OnThicknessChanged();
	afx_msg void OnVisibleClicked();
	afx_msg void OnGreen(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnRed(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnBlue(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnStyleChanged();
	afx_msg void OnPaint();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	CGraphSettings* m_Settings;
	void UpdateControls(void);
	void UpdateColorRect(void);
	int m_CurRelation;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GRAPHRELATIONSPROPPAGE_H__C62C4655_CC3F_11D2_AF8F_00A0C9B71DC4__INCLUDED_)
