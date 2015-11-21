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
// SimplifySelection.cpp : implementation file
//

#include "stdafx.h"
#include "discovermdi.h"
#include "SimplifySelection.h"
#include "SimplifyTree.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSimplifySelection

IMPLEMENT_DYNCREATE(CSimplifySelection, CFormView)

CSimplifySelection::CSimplifySelection()
	: CFormView(CSimplifySelection::IDD)
{
	//{{AFX_DATA_INIT(CSimplifySelection)
	//}}AFX_DATA_INIT
	m_SelectedItem=-1;
}

CSimplifySelection::~CSimplifySelection()
{
}

void CSimplifySelection::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSimplifySelection)
	DDX_Control(pDX, IDC_LIST, m_List);
	DDX_Control(pDX, IDC_DATASOURCE, m_DataSource);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSimplifySelection, CFormView)
	//{{AFX_MSG_MAP(CSimplifySelection)
	ON_WM_SIZE()
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST, OnItemChanged)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSimplifySelection diagnostics

#ifdef _DEBUG
void CSimplifySelection::AssertValid() const
{
	CFormView::AssertValid();
}

void CSimplifySelection::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CSimplifySelection message handlers

void CSimplifySelection::OnInitialUpdate() 
{
	CFormView::OnInitialUpdate();

	// We need no scrollbars in this view
    SIZE sizeTotal;
	sizeTotal.cx=0;
	sizeTotal.cy=0;
	SetScrollSizes( MM_TEXT, sizeTotal);
	CBitmap bmp1;
	bmp1.LoadBitmap(IDB_SIMPLIFY_UNPROCESSED);
	CBitmap bmp2;
	bmp2.LoadBitmap(IDB_SIMPLIFY_VISITED);
	CBitmap bmp3;
	bmp3.LoadBitmap(IDB_SIMPLIFY_UPDATED);
    m_Images.Create(16,16,TRUE,3,0);

	m_Images.Add(&bmp1,GetSysColor(COLOR_BTNFACE));
	m_Images.Add(&bmp2,GetSysColor(COLOR_BTNFACE));
	m_Images.Add(&bmp3,GetSysColor(COLOR_BTNFACE));

	m_List.SetImageList(&m_Images,LVSIL_SMALL );


    m_List.InsertColumn(0,"Source file",LVCFMT_LEFT,150);
	m_List.InsertColumn(1,"Status",LVCFMT_LEFT,70);
	FillSelectionList();
	
}

void CSimplifySelection::OnSize(UINT nType, int cx, int cy) 
{
	CFormView::OnSize(nType, cx, cy);
	if(::IsWindow(m_List)) m_List.MoveWindow(0,0,cx,cy);
	
	
}


void CSimplifySelection::FillSelectionList() {
CString str;
CString query;
BSTR sysQuery;
int selectionSize;

    query = "set SimplifySelection [filter {language==\"CPP\" || language==\"C\"} $DiscoverSelection]";
    sysQuery=query.AllocSysString();
    str=DataSourceAccessSync(&sysQuery);
	SysFreeString(sysQuery); 


    // Calculationg selection size
    query = "size $SimplifySelection";
    sysQuery=query.AllocSysString();
    str=DataSourceAccessSync(&sysQuery);
	SysFreeString(sysQuery); 
    selectionSize = atoi(str);

	for(int i=0;i<selectionSize;i++) {
        query.Format("lname $SimplifySelection %d",i+1);
        sysQuery=query.AllocSysString();
        str=DataSourceAccessSync(&sysQuery);
	    SysFreeString(sysQuery); 
		m_List.InsertItem(i,str,0);
		m_List.SetItemText(i,1,"Unprocessed");
	}

    query = "unset SimplifySelection";
    sysQuery=query.AllocSysString();
    str=DataSourceAccessSync(&sysQuery);
	SysFreeString(sysQuery); 
}

void CSimplifySelection::OnItemChanged(NMHDR* pNMHDR, LRESULT* pResult) {
NM_LISTVIEW *pInfo = (NM_LISTVIEW *)pNMHDR;
CSimplifyTree* view=(CSimplifyTree* )((CSplitterWnd *) (GetParent()->GetParent()))->GetPane(0,0);

    if(pInfo->iItem>=0) {
	   m_SelectedItem = pInfo->iItem;
       CString filename;
	   filename = m_List.GetItemText(pInfo->iItem,0);

	   CString query;
	   BSTR sysQuery;
       query.Format("fname {%s}",filename);
       sysQuery=query.AllocSysString();
       CString path=DataSourceAccessSync(&sysQuery);
	   SysFreeString(sysQuery);
       if((GetFileAttributes(path)&FILE_ATTRIBUTE_READONLY)!=0) 
		   MessageBox("You are trying to simplify read-only file.\n You will be unable to change it.","Simplify warning",MB_ICONINFORMATION|MB_OK);
       view->SetNewElement(pInfo->iItem,filename);
	}
}

//-----------------------------------------------------------------------------------------
// This function will be called from the simplify tree view to update item status.
// Possible valuse of the status are :
//  0 - item was not processed
//  1 - item was processed, file was not updated
//  2 - item was processed, file was updated.
//------------------------------------------------------------------------------------------
void CSimplifySelection::SetItemStatus(int item, int status) {
CString text;
    if(item<0) return;
	m_List.SetItem( item,0,LVIF_IMAGE,"", status,0,0,0);
	switch(status) {
	    case 0 :m_List.SetItemText(item,1,"Unprocessed");break;
		case 1 :m_List.SetItemText(item,1,"Visited");    break;
		case 2 :m_List.SetItemText(item,1,"Updated");    break;
	}
}
//------------------------------------------------------------------------------------------
// This function check communications status after access datasource.
//------------------------------------------------------------------------------------------
CString CSimplifySelection::DataSourceAccessSync(BSTR* command) {
	CString res;
	static BOOL bCommDlgShown = FALSE;
	res = m_DataSource.AccessSync(command);
	if (m_DataSource.IsConnectionLost() == TRUE &&
		bCommDlgShown != TRUE ) {
		::MessageBox(m_hWnd, 
			   _T("Connection with server is lost."
			   " Make sure the server is running."),
			   _T("Server Communication Error."), 
			   MB_OK | MB_ICONINFORMATION); 
		bCommDlgShown = TRUE;
	} else if (m_DataSource.IsConnectionLost() != TRUE) {
		bCommDlgShown = FALSE;
	}	
	return res;
}

bool CSimplifySelection::IsEmpty()
{
	return m_List.GetItemCount()==0;
}
