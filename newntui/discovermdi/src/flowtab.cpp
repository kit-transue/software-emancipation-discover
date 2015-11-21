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
// FlowTab.cpp : implementation file
//

#include "stdafx.h"
#include "discovermdi.h"
#include "FlowTab.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CFlowTab

CFlowTab::CFlowTab()
{
}

CFlowTab::~CFlowTab()
{
}


BEGIN_MESSAGE_MAP(CFlowTab, CTabCtrl)
	//{{AFX_MSG_MAP(CFlowTab)
	ON_WM_LBUTTONDOWN()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFlowTab message handlers

void CFlowTab::OnLButtonDown(UINT nFlags, CPoint point)  {
int* elements;
POINT p;
int item;
TC_HITTESTINFO test;

    p.x=point.x;
    p.y=point.y;
    test.pt.x=p.x;
    test.pt.y=p.y;
    test.flags=TCHT_NOWHERE;
    item=HitTest(&test);
	if(item==-1) return;
	SetCurSel(item);

    // Register or get clipboard format for the task flow node
	UINT formatID=RegisterClipboardFormat("DiscoverTaskFlowNode");
	// Alocate memory for all selected elements indexes.
	// The DragAndDrop Target will receve this array
    HGLOBAL dataID = GlobalAlloc(GMEM_FIXED,3*sizeof(int)); 
	elements = (int *)::GlobalLock(dataID);
    // Fill an array with the selected elements
    elements[0]=OP_MOVE;
    elements[1]=1;
    elements[2]=item;
	GlobalUnlock(dataID);
    m_DropSource.CacheGlobalData(formatID, dataID);
	m_DropSource.DoDragDrop(); 


	NMHDR info;
    info.hwndFrom=*this; 
    info.idFrom=GetWindowLong(*this,GWL_ID); 
    info.code=TCN_SELCHANGE; 
	SetFocus();
	GetParent()->SendMessage(WM_NOTIFY,(WPARAM)GetWindowLong(*this,GWL_ID),(LPARAM)&info);
}

