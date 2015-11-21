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
// AccessWnd.cpp : implementation file
//

#include "stdafx.h"
#include "discovermdi.h"
#include "AccessWnd.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAccessWnd

IMPLEMENT_DYNCREATE(CAccessWnd, CFormView)

CAccessWnd::CAccessWnd()
	: CFormView(CAccessWnd::IDD)
{
	//{{AFX_DATA_INIT(CAccessWnd)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

CAccessWnd::~CAccessWnd()
{
}

void CAccessWnd::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAccessWnd)
	DDX_Control(pDX, IDC_FONT, m_Font);
	DDX_Control(pDX, IDC_EDIT, m_Edit);
	DDX_Control(pDX, IDC_STOP, m_Stop);
	DDX_Control(pDX, IDC_PREVCMD, m_Prev);
	DDX_Control(pDX, IDC_PASTESEL, m_Paste);
	DDX_Control(pDX, IDC_NEXTCMD, m_Next);
	DDX_Control(pDX, IDC_COPYSEL, m_Copy);
	DDX_Control(pDX, IDC_DATASOURCE, m_DataSource);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CAccessWnd, CFormView)
	//{{AFX_MSG_MAP(CAccessWnd)
	ON_WM_SIZE()
	ON_COMMAND(IDC_RETURNPRESSED, OnReturnPressed)
	ON_COMMAND(IDC_STATUSCHANGED, OnStatusChanged)
	ON_BN_CLICKED(IDC_COPYSEL, OnCopysel)
	ON_BN_CLICKED(IDC_PASTESEL, OnPastesel)
	ON_BN_CLICKED(IDC_PREVCMD, OnPrevcmd)
	ON_BN_CLICKED(IDC_NEXTCMD, OnNextcmd)
	ON_BN_CLICKED(IDC_FONT, OnFont)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()
#define HSPACE 1
#define WSPACE 1
#define BSPACE 1
#define SEPCPACE 5

/////////////////////////////////////////////////////////////////////////////
// CAccessWnd diagnostics

#ifdef _DEBUG
void CAccessWnd::AssertValid() const
{
	CFormView::AssertValid();
}

void CAccessWnd::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CAccessWnd message handlers

void CAccessWnd::OnSize(UINT nType, int cx, int cy)  {
	CFormView::OnSize(nType, cx, cy);
	LayoutControls(cx,cy);
}

void CAccessWnd::OnInitialUpdate()  {
	CFormView::OnInitialUpdate();
    SIZE sizeTotal;
	sizeTotal.cx=0;
	sizeTotal.cy=0;
	SetScrollSizes( MM_TEXT, sizeTotal);
	CRect wRect;
	GetWindowRect(wRect);
	LayoutControls(wRect.Width(),wRect.Height());
    CDiscoverMDIApp* app = (CDiscoverMDIApp *) AfxGetApp();
	//m_DataSource.SetSource(app->m_ServiceName,FALSE);
	m_Edit.LimitText(UINT_MAX);
	m_Edit.InsertResults(CString("% \n"));

	m_ArLeft.LoadMappedBitmap(IDB_ARLEFT);
	m_Prev.SetBitmap(m_ArLeft);
	m_ArRight.LoadMappedBitmap(IDB_ARRIGHT);
	m_Next.SetBitmap(m_ArRight);

	m_ImgCopy.LoadMappedBitmap(IDB_COPY);
	m_Copy.SetBitmap(m_ImgCopy);

	m_ImgPaste.LoadMappedBitmap(IDB_PASTE);
	m_Paste.SetBitmap(m_ImgPaste);

	m_ImgStop.LoadMappedBitmap(IDB_STOP);
	m_Stop.SetBitmap(m_ImgStop);

	m_ImgFonts.LoadMappedBitmap(IDB_FONTS);
	m_Font.SetBitmap(m_ImgFonts);

	UpdateButtonsStatus();

    CString settingsPath;
	LOGFONT wndFont;
	char buf[1024];
    if(GetEnvironmentVariable("PSETHOME_LOCAL",buf,1024)!=0) {
         settingsPath=buf;
         settingsPath+="/Settings/";
	} else {
        settingsPath="C:/Discover/Settings/";
	}
    settingsPath+="access.dat";
    CFile file;
	if(file.Open(settingsPath,CFile::modeRead)==TRUE) {
        if(file.Read(&wndFont,sizeof(wndFont))>0) {
	        m_Edit.UseFont(&wndFont);
		}
	}
}

void CAccessWnd::LayoutControls(int cx, int cy) {
    if(!::IsWindow(m_Prev)) return;
	m_Prev.MoveWindow(HSPACE,WSPACE,28,28);
	m_Next.MoveWindow(WSPACE+28+BSPACE,WSPACE,28,28);
	m_Copy.MoveWindow(WSPACE+2*(28+BSPACE)+SEPCPACE,WSPACE,28,28);
	m_Paste.MoveWindow(WSPACE+3*(28+BSPACE)+SEPCPACE,WSPACE,28,28);
	m_Stop.MoveWindow(WSPACE+4*(28+BSPACE)+2*SEPCPACE,WSPACE,28,28);
	m_Font.MoveWindow(WSPACE+5*(28+BSPACE)+3*SEPCPACE,WSPACE,28,28);
    m_Edit.MoveWindow(0,2*HSPACE+28,cx-1,cy-(2*HSPACE+28)-1);

}

void CAccessWnd::OnReturnPressed()  {
CString query;
CString output;
BSTR sysQuery;
static BOOL bCommDlgShown = FALSE;
    query=m_Edit.GetCommand();
    sysQuery=query.AllocSysString();
    output=m_DataSource.AccessDish(&sysQuery);
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
	SysFreeString(sysQuery);
	m_Edit.InsertResults(output);
}

void CAccessWnd::OnStatusChanged()  {
	UpdateButtonsStatus();
}


void CAccessWnd::OnCopysel()  {
	m_Edit.ToClipboard();
}

void CAccessWnd::OnPastesel()  {
	m_Edit.FromClipboard();
}

void CAccessWnd::OnPrevcmd()  {
	m_Edit.GoPrevCommand();
}

void CAccessWnd::OnNextcmd()  {
	m_Edit.GoNextCommand();
}

void CAccessWnd::UpdateButtonsStatus() {
	unsigned int mask = m_Edit.GetEditorStatus();
	m_Copy.EnableWindow((mask & ACCESS_CAN_COPY)!=0); 
	m_Paste.EnableWindow((mask & ACCESS_CAN_PASTE)!=0); 
	m_Prev.EnableWindow((mask & ACCESS_HAS_PREV)!=0); 
	m_Next.EnableWindow((mask & ACCESS_HAS_NEXT)!=0); 
	m_Stop.EnableWindow((mask & ACCESS_CAN_STOP)!=0); 
}

void CAccessWnd::OnFont() {
CFont* oldFont;
LOGFONT oldDescr;
LOGFONT wndFont;
CFontDialog dlg; 
char buf[1024];

    oldFont=m_Edit.GetFont();
	oldFont->GetLogFont(&oldDescr);
	dlg.m_cf.lpLogFont=&oldDescr;
    dlg.m_cf.Flags |= CF_INITTOLOGFONTSTRUCT;
    dlg.m_cf.Flags &= ~CF_EFFECTS;
    if(dlg.DoModal()==IDOK) {
		dlg.GetCurrentFont(&wndFont);
		m_Edit.UseFont(&wndFont);

        CString settingsPath;
        if(GetEnvironmentVariable("PSETHOME_LOCAL",buf,1024)!=0) {
            settingsPath=buf;
            settingsPath+="/Settings/";
		} else {
            settingsPath="C:/Discover/Settings/";
		}
        settingsPath+="access.dat";
	    CFile file(settingsPath,CFile::modeCreate | CFile::modeWrite);
		file.Write(&wndFont,sizeof(wndFont));

	}
}
