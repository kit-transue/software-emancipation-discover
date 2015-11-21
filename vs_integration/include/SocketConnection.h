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
// SocketConnection.h: interface for the SocketConnection class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SOCKETCONNECTION_H__61480E86_8D86_11D4_8E3A_000102390321__INCLUDED_)
#define AFX_SOCKETCONNECTION_H__61480E86_8D86_11D4_8E3A_000102390321__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <afxtempl.h>
#include "commands.h"


class SocketConnection   {
public:
	void RemoveAllClients(void);
	SocketConnection();
	SocketConnection(CCommandsObj* cmds);
	virtual ~SocketConnection();
	void AddClient(int socket);
	void RemoveClient(int socket);
	bool SendMessageToClients(CString msg);
	void ProcessClientsRequests(void);
    void sendEvent(CString event);
    void sendEvent(int socket,CString event);
    void sendReply(int socket,CString reply);

	HWND m_ProxyHwnd;
private:
	void ProcessCommand(int socket,CString cmd);
	int  m_Listener;
	CCommandsObj* m_pCommandProcessor;

    CList<int,int> m_Clients;

	void InitializeWindowsSocketSupport(void);
	CString ServiceName(void);
	bool RegisterService(int socket);
	int CreateServerSocket(void);
	bool StartNameServer(void);
    bool ReadCommand(int socket,CString& command);
    bool CheckSocket(int socket);
    void MakeDiff(CString& localfile, CString& modelfile, CString& diffile);
    int  MapLines(CString& localfile, CString& basefile, CString& diffile, int localline);
    int  GetModelLine(CString& localfile, CString& basefile,int localline);
    int  GetPrivateLine(CString& localfile, CString& basefile,int localline);


};

#endif // !defined(AFX_SOCKETCONNECTION_H__61480E86_8D86_11D4_8E3A_000102390321__INCLUDED_)
