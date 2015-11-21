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
// DataChartViewChildFrame.cpp : implementation file
//

#include "stdafx.h"
#include "discovermdi.h"
#include "DataChartViewChildFrame.h"
#include "GraphView.h"
#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDataChartViewChildFrame

IMPLEMENT_DYNCREATE(CDataChartViewChildFrame, CMDIChildWnd)

CDataChartViewChildFrame::CDataChartViewChildFrame()
{
}

CDataChartViewChildFrame::~CDataChartViewChildFrame()
{
}


BEGIN_MESSAGE_MAP(CDataChartViewChildFrame, CMDIChildWnd)
	//{{AFX_MSG_MAP(CDataChartViewChildFrame)
	ON_WM_CLOSE()
	ON_WM_MDIACTIVATE()
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, OnFilePrintPreview)
	ON_COMMAND(ID_FILE_PRINT, OnPrint)
	ON_WM_CREATE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDataChartViewChildFrame message handlers

BOOL CDataChartViewChildFrame::OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext)  {
static CCreateContext newContext;

	newContext.m_pNewViewClass=RUNTIME_CLASS(CGraphView);
    CGraphView* proc = (CGraphView *)CreateView(&newContext);
	m_ClientWnd=proc;
	proc->OnInitialUpdate();
	return TRUE;

	
}

void CDataChartViewChildFrame::OnClose()  {
    CMenu mainMenu;
    mainMenu.LoadMenu(IDR_MAINFRAME);
    GetMDIFrame()->MDISetMenu(&mainMenu,NULL);	
    ((CMainFrame *)GetMDIFrame())->m_wndDataChartView=NULL;	
	CMDIChildWnd::OnClose();
	
}

void CDataChartViewChildFrame::OnMDIActivate(BOOL bActivate, CWnd* pActivateWnd, CWnd* pDeactivateWnd) {
	
   if(bActivate==TRUE) {
      ((CMainFrame *)GetMDIFrame())->SetNewMenu(IDR_DATACHARTVIEW);	
   }
   GetMDIFrame()->ShowControlBar(&m_LocalToolbar,bActivate,TRUE);
   GetMDIFrame()->RecalcLayout();
   GetMDIFrame()->DrawMenuBar();	
	
}
void CDataChartViewChildFrame::OnFilePrintPreview()  {
   m_ClientWnd->SendMessage(WM_COMMAND,ID_FILE_PRINT_PREVIEW,0);
}

void CDataChartViewChildFrame::OnPrint()  {
   m_ClientWnd->SendMessage(WM_COMMAND,ID_FILE_PRINT,0);
	
}

int CDataChartViewChildFrame::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CMDIChildWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	// Creating the dockable toolbar which will be visible only when in the datachart 
	// window
    if (!m_LocalToolbar.Create(GetMDIFrame()) ||
	    !m_LocalToolbar.LoadToolBar(IDR_DATATOOLS)) {
	    TRACE0("Failed to create data chart toolbar\n");
	}

    m_LocalToolbar.SetBarStyle(m_LocalToolbar.GetBarStyle() |
		CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC);
    m_LocalToolbar.EnableDocking(CBRS_ALIGN_ANY);

    GetMDIFrame()->DockControlBar(&m_LocalToolbar);
    GetMDIFrame()->RecalcLayout();
	
	return 0;
}
