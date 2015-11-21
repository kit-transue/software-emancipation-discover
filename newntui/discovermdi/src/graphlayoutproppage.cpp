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
// GraphLayoutPropPage.cpp : implementation file
//

#include "stdafx.h"
#include "discovermdi.h"
#include "GraphLayoutPropPage.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CGraphLayoutPropPage property page

IMPLEMENT_DYNCREATE(CGraphLayoutPropPage, CPropertyPage)

CGraphLayoutPropPage::CGraphLayoutPropPage() : CPropertyPage(CGraphLayoutPropPage::IDD)
{
	//{{AFX_DATA_INIT(CGraphLayoutPropPage)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

CGraphLayoutPropPage::~CGraphLayoutPropPage()
{
}

void CGraphLayoutPropPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CGraphLayoutPropPage)
	DDX_Control(pDX, IDC_VARSPACE, m_VariableSpace);
	DDX_Control(pDX, IDC_SPACENODENAME, m_SpaceNodeName);
	DDX_Control(pDX, IDC_SPACENODE, m_SpaceNode);
	DDX_Control(pDX, IDC_SPACELEVELNAME, m_SpaceLevelName);
	DDX_Control(pDX, IDC_SPACELEVEL, m_SpaceLevel);
	DDX_Control(pDX, IDC_ORTHOGONAL, m_Orthogonal);
	DDX_Control(pDX, IDC_MERGEROWS, m_MergeRows);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CGraphLayoutPropPage, CPropertyPage)
	//{{AFX_MSG_MAP(CGraphLayoutPropPage)
	ON_BN_CLICKED(IDC_RIGHTTOLEFT, OnRightToLeft)
	ON_BN_CLICKED(IDC_LEFTTORIGHT, OnLeftToRight)
	ON_BN_CLICKED(IDC_TOPTOBOTTOM, OnTopToBottom)
	ON_BN_CLICKED(IDC_BOTTOMTOTOP, OnBottomToTop)
	ON_BN_CLICKED(IDC_JUSTIFY_CENTER, OnJustifyCenter)
	ON_BN_CLICKED(IDC_JUSTIFY_END, OnJustifyEnd)
	ON_BN_CLICKED(IDC_JUSTIFY_START, OnJustifyStart)
	ON_BN_CLICKED(IDC_ORTHOGONAL, OnOrthogonal)
	ON_BN_CLICKED(IDC_MERGEROWS, OnMerge)
	ON_BN_CLICKED(IDC_VARSPACE, OnVarSpace)
	ON_EN_CHANGE(IDC_SPACENODE, OnChangeSpacenode)
	ON_EN_CHANGE(IDC_SPACELEVEL, OnChangeSpacelevel)
	ON_EN_CHANGE(IDC_SPACEROW, OnChangeSpacerow)
	ON_EN_CHANGE(IDC_SPACECOLUMN, OnChangeSpacecolumn)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGraphLayoutPropPage message handlers

void CGraphLayoutPropPage::OnRightToLeft()  {
	m_Settings->m_Orientation = CGraphSettings::RightToLeft;
    GetDlgItem(IDC_JUSTIFY_START)->SetWindowText("Right");
	GetDlgItem(IDC_JUSTIFY_CENTER)->SetWindowText("Center");
	GetDlgItem(IDC_JUSTIFY_END)->SetWindowText("Left");
	
}

void CGraphLayoutPropPage::OnLeftToRight()  {
	m_Settings->m_Orientation = CGraphSettings::LeftToRight;
    GetDlgItem(IDC_JUSTIFY_START)->SetWindowText("Left");
	GetDlgItem(IDC_JUSTIFY_CENTER)->SetWindowText("Center");
	GetDlgItem(IDC_JUSTIFY_END)->SetWindowText("Right");
	
}

void CGraphLayoutPropPage::OnTopToBottom() {
	m_Settings->m_Orientation = CGraphSettings::TopToBottom;
    GetDlgItem(IDC_JUSTIFY_START)->SetWindowText("Top");
	GetDlgItem(IDC_JUSTIFY_CENTER)->SetWindowText("Center");
	GetDlgItem(IDC_JUSTIFY_END)->SetWindowText("Bottom");
	
}

void CGraphLayoutPropPage::OnBottomToTop()  {
	m_Settings->m_Orientation = CGraphSettings::BottomToTop;
    GetDlgItem(IDC_JUSTIFY_START)->SetWindowText("Bottom");
	GetDlgItem(IDC_JUSTIFY_CENTER)->SetWindowText("Center");
	GetDlgItem(IDC_JUSTIFY_END)->SetWindowText("Top");
	
}

void CGraphLayoutPropPage::OnJustifyCenter()  {
     m_Settings->m_Justification = CGraphSettings::Center;	
}

void CGraphLayoutPropPage::OnJustifyEnd()  {
     m_Settings->m_Justification = CGraphSettings::End;	
	
}

void CGraphLayoutPropPage::OnJustifyStart()  {
     m_Settings->m_Justification = CGraphSettings::Start;	
	
}

void CGraphLayoutPropPage::OnOrthogonal() {
	m_Settings->m_Routing.m_Orthogonal = ((CButton *)GetDlgItem(IDC_ORTHOGONAL))->GetCheck();
    GetDlgItem(IDC_MERGEROWS)->EnableWindow(m_Settings->m_Routing.m_Orthogonal);
    GetDlgItem(IDC_SPACEROW)->EnableWindow(m_Settings->m_Routing.m_Orthogonal);
    GetDlgItem(IDC_SPACECOLUMN)->EnableWindow(m_Settings->m_Routing.m_Orthogonal);
}

void CGraphLayoutPropPage::OnMerge()  {
	m_Settings->m_Routing.m_Merge= ((CButton *)GetDlgItem(IDC_MERGEROWS))->GetCheck();
    GetDlgItem(IDC_MERGEROWS)->EnableWindow(m_Settings->m_Routing.m_Orthogonal);
	
}

void CGraphLayoutPropPage::OnVarSpace()  {
	 m_Settings->m_Spacing.m_EnableVatiable = ((CButton *)GetDlgItem(IDC_VARSPACE))->GetCheck();
     GetDlgItem(IDC_SPACELEVEL)->EnableWindow(!m_Settings->m_Spacing.m_EnableVatiable);
     GetDlgItem(IDC_SPACEROW)->EnableWindow(m_Settings->m_Spacing.m_EnableVatiable);
     GetDlgItem(IDC_SPACECOLUMN)->EnableWindow(m_Settings->m_Spacing.m_EnableVatiable);
}

void CGraphLayoutPropPage::SetGraphSettings(CGraphSettings *settings) {
	m_Settings=settings;

}

BOOL CGraphLayoutPropPage::OnInitDialog()  {
	CPropertyPage::OnInitDialog();
	switch(m_Settings->m_Orientation) {
	   case CGraphSettings::LeftToRight : ((CButton *)GetDlgItem(IDC_LEFTTORIGHT))->SetCheck(TRUE);	
		                                  ((CButton *)GetDlgItem(IDC_RIGHTTOLEFT))->SetCheck(FALSE);	
		                                  ((CButton *)GetDlgItem(IDC_TOPTOBOTTOM))->SetCheck(FALSE);	
		                                  ((CButton *)GetDlgItem(IDC_BOTTOMTOTOP))->SetCheck(FALSE);
										  GetDlgItem(IDC_JUSTIFY_START)->SetWindowText("Left");
										  GetDlgItem(IDC_JUSTIFY_CENTER)->SetWindowText("Center");
										  GetDlgItem(IDC_JUSTIFY_END)->SetWindowText("Right");
						                  break;
	   case CGraphSettings::RightToLeft : ((CButton *)GetDlgItem(IDC_LEFTTORIGHT))->SetCheck(FALSE);	
		                                  ((CButton *)GetDlgItem(IDC_RIGHTTOLEFT))->SetCheck(TRUE);	
		                                  ((CButton *)GetDlgItem(IDC_TOPTOBOTTOM))->SetCheck(FALSE);	
		                                  ((CButton *)GetDlgItem(IDC_BOTTOMTOTOP))->SetCheck(FALSE);	
										  GetDlgItem(IDC_JUSTIFY_START)->SetWindowText("Right");
										  GetDlgItem(IDC_JUSTIFY_CENTER)->SetWindowText("Center");
										  GetDlgItem(IDC_JUSTIFY_END)->SetWindowText("Left");
						                  break;
	   case CGraphSettings::TopToBottom : ((CButton *)GetDlgItem(IDC_LEFTTORIGHT))->SetCheck(FALSE);	
		                                  ((CButton *)GetDlgItem(IDC_RIGHTTOLEFT))->SetCheck(FALSE);	
		                                  ((CButton *)GetDlgItem(IDC_TOPTOBOTTOM))->SetCheck(TRUE);	
		                                  ((CButton *)GetDlgItem(IDC_BOTTOMTOTOP))->SetCheck(FALSE);	
										  GetDlgItem(IDC_JUSTIFY_START)->SetWindowText("Top");
										  GetDlgItem(IDC_JUSTIFY_CENTER)->SetWindowText("Center");
										  GetDlgItem(IDC_JUSTIFY_END)->SetWindowText("Bottom");
						                  break;
	   case CGraphSettings::BottomToTop : ((CButton *)GetDlgItem(IDC_LEFTTORIGHT))->SetCheck(FALSE);	
		                                  ((CButton *)GetDlgItem(IDC_RIGHTTOLEFT))->SetCheck(FALSE);	
		                                  ((CButton *)GetDlgItem(IDC_TOPTOBOTTOM))->SetCheck(FALSE);	
		                                  ((CButton *)GetDlgItem(IDC_BOTTOMTOTOP))->SetCheck(TRUE);	
										  GetDlgItem(IDC_JUSTIFY_START)->SetWindowText("Bottom");
										  GetDlgItem(IDC_JUSTIFY_CENTER)->SetWindowText("Center");
										  GetDlgItem(IDC_JUSTIFY_END)->SetWindowText("Top");
						                  break;
	}
	switch(m_Settings->m_Justification) {
	   case CGraphSettings::Start : ((CButton *)GetDlgItem(IDC_JUSTIFY_START))->SetCheck(TRUE);	
		                            ((CButton *)GetDlgItem(IDC_JUSTIFY_CENTER))->SetCheck(FALSE);	
									((CButton *)GetDlgItem(IDC_JUSTIFY_END))->SetCheck(FALSE);	
									break;
	   case CGraphSettings::Center :((CButton *)GetDlgItem(IDC_JUSTIFY_START))->SetCheck(FALSE);	
		                            ((CButton *)GetDlgItem(IDC_JUSTIFY_CENTER))->SetCheck(TRUE);	
									((CButton *)GetDlgItem(IDC_JUSTIFY_END))->SetCheck(FALSE);	
									break;
	   case CGraphSettings::End :   ((CButton *)GetDlgItem(IDC_JUSTIFY_START))->SetCheck(FALSE);	
		                            ((CButton *)GetDlgItem(IDC_JUSTIFY_CENTER))->SetCheck(FALSE);	
									((CButton *)GetDlgItem(IDC_JUSTIFY_END))->SetCheck(TRUE);	
									break;
	}
	if(m_Settings->m_Routing.m_Orthogonal) {
          ((CButton *)GetDlgItem(IDC_ORTHOGONAL))->SetCheck(TRUE);
           GetDlgItem(IDC_MERGEROWS)->EnableWindow(TRUE);
	} else {
          ((CButton *)GetDlgItem(IDC_ORTHOGONAL))->SetCheck(FALSE);
           GetDlgItem(IDC_MERGEROWS)->EnableWindow(FALSE);
	}
	if(m_Settings->m_Routing.m_Merge) 
          ((CButton *)GetDlgItem(IDC_MERGEROWS))->SetCheck(TRUE);
	else
          ((CButton *)GetDlgItem(IDC_MERGEROWS))->SetCheck(FALSE);
	if(m_Settings->m_Spacing.m_EnableVatiable) {
          ((CButton *)GetDlgItem(IDC_VARSPACE))->SetCheck(TRUE);
           GetDlgItem(IDC_SPACELEVEL)->EnableWindow(FALSE);
           GetDlgItem(IDC_SPACEROW)->EnableWindow(TRUE);
           GetDlgItem(IDC_SPACECOLUMN)->EnableWindow(TRUE);
	} else {
          ((CButton *)GetDlgItem(IDC_VARSPACE))->SetCheck(FALSE);
           GetDlgItem(IDC_SPACELEVEL)->EnableWindow(TRUE);
           GetDlgItem(IDC_SPACEROW)->EnableWindow(FALSE);
           GetDlgItem(IDC_SPACECOLUMN)->EnableWindow(FALSE);
	}
	CString nodeSpaceStr;
	nodeSpaceStr.Format("%d",m_Settings->m_Spacing.m_NodeSpacing);
    GetDlgItem(IDC_SPACENODE)->SetWindowText(nodeSpaceStr);
	CString levelSpaceStr;
	levelSpaceStr.Format("%d",m_Settings->m_Spacing.m_LevelSpacing);
    GetDlgItem(IDC_SPACELEVEL)->SetWindowText(levelSpaceStr);

	CString rowSpaceStr;
	rowSpaceStr.Format("%d",m_Settings->m_Spacing.m_RowSpacing);
    GetDlgItem(IDC_SPACEROW)->SetWindowText(rowSpaceStr);

	CString columnSpaceStr;
	columnSpaceStr.Format("%d",m_Settings->m_Spacing.m_ColumnSpacing);
    GetDlgItem(IDC_SPACECOLUMN)->SetWindowText(columnSpaceStr);
	return TRUE;  
}


void CGraphLayoutPropPage::OnChangeSpacenode()  {
	CString nodeSpaceStr;
	GetDlgItem(IDC_SPACENODE)->GetWindowText(nodeSpaceStr);
	m_Settings->m_Spacing.m_NodeSpacing=atoi(nodeSpaceStr);

}

void CGraphLayoutPropPage::OnChangeSpacelevel()  {
	CString levelSpaceStr;
	GetDlgItem(IDC_SPACELEVEL)->GetWindowText(levelSpaceStr);
	m_Settings->m_Spacing.m_LevelSpacing=atoi(levelSpaceStr);
	
}

void CGraphLayoutPropPage::OnChangeSpacerow()  {
	CString rowSpaceStr;
	GetDlgItem(IDC_SPACEROW)->GetWindowText(rowSpaceStr);
	m_Settings->m_Spacing.m_RowSpacing=atoi(rowSpaceStr);
	
}

void CGraphLayoutPropPage::OnChangeSpacecolumn()  {
	CString columnSpaceStr;
	GetDlgItem(IDC_SPACECOLUMN)->GetWindowText(columnSpaceStr);
	m_Settings->m_Spacing.m_ColumnSpacing=atoi(columnSpaceStr);

}
