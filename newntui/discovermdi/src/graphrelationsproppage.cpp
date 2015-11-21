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
// GraphRelationsPropPage.cpp : implementation file
//

#include "stdafx.h"
#include "discovermdi.h"
#include "GraphRelationsPropPage.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CGraphRelationsPropPage property page

IMPLEMENT_DYNCREATE(CGraphRelationsPropPage, CPropertyPage)

CGraphRelationsPropPage::CGraphRelationsPropPage() : CPropertyPage(CGraphRelationsPropPage::IDD)
{
	//{{AFX_DATA_INIT(CGraphRelationsPropPage)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

CGraphRelationsPropPage::~CGraphRelationsPropPage()
{
}

void CGraphRelationsPropPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CGraphRelationsPropPage)
	DDX_Control(pDX, IDC_VISIBLE, m_Visible);
	DDX_Control(pDX, IDC_THICKNESSCOMBO, m_ThicknessSelector);
	DDX_Control(pDX, IDC_STYLECOMBO, m_StyleSelector);
	DDX_Control(pDX, IDC_RELATIONCOMBO, m_RelationSelector);
	DDX_Control(pDX, IDC_RED, m_Red);
	DDX_Control(pDX, IDC_GREEN, m_Green);
	DDX_Control(pDX, IDC_COLOR, m_Color);
	DDX_Control(pDX, IDC_BLUE, m_Blue);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CGraphRelationsPropPage, CPropertyPage)
	//{{AFX_MSG_MAP(CGraphRelationsPropPage)
	ON_CBN_SELCHANGE(IDC_RELATIONCOMBO, OnRelationChanged)
	ON_CBN_SELCHANGE(IDC_THICKNESSCOMBO, OnThicknessChanged)
	ON_BN_CLICKED(IDC_VISIBLE, OnVisibleClicked)
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_GREEN, OnGreen)
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_RED, OnRed)
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_BLUE, OnBlue)
	ON_CBN_SELCHANGE(IDC_STYLECOMBO, OnStyleChanged)
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGraphRelationsPropPage message handlers

void CGraphRelationsPropPage::OnRelationChanged()  {
   m_CurRelation = m_RelationSelector.GetCurSel();
   UpdateControls();
	
}

void CGraphRelationsPropPage::OnThicknessChanged()  {
	switch(m_ThicknessSelector.GetCurSel()) {
	   case 0 : m_Settings->m_Relations[m_CurRelation].m_Thickness = CGraphSettings::Thin;  break;
	   case 1 : m_Settings->m_Relations[m_CurRelation].m_Thickness = CGraphSettings::Medium;break;
	   case 2 : m_Settings->m_Relations[m_CurRelation].m_Thickness = CGraphSettings::Thick; break;
	}	
}

void CGraphRelationsPropPage::OnVisibleClicked()  {
	 m_Settings->m_Relations[m_CurRelation].m_Visible = m_Visible.GetCheck();
}


void CGraphRelationsPropPage::OnRed(NMHDR* pNMHDR, LRESULT* pResult)  {
    m_Settings->m_Relations[m_CurRelation].m_Color.m_Red=255-m_Red.GetPos();
	UpdateColorRect();
	*pResult = 0;
}

void CGraphRelationsPropPage::OnGreen(NMHDR* pNMHDR, LRESULT* pResult) {
    m_Settings->m_Relations[m_CurRelation].m_Color.m_Green=255-m_Green.GetPos();
	UpdateColorRect();
	*pResult = 0;
}

void CGraphRelationsPropPage::OnBlue(NMHDR* pNMHDR, LRESULT* pResult)  {
    m_Settings->m_Relations[m_CurRelation].m_Color.m_Blue=255-m_Blue.GetPos();
	UpdateColorRect();
	*pResult = 0;
}



void CGraphRelationsPropPage::OnStyleChanged()  {
	switch(m_StyleSelector.GetCurSel()) {
	   case 0 : m_Settings->m_Relations[m_CurRelation].m_Style = PS_SOLID;   break;
	   case 1 : m_Settings->m_Relations[m_CurRelation].m_Style = PS_DOT;     break;
	   case 2 : m_Settings->m_Relations[m_CurRelation].m_Style = PS_DASH;    break;
	}
}

void CGraphRelationsPropPage::UpdateColorRect() {
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

void CGraphRelationsPropPage::OnPaint()  {
CRect colorRect;
CRect winRect;

	CPaintDC dc(this); 
	m_Color.GetWindowRect(&colorRect);
	GetWindowRect(&winRect);

	colorRect.left-=winRect.left;
	colorRect.right-=winRect.left;
	colorRect.top-=winRect.top;
	colorRect.bottom-=winRect.top;
    colorRect.InflateRect(-1,-1);

	CBrush nodeBrush(RGB(m_Settings->m_Relations[m_CurRelation].m_Color.m_Red,
		                 m_Settings->m_Relations[m_CurRelation].m_Color.m_Green,
						 m_Settings->m_Relations[m_CurRelation].m_Color.m_Blue
						)
					);
	dc.FillRect(colorRect,&nodeBrush);
    colorRect.InflateRect(1,1);
	dc.FrameRect(&colorRect,&CBrush(RGB(0,0,0)));
	
}

void CGraphRelationsPropPage::UpdateControls() {
	m_Red.SetPos(255-m_Settings->m_Relations[m_CurRelation].m_Color.m_Red);
	m_Green.SetPos(255-m_Settings->m_Relations[m_CurRelation].m_Color.m_Green);
	m_Blue.SetPos(255-m_Settings->m_Relations[m_CurRelation].m_Color.m_Blue);

	switch(m_Settings->m_Relations[m_CurRelation].m_Style) {
	   case  PS_SOLID : m_StyleSelector.SetCurSel(0); break;
	   case  PS_DOT   : m_StyleSelector.SetCurSel(1); break;
	   case  PS_DASH  : m_StyleSelector.SetCurSel(2); break;
	}
	switch(m_Settings->m_Relations[m_CurRelation].m_Thickness) {
	   case CGraphSettings::Thin   : m_ThicknessSelector.SetCurSel(0); break;
	   case CGraphSettings::Medium : m_ThicknessSelector.SetCurSel(1); break;
	   case CGraphSettings::Thick  : m_ThicknessSelector.SetCurSel(2); break;
	}
	m_Visible.SetCheck(m_Settings->m_Relations[m_CurRelation].m_Visible);
	UpdateColorRect();

}

BOOL CGraphRelationsPropPage::OnInitDialog()  {
	CPropertyPage::OnInitDialog();

	m_CurRelation=0;
	m_Red.SetRange(0,255);
	m_Green.SetRange(0,255);
	m_Blue.SetRange(0,255);
	m_Red.SetTicFreq(25);
	m_Green.SetTicFreq(25);
	m_Blue.SetTicFreq(25);
	m_RelationSelector.SetCurSel(m_CurRelation);
    UpdateControls();
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CGraphRelationsPropPage::SetGraphSettings(CGraphSettings *settings) {
	m_Settings=settings;

}
