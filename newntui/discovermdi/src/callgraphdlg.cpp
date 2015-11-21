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
// CallGraphDlg.cpp : implementation file
//

#include "stdafx.h"
#include "discovermdi.h"
#include "CallGraphDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CCallGraphDlg dialog


CCallGraphDlg::CCallGraphDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CCallGraphDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CCallGraphDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CCallGraphDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCallGraphDlg)
	DDX_Control(pDX, IDC_TO_TARGET, m_ToTargetBtn);
	DDX_Control(pDX, IDC_TO_SOURCE, m_ToSourceBtn);
	DDX_Control(pDX, IDC_DATASOURCE, m_DataSource);
	DDX_Control(pDX, IDC_SOURCE, m_Source);
	DDX_Control(pDX, IDC_TARGET, m_Target);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CCallGraphDlg, CDialog)
	//{{AFX_MSG_MAP(CCallGraphDlg)
	ON_BN_CLICKED(IDC_TO_SOURCE, OnMoveToSource)
	ON_BN_CLICKED(IDC_TO_TARGET, OnMoveToTarget)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCallGraphDlg message handlers

void CCallGraphDlg::OnMoveToSource()  {
CString query;
BSTR sysQuery;


   query.Format("sappend $__GraphSource__ $__GraphTarget__S");
   sysQuery=query.AllocSysString();
   DataSourceAccessSync(&sysQuery);
   ::SysFreeString(sysQuery);

   m_Source.SetQuery("");
   m_Source.Update();
   m_Target.SetQuery("set_subtract $__GraphTarget__ $__GraphTarget__S");
   m_Target.Update();
   m_Target.SetQuery("");
}

void CCallGraphDlg::OnMoveToTarget()  {
CString query;
BSTR sysQuery;



   query="sappend $__GraphTarget__ $__GraphSource__S";
   sysQuery=query.AllocSysString();
   DataSourceAccessSync(&sysQuery);
   ::SysFreeString(sysQuery);
   m_Target.SetQuery("");
   m_Target.Update();
   m_Source.SetQuery("set_subtract $__GraphSource__ $__GraphSource__S");
   m_Source.Update();
   m_Source.SetQuery("");

}

BOOL CCallGraphDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

	LPDISPATCH lpDispatch;
	m_DataSource.GetControlUnknown()->QueryInterface(IID_IDispatch,(void **)&lpDispatch);
	m_Source.SetDataSource(lpDispatch);
	m_Target.SetDataSource(lpDispatch);
	
	m_Source.SetFullListName("__GraphSource__");
	m_Source.SetFilterListName("__GraphSource__F");
	m_Source.SetSelectionName("__GraphSource__S");

	m_Target.SetFullListName("__GraphTarget__");
	m_Target.SetFilterListName("__GraphTarget__F");
	m_Target.SetSelectionName("__GraphTarget__S");

    CString query;
    BSTR sysQuery;
    query="set __GraphSource__ [filter kind==\"funct\" $DiscoverSelection]";
    sysQuery=query.AllocSysString();
    DataSourceAccessSync(&sysQuery);
	::SysFreeString(sysQuery);

    query="set __GraphTarget__ [nil_set]";
    sysQuery=query.AllocSysString();
    DataSourceAccessSync(&sysQuery);
	::SysFreeString(sysQuery);

    m_Source.SetQuery("");
	m_Source.Update();
    m_Target.SetQuery("");
	m_Target.Update();
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CCallGraphDlg::OnOK()  {

	if(MessageBox("Call graph relations calculation can take significant time.\n Do you want to continue?","Call graph", MB_ICONQUESTION | MB_YESNO)==IDYES) {
        CString query;
        BSTR sysQuery;
        query="call_graph $__GraphSource__ $__GraphTarget__";
        sysQuery=query.AllocSysString();
        DataSourceAccessSync(&sysQuery);
	    ::SysFreeString(sysQuery);
	}
	CDialog::OnOK();
}

CString CCallGraphDlg::DataSourceAccessSync(BSTR* command) {
	CString res;
	static BOOL bCommDlgShown = FALSE;
	res = m_DataSource.AccessSync(command);
	if (m_DataSource.IsConnectionLost() == TRUE &&
		bCommDlgShown != TRUE) {
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
