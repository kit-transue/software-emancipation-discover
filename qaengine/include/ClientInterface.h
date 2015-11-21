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
// lientInterface.h: interface for the ClientInterface class.
//
//////////////////////////////////////////////////////////////////////
#include "globals.h"
#include "../../dish/include/SocketComm.h"
#include "../../nameserver/include/nameServCalls.h"
#include "HitsTable.h"
#include "TaskDriver.h"
#ifdef _WIN32
    #include <windows.h>
    #include <winsock.h>
    #include <io.h>
#else
    #ifndef hp10
	#include <sysent.h>
    #endif
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <netinet/tcp.h>
    #include <arpa/inet.h>
    #include <netdb.h>
    #include <unistd.h>
    #ifdef sun4
        #include <poll.h>
    #endif
    #ifdef irix6
        #include <sys/poll.h>
    #endif
    #ifdef hp700
        #include <sys/poll.h>
        #include <time.h>
        extern "C" int select(int, int*, int*, int*, const struct timeval*);
    #endif
#endif
#ifndef _WIN32
#define closesocket(sock) close(sock)
#endif

#define SERVICE_EXISTS      -10
#define REGISTRATION_ERROR  -11

#define NO_DELTA      -1
#define ALL_HITS       0
#define NEW_HITS       1
#define REMOVED_HITS   2
#define UNCHANGED_HITS 3


class ClientInfo {
public :

	ClientInfo() {
		m_AdminProject  = new TaskDriver();
		m_BuildResults  = new TaskDriver();
		m_BuildBaseLine = new TaskDriver();
		m_DeltaMode     = NO_DELTA;
		m_IsAdmin       = 0;
    }

	~ClientInfo() {
		delete m_AdminProject;
		delete m_BuildResults;
		delete m_BuildBaseLine;
	}

    int getMode() {
		return m_DeltaMode;
	}

	void setMode(int mode) {
		m_DeltaMode = mode;
	}

    int isAdmin() {
		return m_IsAdmin;
	}

	void setAdmin(int isAdmin) {
		m_IsAdmin = isAdmin;
	}

	TaskDriver* getActiveProject() {
		if(m_IsAdmin) {
			return m_AdminProject;
		} else {
			return m_BuildResults;
		}
	}

	TaskDriver* getActiveBaseLine() {
		if(m_IsAdmin) return NULL;
        if(m_DeltaMode==NO_DELTA) return NULL;
		return m_BuildBaseLine;
    }

	TaskDriver*  getAdminProject() {
		return m_AdminProject;
	}

	TaskDriver*  getBuildResults() {
		return m_BuildResults;
	}

	TaskDriver*  getBuildBaseLine() {
		return m_BuildBaseLine;
	}

private :
	TaskDriver*  m_AdminProject;
	TaskDriver*  m_BuildResults;
	TaskDriver*  m_BuildBaseLine;
	int m_DeltaMode;
	int m_IsAdmin;
};


class ClientInterface  
{
public:
	bool IsServerRunning();
	HANDLE getModelServerHandle(void);
	void setModelServerHandle(int server);
	string getModelRoot(void);
	void setModelRoot(string root);
	string getSharedRoot(void);
	void setSharedRoot(string root);
	void requestsLoop(void);
	int registerService(string name);
	void setMessageGroup(string group);
	string getMessageGroup();
	void setLpService(string service);
	string getLpService();
	ClientInterface();
	virtual ~ClientInterface();
protected:
	virtual int processQuery(int socket);
private:
	int m_Socket;
	int m_Shutdown;
	string m_ModelRoot;
	string m_SharedRoot;
	string m_MessageGroup;
	string m_lpService;
	HANDLE m_ModelServerHandle;

	map<int,ClientInfo*>     m_Clients;

    int countHits(int socket,int id,ScopeSet scope);
    int  CheckSocket(int socket);
    int  createListener(void);
    int  createSocket(void);
    void WaitForAnyInput(void);
    void processClientRequest(void);
    int sendReply(int socket,string& reply);
    int getCommand(int socket,string &command);


	vector<string> parseCommand(string command);
    vector<string> getBuilds(void);
    vector<string> getProjects(void);

    int command_ListBuilds(string& reply);
    int command_Count(string& reply,int socket,ScopeSet scope);
    int command_Hitsmap(string& reply,int socket,ScopeSet scope);
    int command_Glass(string& reply,int socket,ScopeSet scope);
    int command_OpenBuild(string& reply,int socket,string build,ScopeSet scope);
    int command_OpenDelta(string& reply,int socket,string mode, string build1,string build2,ScopeSet scope);
    int command_PrintNode(string& reply,int socket,int id,ScopeSet scope);
    int command_ListChildren(string& reply,int socket,int id,ScopeSet scope);
    int command_ListHits(string& reply,int socket,int id,ScopeSet scope);
    int command_SetAttribute(string& reply,int socket,int id,string name,string value);
    int admin_ListProjects(string& reply);
    int admin_NewProject(string& reply,int socket,string projName,string copyFrom,ScopeSet scope);
    int admin_DupProject(string& reply,int socket,string projName,string copyFrom,ScopeSet scope);
    int admin_OpenProject(string& reply,int socket,string project,ScopeSet scope);
    int admin_SetAttribute(string& reply,int socket,int id,ScopeSet scope, string name,string value);
    int admin_RemoveNode(string& reply,int socket,int id);
    int admin_MoveNode(string& reply,int socket,int id,int parent);
    int admin_AddFolder(string& reply,int socket,int id,string name);
    int admin_AddQuery(string& reply,int socket,int id,string name);

};
