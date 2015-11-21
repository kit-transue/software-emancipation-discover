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
#if !defined(AFX_PW_PROPSHEET_H__BCB77DF4_3356_11D1_BC0C_006008163D0A__INCLUDED_)
#define AFX_PW_PROPSHEET_H__BCB77DF4_3356_11D1_BC0C_006008163D0A__INCLUDED_

#include "ProjectPropPage.h"	// Added by ClassView
#include "StoragePropPage.h"	// Added by ClassView
#include "TargetsPropPage.h"	// Added by ClassView
#include "parseprogressproppage.h"	// Added by ClassView
#include "doneproppage.h"	// Added by ClassView
#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// PW_propSheet.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CPW_propSheet

class CPW_propSheet : public CPropertySheet
{
	DECLARE_DYNAMIC(CPW_propSheet)

// Construction
public:
	CPW_propSheet(UINT nIDCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);
	CPW_propSheet(LPCTSTR pszCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPW_propSheet)
	//}}AFX_VIRTUAL

// Implementation
public:
	CDonePropPage m_donePage;
	CParseProgressPropPage m_progressPage;
	CStoragePropPage m_storagePage;
	CProjectPropPage m_projectPage;
	CTargetsPropPage m_targetsPage;
	virtual ~CPW_propSheet();

	// Generated message map functions
protected:
	//{{AFX_MSG(CPW_propSheet)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	void InitPages();
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PW_PROPSHEET_H__BCB77DF4_3356_11D1_BC0C_006008163D0A__INCLUDED_)
