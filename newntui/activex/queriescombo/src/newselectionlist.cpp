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
// NewSelectionList.cpp : implementation file
//

#include "stdafx.h"
#include "QueriesCombo.h"
#include "NewSelectionList.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CNewSelectionList


CNewSelectionList::CNewSelectionList()
{
}

CNewSelectionList::~CNewSelectionList()
{
}


BEGIN_MESSAGE_MAP(CNewSelectionList, CListCtrl)
	//{{AFX_MSG_MAP(CNewSelectionList)
	ON_WM_KEYUP()
	ON_WM_RBUTTONDOWN()
	ON_WM_LBUTTONDOWN()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CNewSelectionList drawing

/////////////////////////////////////////////////////////////////////////////
// CNewSelectionList diagnostics


/////////////////////////////////////////////////////////////////////////////
// CNewSelectionList message handlers

void CNewSelectionList::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags) {
	CListCtrl::OnKeyUp(nChar, nRepCnt, nFlags);
	GetParent()->PostMessage(WM_USERSELECTION);
}

void CNewSelectionList::OnRButtonDown(UINT nFlags, CPoint point)  {
	CListCtrl::OnRButtonDown(nFlags, point);
	GetParent()->PostMessage(WM_USERSELECTION);
}

void CNewSelectionList::OnLButtonDown(UINT nFlags, CPoint point) {
	
	CListCtrl::OnLButtonDown(nFlags, point);
	GetParent()->PostMessage(WM_USERSELECTION);
}
