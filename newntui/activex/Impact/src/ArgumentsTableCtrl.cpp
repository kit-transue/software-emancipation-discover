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
// ArgumentsTableCtrl.cpp : implementation file
//

#include "stdafx.h"
#include "Impact.h"
#include "ArgumentsTableCtrl.h"
#include "InPlaceEdit.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CArgumentsTableCtrl

CArgumentsTableCtrl::CArgumentsTableCtrl()
{
	m_pArgs = NULL;
}

CArgumentsTableCtrl::~CArgumentsTableCtrl()
{
}


BEGIN_MESSAGE_MAP(CArgumentsTableCtrl, CListCtrl)
	//{{AFX_MSG_MAP(CArgumentsTableCtrl)
	ON_WM_CREATE()
	ON_WM_HSCROLL()
	ON_WM_VSCROLL()
	ON_NOTIFY_REFLECT(LVN_ENDLABELEDIT, OnEndlabeledit)
	ON_NOTIFY_REFLECT(LVN_BEGINLABELEDIT, OnBeginlabeledit)
	ON_WM_LBUTTONDBLCLK()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CArgumentsTableCtrl message handlers

CArguments* CArgumentsTableCtrl::GetArguments()
{
	return m_pArgs;
}

void CArgumentsTableCtrl::SetArguments(CArguments *pArgs)
{
	m_pArgs = pArgs;
}

int CArgumentsTableCtrl::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CListCtrl::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	ModifyStyle(0,LVS_REPORT | LVS_EDITLABELS | LVS_NOSORTHEADER | LVS_SINGLESEL);
	ModifyStyleEx(0,WS_EX_STATICEDGE);
	SetExtendedStyle(GetExtendedStyle()|LVS_EX_FULLROWSELECT|LVS_EX_GRIDLINES);
	
	CRect rcClient;
	GetClientRect(&rcClient);
	InsertColumn(0,"Type",LVCFMT_LEFT,rcClient.Width()/2);
	InsertColumn(1,"Value",LVCFMT_LEFT,rcClient.Width()/2);	

	int nRow = 0;
	if(m_pArgs!=NULL) {
		int nCount = m_pArgs->GetCount();
		POSITION nIdx = m_pArgs->GetHeadPosition();
		while(nIdx!=NULL) {
			CArgument* pArg = m_pArgs->GetNext(nIdx);
			InsertItem(nRow,pArg->GetType());
			SetItemText(nRow,1,pArg->GetName());
			nRow++;
		}
	}
	InsertItem(nRow,CString(""));
	SetItemText(nRow,1,CString(""));
	
	return 0;
}

// HitTestEx	- Determine the row index and column index for a point
// Returns	- the row index or -1 if point is not over a row
// point	- point to be tested.
// col		- to hold the column index
int CArgumentsTableCtrl::HitTestEx(CPoint &point, int *col) const
{
	int colnum = 0;
	int row = HitTest( point, NULL );
	
	if( col ) *col = 0;

	// Make sure that the ListView is in LVS_REPORT
	if( (GetWindowLong(m_hWnd, GWL_STYLE) & LVS_TYPEMASK) != LVS_REPORT )
		return row;

	// Get the top and bottom row visible
	row = GetTopIndex();
	int bottom = row + GetCountPerPage();
	if( bottom > GetItemCount() )
		bottom = GetItemCount();
	
	// Get the number of columns
	CHeaderCtrl* pHeader = (CHeaderCtrl*)GetDlgItem(0);
	int nColumnCount = pHeader->GetItemCount();

	// Loop through the visible rows
	for( ;row <=bottom;row++)
	{
		// Get bounding rect of item and check whether point falls in it.
		CRect rect;
		GetItemRect( row, &rect, LVIR_BOUNDS );
		if( rect.PtInRect(point) )
		{
			// Now find the column
			for( colnum = 0; colnum < nColumnCount; colnum++ )
			{
				int colwidth = GetColumnWidth(colnum);
				if( point.x >= rect.left 
					&& point.x <= (rect.left + colwidth ) )
				{
					if( col ) *col = colnum;
					return row;
				}
				rect.left += colwidth;
			}
		}
	}
	return -1;
}

// EditSubLabel		- Start edit of a sub item label
// Returns		- Temporary pointer to the new edit control
// nItem		- The row index of the item to edit
// nCol			- The column of the sub item.
CEdit* CArgumentsTableCtrl::EditSubLabel(int nItem, int nCol)
{
	// The returned pointer should not be saved

	// Make sure that the item is visible
	if( !EnsureVisible( nItem, TRUE ) ) return NULL;

	// Make sure that nCol is valid
	CHeaderCtrl* pHeader = (CHeaderCtrl*)GetDlgItem(0);
	int nColumnCount = pHeader->GetItemCount();
	if( nCol >= nColumnCount || GetColumnWidth(nCol) < 5 )
		return NULL;

	// Get the column offset
	int offset = 0;
	for( int i = 0; i < nCol; i++ )
		offset += GetColumnWidth( i );

	CRect rect;
	GetItemRect( nItem, &rect, LVIR_BOUNDS );

	// Now scroll if we need to expose the column
	CRect rcClient;
	GetClientRect( &rcClient );
	if( offset + rect.left < 0 || offset + rect.left > rcClient.right )
	{
		CSize size;
		size.cx = offset + rect.left;
		size.cy = 0;
		Scroll( size );
		rect.left -= size.cx;
	}

	// Get Column alignment
	LV_COLUMN lvcol;
	lvcol.mask = LVCF_FMT;
	GetColumn( nCol, &lvcol );
	DWORD dwStyle ;
	if((lvcol.fmt&LVCFMT_JUSTIFYMASK) == LVCFMT_LEFT)
		dwStyle = ES_LEFT;
	else if((lvcol.fmt&LVCFMT_JUSTIFYMASK) == LVCFMT_RIGHT)
		dwStyle = ES_RIGHT;
	else dwStyle = ES_CENTER;

	rect.left += offset+4;
	rect.right = rect.left + GetColumnWidth( nCol ) - 3 ;
	if( rect.right > rcClient.right) rect.right = rcClient.right;

	dwStyle |= WS_BORDER|WS_CHILD|WS_VISIBLE|ES_AUTOHSCROLL;
	CEdit *pEdit = new CInPlaceEdit(nItem, nCol, GetItemText( nItem, nCol ));
	pEdit->Create( dwStyle, rect, this, IDC_IPEDIT );

	return pEdit;
}

void CArgumentsTableCtrl::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	if(GetFocus() != this) SetFocus();
	CListCtrl::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CArgumentsTableCtrl::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	if(GetFocus() != this) SetFocus();
	CListCtrl::OnVScroll(nSBCode, nPos, pScrollBar);
}

void CArgumentsTableCtrl::OnEndlabeledit(NMHDR* pNMHDR, LRESULT* pResult) 
{
	LV_DISPINFO* pDispInfo = (LV_DISPINFO*)pNMHDR;
	LV_ITEM	*plvItem = &pDispInfo->item;

	if(plvItem->pszText != NULL) {
		SetItemText(plvItem->iItem, plvItem->iSubItem, plvItem->pszText);
		// check if all items in the row are empty - delete the row
		int nColumns = GetColumnCount();
		for(int i=0;i<nColumns;i++) {
			CString szText = GetItemText(plvItem->iItem,i);
			szText.TrimLeft();
			szText.TrimRight();
			if(szText.GetLength()!=0) break;
		}
		if((i == nColumns) && (GetItemCount() > 1))
			DeleteItem(plvItem->iItem);

		if(m_pArgs != NULL) { // if have arguments - modify them
			POSITION nArgPos = m_pArgs->FindIndex(plvItem->iItem);
			if((i == nColumns) && (GetItemCount() > 1)) {
				if(nArgPos!=NULL)	m_pArgs->RemoveAt(nArgPos);
			} else {  // update arguments data
				int nArgsCount = m_pArgs->GetCount();
				CString szType = GetItemText(plvItem->iItem,0);
				CString szValue = GetItemText(plvItem->iItem,1);
				CArgument* pArg = NULL;
				if(nArgsCount<plvItem->iItem || nArgPos==NULL) {
					pArg = new CArgument(szType,szValue);
					m_pArgs->AddTail(pArg);
				} else {
					pArg = m_pArgs->GetAt(nArgPos);
					pArg->SetName(szValue);
					pArg->SetType(szType);
				}
			}
		}
	}
	*pResult = FALSE;
}

void CArgumentsTableCtrl::OnLButtonDblClk(UINT nFlags, CPoint point) 
//void CArgumentsTableCtrl::OnLButtonDown(UINT nFlags, CPoint point) 
{
	int index;
	CListCtrl::OnLButtonDown(nFlags, point);

	int colnum;
	if((index = HitTestEx(point, &colnum)) != -1){
		UINT flag = LVIS_FOCUSED;
		if((GetItemState( index, flag ) & flag) == flag) {
			// Add check for LVS_EDITLABELS
			if(GetWindowLong(m_hWnd, GWL_STYLE) & LVS_EDITLABELS) {
				LV_DISPINFO dispinfo;
				dispinfo.hdr.hwndFrom = this->GetSafeHwnd();
				dispinfo.hdr.idFrom = GetDlgCtrlID();
				dispinfo.hdr.code = LVN_BEGINLABELEDIT;

				dispinfo.item.iItem = index;
				// Send message to the parent that we are ready to edit.
				SendMessage( WM_NOTIFY, GetDlgCtrlID(),(LPARAM)&dispinfo);

				EditSubLabel(index, colnum);
			}
		} else
			SetItemState(index, LVIS_SELECTED | LVIS_FOCUSED,LVIS_SELECTED | LVIS_FOCUSED); 
	}
}

void CArgumentsTableCtrl::OnBeginlabeledit(NMHDR* pNMHDR, LRESULT* pResult) 
{
	LV_DISPINFO* pDispInfo = (LV_DISPINFO*)pNMHDR;
	LV_ITEM	*plvItem = &pDispInfo->item;

	int nCount = GetItemCount();
	if(plvItem->iItem == nCount-1) { // if it's the last row - add another one
		InsertItem(nCount,CString(""));
		SetItemText(nCount,1,CString(""));
	}
	
	*pResult = 0;
}

// returns number of columns in the list control  - -1 for failure.
int CArgumentsTableCtrl::GetColumnCount()
{
	return GetHeaderCtrl()->GetItemCount();
}
