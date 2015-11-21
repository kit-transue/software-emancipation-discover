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
//{{AFX_INCLUDES()
//}}AFX_INCLUDES

#ifndef __DATASOURCE_H
   #define __DATASOURCE_H
   #include "datasource.h"
#endif

#if !defined(AFX_ACCESSWND_H__FC3C4C9C_5EF4_11D2_AF3B_00A0C9B71DC4__INCLUDED_)
#define AFX_ACCESSWND_H__FC3C4C9C_5EF4_11D2_AF3B_00A0C9B71DC4__INCLUDED_
#include "AccessEdit.h"
#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// AccessWnd.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CAccessWnd form view

#ifndef __AFXEXT_H__
#include <afxext.h>
#endif

class CAccessWnd : public CFormView
{
protected:
	CAccessWnd();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CAccessWnd)

// Form Data
public:
	//{{AFX_DATA(CAccessWnd)
	enum { IDD = IDD_ACCESS };
	CButton	m_Font;
	CAccessEdit	m_Edit;
	CButton	m_Stop;
	CButton	m_Prev;
	CButton	m_Paste;
	CButton	m_Next;
	CButton	m_Copy;
	CDataSource	m_DataSource;
	//}}AFX_DATA

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAccessWnd)
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CAccessWnd();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
	//{{AFX_MSG(CAccessWnd)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnReturnPressed();
    afx_msg void OnStatusChanged();
	afx_msg void OnCopysel();
	afx_msg void OnPastesel();
	afx_msg void OnPrevcmd();
	afx_msg void OnNextcmd();
	afx_msg void OnFont();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	void UpdateButtonsStatus(void);
	CBitmap m_ImgStop;
	CBitmap m_ImgFonts;
	CBitmap m_ImgPaste;
	CBitmap m_ImgCopy;
	CBitmap m_ArRight;
	CBitmap m_ArLeft;
	void LayoutControls(int cx, int cy);
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ACCESSWND_H__FC3C4C9C_5EF4_11D2_AF3B_00A0C9B71DC4__INCLUDED_)
