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
// IntegratorCtrl.cpp: implementation of the CIntegratorCtrl class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "nameServCalls.h"
#include "IntegratorCtrl.h"
#include "IntegratorSocket.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CIntegratorCtrl::CIntegratorCtrl() {
    m_ConnectionChannel = NULL;
	AfxSocketInit();
}

CIntegratorCtrl::~CIntegratorCtrl() {
	Unregister();
	if(m_ConnectionChannel) delete m_ConnectionChannel;
}

//-------------------------------------------------------------------------------------
// This protected function forms a string which contains complete service name for
// the current integrator.
//-------------------------------------------------------------------------------------
CString CIntegratorCtrl::GetServiceName() 
{	
	CString serviceName;
	CString hostName;
	CString userName;

   
    // Getting current host name  
    char hostNameSys[1024];
    gethostname(hostNameSys,1024);
	hostName = hostNameSys;

	// Getting current user name
	char*		userNameEnv;
	char		userNameSys[1024];
	DWORD		nUserNameSize = 1024;
	userNameEnv = getenv("USER");
	if(userNameEnv == NULL) {
		if(GetUserName(userNameSys,&nUserNameSize)==TRUE)
			userName = userNameSys;
	} else {
		userName = userNameEnv;
	}

    // Making service name string
	serviceName = hostName+":"+userName+":"+GetIntegratorType()+":"+GetIntegratorName();
	return serviceName;
}
//-------------------------------------------------------------------------------------

bool CIntegratorCtrl::ConnectToService(bool runServiceIfNeeded) {
	CString serviceName;
	int port;
	int address;
	SOCKADDR_IN serviceLoc;

	if(m_ConnectionChannel!=NULL) {
		Unregister();
		m_ConnectionChannel->Close();
		delete m_ConnectionChannel;
        m_ConnectionChannel = NULL;
	}

    char buf[1024];
    gethostname(buf, 1024);
    serviceName =  GetServiceName();
    static struct hostent *ent = gethostbyname(buf);
    switch(NameServerGetService(serviceName,port,address)) {
	    case -1 : RunNameServer(); // No name server detected
		case  0 : if(runServiceIfNeeded==false) return false;
			      if(RunDriver()!=true) { // No driver detected
			         return false; 
				  } 
			      NameServerGetService(serviceName,port,address);
		case  1 : 
	              m_ConnectionChannel=new CIntegratorSocket();
				  m_ConnectionChannel->SetControl(this);
				  m_ConnectionChannel->Create();
			      memset(&serviceLoc,0,sizeof(SOCKADDR_IN));
	              serviceLoc.sin_family=AF_INET;
	              serviceLoc.sin_port=htons(port);
	              serviceLoc.sin_addr=*(in_addr *)*ent->h_addr_list;
				  if(m_ConnectionChannel->Connect((SOCKADDR *)&serviceLoc,sizeof(serviceLoc))==false) {
					 delete m_ConnectionChannel;
					 m_ConnectionChannel = NULL;
					 return false;
				  } 
 
	}
	return Register();
}

bool CIntegratorCtrl::RunNameServer() {
	CString execName;
	char* psethomeEnv;
	STARTUPINFO si;
	PROCESS_INFORMATION pi;


	psethomeEnv = getenv("PSETHOME");
	execName="\"";
	if(psethomeEnv==NULL) {
       execName+="C:\\Discover\\";
	} else {
       int len = strlen(psethomeEnv);
       execName+=psethomeEnv;
       if(psethomeEnv[len - 1] != '\\') {
           execName += "\\";
       }
	}
	execName+="bin\\nameserv.exe";
	execName+="\"";

    memset(&si,0,sizeof(STARTUPINFO));
    si.wShowWindow= SW_HIDE;
	si.cb=sizeof(STARTUPINFO);
	if(::CreateProcess(NULL,execName.GetBuffer(50),NULL,NULL,FALSE,CREATE_NO_WINDOW,NULL,NULL,&si,&pi)==FALSE) {
		 return false;
	}
	CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
	return true;
}

bool CIntegratorCtrl::Unregister() {
    if(m_ConnectionChannel) {
        if(m_ConnectionChannel->SendCommand(CString("unregister"))) {
			CString szReply = m_ConnectionChannel->GetReply();
			return szReply.IsEmpty() || szReply.CompareNoCase("done")==0;
        }
    }
	return false;
}

bool CIntegratorCtrl::Register() {
	if(m_ConnectionChannel) {
		if(m_ConnectionChannel->SendCommand(CString("register"))) {
			CString szReply = m_ConnectionChannel->GetReply();
			return szReply.IsEmpty() || szReply.CompareNoCase("done")==0;
		}
	}
	return false;
}
