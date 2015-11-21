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
// IntegratorSocket.cpp : implementation file
//

#include "stdafx.h"
#include "IntegratorSocket.h"
#include "IntegratorCtrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CIntegratorSocket

const int CIntegratorSocket::CLOSED = 1;
const int CIntegratorSocket::OPEN = 2;

CIntegratorSocket::CIntegratorSocket() {
   m_Ctrl = NULL;
   SetState(OPEN);
}

CIntegratorSocket::~CIntegratorSocket()
{
}


// Do not edit the following lines, which are needed by ClassWizard.
#if 0
BEGIN_MESSAGE_MAP(CIntegratorSocket, CSocket)
	//{{AFX_MSG_MAP(CIntegratorSocket)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()
#endif	// 0

/////////////////////////////////////////////////////////////////////////////
// CIntegratorSocket member functions

void CIntegratorSocket::OnReceive(int nErrorCode)  {
    CString data = ReadString();
	if(data[0]=='1') {
		m_ReplyList.AddTail(data.Right(data.GetLength()-2));
	}
	if(data[0]=='0') {
		if(m_Ctrl) {
		    m_EventList.AddTail(data.Right(data.GetLength()-2));
		    m_Ctrl->OnIntegratorEvent();
		}
	}
	CSocket::OnReceive(nErrorCode);
}

#error XXX: change to network byte order everywhere in this file!!!!

bool CIntegratorSocket::SendCommand(CString &command) {
	int len = command.GetLength();
	unsigned char b[4];
	b[0] = len & 0xFF;
	b[1] = (len>>8)  & 0xFF;
	b[2] = (len>>16) & 0xFF;
	b[3] = (len>>24) & 0xFF;
	if(Send(b,4)!=4) return false;
	if(Send(command.GetBuffer(10),len)!=len) return false;
	return true;

}

CString CIntegratorSocket::GetReply() {
	const int nCountMax = 100;
	int repCount = nCountMax;

	while(m_ReplyList.GetCount()==0 && repCount>0  && GetState()!=CLOSED) {
		MSG msg;
		if(PeekMessage(&msg,NULL,0,0,PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		repCount--;
		Sleep(nCountMax - repCount);
	}
	if(repCount==0 || GetState()==CLOSED) return "";
	CString data = m_ReplyList.GetHead();
	m_ReplyList.RemoveHead();
	return data;
}

CString CIntegratorSocket::GetEvent() {
int repCount = 300;

	while(m_EventList.GetCount()==0 && repCount>0 && GetState()!=CLOSED) {
		MSG msg;
		GetMessage(&msg,NULL,0,0);
		TranslateMessage(&msg);
		DispatchMessage(&msg);
		Sleep(100);
		repCount--;
	}
	if(repCount==0 || GetState()==CLOSED) return "";
	CString data = m_EventList.GetHead();
	m_EventList.RemoveHead();
	return data;

}

CString CIntegratorSocket::ReadString() {
    unsigned char b[4];
	Receive(b,4);
	int len = (unsigned int)b[0]+((unsigned int)b[1]<<8)+((unsigned int)b[2]<<16)+((unsigned int)b[3]<<24);
	CString retString;
	char* buf = retString.GetBuffer(len+1);
	int readed=0;
	while(readed<len) {
		int res = Receive(buf+readed,len-readed);
		if(res<0) break;
		readed+=res;
	}
    buf[readed]=0;
	retString.ReleaseBuffer();
	return retString;
}


void CIntegratorSocket::SetControl(CIntegratorCtrl *ctrl) {
	m_Ctrl = ctrl;

}

void CIntegratorSocket::ClearCommandsBuffer() {
	while(m_ReplyList.GetHeadPosition()!=NULL)
	   m_ReplyList.RemoveHead();

}

void CIntegratorSocket::OnClose(int nErrorCode) 
{
	SetState(CLOSED);
	CSocket::OnClose(nErrorCode);
}

int CIntegratorSocket::GetState()
{
	return m_nState;
}

void CIntegratorSocket::SetState(int state)
{
	m_nState = state;
}
