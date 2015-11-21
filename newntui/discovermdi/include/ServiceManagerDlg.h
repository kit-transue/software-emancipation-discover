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
// ServiceManagerDlg.h : header file
//

#if !defined(AFX_SERVICEMANAGERDLG_H__CADBE104_B339_40AA_B087_347F40EF8575__INCLUDED_)
#define AFX_SERVICEMANAGERDLG_H__CADBE104_B339_40AA_B087_347F40EF8575__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "NoCollapseTreeCtrl.h"
#include "nameServCalls.h"
#include "WaitDlg.h"

/////////////////////////////////////////////////////////////////////////////
// CServiceManagerDlg dialog

#define WM_RELOADMODELS  WM_USER+1

class CServiceManagerDlg : public CDialog
{
private:
	class StartCompletionProvider : public CCompletionProvider{
	public :
		StartCompletionProvider(CServiceManagerDlg* pDlg,CString szModel,CString szVersion) {
			m_szModel = szModel;
			m_szVersion = szVersion;
			m_pDlg = pDlg;
		}
		bool IsWaitComplete() {
			return m_pDlg->IsPSETRunning(m_szModel,m_szVersion);
		}
	private:
		CString m_szModel,m_szVersion;
		CServiceManagerDlg* m_pDlg;
	};

	class StopCompletionProvider : public CCompletionProvider{
	public :
		StopCompletionProvider(CServiceManagerDlg* pDlg,CString szModel,CString szVersion) {
			m_szModel = szModel;
			m_szVersion = szVersion;
			m_pDlg = pDlg;
		}
		bool IsWaitComplete() {
			return !m_pDlg->IsPSETRunning(m_szModel,m_szVersion);
		}
	private:
		CString m_szModel,m_szVersion;
		CServiceManagerDlg* m_pDlg;
	};

// Construction
public:
	CString GetServiceName();
	bool IsPSETRunning(CString& szModel, CString& szVersion);
	bool IsModelRunning(CString& szModel, CString& szVersion);
	CString& GetRepositoryPath();
	void SetRepositoryPath(CString& szPath);
	CServiceManagerDlg(CWnd* pParent = NULL, CString* pszRepository = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CServiceManagerDlg)
	enum { IDD = IDD_SERVICEMANAGER_DIALOG };
	CButton	m_RefreshBtn;
	CEdit	m_RepositoryEditCtrl;
	CButton	m_BrowseBtn;
	CButton	m_CloseBtn;
	CButton	m_StopBtn;
	CButton	m_StartBtn;
	CNoCollapseTreeCtrl	m_ModelsTreeCtrl;
	CButton	m_SelectRepositoryBtn;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CServiceManagerDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	CString m_szRepository;
	CString m_szRepositoryID;
	bool loadID(CString& szRepository, CString& szID);
	void UpdateControls();
	void ReloadModels();
	void RefreshModels();
	bool IsServiceRunning(CString& szService);
	void GetSelectedModelAndVersion(CString& szModel,CString& szVersion);
	bool ServiceAdmin(CString& szCmd, CString& szParams, CString& szMessage, CCompletionProvider* pProvider);
	static const int PSET_RUNNING;
	static const int MODEL_RUNNING;
	static const int NOT_RUNNING;
	static const int MODEL;
	static const int MODEL_ROOT;
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CServiceManagerDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnModelVersionChanged(NMHDR* pNMHdr, LRESULT* pResult);
	afx_msg BOOL OnToolTipNotify(UINT id, NMHDR * pTTTStruct, LRESULT * pResult);
	afx_msg void OnSelectRepository();
	afx_msg LRESULT OnReloadModels(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnBrowseModelMSG(WPARAM wParam, LPARAM lParam);
	afx_msg void OnBrowseModel();
	afx_msg void OnCloseManager();
	afx_msg void OnDestroy();
	afx_msg void OnStartModel();
	afx_msg void OnRefreshModels();
	afx_msg void OnStopModel();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	HTREEITEM NodeExists(HTREEITEM hParent, CString& szName);
	void RecursiveUpdateNodes(HTREEITEM hParent, bool validState);
	void RecursiveDeleteNodes(HTREEITEM hParent);
	void ValidateNode(HTREEITEM hHode,bool validState = true);
	bool IsNodeValid(HTREEITEM hHode);
	void InvalidateModels();
	void ValidateModels();
	void DeleteInvalidModels();


	CString m_szSelectedService;
	char* m_lpszRepositoryNameTooltip;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SERVICEMANAGERDLG_H__CADBE104_B339_40AA_B087_347F40EF8575__INCLUDED_)
