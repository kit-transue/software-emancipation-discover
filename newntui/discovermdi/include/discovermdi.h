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
// DiscoverMDI.h : main header file for the DISCOVERMDI application
//

#if !defined(AFX_DISCOVERMDI_H__64FA0D5B_261E_11D2_AEFB_00A0C9B71DC4__INCLUDED_)
#define AFX_DISCOVERMDI_H__64FA0D5B_261E_11D2_AEFB_00A0C9B71DC4__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "settings.h"
#include "resource.h"       // main symbols
#define WM_SASHMOVED WM_USER+13
#define WM_SPLITMOVE WM_USER+14
#define WM_PROJECTSPLITMOVE WM_USER+15
#define WM_QUERYSPLITMOVE   WM_USER+16
#define WM_UPDATEGROUPS     WM_USER+17
#define WM_UPDATE_ERRORS    WM_USER+18
#define WM_SERVICE_DISCONNECT    WM_USER+19



/////////////////////////////////////////////////////////////////////////////
// CDiscoverMDIApp:
// See DiscoverMDI.cpp for the implementation of this class
//

class Preferences : public Settings {
public:
	Preferences();
	class Manipulator : public Settings::Persistent {
		public:
			Manipulator(const char *);
			int load(const char * filename, parray * values);
	};
    virtual int open(const char *);
    virtual int flush(const char *);

	bool set(const char* szKey, CString& value);
	bool get(const char* szKey,CString& value);
};


class CDiscoverMDIApp : public CWinApp
{
public:
	static HANDLE ShellOpenExecute(CString& szCmd, CString& szParams,bool bAllowNull=true);
	static BOOL IsNT();
	CString m_ServiceName;
	BOOL m_AsServer;
	CDiscoverMDIApp();

	Preferences* GetPreferences() { return m_pPreferences; }

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDiscoverMDIApp)
	public:
	virtual BOOL InitInstance();
	virtual CDocument* OpenDocumentFile(LPCTSTR lpszFileName);
	virtual int ExitInstance();
	//}}AFX_VIRTUAL

// Implementation
	COleTemplateServer m_server;
		// Server object for document creation

	//{{AFX_MSG(CDiscoverMDIApp)
	afx_msg void OnAppAbout();
	afx_msg void OnHelpContents();
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
protected:
     void LoadPrefs(CString & filename);
     void LoadPreferences();
     void SavePreferences();
     CString Parse(CString & src);
     void StartServer(void);
     HRESULT ResolveIt(HWND hwnd, LPCSTR pszShortcutFile, LPSTR pszPath);
private:
	Preferences* m_pPreferences;
	CString m_PrefsName;
	CString m_PrivateModelName;
	CString m_SharedModelName;
	CString m_PrivateSrcName;
	CString m_SharedSrcName;
	CString m_AdminName;
	CString m_HomeName;
	CString m_PdfName;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DISCOVERMDI_H__64FA0D5B_261E_11D2_AEFB_00A0C9B71DC4__INCLUDED_)
