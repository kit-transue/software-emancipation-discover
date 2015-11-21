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
// StoragePropPage.cpp : implementation file
//

#include "stdafx.h"
#include "pdf_wizard.h"
#include "pdfwiz_globals.h"
#include "StoragePropPage.h"
#include "PW_propSheet.h"

#include <fstream>
using namespace std;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CStoragePropPage property page

IMPLEMENT_DYNCREATE(CStoragePropPage, CPropertyPage)

CStoragePropPage::CStoragePropPage() : CPropertyPage(CStoragePropPage::IDD), m_prefsroot(0)
{
	//{{AFX_DATA_INIT(CStoragePropPage)
	m_project_name = _T("");
	m_admindir = _T("");
	//}}AFX_DATA_INIT
}

CStoragePropPage::~CStoragePropPage()
{
	if (m_prefsroot != 0)
		delete m_prefsroot;

}

void CStoragePropPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CStoragePropPage)
	DDX_Text(pDX, IDC_PROJECT_NAME, m_project_name);
	DDX_Text(pDX, IDC_ADMINDIR, m_admindir);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CStoragePropPage, CPropertyPage)
	//{{AFX_MSG_MAP(CStoragePropPage)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CStoragePropPage message handlers


BOOL CStoragePropPage::OnSetActive() 
{
	// all pdf creation done on this page--we can keep *project_ptrs private!

	// avoid memory leaks, since we're doing this blindly:
	if (m_prefsroot != 0)
			delete m_prefsroot;

	// create project
	// get target from previous dialog:
	CPW_propSheet *pSheet = STATIC_DOWNCAST(CPW_propSheet, GetParent());
	CString cstarget;
	pSheet->m_targetsPage.m_target_combo.GetWindowText(cstarget);
	string target(cstarget);
	m_prefsroot = &prefs; //new prefsroot(*makefile::globalmakefile, &target);

	// default admindir location:
	m_admindir = m_prefsroot->adminDir().c_str();

	m_project_name = m_prefsroot->projectName().c_str();
	// and try and make these changes visible:
	UpdateData(FALSE);
	return CPropertyPage::OnSetActive();
}

LRESULT CStoragePropPage::OnWizardNext() 
{
	// transfer data from widgets to member variables:
	UpdateData(TRUE);

	// write pdf
	m_prefsroot->setProjectName(string(m_project_name));
	m_prefsroot->setAdminDir(string(m_admindir));
	m_prefsroot->write();

	// cleanup
//	delete m_prefsroot;
	m_prefsroot = 0;
	
	return CPropertyPage::OnWizardNext();
}
