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
// ServerStartDlg.cpp : implementation file
//

#include "stdafx.h"
#include <afxsock.h>
#include "ServerStart.h"
#include "ServerStartDlg.h"
#include "settings.h"
#include "nameServCalls.h"
#include "SocketComm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
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
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CServerStartDlg dialog

CServerStartDlg::CServerStartDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CServerStartDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CServerStartDlg)
	m_Admin = _T("");
	m_Home = _T("");
	m_Pdf = _T("");
	m_Prefs = _T("");
	m_PrivateModel = _T("");
	m_PrivateSrc = _T("");
	m_SharedModel = _T("");
	m_SharedSrc = _T("");
	m_Debug = FALSE;
	m_StartClient = FALSE;
	m_NameDebug = FALSE;
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CServerStartDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CServerStartDlg)
	DDX_Control(pDX, IDC_NEWSERVICES, m_NewGroup);
	DDX_Control(pDX, IDC_NEWNAME, m_Name);
	DDX_Control(pDX, IDC_NAMEEDITEDIT, m_NameEditor);
	DDX_Control(pDX, IDC_MODESELECTOR, m_ModeSelector);
	DDX_Control(pDX, IDC_TIMEOUTSELECTOR, m_TimeoutSelector);
	DDX_Control(pDX, IDC_TIMEOUT, m_Timeout);
	DDX_Control(pDX, IDC_CLIENTCHECK, m_StartClientCheck);
	DDX_Control(pDX, IDC_REMOVE, m_RemoveButton);
	DDX_Control(pDX, IDC_NAMEDEBUG, m_NameDebugCheck);
	DDX_Control(pDX, IDC_SAVE, m_SaveButton);
	DDX_Control(pDX, IDC_DEBUG, m_DebugCheck);
	DDX_Control(pDX, IDC_SERVICELIST, m_ServiceList);
	DDX_Control(pDX, IDC_SHAREDMODEL, m_SharedModelEdit);
	DDX_Control(pDX, IDC_PRIVATESOURCE, m_PrivateSrcEdit);
	DDX_Control(pDX, IDC_PRIVATEMODEL, m_PrivateModelEdit);
	DDX_Control(pDX, IDC_HOME, m_HomeEdit);
	DDX_Control(pDX, IDC_SHAREDSOURCE, m_SharedSrcEdit);
	DDX_Control(pDX, IDC_ADMIN, m_AdminEdit);
	DDX_Control(pDX, IDC_PDF, m_PdfEdit);
	DDX_Control(pDX, IDC_PREFS, m_PrefsEdit);
	DDX_Text(pDX, IDC_ADMIN, m_Admin);
	DDX_Text(pDX, IDC_HOME, m_Home);
	DDX_Text(pDX, IDC_PDF, m_Pdf);
	DDX_Text(pDX, IDC_PREFS, m_Prefs);
	DDX_Text(pDX, IDC_PRIVATEMODEL, m_PrivateModel);
	DDX_Text(pDX, IDC_PRIVATESOURCE, m_PrivateSrc);
	DDX_Text(pDX, IDC_SHAREDMODEL, m_SharedModel);
	DDX_Text(pDX, IDC_SHAREDSOURCE, m_SharedSrc);
	DDX_Check(pDX, IDC_DEBUG, m_Debug);
	DDX_Check(pDX, IDC_CLIENTCHECK, m_StartClient);
	DDX_Check(pDX, IDC_NAMEDEBUG, m_NameDebug);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CServerStartDlg, CDialog)
	//{{AFX_MSG_MAP(CServerStartDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BROWSEPREFS, OnBrowsePrefs)
	ON_BN_CLICKED(IDC_BROWSEPDF, OnBrowsePdf)
	ON_BN_CLICKED(IDC_ADD, OnAdd)
	ON_LBN_SELCHANGE(IDC_SERVICELIST, OnServiceSelected)
	ON_EN_CHANGE(IDC_ADMIN, OnChangeAdmin)
	ON_EN_CHANGE(IDC_HOME, OnChangeHome)
	ON_EN_CHANGE(IDC_PREFS, OnChangePrefs)
	ON_EN_CHANGE(IDC_PDF, OnChangePdf)
	ON_EN_KILLFOCUS(IDC_PREFS,OnPrefsEntered)
	ON_EN_CHANGE(IDC_SHAREDMODEL, OnChangeSharedmodel)
	ON_EN_CHANGE(IDC_PRIVATEMODEL, OnChangePrivatemodel)
	ON_EN_CHANGE(IDC_PRIVATESOURCE, OnChangePrivatesource)
	ON_EN_CHANGE(IDC_SHAREDSOURCE, OnChangeSharedsource)
	ON_BN_CLICKED(IDC_SAVE, OnSave)
	ON_BN_CLICKED(IDC_REMOVE, OnRemove)
	ON_NOTIFY(TCN_SELCHANGE, IDC_MODESELECTOR, OnPageChanged)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CServerStartDlg message handlers

BOOL CServerStartDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	// TODO: Add extra initialization here
	m_PrefsFile=m_Prefs;
	LoadPrefs(m_PrefsFile);
    m_NameEditor.SetWindowText(m_Prefs);

    TCITEM item;

    item.mask       = TCIF_TEXT;
    item.pszText    = "PSET servers";
    item.cchTextMax = strlen("PSET servers");
	m_ModeSelector.InsertItem(0,&item);
    item.pszText    = "Model servers";
    item.cchTextMax = strlen("Model servers");
	m_ModeSelector.InsertItem(1,&item);

    m_Timeout.EnableWindow(FALSE);
	m_TimeoutSelector.EnableWindow(FALSE);
	m_NewGroup.EnableWindow(FALSE);
	m_NameEditor.EnableWindow(FALSE);
	m_Name.EnableWindow(FALSE);
	m_TimeoutSelector.SetCurSel(0);
	FillServiceList(0);
    m_SaveButton.EnableWindow(FALSE);
	m_RemoveButton.EnableWindow(FALSE);
	m_PathChanged = FALSE;

	   m_DebugCheck.ShowWindow(TRUE);
	#ifdef _DEBUG
	   m_NameDebugCheck.ShowWindow(TRUE);
    #endif
	
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CServerStartDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CServerStartDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CServerStartDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}


//--------------------------------------------------------------------------------
// This callback will run when the user changed the name of the preference file.
// We will need to read data from the new preference file and fill all edit fields
// with this data.
//--------------------------------------------------------------------------------
void CServerStartDlg::OnBrowsePrefs()  {
CFileDialog openDlg(TRUE,"prefs","default",OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
					"Discover Settings (*.prefs) |*.prefs||",
					this);
CString name;

   if(openDlg.DoModal()==IDOK) {
	   if(m_SaveButton.IsWindowEnabled()==TRUE) {
		   if(MessageBox("You have made changes in the service settings.\n Would you like to save them?","Service Settings",MB_YESNO|MB_ICONEXCLAMATION)==IDYES) {
			   SavePrefs(m_PrefsFile);
		   }
	   }
	   name=openDlg.GetPathName();
	   m_PrefsEdit.SetWindowText(name);
	   m_NameEditor.SetWindowText(name);

	   // Loading the data from the *.prefs file
	   LoadPrefs(name);
	   m_PrefsFile=name;
	   m_NameEditor.SetWindowText(name);
       m_SaveButton.EnableWindow(FALSE);	
	   m_PathChanged = FALSE;
   }
}


//--------------------------------------------------------------------------------
// Callback runs when user push "Browse" button and want to select new Pdf file.
//--------------------------------------------------------------------------------
void CServerStartDlg::OnBrowsePdf()  {
CFileDialog openDlg(TRUE,"prefs","default",OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
					"Discover Project Definition (*.pdf) |*.pdf||",
					this);
   if(openDlg.DoModal()==IDOK) {
	   CString path;
	   path=openDlg.GetPathName();
	   m_PdfEdit.SetWindowText(path);
	   m_SaveButton.EnableWindow(TRUE);
   }
}
//--------------------------------------------------------------------------------


//--------------------------------------------------------------------------------
// This function will ask nameserver for all available pset servers.
//--------------------------------------------------------------------------------
void CServerStartDlg::FillServiceList(int mode) {
NameServerServiceList* services;
const char* servNamePtr;
char host[1024];
CString name;
CString templ;
int service_port,tcp_addr;

    // All valid services must start as "Discover:<host>"
	gethostname(host,1024);
    templ="Discover:";
	templ+=host;
    m_ServiceList.ResetContent();
    NameServerGetListOfServices(&services);
	while(1) {
		if(services==NULL) break;
		servNamePtr = services->getName();
		if(servNamePtr==NULL) break;
		name=servNamePtr;
		if(NameServerGetService(name,service_port,tcp_addr)==1) {
		    if(mode == 0)  {
                if(name.Find(templ)==0)        
                     m_ServiceList.AddString(name);
			} else {
			    if(name.Find("Dislite:")==0) {
				    if(name[(int)strlen("Dislite:")]!='.') {
                        m_ServiceList.AddString(name);
					}
				}
			}
		}
		services=services->getNext();
	}
    

}
//---------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------
// User wants to add service to the list of available services.
//---------------------------------------------------------------------------------------
void CServerStartDlg::OnAdd()  {
CString adminStr;
CString sharedSrcStr;
CString privateSrcStr;
CString sharedModelStr;
CString privateModelStr;
CString prefsStr;
CString pdfStr;
CString homeStr;
int service_port;
int tcp_addr;
int i;
CString command;
CString sname;
STARTUPINFO si;
PROCESS_INFORMATION pi;
CString pmod;

	m_NameEditor.GetWindowText(pmod);
    // Generating the complete path to the BIN directory
	char buf[1024];
	CString serverFileDir;
    if(GetEnvironmentVariable("PSETHOME",buf,1024)!=0) {
	   char shortPath[1024];
	   if(GetShortPathName(buf,shortPath,1024))
			SetEnvironmentVariable("SHORT_PSETHOME",shortPath);
       serverFileDir=buf;
       serverFileDir+="\\Bin\\";
	} else {
       serverFileDir="C:\\Discover\\Bin\\";
	}

    // Getting data from all edit controls
    m_AdminEdit.GetWindowText(adminStr);
	m_SharedSrcEdit.GetWindowText(sharedSrcStr);
	m_PrivateSrcEdit.GetWindowText(privateSrcStr);
	m_SharedModelEdit.GetWindowText(sharedModelStr);
	m_PrivateModelEdit.GetWindowText(privateModelStr);
	m_PrefsEdit.GetWindowText(prefsStr);
	m_PdfEdit.GetWindowText(pdfStr);
	m_HomeEdit.GetWindowText(homeStr);

    // Calculating selected service name
	char host[1024];
	gethostname(host,1024);
	switch(m_ModeSelector.GetCurSel()) {
	    case 0 : sname="Discover:";
	             sname+=host;
	             sname+=":";
	             for(i=0;i<prefsStr.GetLength();i++) {
					 switch(prefsStr[i]) {
					 case '\\':
						 sname+='/';
						 break;
					 case ' ':
						sname += '_';
						break;
					 default:
						 sname+=prefsStr[i];
						 break;
					 }
				 }
				 break;
		case 1 : for(i=0;i<pmod.GetLength();i++) {
					 switch(pmod[i]) {
					 case '\\':
						 sname+='/';
						 break;
					 case ' ':
						sname += '_';
						break;
					 default:
						 sname+=pmod[i];
						 break;
					 }
				 }
				 break;
	}
	CString oldXterm;
	if(m_DebugCheck.GetCheck()==1) {
		buf[0]=0;
	    GetEnvironmentVariable("DISCOVER_XTERM",buf,1024);
		oldXterm = buf;
		CString up = oldXterm;
		up.MakeUpper();
		if(oldXterm!="CONSOLE") {
	        SetEnvironmentVariable("DISCOVER_XTERM","CONSOLE");
		}
	}


    // Checking for the selected service, if is not available or dead
	// starting new service.
	int res;
	switch((res=NameServerGetService(sname,service_port,tcp_addr))) {
	   case 1 : if(m_NameDebugCheck.GetCheck()==1)
		            AfxMessageBox("Can't open Name Server debug window because it is already running.\n You need to kill it first.");
		        AfxMessageBox("The service you want to start is already available\nand will be selected in the available services list.");
	            break;
	   case -1: if(m_NameDebugCheck.GetCheck()==1)
		            command.Format("\"%snameserv\" -debug",serverFileDir);
		        else
		            command.Format("\"%snameserv\"",serverFileDir);
	            memset(&si,0,sizeof(STARTUPINFO));
				si.wShowWindow=m_NameDebugCheck.GetCheck()== 1 ? SW_SHOW : SW_HIDE;
	            si.cb=sizeof(STARTUPINFO);
				if(::CreateProcess(NULL,command.GetBuffer(50),NULL,NULL,FALSE, m_NameDebugCheck.GetCheck()==1 ? 0 : CREATE_NO_WINDOW,NULL,NULL,&si,&pi)==FALSE) {
		            AfxMessageBox("Can't start nameserv.exe.\n Check your path variable.");
		            return;
				}
		        CloseHandle(pi.hThread);
		        CloseHandle(pi.hProcess);
	   case 0 : if((res!=-1) && (m_NameDebugCheck.GetCheck()==1))
	                AfxMessageBox("Can't open Name Server debug window because it is already running.\n You need to kill it first.");
		        SetEnvironmentVariable("ADMINDIR",Parse(adminStr));
	            SetEnvironmentVariable("sharedsrcroot",Parse(sharedSrcStr));
	            SetEnvironmentVariable("privatesrcroot",Parse(privateSrcStr));
	            SetEnvironmentVariable("sharedmodelroot",Parse(sharedModelStr));
	            SetEnvironmentVariable("privatemodelroot",Parse(privateModelStr));
                if(m_ModeSelector.GetCurSel()==0) {
	                if(m_DebugCheck.GetCheck()==1)
	                    command.Format("\"%spset_server\" -prefs \"%s\" -pdf \"%s\" -home %s -server -server_name %s -trace",serverFileDir,Parse(prefsStr),Parse(pdfStr),homeStr,sname);
	                else 
		                command.Format("\"%spset_server\" -prefs \"%s\" -pdf \"%s\" -home %s -server -server_name %s",serverFileDir,Parse(prefsStr),Parse(pdfStr),homeStr,sname);
				} else {
					int timeout;
					CString timetext;
					m_TimeoutSelector.GetLBText(m_TimeoutSelector.GetCurSel(),timetext);
					if(timetext == "Infinite") timeout = -1;
					else timeout       = atoi(timetext);
					if(timeout ==0) timeout = -1;
					if(sname.GetLength()==0) {
	                    AfxMessageBox("Can't start service without name.");
						return;
					}
					sname.Replace(":","_");
					sname.Replace(".","_");
					sname.Replace("/","_");
					sname.Replace("\\","_");
					if(m_DebugCheck.GetCheck()==1)
	                    command.Format("\"%sserverspy\" -prefs \"%s\" -pdf \"%s\" -service %s -debug -timeout %d",serverFileDir,Parse(prefsStr),Parse(pdfStr),sname,timeout);
	                else 
		                command.Format("\"%sserverspy\" -prefs \"%s\" -pdf \"%s\" -service %s -timeout %d",serverFileDir,Parse(prefsStr),Parse(pdfStr),sname,timeout);
					sname="Dislite:"+sname;
				}
	            memset(&si,0,sizeof(STARTUPINFO));
	            si.wShowWindow=SW_HIDE;
	            si.cb=sizeof(STARTUPINFO);
	            if(::CreateProcess(NULL,command.GetBuffer(50),NULL,NULL,FALSE,m_DebugCheck.GetCheck()==1 ? 0 : CREATE_NO_WINDOW,NULL,NULL,&si,&pi)==FALSE) {
	                 AfxMessageBox("Can't find pset_server.exe.\n Check your path variable.");
	                 return;
				}
                // Waiting the pset_server start by asking it's service from
		        // the nameserver.
		        HCURSOR save = SetCursor(LoadCursor(NULL, IDC_WAIT));
				DWORD code;
	            for(i=0;i<30000;i++) {
	                if(NameServerGetService(sname,service_port,tcp_addr)==1) break;
					GetExitCodeProcess(pi.hProcess,&code);
					if(code!=STILL_ACTIVE) {
						i=30000;
						break;
					}
				}
	            CloseHandle(pi.hThread);
	            CloseHandle(pi.hProcess);
		        SetCursor(save);
		        // Testing if everything OK.
				if(i<30000) {
					FillServiceList(m_ModeSelector.GetCurSel()); 
                    m_SelectedService=sname;
                    m_RemoveButton.EnableWindow(TRUE);
				}
	            else AfxMessageBox("Can't start service selected");
				break;
	}
	// Selecting current service in the service list.
	m_ServiceList.SelectString(-1,sname);
	if(m_DebugCheck.GetCheck()==1) {
		if(oldXterm.GetLength()>0) {
	        SetEnvironmentVariable("DISCOVER_XTERM",oldXterm);
		}
	}
}
//---------------------------------------------------------------------------------------


//---------------------------------------------------------------------------------------
// This private function will recursively parse any string with variables inside
// %..% and will return the real string changing all environment variables inside
// %..% into it's real value.
//---------------------------------------------------------------------------------------
CString CServerStartDlg::Parse(CString & src) {
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
  	            if(GetEnvironmentVariable(env,buf,1024)==0) {
			        CString error;
			        error.Format("Undefined variable %s",env);
			        MessageBox(error,"Parsing error",MB_OK|MB_ICONERROR);
			        buf[0]=0;
				}
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
				if(GetEnvironmentVariable(env,buf,1024)==0) {
					CString error;
					error.Format("Undefined variable %s",env);
					MessageBox(error,"Parsing error",MB_OK|MB_ICONERROR);
			        buf[0]=0;
				}
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
  	    if(GetEnvironmentVariable(env,buf,1024)==0) {
			CString error;
			error.Format("Undefined variable %s",env);
			MessageBox(error,"Parsing error",MB_OK|MB_ICONERROR);
			buf[0]=0;
		}
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
//--------------------------------------------------------------------------------




//--------------------------------------------------------------------------------
// This callback will run every time the user selected new service in the service 
// list. It will ping the service specified and will remove the service from the 
// list if it is dead. The currently selected service will be placed into 
// m_SelectedService member variable and will be used as a default service when
// starting the client.
//--------------------------------------------------------------------------------
void CServerStartDlg::OnServiceSelected()  {
CString service;
int i;

	int sel=m_ServiceList.GetCurSel();
	if(sel>=0) {
		m_ServiceList.GetText(sel,service);
	    int service_port;
	    int tcp_addr;
		HCURSOR save = SetCursor(LoadCursor(NULL, IDC_WAIT));
		for(i=0;i<10;i++) {
		   if(NameServerGetService(service,service_port,tcp_addr)==1) break;
		}
		SetCursor(save);
		if(i==10) {
		   AfxMessageBox("Selected service is no longer available\n and will be removed from the service list.");
		   m_SelectedService="";
           FillServiceList(m_ModeSelector.GetCurSel());	
		} else {
			m_SelectedService=service;
	        m_RemoveButton.EnableWindow(TRUE);
		}

	}
}
//--------------------------------------------------------------------------------


//--------------------------------------------------------------------------------
// This callbacks will run when user changed something in the edit fields. The
// only task is to set the status of the "Save" button to "Enable".
//--------------------------------------------------------------------------------
void CServerStartDlg::OnChangeAdmin()  {
   m_SaveButton.EnableWindow(TRUE);	
}
void CServerStartDlg::OnChangePdf()  {
   m_SaveButton.EnableWindow(TRUE);	
}

void CServerStartDlg::OnChangeSharedmodel()  {
   m_SaveButton.EnableWindow(TRUE);	
}

void CServerStartDlg::OnChangePrivatemodel()  {
   m_SaveButton.EnableWindow(TRUE);	
}

void CServerStartDlg::OnChangePrivatesource()  {
   m_SaveButton.EnableWindow(TRUE);	
}

void CServerStartDlg::OnChangeSharedsource()  {
   m_SaveButton.EnableWindow(TRUE);	
}

void CServerStartDlg::OnChangeHome() {
   m_SaveButton.EnableWindow(TRUE);	
}
//--------------------------------------------------------------------------------


//--------------------------------------------------------------------------------
// If the user changed the name of the preference file we need to ask him to save
// changes, if any, in the old preference file.
//--------------------------------------------------------------------------------
void CServerStartDlg::OnPrefsEntered() {
CString prefsFile;

	if(m_PathChanged==TRUE) {
	    m_PrefsEdit.GetWindowText(prefsFile);
		prefsFile = Parse(prefsFile);
	    m_PrefsEdit.SetWindowText(prefsFile);
	    m_NameEditor.SetWindowText(prefsFile);
		CFile prefs;
		CFileException openEx;
		if(prefs.Open(prefsFile,CFile::modeRead|CFile::shareDenyNone,&openEx)==TRUE) {
			prefs.Close();
			m_PrefsFile = prefsFile;
            LoadPrefs(m_PrefsFile); 
		    m_SaveButton.EnableWindow(FALSE);	
		} else {
			TCHAR szError[1024];
			openEx.GetErrorMessage(szError, 1024);

			MessageBox(szError,"File opening error", MB_ICONSTOP | MB_OK);
			m_PrefsEdit.SetWindowText(m_PrefsFile);
	        m_NameEditor.SetWindowText(m_PrefsFile);
		}
		m_PathChanged = FALSE;
	}
}
//--------------------------------------------------------------------------------


//--------------------------------------------------------------------------------
// If the user changed the name of the preference file we need to ask him to save
// changes, if any, in the old preference file.
//--------------------------------------------------------------------------------
void CServerStartDlg::OnChangePrefs() {
   if(m_SaveButton.IsWindowEnabled()==TRUE) {
	   if(MessageBox("You have made changes in the service settings.\n Would you like to save them?","Service Settings",MB_YESNO|MB_ICONEXCLAMATION)==IDYES) {
		   SavePrefs(m_PrefsFile);
	   } else {
		   m_SaveButton.EnableWindow(FALSE);	
	   }
   }
   m_PathChanged = TRUE;
}
//--------------------------------------------------------------------------------

//--------------------------------------------------------------------------------
// This private function will load all values from the *.prefs file and will set
// all edit controls using this values.
//--------------------------------------------------------------------------------
void CServerStartDlg::LoadPrefs(CString & filename) {
const char *pszValue = NULL;

       Settings *new_settings_instance = new Settings("psetPrefs");
       int res=new_settings_instance->open(filename);
       Settings::Item pdf_item("pdfFileUser");
       if( new_settings_instance->read(pdf_item) != -1 ) {
		   if(pdf_item.read(&pszValue)!=-1 && pszValue) {
               m_PdfEdit.SetWindowText(pszValue);
		   }
	   }
       Settings::Item home_item("projectHome");
       if( new_settings_instance->read(home_item) != -1 ) {
		   if(home_item.read(&pszValue)!=-1 && pszValue) {
               m_HomeEdit.SetWindowText(pszValue);
		   }
	   }

       Settings::Item admin_item("ADMINDIR");
       if( new_settings_instance->read(admin_item) != -1 ) {
		   if(admin_item.read(&pszValue)!=-1 && pszValue) {
               m_AdminEdit.SetWindowText(pszValue);
		   }
	   }
       Settings::Item sharedsrc_item("sharedSrcRoot");
       if( new_settings_instance->read(sharedsrc_item) != -1 ) {
		   if(sharedsrc_item.read(&pszValue)!=-1 && pszValue) {
               m_SharedSrcEdit.SetWindowText(pszValue);
		   }
	   }
       Settings::Item privatesrc_item("privateSrcRoot");
       if( new_settings_instance->read(privatesrc_item) != -1 ) {
		   if(privatesrc_item.read(&pszValue)!=-1 && pszValue) {
               m_PrivateSrcEdit.SetWindowText(pszValue);
		   }
	   }
       Settings::Item sharedmodel_item("sharedModelRoot");
       if( new_settings_instance->read(sharedmodel_item) != -1 ) {
		   if(sharedmodel_item.read(&pszValue)!=-1 && pszValue) {
               m_SharedModelEdit.SetWindowText(pszValue);
		   }
	   }
       Settings::Item privatemodel_item("privateModelRoot");
       if( new_settings_instance->read(privatemodel_item) != -1 ) {
		   if(privatemodel_item.read(&pszValue)!=-1 && pszValue) {
               m_PrivateModelEdit.SetWindowText(pszValue);
		   }
	   }
	   new_settings_instance->close(0);
	   delete new_settings_instance;
}
//-------------------------------------------------------------------------------

//-------------------------------------------------------------------------------
// This private function will read all values from the edit controls and will 
// save this values in the *.prefs file.
//-------------------------------------------------------------------------------
void CServerStartDlg::SavePrefs(CString & filename) {
CString adminStr;
CString sharedSrcStr;
CString privateSrcStr;
CString sharedModelStr;
CString privateModelStr;
CString prefsStr;
CString pdfStr;
CString homeStr;

    // Getting data from all edit controls
    m_AdminEdit.GetWindowText(adminStr);
	m_SharedSrcEdit.GetWindowText(sharedSrcStr);
	m_PrivateSrcEdit.GetWindowText(privateSrcStr);
	m_SharedModelEdit.GetWindowText(sharedModelStr);
	m_PrivateModelEdit.GetWindowText(privateModelStr);
	m_PrefsEdit.GetWindowText(prefsStr);
   	m_PdfEdit.GetWindowText(pdfStr);
	m_HomeEdit.GetWindowText(homeStr);

       Settings *new_settings_instance = new Settings("psetPrefs");
       int res=new_settings_instance->open(filename);

       Settings::Item pdf_item("pdfFileUser");
       if( new_settings_instance->read(pdf_item) != -1 ) {
		   pdf_item.write(pdfStr);
		   new_settings_instance->write(pdf_item);
	   }

       Settings::Item home_item("projectHome");
       if( new_settings_instance->read(home_item) != -1 ) {
		   home_item.write(homeStr);
		   new_settings_instance->write(home_item);
	   }


       Settings::Item admin_item("ADMINDIR");
       if( new_settings_instance->read(admin_item) != -1 ) {
		   admin_item.write(adminStr);
		   new_settings_instance->write(admin_item);
	   }

       Settings::Item sharedsrc_item("sharedSrcRoot");
       if( new_settings_instance->read(sharedsrc_item) != -1 ) {
		   sharedsrc_item.write(sharedSrcStr);
		   new_settings_instance->write(sharedsrc_item);
	   }


       Settings::Item privatesrc_item("privateSrcRoot");
       if( new_settings_instance->read(privatesrc_item) != -1 ) {
		   privatesrc_item.write(privateSrcStr);
		   new_settings_instance->write(privatesrc_item);
	   }


       Settings::Item sharedmodel_item("sharedModelRoot");
       if( new_settings_instance->read(sharedmodel_item) != -1 ) {
		   sharedmodel_item.write(sharedModelStr);
		   new_settings_instance->write(sharedmodel_item);
	   }


       Settings::Item privatemodel_item("privateModelRoot");
       if( new_settings_instance->read(privatemodel_item) != -1 ) {
		   privatemodel_item.write(privateModelStr);
		   new_settings_instance->write(privatemodel_item);
	   }
	   new_settings_instance->close(1);
	   delete new_settings_instance;
}
//-------------------------------------------------------------------------------

void CServerStartDlg::OnSave()  {
	SavePrefs(m_PrefsFile);
    m_SaveButton.EnableWindow(FALSE);	
}

void CServerStartDlg::OnOK() {

	// We need to ask user to save his changes in the *prefs file.
    if(m_SaveButton.IsWindowEnabled()==TRUE) {
	   if(MessageBox("You have made changes in the service settings.\n Would you like to save them?","Service Settings",MB_YESNO|MB_ICONEXCLAMATION)==IDYES) {
		   SavePrefs(m_PrefsFile);
	   }
    }
    CDialog::OnOK();
}


//------------------------------------------------------------------------------------------
// This callback will run when user press "Remove" button. It will send Access 
// "stop_server -y" command to the selected server.
//------------------------------------------------------------------------------------------
void CServerStartDlg::OnRemove()  {
int service_port;
int tcp_addr;
CString proxy_name;

    switch(m_ModeSelector.GetCurSel()) {
        case 0 :    // Will send the command only if selected service exists.
                    if(NameServerGetService(m_SelectedService,service_port,tcp_addr)==1) {
	                    int socket = ConnectToServer(service_port,tcp_addr);
                        if(socket!=-1) {
                            CString command("stop_server -y");
                            DISCommand RPCcommand(command.GetBuffer(20),RESULT_BATCH_CMD_TYPE);
                            SendCommand(socket,&RPCcommand);
	                        closesocket(socket);
	                        HCURSOR save = SetCursor(LoadCursor(NULL, IDC_WAIT));
                            for(int i=0;i<400;i++) {
	                           if(NameServerGetService(m_SelectedService,service_port,tcp_addr)!=1) break;
							}
			                SetCursor(save);
						}
					}
					break;
		case 1 :    // Will send stop command to the proxy and to the server
			        proxy_name = "Proxy:"+m_SelectedService.Right(m_SelectedService.GetLength()-strlen("Dislite:"));
                    if(NameServerGetService(proxy_name,service_port,tcp_addr)==1) {
	                    int socket = ConnectToServer(service_port,tcp_addr);
                        if(socket!=-1) {
							CString command = "stop";
                            send(socket,command,strlen(command),0);
	                        closesocket(socket);
	                        HCURSOR save = SetCursor(LoadCursor(NULL, IDC_WAIT));
                            for(int i=0;i<400;i++) {
	                           if(NameServerGetService(proxy_name,service_port,tcp_addr)!=1) break;
							}
			                SetCursor(save);
						}
					} else {
                        if(NameServerGetService(m_SelectedService,service_port,tcp_addr)==1) {
	                        int socket = ConnectToServer(service_port,tcp_addr);
                            if(socket!=-1) {
                                CString command("stop_server -y");
                                DISCommand RPCcommand(command.GetBuffer(20),RESULT_BATCH_CMD_TYPE);
                                SendCommand(socket,&RPCcommand);
	                            closesocket(socket);
	                            HCURSOR save = SetCursor(LoadCursor(NULL, IDC_WAIT));
                                for(int i=0;i<400;i++) {
	                                if(NameServerGetService(m_SelectedService,service_port,tcp_addr)!=1) break;
								}
			                    SetCursor(save);
							}
						}
					}
					break;
	}
	int sel = m_ServiceList.GetCurSel();
	if(sel!=-1) {
		m_ServiceList.DeleteString(sel);
	}
}
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
// Callback runs when user switches the page between local and remote services.
//------------------------------------------------------------------------------------------
void CServerStartDlg::OnPageChanged(NMHDR* pNMHDR, LRESULT* pResult)  {
	switch(m_ModeSelector.GetCurSel()) {
	    case 0 : m_Timeout.EnableWindow(FALSE);
			     m_TimeoutSelector.EnableWindow(FALSE);
				 m_NewGroup.EnableWindow(FALSE);
				 m_NameEditor.EnableWindow(FALSE);
				 m_Name.EnableWindow(FALSE);
	             FillServiceList(0);
				 break;
	    case 1 : m_Timeout.EnableWindow(TRUE);
			     m_TimeoutSelector.EnableWindow(TRUE);
				 m_NewGroup.EnableWindow(TRUE);
				 m_NameEditor.EnableWindow(TRUE);
				 m_Name.EnableWindow(TRUE);
	             FillServiceList(1);
				 break;
	}
	*pResult = 0;
}
//------------------------------------------------------------------------------------------
