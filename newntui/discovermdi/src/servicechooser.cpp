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
// ServiceChooser.cpp : implementation file
//

#include "stdafx.h"
#include "discovermdi.h"
#include "ServiceChooser.h"
#include "nameServCalls.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CServiceChooser dialog


CServiceChooser::CServiceChooser(CWnd* pParent /*=NULL*/)
	: CDialog(CServiceChooser::IDD, pParent)
{
	//{{AFX_DATA_INIT(CServiceChooser)
	m_ServiceName = _T("");
	//}}AFX_DATA_INIT
}


void CServiceChooser::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CServiceChooser)
	DDX_Control(pDX, IDC_SERVICELIST, m_ServiceList);
	DDX_LBString(pDX, IDC_SERVICELIST, m_ServiceName);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CServiceChooser, CDialog)
	//{{AFX_MSG_MAP(CServiceChooser)
	ON_LBN_DBLCLK(IDC_SERVICELIST, OnDblclkServicelist)
	ON_LBN_SELCHANGE(IDC_SERVICELIST, OnSelchangeServicelist)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CServiceChooser message handlers

BOOL CServiceChooser::OnInitDialog()  {
	CDialog::OnInitDialog();
	FillServiceList();
	return TRUE;  
}


//--------------------------------------------------------------------------------
// This function will ask nameserver for all available pset servers.
//--------------------------------------------------------------------------------
void CServiceChooser::FillServiceList() {
NameServerServiceList* services;
CString name;
const char* strPtr;
char host[1024];
CString templ;

    // All valid services must start as "Discover:<host>"
	gethostname(host,1024);
    templ="Discover:";
	templ+=host;
    templ+=":";
    m_ServiceList.ResetContent();
    NameServerGetListOfServices(&services);
	NameServerServiceList* pTemp = services;
	while(pTemp!=NULL) {
		strPtr = pTemp->getName();
		if(strPtr!=NULL) {
			name=strPtr;
			if(name.Find(templ)==0)        
				 m_ServiceList.AddString(name);
		}
		pTemp=pTemp->getNext();
	}
	delete services;
}
//---------------------------------------------------------------------------------------

void CServiceChooser::OnDblclkServicelist() {
	OnOK();
}

void CServiceChooser::OnOK()  {
	CDialog::OnOK();
}

void CServiceChooser::OnSelchangeServicelist()  {
CString service;
int i;

	int sel=m_ServiceList.GetCurSel();
	if(sel>=0) {
		m_ServiceList.GetText(sel,service);
	    int service_port;
	    int tcp_addr;
		HCURSOR save = SetCursor(LoadCursor(NULL, IDC_WAIT));
		for(i=0;i<10;i++) {
		   if(NameServerGetService(service,service_port,tcp_addr)==1) break;
		}
		SetCursor(save);
		if(i==10) {
		   AfxMessageBox("Selected service is no longer available\n and will be removed from the service list.");
           FillServiceList();	
		} 
	}
}
