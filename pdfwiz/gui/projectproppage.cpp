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
// ProjectPropPage.cpp : implementation file
//

#include "stdafx.h"
#include "pdf_wizard.h"
#include "ProjectPropPage.h"
#include "ide_interface.h"
#include "make_toplevel.h"
#include "pdfwiz_globals.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CProjectPropPage property page

IMPLEMENT_DYNCREATE(CProjectPropPage, CPropertyPage)

CProjectPropPage::CProjectPropPage() : CPropertyPage(CProjectPropPage::IDD)
{
	//{{AFX_DATA_INIT(CProjectPropPage)
	//}}AFX_DATA_INIT
}

CProjectPropPage::~CProjectPropPage()
{
}

void CProjectPropPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CProjectPropPage)
	DDX_Control(pDX, IDC_PROJWORKSPACE_COMBO, m_projectworkspace_combo);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CProjectPropPage, CPropertyPage)
	//{{AFX_MSG_MAP(CProjectPropPage)
	ON_BN_CLICKED(IDC_ADD_BROWSE, OnBrowse)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CProjectPropPage message handlers

BOOL CProjectPropPage::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	
	ide_interface *ide = ide_interface::get_platform_interface();
	ide_interface::stringvec const &projectvec = ide->last_projects();
	ide_interface::stringvec::const_iterator i = projectvec.begin();
	while (i != projectvec.end()) {
		m_projectworkspace_combo.InsertString(-1, i->c_str());
		++i;
	}
	delete ide;
	m_projectworkspace_combo.SetCurSel(0);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CProjectPropPage::OnBrowse() 
{
	// browse button selected.  Find a makefile/workspace not provided from recently-used-file list
	CFileDialog filebrowser(true, NULL, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, "Makefiles (*.mak)|*.mak; Makefile|All Files (*.*)|*.*||");
	filebrowser.DoModal();
	// send the information to the combo box:
	m_projectworkspace_combo.AddString(filebrowser.GetPathName());
	m_projectworkspace_combo.SelectString(-1, filebrowser.GetPathName());
	// or would it be better to simply go on to the next screen?
}


LRESULT CProjectPropPage::OnWizardNext() 
{
	// TODO: Add your specialized code here and/or call the base class
	// Create a new make action, and push it on the pending action list
	// need to make a decision: should the first target be different from later ones where
	// the target is known?

	CString filename;
	m_projectworkspace_combo.GetWindowText(filename);
	string actiontext("nmake -f \"");	// make sure the filename's wrapped in quotes
	actiontext += string(filename);
	actiontext += '"';

	// should try to make these locals more, well, I don't know.
	env_table environment;
	directory_vector cwd;	// should be dirname of makefile by default, not cwd!
	pendingmakeactions.push(action(actiontext, cwd, environment));

	return CPropertyPage::OnWizardNext();
}

BOOL CProjectPropPage::OnSetActive() 
{
 	// enable the next button--don't go back, 'cause we just started!
	CPropertySheet *pSheet = STATIC_DOWNCAST(CPropertySheet, GetParent());
	pSheet->SetWizardButtons(PSWIZB_NEXT);
	
	return CPropertyPage::OnSetActive();
}
