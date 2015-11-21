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
// CMIntegratorCtl.cpp : Implementation of the CCMIntegratorCtrl ActiveX Control class.

#include "stdafx.h"
#include "CMIntegrator.h"
#include "CMIntegratorCtl.h"
#include "CMIntegratorPpg.h"

#include "nameServCalls.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


IMPLEMENT_DYNCREATE(CCMIntegratorCtrl, COleControl)


/////////////////////////////////////////////////////////////////////////////
// Message map

BEGIN_MESSAGE_MAP(CCMIntegratorCtrl, COleControl)
	//{{AFX_MSG_MAP(CCMIntegratorCtrl)
	// NOTE - ClassWizard will add and remove message map entries
	//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG_MAP
	ON_OLEVERB(AFX_IDS_VERB_PROPERTIES, OnProperties)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// Dispatch map

BEGIN_DISPATCH_MAP(CCMIntegratorCtrl, COleControl)
	//{{AFX_DISPATCH_MAP(CCMIntegratorCtrl)
	DISP_PROPERTY_EX(CCMIntegratorCtrl, "CMName", GetCMName, SetCMName, VT_BSTR)
	DISP_FUNCTION(CCMIntegratorCtrl, "GetCMs", GetCMs, VT_BSTR, VTS_NONE)
	DISP_FUNCTION(CCMIntegratorCtrl, "GetCommands", GetCommands, VT_BSTR, VTS_NONE)
	DISP_FUNCTION(CCMIntegratorCtrl, "Connect", Connect, VT_BOOL, VTS_BSTR)
	DISP_FUNCTION(CCMIntegratorCtrl, "IsConnected", IsConnected, VT_BOOL, VTS_NONE)
	DISP_FUNCTION(CCMIntegratorCtrl, "IsAvailable", IsAvailable, VT_BOOL, VTS_NONE)
	DISP_FUNCTION(CCMIntegratorCtrl, "GetAttributes", GetAttributes, VT_BSTR, VTS_NONE)
	DISP_FUNCTION(CCMIntegratorCtrl, "ExecCommand", ExecCommand, VT_BSTR, VTS_BSTR)
	DISP_FUNCTION(CCMIntegratorCtrl, "Reconnect", Reconnect, VT_BOOL, VTS_NONE)
	//}}AFX_DISPATCH_MAP
END_DISPATCH_MAP()


/////////////////////////////////////////////////////////////////////////////
// Event map

BEGIN_EVENT_MAP(CCMIntegratorCtrl, COleControl)
	//{{AFX_EVENT_MAP(CCMIntegratorCtrl)
	// NOTE - ClassWizard will add and remove event map entries
	//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_EVENT_MAP
END_EVENT_MAP()


/////////////////////////////////////////////////////////////////////////////
// Property pages

// TODO: Add more property pages as needed.  Remember to increase the count!
BEGIN_PROPPAGEIDS(CCMIntegratorCtrl, 1)
	PROPPAGEID(CCMIntegratorPropPage::guid)
END_PROPPAGEIDS(CCMIntegratorCtrl)


/////////////////////////////////////////////////////////////////////////////
// Initialize class factory and guid

IMPLEMENT_OLECREATE_EX(CCMIntegratorCtrl, "CMINTEGRATOR.CMIntegratorCtrl.1",
	0x3ca23651, 0xb2f, 0x4e42, 0x9c, 0x35, 0x94, 0x1d, 0xcb, 0xc9, 0x54, 0x80)


/////////////////////////////////////////////////////////////////////////////
// Type library ID and version

IMPLEMENT_OLETYPELIB(CCMIntegratorCtrl, _tlid, _wVerMajor, _wVerMinor)


/////////////////////////////////////////////////////////////////////////////
// Interface IDs

const IID BASED_CODE IID_DCMIntegrator =
		{ 0x29fea4ae, 0x16b9, 0x4a8d, { 0xb9, 0xbe, 0xbf, 0xb, 0x63, 0xa9, 0xe7, 0x2b } };
const IID BASED_CODE IID_DCMIntegratorEvents =
		{ 0x298aab4, 0x98df, 0x41cf, { 0x92, 0xd4, 0x83, 0xa5, 0x3, 0xc8, 0x37, 0x11 } };


/////////////////////////////////////////////////////////////////////////////
// Control type information

static const DWORD BASED_CODE _dwCMIntegratorOleMisc =
	OLEMISC_SETCLIENTSITEFIRST |
	OLEMISC_INSIDEOUT |
	OLEMISC_CANTLINKINSIDE |
	OLEMISC_RECOMPOSEONRESIZE;

IMPLEMENT_OLECTLTYPE(CCMIntegratorCtrl, IDS_CMINTEGRATOR, _dwCMIntegratorOleMisc)


/////////////////////////////////////////////////////////////////////////////
// CCMIntegratorCtrl::CCMIntegratorCtrlFactory::UpdateRegistry -
// Adds or removes system registry entries for CCMIntegratorCtrl

BOOL CCMIntegratorCtrl::CCMIntegratorCtrlFactory::UpdateRegistry(BOOL bRegister)
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
			IDS_CMINTEGRATOR,
			IDB_CMINTEGRATOR,
			afxRegApartmentThreading,
			_dwCMIntegratorOleMisc,
			_tlid,
			_wVerMajor,
			_wVerMinor);
	else
		return AfxOleUnregisterClass(m_clsid, m_lpszProgID);
}


/////////////////////////////////////////////////////////////////////////////
// CCMIntegratorCtrl::CCMIntegratorCtrl - Constructor

CCMIntegratorCtrl::CCMIntegratorCtrl()
{
	InitializeIIDs(&IID_DCMIntegrator, &IID_DCMIntegratorEvents);
	m_szIntegratorName = "Generic";
	m_bConnected = false;
	ConnectToService(true);
}


/////////////////////////////////////////////////////////////////////////////
// CCMIntegratorCtrl::~CCMIntegratorCtrl - Destructor

CCMIntegratorCtrl::~CCMIntegratorCtrl()
{
	// TODO: Cleanup your control's instance data here.
}


/////////////////////////////////////////////////////////////////////////////
// CCMIntegratorCtrl::OnDraw - Drawing function

void CCMIntegratorCtrl::OnDraw(
			CDC* pdc, const CRect& rcBounds, const CRect& rcInvalid)
{
	// TODO: Replace the following code with your own drawing code.
	pdc->FillRect(rcBounds, CBrush::FromHandle((HBRUSH)GetStockObject(WHITE_BRUSH)));
	RECT rcText = rcBounds;
	pdc->DrawText(CString("CM Integrator Control"),&rcText,DT_NOCLIP|DT_CENTER|DT_WORDBREAK|DT_END_ELLIPSIS);
}


/////////////////////////////////////////////////////////////////////////////
// CCMIntegratorCtrl::DoPropExchange - Persistence support

void CCMIntegratorCtrl::DoPropExchange(CPropExchange* pPX)
{
	ExchangeVersion(pPX, MAKELONG(_wVerMinor, _wVerMajor));
	COleControl::DoPropExchange(pPX);

	// TODO: Call PX_ functions for each persistent custom property.

}


/////////////////////////////////////////////////////////////////////////////
// CCMIntegratorCtrl::OnResetState - Reset control to default state

void CCMIntegratorCtrl::OnResetState()
{
	COleControl::OnResetState();  // Resets defaults found in DoPropExchange

	// TODO: Reset any other control state here.
}


/////////////////////////////////////////////////////////////////////////////
// CCMIntegratorCtrl message handlers

void CCMIntegratorCtrl::OnIntegratorEvent() {
}

BSTR CCMIntegratorCtrl::GetCMName() 
{
	CString strResult;
	if(IsConnected()) strResult = m_szCMName;
	return strResult.AllocSysString();
}

void CCMIntegratorCtrl::SetCMName(LPCTSTR lpszNewValue) 
{
	Connect(lpszNewValue);
	SetModifiedFlag();
}

CString CCMIntegratorCtrl::GetIntegratorType() {
	return "CM";
}

CString CCMIntegratorCtrl::GetIntegratorName() {
	return m_szIntegratorName;
}

bool CCMIntegratorCtrl::RunDriver() {

    // Get "our" home directory.
	char *pset = getenv("PSETHOME");
	CString psetHomeEnv;
	if(pset == NULL) {
        psetHomeEnv = "C:\\Discover\\";
	} else {
        psetHomeEnv = pset;
        // Add trailing slash if needed
        int len = strlen(pset);
        if(pset[len - 1] != '\\') {
            psetHomeEnv += "\\";
        }
    }

    // This is the program to start.
	CString execName;
	execName = "\"" + psetHomeEnv + "bin\\cmdriver.exe" + "\"";

	// Add path to our libraries to %PATH%.
	char* path = getenv("PATH");
    CString pathEnv;
    if(path == NULL) {
        pathEnv.Format("PATH=%slib;", psetHomeEnv);
    } else {
        pathEnv.Format("PATH=%s;%slib;", path, psetHomeEnv);
    }
	_putenv((LPCTSTR)pathEnv);

    // Start 'cmdriver'.
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
    memset(&si,0,sizeof(STARTUPINFO));
    si.wShowWindow= SW_HIDE;
	si.cb=sizeof(STARTUPINFO);
	if(::CreateProcess(NULL,execName.GetBuffer(50),NULL,NULL,FALSE,CREATE_NO_WINDOW,NULL,NULL,&si,&pi)==FALSE) {
		 return false;
	}
	CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);

	CString serviceName =  GetServiceName();
	int port;
	int address;
	int nCount = LOOP_COUNT;
	int nRes = 0;

    //while((nRes=NameServerGetService(serviceName,port,address))!=1 && (--nCount)>0);

    while ( ( nRes = NameServerGetService( serviceName, port, address ) ) != 1 && ( --nCount > 0 ) )
    {
        Sleep( ( LOOP_COUNT - nCount ) * 200 + ( rand() % 100 ) );
    }

	return nRes==1;
}

BSTR CCMIntegratorCtrl::GetCMs() 
{
	CString strResult;
	if(IsAvailable())
		if(m_ConnectionChannel->SendCommand(CString("get_cm_systems")))
			strResult = m_ConnectionChannel->GetReply();
	return strResult.AllocSysString();
}

BSTR CCMIntegratorCtrl::GetCommands() 
{
	CString strResult;
	if(IsAvailable()) {
		if(m_ConnectionChannel->SendCommand(CString("get_commands")))
			strResult = m_ConnectionChannel->GetReply();
	}
	return strResult.AllocSysString();
}

BOOL CCMIntegratorCtrl::Connect(LPCTSTR szName) 
{
	if(IsAvailable()) {
		CString szCommand = "connect\t";
		szCommand+=szName;
		if(m_ConnectionChannel->SendCommand(szCommand)) {
			CString strResult = m_ConnectionChannel->GetReply();
			m_bConnected = strResult.IsEmpty() || strResult.CompareNoCase("done")==0;
			if(m_bConnected) m_szCMName = szName;
			return m_bConnected;
		}
	}
	return false;
}

BOOL CCMIntegratorCtrl::IsConnected() 
{
	return m_bConnected;
}

BOOL CCMIntegratorCtrl::IsAvailable() 
{
	return m_ConnectionChannel!=NULL;
}

BSTR CCMIntegratorCtrl::GetAttributes() 
{
	CString strResult;
	if(IsAvailable()) {
		if(m_ConnectionChannel->SendCommand(CString("get_attributes")))
			strResult = m_ConnectionChannel->GetReply();
	}
	return strResult.AllocSysString();
}

BSTR CCMIntegratorCtrl::ExecCommand(LPCTSTR szCommand) 
{
	CString strResult;
	if(IsConnected()) {
		if(m_ConnectionChannel->SendCommand(CString(szCommand))) {
			strResult = m_ConnectionChannel->GetReply();
			if(strResult.CompareNoCase("Error")==0) {
				strResult = "Error:"+m_ConnectionChannel->GetReply();
			}
		} else
			strResult = "Error:Communication error.";
	}
	return strResult.AllocSysString();
}

BOOL CCMIntegratorCtrl::Reconnect() 
{
	return ConnectToService(true);
}
