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
// FilterSheet.cpp : implementation file
//

#include "stdafx.h"
#include "FilterSheet.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// FilterSheet

IMPLEMENT_DYNAMIC(FilterSheet, CPropertySheet)

FilterSheet::FilterSheet(UINT nIDCaption, CWnd* pParentWnd, UINT iSelectPage)
	:CPropertySheet(nIDCaption, pParentWnd, iSelectPage)
{
    AddPage(&m_FilterList);
	AddPage(&m_FilterWizard);
}

FilterSheet::FilterSheet(LPCTSTR pszCaption, CWnd* pParentWnd, UINT iSelectPage)
	:CPropertySheet(pszCaption, pParentWnd, iSelectPage)
{
	AddPage(&m_FilterList);
	AddPage(&m_FilterWizard);
}

FilterSheet::~FilterSheet()
{
}


BEGIN_MESSAGE_MAP(FilterSheet, CPropertySheet)
	//{{AFX_MSG_MAP(FilterSheet)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// FilterSheet message handlers


BOOL FilterSheet::OnInitDialog() {

    BOOL result = CPropertySheet::OnInitDialog();

	    CTabCtrl* tab = GetTabControl();
    m_ImageList.Create(16,16,TRUE,2,0);
	CBitmap listBitmap;
	listBitmap.LoadBitmap(IDB_FILTERLIST);
	m_ImageList.Add(&listBitmap,RGB(0,0,0));
	CBitmap wizardBitmap;
	wizardBitmap.LoadBitmap(IDB_FILTERWIZARD);
	m_ImageList.Add(&wizardBitmap,RGB(0,0,0));
	tab->SetImageList(&m_ImageList);
    TC_ITEM item;
	item.mask= TCIF_TEXT | TCIF_IMAGE;
	item.pszText="Saved Filters";
	item.iImage=0;
	tab->SetItem(0,&item);
	item.pszText="Filter Wizard";
	item.iImage=1;
	tab->SetItem(1,&item);
	return result;
}

void FilterSheet::SetApplyTaget(CQueryResults* workWith) {
	m_QueryResults=workWith;
}



LRESULT FilterSheet::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
	// TODO: Add your specialized code here and/or call the base class
	return CPropertySheet::WindowProc(message, wParam, lParam);
}

