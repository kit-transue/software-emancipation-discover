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
// lientInterface.cpp: implementation of the ClientInterface class.
//
//////////////////////////////////////////////////////////////////////

#include "ClientInterface.h"
#include "BatchBuild.h"
#ifndef _WIN32
  #include <unistd.h>
  #include <sys/time.h>
  #include <sys/resource.h>
  #include <sys/types.h>
  #include <sys/stat.h>
  #include <dirent.h>
  #include <pwd.h>
  #include <sys/wait.h>
#else
  #include <direct.h>
  #include <process.h>
  #include <io.h>
  #include <string.h>
#endif
#include <fcntl.h>
#define DATA_PACKET                 1
#define DATA_FINAL_PACKET           2
#define STDOUT_DISH_STREAM_CMD_TYPE 4

#error network byte order issue: use nameServCalls.h functions instead!

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

ClientInterface::ClientInterface() {
#ifdef _WIN32
    WORD    wVersionRequested;  
    WSADATA wsaData; 
    int     err; 

    wVersionRequested = MAKEWORD(1, 1); 
    err               = WSAStartup(wVersionRequested, &wsaData); 
#endif
	m_Socket = -1;
	m_Shutdown = 0;
	m_ModelServerHandle = NULL;
}

ClientInterface::~ClientInterface() {
	// Disconnect and remove all connected clients
	map<int,ClientInfo*>::iterator i;
	for(i=m_Clients.begin();i!=m_Clients.end();i++) {
		closesocket(i->first);
		delete i->second;
	}

	// Close listening socket
	if(m_Socket!=-1) {
		closesocket(m_Socket);
	}

}

HANDLE ClientInterface::getModelServerHandle(void) {
	return m_ModelServerHandle;
}

void ClientInterface::setModelServerHandle(int server){
	m_ModelServerHandle = OpenProcess(PROCESS_QUERY_INFORMATION,false,server);
}

//------------------------------------------------------------------------------
// This procedure will create a socket and set it to close on exec.
//------------------------------------------------------------------------------
int ClientInterface::createSocket() {
	if(m_Socket!=-1) {
		closesocket(m_Socket);
		m_Socket = -1;
	}
    int sock = socket (PF_INET, SOCK_STREAM, 0);

#ifndef _WIN32
    if (sock >= 0) {
	fcntl(sock, F_SETFD, 1);
    }
#endif
    m_Socket = sock;
    return sock;
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
// This function will check if data available in the selected socket
//------------------------------------------------------------------------------
int ClientInterface::CheckSocket(int socket) {
    timeval timeout;
    timeout.tv_sec  = 0;
    timeout.tv_usec = 0;
    fd_set sock_set;
    int    nfsd = 0;
#ifndef _WIN32
    nfsd = FD_SETSIZE;
#endif
    FD_ZERO(&sock_set);
    FD_SET(socket, &sock_set);
#ifdef hp700
    if(select(nfsd,(int *)&sock_set,NULL,NULL,&timeout)>0) {
#else
    if(select(nfsd,&sock_set,NULL,NULL,&timeout)>0) {
#endif
        return 1;
    }
    return 0;
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
// This function will wait any data on the client or admin socket
//------------------------------------------------------------------------------
void ClientInterface::WaitForAnyInput(void) {
    timeval timeout;
    timeout.tv_sec  = 1;
    timeout.tv_usec = 0;
    fd_set sock_set;
    int nfsd = 0;
#ifndef _WIN32
    nfsd = FD_SETSIZE;
#endif
    FD_ZERO(&sock_set);
	FD_SET(m_Socket, &sock_set);
	map<int,ClientInfo*>::iterator i;
	for(i=m_Clients.begin();i!=m_Clients.end();i++) {
	    FD_SET(i->first, &sock_set);
	}
#ifdef hp700
    select(nfsd,(int *)&sock_set,NULL,NULL,&timeout);
#else
    select(nfsd,&sock_set,NULL,NULL,&timeout);
#endif
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
// This procedure will create a listen socket for client requests 
// channel and for admin requests channel
//------------------------------------------------------------------------------
int ClientInterface::createListener() {
    int sock;
    struct sockaddr_in name;
    /* Create the socket. */
    sock = createSocket();
    if (sock < 0) return -1;

    unsigned int set_option = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (const char *)&set_option, sizeof(set_option));

    /* Give the socket a name. */
    name.sin_family      = AF_INET;
    name.sin_port        = 0;
    name.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind (sock, (struct sockaddr *) &name, sizeof (name)) < 0) return -1;
    if(listen(sock, 5) < 0) return -1;
    return sock;

}
//------------------------------------------------------------------------------

bool ClientInterface::IsServerRunning() {
	if(m_ModelServerHandle!=NULL) {
		DWORD lExitCode;
		if(GetExitCodeProcess(m_ModelServerHandle,&lExitCode)) {
			return lExitCode==STILL_ACTIVE;
		}
		DWORD error = GetLastError();
		return false;
	}
	return true;
}

int ClientInterface::registerService(string name) {
int service_port, tcp_addr;
int success;
     if(createListener()==-1) {
	     return REGISTRATION_ERROR;
	 }
     success = NameServerGetService(name.c_str(), service_port, tcp_addr);
     if(success >0) {
	    return SERVICE_EXISTS;
     }
     struct sockaddr_in socketInfo;
     int len = sizeof(socketInfo);
     if(getsockname(m_Socket, (struct sockaddr *)&socketInfo, &len) == 0) {
        return  NameServerRegisterService(name.c_str(),ntohs(socketInfo.sin_port));
	 }
	 return REGISTRATION_ERROR;
}

//------------------------------------------------------------------------------
// Loop while not shutting down. Will listen clients sockets
// and process clients requests if any.
//------------------------------------------------------------------------------
void ClientInterface::requestsLoop() {
    struct sockaddr s;
    int s_size = sizeof(s);
    m_Shutdown = 0;
    do {
	    WaitForAnyInput();
	    if (CheckSocket(m_Socket)) {
           // Request to the listening socket - new client try to connect
	       int connection = accept(m_Socket, &s, &s_size);
	       int tmp_switch = 1;
	       setsockopt(connection, IPPROTO_TCP, TCP_NODELAY, (char *)&tmp_switch, sizeof(tmp_switch));
	       if(connection >= 0) {
//			   cout << "New client created."<<endl;
			   m_Clients[connection] = new ClientInfo();
		   }
		} else {
           // Request to the client socket - client exec query.  
		   processClientRequest();
		}
		m_Shutdown = m_Shutdown || !IsServerRunning();
    } while(!m_Shutdown);
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// This function will process all administrator requests
//------------------------------------------------------------------------------
void ClientInterface::processClientRequest(void) {

	map<int,ClientInfo*>::iterator i;
	for(i=m_Clients.begin();i!=m_Clients.end();i++) {
		if(CheckSocket(i->first)) {
            if(processQuery(i->first)==-1) {
				// Client disconnected - we will free memory
				// allocated for the current build TaskDriver
				// and, if in the delta mode, memory allocated for the
				// old build TaskDriver
				delete m_Clients[i->first];
				m_Clients.erase(i);				
//                cout << "Client disconnected."<<endl;
				return;
			}
		}
	}
}
//------------------------------------------------------------------------------




//================================================================================
//================                  API COMMANDS              ====================
//================================================================================



//--------------------------------------------------------------------------------
// returns a vector of strings describing qa builds
//--------------------------------------------------------------------------------
vector<string> ClientInterface::getBuilds() {
vector<string> folders;
int handle;
string mask = m_ModelRoot+"/*";

#ifdef _WIN32
    struct _finddata_t fileinfo;
    handle = _findfirst(mask.c_str(),&fileinfo);
    if(handle==-1) {
		_findclose(handle);
		return folders;
	}
	do {
		if((fileinfo.attrib&_A_SUBDIR)!=0) {
			string dir = fileinfo.name;
			if(dir!="." && dir!="..") {
               // append only if hits.dat file exists in this folder
               string hitsFileName = m_ModelRoot+"/"+dir+"/hits.dat";
			   int fd = open(hitsFileName.c_str(),O_RDONLY);
			   if(fd>0) {
				   close(fd);
				   folders.push_back(dir);
			   }		   
			}
		}
	} while(_findnext(handle,&fileinfo)!=-1);
	_findclose(handle);
#else
    DIR *dp;
	struct dirent *dirp;
    struct stat statbuf;
	if( (dp = opendir(mask.c_str())) ==NULL) return folders;
	while( (dirp = readdir(dp)) !=NULL) {
  	    if(strcmp(dirp->d_name,".")==0 || strcmp(dirp->d_name,"..")==0) 
		     continue;
	    string dir = dirp->d_name;
		if(lstat(dir.c_str(),&statbuf) <0) continue;
		if(S_ISDIR(statbuf.st_mode) !=0) {
            string hitsFileName = m_ModelRoot+"/"+dir+"/hits.dat";
	        int fd = open(hitsFileName.c_str(),O_RDONLY);
			if(fd>0) {
			   close(fd);
			   folders.push_back(dir);
		   }
		}
	}
	closedir(dp);
#endif
	return folders;
}
//--------------------------------------------------------------------------------


//--------------------------------------------------------------------------------
// returns a vector of strings describing qa builds
//--------------------------------------------------------------------------------
vector<string> ClientInterface::getProjects() {
vector<string> projects;
int handle;

#ifdef _WIN32
	string mask = m_SharedRoot + "/*";
    struct _finddata_t fileinfo;
    handle = _findfirst(mask.c_str(),&fileinfo);
    if(handle==-1) {
		_findclose(handle);
		return projects;
	}
	do {
		if((fileinfo.attrib&_A_SUBDIR)==0) {
			string fname = fileinfo.name;
			int pos = fname.find(".policy.xml");
			if(pos>0) {
		       projects.push_back(fname.substr(0,pos));
            }
		}
	} while(_findnext(handle,&fileinfo)!=-1);
	_findclose(handle);
#else
	string mask = m_SharedRoot+"/*";
    DIR *dp;
	struct dirent *dirp;
    struct stat statbuf;
	if( (dp = opendir(mask.c_str())) ==NULL) return projects;
	while( (dirp = readdir(dp)) !=NULL) {
  	    if(strcmp(dirp->d_name,".")==0 || strcmp(dirp->d_name,"..")==0) 
		     continue;
	    string file = dirp->d_name;
		if(lstat(file.c_str(),&statbuf) <0) continue;
		if(S_ISDIR(statbuf.st_mode) ==0) {
			int pos = file.find(".policy.xml");
			if(pos>0) {
		       projects.push_back(file.substr(0,pos));
            }
		}
	}
	closedir(dp);
#endif
	return projects;
}
//--------------------------------------------------------------------------------


//--------------------------------------------------------------------------------
// Client wants to get a list of builds
//--------------------------------------------------------------------------------
int ClientInterface::command_ListBuilds(string& reply) {
	reply = "";
	vector<string> builds = getBuilds();
	vector<string>::iterator i;
	for(i=builds.begin();i!=builds.end();i++) {
		reply+=*i;
		reply+="\n";
	}
	return 0;
}
//--------------------------------------------------------------------------------

//--------------------------------------------------------------------------------
// Client wants to get a statistics
//--------------------------------------------------------------------------------
int ClientInterface::command_Count(string& reply,int socket,ScopeSet scope) {
	reply = "";
    TaskDriver* currentBuild = m_Clients[socket]->getActiveProject();
	reply = currentBuild->count(scope);
	return 0;
}
//--------------------------------------------------------------------------------


//--------------------------------------------------------------------------------
// Client wants to get a plain list of hits
//--------------------------------------------------------------------------------
int ClientInterface::command_Hitsmap(string& reply,int socket,ScopeSet scope) {
	reply = "";
    TaskDriver* currentBuild = m_Clients[socket]->getActiveProject();
	reply = currentBuild->hitsmap(scope);
	return 0;
}
//--------------------------------------------------------------------------------


//--------------------------------------------------------------------------------
// Client wants to get a plain list of hits
//--------------------------------------------------------------------------------
int ClientInterface::command_Glass(string& reply,int socket,ScopeSet scope) {
	reply = "";
    TaskDriver* currentBuild = m_Clients[socket]->getActiveProject();
	reply = currentBuild->glass(scope);
	return 0;
}
//--------------------------------------------------------------------------------


//--------------------------------------------------------------------------------
// Client wants to open specified build
//--------------------------------------------------------------------------------
int ClientInterface::command_OpenBuild(string& reply,int socket,string build,ScopeSet scope) {
string policyFileName  = m_ModelRoot+"/"+build+"/policy.xml";
string projectFileName = m_ModelRoot+"/"+build+"/project.xml";
string hitsFileName    = m_ModelRoot+"/"+build+"/hits.dat";

   TaskDriver* currentBuild = m_Clients[socket]->getActiveProject();
   m_Clients[socket]->setMode(NO_DELTA);

   reply = "";
   if(currentBuild->openPolicy(policyFileName)!=0) return -1;
   if(currentBuild->openHits(hitsFileName)!=0)     return -2;
   if(currentBuild->openProject(projectFileName)!=0) {
	   currentBuild->createProject(projectFileName);
   }
   int id = currentBuild->getRoot();
   char buf[100];
   sprintf(buf,"%d",id);
   reply = buf;
   reply+="\t";

   PropertiesMap map = currentBuild->getNodeProperties(id,scope);
   PropertiesMap::iterator i;
   for(i=map.begin();i!=map.end();i++) {
		reply+=i->first;
		reply+="\t";
		reply+=i->second;
		reply+="\t";
   }
   reply+="Hits";
   reply+="\t";
   sprintf(buf,"%d",currentBuild->getNodeHitsCount(id,scope));
   reply+=buf;
   return 0;
}
//--------------------------------------------------------------------------------



//--------------------------------------------------------------------------------
//--------------------------------------------------------------------------------
int ClientInterface::command_OpenDelta(string& reply,int socket,string mode, string build1,string build2,ScopeSet scope) {
string policyFileName  = m_ModelRoot+"/"+build1+"/policy.xml";
string projectFileName = m_ModelRoot+"/"+build1+"/project.xml";
string hitsFileName    = m_ModelRoot+"/"+build1+"/hits.dat";
string oldPolicyFileName  = m_ModelRoot+"/"+build2+"/policy.xml";
string oldProjectFileName = m_ModelRoot+"/"+build2+"/project.xml";
string oldHitsFileName    = m_ModelRoot+"/"+build2+"/hits.dat";

   m_Clients[socket]->setMode(ALL_HITS);
   if(mode=="new")       m_Clients[socket]->setMode(NEW_HITS);
   if(mode=="removed")   m_Clients[socket]->setMode(REMOVED_HITS);
   if(mode=="unchanged") m_Clients[socket]->setMode(UNCHANGED_HITS);


   reply = "";

   // Allocating memory and filling data from the old build
   TaskDriver* oldBuild        = m_Clients[socket]->getActiveBaseLine();
   if(oldBuild->openPolicy(oldPolicyFileName)!=0) return -1;
   if(oldBuild->openHits(oldHitsFileName)!=0)     return -2;
   if(oldBuild->openProject(oldProjectFileName)!=0) {
	   oldBuild->createProject(oldProjectFileName);
   }

   // Filling data for the current build
   TaskDriver* currentBuild    = m_Clients[socket]->getActiveProject();
   if(currentBuild->openPolicy(policyFileName)!=0) return -1;
   if(currentBuild->openHits(hitsFileName)!=0)     return -2;
   if(currentBuild->openProject(projectFileName)!=0) {
	   currentBuild->createProject(projectFileName);
   }

   // We will return root node for the current build
   // as a result of this command
   int id = currentBuild->getRoot();
   char buf[100];
   sprintf(buf,"%d",id);
   reply = buf;
   reply+="\t";
   PropertiesMap map = currentBuild->getNodeProperties(id,scope);
   PropertiesMap::iterator i;
   for(i=map.begin();i!=map.end();i++) {
		reply+=i->first;
		reply+="\t";
		reply+=i->second;
		reply+="\t";
   }
   reply+="Hits";
   reply+="\t";
   int oldHits = oldBuild->getNodeHitsCount(id,scope);
   int hits = currentBuild->getNodeHitsCount(id,scope);
   if(hits < oldHits) hits = oldHits;
   sprintf(buf,"%d",hits);
   reply+=buf;
   return 0;
}
//--------------------------------------------------------------------------------

//--------------------------------------------------------------------------------
// Client wants to get list of children
//--------------------------------------------------------------------------------
int ClientInterface::command_ListChildren(string& reply,int socket,int id,ScopeSet scope) {
	int hasDeviation = 0;
	reply = "";
    TaskDriver* task=m_Clients[socket]->getActiveProject();
	int ch = task->getFirstChild(id);
	while(ch>=0) {
		char buf[100];
		sprintf(buf,"%d",ch);
		reply+=buf;
	    reply+="\t";
		PropertiesMap map = task->getNodeProperties(ch,scope);
		PropertiesMap::iterator i;
		for(i=map.begin();i!=map.end();i++) {
			if(i->first.length()==0) reply+=" ";
			else reply+=i->first;
			reply+="\t";
			if(i->second.length()==0) reply+=" ";
			else reply+=i->second;
			reply+="\t";
		}
        int statistical = task->isStatistical(ch);
		if(!statistical) {
  		    reply+="Hits";
		    reply+="\t";
		    sprintf(buf,"%d",countHits(socket,ch,scope));
		    reply+=buf;
		} else {
            string kind = task->getNodeProperty(ch,scope,"Kind");
			if(kind=="Query") {
				HitsSet hits = task->getNodeHits(ch,scope);
				HitsSet::iterator i;
				int hitsCounter = 0;
				int globalTotal= 0;
				int globalMin = 100000000;
				int globalMax = 0;
				double globalAverage   = 0;
				double globalDeviation = 0;
				for(i=hits.begin();i!=hits.end();i++) {
					Hit* hit = *i;
					PropertiesMap* attributes = hit->getAttributes();
					PropertiesMap::iterator pos;
					pos = attributes->find("Minimum");
					if(pos!=attributes->end()) {
						int localMin = atoi(pos->second.c_str());
						if(globalMin>localMin) globalMin=localMin;
						globalTotal+=localMin;
					}
					pos = attributes->find("Maximum");
					if(pos!=attributes->end()) {
						int localMax = atoi(pos->second.c_str());
						if(globalMax<localMax) globalMax=localMax;
					}
					pos = attributes->find("Average");
					if(pos!=attributes->end()) {
						double localAverage = atof(pos->second.c_str());
						globalAverage+=localAverage;
					}
					pos = attributes->find("Deviation");
					if(pos!=attributes->end()) {
						hasDeviation = 1;
						double localDeviation = atof(pos->second.c_str());
						globalDeviation+=localDeviation;
					}
					hitsCounter++;
				}
				if(hitsCounter>0) {
					globalAverage/=hitsCounter;
					globalDeviation/=hitsCounter;
				}
				if(hasDeviation) {
   		            reply+="Minimum";
		            reply+="\t";
		            sprintf(buf,"%d",globalMin);
				    reply+=buf;
		            reply+="\t";
   		            reply+="Maximum";
		            reply+="\t";
		            sprintf(buf,"%d",globalMax);
				    reply+=buf;
		            reply+="\t";
   		            reply+="Average";
		            reply+="\t";
		            sprintf(buf,"%f",globalAverage);
				    reply+=buf;
		            reply+="\t";
   		            reply+="Deviation";
		            reply+="\t";
		            sprintf(buf,"%f",globalDeviation);
				    reply+=buf;
		            reply+="\t";
				} else {
   		            reply+="Total";
		            reply+="\t";
		            sprintf(buf,"%d",globalTotal);
				    reply+=buf;
		            reply+="\t";
				}
			}
			reply+="Statistical";
			reply+="\t";
			reply+="true";
		}
	    ch = task->getNextSibling(ch);
	    if(ch>=0) reply+="\n";
	}
	return 0;
}
//--------------------------------------------------------------------------------

//--------------------------------------------------------------------------------
// Client wants to get list of children
//--------------------------------------------------------------------------------
int ClientInterface::command_PrintNode(string& reply,int socket,int id,ScopeSet scope) {
	reply = "";
    TaskDriver* task=m_Clients[socket]->getActiveProject();
	int ch = task->getFirstChild(id);
	while(ch>=0) {
		char buf[100];
		sprintf(buf,"%d",ch);
		reply+=buf;
	    reply+="\t";
		PropertiesMap map = task->getNodeProperties(ch,scope);
		PropertiesMap::iterator i;
		for(i=map.begin();i!=map.end();i++) {
			reply+=i->first;
			reply+="\t";
			reply+=i->second;
			reply+="\t";
		}
		reply+="Hits";
		reply+="\t";
		sprintf(buf,"%d",task->getNodeHitsCount(ch,scope));
		reply+=buf;
		ch = task->getNextSibling(ch);
		if(ch>=0) reply+="\n";
	}
	return 0;
}
//--------------------------------------------------------------------------------

int ClientInterface::countHits(int socket, int id, ScopeSet scope) {
	string query;

    TaskDriver* newBuild    = m_Clients[socket]->getActiveProject();
	TaskDriver* oldBuild    = m_Clients[socket]->getActiveBaseLine();
	int mode = m_Clients[socket]->getMode();

	// Check if we are in delta mode.
	// open command creates and assignes old build to the m_BuildBaseLines
	// if it runs with two arguments
	if(oldBuild!=NULL) {
		// We need procedure attribute as a static id of the query node.
		// We will use it to search for the matching hits while
		// creating delta
		query = newBuild->getNodeProperty(id,scope,"Procedure");
	}

	int nHitsCount = 0;

	HitsSet newHits;
	HitsSet oldHits;
    HitsSet::iterator i;
	if(oldBuild==NULL) {
		// We are not in delta mode - so we will report hits as usual
        nHitsCount = newBuild->getNodeHitsCount(id,scope);
    } else {
        string kind = newBuild->getNodeProperty(id,scope,"Kind");
		if(kind=="Query") {
			// We are in delta mode so two passes will be executed - first to
			// report only new hits , second to report removed hist.
			newHits = newBuild->getNodeHits(id,scope);
			oldHits = oldBuild->getNodeHits(id,scope);
			// Pass 1 - we will compare current hist against old reference table
			// to detect new hits.
			for(i=newHits.begin();i!=newHits.end();i++) {
				Hit* hit = *i;
				HitsSet matches = oldBuild->getHitsTablePtr()->compare(query,*hit);
				// No matching hit in the reference set -
				// assuming that the hit is new
				if(matches.size()==0) {
					if(mode==ALL_HITS || mode==NEW_HITS) nHitsCount++;
				} else {
					if(matches.size()==1) {
						if(mode==ALL_HITS || mode==UNCHANGED_HITS) nHitsCount++;
					} else {
						// Multiple matching hits in the reference set -
						// we will try to resolve it using line numbers
						if(matches.size()>1) {
							// Trying to select hit using its line number
							// (will work if file was not modified)
							HitsSet::iterator j;
							for(j=matches.begin();j!=matches.end();j++) {
							   Hit* oldHit = *j;
							   if((oldHit->getLine()==hit->getLine()) && (oldHit->getModule()==hit->getModule())) {
								   break;
							   }
							}
							// No old hits with the same line number -
							// assuming this is new hit.
							if(j==matches.end()) {
								if(mode==ALL_HITS || mode==NEW_HITS) nHitsCount++;
							} else {
								if(mode==ALL_HITS || mode==UNCHANGED_HITS) nHitsCount++;
							}
						}
					}
				}
			}
			// -- Pass 1 ends

			// Pass 2 - we will compare old hist against current hits
			// to detect removed hits
			for(i=oldHits.begin();i!=oldHits.end();i++) {
				Hit* hit = *i;
				HitsSet matches = newBuild->getHitsTablePtr()->compare(query,*hit);
				// No matching hit in the reference set -
				// assuming that the hit is new
				if(matches.size()==0) {
					if(mode==ALL_HITS || mode==REMOVED_HITS) nHitsCount++;
				} else {
					// Multiple matching hits in the reference set -
					// we will try to resolve it using line numbers
					if(matches.size()>1) {
						// Trying to select hit using its line number
						// (will work if file was not modified)
						HitsSet::iterator j;
						for(j=matches.begin();j!=matches.end();j++) {
						   Hit* oldHit = *j;
						   if((oldHit->getLine()==hit->getLine()) && (oldHit->getModule()==hit->getModule())) {
							   break;
						   }
						}
						// No old hits with the same line number -
						// assuming this is new hit.
						if(j==matches.end()) {
							if(mode==ALL_HITS || mode==REMOVED_HITS) nHitsCount++;
						}
					}
				}
			}
			// -- Pass 2 ends
		} else {
			nHitsCount += newBuild->getNodeHitsCount(id,scope);
			nHitsCount += oldBuild->getNodeHitsCount(id,scope);
		}
	}
	return nHitsCount;
}
//--------------------------------------------------------------------------------

//--------------------------------------------------------------------------------
// Client hits to get list of hits
//--------------------------------------------------------------------------------
int ClientInterface::command_ListHits(string& reply,int socket, int id, ScopeSet scope) {
string query;

	reply = "";
    TaskDriver* newBuild    = m_Clients[socket]->getActiveProject();
	TaskDriver* oldBuild    = m_Clients[socket]->getActiveBaseLine();
	int mode = m_Clients[socket]->getMode();

	// Check if we are in delta mode.
	// open command creates and assignes old build to the m_BuildBaseLines
	// if it runs with two arguments
	if(oldBuild!=NULL) {
		// We need procedure attribute as a static id of the query node.
		// We will use it to search for the matching hits while
		// creating delta
		query = newBuild->getNodeProperty(id,scope,"Procedure");
	}


	HitsSet newHits;
	HitsSet oldHits;
    HitsSet::iterator i;
	if(oldBuild==NULL) {
		// We are not in delta mode - so we will report hits as usual
        newHits = newBuild->getNodeHits(id,scope);
	    for(i=newHits.begin();i!=newHits.end();i++) {
		    Hit* hit = *i;
		    reply+=hit->toString();
		    reply+="\n";
		}
    } else {
		// We are in delta mode so two passes will be executed - first to
		// report only new hits , second to report removed hist.
        newHits = newBuild->getNodeHits(id,scope);
        oldHits = oldBuild->getNodeHits(id,scope);
		// Pass 1 - we will compare current hist against old reference table
		// to detect new hits.
	    for(i=newHits.begin();i!=newHits.end();i++) {
		    Hit* hit = *i;
			HitsSet matches = oldBuild->getHitsTablePtr()->compare(query,*hit);
			// No matching hit in the reference set -
			// assuming that the hit is new
			if(matches.size()==0) {
				if(mode==ALL_HITS || mode==NEW_HITS) {
		            reply+=hit->toString();
			        reply+="\t";
			        reply+="Delta";
			        reply+="\t";
			        reply+="New";
			        reply+="\n";
				}
			}
            if(matches.size()==1) {
				if(mode==ALL_HITS || mode==UNCHANGED_HITS) {
		            reply+=hit->toString();
			        reply+="\t";
			        reply+="Delta";
			        reply+="\t";
			        reply+="Unchanged";
			        reply+="\n";
				}
			}

			// Multiple matching hits in the reference set -
			// we will try to resolve it using line numbers
			if(matches.size()>1) {
				// Trying to select hit using its line number
				// (will work if file was not modified)
				HitsSet::iterator j;
				for(j=matches.begin();j!=matches.end();j++) {
				   Hit* oldHit = *j;
				   if((oldHit->getLine()==hit->getLine()) && (oldHit->getModule()==hit->getModule())) {
					   break;
				   }
				}
                // No old hits with the same line number -
				// assuming this is new hit.
				if(j==matches.end()) {
				    if(mode==ALL_HITS || mode==NEW_HITS) {
		                reply+=hit->toString();
			            reply+="\t";
			            reply+="Delta";
			            reply+="\t";
			            reply+="New";
			            reply+="\n";
					}
				} else {
				    if(mode==ALL_HITS || mode==UNCHANGED_HITS) {
		                reply+=hit->toString();
			            reply+="\t";
			            reply+="Delta";
			            reply+="\t";
			            reply+="Unchanged";
			            reply+="\n";
					}
				}
			}
		}
		// -- Pass 1 ends

		// Pass 2 - we will compare old hist against current hits
		// to detect removed hits
	    for(i=oldHits.begin();i!=oldHits.end();i++) {
		    Hit* hit = *i;
			HitsSet matches = newBuild->getHitsTablePtr()->compare(query,*hit);
			// No matching hit in the reference set -
			// assuming that the hit is new
			if(matches.size()==0) {
				if(mode==ALL_HITS || mode==REMOVED_HITS) {
		            reply+=hit->toString();
			        reply+="\t";
			        reply+="Delta";
			        reply+="\t";
			        reply+="Removed";
			        reply+="\n";
				}
			}
			// Multiple matching hits in the reference set -
			// we will try to resolve it using line numbers
			if(matches.size()>1) {
				// Trying to select hit using its line number
				// (will work if file was not modified)
				HitsSet::iterator j;
				for(j=matches.begin();j!=matches.end();j++) {
				   Hit* oldHit = *j;
				   if((oldHit->getLine()==hit->getLine()) && (oldHit->getModule()==hit->getModule())) {
					   break;
				   }
				}
                // No old hits with the same line number -
				// assuming this is new hit.
				if(j==matches.end()) {
				    if(mode==ALL_HITS || mode==REMOVED_HITS) {
				        reply+=hit->toString();
			            reply+="\t";
			            reply+="Delta";
			            reply+="\t";
			            reply+="Removed";
			            reply+="\n";
					}
				}
			}
		}
		// -- Pass 2 ends

	}
	return 0;
}
//--------------------------------------------------------------------------------

//--------------------------------------------------------------------------------
// Client wants to get list of children
//--------------------------------------------------------------------------------
int ClientInterface::command_SetAttribute(string& reply,int socket,int id,string name,string value) {
	reply = "";
    TaskDriver* task=m_Clients[socket]->getActiveProject();
    task->setHitAttribute(id,name,value);
	return 0;
}
//--------------------------------------------------------------------------------


//--------------------------------------------------------------------------------
// Admin wants to get complete list of available projects
//--------------------------------------------------------------------------------
int ClientInterface::admin_ListProjects(string& reply) {
	reply = "";
	vector<string> projects = getProjects();
	vector<string>::iterator i;
	for(i=projects.begin();i!=projects.end();i++) {
		reply+=*i;
		reply+="\n";
	}
	return 0;
}
//--------------------------------------------------------------------------------

//--------------------------------------------------------------------------------
// Admin wants to open specified project
//--------------------------------------------------------------------------------
int ClientInterface::admin_OpenProject(string& reply,int socket,string project,ScopeSet scope) {
string policyFileName  = m_SharedRoot+"/"+project+".policy.xml";
string projectFileName = m_SharedRoot+"/"+project+".project.xml";

   TaskDriver* currentProject = m_Clients[socket]->getAdminProject();
   if(currentProject->openPolicy(policyFileName)!=0) return -1;
   if(currentProject->openProject(projectFileName)!=0) {
	   currentProject->createProject(projectFileName);
   }
   int id = currentProject->getRoot();
   char buf[100];
   sprintf(buf,"%d",id);
   reply = buf;
   reply+="\t";

   PropertiesMap map = currentProject->getNodeProperties(id,scope);
   PropertiesMap::iterator i;
   for(i=map.begin();i!=map.end();i++) {
		reply+=i->first;
		reply+="\t";
		reply+=i->second;
		reply+="\t";
   }
   return 0;
}
//--------------------------------------------------------------------------------

//--------------------------------------------------------------------------------
// Admin wants to create new project
//--------------------------------------------------------------------------------
int ClientInterface::admin_NewProject(string& reply,int socket,string projName,string copyFrom,ScopeSet scope) {
string policyFileName  = m_SharedRoot+"/"+projName+".policy.xml";
string projectFileName = m_SharedRoot+"/"+projName+".project.xml";

   TaskDriver* currentProject = m_Clients[socket]->getAdminProject();
   if(copyFrom.length()==0) {
       currentProject->createPolicy(policyFileName);
       currentProject->createProject(projectFileName);
   } else {
       string policyCopyName  = m_SharedRoot+"/"+copyFrom+".policy.xml";
       string projectCopyName = m_SharedRoot+"/"+copyFrom+".project.xml";
       currentProject->createPolicy(policyFileName,policyCopyName);
       currentProject->createProject(projectFileName,projectCopyName);
   }
   int id = currentProject->getRoot();
   char buf[100];
   sprintf(buf,"%d",id);
   reply = buf;
   reply+="\t";
   PropertiesMap map = currentProject->getNodeProperties(id,scope);
   PropertiesMap::iterator i;
   for(i=map.begin();i!=map.end();i++) {
		reply+=i->first;
		reply+="\t";
		reply+=i->second;
		reply+="\t";
   }
   return 0;
}
//--------------------------------------------------------------------------------


//--------------------------------------------------------------------------------
// Admin wants to create new project - copy of the one saved during build
//--------------------------------------------------------------------------------
int ClientInterface::admin_DupProject(string& reply,int socket,string projName,string copyFrom,ScopeSet scope) {
string policyFileName  = m_SharedRoot+"/"+projName+".policy.xml";
string projectFileName = m_SharedRoot+"/"+projName+".project.xml";

   TaskDriver* currentProject = m_Clients[socket]->getAdminProject();
   if(copyFrom.length()==0) {
       currentProject->createPolicy(policyFileName);
       currentProject->createProject(projectFileName);
   } else {
       string policyCopyName  = m_ModelRoot+"/"+copyFrom+"/policy.xml";
       string projectCopyName = m_ModelRoot+"/"+copyFrom+"/project.xml";
       currentProject->createPolicy(policyFileName,policyCopyName);
       currentProject->createProject(projectFileName,projectCopyName);
   }
   int id = currentProject->getRoot();
   char buf[100];
   sprintf(buf,"%d",id);
   reply = buf;
   reply+="\t";
   PropertiesMap map = currentProject->getNodeProperties(id,scope);
   PropertiesMap::iterator i;
   for(i=map.begin();i!=map.end();i++) {
		reply+=i->first;
		reply+="\t";
		reply+=i->second;
		reply+="\t";
   }
   return 0;
}
//--------------------------------------------------------------------------------


//--------------------------------------------------------------------------------
// Admin wants to set tree attribute
//--------------------------------------------------------------------------------
int ClientInterface::admin_SetAttribute(string& reply,int socket,int id,ScopeSet scope, string name,string value) {
    TaskDriver* task = m_Clients[socket]->getActiveProject();
    task->setNodeProperty(id,scope,name,value);
	return 0;
}
//--------------------------------------------------------------------------------



//--------------------------------------------------------------------------------
// Admin wants to remove node
//--------------------------------------------------------------------------------
int ClientInterface::admin_RemoveNode(string& reply,int socket,int id) {
    TaskDriver* task = m_Clients[socket]->getActiveProject();
    task->removeNode(id);
	return 0;
}
//--------------------------------------------------------------------------------

//--------------------------------------------------------------------------------
// Admin wants to add folder
//--------------------------------------------------------------------------------
int ClientInterface::admin_AddFolder(string& reply,int socket,int id,string name) {
    TaskDriver* task = m_Clients[socket]->getActiveProject();
    int newId=task->addNode(id,"Folder",name);
    char buf[100];
	sprintf(buf,"%d",newId);
	reply = buf;
	return 0;
}
//--------------------------------------------------------------------------------

//--------------------------------------------------------------------------------
// Admin wants to add query
//--------------------------------------------------------------------------------
int ClientInterface::admin_AddQuery(string& reply,int socket,int id,string name) {
    TaskDriver* task = m_Clients[socket]->getActiveProject();
    int newId = task->addNode(id,"Query",name);
    char buf[100];
	sprintf(buf,"%d",newId);
	reply = buf;
	return 0;
}
//--------------------------------------------------------------------------------

//--------------------------------------------------------------------------------
// Admin wants to move node
//--------------------------------------------------------------------------------
int ClientInterface::admin_MoveNode(string& reply,int socket,int id,int parent) {
    TaskDriver* task = m_Clients[socket]->getActiveProject();
    task->moveNode(id,parent);
	return 0;
}
//--------------------------------------------------------------------------------

int ClientInterface::processQuery(int socket) {
string command;
string reply="unknown command";
vector<string> tags;

	int res = getCommand(socket,command);
	if(res<0) return res;

//	cout << "Command receved : " << command << endl;
	tags = parseCommand(command);
	if(tags.size()>0) {
        vector<string>::iterator tag = tags.begin();
		vector<string>::iterator test = tags.begin();



	    // ------- Client wants to get a list of builds --------
		string cmd = *tag;
        if(cmd=="list" && tags.size()==1) {
			command_ListBuilds(reply);
		}
		//-------------------------------------------------------

		//----- Switching to admin mode. Project editing --------
        if(cmd=="admin" && tags.size()==1) {
			m_Clients[socket]->setAdmin(1);
			reply = "Working in admin mode.";
		}

		//------ Switching to user mode. Hits browsing ----------
        if(cmd=="user" && tags.size()==1) {
			m_Clients[socket]->setAdmin(0);
			reply = "Working in user mode.";
		}

	    // ------- Client wants to open specified build  --------
        if(cmd=="open" && tags.size()==2) {
			m_Clients[socket]->setAdmin(0);
			m_Clients[socket]->setMode(NO_DELTA);
			string build = *(++tag);
			ScopeSet scope;
			command_OpenBuild(reply,socket,build,scope);
		}
		//-------------------------------------------------------

	    // ------- Client wants to open specified build with selection --
        if(cmd=="extopen" && tags.size()>=3) {
			m_Clients[socket]->setAdmin(0);
			m_Clients[socket]->setMode(NO_DELTA);
			string build = *(++tag);
			ScopeSet scope;
			while(++tag!=tags.end()) {
				scope.insert(*tag);
			}
			command_OpenBuild(reply,socket,build,scope);
		}
		//-------------------------------------------------------

	    // ------- Client wants to open specified delta with selection --
        if(cmd=="extopend" && tags.size()>4) {
			m_Clients[socket]->setAdmin(0);
			m_Clients[socket]->setMode(ALL_HITS);
			string mode = *(++tag);
			string newBuild = *(++tag);
			string baseBuild = *(++tag);
			ScopeSet scope;
			while(++tag!=tags.end()) {
				scope.insert(*tag);
			}
			command_OpenDelta(reply,socket,mode, newBuild,baseBuild,scope);
		}
		//-------------------------------------------------------


	    // ------- Client wants to get total hits for the scope  -
        if(cmd=="print" && tags.size()==3) {
			string idStr = *(++tag);
            int id = atoi(idStr.c_str());
			ScopeSet scope;
			while(++tag!=tags.end()) {
				scope.insert(*tag);
			}
			command_PrintNode(reply,socket,id,scope);
		}
		//-------------------------------------------------------

	    // ------- Client wants to get statistics for the scope  -
        if(cmd=="count" && tags.size()>=1) {
			ScopeSet scope;
			while(++tag!=tags.end()) {
				scope.insert(*tag);
			}
			command_Count(reply,socket,scope);
		}
		//-------------------------------------------------------

	    // ------- Client wants to get plain map of hits  -
        if(cmd=="hitsmap" && tags.size()>=1) {
			ScopeSet scope;
			while(++tag!=tags.end()) {
				scope.insert(*tag);
			}
			command_Hitsmap(reply,socket,scope);
		}
		//-------------------------------------------------------

	    // ------- Client wants to get glass filter  -
        if(cmd=="glass" && tags.size()>=1) {
			ScopeSet scope;
			while(++tag!=tags.end()) {
				scope.insert(*tag);
			}
			command_Glass(reply,socket,scope);
		}
		//-------------------------------------------------------

	    // ------- Client wants to open specified pair of builds -
        if(cmd=="open" && tags.size()==3) {
			string build1 = *(++tag);
			string build2 = *(++tag);
			ScopeSet scope;
			command_OpenDelta(reply,socket,"all",build1,build2,scope);
		}
		//-------------------------------------------------------


	    // ------- Client wants to get list of children  --------
        if(cmd=="children" && tags.size()>=2) {
			string idStr = *(++tag);
            int id = atoi(idStr.c_str());
			ScopeSet scope;
			while(++tag!=tags.end()) {
				scope.insert(*tag);
			}
			command_ListChildren(reply,socket,id,scope);
		}
		//-------------------------------------------------------


	    // ------- Client hits to get list of hits  --------
        if(cmd=="hits" && tags.size()>=2  && !m_Clients[socket]->isAdmin()) {
			reply = "";
			string idStr = *(++tag);
            int id = atoi(idStr.c_str());
			ScopeSet scope;
			while(++tag!=tags.end()) {
				scope.insert(*tag);
			}
			command_ListHits(reply,socket,id,scope);

		}
		//-------------------------------------------------------


	    // ------- Client wants to assign attribute--------
        if(cmd=="attribute" && tags.size()>3) {
			reply = "";
			string idStr = *(++tag);
            int id = atoi(idStr.c_str());
			string name = *(++tag);
			string value = *(++tag);
			command_SetAttribute(reply,socket,id,name,value);

		}

	    // ------- Client wants to remove attribute--------
        if(cmd=="attribute" && tags.size()==3) {
			reply = "";
			string idStr = *(++tag);
            int id = atoi(idStr.c_str());
			string name = *(++tag);
			command_SetAttribute(reply,socket,id,name,"");

		}

		//-------------------------------------------------------
	    // Client wants to build new model with default policy
        if(cmd=="build" && tags.size()==3) {
	   	    BatchBuild buildProcess;
		    buildProcess.setSharedRoot(m_SharedRoot);
		    buildProcess.setBuildRoot(m_ModelRoot);
			string host    = *(++tag);
			string service = *(++tag);
		    buildProcess.connect(host,service);
		    int result = buildProcess.build("");
			buildProcess.disconnect();
			if(result==OK) {
				reply = "done";
			} else {
				reply = "error";
			}
		}

		//-------------------------------------------------------
	    // Client wants to build new model with specified policy
        if(cmd=="build" && tags.size()==4) {
	   	    BatchBuild buildProcess;
		    buildProcess.setSharedRoot(m_SharedRoot);
		    buildProcess.setBuildRoot(m_ModelRoot);
			string host    = *(++tag);
			string service = *(++tag);
			string project = *(++tag);
		    buildProcess.connect(host,service);
		    int result = buildProcess.build(project);
			buildProcess.disconnect();
			if(result==OK) {
				reply = "done";
			} else {
				reply = "error";
			}
		}


	    // ------- Admin wants to get a list of projects --------
        if(cmd=="projects" && tags.size()==1) {
			admin_ListProjects(reply);
		}
		//-------------------------------------------------------

		
		// ------- Admin wants to create new project --------
        if(cmd=="new" && tags.size()==2) {
			reply = "";
			m_Clients[socket]->setAdmin(1);
			string newName = *(++tag);
			ScopeSet scope;
			admin_NewProject(reply,socket,newName,"",scope);
		}

		// ------- Admin wants to create new project --------
        if(cmd=="new" && tags.size()==3) {
			reply = "";
			m_Clients[socket]->setAdmin(1);
			string newName = *(++tag);
			string copyName = *(++tag);
			ScopeSet scope;
			admin_NewProject(reply,socket,newName,copyName,scope);
		}

		// ------- Admin wants to create new project --------
        if(cmd=="new" && tags.size()>3) {
			reply = "";
			m_Clients[socket]->setAdmin(1);
			string newName = *(++tag);
			string copyName = *(++tag);
			ScopeSet scope;
			while(++tag!=tags.end()) {
				scope.insert(*tag);
			}
			admin_NewProject(reply,socket,newName,copyName,scope);
		}

		// ------- Admin wants to create new project --------
        if(cmd=="dup" && tags.size()==3) {
			reply = "";
			m_Clients[socket]->setAdmin(1);
			string newName = *(++tag);
			string copyName = *(++tag);
			ScopeSet scope;
			admin_DupProject(reply,socket,newName,copyName,scope);
		}

		// ------- Admin wants to create new project --------
        if(cmd=="dup" && tags.size()>3) {
			reply = "";
			m_Clients[socket]->setAdmin(1);
			string newName = *(++tag);
			string copyName = *(++tag);
			ScopeSet scope;
			while(++tag!=tags.end()) {
				scope.insert(*tag);
			}
			admin_DupProject(reply,socket,newName,copyName,scope);
		}



	    // ------- Admin wants to edit specified project  --------
        if(cmd=="edit" && tags.size()==2) {
			m_Clients[socket]->setAdmin(1);
			string project = *(++tag);
			ScopeSet scope;
			admin_OpenProject(reply,socket,project,scope);
		}
		//-------------------------------------------------------

	    // ------- Admin wants to set attribute--------
        if(cmd=="set" && tags.size()>2 && m_Clients[socket]->isAdmin()) {
			reply = "";
			string idStr = *(++tag);
            int id = atoi(idStr.c_str());
			string name = *(++tag);
			string value;
			if(tags.size()>3)
				value = *(++tag);
			ScopeSet scope;
			while(++tag!=tags.end()) {
				scope.insert(*tag);
			}
			admin_SetAttribute(reply,socket,id,scope,name,value);
		}

	    // ------- Admin wants to remove node --------
        if(cmd=="delete" && tags.size()==2 && m_Clients[socket]->isAdmin()) {
			reply = "";
			string idStr = *(++tag);
            int id = atoi(idStr.c_str());
			admin_RemoveNode(reply,socket,id);
		}

	    // ------- Admin wants to move node to another parent --------
        if(cmd=="movenode" && tags.size()==3 && m_Clients[socket]->isAdmin()) {
			reply = "";
			string idStr = *(++tag);
            int id = atoi(idStr.c_str());
			idStr = *(++tag);
            int parent = atoi(idStr.c_str());
			admin_MoveNode(reply,socket,id,parent);
		}

		// ------- Admin wants to add folder --------
        if(cmd=="addfolder" && tags.size()==3 && m_Clients[socket]->isAdmin()) {
			reply = "";
			string idStr = *(++tag);
            int parent = atoi(idStr.c_str());
			string name = *(++tag);
			admin_AddFolder(reply,socket,parent,name);
		}

	    // ------- Admin wants to add query --------
        if(cmd=="addquery" && tags.size()==3 && m_Clients[socket]->isAdmin()) {
			reply = "";
			string idStr = *(++tag);
            int parent = atoi(idStr.c_str());
			string name = *(++tag);
			admin_AddQuery(reply,socket,parent,name);

		}
	    // ------- Admin or user wants to save current project --------
		if(cmd=="save" && tags.size()==1) {
			reply = "";
			TaskDriver* task=m_Clients[socket]->getActiveProject();
			task->save();
		}

	    // ------- Admin wants to save current project under different name--------
		if(cmd=="save" && tags.size()==2 && m_Clients[socket]->isAdmin()) {
			reply = "";
			string projectName = *(++tag);
            string policy  =  m_SharedRoot+"/"+projectName+".policy.xml";
            string project =  m_SharedRoot+"/"+projectName+".project.xml";
			TaskDriver* task=m_Clients[socket]->getActiveProject();
			task->save(policy,project,"");
		}
	    // ------- Somebody wants to close current project without saving --------
		if(cmd=="close" && tags.size()==1) {
			reply = "";
			TaskDriver* task=m_Clients[socket]->getActiveProject();
			task->close();
		}
		if(cmd=="bye" && tags.size()==1) {
			m_Shutdown = 1;
		}

	}
	sendReply(socket,reply);
	return res;
}



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int ClientInterface::getCommand(int socket,string &command) {
char packetType;
unsigned char packetNum;
char lenStr[4];
unsigned len;

  if (socket <= 0) return 0;

  if (recv(socket, (char *) &packetNum, 1, 0) <= 0)  return -1;
  if (recv(socket, &packetType, 1, 0) <= 0)          return -1;

  if (recv(socket, lenStr, 4, 0) <= 0)               return -1;
  len = (unsigned)((unsigned char)lenStr[0])+
	    (((unsigned)(unsigned char)lenStr[1])<<8)+
		(((unsigned)(unsigned char)lenStr[2])<<16)+
		(((unsigned)(unsigned char)lenStr[3])<<24);  
  char *tmp = new char[len];

  int total=0;
  do {
	  int receved = recv(socket,tmp+total, len-total, 0);
	  if(receved<=0) {
		  delete tmp;
		  return -1;
	  }
	  total+=receved;
  } while(total<len);
  command = tmp;
  delete [] tmp;
  return 0;
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int ClientInterface::sendReply(int socket, string& reply) {
char buf[1024];
unsigned bufLen = 0;
char packetType;
int packetNum = 0;
unsigned char resultCode=1;

  if (socket <= 0) return 0;
  int len = reply.length();
  for(int i = 0;i<=len;i++) {

	  if(i!=len) {
	      buf[bufLen++]=reply[i];
	  } else {
		  buf[bufLen++] = 0;
	  }

	  if(bufLen == 1023) {
		  if(i==len) {
			  packetType = DATA_FINAL_PACKET; 
		  } else {
			  packetType = DATA_PACKET;
			  buf[bufLen++]=0;
		  }
          if (send(socket, (char *) &packetNum,  1, 0) <= 0)  return -1;
          if (send(socket, (char *) &packetType, 1, 0) <= 0)  return -1;
          if (send(socket, (char *) &resultCode, 1, 0) <= 0)  return -1;

          unsigned char lbuf[4];
          lbuf[0]  = (unsigned char)(bufLen & 255);
          lbuf[1]  = (unsigned char)((bufLen >> 8) & 255);
          lbuf[2]  = (unsigned char)((bufLen >> 16) & 255);
          lbuf[3]  = (unsigned char)((bufLen >> 24) & 255);
		  int sended = 0;
		  do {
			  int res = send(socket, (char *)lbuf,4-sended,0);
              if (res <= 0)   return -1;
			  sended+=res;
		  } while(sended!=4);

		  sended = 0;
		  do {
			  int res = send(socket, buf, bufLen-sended, 0);
              if (res <= 0) return -1;
			  sended+=res;
		  } while(sended<bufLen);
		  packetNum++;
		  bufLen = 0;
	  }
  }
  if(bufLen>0) {
	  packetType = DATA_FINAL_PACKET; 
      if (send(socket, (char *) &packetNum, 1, 0) <= 0)  return -1;
      if (send(socket, &packetType, 1, 0) <= 0)  return -1;
      if (send(socket, (char *)&resultCode, 1, 0) <= 0)  return -1;
      unsigned char lbuf[4];
      lbuf[0]  = (unsigned char)(bufLen & 255);
      lbuf[1]  = (unsigned char)((bufLen >> 8) & 255);
      lbuf[2]  = (unsigned char)((bufLen >> 16) & 255);
      lbuf[3]  = (unsigned char)((bufLen >> 24) & 255);
      int sended = 0;
	  do {
		  int res = send(socket, (char *)lbuf,4-sended,0);
          if (res <= 0)   return -1;
		  sended+=res;
	  } while(sended!=4);
	  sended = 0;
	  do {
		  int res = send(socket, buf, bufLen-sended, 0);
          if (res <= 0) return -1;
		  sended+=res;
	  } while(sended<bufLen);
  }
  return 0;
}
//------------------------------------------------------------------------------


vector<string> ClientInterface::parseCommand(string command) {
vector<string> tags;
string      tag;
int isTag = 0;

    for(int i = 0; i<command.length();i++) {
		if(command[i]=='"') {
			isTag=++isTag%2;
			if(isTag==0 && (i+1)>=command.length()) { // processing last empty tag...
				tags.push_back(tag);
				tag = "";
			}
			continue;
		}
		if(command[i]=='\n' || command[i]=='\r' || (isTag ==0 && (command[i]==' '|| command[i]=='\t'))) {
			tags.push_back(tag);
			tag = "";
			continue;
		}
		tag+=command[i];
	}
	if(tag.length()>0) tags.push_back(tag);
	return tags;
}



void ClientInterface::setModelRoot(string root) {
	m_ModelRoot = root;

}

string ClientInterface::getModelRoot() {
	return m_ModelRoot;

}

void ClientInterface::setSharedRoot(string root) {
	m_SharedRoot = root;

}

string ClientInterface::getSharedRoot() {
	return m_SharedRoot;

}

void ClientInterface::setMessageGroup(string group) {
	m_MessageGroup = group;

}

string ClientInterface::getMessageGroup() {
	return m_MessageGroup;

}

void ClientInterface::setLpService(string service) {
	m_lpService = service;

}

string ClientInterface::getLpService() {
	return m_lpService;

}
