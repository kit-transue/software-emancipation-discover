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
// CodeRoverIntegratorCtl.cpp : Implementation of the CCodeRoverIntegratorCtrl ActiveX Control class.

#include "stdafx.h"
#include "CodeRoverIntegrator.h"
#include "CodeRoverIntegratorCtl.h"
#include "CodeRoverIntegratorPpg.h"
#include "IntegratorSocket.h"
#include "nameServCalls.h"
#include "io.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


IMPLEMENT_DYNCREATE(CCodeRoverIntegratorCtrl, COleControl)


/////////////////////////////////////////////////////////////////////////////
// Message map

BEGIN_MESSAGE_MAP(CCodeRoverIntegratorCtrl, COleControl)
	//{{AFX_MSG_MAP(CCodeRoverIntegratorCtrl)
	// NOTE - ClassWizard will add and remove message map entries
	//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG_MAP
	ON_OLEVERB(AFX_IDS_VERB_PROPERTIES, OnProperties)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// Dispatch map

BEGIN_DISPATCH_MAP(CCodeRoverIntegratorCtrl, COleControl)
	//{{AFX_DISPATCH_MAP(CCodeRoverIntegratorCtrl)
	DISP_PROPERTY_EX(CCodeRoverIntegratorCtrl, "EditorName", GetEditorName, SetEditorName, VT_BSTR)
	DISP_PROPERTY_EX(CCodeRoverIntegratorCtrl, "EditorsList", GetEditorsList, SetEditorsList, VT_BSTR)
	DISP_FUNCTION(CCodeRoverIntegratorCtrl, "MakeSelection", MakeSelection, VT_EMPTY, VTS_BSTR VTS_I4 VTS_I4 VTS_I4)
	DISP_FUNCTION(CCodeRoverIntegratorCtrl, "OpenFile", OpenFile, VT_EMPTY, VTS_BSTR VTS_I4)
	DISP_FUNCTION(CCodeRoverIntegratorCtrl, "SetInfo", SetInfo, VT_EMPTY, VTS_BSTR)
	DISP_FUNCTION(CCodeRoverIntegratorCtrl, "Reconnect", Reconnect, VT_BOOL, VTS_NONE)
	DISP_FUNCTION(CCodeRoverIntegratorCtrl, "GetModelLine", GetModelLine, VT_I4, VTS_BSTR VTS_BSTR VTS_I4)
	//}}AFX_DISPATCH_MAP
END_DISPATCH_MAP()


/////////////////////////////////////////////////////////////////////////////
// Event map

BEGIN_EVENT_MAP(CCodeRoverIntegratorCtrl, COleControl)
	//{{AFX_EVENT_MAP(CCodeRoverIntegratorCtrl)
	EVENT_CUSTOM("CtrlClick", FireCtrlClick, VTS_BSTR  VTS_I4  VTS_I4  VTS_I4)
	EVENT_CUSTOM("Query", FireQuery, VTS_BSTR  VTS_I4  VTS_I4  VTS_I4)
	EVENT_CUSTOM("Definition", FireDefinition, VTS_BSTR  VTS_I4  VTS_I4  VTS_I4)
	EVENT_CUSTOM("Activate", FireActivate, VTS_NONE)
	//}}AFX_EVENT_MAP
END_EVENT_MAP()


/////////////////////////////////////////////////////////////////////////////
// Property pages

// TODO: Add more property pages as needed.  Remember to increase the count!
BEGIN_PROPPAGEIDS(CCodeRoverIntegratorCtrl, 1)
	PROPPAGEID(CCodeRoverIntegratorPropPage::guid)
END_PROPPAGEIDS(CCodeRoverIntegratorCtrl)


/////////////////////////////////////////////////////////////////////////////
// Initialize class factory and guid

IMPLEMENT_OLECREATE_EX(CCodeRoverIntegratorCtrl, "CODEROVERINTEGRATOR.CodeRoverIntegratorCtrl.1",
	0x8dbae788, 0xdb45, 0x11d4, 0x8e, 0x41, 0, 0x1, 0x2, 0x39, 0x3, 0x21)


/////////////////////////////////////////////////////////////////////////////
// Type library ID and version

IMPLEMENT_OLETYPELIB(CCodeRoverIntegratorCtrl, _tlid, _wVerMajor, _wVerMinor)


/////////////////////////////////////////////////////////////////////////////
// Interface IDs

const IID BASED_CODE IID_DCodeRoverIntegrator =
		{ 0x8dbae786, 0xdb45, 0x11d4, { 0x8e, 0x41, 0, 0x1, 0x2, 0x39, 0x3, 0x21 } };
const IID BASED_CODE IID_DCodeRoverIntegratorEvents =
		{ 0x8dbae787, 0xdb45, 0x11d4, { 0x8e, 0x41, 0, 0x1, 0x2, 0x39, 0x3, 0x21 } };


/////////////////////////////////////////////////////////////////////////////
// Control type information

static const DWORD BASED_CODE _dwCodeRoverIntegratorOleMisc =
	OLEMISC_INVISIBLEATRUNTIME |
	OLEMISC_SETCLIENTSITEFIRST |
	OLEMISC_INSIDEOUT |
	OLEMISC_CANTLINKINSIDE |
	OLEMISC_RECOMPOSEONRESIZE;

IMPLEMENT_OLECTLTYPE(CCodeRoverIntegratorCtrl, IDS_CODEROVERINTEGRATOR, _dwCodeRoverIntegratorOleMisc)


/////////////////////////////////////////////////////////////////////////////
// CCodeRoverIntegratorCtrl::CCodeRoverIntegratorCtrlFactory::UpdateRegistry -
// Adds or removes system registry entries for CCodeRoverIntegratorCtrl

BOOL CCodeRoverIntegratorCtrl::CCodeRoverIntegratorCtrlFactory::UpdateRegistry(BOOL bRegister)
{
	// TODO: Verify that your control follows apartment-model threading rules.
	// Refer to MFC TechNote 64 for more information.
	// If your control does not conform to the apartment-model rules, then
	// you must modify the code below, changing the 6th parameter from
	// afxRegApartmentThreading to 0.

	if (bRegister)
		return AfxOleRegisterControlClass(
			AfxGetInstanceHandle(),
			m_clsid,
			m_lpszProgID,
			IDS_CODEROVERINTEGRATOR,
			IDB_CODEROVERINTEGRATOR,
			afxRegApartmentThreading,
			_dwCodeRoverIntegratorOleMisc,
			_tlid,
			_wVerMajor,
			_wVerMinor);
	else
		return AfxOleUnregisterClass(m_clsid, m_lpszProgID);
}


/////////////////////////////////////////////////////////////////////////////
// CCodeRoverIntegratorCtrl::CCodeRoverIntegratorCtrl - Constructor

CCodeRoverIntegratorCtrl::CCodeRoverIntegratorCtrl()
{
	InitializeIIDs(&IID_DCodeRoverIntegrator, &IID_DCodeRoverIntegratorEvents);
	AllIntegratedEditorsList();
	m_EditorName = "Microsoft Visual Studio";
    ConnectToService(false);
}


/////////////////////////////////////////////////////////////////////////////
// CCodeRoverIntegratorCtrl::~CCodeRoverIntegratorCtrl - Destructor

CCodeRoverIntegratorCtrl::~CCodeRoverIntegratorCtrl()
{
	POSITION pos = m_EditorsList.GetStartPosition();
	void* pEditorInfo = NULL;
	CString editorName;
	while(pos) {
	    m_EditorsList.GetNextAssoc(pos, editorName, pEditorInfo);
		if(pEditorInfo!=NULL) delete pEditorInfo;
	}
}

CString CCodeRoverIntegratorCtrl::SendRequestGetReply(CString request) {
	SendCommand(request);
	return GetReply();
}

/////////////////////////////////////////////////////////////////////////////
// CCodeRoverIntegratorCtrl::OnDraw - Drawing function

void CCodeRoverIntegratorCtrl::OnDraw(
			CDC* pdc, const CRect& rcBounds, const CRect& rcInvalid)
{
	// TODO: Replace the following code with your own drawing code.
	pdc->FillRect(rcBounds, CBrush::FromHandle((HBRUSH)GetStockObject(WHITE_BRUSH)));
	pdc->Ellipse(rcBounds);
}


/////////////////////////////////////////////////////////////////////////////
// CCodeRoverIntegratorCtrl::DoPropExchange - Persistence support

void CCodeRoverIntegratorCtrl::DoPropExchange(CPropExchange* pPX)
{
	ExchangeVersion(pPX, MAKELONG(_wVerMinor, _wVerMajor));
	COleControl::DoPropExchange(pPX);

	// TODO: Call PX_ functions for each persistent custom property.

}


/////////////////////////////////////////////////////////////////////////////
// CCodeRoverIntegratorCtrl::OnResetState - Reset control to default state

void CCodeRoverIntegratorCtrl::OnResetState()
{
	COleControl::OnResetState();  // Resets defaults found in DoPropExchange

	// TODO: Reset any other control state here.
}

/////////////////////////////////////////////////////////////////////////////
// CCodeRoverIntegratorCtrl message handlers

BSTR CCodeRoverIntegratorCtrl::GetEditorName()  {
	CString strResult;
	// TODO: Add your property handler here
	strResult = m_EditorName;
	return strResult.AllocSysString();
}

void CCodeRoverIntegratorCtrl::SetEditorName(LPCTSTR lpszNewValue)  {
    m_EditorName = lpszNewValue;
	ConnectToService(false);
	SetModifiedFlag();
}


void CCodeRoverIntegratorCtrl::MakeSelection(LPCTSTR filename, long line, long column, long len)  {    
CString query;
    if(m_EditorName!="Microsoft Visual Studio") {
        OpenFile(filename,line);
		return;
	} 
    if(m_ConnectionChannel==NULL) 	ConnectToService(true);
	if(m_ConnectionChannel) {
        query.Format("mark\t%s\t%d\t%d\t%d",filename,line,column,len);
		if(SendCommand(query)==false) {
			ConnectToService(true);
			if(SendCommand(query)==false) {
				if(m_ConnectionChannel==NULL) {
				    m_ConnectionChannel->Close();
				    delete m_ConnectionChannel;
				    m_ConnectionChannel=NULL;
				}
				return;
			}
		}
	    CString reply = GetReply();
    }

}

void CCodeRoverIntegratorCtrl::OpenFile(LPCTSTR filename, long line)  {
CString query;
    if(m_ConnectionChannel==NULL) 	ConnectToService(true);
	if(m_ConnectionChannel) {
        query.Format("open\t%s\t%d",filename,line);
		if(SendCommand(query)==false) {
			ConnectToService(true);
			if(SendCommand(query)==false) {
				if(m_ConnectionChannel!=NULL) {
				    m_ConnectionChannel->Close();
				    delete m_ConnectionChannel;
				    m_ConnectionChannel=NULL;
				}
				return;
			}
		}
	    CString reply = GetReply();
    }
}

void CCodeRoverIntegratorCtrl::SetInfo(LPCTSTR text)  {
    if(m_ConnectionChannel==NULL) 	ConnectToService(true);
	if(m_ConnectionChannel) {
	    CString query;
        query.Format("info\t%s",text);
		if(SendCommand(query)==false) {
			ConnectToService(true);
			if(SendCommand(query)==false) {
				if(m_ConnectionChannel!=NULL) {
				    m_ConnectionChannel->Close();
				    delete m_ConnectionChannel;
				    m_ConnectionChannel=NULL;
				}
				return;
			}
		}
	    CString reply = GetReply();
    }
}

bool CCodeRoverIntegratorCtrl::RunDriver() {
CString execName;
void* pEditorInfo = NULL;
STARTUPINFO si;
PROCESS_INFORMATION pi;
int i;

    m_EditorsList.Lookup(m_EditorName, pEditorInfo);
	if(pEditorInfo == NULL) return false;

	execName = ((StringPair*)pEditorInfo)->szValue;

    if(execName.GetLength()==0) return false;

    memset(&si,0,sizeof(STARTUPINFO));
    si.wShowWindow= SW_HIDE;
	si.cb=sizeof(STARTUPINFO);
    for(i=0;i<execName.GetLength();i++) {
	  if(execName.GetAt(i)=='\\')
		  execName.SetAt(i,'/');
	}

	if(::CreateProcess(NULL,(char*)(LPCTSTR)execName,NULL,NULL,TRUE,CREATE_NO_WINDOW,NULL,NULL,&si,&pi)==FALSE) {
		LPVOID lpMsgBuf;
		if(FormatMessage( 
			FORMAT_MESSAGE_ALLOCATE_BUFFER | 
			FORMAT_MESSAGE_FROM_SYSTEM | 
			FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			GetLastError(),
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
			(LPTSTR) &lpMsgBuf,
			0,
			NULL)) {
			// Display the string.
			MessageBox((LPCTSTR)lpMsgBuf, "Error", MB_OK | MB_ICONERROR);

			// Free the buffer.
			LocalFree( lpMsgBuf );
		}
		return false;
	}

	int port;
	int addr;
	CString service = GetServiceName();
    for(i=0;i<120;i++) {
       if(NameServerGetService(service,port,addr)==1) break;
	   Sleep(500);
	}
	if(i==120) {
		return false;
    } 
	CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
	return true;
}


/////////////////////////////////////////////////////////////////////////////
// CCodeRoverIntegratorCtrl::AllIntegratedEditorsList - Make a list of all
// editors that are integrated

void CCodeRoverIntegratorCtrl::AllIntegratedEditorsList()
{
/*    //Setup environment
    CString homeName;
    char* psethomeEnv = getenv("PSETHOME");
    if(psethomeEnv==NULL) {
		homeName="C:\\Discover\\";
    } else {
		homeName=psethomeEnv;
    }
    CString crHomeEnv;
    crHomeEnv.Format("CODE_ROVER_HOME=%s",homeName);
    putenv(crHomeEnv);
  
    //emacs exec command
    CString emacsCommand=homeName + "\\bin\\emacsclient.exe";

	// gvim execution command
	CString gvimCommand="\"" + homeName + "\\bin\\gvim.exe\" -g";

    //JBuilder exec command
    CString jBuilderCommand;
    char* jBuilderHome = getenv("CR_JBUILDER_PATH");
    if(jBuilderHome != NULL) {
		jBuilderCommand+=jBuilderHome;
		jBuilderCommand+="\\bin\\";
    }
    jBuilderCommand+="JBuilderW.exe";

    //First add the default editors supported by us to the list
    m_EditorsList.SetAt("Microsoft Visual Studio", "msdev.exe");
    m_EditorsList.SetAt("Emacs", emacsCommand);
    m_EditorsList.SetAt("JBuilder", jBuilderCommand);
    m_EditorsList.SetAt("gvim", gvimCommand);
*/
    //Now see if users have added any new editors
    AddUserIntegratedEditors();
}

/////////////////////////////////////////////////////////////////////////////
// CCodeRoverIntegratorCtrl::AddUserIntegratedEditorsList - Add the editors that
// users integrated by reading file %USERPROFILE%\integratedEditors.txt. The
// first field on each line is name of the editor(quotes around name if it has spaces)
// and the rest of the line is treated as command to be exec'ed to launch the editor.

void CCodeRoverIntegratorCtrl::AddUserIntegratedEditors()
{
	const int nBuffMax = 1024;
	char readBuff[nBuffMax];

    char* userProfileDir = getenv("USERPROFILE");
    if (userProfileDir != NULL) {
		CString editorConfigFilename = userProfileDir;
		editorConfigFilename += "\\integratedEditors.txt";
		// if file does not exist - try to grab it from %PSETHOME%/lib - installer should create it
		if(_access((LPCTSTR)editorConfigFilename,0)) {
			char* psethome = getenv("PSETHOME");
			if(psethome != NULL) {
				CString psethomeCfgFile = psethome;
				psethomeCfgFile += "\\lib\\integratedEditors.txt";
				if(!_access(psethomeCfgFile,0)) {
					FILE* cfgFile = fopen(psethomeCfgFile,"r");
					if(cfgFile) {
						FILE *ecFilePtr = fopen(editorConfigFilename, "w+");
						while(fgets(readBuff, nBuffMax, cfgFile) != NULL) {
							fputs(readBuff,ecFilePtr);
						}
						fclose(ecFilePtr);
						fclose(cfgFile);
					}	
				} else
					return;
			} else 
				return;
		}

		FILE *ecFilePtr = fopen(editorConfigFilename, "r");
		if (ecFilePtr) {
			CString line;
			while (fgets(readBuff, nBuffMax, ecFilePtr) != NULL) {
				line = readBuff;
				line.TrimLeft();line.TrimRight();
				int line_len = line.GetLength();
				int nItem = 0;
				bool bInQuotes = false;
				CString tmpElement;
				CString editorName;
				CString editorID;
				CString editorCommand;
				for(int i=0;i<line_len && nItem<3;i++) {
					if(line[i]==' ' && !bInQuotes) {
						switch(nItem) {
						case 0: editorName = tmpElement; break;
						case 1: 
							editorID = tmpElement;
							nItem++;
							editorCommand = line.Mid(i+1);
							break;
						}
						nItem++;
						tmpElement.Empty();
					} else {
						if(line[i]=='"') {
							bInQuotes = !bInQuotes;
						} else {
							tmpElement += line[i];
						}
					}
				}
				m_EditorsList.SetAt(editorName, new StringPair(editorID,editorCommand));
			}
			fclose(ecFilePtr);
		}
    }

}

bool CCodeRoverIntegratorCtrl::SendCommand(CString &command) {
	if(m_ConnectionChannel==false) return false;
	return m_ConnectionChannel->SendCommand(command);
}

CString CCodeRoverIntegratorCtrl::GetReply() {
	if(m_ConnectionChannel==false) return "";
	return m_ConnectionChannel->GetReply();

}

void CCodeRoverIntegratorCtrl::OnIntegratorEvent() {
	if(m_ConnectionChannel==false) return;
    CString event = m_ConnectionChannel->GetEvent();
	CString szCmd;
	CString szFile;
	CString szOffset;
	CString szLine;
	CString szColumn;

	// remove leading and trailing whitespaces
	event.TrimLeft();
	event.TrimRight();

	// parse the event
	int nEventLen = event.GetLength();
	int nParamEnd = 0;
	int nParamStart = 0;
	int nToken = 0;

	while(nParamStart>=0 && nParamStart<nEventLen) {
		nParamEnd = event.Find((TCHAR)'\t',nParamStart);
		if(nParamEnd==-1) nParamEnd = nEventLen;
		switch(nToken) {
		case 0: szCmd = event.Mid(nParamStart,nParamEnd-nParamStart); break;
		case 1: szFile = event.Mid(nParamStart,nParamEnd-nParamStart); break;
		case 2: szOffset = event.Mid(nParamStart,nParamEnd-nParamStart); break;
		case 3: szLine = event.Mid(nParamStart,nParamEnd-nParamStart); break;
		case 4: szColumn = event.Mid(nParamStart,nParamEnd-nParamStart); break;
		}
		nParamStart = nParamEnd + 1;
		nToken++;
	}

	int nLine = atoi((LPCTSTR)szLine);
	int nColumn = atoi((LPCTSTR)szColumn);

	if(szCmd=="popup") { 
		FireActivate(); 
		return;
	} else {
		if(szCmd=="inst_def") {
			FireCtrlClick(szFile,nLine,nColumn,1);
			return;
		} else {
			if(szCmd=="open_def") {
				FireDefinition(szFile,nLine,nColumn,1);
				return;
			} else {
				if(szCmd=="query") {
					FireQuery(szFile,nLine,nColumn,1);
					return;
				} else {
					if(szCmd=="instances") {
					}			
				}
			}
		}
	}
}


BOOL CCodeRoverIntegratorCtrl::Reconnect()  {

	return ConnectToService(false);
}

CString CCodeRoverIntegratorCtrl::GetIntegratorType() {
	return "Editor";
}

CString CCodeRoverIntegratorCtrl::GetIntegratorName() {
	void* pEditorInfo = NULL;
	CString szName;
	m_EditorsList.Lookup(m_EditorName,pEditorInfo);
	if(pEditorInfo!=NULL)
		szName = ((StringPair*)pEditorInfo)->szKey;

	return szName;
}

long CCodeRoverIntegratorCtrl::GetModelLine(LPCTSTR pszLocalName, LPCTSTR pszBaseName, long nLine) 
{
	CString query;
	CString reply = "-1";
    if(m_ConnectionChannel==NULL) 	ConnectToService(true);
	if(m_ConnectionChannel) {
		query.Format("private_line_to_model_line\t%s\t%s\t%ld",pszLocalName,pszBaseName,nLine);
		if(SendCommand(query)==false) {
			ConnectToService(true);
			if(SendCommand(query)==false) {
				if(m_ConnectionChannel!=NULL) {
				    m_ConnectionChannel->Close();
				    delete m_ConnectionChannel;
				    m_ConnectionChannel=NULL;
				}
				return -1;
			}
		}
	    reply = GetReply();
    }
	return atol((LPCTSTR)reply);
}


BSTR CCodeRoverIntegratorCtrl::GetEditorsList() 
{
	CString strResult;
	// TODO: Add your property handler here
	
	//build a coma separated editor list
	CString editorName;

	POSITION pos = m_EditorsList.GetStartPosition();
	void* pEditorInfo = NULL;
	while(pos) {
	    m_EditorsList.GetNextAssoc(pos, editorName, pEditorInfo);
		if(strResult.GetLength()>0)
			strResult += ",";	    
	    strResult += editorName;
	}
	return strResult.AllocSysString();
}

void CCodeRoverIntegratorCtrl::SetEditorsList(LPCTSTR lpszNewValue) 
{
	// TODO: Add your property handler here

	//dummy, do nothing
}
