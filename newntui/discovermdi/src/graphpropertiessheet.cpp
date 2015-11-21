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
// GraphPropertiesSheet.cpp : implementation file
//

#include "stdafx.h"
#include "discovermdi.h"
#include "GraphPropertiesSheet.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CGraphPropertiesSheet

IMPLEMENT_DYNAMIC(CGraphPropertiesSheet, CPropertySheet)

CGraphPropertiesSheet::CGraphPropertiesSheet(UINT nIDCaption, CWnd* pParentWnd, UINT iSelectPage)
	:CPropertySheet(nIDCaption, pParentWnd, iSelectPage)
{
/*	AddPage(&m_Layout);
	AddPage(&m_Node);
	AddPage(&m_Edge);
	AddPage(&m_Inheritance);*/
}

CGraphPropertiesSheet::CGraphPropertiesSheet(LPCTSTR pszCaption, CWnd* pParentWnd, UINT iSelectPage)
	:CPropertySheet(pszCaption, pParentWnd, iSelectPage)
{
	/*AddPage(&m_Layout);
	AddPage(&m_Node);
	AddPage(&m_Edge);
	AddPage(&m_Inheritance);*/
}

CGraphPropertiesSheet::~CGraphPropertiesSheet() {
}


BEGIN_MESSAGE_MAP(CGraphPropertiesSheet, CPropertySheet)
	//{{AFX_MSG_MAP(CGraphPropertiesSheet)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGraphPropertiesSheet message handlers

void CGraphPropertiesSheet::SetParameters(CGraphSettings *settings) {
	m_Settings = settings;
	m_Layout.SetGraphSettings(settings);
	m_Node.SetGraphSettings(settings);
	m_Edge.SetSettings(settings);
	m_Inheritance.SetGraphSettings(settings);
	m_Relations.SetGraphSettings(settings);

	AddPage(&m_Layout);
	AddPage(&m_Node);
	AddPage(&m_Edge);
	if(settings->GetViewType()==INHERITANCE || settings->GetViewType()==ERD) {
	    AddPage(&m_Inheritance);
	}
	if(settings->GetViewType()==ERD) {
	    AddPage(&m_Relations);
	}
}
