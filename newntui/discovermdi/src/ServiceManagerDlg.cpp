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
// ServiceManagerDlg.cpp : implementation file
//
#include "stdafx.h"
#include "discovermdi.h"
#include "mainfrm.h"
#include "ServiceManagerDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CServiceManagerDlg dialog
const int CServiceManagerDlg::MODEL_ROOT=4;
const int CServiceManagerDlg::PSET_RUNNING=3;
const int CServiceManagerDlg::MODEL_RUNNING=2;
const int CServiceManagerDlg::NOT_RUNNING=1;
const int CServiceManagerDlg::MODEL=0;

CServiceManagerDlg::CServiceManagerDlg(CWnd* pParent /*=NULL*/, CString* pszRepository /*=NULL*/)
	: CDialog(CServiceManagerDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CServiceManagerDlg)
	//}}AFX_DATA_INIT
	m_lpszRepositoryNameTooltip = NULL;
	m_szSelectedService = "";
	m_szRepository = *pszRepository;
}

void CServiceManagerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CServiceManagerDlg)
	DDX_Control(pDX, IDC_REFRESHMODELS, m_RefreshBtn);
	DDX_Control(pDX, IDC_REPOSITORYNAME, m_RepositoryEditCtrl);
	DDX_Control(pDX, IDC_BROWSEMODEL, m_BrowseBtn);
	DDX_Control(pDX, IDC_CLOSEMANAGER, m_CloseBtn);
	DDX_Control(pDX, IDC_STOPMODEL, m_StopBtn);
	DDX_Control(pDX, IDC_STARTMODEL, m_StartBtn);
	DDX_Control(pDX, IDC_AVAILABLEMODELS, m_ModelsTreeCtrl);
	DDX_Control(pDX, IDC_SELECTREPOSITORY, m_SelectRepositoryBtn);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CServiceManagerDlg, CDialog)
	//{{AFX_MSG_MAP(CServiceManagerDlg)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_NOTIFY( TVN_SELCHANGED, IDC_AVAILABLEMODELS, OnModelVersionChanged )
	ON_NOTIFY_EX( TTN_NEEDTEXT, 0, OnToolTipNotify )
	ON_BN_CLICKED(IDC_SELECTREPOSITORY, OnSelectRepository)
	ON_MESSAGE(WM_RELOADMODELS, OnReloadModels)
	ON_MESSAGE(WM_BROWSEMODEL, OnBrowseModelMSG)
	ON_BN_CLICKED(IDC_BROWSEMODEL, OnBrowseModel)
	ON_BN_CLICKED(IDC_CLOSEMANAGER, OnCloseManager)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_STARTMODEL, OnStartModel)
	ON_BN_CLICKED(IDC_REFRESHMODELS, OnRefreshModels)
	ON_BN_CLICKED(IDC_STOPMODEL, OnStopModel)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CServiceManagerDlg message handlers

BOOL CServiceManagerDlg::OnToolTipNotify( UINT id, NMHDR * pNMHDR, LRESULT * pResult )
{
    TOOLTIPTEXT *pTTT = (TOOLTIPTEXT *)pNMHDR;
    UINT nID =pNMHDR->idFrom;
    if (pTTT->uFlags & TTF_IDISHWND)
    {
        // idFrom is actually the HWND of the tool
        nID = ::GetDlgCtrlID((HWND)nID);
        if(nID)
        {
			if(nID!=IDC_REPOSITORYNAME)
				pTTT->lpszText = MAKEINTRESOURCE(nID);
			else {
				int nLen = m_RepositoryEditCtrl.GetWindowTextLength();
				if(nLen > 0) {
					if(m_lpszRepositoryNameTooltip!=NULL) delete m_lpszRepositoryNameTooltip;
					m_lpszRepositoryNameTooltip = new TCHAR[nLen+1];
					m_RepositoryEditCtrl.GetWindowText(m_lpszRepositoryNameTooltip,nLen+1);
					pTTT->lpszText = m_lpszRepositoryNameTooltip;
				} else
					pTTT->lpszText = NULL;
			}
            pTTT->hinst = AfxGetResourceHandle();
            return(TRUE);
        }
    }
    return(FALSE);
}

BOOL CServiceManagerDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	EnableToolTips(TRUE);

	m_StopBtn.EnableWindow(FALSE);
	m_StartBtn.EnableWindow(FALSE);
	m_BrowseBtn.EnableWindow(FALSE);
	m_RefreshBtn.EnableWindow(FALSE);

	m_ModelsTreeCtrl.CreateImages();

	SetRepositoryPath(m_szRepository);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CServiceManagerDlg::OnPaint() 
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
HCURSOR CServiceManagerDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

// User selected model and wants to browse it
void CServiceManagerDlg::OnOK() 
{
	// TODO: Add extra cleanup here

	CDialog::OnOK();
}

// User just exits from dialog and does not want to do anything
void CServiceManagerDlg::OnCancel() 
{
	// TODO: Add extra cleanup here
	
	CDialog::OnCancel();
}

// A Callback procedure to set a defauld starting directory (using a BFFM_SETSELECTION message)
// in the select repository dialog 
int CALLBACK BrowseCallbackProc(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData) {
    char szBuffer[MAX_PATH];

    switch( uMsg ) {
	case BFFM_INITIALIZED:
		GetCurrentDirectory(MAX_PATH, szBuffer);
		SendMessage(hwnd, BFFM_SETSELECTION, (WPARAM)TRUE, (LPARAM)szBuffer);
		break;
	default:
		break;
    }

    return 0;
}

void CServiceManagerDlg::OnSelectRepository() 
{
    BROWSEINFO bi = { 0 };
    bi.lpszTitle = _T("Select a new repository:");
	bi.hwndOwner = this->GetSafeHwnd();
	bi.ulFlags = BIF_RETURNONLYFSDIRS;
	bi.lpfn = BrowseCallbackProc;

    char szCWD[MAX_PATH];
	GetCurrentDirectory(MAX_PATH,szCWD);
	SetCurrentDirectory(GetRepositoryPath());

    LPITEMIDLIST pidl = SHBrowseForFolder(&bi);
    if ( pidl != 0 )
    {
        // get the name of the folder
        TCHAR path[MAX_PATH];
        if ( SHGetPathFromIDList ( pidl, path ) )
        {
			SetRepositoryPath(CString(path));
        }

        // free memory used
        IMalloc * imalloc = 0;
		if ( SUCCEEDED( SHGetMalloc ( &imalloc )) )
        {
            imalloc->Free ( pidl );
            imalloc->Release ( );
        }
    }
	SetCurrentDirectory(szCWD);
}

LRESULT CServiceManagerDlg::OnBrowseModelMSG(WPARAM wParam, LPARAM lParam) {
	OnBrowseModel();
	return 1;
}

LRESULT CServiceManagerDlg::OnReloadModels(WPARAM wParam, LPARAM lParam)
{
	ReloadModels();
	RefreshModels();
	return 1;
}

void CServiceManagerDlg::RecursiveUpdateNodes(HTREEITEM hParent, bool validState) {
	HTREEITEM hCurrent = m_ModelsTreeCtrl.GetChildItem(hParent);
	while(hCurrent!=NULL) {
		ValidateNode(hCurrent,validState);
		if(m_ModelsTreeCtrl.ItemHasChildren(hCurrent)) 
			RecursiveUpdateNodes(hCurrent, validState);
		hCurrent = m_ModelsTreeCtrl.GetNextItem(hCurrent, TVGN_NEXT);
	}
}

void CServiceManagerDlg::InvalidateModels() {
	HTREEITEM rootItem = m_ModelsTreeCtrl.GetRootItem();
	RecursiveUpdateNodes(rootItem,false);
}

void CServiceManagerDlg::ValidateModels() {
	HTREEITEM rootItem = m_ModelsTreeCtrl.GetRootItem();
	RecursiveUpdateNodes(rootItem,true);
}

void CServiceManagerDlg::RecursiveDeleteNodes(HTREEITEM hParent) {
   HTREEITEM hChildItem = m_ModelsTreeCtrl.GetChildItem(hParent);

   while (hChildItem != NULL) {
      HTREEITEM hNextItem = m_ModelsTreeCtrl.GetNextItem(hChildItem, TVGN_NEXT);
	  if(!IsNodeValid(hChildItem))
		m_ModelsTreeCtrl.DeleteItem(hChildItem);
	  else {
		if(m_ModelsTreeCtrl.ItemHasChildren(hChildItem)) 
			RecursiveDeleteNodes(hChildItem);
	  }
      hChildItem = hNextItem;
   }
}

void CServiceManagerDlg::DeleteInvalidModels() {
	HTREEITEM rootItem = m_ModelsTreeCtrl.GetRootItem();
	RecursiveDeleteNodes(rootItem);
}

HTREEITEM CServiceManagerDlg::NodeExists(HTREEITEM hParent, CString& szName) {
	HTREEITEM hCurrent = m_ModelsTreeCtrl.GetChildItem(hParent);
	while(hCurrent!=NULL) {
		CString szCurrentName = m_ModelsTreeCtrl.GetItemText(hCurrent);
		if(!szCurrentName.Compare(szName)) return hCurrent;
		hCurrent = m_ModelsTreeCtrl.GetNextItem(hCurrent, TVGN_NEXT);
	}
   	return NULL;
}

void CServiceManagerDlg::ValidateNode(HTREEITEM hNode, bool validState) {
	DWORD dwData = 0;
	if(!validState) dwData=-1;
	m_ModelsTreeCtrl.SetItemData(hNode,dwData);
}

bool CServiceManagerDlg::IsNodeValid(HTREEITEM hNode) {
	return m_ModelsTreeCtrl.GetItemData(hNode)==0;
}

void CServiceManagerDlg::ReloadModels() {
	CString cszTemplateSuffix = "Template";
	const int nSuffixLen = cszTemplateSuffix.GetLength();
	CString szTemplatesPath = GetRepositoryPath()+"\\template";
	CString szModelsPath = GetRepositoryPath()+"\\model";
	CFileFind modelFinder;

	InvalidateModels();

	HTREEITEM rootItem = m_ModelsTreeCtrl.GetRootItem();

	BOOL bModelsAvailable = modelFinder.FindFile(szTemplatesPath+"\\*.*");
	while(bModelsAvailable) {
		bModelsAvailable = modelFinder.FindNextFile();
		if(modelFinder.IsDots()) continue;
		if(modelFinder.IsDirectory()) {
			CString szTemplateName = modelFinder.GetFileName();
			if(szTemplateName.Find(cszTemplateSuffix)>=0) {
				CString szModelName = szTemplateName.Left(szTemplateName.GetLength()-nSuffixLen);
				if(szModelName.Compare("Model")) {
					CString szModelPath = szModelsPath+"\\"+szModelName;
					HTREEITEM modelItem = NodeExists(rootItem,szModelName);
					if(modelItem==NULL)
						modelItem = m_ModelsTreeCtrl.InsertItem(szModelName,MODEL,MODEL,rootItem);
					ValidateNode(modelItem);
					CFileFind versionFinder;

					BOOL bVersionsAvailable = versionFinder.FindFile(szModelPath+"\\*.*");
					while(bVersionsAvailable) {
						bVersionsAvailable=versionFinder.FindNextFile();
						if(versionFinder.IsDots()) continue;
						if(versionFinder.IsDirectory()) {
							CString szVersion = versionFinder.GetFileName();
							HTREEITEM versionItem = NodeExists(modelItem,szVersion);
							if(versionItem==NULL)
								versionItem = m_ModelsTreeCtrl.InsertItem(szVersion,NOT_RUNNING,NOT_RUNNING,modelItem);
							ValidateNode(versionItem);
						}
					}
					m_ModelsTreeCtrl.Expand(modelItem,TVE_EXPAND);
				}
			}
		}
	}
	
	m_ModelsTreeCtrl.Expand(rootItem,TVE_EXPAND);

	DeleteInvalidModels();

	ValidateModels();

	m_RefreshBtn.EnableWindow(TRUE);
}

bool CServiceManagerDlg::loadID(CString& szRepository, CString& szID) {
	CString szPrjName = szRepository + "\\repository.rpj";
	szID.Empty();
	TRY { 
		CStdioFile repPrj(szPrjName,CFile::typeText|CFile::modeRead);
		CFileStatus prjStat;
		repPrj.GetStatus(prjStat);
		if(repPrj.GetStatus(prjStat)) {
			CString szLine;
			while(repPrj.ReadString(szLine)) {
				if(szLine[0]!='#') {
					int nCommentIdx = szLine.Find("#");
					if(nCommentIdx!=-1)
						szLine = szLine.Mid(0,nCommentIdx-1);
					int nVarNameEnd = szLine.Find("=");
					if(nVarNameEnd!=-1) {
						CString szVarName = szLine.Mid(0,nVarNameEnd);
						szVarName.TrimLeft();
						szVarName.TrimRight();
						CString szVarValue = szLine.Right(szLine.GetLength()-nVarNameEnd-1);
						if(!szVarName.Compare("ID"))  {
							szID = szVarValue;
							szID.TrimLeft();
							szID.TrimRight();
							szID += ":";
							break;
						}
					}
				}
			}
			repPrj.Close();
		}
	} 
	CATCH (CFileException, pFe) {
		// just ignore the error - we support old-fashioned repositories
		// pFe->ReportError(MB_OK|MB_ICONERROR);
	} 
	END_CATCH
    return !szID.IsEmpty();
}

void CServiceManagerDlg::SetRepositoryPath(CString& szPath)
{
	if(m_RepositoryEditCtrl.GetSafeHwnd()!=NULL && !szPath.IsEmpty()) {
		loadID(szPath,m_szRepositoryID);
		m_szRepository = szPath;
		m_RepositoryEditCtrl.SetWindowText(szPath);
		m_ModelsTreeCtrl.DeleteAllItems();
		m_ModelsTreeCtrl.InsertItem(GetRepositoryPath(),MODEL_ROOT,MODEL_ROOT);
		PostMessage(WM_RELOADMODELS);
	}
}

CString& CServiceManagerDlg::GetRepositoryPath()
{
	return m_szRepository;
}

bool CServiceManagerDlg::IsModelRunning(CString &szModel, CString &szVersion)
{
	if(!(szModel.IsEmpty()||szVersion.IsEmpty())) {
		CString szService = "Dislite:"+m_szRepositoryID+szModel+"_"+szVersion;
		return IsServiceRunning(szService);
	}
	return false;
}

bool CServiceManagerDlg::IsPSETRunning(CString &szModel, CString &szVersion)
{
	if(!(szModel.IsEmpty()||szVersion.IsEmpty())) {
		CString szService = "Discover:"+m_szRepositoryID+szModel+"_"+szVersion;
		return IsServiceRunning(szService);
	}
	return false;
}

CString CServiceManagerDlg::GetServiceName()
{
	return m_szSelectedService;
}

void CServiceManagerDlg::OnBrowseModel() 
{
	CString szModel,szVersion;

	m_szSelectedService = "";

	GetSelectedModelAndVersion(szModel,szVersion);
	if(!(szModel.IsEmpty()||szVersion.IsEmpty())) {
		HTREEITEM hSelectedItem = m_ModelsTreeCtrl.GetSelectedItem();
		if(hSelectedItem!=NULL) {
			int nImage,nSelectedImage;
			m_ModelsTreeCtrl.GetItemImage(hSelectedItem,nImage,nSelectedImage);
			switch(nImage) {
			case PSET_RUNNING:
				m_szSelectedService = "Discover:";
				break;
			case MODEL_RUNNING:
				m_szSelectedService = "Dislite:";
				break;
			default:
				return;
			}
		}
		m_szSelectedService += m_szRepositoryID+szModel+"_"+szVersion;
		OnOK();
	}
}

void CServiceManagerDlg::OnCloseManager() 
{
	m_szSelectedService = "";
	OnCancel();
}

void CServiceManagerDlg::OnDestroy() 
{
	CDialog::OnDestroy();
	
	if(m_lpszRepositoryNameTooltip!=NULL) {
		delete m_lpszRepositoryNameTooltip;
		m_lpszRepositoryNameTooltip = NULL;
	}
}

void CServiceManagerDlg::OnModelVersionChanged(NMHDR* pNMHdr, LRESULT* pResult) 
{
	UpdateControls();
	*pResult = 0;
}

bool CServiceManagerDlg::IsServiceRunning(CString &szService)
{
	int nPort,nAddress;
	return NameServerGetService(szService, nPort, nAddress)==1;
}

void CServiceManagerDlg::OnStartModel() 
{
	CString szModel, szVersion;
	GetSelectedModelAndVersion(szModel,szVersion);
	StartCompletionProvider provider(this,szModel,szVersion);

	if(!(szModel.IsEmpty()||szVersion.IsEmpty())) {
		CString szCmd;
		CString szParams;

		CString szDISIM = GetRepositoryPath();
		CString szScriptName;

		char* pszPSETHOME = getenv("PSETHOME");

		if(CDiscoverMDIApp::IsNT()) {
			DWORD dwLen = strlen(pszPSETHOME)+1;
			char* pszShortPSETHOME = new char[dwLen];
			GetShortPathName(pszPSETHOME,pszShortPSETHOME,dwLen);
			szScriptName = pszShortPSETHOME;
			szScriptName += "\\bin\\NTPSETServerAdmin.bat";

			if(szDISIM.Find(" ")>=0) {
				DWORD nDisimLen = szDISIM.GetLength()+1;
				char* pszShortDISIM = new char[nDisimLen];
				GetShortPathName(szDISIM,pszShortDISIM,nDisimLen);
				szDISIM = pszShortDISIM;
				delete pszShortDISIM;
			}
			szCmd = "cmd.exe";
			szParams.Format("/C %s -mstart -v%s/%s -D\"%s\"",(char*)(LPCTSTR)szScriptName,
											  (char*)(LPCTSTR)szModel,
											  (char*)(LPCTSTR)szVersion,
											  (char*)(LPCTSTR)szDISIM);
			delete pszShortPSETHOME;
		} else {
			szScriptName = pszPSETHOME;
			szScriptName += "\\bin\\NTPSETServerAdmin.bat";
			if(szScriptName.Find(" ")>=0) szScriptName = "\"" + szScriptName + "\"";
			if(szDISIM.Find(" ")>=0) szDISIM = "\"" + szDISIM + "\"";
			szCmd.Format("%s",(char*)(LPCTSTR)szScriptName);
			szParams.Format("-mstart -v%s/%s -D%s",(char*)(LPCTSTR)szModel,
											  (char*)(LPCTSTR)szVersion,
											  (char*)(LPCTSTR)szDISIM);
		}		
		ServiceAdmin(szCmd,szParams,_T("Starting the model ")+szModel+"/"+szVersion+_T("..."),&provider);
		RefreshModels();
	}
}

bool CServiceManagerDlg::ServiceAdmin(CString& szCmd, CString& szParams, CString& szMessage, CCompletionProvider* pProvider)
{
	HANDLE hProcess = CDiscoverMDIApp::ShellOpenExecute(szCmd,szParams,false);

    // Wait until child process exits.
	if(hProcess!=NULL) {
		CWaitDlg dlg(hProcess,this);
		dlg.SetMessage(szMessage);
		dlg.SetCompletionProvider(pProvider);
		dlg.Wait(120000); // wait for two minutes at most
		// Close process handle. 
		CloseHandle( hProcess );
	}

	return true;
}

void CServiceManagerDlg::GetSelectedModelAndVersion(CString &szModel, CString &szVersion)
{
	szModel.Empty();
	szVersion.Empty();

	HTREEITEM hSelectedItem = m_ModelsTreeCtrl.GetSelectedItem();
	if(hSelectedItem!=NULL) {
		szVersion = m_ModelsTreeCtrl.GetItemText(hSelectedItem);
		HTREEITEM hModelItem = m_ModelsTreeCtrl.GetParentItem(hSelectedItem);
		if(hModelItem!=NULL) {
			szModel = m_ModelsTreeCtrl.GetItemText(hModelItem);
		}
	}
}

void CServiceManagerDlg::OnRefreshModels() 
{
	ReloadModels();
	RefreshModels();
}

void CServiceManagerDlg::RefreshModels() 
{
	NameServerServiceList* services;
	CString szModel,szVersion;
	CString szPSETService,szService;

    NameServerGetListOfServices(&services);

	HTREEITEM hRootItem = m_ModelsTreeCtrl.GetRootItem();
	if(hRootItem!=NULL) {
		HTREEITEM hModelItem = m_ModelsTreeCtrl.GetChildItem(hRootItem);
		if(hModelItem!=NULL) {
			do {
				szModel = m_ModelsTreeCtrl.GetItemText(hModelItem);
				HTREEITEM hVersionItem = m_ModelsTreeCtrl.GetChildItem(hModelItem);
				if(hVersionItem!=NULL) {
					do {
						szVersion = m_ModelsTreeCtrl.GetItemText(hVersionItem);
						szPSETService = "Discover:"+m_szRepositoryID+szModel+"_"+szVersion;
						szService = "Dislite:"+m_szRepositoryID+szModel+"_"+szVersion;
						NameServerServiceList* pTemp = services;
						bool bPSETUpdated = false;
						bool bUpdated = false;
						while(pTemp!=NULL && !bPSETUpdated) {
							CString szRegisteredName = pTemp->getName();
							if(szPSETService==szRegisteredName) { // if pset_server is running with this
																  // model	version - we display it running
								m_ModelsTreeCtrl.SetItemImage(hVersionItem,PSET_RUNNING,PSET_RUNNING);
								bPSETUpdated = true;
							} else {
								if(szService==szRegisteredName) { // if model_server is running with this 
																  // model version - we display it running
									m_ModelsTreeCtrl.SetItemImage(hVersionItem,MODEL_RUNNING,MODEL_RUNNING);
									bUpdated = true;
								} 
							}
							pTemp=pTemp->getNext();
						}
						if(!bUpdated && !bPSETUpdated){
							// if none of the servers is running with this model 
							// version - we display it as existing but not available for 
							// browsing
							m_ModelsTreeCtrl.SetItemImage(hVersionItem,NOT_RUNNING,NOT_RUNNING);
						}
					}while((hVersionItem = m_ModelsTreeCtrl.GetNextSiblingItem(hVersionItem))!=NULL);
				}
			}while((hModelItem = m_ModelsTreeCtrl.GetNextSiblingItem(hModelItem))!=NULL);
		}
	}
	delete services;
	UpdateControls();
}

void CServiceManagerDlg::OnStopModel() 
{
	CString szModel, szVersion;
	int nSecs = 100;
	char* pszPSETHOME = getenv("PSETHOME");
	CString szDISIM = GetRepositoryPath();

	HTREEITEM hSelectedItem = m_ModelsTreeCtrl.GetSelectedItem();
	if(hSelectedItem!=NULL) {
		GetSelectedModelAndVersion(szModel,szVersion);
		if(!(szModel.IsEmpty()||szVersion.IsEmpty())) {
			StopCompletionProvider provider(this,szModel,szVersion);
			int nImage,nSelectedImage;
			m_ModelsTreeCtrl.GetItemImage(hSelectedItem,nImage,nSelectedImage);
			CString szCmd;
			CString szParams;
			
			CDiscoverMDIApp* app = (CDiscoverMDIApp *) AfxGetApp();
			CString szCurrentService=app->m_ServiceName;
			CString szSelectedService = "Discover:"+m_szRepositoryID+szModel + "_" + szVersion;

			switch(nImage) {
			default:
			case NOT_RUNNING:
				return; // do nothing
			case PSET_RUNNING:
				if(!szCurrentService.IsEmpty() && !szCurrentService.Compare(szSelectedService)) {
					AfxGetMainWnd()->SendMessage(WM_SERVICE_DISCONNECT);
				}
				if(CDiscoverMDIApp::IsNT()) {
					DWORD dwLen = strlen(pszPSETHOME)+1;
					char* pszShortPSETHOME = new char[dwLen];
					GetShortPathName(pszPSETHOME,pszShortPSETHOME,dwLen);
					CString szScriptName = pszShortPSETHOME;
					szScriptName += "\\bin\\NTPSETServerAdmin.bat";

					if(szDISIM.Find(" ")>=0) {
						DWORD nDisimLen = szDISIM.GetLength()+1;
						char* pszShortDISIM = new char[nDisimLen];
						GetShortPathName(szDISIM,pszShortDISIM,nDisimLen);
						szDISIM = pszShortDISIM;
						delete pszShortDISIM;
					}
					szCmd = "cmd.exe";
					szParams.Format("/C %s -mstop -v%s/%s -D\"%s\"",(char*)(LPCTSTR)szScriptName,
													  (char*)(LPCTSTR)szModel,
													  (char*)(LPCTSTR)szVersion,
													  (char*)(LPCTSTR)szDISIM);
					delete pszShortPSETHOME;
				} else {
					CString szScriptName = pszPSETHOME;
					szScriptName += "\\bin\\NTPSETServerAdmin.bat";
					if(szScriptName.Find(" ")>=0) szScriptName = "\"" + szScriptName + "\"";
					if(szDISIM.Find(" ")>=0) szDISIM = "\"" + szDISIM + "\"";
					szCmd.Format("%s",(char*)(LPCTSTR)szScriptName);
					szParams.Format("-mstop -v%s/%s -D%s",(char*)(LPCTSTR)szModel,
													  (char*)(LPCTSTR)szVersion,
													  (char*)(LPCTSTR)szDISIM);
				}
				ServiceAdmin(szCmd,szParams,_T("Stopping the model ")+szModel+"/"+szVersion+_T("..."), &provider);
				break;
			case MODEL_RUNNING:
				break;
			}
		
			RefreshModels();
		}
	}
}

void CServiceManagerDlg::UpdateControls()
{
	HTREEITEM hSelectedItem = m_ModelsTreeCtrl.GetSelectedItem();
	int nImage,nSelectedImage;
	if(m_ModelsTreeCtrl.GetItemImage(hSelectedItem,nImage,nSelectedImage)) {
		switch(nImage) {
		default:
			m_StopBtn.EnableWindow(FALSE);
			m_StartBtn.EnableWindow(FALSE);
			m_BrowseBtn.EnableWindow(FALSE);
			break;
		case NOT_RUNNING:
			m_StopBtn.EnableWindow(FALSE);
			m_StartBtn.EnableWindow(TRUE);
			m_BrowseBtn.EnableWindow(FALSE);
			break;
		case PSET_RUNNING:
			m_StopBtn.EnableWindow(TRUE);
			m_StartBtn.EnableWindow(FALSE);
			m_BrowseBtn.EnableWindow(TRUE);
			break;
		case MODEL_RUNNING:
			m_StopBtn.EnableWindow(FALSE);
			m_StartBtn.EnableWindow(TRUE);
			m_BrowseBtn.EnableWindow(TRUE);
			break;
		}
	} else {
		m_StopBtn.EnableWindow(FALSE);
		m_StartBtn.EnableWindow(FALSE);
		m_BrowseBtn.EnableWindow(FALSE);
	}
}
