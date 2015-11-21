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
// SubmitInfo.cpp : implementation file
//

#include "stdafx.h"
#include "discovermdi.h"
#include "SubmitInfo.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#define SPACE 5
/////////////////////////////////////////////////////////////////////////////
// CSubmitInfo

IMPLEMENT_DYNCREATE(CSubmitInfo, CFormView)

CSubmitInfo::CSubmitInfo()
	: CFormView(CSubmitInfo::IDD)
{
	//{{AFX_DATA_INIT(CSubmitInfo)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_Updated=FALSE;
}

CSubmitInfo::~CSubmitInfo()
{
}

void CSubmitInfo::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSubmitInfo)
	DDX_Control(pDX, IDC_SELECTOR, m_Selector);
	DDX_Control(pDX, IDC_VIEWER, m_Summary);
	DDX_Control(pDX, IDC_DATASOURCECTRL, m_DataSource);
	DDX_Control(pDX, IDC_REPORTER, m_Detailed);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSubmitInfo, CFormView)
	//{{AFX_MSG_MAP(CSubmitInfo)
	ON_WM_SIZE()
	ON_NOTIFY(TCN_SELCHANGE, IDC_SELECTOR, OnSelchangeSelector)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSubmitInfo diagnostics

#ifdef _DEBUG
void CSubmitInfo::AssertValid() const
{
	CFormView::AssertValid();
}

void CSubmitInfo::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CSubmitInfo message handlers

void CSubmitInfo::LayoutControls(int cx, int cy) {
int titleHeight=25;
   
	if(!::IsWindow(m_Selector))  return; 
	m_Selector.MoveWindow(0,0,cx,titleHeight);
    UpdateHTMLView(cx,cy);
}

void CSubmitInfo::OnInitialUpdate()  {
	CFormView::OnInitialUpdate();
	if(m_Updated==FALSE) {
	    m_Updated=TRUE;
	    TC_ITEM item;
	    item.mask         = TCIF_TEXT;
        item.pszText      = "Summary";     
        item.cchTextMax   = 20;     
	    m_Selector.InsertItem(0,&item);

	    item.mask         = TCIF_TEXT;
        item.pszText      = "Detailed report";     
        item.cchTextMax   = 20;     
	    m_Selector.InsertItem(1,&item);

	    m_InfoFont.CreateFont(14,0,0,0,0,0,0,0,0,0,0,0,0,"Arial");
	    m_Selector.SetFont(&m_InfoFont,FALSE);

        // Generating the complete path to the reports directory+ submisson check report filename.
	    char buf[1024];
		CString reportFileDir;
	    CString reportFileName;
        if(GetEnvironmentVariable("PSETHOME_LOCAL",buf,1024)!=0) {
            reportFileDir=buf;
            reportFileDir+="\\Reports\\";
		} else {
            reportFileDir="C:\\Discover\\Reports\\";
		}

        reportFileName=reportFileDir+"Submission Check Summary.htm";
	    m_Summary.Navigate(reportFileName,NULL,NULL,NULL,NULL);
        reportFileName=reportFileDir+"Submission Check Detailed.htm";
	    m_Detailed.Navigate(reportFileName,NULL,NULL,NULL,NULL);
	}
}

void CSubmitInfo::OnSize(UINT nType, int cx, int cy)  {
	CFormView::OnSize(nType, cx, cy);
    SIZE sizeTotal;
	sizeTotal.cx=0;
	sizeTotal.cy=0;
	SetScrollSizes( MM_TEXT, sizeTotal);
	LayoutControls(cx,cy);
}

void CSubmitInfo::OnSelchangeSelector(NMHDR* pNMHDR, LRESULT* pResult)  {
CRect rect;
    GetClientRect(&rect);
    UpdateHTMLView(rect.Width(),rect.Height());
	*pResult = 0;
}

void CSubmitInfo::UpdateHTMLView(int cx, int cy) {
int titleHeight=25;


	switch(m_Selector.GetCurSel()) {
	   case 0 : m_Summary.SetTop(titleHeight-2);
                m_Summary.SetLeft(-2);
                m_Summary.SetWidth(cx+4);
                m_Summary.SetHeight(cy-titleHeight+4);

                m_Detailed.SetTop(0);
                m_Detailed.SetLeft(0);
                m_Detailed.SetWidth(0);
                m_Detailed.SetHeight(0);
				break;
	   case 1 : m_Detailed.SetTop(titleHeight-2);
                m_Detailed.SetLeft(-2);
                m_Detailed.SetWidth(cx+4);
                m_Detailed.SetHeight(cy-titleHeight+4);

                m_Summary.SetTop(0);
                m_Summary.SetLeft(0);
                m_Summary.SetWidth(0);
                m_Summary.SetHeight(0);
				break;
	}
}



void CSubmitInfo::Update() {

    char buf[1024];
	CString reportFileDir;
    CString reportFileName;
    if(GetEnvironmentVariable("PSETHOME_LOCAL",buf,1024)!=0) {
         reportFileDir=buf;
         reportFileDir+="\\Reports\\";
	} else {
         reportFileDir="C:\\Discover\\Reports\\";
	}

    reportFileName=reportFileDir+"Submission Check Summary.htm";
    m_Summary.Navigate(reportFileName,NULL,NULL,NULL,NULL);

    reportFileName=reportFileDir+"Submission Check Detailed.htm";
    m_Detailed.Navigate(reportFileName,NULL,NULL,NULL,NULL);

    m_Summary.Refresh();
    m_Detailed.Refresh();
}
