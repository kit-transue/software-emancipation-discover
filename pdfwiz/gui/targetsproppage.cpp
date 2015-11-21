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
// TargetsPropPage.cpp : implementation file
//

#include "stdafx.h"
#include "pdf_wizard.h"
#include "TargetsPropPage.h"
#include "make_toplevel.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTargetsPropPage property page

IMPLEMENT_DYNCREATE(CTargetsPropPage, CPropertyPage)

CTargetsPropPage::CTargetsPropPage() : CPropertyPage(CTargetsPropPage::IDD)
{
	//{{AFX_DATA_INIT(CTargetsPropPage)
	//}}AFX_DATA_INIT
}

CTargetsPropPage::~CTargetsPropPage()
{
}

void CTargetsPropPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTargetsPropPage)
	DDX_Control(pDX, IDC_COMBO1, m_target_combo);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CTargetsPropPage, CPropertyPage)
	//{{AFX_MSG_MAP(CTargetsPropPage)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTargetsPropPage message handlers

BOOL CTargetsPropPage::OnSetActive() 
{
 	// re-enable the next button--don't go back, or we'll reparse the makefile
	CPropertySheet *pSheet = STATIC_DOWNCAST(CPropertySheet, GetParent());
	pSheet->SetWizardButtons(PSWIZB_NEXT);

	// get the targets
	makefile::stringset * targets = makefile::globalmakefile->list_all_targets();
	
	// confirm that some exist:
	if (targets->empty())
		;	// pop up a warning dialog--tell the users to export makefile
	else {
		// populate the box with available targets
		
		makefile::stringset::iterator i = targets->begin();
		while (i != targets->end()) {
			m_target_combo.AddString(i->c_str());
			++i;
		}
		// and default to default target
		if (!makefile::globalmakefile->list_default_targets()->empty())
			m_target_combo.SelectString(-1, makefile::globalmakefile->list_default_targets()->begin()->c_str());
	}
	return CPropertyPage::OnSetActive();
}
