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
#ifndef __INTEGRATOR_SOCKET_HEADER
#define __INTEGRATOR_SOCKET_HEADER

#include "Integrator.h"
#include "stl.h"

class CSocketInfo;
typedef vector<CSocketInfo*> TSocketsList;
typedef list<string*> TPStringList;
typedef TPStringList::iterator TStringIterator;

extern "C" {typedef void SignalHandler(int);}

extern void* registerHandler(int signo, SignalHandler *handler);

class CIntegratorSocket
{
// Attributes
public:

// Operations
public:
        CIntegratorSocket();
        virtual ~CIntegratorSocket();

// Overrides
public:
        virtual bool Create();
        virtual bool Close();
	virtual bool good() const;
        virtual bool Connect(struct sockaddr* servaddr, int addr_len);
        virtual int Send( const char* lpBuf, int nBufLen, int nFlags = 0 );
        virtual int Receive( char* lpBuf, int nBufLen, int nFlags = 0 );

        virtual void OnReceive();

        void ClearCommandsBuffer(void);
        void SetListener(CIntegrator* ctrl);
        string GetEvent(void);
        string GetReply(void);
        bool SendCommand(string& command);
        bool IsDataPending();

        static TSocketsList m_SocketsInfoList; 
// Implementation
protected:
        TPStringList* m_pReplyList;
        TPStringList* m_pEventList;
private:
        static bool bIOInited;
        int m_nSocket;
        CIntegrator* m_Ctrl;
        string ReadString(void);
	bool state_OK;
};

class CSocketInfo {
 public:
  CSocketInfo(int nSocket, CIntegratorSocket* pIntegrator) {
    m_nSocket = nSocket;
    m_pIntegrator = pIntegrator;
  }
  CIntegratorSocket* m_pIntegrator;
  int m_nSocket;
};

#endif // __INTEGRATOR_SOCKET_HEADER
