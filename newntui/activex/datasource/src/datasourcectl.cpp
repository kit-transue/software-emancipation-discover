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
// DataSourceCtl.cpp : Implementation of the CDataSourceCtrl OLE control class.


#include <fcntl.h>      /* Needed only for _O_RDWR definition */
#include <stdlib.h>
#include <stdio.h>
#include "stdafx.h"
#include <afxsock.h>
#include "SocketComm.h"
#include "nameServCalls.h"
#include "DataSource.h"
#include "DataSourceCtl.h"
#include "DataSourcePpg.h"
#include <io.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


static BOOL m_ChannelOpened = FALSE;
static int  m_Address=0;
static int  m_Port=0;
static int  m_ServerLock=FALSE;
static CString lastName;
static BOOL ServerBusy=FALSE;

static int ref_count = 0;
static CRITICAL_SECTION g_CriticalSection;

IMPLEMENT_DYNCREATE(CDataSourceCtrl, COleControl)

CConnection CDataSourceCtrl::m_ServerConnection;
CConnection CDataSourceCtrl::m_ReservedServerConnection;

CNotificationCreator CDataSourceCtrl::m_NotificationCreator;
CNotificationRecever CDataSourceCtrl::m_NotificationRecever;

//---------------------------------------------------------------------------------------
// Runs when server try to conect to the DataSource using notification channel.
// (this hapens after evaluation of the add_client_notificator Access function).
// This function will call Accept to create new socket connected to the server
// notification socket, 
//---------------------------------------------------------------------------------------
void CNotificationCreator::OnAccept( int nErrorCode ) {
	int res = Accept(CDataSourceCtrl::m_NotificationRecever);
    int tmp_switch = 1;
	setsockopt(CDataSourceCtrl::m_NotificationRecever,IPPROTO_TCP, TCP_NODELAY, (char *)&tmp_switch, sizeof(tmp_switch));

}
//---------------------------------------------------------------------------------------


//---------------------------------------------------------------------------------------
// Runs every time the server sends notification string, after send_notification
// function. We will generate DataSource event passing the notification string
// as a parameter.
//---------------------------------------------------------------------------------------
void CNotificationRecever::OnReceive( int nErrorCode ) {
POSITION client;
static int size=-1;
union {
	char bytes[4];
	int val;
} sizeBlock;
char* str;
    if(size==-1) {
	    Receive(sizeBlock.bytes,4);
		size=sizeBlock.val;
	} else {
		str= new char [size];
	    Receive(str,size);
        client=m_ClientsList.GetHeadPosition();
        while(client!=NULL) {
		   m_ClientsList.GetAt(client)->ServerNotification(CString(str));
		   m_ClientsList.GetNext(client);
		}
		delete str;
		size=-1;

	}
}
//---------------------------------------------------------------------------------------


//---------------------------------------------------------------------------------------		
// This function will add a new DataSource object to the list.
// All DataSource objects from this list will generate Notify event when
// this socket will receve data.
//---------------------------------------------------------------------------------------
void CNotificationRecever::AddClient(CDataSourceCtrl* client) {
	m_ClientsList.AddTail(client);
}
//---------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------		
// This function will remove DataSource object from the list.
//---------------------------------------------------------------------------------------
void CNotificationRecever::RemoveClient(CDataSourceCtrl* client) {
POSITION pos;
    pos=m_ClientsList.GetHeadPosition();
    while(pos!=NULL) {
		if(m_ClientsList.GetAt(pos)==client) {
	       m_ClientsList.RemoveAt(pos);
		   return;
		}
		m_ClientsList.GetNext(pos);
	}
}
//---------------------------------------------------------------------------------------



/////////////////////////////////////////////////////////////////////////////
// Message map

BEGIN_MESSAGE_MAP(CDataSourceCtrl, COleControl)
	//{{AFX_MSG_MAP(CDataSourceCtrl)
	ON_WM_CREATE()
	//}}AFX_MSG_MAP
	ON_OLEVERB(AFX_IDS_VERB_EDIT, OnEdit)
	ON_OLEVERB(AFX_IDS_VERB_PROPERTIES, OnProperties)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// Dispatch map

BEGIN_DISPATCH_MAP(CDataSourceCtrl, COleControl)
	//{{AFX_DISPATCH_MAP(CDataSourceCtrl)
	DISP_PROPERTY_EX(CDataSourceCtrl, "ServerLock", GetServerLock, SetServerLock, VT_BOOL)
	DISP_FUNCTION(CDataSourceCtrl, "AccessSync", AccessSync, VT_BSTR, VTS_PBSTR)
	DISP_FUNCTION(CDataSourceCtrl, "AccessOutput", AccessOutput, VT_BSTR, VTS_PBSTR)
	DISP_FUNCTION(CDataSourceCtrl, "AccessDish", AccessDish, VT_BSTR, VTS_PBSTR)
	DISP_FUNCTION(CDataSourceCtrl, "SetSource", SetSource, VT_EMPTY, VTS_BSTR VTS_BOOL)
	DISP_FUNCTION(CDataSourceCtrl, "AcceptNotification", AcceptNotification, VT_EMPTY, VTS_BSTR)
	DISP_FUNCTION(CDataSourceCtrl, "RejectNotification", RejectNotification, VT_EMPTY, VTS_NONE)
	DISP_FUNCTION(CDataSourceCtrl, "IsServerBusy", IsServerBusy, VT_BOOL, VTS_NONE)
	DISP_FUNCTION(CDataSourceCtrl, "IsConnectionLost", IsConnectionLost, VT_BOOL, VTS_NONE)
	DISP_FUNCTION_ID(CDataSourceCtrl, "AboutBox", DISPID_ABOUTBOX, AboutBox, VT_EMPTY, VTS_NONE)
	//}}AFX_DISPATCH_MAP
END_DISPATCH_MAP()


/////////////////////////////////////////////////////////////////////////////
// Event map

BEGIN_EVENT_MAP(CDataSourceCtrl, COleControl)
	//{{AFX_EVENT_MAP(CDataSourceCtrl)
	EVENT_CUSTOM("ServerNotification", FireServerNotification, VTS_BSTR)
	//}}AFX_EVENT_MAP
END_EVENT_MAP()


/////////////////////////////////////////////////////////////////////////////
// Property pages

// TODO: Add more property pages as needed.  Remember to increase the count!
BEGIN_PROPPAGEIDS(CDataSourceCtrl, 1)
	PROPPAGEID(CDataSourcePropPage::guid)
END_PROPPAGEIDS(CDataSourceCtrl)


/////////////////////////////////////////////////////////////////////////////
// Initialize class factory and guid

IMPLEMENT_OLECREATE_EX(CDataSourceCtrl, "DATASOURCE.DataSourceCtrl.1",
	0x7ca41f77, 0x91c, 0x11d2, 0xae, 0xe6, 0, 0xa0, 0xc9, 0xb7, 0x1d, 0xc4)


/////////////////////////////////////////////////////////////////////////////
// Type library ID and version

IMPLEMENT_OLETYPELIB(CDataSourceCtrl, _tlid, _wVerMajor, _wVerMinor)


/////////////////////////////////////////////////////////////////////////////
// Interface IDs

const IID BASED_CODE IID_DDataSource =
		{ 0x77b20dc, 0x934, 0x11d2, { 0xae, 0xe6, 0, 0xa0, 0xc9, 0xb7, 0x1d, 0xc4 } };
const IID BASED_CODE IID_DDataSourceEvents =
		{ 0x77b20dd, 0x934, 0x11d2, { 0xae, 0xe6, 0, 0xa0, 0xc9, 0xb7, 0x1d, 0xc4 } };


/////////////////////////////////////////////////////////////////////////////
// Control type information

static const DWORD BASED_CODE _dwDataSourceOleMisc =
	OLEMISC_INVISIBLEATRUNTIME |
	OLEMISC_ACTIVATEWHENVISIBLE |
	OLEMISC_SETCLIENTSITEFIRST |
	OLEMISC_INSIDEOUT |
	OLEMISC_CANTLINKINSIDE |
	OLEMISC_RECOMPOSEONRESIZE;

IMPLEMENT_OLECTLTYPE(CDataSourceCtrl, IDS_DATASOURCE, _dwDataSourceOleMisc)


/////////////////////////////////////////////////////////////////////////////
// CDataSourceCtrl::CDataSourceCtrlFactory::UpdateRegistry -
// Adds or removes system registry entries for CDataSourceCtrl

BOOL CDataSourceCtrl::CDataSourceCtrlFactory::UpdateRegistry(BOOL bRegister)
{
	// TODO: Verify that your control follows apartment-model threading rules.
	// Refer to MFC TechNote 64 for more information.
	// If your control does not conform to the apartment-model rules, then
	// you must modify the code below, changing the 6th parameter from
	// afxRegInsertable | afxRegApartmentThreading to afxRegInsertable.

	if (bRegister)
		return AfxOleRegisterControlClass(
			AfxGetInstanceHandle(),
			m_clsid,
			m_lpszProgID,
			IDS_DATASOURCE,
			IDB_DATASOURCE,
			afxRegInsertable | afxRegApartmentThreading,
			_dwDataSourceOleMisc,
			_tlid,
			_wVerMajor,
			_wVerMinor);
	else
		return AfxOleUnregisterClass(m_clsid, m_lpszProgID);
}


/////////////////////////////////////////////////////////////////////////////
// CDataSourceCtrl::CDataSourceCtrl - Constructor

CDataSourceCtrl::CDataSourceCtrl()
{
	AfxSocketInit();

	InitializeIIDs(&IID_DDataSource, &IID_DDataSourceEvents);
	if(ref_count==0)
        InitializeCriticalSection(&g_CriticalSection);
	ref_count++;
	m_UseReserved=FALSE;
	m_ReservedChannelOpened=FALSE;
	m_NotificationRecever.AddClient(this);
	m_ServerLock=FALSE;
}


/////////////////////////////////////////////////////////////////////////////
// CDataSourceCtrl::~CDataSourceCtrl - Destructor

CDataSourceCtrl::~CDataSourceCtrl() {
   ref_count--;
   if(ref_count==0)
       DeleteCriticalSection(&g_CriticalSection);
   m_NotificationRecever.RemoveClient(this);
   if(m_UseReserved==TRUE && m_ReservedChannelOpened==TRUE) {
	  m_ReservedServerConnection.Close();
   }
}


/////////////////////////////////////////////////////////////////////////////
// CDataSourceCtrl::OnDraw - Drawing function

void CDataSourceCtrl::OnDraw(
			CDC* pdc, const CRect& rcBounds, const CRect& rcInvalid)
{
	// TODO: Replace the following code with your own drawing code.
	pdc->FillRect(rcBounds, CBrush::FromHandle((HBRUSH)GetStockObject(WHITE_BRUSH)));
	pdc->Ellipse(rcBounds);
}


/////////////////////////////////////////////////////////////////////////////
// CDataSourceCtrl::DoPropExchange - Persistence support

void CDataSourceCtrl::DoPropExchange(CPropExchange* pPX)
{  
	ExchangeVersion(pPX, MAKELONG(_wVerMinor, _wVerMajor));
	COleControl::DoPropExchange(pPX);

	if(m_ChannelOpened==FALSE) {
		if(m_ServiceName.GetLength()==0) {
			 char host[1024];
			 gethostname(host,1024);
			 CString serviceName;
			 serviceName="Discover:";
			 serviceName+=host;
		     NameServerGetService(serviceName,m_Port,m_Address);
		} else {
		     NameServerGetService(m_ServiceName,m_Port,m_Address);
		}
		m_ServerConnection.Close();
		if(m_ServerConnection.ConnectToServerEx(m_Port,m_Address)==false) {
			return;	
		}
		m_ChannelOpened=TRUE;
		m_bConnectionLost = FALSE;
	}

}


/////////////////////////////////////////////////////////////////////////////
// CDataSourceCtrl::OnResetState - Reset control to default state

void CDataSourceCtrl::OnResetState()
{
	COleControl::OnResetState();  // Resets defaults found in DoPropExchange

	// TODO: Reset any other control state here.
}


/////////////////////////////////////////////////////////////////////////////
// CDataSourceCtrl::AboutBox - Display an "About" box to the user

void CDataSourceCtrl::AboutBox()
{
	CDialog dlgAbout(IDD_ABOUTBOX_DATASOURCE);
	dlgAbout.DoModal();
}


void CDataSourceCtrl::ProcessMessages(CConnection* pConnection) {
	const int nMaxDelay = 10;
	unsigned long bytes = 0;
	int nSleep = 0;
	pConnection->IOCtl(FIONREAD, &bytes);
	MSG msg;
	bool bWait = !bytes;
	if(bWait) AfxGetApp()->BeginWaitCursor();
	while(bytes==0 && !pConnection->IsConnectionClosed() ) {
		nSleep++;
		WaitMessage();
		if(PeekMessage(&msg,0,WM_SOCKET_NOTIFY,WM_SOCKET_DEAD,PM_REMOVE)!=0) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			nSleep = 0;
		}
		if(PeekMessage(&msg,0,WM_PAINT,WM_PAINT,PM_REMOVE)!=0) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);		
			nSleep = 0;
		}
		
		if(nSleep>nMaxDelay) nSleep=nMaxDelay;
		if(nSleep) Sleep(nSleep);
		pConnection->IOCtl(FIONREAD, &bytes);
	}
	if(bWait) AfxGetApp()->EndWaitCursor();
}

/////////////////////////////////////////////////////////////////////////////
// CDataSourceCtrl message handlers

BSTR CDataSourceCtrl::AccessSync(BSTR FAR* query)  {
CString command(*query);
DISCommand RPCcommand(command.GetBuffer(20),RESULT_BATCH_CMD_TYPE);
char* result_ptr;
unsigned char result_code;
CString result;

    if(m_ServerLock)  return result.AllocSysString();
    EnterCriticalSection(&g_CriticalSection);
	ServerBusy=TRUE;
	CConnection* pServerConnection = m_UseReserved ? &m_ReservedServerConnection : &m_ServerConnection; 
    if(pServerConnection != NULL) {
		pServerConnection->SendCommand(&RPCcommand);

		ProcessMessages(pServerConnection);

		if(pServerConnection->IsConnectionClosed()) {// if data not received 
			m_bConnectionLost = TRUE;
			result = "";
		} else {
			int rc = pServerConnection->ReceiveReply(&result_ptr, result_code);
			if(rc == 1) {
				m_bConnectionLost = FALSE;
				result=result_ptr;
			} else {
				m_bConnectionLost = TRUE;
				result = "";
			}
		}
	}
	ServerBusy=FALSE;
	LeaveCriticalSection(&g_CriticalSection);
    return result.AllocSysString();
}




BSTR CDataSourceCtrl::AccessOutput(BSTR FAR* query)  {
CString command(*query);
DISCommand RPCcommand(command.GetBuffer(20),STDOUT_BATCH_CMD_TYPE);
char* result_ptr;
unsigned char result_code;
CString result;

    if(m_ServerLock)  return result.AllocSysString();    
    EnterCriticalSection(&g_CriticalSection);
	ServerBusy=TRUE;
	CConnection* pServerConnection = m_UseReserved ? &m_ReservedServerConnection : &m_ServerConnection; 
    if(pServerConnection != NULL) {
		pServerConnection->SendCommand(&RPCcommand);

		ProcessMessages(pServerConnection);

		if(pServerConnection->IsConnectionClosed()) {// if data not received 
			m_bConnectionLost = TRUE;
			result = "";
		} else {
			int rc = pServerConnection->ReceiveReply(&result_ptr, result_code);
			if(rc == 1) {
				m_bConnectionLost = FALSE;
				result=result_ptr;
			} else {
				m_bConnectionLost = TRUE;
				result = "";
			}
		}
	}
	ServerBusy=FALSE;
	LeaveCriticalSection(&g_CriticalSection);
    return result.AllocSysString();
}


//---------------------------------------------------------------------------------------
// This method designed to communicate with the server using the terminal - dish. It will
// send to the client the complete set of the interpreter data including prompt.
//---------------------------------------------------------------------------------------
BSTR CDataSourceCtrl::AccessDish(BSTR FAR* query)  {
CString command(*query);
DISCommand RPCcommand(command.GetBuffer(20),STDOUT_DISH_STREAM_CMD_TYPE);
char* result_ptr;
unsigned char result_code;
CString result;

    if(m_ServerLock)  return result.AllocSysString();    
    EnterCriticalSection(&g_CriticalSection);
	ServerBusy=TRUE;
	CConnection* pServerConnection = m_UseReserved ? &m_ReservedServerConnection : &m_ServerConnection; 
    if(pServerConnection != NULL) {
		pServerConnection->SendCommand(&RPCcommand);
		
		ProcessMessages(pServerConnection);

		if(pServerConnection->IsConnectionClosed()) {// if data not received 
			m_bConnectionLost = TRUE;
			result = "";
		} else {
			int rc = pServerConnection->ReceiveReply(&result_ptr, result_code);
			if(rc == 1) {
				m_bConnectionLost = FALSE;
				result=result_ptr;
			} else {
				m_bConnectionLost = TRUE;
				result = "";
			}
		}
	}
	ServerBusy=FALSE;
	LeaveCriticalSection(&g_CriticalSection);
    return result.AllocSysString();
}
//---------------------------------------------------------------------------------------



void CDataSourceCtrl::SetSource(LPCTSTR name, BOOL alone) {
   m_ServiceName=name;
   if(alone==TRUE) {
	   m_UseReserved=TRUE;
       if(m_UseReserved==TRUE && m_ReservedChannelOpened==TRUE) {
		   m_ReservedServerConnection.Close();
	   }
	   m_ReservedServerConnection.Close();
	   m_ReservedChannelOpened=FALSE;
	   m_bConnectionLost = TRUE;
       if(!m_ServiceName.IsEmpty()) {
	       NameServerGetService(m_ServiceName,m_ReservedPort,m_ReservedAddress);
		   m_ReservedServerConnection.ConnectToServerEx(m_Port,m_Address); 
		   m_ReservedChannelOpened=TRUE;
		   m_bConnectionLost = FALSE;
	   }
   } else {
	   if(m_UseReserved==TRUE) {
		     if(m_ReservedChannelOpened==TRUE) {
				  m_ReservedServerConnection.Close();
				  m_ReservedChannelOpened=FALSE;
			 }
			 m_UseReserved=FALSE;
	   } else{
		     if(m_ChannelOpened==TRUE) {
				  m_ServerConnection.Close();
				  m_ChannelOpened=FALSE;
			 }
	   }
	   m_ServerConnection.Close();
	   m_ChannelOpened = FALSE;
	   m_bConnectionLost = TRUE;
       if(!m_ServiceName.IsEmpty()) {
	       NameServerGetService(m_ServiceName,m_Port,m_Address);
		   m_ServerConnection.ConnectToServerEx(m_Port,m_Address); 
		   m_ChannelOpened=TRUE;
		   m_bConnectionLost = FALSE;
	   }   
	}
    if(m_ServiceName!=lastName && !m_bConnectionLost) {
        // Closing old notification sockets
        m_NotificationRecever.Close();
	    m_NotificationCreator.Close();

        // Creating the listening socket if none
        CString addr;
        unsigned int port;
        CString notificationQuery;
        BSTR sysQuery;
        m_NotificationCreator.Create();
        m_NotificationCreator.Listen();

        // Sending command to the server - I want to receve notifications
        m_NotificationCreator.GetSockName(addr,port);
        notificationQuery.Format("add_client_notifier %d",port);
        sysQuery=notificationQuery.AllocSysString();
        AccessSync(&sysQuery);
		lastName=m_ServiceName;
	} 
}

void CDataSourceCtrl::ServerNotification(CString & str) {
	FireServerNotification(str);

}

void CDataSourceCtrl::AcceptNotification(LPCTSTR resStr)  {
union {
	char bytes[4];
	int val;
} sizeBlock;
   sizeBlock.val=strlen(resStr)+1;
   m_NotificationRecever.Send(sizeBlock.bytes,4,0);
    m_NotificationRecever.Send(resStr,sizeBlock.val,0);
}

void CDataSourceCtrl::RejectNotification()  {
union {
	char bytes[4];
	int val;
} sizeBlock;
   sizeBlock.val=0;
    m_NotificationRecever.Send(sizeBlock.bytes,4,0);
}


int CDataSourceCtrl::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (COleControl::OnCreate(lpCreateStruct) == -1)
		return -1;	
	return 0;
}

BOOL CDataSourceCtrl::GetServerLock()  {
	return m_ServerLock;
}

void CDataSourceCtrl::SetServerLock(BOOL bNewValue)  {
	m_ServerLock=bNewValue;
}


BOOL CDataSourceCtrl::IsServerBusy()  {
	return ServerBusy;
}

BOOL CDataSourceCtrl::IsConnectionLost() 
{
	return m_bConnectionLost;
}
