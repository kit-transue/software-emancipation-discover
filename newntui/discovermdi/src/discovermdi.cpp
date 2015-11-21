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
// DiscoverMDI.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include <pdustring.h>
#include <pduio.h>
#include "DiscoverMDI.h"
#include "SHLOBJ.H"
#include "MainFrm.h"
#include "ChildFrm.h"
#include "IpFrame.h"
#include "DiscoverMDIDoc.h"
#include "TpmProcessor.h"
#include "nameServCalls.h"
#include "DisRegistry.h"
#include "SocketComm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
static char integrator_exit_code[9]={27,0,0,27,0,0,0,0,0};
int integrator_socket;

/////////////////////////////////////////////////////////////////////////////
// CDiscoverMDIApp

BEGIN_MESSAGE_MAP(CDiscoverMDIApp, CWinApp)
	//{{AFX_MSG_MAP(CDiscoverMDIApp)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
	ON_COMMAND(ID_HELP_INDEX, OnHelpContents)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
	// Standard file based document commands
	//ON_COMMAND(ID_FILE_NEW, CWinApp::OnFileNew)
	//ON_COMMAND(ID_FILE_OPEN, CWinApp::OnFileOpen)
	// Standard print setup command
	ON_COMMAND(ID_FILE_PRINT_SETUP, CWinApp::OnFilePrintSetup)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDiscoverMDIApp construction

CDiscoverMDIApp::CDiscoverMDIApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CDiscoverMDIApp object

CDiscoverMDIApp theApp;

// This identifier was generated to be statistically unique for your app.
// You may change it if you prefer to choose a specific identifier.

// {64FA0D56-261E-11D2-AEFB-00A0C9B71DC4}
//static const CLSID clsid =
//{ 0x64fa0d56, 0x261e, 0x11d2, { 0xae, 0xfb, 0x0, 0xa0, 0xc9, 0xb7, 0x1d, 0xc4 } };

// {C218F292-2657-11D2-AEFD-00A0C9B71DC4}
static const CLSID clsid =
{ 0xc218f292, 0x2657, 0x11d2, { 0xae, 0xfd, 0x0, 0xa0, 0xc9, 0xb7, 0x1d, 0xc4 } };

/////////////////////////////////////////////////////////////////////////////
// CDiscoverMDIApp initialization

BOOL CDiscoverMDIApp::InitInstance()
{
	if (!AfxSocketInit())
	{
		AfxMessageBox(IDP_SOCKETS_INIT_FAILED);
		return FALSE;
	}

	// Initialize OLE libraries
	if (!AfxOleInit())
	{
		AfxMessageBox(IDP_OLE_INIT_FAILED);
		return FALSE;
	}

	// Initialize RichEdit control
	if (LoadLibrary(_T("RICHED32.DLL")) == NULL)
	{
		AfxMessageBox("Can't load RICHED32.DLL", MB_OK|MB_ICONEXCLAMATION);
		return FALSE;
	}

	AfxEnableControlContainer();

	char buf[1024];
	CString helpFileDir;
    if(GetEnvironmentVariable("PSETHOME",buf,1024)!=0) {
       helpFileDir=buf;
       helpFileDir+="\\lib\\hyperhelp\\";
	} else {
       helpFileDir="C:\\Discover\\lib\\hyperhelp\\";
	}
	helpFileDir+="discover.hlp";
    m_pszHelpFilePath=_tcsdup(helpFileDir);

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.

#ifdef _AFXDLL
	Enable3dControls();			// Call this when using MFC in a shared DLL
#else
	Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif
	
	LoadPreferences();

	// Change the registry key under which our settings are stored.
	// You should modify this string to be something appropriate
	// such as the name of your company or organization.
	SetRegistryKey(_T("Local AppWizard-Generated Applications"));

	LoadStdProfileSettings();  // Load standard INI file options (including MRU)

	// Register the application's document templates.  Document templates
	//  serve as the connection between documents, frame windows and views.

	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);
	// Check to see if launched as OLE server
	m_AsServer=FALSE;
	if (cmdInfo.m_bRunEmbedded || cmdInfo.m_bRunAutomated)
	{
		// Application was run with /Embedding or /Automation.  Don't show the
		//  main window in this case.
		m_AsServer=TRUE;
	}
	CMultiDocTemplate* pDocTemplate;
	if(m_AsServer==TRUE) {
	   pDocTemplate = new CMultiDocTemplate(
		   IDR_DISCOVTYPE,
		   RUNTIME_CLASS(CDiscoverMDIDoc),
		   RUNTIME_CLASS(CFrameWnd), // custom MDI child frame
		   RUNTIME_CLASS(CMainFrame));
	} else {
	   pDocTemplate = new CMultiDocTemplate(
		   IDR_DISCOVTYPE,
		   RUNTIME_CLASS(CDiscoverMDIDoc),
		   RUNTIME_CLASS(CMainFrame),
		   NULL);
	   if(cmdInfo.m_nShellCommand==CCommandLineInfo::FileNew)
	        cmdInfo.m_nShellCommand=CCommandLineInfo::FileNothing;
	}
    pDocTemplate->SetContainerInfo(IDR_DISCOVTYPE_CNTR_IP);
	pDocTemplate->SetServerInfo( IDR_DISCOVTYPE_SRVR_EMB, 
		                         IDR_DISCOVTYPE_SRVR_IP,
		                         RUNTIME_CLASS(CInPlaceFrame));
	AddDocTemplate(pDocTemplate); 

    // Connect the COleTemplateServer to the document template.
    //  The COleTemplateServer creates new documents on behalf
    //  of requesting OLE containers by using information
    //  specified in the document template.
    m_server.ConnectTemplate(clsid, pDocTemplate, FALSE);
    // Register all OLE server factories as running.  This enables the
    //  OLE libraries to create objects from other applications.
    COleTemplateServer::RegisterAll();
    // Enable DDE Execute open
    EnableShellOpen();
    RegisterShellFileTypes(TRUE);

	// Check to see if launched as OLE server
	// Application was run with /Embedding or /Automation.  Don't show the
	// main window in this case.
	if (m_AsServer==TRUE) return TRUE;

	// Note: MDI applications register all server objects without regard
    //  to the /Embedding or /Automation on the command line.
	// create main MDI Frame window
	CMainFrame* pMainFrame = new CMainFrame;
	if (!pMainFrame->LoadFrame(IDR_MAINFRAME))
		return FALSE;
	m_pMainWnd = pMainFrame;

	// When a server application is launched stand-alone, it is a good idea
	// to update the system registry in case it has been damaged.
    m_server.UpdateRegistry(OAT_DOC_OBJECT_SERVER);
	COleObjectFactory::UpdateRegistryAll();
	// Dispatch commands specified on the command line
	if (!ProcessShellCommand(cmdInfo))
		return FALSE;


	pMainFrame->SetNewMenu(IDR_MAINFRAME);
	pMainFrame->ShowWindow(m_nCmdShow);
	pMainFrame->UpdateWindow();

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	CStatic	m_ImageAbout;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	DDX_Control(pDX, IDC_ABOUT_IMAGE, m_ImageAbout);
	//}}AFX_DATA_MAP
}

BOOL CAboutDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

	CRect rcImage;
	m_ImageAbout.GetWindowRect(&rcImage);
	m_ImageAbout.MoveWindow(0,0,rcImage.Width(),rcImage.Height());

	CRect rcDlg;
	GetWindowRect(&rcDlg);
	CRect rcClient;
	GetClientRect(&rcClient);
	 // Calc non-client size of frame window
	int ncWidth = rcDlg.Width() - rcClient.Width();
	int ncHeight = rcDlg.Height() - rcClient.Height();
	MoveWindow(rcDlg.left,rcDlg.top,rcImage.Width()+ncWidth,rcImage.Height()+ncHeight);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDiscoverMDIApp commands

// App command to run the dialog
void CDiscoverMDIApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal(); //realpath
}

// App command to run the help browser
void CDiscoverMDIApp::OnHelpContents()
{
	char* pszPSETHOME = getenv("PSETHOME");
	CString szCmd;
	CString szParams;

	szCmd.Format("\"%s\\webhelp\\model_browser\\ciee_win_webhelp.htm\"",pszPSETHOME);

	CloseHandle(ShellOpenExecute(szCmd,szParams));
}

CDocument* CDiscoverMDIApp::OpenDocumentFile(LPCTSTR lpszFileName)  {
char path[2048];
char realPath[2048];
char* pt;
CString inputName;

    inputName=lpszFileName;
	// Making template
	char host[1024];
	gethostname(host,1024);
	CString templ;
	templ="Discover:";
	templ+=host;

	int findRes=inputName.Find(templ); 
 	if(findRes==-1) {
		 // shortcut can come instead filename - we need to extract 
		 // real filename from the shortcut. 
		 findRes=inputName.Find(".lnk"); 
		 if(findRes==-1) findRes=inputName.Find(".LNK"); 
	     if(findRes!=-1) {
              ResolveIt(*AfxGetMainWnd(), lpszFileName, realPath);
		      inputName=realPath;
		 }
	     // We need to convert short file name into long one.
         WIN32_FIND_DATA findFileData;
         ::FindFirstFile(inputName,&findFileData);
         GetFullPathName(inputName,2048,path,&pt);
         *pt=0;
		 CString fileName;
		 fileName=path;
		 fileName+=findFileData.cFileName;
		 // New service name will be in the form "Discover:<host>:<long file name>
		 m_ServiceName=templ;
		 m_ServiceName+=":";
		 m_ServiceName+=fileName;
		 // In future we will need to check if service available and run
		 // pset_server if not.
		 if(fileName.Find(".prefs")!=-1 || fileName.Find(".PREFS")!=-1) {
		    LoadPrefs(fileName);
		    StartServer();
		 } else {
            m_ServiceName=="";
		 }

	} else m_ServiceName=lpszFileName;
	AfxGetMainWnd()->SendMessage(WM_COMMAND,IDC_SERVICECHANGED,0);
	CDocument* doc = new CDocument;
	return doc;

}

int CDiscoverMDIApp::ExitInstance()  {
	SavePreferences();
    closesocket(integrator_socket);
	return CWinApp::ExitInstance();
}

//--------------------------------------------------------------------------------
// This private function will load all values from the *.prefs file and will set
// all edit controls using this values.
//--------------------------------------------------------------------------------
void CDiscoverMDIApp::LoadPrefs(CString & filename) {
const char *pszValue = NULL;

       Settings *new_settings_instance = new Settings("psetPrefs");
       int res=new_settings_instance->open(filename);
	   m_PrefsName=filename;

       Settings::Item pdf_item("pdfFileUser");
       if( new_settings_instance->read(pdf_item) != -1 ) {
		   if(pdf_item.read(&pszValue)!=-1 && pszValue) {
               m_PdfName=pszValue;
		   } else {
               Settings::Item pdf_item("defaultPDF");
               if( new_settings_instance->read(pdf_item) != -1 ) {
		           if(pdf_item.read(&pszValue)!=-1 && pszValue) {
                       m_PdfName=pszValue;
				   }
			   }
		   } 
	   } else {
           Settings::Item pdf_item("defaultPDF");
           if( new_settings_instance->read(pdf_item) != -1 ) {
		       if(pdf_item.read(&pszValue)!=-1 && pszValue) {
                   m_PdfName=pszValue;
			   }
		   }
	   }


       Settings::Item home_item("projectHome");
       if( new_settings_instance->read(home_item) != -1 ) {
		   if(home_item.read(&pszValue)!=-1 && pszValue) {
               m_HomeName=pszValue;
		   }
	   }

       Settings::Item admin_item("ADMINDIR");
       if( new_settings_instance->read(admin_item) != -1 ) {
		   if(admin_item.read(&pszValue)!=-1 && pszValue) {
               m_AdminName=pszValue;
		   }
	   }
       Settings::Item sharedsrc_item("sharedSrcRoot");
       if( new_settings_instance->read(sharedsrc_item) != -1 ) {
		   if(sharedsrc_item.read(&pszValue)!=-1 && pszValue) {
               m_SharedSrcName=pszValue;
		   }
	   }
       Settings::Item privatesrc_item("privateSrcRoot");
       if( new_settings_instance->read(privatesrc_item) != -1 ) {
		   if(privatesrc_item.read(&pszValue)!=-1 && pszValue) {
               m_PrivateSrcName=pszValue;
		   }
	   }
       Settings::Item sharedmodel_item("sharedModelRoot");
       if( new_settings_instance->read(sharedmodel_item) != -1 ) {
		   if(sharedmodel_item.read(&pszValue)!=-1 && pszValue) {
               m_SharedModelName=pszValue;
		   }
	   }
       Settings::Item privatemodel_item("privateModelRoot");
       if( new_settings_instance->read(privatemodel_item) != -1 ) {
		   if(privatemodel_item.read(&pszValue)!=-1 && pszValue) {
               m_PrivateModelName=pszValue;
		   }
	   }
	   new_settings_instance->close(0);
	   delete new_settings_instance;
}
//-------------------------------------------------------------------------------



//---------------------------------------------------------------------------------------
// This private function will recursively parse any string with variables inside
// %..% and will return the real string changing all environment variables inside
// %..% into it's real value.
//---------------------------------------------------------------------------------------
CString CDiscoverMDIApp::Parse(CString & src) {
register i,j;
CString dest;
CString env;
BOOL in_env=FALSE;
BOOL in_add_env=FALSE;
char buf[1024];
CString vstr;;

    for(i=0;i<src.GetLength();i++) {
		if(src[i]=='%' && in_add_env==FALSE) {
			if(in_env==FALSE)  {
				in_env=TRUE;
				continue;
			}
			if(in_env==TRUE)  {
				in_env=FALSE;
		        GetEnvironmentVariable(env,buf,1024);
				BOOL need_parse=false;
			    for(j=0;j<(int)strlen(buf);j++) 
					if(buf[j]=='%' || buf[j]=='$') need_parse=TRUE;
				if(need_parse==TRUE) vstr=Parse(CString(buf));
				else                 vstr=buf;
				env="";
				dest+=vstr;
				continue;
			}
		}

		if(src[i]=='$' && in_env == FALSE) {
			if(in_add_env==FALSE)  {
				in_add_env=TRUE;
				continue;
			}
		}

		if(src[i]=='/' || src[i]=='\\' || src[i]==' ' || src[i]=='\n') {
			if(in_add_env==TRUE)  {
				in_add_env=FALSE;
		        GetEnvironmentVariable(env,buf,1024);
				BOOL need_parse=false;
			    for(j=0;j<(int)strlen(buf);j++) 
					if(buf[j]=='%' || buf[j]=='$') need_parse=TRUE;
				if(need_parse==TRUE) vstr=Parse(CString(buf));
				else                 vstr=buf;
				env="";
				dest+=vstr;
			}
		}

		if(in_env==TRUE || in_add_env==TRUE) env+=src[i];
		else                                 dest+=src[i];

	}
	if(in_add_env==TRUE)  {
		in_add_env=FALSE;
        GetEnvironmentVariable(env,buf,1024);
		BOOL need_parse=false;
	    for(j=0;j<(int)strlen(buf);j++) 
		if(buf[j]=='%' || buf[j]=='$') need_parse=TRUE;
		if(need_parse==TRUE) vstr=Parse(CString(buf));
		else                 vstr=buf;
		env="";
		dest+=vstr;
	}
	return dest;
}
//---------------------------------------------------------------------------------------



//---------------------------------------------------------------------------------------
// User wants to add service to the list of available services.
//---------------------------------------------------------------------------------------
void CDiscoverMDIApp::StartServer(void) {
int service_port;
int tcp_addr;
int i;
CString command;
CString sname;
STARTUPINFO si;
PROCESS_INFORMATION pi;


    // Generating the complete path to the BIN directory
	char buf[1024];
	CString serverFileDir;
    if(GetEnvironmentVariable("PSETHOME",buf,1024)!=0) {
       serverFileDir=buf;
       serverFileDir+="\\Bin\\";
	} else {
       serverFileDir="C:\\Discover\\Bin\\";
	}

    // Checking for the selected service, if is not available or dead
	// starting new service.
	int res;
	switch((res=NameServerGetService(m_ServiceName,service_port,tcp_addr))) {
	   case 1 : break; // Server is running
	   case -1: // Need to start nameserver first
		        command.Format("%snameserv",serverFileDir);
	            memset(&si,0,sizeof(STARTUPINFO));
				si.wShowWindow= SW_HIDE;
	            si.cb=sizeof(STARTUPINFO);
				if(::CreateProcess(NULL,command.GetBuffer(50),NULL,NULL,FALSE,CREATE_NO_WINDOW,NULL,NULL,&si,&pi)==FALSE) {
		            AfxMessageBox("Can't start nameserv.exe.\n Check your path variable.");
		            return;
				}
		        CloseHandle(pi.hThread);
		        CloseHandle(pi.hProcess);
	   case 0 : // Setting environment variables and starting the server   
		        SetEnvironmentVariable("ADMINDIR",Parse(m_AdminName));
		        SetEnvironmentVariable("sharedsrcroot",Parse(m_SharedSrcName));
	            SetEnvironmentVariable("privatesrcroot",Parse(m_PrivateSrcName));
	            SetEnvironmentVariable("sharedmodelroot",Parse(m_SharedModelName));
	            SetEnvironmentVariable("privatemodelroot",Parse(m_PrivateModelName));

	            command.Format("%spset_server -prefs %s -pdf %s -home %s -server -server_name %s",serverFileDir,Parse(m_PrefsName),Parse(m_PdfName),m_HomeName,m_ServiceName);
	            memset(&si,0,sizeof(STARTUPINFO));
	            si.wShowWindow=SW_HIDE;
	            si.cb=sizeof(STARTUPINFO);
	            if(::CreateProcess(NULL,command.GetBuffer(50),NULL,NULL,FALSE, CREATE_NO_WINDOW,NULL,NULL,&si,&pi)==FALSE) {
	                 AfxMessageBox("Can't find pset_server.exe.\n Check your path variable.");
	                 return;
				}
                // Waiting the pset_server start by asking it's service from
		        // the nameserver.
				HCURSOR save = SetCursor(::LoadCursor(NULL, IDC_WAIT));
				DWORD code;
	            for(i=0;i<400;i++) {
	                if(NameServerGetService(m_ServiceName,service_port,tcp_addr)==1) break;
					GetExitCodeProcess(pi.hProcess,&code);
					if(code!=STILL_ACTIVE) {
						::MessageBox(NULL,"Use \"Service Manager\" menu item with the \"Server log window\" checkbox enabled\nto see the server startup log.","Error starting server",MB_OK|MB_ICONEXCLAMATION);
						break;
					}
				}
				if(i==400) {
					::MessageBox(NULL,"Use \"Service Manager\" menu item with the \"Server log window\" checkbox enabled\nto see the server startup log.","Timeout starting server",MB_OK|MB_ICONEXCLAMATION);
				}
	            CloseHandle(pi.hThread);
	            CloseHandle(pi.hProcess);
		        SetCursor(save);
				break;
	}
}
//---------------------------------------------------------------------------------------


HRESULT CDiscoverMDIApp::ResolveIt(HWND hwnd, LPCSTR pszShortcutFile, LPSTR pszPath) {
  HRESULT hres;
  IShellLink* psl;
  char szDescription[MAX_PATH];
  WIN32_FIND_DATA wfd;

  // Get a pointer to the IShellLink interface.
  hres = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER,
                            IID_IShellLink, (void **)&psl);
  if (SUCCEEDED(hres))
  {
    IPersistFile* ppf;

    // Get a pointer to the IPersistFile interface.
    hres = psl->QueryInterface(IID_IPersistFile, (void **)&ppf);
    if (SUCCEEDED(hres))
    {
      WORD wsz[MAX_PATH];

      // Ensure string is Unicode.
      MultiByteToWideChar(CP_ACP, 0, pszShortcutFile, -1, wsz,
                                 MAX_PATH);

     // Load the shell link.
     hres = ppf->Load(wsz, STGM_READ);
     if (SUCCEEDED(hres))
     {
       // Resolve the link.
       hres = psl->Resolve(hwnd, SLR_ANY_MATCH);

       if (SUCCEEDED(hres))
       {
          strcpy(pszPath, pszShortcutFile);
          // Get the path to the link target.
          hres = psl->GetPath(pszPath, MAX_PATH, (WIN32_FIND_DATA *)&wfd, 
                 SLGP_SHORTPATH );
          if (!SUCCEEDED(hres))
             AfxMessageBox("GetPath failed!");
                   
          // Get the description of the target.         
          hres = psl->GetDescription(szDescription, MAX_PATH);
          if (!SUCCEEDED(hres))
             AfxMessageBox("GetDescription failed!");
        }
      }
      // Release pointer to IPersistFile interface.
      ppf->Release();
    }
    // Release pointer to IShellLink interface.
    psl->Release();
  }
  return hres;
}


//-----------------------------------------------------------------------------------
// UI preferences
//-----------------------------------------------------------------------------------

Preferences::Manipulator::Manipulator(const char * group) : Settings::Persistent(group)
{
    // -- parsing parameters
    vardelm   = '=';
    gidprefix = pdstrdup("");

	const char *lf = "\r\n";
	format = pdmpack("s", "%s%cs%c%cs%s", gidprefix, '%', vardelm, '%', lf);
}

//
// returns: -1: error, >=0: the number of loaded objects
//
int Preferences::Manipulator::load(const char * filename, parray * values)
{
	CFileStatus prefStatus; // if preference file exists but empty - do not read it
							// just return 0
    if(CFile::GetStatus(filename, prefStatus)) {
		if(prefStatus.m_size == 0) return 0;
	} 
	return Persistent::load(filename,values);
}


Preferences::Preferences() : Settings("") {}

//
// returns: -1: error, >=0: number of loaded variables
//
int Preferences::open(const char * load_loc)
{
    int ret_val = -1;

    if(!location) {
		if(load_loc!=0 && values!=0) {
			if(Settings::Item::dispose(values) >= 0) {
				Preferences::Manipulator sfile(ident);

				if((ret_val=sfile.load(load_loc, values)) >= 0)
					location = pdstrdup(load_loc);
			}
		}
    }

    return ret_val;
}

//
// returns: -1: error, >=0: the number of saved items
//
int Preferences::flush(const char * flush_loc)
{
    int ret_val = -1;

    if(!flush_loc) flush_loc = location;
    if(flush_loc) {
		Preferences::Manipulator sfile(ident);

		ret_val = sfile.save(values, flush_loc);
    }

    return ret_val;
}

bool Preferences::set(const char* szKey, CString& szValue) {
    Settings::Item prefs_item(szKey);
	prefs_item.write((LPCTSTR)szValue);
	write(prefs_item);
	return true;
}

bool Preferences::get(const char* szKey, CString& szValue) {
	const char *pszValue = NULL;
    Settings::Item prefs_item(szKey);
    if(read(prefs_item) != -1 ) {
		if(prefs_item.read(&pszValue)==-1) {
			pszValue = NULL;
		}
	}
	if(pszValue!=NULL) szValue = pszValue;
	return pszValue!=NULL;
}

void CDiscoverMDIApp::LoadPreferences() {
	CString filename = ::getenv("USERPROFILE");
	filename += "\\.codeintegrity";
	
	CFileStatus prefStatus; // if preference file does not exist - 
							// create an empty one before reading it
    if(!CFile::GetStatus(filename, prefStatus)) {
		CFile prefsFile;
		if(prefsFile.Open( filename, CFile::modeCreate | CFile::modeWrite)) {
			prefsFile.Close();
		}
    }
    m_pPreferences = new Preferences();
	m_pPreferences->open(filename);
}

void CDiscoverMDIApp::SavePreferences() {
	m_pPreferences->close(1);
	delete m_pPreferences;
}


BOOL CDiscoverMDIApp::IsNT() {
	DWORD dwVersion = GetVersion();
 
	// Get the Windows version.
	DWORD dwWindowsMajorVersion =  (DWORD)(LOBYTE(LOWORD(dwVersion)));
	DWORD dwWindowsMinorVersion =  (DWORD)(HIBYTE(LOWORD(dwVersion)));
	return (dwWindowsMinorVersion==0) && (dwWindowsMajorVersion==4);
}

HANDLE CDiscoverMDIApp::ShellOpenExecute(CString& szCmd, CString& szParams, bool bAllowNull/*=true*/)
{
	BOOL bExecuted = FALSE;

	SHELLEXECUTEINFO execInfo;

	execInfo.cbSize = sizeof(execInfo);
	execInfo.fMask = SEE_MASK_FLAG_NO_UI | 
					 SEE_MASK_NO_CONSOLE |
					 SEE_MASK_DOENVSUBST | 
					 SEE_MASK_FLAG_DDEWAIT | 
					 SEE_MASK_NOCLOSEPROCESS;
	execInfo.hwnd = 0;
	execInfo.lpVerb = _T("open");
	execInfo.lpFile = (LPCTSTR)szCmd;
	execInfo.lpParameters = (LPCTSTR)szParams;
	execInfo.lpDirectory = NULL;
	execInfo.nShow = SW_HIDE;
	bExecuted = ShellExecuteEx(&execInfo);

	if(bExecuted && execInfo.hProcess==NULL && !bAllowNull) 
		bExecuted=FALSE;

	if(bExecuted == FALSE) {
		LPVOID lpMsgBuf;
		FormatMessage( 
			FORMAT_MESSAGE_ALLOCATE_BUFFER | 
			FORMAT_MESSAGE_FROM_SYSTEM | 
			FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			GetLastError(),
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
			(LPTSTR) &lpMsgBuf,
			0,
			NULL 
		);
		CString szMessage=szCmd+"\n";
		szMessage += (LPCTSTR)lpMsgBuf;
		// Display the string.
		MessageBox(NULL,szMessage, "Can not start process", MB_OK | MB_ICONERROR);
		// Free the buffer.
		LocalFree( lpMsgBuf );    
		return NULL;
	}
	return execInfo.hProcess;
}
