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
// FilterDialog.h : header file
//
#include <afxcmn.h>
#include <afxtempl.h>
#include "CommonDialogs.rh"
#include "QueryResults.h"

#define INTEGER_ATTRIBUTE 0
#define STRING_ATTRIBUTE  1

class AttributeEntry {
public:
	 CString Name;
	 CString Option;
	 int     Type;
};

class TokenLoc {
public:
	 int     From;
	 int     To;
	 int     Attribute;
	 int     Operation;
	 int     Concatination;
	 CString User;
};

/////////////////////////////////////////////////////////////////////////////
// CFilterDialog dialog

class CFilterDialog : public  CPropertyPage
{
	DECLARE_DYNCREATE(CFilterDialog)

// Construction
public:
	CFilterDialog();   // standard constructor
	~CFilterDialog();

// Dialog Data
	//{{AFX_DATA(CFilterDialog)
	enum { IDD = IDD_FILTERWIZARD };
	CButton	m_ChangeButton;
	CButton	m_RemoveButton;
	CButton	m_OrButton;
	CButton	m_AndButton;
	CEdit	m_ArgumentEdit;
	CComboBox	m_OperationCombo;
	CTabCtrl	m_FlowTab;
	CEdit	m_CommandEdit;
	CComboBox	m_AttributeCombo;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFilterDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CFilterDialog)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeAttribute();
	afx_msg void OnAnd();
	afx_msg void OnOr();
	afx_msg void OnAddbutton();
	afx_msg void OnSelchangeTabexpression(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnRemovebutton();
	afx_msg void OnChangebutton();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	CImageList m_ImageList;
	int m_TokensCount;
	CString& GetSubexpression(void);
	BOOL CheckSubexpression(void);
	void FillOperationsCombo();
	CArray<AttributeEntry,AttributeEntry&> m_AttributesTable;
	CArray<TokenLoc,TokenLoc&> m_TokensLocs;

};
