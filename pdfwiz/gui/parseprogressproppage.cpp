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
// ParseProgressPropPage.cpp : implementation file
//

#include "stdafx.h"
#include "pdf_wizard.h"
#include "ParseProgressPropPage.h"
#include "parsemake_if.h"
#include "usermessage.h"

#include "make_toplevel.h"
#include "pdfwiz_globals.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CParseProgressPropPage property page

IMPLEMENT_DYNCREATE(CParseProgressPropPage, CPropertyPage)

CParseProgressPropPage::CParseProgressPropPage() : CPropertyPage(CParseProgressPropPage::IDD)
{
	//{{AFX_DATA_INIT(CParseProgressPropPage)
	m_pending_counter = _T("");
	//}}AFX_DATA_INIT
	m_timer = 0;
}

CParseProgressPropPage::~CParseProgressPropPage()
{
	if (m_timer != 0)
		KillTimer(m_timer);
}

void CParseProgressPropPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CParseProgressPropPage)
	DDX_Control(pDX, IDC_PARSE_PROGRESS, m_progress_bar);
	DDX_Text(pDX, IDC_PENDING_COUNTER, m_pending_counter);
	DDV_MaxChars(pDX, m_pending_counter, 12);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CParseProgressPropPage, CPropertyPage)
	//{{AFX_MSG_MAP(CParseProgressPropPage)
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_THREADFINISHED, OnThreadFinished)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CParseProgressPropPage message handlers

LRESULT CParseProgressPropPage::OnThreadFinished(WPARAM wParam, LPARAM lParam)
{
	KillTimer(m_timer);
	m_timer = 0;
	
	CPropertySheet* pSheet = STATIC_DOWNCAST(CPropertySheet, GetParent());
	
	// re-enable buttons
	pSheet->SetWizardButtons(PSWIZB_NEXT);
	
	pSheet->PressButton(PSBTN_NEXT);
	
	return 0;
}

BOOL CParseProgressPropPage::OnSetActive() 
{
	// disable buttons
	CPropertySheet *pSheet = STATIC_DOWNCAST(CPropertySheet, GetParent());
	pSheet->SetWizardButtons(0);  // all buttons disabled?!

	m_timer = SetTimer(1, 500, NULL);	// every half second
	CWinThread * pThread =
		AfxBeginThread(parsemake_thread_proc, GetSafeHwnd(), THREAD_PRIORITY_NORMAL);	
	return CPropertyPage::OnSetActive();
}

void CParseProgressPropPage::OnTimer(UINT nIDEvent) 
{
	if (makefile::globalmakefile) {
		m_progress_bar.SetRange(0, makefile::globalmakefile->progress_totalchars);
		m_progress_bar.SetPos(makefile::globalmakefile->progress_currentchar);
	}
	m_pending_counter.Format("%d", pendingmakeactions.size());
	UpdateData(FALSE);
}
