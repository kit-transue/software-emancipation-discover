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
// ServerStart.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "ServerStart.h"
#include "ServerStartDlg.h"
#include "nameServCalls.h"
#include <afxsock.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//---------------------------------------------------------------------------------------
// This methid will save all startup options in the file "psetstart.dat"
//---------------------------------------------------------------------------------------
void COptions::Save(void) {
char buf[1024];
CString fileName;

    if(GetEnvironmentVariable("PSETHOME",buf,1024)!=0) {
		fileName=buf;
		fileName+="/Settings/";
	} else {
		fileName="C:/Discover/Settings/";
	}
	fileName+="psetstart.dat";
    TRY  {
	     CFile file( fileName, CFile::modeCreate | CFile::modeWrite);
         TRY  {
             CArchive storage(&file,CArchive::store);
             storage << m_Prefs;
		 }
         CATCH( CArchiveException, e ) {
	          return;
		 }
         END_CATCH
	}
    CATCH( CFileException, e ) {
	     return;
	}
    END_CATCH
}
//---------------------------------------------------------------------------------------


//---------------------------------------------------------------------------------------
// This method will load all startup options from the  file "psetstart.dat"
//---------------------------------------------------------------------------------------
void COptions::Load(void) {
char buf[1024];
CString fileName;

    if(GetEnvironmentVariable("PSETHOME",buf,1024)!=0) {
		fileName=buf;
		fileName+="/Settings/";
	} else {
		fileName="C:/Discover/Settings/";
	}
	fileName+="psetstart.dat";
    TRY  {
	     CFile file( fileName, CFile::modeRead);
	     TRY {
             CArchive storage(&file,CArchive::load);
             storage >> m_Prefs;
		 } CATCH(CArchiveException, ae) {
		     return;
		 }
         END_CATCH
	}
    CATCH( CFileException, e ) {
	     return;
	}
    END_CATCH
}
//---------------------------------------------------------------------------------------






/////////////////////////////////////////////////////////////////////////////
// CServerStartApp

BEGIN_MESSAGE_MAP(CServerStartApp, CWinApp)
	//{{AFX_MSG_MAP(CServerStartApp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CServerStartApp construction

CServerStartApp::CServerStartApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CServerStartApp object

CServerStartApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CServerStartApp initialization

BOOL CServerStartApp::InitInstance() {
char buf[1024];
CString clientName;
CString clientService;
STARTUPINFO si;
PROCESS_INFORMATION pi;

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.
    AfxSocketInit();

#ifdef _AFXDLL
	Enable3dControls();			// Call this when using MFC in a shared DLL
#else
	Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif
	COptions options;
	options.Load();
	CServerStartDlg dlg;
	dlg.m_Prefs=options.m_Prefs;
	dlg.m_Debug=FALSE;
	dlg.m_NameDebug=FALSE;
	dlg.m_StartClient=FALSE;
	m_pMainWnd = &dlg;
	int nResponse = dlg.DoModal();
	if (nResponse == IDOK) {
	    options.m_Prefs=dlg.m_Prefs;
		options.Save();
		if(dlg.m_StartClient==TRUE) {
			if(dlg.m_SelectedService.GetLength()>0)
			    clientService.Format(" %s",dlg.m_SelectedService);
			else 
			    clientService="";

            if(GetEnvironmentVariable("PSETHOME",buf,1024)!=0) {
		        clientName=CString("\"") + buf;
		        clientName+="/Bin/Discovermdi.exe\"";
			} else {
		        clientName="C:/Discover/Bin/Discovermdi.exe";
			}
			clientName+=clientService;
            memset(&si,0,sizeof(STARTUPINFO));
	        si.cb=sizeof(STARTUPINFO);
			if(::CreateProcess(NULL,clientName.GetBuffer(50),NULL,NULL,FALSE,0,NULL,NULL,&si,&pi)==FALSE) {
		        return FALSE;
			}
		    CloseHandle(pi.hThread);
		    CloseHandle(pi.hProcess);
		}

	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with Cancel GetProcess
	}
	return FALSE;
}

CString CServerStartApp::Parse(CString & src) {
register i,j;
CString dest;
CString env;
BOOL in_env=FALSE;
char buf[1024];
CString vstr;

    for(i=0;i<src.GetLength();i++) {
		if(src[i]=='%') {
			if(in_env==FALSE)  {
				in_env=TRUE;
				continue;
			}
			if(in_env==TRUE)  {
				in_env=FALSE;
		        GetEnvironmentVariable(env,buf,1024);
				BOOL need_parse=false;
			    for(j=0;j<strlen(buf);j++) 
					if(buf[j]=='%') need_parse=TRUE;
				if(need_parse==TRUE) vstr=Parse(CString(buf));
				else                 vstr=buf;
				env="";
				dest+=vstr;
				continue;
			}
		}
		if(in_env==TRUE) env+=src[i];
		if(in_env==FALSE) dest+=src[i];

	}
	return dest;
}

