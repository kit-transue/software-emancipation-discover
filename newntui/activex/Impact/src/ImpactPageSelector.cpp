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
// ImpactPageSelector.cpp : implementation file
//

#include "stdafx.h"
#include "impact.h"
#include "custommsgs.h"
#include "ImpactPageSelector.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CImpactPageSelector

CImpactPageSelector::CImpactPageSelector()
{
	m_ItemFont.CreatePointFont(80,"Arial");
}

CImpactPageSelector::~CImpactPageSelector()
{
}


BEGIN_MESSAGE_MAP(CImpactPageSelector, CTabCtrl)
	//{{AFX_MSG_MAP(CImpactPageSelector)
	ON_WM_SIZE()
	ON_NOTIFY_REFLECT(TCN_SELCHANGE, OnSelchange)
	ON_MESSAGE( WM_TREEITEMSTATECHANGED, OnItemStateChanged )
	ON_MESSAGE( WM_OPENINSTANCE, OnOpenInstance )
	ON_WM_CREATE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CImpactPageSelector message handlers

void CImpactPageSelector::AddPage(CWnd *pWnd,CString& szTitle,int nImage)
{
	if(pWnd!=NULL) {
		InsertItem(m_Pages.GetCount(),szTitle,nImage);
		m_Pages.AddTail(pWnd);

		CRect rcClient;
		GetClientRect(&rcClient);
		pWnd->MoveWindow(rcClient);
		pWnd->ModifyStyleEx(0,WS_EX_STATICEDGE);
	}
}

void CImpactPageSelector::OnSize(UINT nType, int cx, int cy) 
{
	CTabCtrl::OnSize(nType, cx, cy);
	
	int nPage = 0;
	CRect rcBounds(-2,-4,-3,-3);
	CRect rcItem;
	CRect rcClient;
	GetClientRect(&rcClient);
	POSITION nPos = m_Pages.GetHeadPosition();
	while(nPos!=NULL) {
		GetItemRect(nPage++,&rcItem);
		CWnd* pWnd = m_Pages.GetNext(nPos);
		CRect rcPage(rcClient+rcBounds);
		rcPage.top += rcItem.Height();
		pWnd->MoveWindow(rcPage);
	}
}

void CImpactPageSelector::OnSelchange(NMHDR* pNMHDR, LRESULT* pResult) 
{
	int nPage = GetCurSel();
	POSITION nPos = m_Pages.GetHeadPosition();
	while(nPos!=NULL) {
		CWnd* pWnd = m_Pages.GetNext(nPos);
		pWnd->ShowWindow(SW_HIDE);
	}
	nPos = m_Pages.FindIndex(nPage);
	if(nPos!=NULL) {
		CWnd* pWnd= m_Pages.GetAt(nPos);
		pWnd->ShowWindow(SW_SHOW);
	}
	*pResult = 0;
}

int CImpactPageSelector::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CTabCtrl::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	SetFont(&m_ItemFont);

	return 0;
}

LRESULT CImpactPageSelector::OnItemStateChanged(WPARAM wparm, LPARAM lparm) {
	return GetParent()->SendMessage(WM_TREEITEMSTATECHANGED,wparm,lparm);
}

LRESULT CImpactPageSelector::OnOpenInstance(WPARAM wparm, LPARAM lparm) {
	return GetParent()->SendMessage(WM_OPENINSTANCE,wparm,lparm);
}
