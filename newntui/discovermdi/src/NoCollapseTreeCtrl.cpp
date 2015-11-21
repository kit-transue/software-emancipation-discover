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
// NoCollapseTreeCtrl.cpp : implementation file
//

#include "stdafx.h"
#include "discovermdi.h"
#include "NoCollapseTreeCtrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CNoCollapseTreeCtrl

CNoCollapseTreeCtrl::CNoCollapseTreeCtrl()
{
	m_pImages = NULL;
}

CNoCollapseTreeCtrl::~CNoCollapseTreeCtrl()
{
	if(m_pImages) delete m_pImages;
}


BEGIN_MESSAGE_MAP(CNoCollapseTreeCtrl, CTreeCtrl)
	//{{AFX_MSG_MAP(CNoCollapseTreeCtrl)
	ON_NOTIFY_REFLECT(TVN_ITEMEXPANDING, OnItemExpanding)
	ON_NOTIFY_REFLECT(NM_DBLCLK, OnDblclk)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CNoCollapseTreeCtrl message handlers

void CNoCollapseTreeCtrl::OnItemExpanding(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;
	// TODO: Add your control notification handler code here
	if(pNMTreeView->action == TVE_COLLAPSE)
		*pResult = 1;
	else
		*pResult = 0;
}

	
void CNoCollapseTreeCtrl::CreateImages()
{
	m_pImages = new CImageList();
	m_pImages->Create(IDB_MODELIMAGES,16,1,RGB(255,255,255));
	SetImageList(m_pImages,TVSIL_NORMAL);
}

void CNoCollapseTreeCtrl::OnDblclk(NMHDR* pNMHDR, LRESULT* pResult) 
{
	CWnd* pParent = GetParent();
	if(pParent!=NULL)
		pParent->PostMessage(WM_BROWSEMODEL);
	*pResult = 0;
}
