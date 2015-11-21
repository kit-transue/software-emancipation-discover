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
// GraphInheritancePropPage.cpp : implementation file
//

#include "stdafx.h"
#include "discovermdi.h"
#include "GraphInheritancePropPage.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CGraphInheritancePropPage property page

IMPLEMENT_DYNCREATE(CGraphInheritancePropPage, CPropertyPage)

CGraphInheritancePropPage::CGraphInheritancePropPage() : CPropertyPage(CGraphInheritancePropPage::IDD)
{
	//{{AFX_DATA_INIT(CGraphInheritancePropPage)
	//}}AFX_DATA_INIT
}

CGraphInheritancePropPage::~CGraphInheritancePropPage()
{
}

void CGraphInheritancePropPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CGraphInheritancePropPage)
	DDX_Control(pDX, IDC_MERGESAME, m_Merge);
	DDX_Control(pDX, IDC_PUBLIC, m_Public);
	DDX_Control(pDX, IDC_PROTECTED, m_Protected);
	DDX_Control(pDX, IDC_PRIVATE, m_Private);
	DDX_Control(pDX, IDC_PACKAGE_ACCESS, m_PackageAccess);
	DDX_Control(pDX, IDC_ENABLE_CLASS_PROPERTIES, m_ClassProperties);
	DDX_Control(pDX, IDC_ENABLE_CLASS_MEHTODS, m_ClassMethods);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CGraphInheritancePropPage, CPropertyPage)
	//{{AFX_MSG_MAP(CGraphInheritancePropPage)
	ON_BN_CLICKED(IDC_ENABLE_CLASS_MEHTODS, OnEnableClassMehtods)
	ON_BN_CLICKED(IDC_ENABLE_CLASS_PROPERTIES, OnEnableClassProperties)
	ON_BN_CLICKED(IDC_LONG_NAMES, OnLongNames)
	ON_BN_CLICKED(IDC_PACKAGE_ACCESS, OnPackageAccess)
	ON_BN_CLICKED(IDC_PRIVATE, OnPrivate)
	ON_BN_CLICKED(IDC_PROTECTED, OnProtected)
	ON_BN_CLICKED(IDC_PUBLIC, OnPublic)
	ON_BN_CLICKED(IDC_SHORT_NAMES, OnShortNames)
	ON_BN_CLICKED(IDC_SORT_BY_ACCESS, OnSortByAccess)
	ON_BN_CLICKED(IDC_SORT_BY_NAME, OnSortByName)
	ON_BN_CLICKED(IDC_MERGESAME, OnMergeClasses)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGraphInheritancePropPage message handlers

void CGraphInheritancePropPage::SetGraphSettings(CGraphSettings *settings) {
	m_Settings = settings;

}

BOOL CGraphInheritancePropPage::OnInitDialog()  {
	CPropertyPage::OnInitDialog();
	m_ClassMethods.SetCheck(m_Settings->m_Inheritance.m_ShowClassMethods);
	m_ClassProperties.SetCheck(m_Settings->m_Inheritance.m_ShowClassProperties);
	m_Public.SetCheck(m_Settings->m_Inheritance.m_ShowPublicMembers);
	m_Protected.SetCheck(m_Settings->m_Inheritance.m_ShowProtectedMembers);
	m_PackageAccess.SetCheck(m_Settings->m_Inheritance.m_ShowPackageAccessMembers);
	m_Private.SetCheck(m_Settings->m_Inheritance.m_ShowPrivateMembers);
	m_Merge.SetCheck(m_Settings->m_Inheritance.m_Merge);
	if(m_Settings->m_Inheritance.m_Sort==CGraphSettings::ByName) {
		((CButton *)GetDlgItem(IDC_SORT_BY_NAME))->SetCheck(TRUE);
		((CButton *)GetDlgItem(IDC_SORT_BY_ACCESS))->SetCheck(FALSE);
	} else {
		((CButton *)GetDlgItem(IDC_SORT_BY_NAME))->SetCheck(FALSE);
		((CButton *)GetDlgItem(IDC_SORT_BY_ACCESS))->SetCheck(TRUE);
	}
	if(m_Settings->m_Inheritance.m_Name==CGraphSettings::Long) {
		((CButton *)GetDlgItem(IDC_SHORT_NAMES))->SetCheck(FALSE);
		((CButton *)GetDlgItem(IDC_LONG_NAMES))->SetCheck(TRUE);
	} else {
		((CButton *)GetDlgItem(IDC_SHORT_NAMES))->SetCheck(TRUE);
		((CButton *)GetDlgItem(IDC_LONG_NAMES))->SetCheck(FALSE);
	}
	
	return TRUE;  
}

void CGraphInheritancePropPage::OnEnableClassMehtods()  {
   m_Settings->m_Inheritance.m_ShowClassMethods=m_ClassMethods.GetCheck();
}

void CGraphInheritancePropPage::OnEnableClassProperties()  {
   m_Settings->m_Inheritance.m_ShowClassProperties=m_ClassProperties.GetCheck();
	
}


void CGraphInheritancePropPage::OnPublic()  {
   m_Settings->m_Inheritance.m_ShowPublicMembers=m_Public.GetCheck();

}

void CGraphInheritancePropPage::OnProtected()  {
   m_Settings->m_Inheritance.m_ShowProtectedMembers=m_Protected.GetCheck();
	
}
	
void CGraphInheritancePropPage::OnPackageAccess()  {
   m_Settings->m_Inheritance.m_ShowPackageAccessMembers=m_PackageAccess.GetCheck();
	
}

void CGraphInheritancePropPage::OnPrivate()  {
   m_Settings->m_Inheritance.m_ShowPrivateMembers=m_Private.GetCheck();
	
}





void CGraphInheritancePropPage::OnLongNames()  {
   m_Settings->m_Inheritance.m_Name=CGraphSettings::Long;
}

void CGraphInheritancePropPage::OnShortNames()  {
   m_Settings->m_Inheritance.m_Name=CGraphSettings::Short;
}



void CGraphInheritancePropPage::OnSortByAccess()  {
   m_Settings->m_Inheritance.m_Sort=CGraphSettings::ByAccess;
}

void CGraphInheritancePropPage::OnSortByName()  {
   m_Settings->m_Inheritance.m_Sort=CGraphSettings::ByName;
}

void CGraphInheritancePropPage::OnMergeClasses()  {
   m_Settings->m_Inheritance.m_Merge=m_Merge.GetCheck();
	
}
