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
// FilterList.h : header file
//
#include <afxtempl.h>
#include "resource.h"
#ifndef __QUERYRESULTS_H
   #define __QUERYRESULTS_H
   #include "queryresults.h"
#endif

class CFilterEntry : public CObject {
   public :
	  CFilterEntry() {
		  Name="";
		  Command="";
	  }
	  ~CFilterEntry() {
	  }
      CString Name;
      CString Command;
   public:
	  virtual void Serialize(CArchive& ar) {
        CObject::Serialize( ar );
        if( ar.IsStoring() ) {
            ar<<Name<<Command;
		} else {
            ar>>Name;
            ar>>Command;
		}
      }
   DECLARE_SERIAL(CFilterEntry)
};

/////////////////////////////////////////////////////////////////////////////
// CFilterList property page

class CFilterList : public CPropertyPage
{
	DECLARE_DYNCREATE(CFilterList)

// Construction
public:
	CFilterList();
	~CFilterList();

// Dialog Data
	//{{AFX_DATA(CFilterList)
	enum { IDD = IDD_FILTERLIST };
	CButton	m_RemoveButton;
	CButton	m_ChangeButton;
	CEdit	    m_NameEdit;
	CEdit	    m_CommandEdit;
	CListBox	m_SaveList;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CFilterList)
	public:
	virtual BOOL OnSetActive();
	virtual void OnOK();
	virtual BOOL OnApply();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CFilterList)
	afx_msg void OnAddbutton();
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeSavelist();
	afx_msg void OnChangebutton();
	afx_msg void OnRemovebutton();
	afx_msg void OnChangeCommandedit();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	void UpdateParentName(void);
	void LoadFilters(void);
    void SaveFilters(void);
	CArray<CFilterEntry*,CFilterEntry*> m_AllFilters;
};
