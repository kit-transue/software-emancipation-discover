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
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/types.h>
#include <unistd.h>

#if !defined(hp10) && !defined(irix6)
	extern "C" int gethostname(char *name, int namelen);
#endif

#include "Integrator.h"
#include "IntegratorSocket.h"
#include "nameServCalls.h"
#include "msg.h"
#include "SocketClient.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CIntegrator::CIntegrator() {
	m_ConnectionChannel = NULL;
        m_ServiceName.erase();
}

CIntegrator::~CIntegrator() {
	Unregister();
	if(m_ConnectionChannel) {
		m_ConnectionChannel->Close();
		delete m_ConnectionChannel;
	}
}

//-------------------------------------------------------------------------------------
// This protected function forms a string which contains complete service name for
// the current integrator.
//-------------------------------------------------------------------------------------

string CIntegrator::GetServiceName() 
{	
	if(m_ServiceName.empty()) {

	        string hostName;
	        string userName;
   
    	        // Getting current host name  
    	        char hostNameSys[1024];
    	        gethostname(hostNameSys,1024);
	        hostName = hostNameSys;

	        // Getting current user name
	        char*		userNameEnv;
	        userNameEnv = getenv("USER");
	        if(userNameEnv == NULL) {
		        userName = "";
	        } else {
		        userName = userNameEnv;
	        }

	        // Making service name string
	        m_ServiceName = hostName+":"+userName+":"+GetIntegratorType()+":"+GetIntegratorName();
        }

	return m_ServiceName;
}
//-------------------------------------------------------------------------------------

bool CIntegrator::ConnectToService(bool runServiceIfNeeded) {
	string serviceName;
	int port;
	int address;
	sockaddr_in serviceLoc;

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
	int nResult = 0;
	int nSecs = 30;
	int nAttempt = 0;

        msg("Looking for service '$1'... ", normal_sev) \
                                                << serviceName.c_str() << eom;

	int serverRes = NameServerGetService(serviceName.c_str(),port,address);

	switch(serverRes) {
		case -1 : 
                        msg("  'nameserv' is not detected!", normal_sev) << eom;
                        break;
		case  0 : 
                        msg("  Service '$1' is not detected.", normal_sev) \
                                                << serviceName.c_str() << eom;
                        break;
		case  1 :
                        msg("  Service '$1' is found.", normal_sev) \
                                                << serviceName.c_str() << eom;
                        break;
        }

	switch(serverRes) {
		case -1 : 
			RunNameServer(); // No name server detected
		case  0 : 
			if(runServiceIfNeeded==false) return false;
			if(RunDriver()!=true) { // No driver detected
				return false; 
			}
			do{ 
			   nResult = NameServerGetService(serviceName.c_str(),port,address);
			   nSecs--;
			} while(nResult != 1 && nSecs);
			if(nSecs==0)	return false;
		case  1 :
ANOTHER_ATTEMPT:
			m_ConnectionChannel=new CIntegratorSocket();
			m_ConnectionChannel->SetListener(this);
			m_ConnectionChannel->Create();
			memset(&serviceLoc,0,sizeof(sockaddr_in));
			serviceLoc.sin_family=AF_INET;
			serviceLoc.sin_port=htons(port);
			serviceLoc.sin_addr=*(in_addr *)*ent->h_addr_list;
			if(m_ConnectionChannel->Connect((sockaddr *)&serviceLoc,sizeof(serviceLoc))==false) {
				m_ConnectionChannel->Close();
				delete m_ConnectionChannel;
				m_ConnectionChannel = NULL;
				
				if(nAttempt == 0) {
				  if(runServiceIfNeeded==false) return false;
				  if(RunDriver()!=true) { // No driver detected
				    return false; 
				  }
				  do{ 
				    nResult = NameServerGetService(serviceName.c_str(),port,address);
				    nSecs--;
				  } while(nResult != 1 && nSecs);
				  if(nSecs==0)	return false;
				  nAttempt++;
				  goto ANOTHER_ATTEMPT;
				}
				return false;
			} 
 
	}
	return Register();
}

bool CIntegrator::RunNameServer() {
        return (start_name_server_process());
}

bool CIntegrator::Unregister() {
	if(m_ConnectionChannel) {
		string szUnregister("unregister");
		if(m_ConnectionChannel->SendCommand(szUnregister)) {
			string szReply = m_ConnectionChannel->GetReply();
			bool r = szReply.empty() || szReply.compare("done")==0;

                        if(r == true) {
	                        string serviceName =  GetServiceName();
                                msg("Service '$1' has been unregistered.", \
                                               normal_sev) \
                                               << serviceName.c_str() << eom;
                        }

                        return r;
		}
	}
	return false;
}

bool CIntegrator::Register() {
	if(m_ConnectionChannel) {
		string szRegister("register");
		if(m_ConnectionChannel->SendCommand(szRegister)) {
			string szReply = m_ConnectionChannel->GetReply();
			bool r = szReply.empty() || szReply.compare("done")==0;
                        if(r == true) {
	                        string serviceName =  GetServiceName();
                                msg("Service '$1' has been registered.", \
                                               normal_sev) \
                                               << serviceName.c_str() << eom;
                        }
                        return r;
		}
	}
	return false;
}
