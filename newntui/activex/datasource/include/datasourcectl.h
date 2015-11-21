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
// DataSourceCtl.h : Declaration of the CDataSourceCtrl OLE control class.
#include <afxsock.h>
#include <afxtempl.h>
#define WM_SOCKET_NOTIFY    0x0373
#define WM_SOCKET_DEAD      0x0374
////////////////////////////////////////////////////////////////////////////////////////
// CNotificationCreator
////////////////////////////////////////////////////////////////////////////////////////
class CNotificationCreator : public CAsyncSocket  {
   public : 
	    // Runs when server try to conect to the DataSource using notification channel.
	    // (this hapens after evaluation of the add_client_notificator Access function).
	    // This function will call Accept to create new socket connected to the server
	    // notification socket, 
        virtual void OnAccept( int nErrorCode );
};


////////////////////////////////////////////////////////////////////////////////////////
// CNotificationRecever
////////////////////////////////////////////////////////////////////////////////////////
class CDataSourceCtrl;
class CNotificationRecever : public CAsyncSocket  {
   public : 
	    // Runs every time the server sends notification string, after send_notification
	    // function. We will generate DataSource event passing the notification string
	    // as a parameter.
        virtual void OnReceive( int nErrorCode );
		// This function will add a new DataSource object to the list.
		// All DataSource objects from this list will generate Notify event when
		// this socket will receve data.
		void AddClient(CDataSourceCtrl* client);
		// Removes client from the notifications list.
        void RemoveClient(CDataSourceCtrl* client);
   private:
	    CList <CDataSourceCtrl*,CDataSourceCtrl*> m_ClientsList;
};
////////////////////////////////////////////////////////////////////////////////////////





/////////////////////////////////////////////////////////////////////////////
// CDataSourceCtrl : See DataSourceCtl.cpp for implementation.
class CDataSourceCtrl : public COleControl
{
	DECLARE_DYNCREATE(CDataSourceCtrl)

// Constructor
public:
	CDataSourceCtrl();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDataSourceCtrl)
	public:
	virtual void OnDraw(CDC* pdc, const CRect& rcBounds, const CRect& rcInvalid);
	virtual void DoPropExchange(CPropExchange* pPX);
	virtual void OnResetState();
	//}}AFX_VIRTUAL

// Implementation
protected:
	~CDataSourceCtrl();

	DECLARE_OLECREATE_EX(CDataSourceCtrl)    // Class factory and guid
	DECLARE_OLETYPELIB(CDataSourceCtrl)      // GetTypeInfo
	DECLARE_PROPPAGEIDS(CDataSourceCtrl)     // Property page IDs
	DECLARE_OLECTLTYPE(CDataSourceCtrl)		// Type name and misc status

// Message maps
	//{{AFX_MSG(CDataSourceCtrl)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// Dispatch maps
	//{{AFX_DISPATCH(CDataSourceCtrl)
	afx_msg BOOL GetServerLock();
	afx_msg void SetServerLock(BOOL bNewValue);
	afx_msg BSTR AccessSync(BSTR FAR* command);
	afx_msg BSTR AccessOutput(BSTR FAR* query);
	afx_msg BSTR AccessDish(BSTR FAR* query);
	afx_msg void SetSource(LPCTSTR name, BOOL alone);
	afx_msg void AcceptNotification(LPCTSTR resStr);
	afx_msg void RejectNotification();
	afx_msg BOOL IsServerBusy();
	afx_msg BOOL IsConnectionLost();
	afx_msg void AboutBox();
	//}}AFX_DISPATCH
	DECLARE_DISPATCH_MAP()

// Event maps
	//{{AFX_EVENT(CDataSourceCtrl)
	void FireServerNotification(LPCTSTR str)
		{FireEvent(eventidServerNotification,EVENT_PARAM(VTS_BSTR), str);}
	//}}AFX_EVENT
	DECLARE_EVENT_MAP()

// Dispatch and event IDs
public:
	enum {
	//{{AFX_DISP_ID(CDataSourceCtrl)
	dispidServerLock = 1L,
	dispidAccessSync = 2L,
	dispidAccessOutput = 3L,
	dispidAccessDish = 4L,
	dispidSetSource = 5L,
	dispidAcceptNotification = 6L,
	dispidRejectNotification = 7L,
	dispidIsServerBusy = 8L,
	dispidIsConnectionLost = 9L,
	eventidServerNotification = 1L,
	//}}AFX_DISP_ID
	};
private:
	void ProcessMessages(CConnection* pConnection);
	BOOL m_bConnectionLost;
	BOOL m_UseReserved;
	BOOL m_ReservedChannelOpened;
	int  m_ReservedAddress;
	int  m_ReservedPort;
	CString m_ServiceName;
	// Socket to communicate with pset_server.
	static CConnection m_ServerConnection;
	static CConnection m_ReservedServerConnection;

    // Listening socket which will accept pset_server connections.
	// Only one per application
    static CNotificationCreator m_NotificationCreator;
public:
	void ServerNotification(CString& str);
    // Connection to the server interpreter. One per client.
	// Filled by creator as a result of the add_client_notificator call.
	static CNotificationRecever m_NotificationRecever;
};
