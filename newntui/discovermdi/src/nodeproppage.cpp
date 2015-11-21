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
// NodePropPage.cpp : implementation file
//

#include "stdafx.h"
#include "discovermdi.h"
#include "NodePropPage.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CNodePropPage property page

IMPLEMENT_DYNCREATE(CNodePropPage, CPropertyPage)

CNodePropPage::CNodePropPage() : CPropertyPage(CNodePropPage::IDD)
{
	//{{AFX_DATA_INIT(CNodePropPage)
	//}}AFX_DATA_INIT
}

CNodePropPage::~CNodePropPage()
{
}

void CNodePropPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CNodePropPage)
	DDX_Control(pDX, IDC_THICKNESS, m_BorderSize);
	DDX_Control(pDX, IDC_NODERED, m_NodeRed);
	DDX_Control(pDX, IDC_NODEGREEN, m_NodeGreen);
	DDX_Control(pDX, IDC_NODECOLOR, m_NodeColor);
	DDX_Control(pDX, IDC_BORDERRED, m_BorderRed);
	DDX_Control(pDX, IDC_BORDERGREEN, m_BorderGreen);
	DDX_Control(pDX, IDC_NODEBLUE, m_NodeBlue);
	DDX_Control(pDX, IDC_BORDERCOLOR, m_BorderColor);
	DDX_Control(pDX, IDC_BORDERBLUE, m_BorderBlue);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CNodePropPage, CPropertyPage)
	//{{AFX_MSG_MAP(CNodePropPage)
	ON_BN_CLICKED(IDC_FOCUSED, OnFocused)
	ON_BN_CLICKED(IDC_SELECTED, OnSelected)
	ON_BN_CLICKED(IDC_UNSELECTED, OnUnselected)
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_BORDERRED, OnBorderRed)
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_BORDERGREEN, OnBorderGreen)
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_BORDERBLUE, OnBorderBlue)
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_NODERED, OnNodeRed)
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_NODEGREEN, OneNodeGreen)
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_NODEBLUE, OnNodeBlue)
	ON_WM_PAINT()
	ON_BN_CLICKED(IDC_FONT, OnFont)
	ON_CBN_SELCHANGE(IDC_THICKNESS, OnSelchangeThickness)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CNodePropPage message handlers

void CNodePropPage::OnFocused()  {
	m_CurType=2;
	SetSliders();
	UpdateNodeColorRect();
	UpdateBorderColorRect();
    SetBorderSizeControl();
}

void CNodePropPage::OnSelected()  {
	m_CurType=1;
	SetSliders();
	UpdateNodeColorRect();
	UpdateBorderColorRect();
    SetBorderSizeControl();
	
}

void CNodePropPage::OnUnselected()  {
	m_CurType=0;
	SetSliders();
	UpdateNodeColorRect();
	UpdateBorderColorRect();
    SetBorderSizeControl();
	
}

BOOL CNodePropPage::OnInitDialog()  {


	CPropertyPage::OnInitDialog();

    m_CurType=0;
	((CButton *)GetDlgItem(IDC_UNSELECTED))->SetCheck(TRUE);
	m_BorderRed.SetRange(0,255);
	m_BorderGreen.SetRange(0,255);
	m_BorderBlue.SetRange(0,255);
	m_NodeRed.SetRange(0,255);
	m_NodeGreen.SetRange(0,255);
	m_NodeBlue.SetRange(0,255);

	m_BorderRed.SetTicFreq(25);
	m_BorderGreen.SetTicFreq(25);
	m_BorderBlue.SetTicFreq(25);
	m_NodeRed.SetTicFreq(25);
	m_NodeGreen.SetTicFreq(25);
	m_NodeBlue.SetTicFreq(25);

	SetSliders();
    SetBorderSizeControl();
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CNodePropPage::SetGraphSettings(CGraphSettings *settings) {
	m_Settings=settings;

}

void CNodePropPage::SetSliders() {
	m_NodeRed.SetPos(255-m_Settings->m_NodeBackground[m_CurType].m_Red);
	m_NodeGreen.SetPos(255-m_Settings->m_NodeBackground[m_CurType].m_Green);
	m_NodeBlue.SetPos(255-m_Settings->m_NodeBackground[m_CurType].m_Blue);
	m_BorderRed.SetPos(255-m_Settings->m_NodeBorder[m_CurType].m_Red);
	m_BorderGreen.SetPos(255-m_Settings->m_NodeBorder[m_CurType].m_Green);
	m_BorderBlue.SetPos(255-m_Settings->m_NodeBorder[m_CurType].m_Blue);
}

void CNodePropPage::OnBorderRed(NMHDR* pNMHDR, LRESULT* pResult)  {
    m_Settings->m_NodeBorder[m_CurType].m_Red=255-m_BorderRed.GetPos();
	UpdateBorderColorRect();
	*pResult = 0;
}

void CNodePropPage::OnBorderGreen(NMHDR* pNMHDR, LRESULT* pResult)  {
    m_Settings->m_NodeBorder[m_CurType].m_Green=255-m_BorderGreen.GetPos();
	UpdateBorderColorRect();
	*pResult = 0;
}

void CNodePropPage::OnBorderBlue(NMHDR* pNMHDR, LRESULT* pResult)  {
    m_Settings->m_NodeBorder[m_CurType].m_Blue=255-m_BorderBlue.GetPos();
	UpdateBorderColorRect();
	*pResult = 0;
}

void CNodePropPage::OnNodeRed(NMHDR* pNMHDR, LRESULT* pResult)  {
    m_Settings->m_NodeBackground[m_CurType].m_Red=255-m_NodeRed.GetPos();
	UpdateNodeColorRect();
	*pResult = 0;
}

void CNodePropPage::OneNodeGreen(NMHDR* pNMHDR, LRESULT* pResult)  {
    m_Settings->m_NodeBackground[m_CurType].m_Green=255-m_NodeGreen.GetPos();
	UpdateNodeColorRect();
	*pResult = 0;
}

void CNodePropPage::OnNodeBlue(NMHDR* pNMHDR, LRESULT* pResult)  {
    m_Settings->m_NodeBackground[m_CurType].m_Blue=255-m_NodeBlue.GetPos();
	UpdateNodeColorRect();
	*pResult = 0;
}

void CNodePropPage::OnPaint()  {
CRect nodeColorRect;
CRect borderColorRect;
CRect winRect;

	CPaintDC dc(this); 
	m_NodeColor.GetClientRect(&nodeColorRect);
	m_NodeColor.GetWindowRect(&nodeColorRect);
	m_BorderColor.GetWindowRect(&borderColorRect);
	GetWindowRect(&winRect);
	nodeColorRect.left-=winRect.left;
	nodeColorRect.right-=winRect.left;
	nodeColorRect.top-=winRect.top;
	nodeColorRect.bottom-=winRect.top;
	borderColorRect.left-=winRect.left;
	borderColorRect.right-=winRect.left;
	borderColorRect.top-=winRect.top;
	borderColorRect.bottom-=winRect.top;

    borderColorRect.InflateRect(-1,-1);
    nodeColorRect.InflateRect(-1,-1);
	CBrush nodeBrush(RGB(m_Settings->m_NodeBackground[m_CurType].m_Red,
		                 m_Settings->m_NodeBackground[m_CurType].m_Green,
						 m_Settings->m_NodeBackground[m_CurType].m_Blue
						)
					);
	dc.FillRect(nodeColorRect,&nodeBrush);
	CBrush borderBrush(RGB(m_Settings->m_NodeBorder[m_CurType].m_Red,
                           m_Settings->m_NodeBorder[m_CurType].m_Green,
						   m_Settings->m_NodeBorder[m_CurType].m_Blue
						  )
					   );
	dc.FillRect(borderColorRect,&borderBrush);
    borderColorRect.InflateRect(1,1);
    nodeColorRect.InflateRect(1,1);
	dc.FrameRect(&borderColorRect,&CBrush(RGB(0,0,0)));
	dc.FrameRect(&nodeColorRect,&CBrush(RGB(0,0,0)));
}

void CNodePropPage::UpdateNodeColorRect() {
CRect nodeColorRect;
CRect winRect;
	m_NodeColor.GetWindowRect(&nodeColorRect);
	GetWindowRect(&winRect);
	nodeColorRect.left-=winRect.left;
	nodeColorRect.right-=winRect.left;
	nodeColorRect.top-=winRect.top;
	nodeColorRect.bottom-=winRect.top;
	InvalidateRect(&nodeColorRect,FALSE);
}



void CNodePropPage::UpdateBorderColorRect() {
CRect borderColorRect;
CRect winRect;


	m_BorderColor.GetWindowRect(&borderColorRect);
	GetWindowRect(&winRect);
	borderColorRect.left-=winRect.left;
	borderColorRect.right-=winRect.left;
	borderColorRect.top-=winRect.top;
	borderColorRect.bottom-=winRect.top;
	InvalidateRect(&borderColorRect,FALSE);
}

void CNodePropPage::OnFont()  {
LOGFONT lf;

    memcpy(&lf,&(m_Settings->m_NodeFont),sizeof(LOGFONT));
	lf.lfHeight=lf.lfHeight/30;
    CFontDialog fontDlg(&lf);
    if(fontDlg.DoModal()==IDOK) {
	     lf.lfHeight=lf.lfHeight*30;
         memcpy(&(m_Settings->m_NodeFont),&lf,sizeof(LOGFONT));
	}
	
}

void CNodePropPage::SetBorderSizeControl() {
	if(m_Settings->m_BorderThickness==CGraphSettings::Thin) 
		m_BorderSize.SetCurSel(0);
	if(m_Settings->m_BorderThickness==CGraphSettings::Medium) 
		m_BorderSize.SetCurSel(1);
	if(m_Settings->m_BorderThickness==CGraphSettings::Thick) 
		m_BorderSize.SetCurSel(2);

}

void CNodePropPage::OnSelchangeThickness()  {
	switch(m_BorderSize.GetCurSel()) {
	   case 0 : m_Settings->m_BorderThickness=CGraphSettings::Thin;
		        break;
	   case 1 : m_Settings->m_BorderThickness=CGraphSettings::Medium;
		        break;
	   case 2 : m_Settings->m_BorderThickness=CGraphSettings::Thick;
		        break;
	}

	
}
