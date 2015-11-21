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
// SubsystemsPage.cpp : implementation file
//

#include "stdafx.h"
#include "cbrowserframe.h"
#include "GroupsPage.h"


#ifndef __BROWSERFRAMECTRL__
   #define __BROWSERFRAMECTRL__
   #include "CBrowserFrameCtl.h"
#endif

#ifndef __ADDSELECTORDLG__
   #define __ADDSELECTORDLG__
   #include "AddSelectorDlg.h"
#endif

#include "NewSubsystemDialog.h"




#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSubsystemsPage property page

IMPLEMENT_DYNCREATE(CSubsystemsPage, CPropertyPage)

CSubsystemsPage::CSubsystemsPage() : CPropertyPage(CSubsystemsPage::IDD)
{
	//{{AFX_DATA_INIT(CSubsystemsPage)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

CSubsystemsPage::~CSubsystemsPage()
{
}

void CSubsystemsPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSubsystemsPage)
	DDX_Control(pDX, IDC_DATASOURCE, m_DataSource);
	DDX_Control(pDX, IDC_SUBSYSTEMS, m_Subsystems);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSubsystemsPage, CPropertyPage)
	//{{AFX_MSG_MAP(CSubsystemsPage)
	ON_BN_CLICKED(IDC_NEW, OnNew)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSubsystemsPage message handlers

BOOL CSubsystemsPage::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	
	m_Subsystems.SetQuery(CString("ogroup info"));
	m_Subsystems.Update();
	LPDISPATCH lpDispatch;
	m_DataSource.GetControlUnknown()->QueryInterface(IID_IDispatch,(void **)&lpDispatch);
	m_Subsystems.SetDataSource(lpDispatch);

	m_Subsystems.SetFullListName("__SubsystemsSelector__");
	m_Subsystems.SetFilterListName("__SubsystemsSelector__F");
	m_Subsystems.SetSelectionName("__SubsystemsSelector__S");
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CSubsystemsPage::OnNew()  {
CString query;
BSTR sysQuery;
CNewSubsystemDialog subsName;
CString name, oname;

    subsName.m_Name="Untitled";
	if(subsName.DoModal()==IDOK) {
	   name=subsName.m_Name;
	   for(register i=0;i<name.GetLength();i++) { 
		   if( name[i] != '{' && name[i]!= '}' && name[i]!= ' '  && 
			   name[i] != '~' && name[i]!= '@' && name[i]!= '#'  && 
			   name[i] != '$' && name[i]!= '%' && name[i]!= '^'  && 
			   name[i] != '&' && name[i]!= '*' && name[i]!= '('  && 
			   name[i] != ')' && name[i]!= '+' && name[i]!= '\\' && 
			   name[i] != '|' && name[i]!= '=' && name[i]!= '`'  && 
			   name[i] != '<' && name[i]!= '>' && name[i]!= '/') 
			   oname+=name[i];
		   else
			   oname+='_';
	   }
       query.Format("ogroup new %s",oname);
	   sysQuery=query.AllocSysString();
	   DataSourceAccessSync(&sysQuery);
	   m_Subsystems.SetQuery(CString("ogroup info"));
	   m_Subsystems.Update();
	}

}

void CSubsystemsPage::OnOK() {
CString subsystemsListName;
CString elementsListName;
CString query;
BSTR sysQuery;
CAddSelectorDlg* dlg;
CBrowserFrameCtrl * frame;

	dlg=(CAddSelectorDlg *) GetParent();
	if(dlg->GetActiveIndex()!=1) return;
	frame=(CBrowserFrameCtrl *) dlg->GetParent();
	switch(frame->m_ModeID) {
	   case CATEGORIES_MODE : elementsListName=frame->m_SelectionFromCategory; 
		                      break;
	   case QUERIES_MODE    : elementsListName=frame->m_SelectionFromQuery;
		                      break;
	   case GROUPS_MODE     : elementsListName=frame->m_SelectionFromGroup;
		                      break;
	}
    subsystemsListName=m_Subsystems.GetSelectionName();

	query.Format("foreach __subs $%s { ogroup add [name $__subs] $%s }",subsystemsListName,elementsListName);
	sysQuery=query.AllocSysString();
	DataSourceAccessSync(&sysQuery);
	frame->UpdateSubsystems();
	CPropertyPage::OnOK();
}

CString CSubsystemsPage::DataSourceAccessSync(BSTR* command) {
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
