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
// CodeRoverIntegratorPpg.cpp : Implementation of the CCodeRoverIntegratorPropPage property page class.

#include "stdafx.h"
#include "CodeRoverIntegrator.h"
#include "CodeRoverIntegratorPpg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


IMPLEMENT_DYNCREATE(CCodeRoverIntegratorPropPage, COlePropertyPage)


/////////////////////////////////////////////////////////////////////////////
// Message map

BEGIN_MESSAGE_MAP(CCodeRoverIntegratorPropPage, COlePropertyPage)
	//{{AFX_MSG_MAP(CCodeRoverIntegratorPropPage)
	// NOTE - ClassWizard will add and remove message map entries
	//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// Initialize class factory and guid

IMPLEMENT_OLECREATE_EX(CCodeRoverIntegratorPropPage, "CODEROVERINTEGRATOR.CodeRoverIntegratorPropPage.1",
	0x8dbae789, 0xdb45, 0x11d4, 0x8e, 0x41, 0, 0x1, 0x2, 0x39, 0x3, 0x21)


/////////////////////////////////////////////////////////////////////////////
// CCodeRoverIntegratorPropPage::CCodeRoverIntegratorPropPageFactory::UpdateRegistry -
// Adds or removes system registry entries for CCodeRoverIntegratorPropPage

BOOL CCodeRoverIntegratorPropPage::CCodeRoverIntegratorPropPageFactory::UpdateRegistry(BOOL bRegister)
{
	if (bRegister)
		return AfxOleRegisterPropertyPageClass(AfxGetInstanceHandle(),
			m_clsid, IDS_CODEROVERINTEGRATOR_PPG);
	else
		return AfxOleUnregisterClass(m_clsid, NULL);
}


/////////////////////////////////////////////////////////////////////////////
// CCodeRoverIntegratorPropPage::CCodeRoverIntegratorPropPage - Constructor

CCodeRoverIntegratorPropPage::CCodeRoverIntegratorPropPage() :
	COlePropertyPage(IDD, IDS_CODEROVERINTEGRATOR_PPG_CAPTION)
{
	//{{AFX_DATA_INIT(CCodeRoverIntegratorPropPage)
	// NOTE: ClassWizard will add member initialization here
	//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_DATA_INIT
}


/////////////////////////////////////////////////////////////////////////////
// CCodeRoverIntegratorPropPage::DoDataExchange - Moves data between page and properties

void CCodeRoverIntegratorPropPage::DoDataExchange(CDataExchange* pDX)
{
	//{{AFX_DATA_MAP(CCodeRoverIntegratorPropPage)
	// NOTE: ClassWizard will add DDP, DDX, and DDV calls here
	//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_DATA_MAP
	DDP_PostProcessing(pDX);
}


/////////////////////////////////////////////////////////////////////////////
// CCodeRoverIntegratorPropPage message handlers
