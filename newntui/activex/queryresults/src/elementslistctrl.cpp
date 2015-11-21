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
// ElementsListCtrl.cpp : implementation file
//

#include "stdafx.h"
#ifndef __QUERYRESULTS_H
   #define __QUERYRESULTS_H
   #include "queryresults.h"
#endif
#include "queryresultsctl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CElementsListCtrl

CElementsListCtrl::CElementsListCtrl()
{
}

CElementsListCtrl::~CElementsListCtrl()
{
}


BEGIN_MESSAGE_MAP(CElementsListCtrl, CListCtrl)
	//{{AFX_MSG_MAP(CElementsListCtrl)
	ON_WM_KEYUP()
	ON_WM_LBUTTONDOWN()
	ON_WM_RBUTTONDOWN()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_KEYDOWN()
	ON_WM_CHAR()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CElementsListCtrl message handlers


void CElementsListCtrl::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags)  {
	if(nChar==40 || nChar==38 || nChar==34 || nChar==33 || nChar==35 || nChar==36 || nChar==37 || nChar==39 ||  nChar==16 ||  nChar==17) {
	    CListCtrl::OnKeyUp(nChar, nRepCnt, nFlags);
	}
	if(nChar!=37 && nChar!=39)
	   GetParent()->PostMessage(WM_USERSELECTION);
}

void CElementsListCtrl::OnLButtonDown(UINT nFlags, CPoint point)  {
	CListCtrl::OnLButtonDown(nFlags, point);
	GetParent()->PostMessage(WM_USERSELECTION);
}

void CElementsListCtrl::OnRButtonDown(UINT nFlags, CPoint point)  {
	CListCtrl::OnRButtonDown(nFlags, point);
	GetParent()->PostMessage(WM_USERSELECTION);
	DWORD pt;
	pt=point.y<<16;
	pt+=point.x;
	GetParent()->PostMessage(WM_RBUTTONDOWN,(WPARAM)nFlags,(LPARAM)pt);
}


//-------------------------------------------------------------------------------------
// This message will be send to the parent ActiveX control to forcr it to 
// fire ElementDoubleclicked event.
//-------------------------------------------------------------------------------------
void CElementsListCtrl::OnLButtonDblClk(UINT nFlags, CPoint point)  {
	CListCtrl::OnRButtonDblClk(nFlags, point);
	DWORD pt;
	pt=point.y<<16;
	pt+=point.x;
	GetParent()->PostMessage(WM_LBUTTONDBLCLK,(WPARAM)nFlags,(LPARAM)pt);
}
//-------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------
// Need this to lock this control incremental search
//-------------------------------------------------------------------------------------
void CElementsListCtrl::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)  {
	if(nChar==40 || nChar==38 || nChar==34 || nChar==33 || nChar==35 || nChar==36 || nChar==37 || nChar==39 ||  nChar==16 ||  nChar==17) {
	    CListCtrl::OnKeyDown(nChar, nRepCnt, nFlags);
	}
	else  {
		GetParent()->PostMessage(WM_FILTERKEY,nChar,nFlags);
	}

}
//-------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------
// Need this to lock this control incremental search
//-------------------------------------------------------------------------------------
void CElementsListCtrl::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)  {
	
	if(nChar==40 || nChar==38 || nChar==34 || nChar==33 || nChar==35 || nChar==36 || nChar==37 || nChar==39 ||  nChar==16 ||  nChar==17) {
	   CListCtrl::OnChar(nChar, nRepCnt, nFlags);
	}
}
//-------------------------------------------------------------------------------------

