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
// ComplexSort.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CComplexSort dialog
#include <afxtempl.h>
#include "resource.h"
#ifndef __QUERYRESULTS_H
   #define __QUERYRESULTS_H
   #include "queryresults.h"
#endif


class CComplexSort : public CDialog
{
// Construction
public:
	CComplexSort(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CComplexSort)
	enum { IDD = IDD_SORT };
	CButton	m_Apply;
	CButton	m_Decending;
	CButton	m_Acending;
	CButton	m_Remove;
	CButton	m_Add;
	CListBox	m_Source;
	CListBox	m_Destination;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CComplexSort)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CComplexSort)
	virtual BOOL OnInitDialog();
	afx_msg void OnApply();
	virtual void OnCancel();
	virtual void OnOK();
	afx_msg void OnAdd();
	afx_msg void OnRemove();
	afx_msg void OnAcending();
	afx_msg void OnDecending();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
    void SetApplyTaget(CQueryResults* workWith);

private:
	CBitmap m_ArDn;
	CBitmap m_ArUp;
	CBitmap m_ArLeft;
	CBitmap m_ArRight;
	CString m_StartupSort;
	void MakeCollection(CString& collection);
	CArray<CString,CString&> m_CommandsTable;
	BOOL  m_RefillOnCancel;
	BOOL  m_Durty;
	CQueryResults* m_QueryResults;
};
