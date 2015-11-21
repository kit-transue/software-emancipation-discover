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
// ServerStartDlg.h : header file
//

#if !defined(AFX_SERVERSTARTDLG_H__61F5B0D8_5DF4_11D2_AF37_00A0C9B71DC4__INCLUDED_)
#define AFX_SERVERSTARTDLG_H__61F5B0D8_5DF4_11D2_AF37_00A0C9B71DC4__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

class COptions {
public:
       CString m_Prefs;

	   void Save(void);
	   void Load(void);
};


/////////////////////////////////////////////////////////////////////////////
// CServerStartDlg dialog

class CServerStartDlg : public CDialog
{
// Construction
public:
	CString m_SelectedService;
	CServerStartDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CServerStartDlg)
	enum { IDD = IDD_SERVERSTART_DIALOG };
	CButton	m_NewGroup;
	CStatic	m_Name;
	CEdit	m_NameEditor;
	CTabCtrl	m_ModeSelector;
	CComboBox	m_TimeoutSelector;
	CStatic	m_Timeout;
	CButton	m_StartClientCheck;
	CButton	m_RemoveButton;
	CButton	m_NameDebugCheck;
	CButton	m_SaveButton;
	CButton	m_DebugCheck;
	CListBox	m_ServiceList;
	CEdit	m_SharedModelEdit;
	CEdit	m_PrivateSrcEdit;
	CEdit	m_PrivateModelEdit;
	CEdit	m_HomeEdit;
	CEdit	m_SharedSrcEdit;
	CEdit	m_AdminEdit;
	CEdit	m_PdfEdit;
	CEdit	m_PrefsEdit;
	CString	m_Admin;
	CString	m_Home;
	CString	m_Pdf;
	CString	m_Prefs;
	CString	m_PrivateModel;
	CString	m_PrivateSrc;
	CString	m_SharedModel;
	CString	m_SharedSrc;
	BOOL	m_Debug;
	BOOL	m_StartClient;
	BOOL	m_NameDebug;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CServerStartDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CServerStartDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnBrowsePrefs();
	afx_msg void OnBrowsePdf();
	afx_msg void OnAdd();
	afx_msg void OnServiceSelected();
	afx_msg void OnChangeAdmin();
	afx_msg void OnChangeHome();
	afx_msg void OnChangePrefs();
	afx_msg void OnChangePdf();
	afx_msg void OnPrefsEntered();
	afx_msg void OnChangeSharedmodel();
	afx_msg void OnChangePrivatemodel();
	afx_msg void OnChangePrivatesource();
	afx_msg void OnChangeSharedsource();
	afx_msg void OnSave();
	virtual void OnOK();
	afx_msg void OnRemove();
	afx_msg void OnPageChanged(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	CString m_PrefsFile;
	void SavePrefs(CString& filename);
	void LoadPrefs(CString& filename);
	void FillServiceList(int mode);
    CString Parse(CString & src);
	BOOL m_PathChanged;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SERVERSTARTDLG_H__61F5B0D8_5DF4_11D2_AF37_00A0C9B71DC4__INCLUDED_)
