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
// EditWithQuit.cpp : implementation file
//

#include "stdafx.h"
#include "Spreadsheet.h"
#include "EditWithQuit.h"
#include "spreadsheetctl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif



/////////////////////////////////////////////////////////////////////////////
// CEditWithQuit

CEditWithQuit::CEditWithQuit() {
	Destination=NULL;
}

CEditWithQuit::~CEditWithQuit()
{
}


BEGIN_MESSAGE_MAP(CEditWithQuit, CEdit)
	//{{AFX_MSG_MAP(CEditWithQuit)
	ON_WM_SETCURSOR()
	ON_WM_CHAR()
	ON_WM_KILLFOCUS()
	ON_WM_GETDLGCODE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEditWithQuit message handlers


BOOL CEditWithQuit::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)  {
	return 1;
}



void CEditWithQuit::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)  {
int x,y;

	if(Destination!=NULL) {
		if(nChar==13) {
			Destination->CloseCellEditor();
			Destination->m_IgnoreCellChange=TRUE;
			Destination->m_InFocusSet=TRUE;
	        Destination->m_GridWindow.SetRow(Destination->m_CurrentRow);
	        Destination->m_GridWindow.SetCol(Destination->m_CurrentColumn);
	        x=Destination->m_GridWindow.GetCellLeft();
	        y=Destination->m_GridWindow.GetCellTop();
            Destination->m_GridWindow.SendMessage(WM_LBUTTONDOWN,MK_LBUTTON,(y<<16)+x);

	        Destination->m_GridWindow.SetRow(Destination->m_CurrentRow);
	        Destination->m_GridWindow.SetCol(Destination->m_CurrentColumn);
	        x=Destination->m_GridWindow.GetCellLeft();
	        y=Destination->m_GridWindow.GetCellTop();
            Destination->m_GridWindow.SendMessage(WM_LBUTTONUP,MK_LBUTTON,(y<<16)+x);
			Destination->m_IgnoreCellChange=FALSE;
			Destination->m_InFocusSet=FALSE;
			return;
		}
	}
	
	CEdit::OnChar(nChar, nRepCnt, nFlags);
}

void CEditWithQuit::OnKillFocus(CWnd* pNewWnd)  {
	if(Destination!=NULL) Destination->CloseCellEditor();
	CEdit::OnKillFocus(pNewWnd);
}


UINT CEditWithQuit::OnGetDlgCode()  {
	return DLGC_WANTALLKEYS;
}
