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
// EdgePropPage.cpp : implementation file
//

#include "stdafx.h"
#include "discovermdi.h"
#include "EdgePropPage.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CEdgePropPage property page

IMPLEMENT_DYNCREATE(CEdgePropPage, CPropertyPage)

CEdgePropPage::CEdgePropPage() : CPropertyPage(CEdgePropPage::IDD)
{
	//{{AFX_DATA_INIT(CEdgePropPage)
	//}}AFX_DATA_INIT
}

CEdgePropPage::~CEdgePropPage()
{
}

void CEdgePropPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEdgePropPage)
	DDX_Control(pDX, IDC_EDGECOLOR, m_Color);
	DDX_Control(pDX, IDC_THICKNESS, m_Thickness);
	DDX_Control(pDX, IDC_EDGERED, m_Red);
	DDX_Control(pDX, IDC_EDGEGREEN, m_Green);
	DDX_Control(pDX, IDC_EDGEBLUE, m_Blue);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CEdgePropPage, CPropertyPage)
	//{{AFX_MSG_MAP(CEdgePropPage)
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_EDGEBLUE, OnEdgeBlue)
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_EDGEGREEN, OnEdgeGreen)
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_EDGERED, OnEdgeRed)
	ON_BN_CLICKED(IDC_EDGESELECTED, OnEdgeSelected)
	ON_BN_CLICKED(IDC_EDGEUNSELECTED, OnEdgeUnselected)
	ON_BN_CLICKED(IDC_EDGEFOCUSED, OnEdgeFocused)
	ON_CBN_SELCHANGE(IDC_THICKNESS, OnSelchangeThickness)
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEdgePropPage message handlers

void CEdgePropPage::SetSettings(CGraphSettings *settings) {
	m_Settings=settings;

}

void CEdgePropPage::OnEdgeBlue(NMHDR* pNMHDR, LRESULT* pResult)  {
    m_Settings->m_EdgeColor[m_CurType].m_Blue=255-m_Blue.GetPos();
	UpdateColorRect();
	*pResult = 0;	
}

void CEdgePropPage::OnEdgeGreen(NMHDR* pNMHDR, LRESULT* pResult)  {
    m_Settings->m_EdgeColor[m_CurType].m_Green=255-m_Green.GetPos();
	UpdateColorRect();
	*pResult = 0;
}

void CEdgePropPage::OnEdgeRed(NMHDR* pNMHDR, LRESULT* pResult)  {
    m_Settings->m_EdgeColor[m_CurType].m_Red=255-m_Red.GetPos();
	UpdateColorRect();
	*pResult = 0;
}

void CEdgePropPage::OnEdgeSelected()  {
	m_CurType=1;
	SetSlider();
	UpdateColorRect();
    SetEdgeSizeControl();
}

void CEdgePropPage::OnEdgeUnselected()  {
	m_CurType=0;
	SetSlider();
	UpdateColorRect();
    SetEdgeSizeControl();

}

void CEdgePropPage::OnEdgeFocused()  {
	m_CurType=2;
	SetSlider();
	UpdateColorRect();
    SetEdgeSizeControl();	
}

void CEdgePropPage::OnSelchangeThickness()  {
	switch(m_Thickness.GetCurSel()) {
	   case 0 : m_Settings->m_EdgeThickness=CGraphSettings::Thin;
		        break;
	   case 1 : m_Settings->m_EdgeThickness=CGraphSettings::Medium;
		        break;
	   case 2 : m_Settings->m_EdgeThickness=CGraphSettings::Thick;
		        break;
	}
}

BOOL CEdgePropPage::OnInitDialog()  {
	CPropertyPage::OnInitDialog();
	
    m_CurType=0;
	((CButton *)GetDlgItem(IDC_EDGEUNSELECTED))->SetCheck(TRUE);
	m_Red.SetRange(0,255);
	m_Green.SetRange(0,255);
	m_Blue.SetRange(0,255);

	m_Red.SetTicFreq(25);
	m_Green.SetTicFreq(25);
	m_Blue.SetTicFreq(25);

	SetSlider();
    SetEdgeSizeControl();
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CEdgePropPage::SetSlider() {
	m_Red.SetPos(255-m_Settings->m_EdgeColor[m_CurType].m_Red);
	m_Green.SetPos(255-m_Settings->m_EdgeColor[m_CurType].m_Green);
	m_Blue.SetPos(255-m_Settings->m_EdgeColor[m_CurType].m_Blue);
}

void CEdgePropPage::SetEdgeSizeControl() {
	if(m_Settings->m_EdgeThickness==CGraphSettings::Thin) 
		m_Thickness.SetCurSel(0);
	if(m_Settings->m_EdgeThickness==CGraphSettings::Medium) 
		m_Thickness.SetCurSel(1);
	if(m_Settings->m_EdgeThickness==CGraphSettings::Thick) 
		m_Thickness.SetCurSel(2);


}

void CEdgePropPage::OnPaint()  {
CRect edgeColorRect;
CRect winRect;

	CPaintDC dc(this); 
	m_Color.GetWindowRect(&edgeColorRect);
	GetWindowRect(&winRect);

	edgeColorRect.left-=winRect.left;
	edgeColorRect.right-=winRect.left;
	edgeColorRect.top-=winRect.top;
	edgeColorRect.bottom-=winRect.top;

    edgeColorRect.InflateRect(-1,-1);
	CBrush edgeBrush(RGB(m_Settings->m_EdgeColor[m_CurType].m_Red,
		                 m_Settings->m_EdgeColor[m_CurType].m_Green,
						 m_Settings->m_EdgeColor[m_CurType].m_Blue
						)
					);
	dc.FillRect(edgeColorRect,&edgeBrush);	
}

void CEdgePropPage::UpdateColorRect() {
CRect colorRect;
CRect winRect;


	m_Color.GetWindowRect(&colorRect);
	GetWindowRect(&winRect);
	colorRect.left-=winRect.left;
	colorRect.right-=winRect.left;
	colorRect.top-=winRect.top;
	colorRect.bottom-=winRect.top;
	InvalidateRect(&colorRect,FALSE);
}
