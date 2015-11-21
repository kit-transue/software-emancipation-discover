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
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#ifndef hp10
        #include <sys/select.h>
        #include <sys/filio.h>
#else
        #include <sys/ioctl.h>
#endif
#include <sys/time.h>
#include <netinet/tcp.h>
#include <netinet/in.h>

#include "stl.h"

#include "IntegratorSocket.h"

extern "C" {void receiveHandler(int signo);}

TSocketsList CIntegratorSocket::m_SocketsInfoList; 
bool CIntegratorSocket::bIOInited = false;

/////////////////////////////////////////////////////////////////////////////
// CIntegratorSocket

CIntegratorSocket::CIntegratorSocket() :
	m_nSocket(-1),
	state_OK(false)
{
   m_Ctrl = NULL;
   if(!bIOInited) {
        bIOInited = true;
        registerHandler(SIGIO,receiveHandler);
   }
   m_pReplyList = new TPStringList();
   m_pEventList = new TPStringList();
}

CIntegratorSocket::~CIntegratorSocket()
{
        delete m_pReplyList;
        delete m_pEventList; 
}

/////////////////////////////////////////////////////////////////////////////
// CIntegratorSocket member functions


void CIntegratorSocket::OnReceive()  {
        while(good() && IsDataPending()) {
                string data = ReadString();
                if (!data.empty()) {
                    if(data[0]=='1') {
                        m_pReplyList->push_back(new string(data.substr(2,data.length()-3)));
                    } else {
                        if(data[0]=='0') {
                                if(m_Ctrl) {
                                    m_pEventList->push_back(new string(data.substr(2,data.length()-3)));
                                    m_Ctrl->OnIntegratorEvent();
                                }
                        }
                   }
            }
        }
}

bool CIntegratorSocket::SendCommand(string &command) {
        int len = command.length();
        unsigned char b[4];
        b[3] = len & 0xFF;
        b[2] = (len>>8)  & 0xFF;
        b[1] = (len>>16) & 0xFF;
        b[0] = (len>>24) & 0xFF;
        if(Send((const char* )b,4)!=4) return false;
        if(Send(command.c_str(),len)!=len) return false;
        return true;
}

string CIntegratorSocket::GetReply() {
        string data;
        int nSecs = 300;
        while(nSecs){
                int nSize = m_pReplyList->size();
                if(nSize>0) break;
                sleep(1);
                nSecs--;
        }
        if(nSecs>0) {
                string* pReply = m_pReplyList->front();
                data = *pReply;
                m_pReplyList->pop_front();
                delete pReply;
	}
        return data;
}

bool CIntegratorSocket::IsDataPending() {
        struct timeval timeout;
        timeout.tv_sec = 0;
        timeout.tv_usec = 0;

        fd_set sock_set;
        int    nfsd = 0;
#ifndef _WIN32
        nfsd = FD_SETSIZE;
#endif

        FD_ZERO(&sock_set);
        FD_SET(m_nSocket, &sock_set);
        if(select(nfsd,&sock_set,NULL,NULL,&timeout)>0) 
                return true;
        return false;
                
}

string CIntegratorSocket::GetEvent() {
        string* pEvent = NULL;
        int nSecs = 300;
        while(m_pEventList->size()==0 && nSecs) {
                nSecs--;
        }
        pEvent = m_pEventList->front();
        string data(*pEvent);
        m_pEventList->pop_front();
        delete pEvent;
        return data;
}

string CIntegratorSocket::ReadString() {
        string retString;
        unsigned char b[4];
        
        int nCode = Receive((char *)b,4);
        if(nCode == 4) {
          int len = (unsigned int)b[3]+((unsigned int)b[2]<<8)+((unsigned int)b[1]<<16)+((unsigned int)b[0]<<24);
          if(len>0) {
            retString.resize(len+1);
            char* buf = (char*)retString.data();
            int bytes_read = 0;
            while(bytes_read < len) {
                    int res = Receive(buf + bytes_read, len - bytes_read);
                    if(res <= 0) {
            		   state_OK = false;
                           break;
                    }
                    bytes_read += res;
            }
          }
        } else {
            state_OK = false;
        }
        return retString;
}


void CIntegratorSocket::SetListener(CIntegrator *ctrl) {
        m_Ctrl = ctrl;

}

void CIntegratorSocket::ClearCommandsBuffer() {
        string* pMessage = NULL;
        while((pMessage=m_pReplyList->front())!=NULL) {
                m_pReplyList->pop_front();
                delete pMessage;
                pMessage = NULL;
        }
}

bool CIntegratorSocket::Create() {
        m_nSocket = socket(AF_INET,SOCK_STREAM,0);
        if(m_nSocket >= 0) {
                m_SocketsInfoList.push_back(new CSocketInfo(m_nSocket,this));
        }
        return m_nSocket>0;
}

bool CIntegratorSocket::Close() {
        TSocketsList::iterator iter = m_SocketsInfoList.begin();
        TSocketsList::iterator iEnd = m_SocketsInfoList.end();
        while(iter!=iEnd) {
                CSocketInfo* pInfo = *iter;
                if(pInfo->m_nSocket == m_nSocket) {
                        m_SocketsInfoList.erase(iter);
                        delete pInfo;
                        break;
                }
                iter++;
        }
        return close(m_nSocket)==0;
}

bool CIntegratorSocket::Connect(struct sockaddr* servaddr, int addr_len) {
        if(connect(m_nSocket,servaddr,addr_len)==0) {
                const int on = 1;
                fcntl(m_nSocket,F_SETOWN,getpid());
                ioctl(m_nSocket,FIOASYNC,&on);
#if defined(hp10) || defined(irix6)
                setsockopt(m_nSocket, IPPROTO_TCP, TCP_NODELAY, &on, sizeof(on));
#else
                char cOn = 1;
                setsockopt(m_nSocket, IPPROTO_TCP, TCP_NODELAY, &cOn, sizeof(cOn));
#endif
		state_OK = true;
                return true;
        }
        return false;
}

int CIntegratorSocket::Send( const char* lpBuf, int nBufLen, int nFlags) {
#ifdef hp10
        return send(m_nSocket,(void*)lpBuf,nBufLen,nFlags);
#else
        return send(m_nSocket,lpBuf,nBufLen,nFlags);
#endif
}

int CIntegratorSocket::Receive(char* lpBuf, int nBufLen, int nFlags) {
        return recv(m_nSocket,lpBuf,nBufLen,nFlags);
}

bool
CIntegratorSocket::good() const
{
	return state_OK;
}

#define cma_sigaction sigaction

void* registerHandler(int signo,SignalHandler *handler) {
        struct sigaction act,oact;

        act.sa_handler = handler;
        sigemptyset(&act.sa_mask);
        act.sa_flags = 0;
        if(signo == SIGALRM) {
#ifdef SA_INTERRUPT
                act.sa_flags |= SA_INTERRUPT;
#endif
        } else {
#ifdef SA_RESTART
                act.sa_flags |= SA_RESTART;
#endif
        }
        if(sigaction(signo,&act,&oact) < 0)
                return (void*)SIG_ERR;
        return (void*)oact.sa_handler;
}

extern "C" void receiveHandler(int signo) {
        if(signo == SIGIO) {
                sigset_t newmask,oldmask;
                sigemptyset(&oldmask);
                sigemptyset(&newmask);
                sigaddset(&newmask,SIGIO);
                
                sigprocmask(SIG_BLOCK,&newmask,&oldmask);

                TSocketsList::iterator iList = CIntegratorSocket::m_SocketsInfoList.begin();
                TSocketsList::iterator iEnd = CIntegratorSocket::m_SocketsInfoList.end();
                while(iList!=iEnd) {
                        CSocketInfo* pInfo = *iList;
                        CIntegratorSocket* pThis = pInfo->m_pIntegrator;
                        if(pThis!=NULL && pThis->IsDataPending()) {
                                pThis->OnReceive();
                        }
                        iList++;  
                }
                
                sigprocmask(SIG_SETMASK,&oldmask,NULL);

        }
}
