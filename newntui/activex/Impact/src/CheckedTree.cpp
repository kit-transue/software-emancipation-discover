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
// CheckedTree.cpp : implementation file
//

#include "stdafx.h"
#include "Impact.h"
#include "CheckedTree.h"

#include "custommsgs.h" // user defined messages IDS
#include "ResultsTree.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CCheckedTree

CCheckedTree::CCheckedTree()
{
}

CCheckedTree::~CCheckedTree()
{
}


BEGIN_MESSAGE_MAP(CCheckedTree, CTreeCtrl)
	//{{AFX_MSG_MAP(CCheckedTree)
	ON_WM_CREATE()
	ON_WM_LBUTTONDOWN()
	ON_WM_KEYDOWN()
	ON_NOTIFY_REFLECT(NM_DBLCLK, OnDblclk)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCheckedTree message handlers

int CCheckedTree::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CTreeCtrl::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	m_StateIcons.Create( IDB_STATES, 13, 1, RGB(255,255,255) );
	SetImageList( &m_StateIcons, TVSIL_STATE );
	return 0;
}

void CCheckedTree::OnLButtonDown(UINT nFlags, CPoint point) 
{
	UINT uFlags=0;
	HTREEITEM hti = HitTest(point,&uFlags);

	if(uFlags & TVHT_ONITEMSTATEICON) {
		int iImage = GetItemState( hti, TVIS_STATEIMAGEMASK )>>12;
		if(iImage>0) {
			SetItemState( hti, INDEXTOSTATEIMAGEMASK(iImage == 1 ? 2 : 1), 
						TVIS_STATEIMAGEMASK );
			// notify parent that state has been changed
			CWnd* pParent = GetParent();
			if(pParent!=NULL) pParent->SendMessage(WM_TREEITEMSTATECHANGED,NULL,(LPARAM)hti);
			return;
		}
	}
	
	CTreeCtrl::OnLButtonDown(nFlags, point);
}

void CCheckedTree::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	if(nChar == VK_SPACE) {
		HTREEITEM hti = GetSelectedItem();
		int iImage = GetItemState( hti, TVIS_STATEIMAGEMASK )>>12;
		if(iImage > 0) {
			SetItemState( hti, INDEXTOSTATEIMAGEMASK(iImage == 1 ? 2 : 1),
						TVIS_STATEIMAGEMASK );
			
			// notify parent that state has been changed
			CWnd* pParent = GetParent();
			if(pParent!=NULL) pParent->SendMessage(WM_TREEITEMSTATECHANGED,NULL,(LPARAM)hti);
			return;
		}
	}
	
	CTreeCtrl::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CCheckedTree::OnDblclk(NMHDR* pNMHDR, LRESULT* pResult) 
{
	HTREEITEM hItem = GetSelectedItem();
	TVITEM item;
	item.hItem = hItem;
	item.mask = TVIF_PARAM;
	BOOL bWorked = GetItem(&item);
	if(bWorked) {
		CResultEntity* pEntity = (CResultEntity*)item.lParam;
		if(pEntity!=NULL) {
			CString* pszFile = pEntity->getFile();
			if(pszFile!=NULL && !pszFile->IsEmpty()) {
				// notify parent that state has been changed
				CWnd* pParent = GetParent();
				if(pParent!=NULL) pParent->SendMessage(WM_OPENINSTANCE,NULL,(LPARAM)pEntity);
			}
		}
	}
	*pResult = 0;
}
