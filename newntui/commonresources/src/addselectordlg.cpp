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
// AddSelectorDlg.cpp : implementation file
//

#include "stdafx.h"
#include "cbrowserframe.h"
#include "AddSelectorDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAddSelectorDlg

IMPLEMENT_DYNAMIC(CAddSelectorDlg, CPropertySheet)

CAddSelectorDlg::CAddSelectorDlg(UINT nIDCaption, CWnd* pParentWnd, UINT iSelectPage)
	:CPropertySheet(nIDCaption, pParentWnd, iSelectPage)
{
    AddPage(&m_Groups);
}

CAddSelectorDlg::CAddSelectorDlg(LPCTSTR pszCaption, CWnd* pParentWnd, UINT iSelectPage)
	:CPropertySheet(pszCaption, pParentWnd, iSelectPage)
{
    AddPage(&m_Groups);
}

CAddSelectorDlg::~CAddSelectorDlg()
{
}


BEGIN_MESSAGE_MAP(CAddSelectorDlg, CPropertySheet)
	//{{AFX_MSG_MAP(CAddSelectorDlg)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAddSelectorDlg message handlers

BOOL CAddSelectorDlg::OnInitDialog() 
{
	BOOL bResult = CPropertySheet::OnInitDialog();
	
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
	item.pszText="Groups";
	item.iImage=0;
	tab->SetItem(0,&item);
	
	return bResult;
}
