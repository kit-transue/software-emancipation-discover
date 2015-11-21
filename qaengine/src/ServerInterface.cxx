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
// ServerInterface.cpp: implementation of the ServerInterface class.
//
//////////////////////////////////////////////////////////////////////

#include "ServerInterface.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

ServerInterface::ServerInterface() {
    m_Socket = -1;
#ifdef _WIN32
    WORD    wVersionRequested;  
    WSADATA wsaData; 
    int     err; 

    wVersionRequested = MAKEWORD(1, 1); 
    err               = WSAStartup(wVersionRequested, &wsaData); 
#endif
	m_BatchScriptSourced=false;
	m_PreviousModule="";
}

ServerInterface::~ServerInterface() {
}

int ServerInterface::isConnected() {
	if(m_Socket!=-1) return 1;
	return 0;

}

int ServerInterface::connect(string host, string service) {
int service_port, tcp_addr;
int success;
	if(host=="") {
        success = NameServerGetService(service.c_str(), service_port, tcp_addr);
	} else {
        success = NameServerGetServiceRemote(host.c_str(),service.c_str(), service_port, tcp_addr);
	}
    if(success < 0) {
	    return -1;
    }
    if(success == 0) {
	    return -1;
    }
    m_Socket = ConnectToServer(service_port, tcp_addr);
	m_SourcedScripts.clear();
	m_BatchScriptSourced = 0;
    return m_Socket;
}

string ServerInterface::eval(string command) {
int    success = 0;
char*  reply;
string results;

    DISCommand *cmd;
    cmd = new DISCommand((char *)command.c_str(),STDOUT_DISH_STREAM_CMD_TYPE);
    SendCommand(m_Socket, cmd);
    unsigned char result_code;
    success = ReceiveReply(m_Socket, &reply, result_code);
    delete cmd;
	if(success) {
	    results.append(reply);
		delete reply;
	}
	return results;
}

ScopeSet ServerInterface::getBuildScope() {
ScopeSet scope;
string module="";
int skipNumbers = 1;

    string reply = eval("printformat \"%s\" lname;defines -modules /");
    for(int i = 0; i<reply.length();i++) {
		if(reply[i]=='\n') {
			if(!skipNumbers) {
			   scope.insert(module);
			}
 		    module = "";
			skipNumbers = 1;
			continue;
		}
		if(reply[i]==' ' && skipNumbers) {
			skipNumbers = 0;
			continue;
		}
		if(!skipNumbers) {
			module+=reply[i];
		}
	}
	return scope;
}

HitsSet ServerInterface::execQuery(int type, string script,string command,string module,string arguments) {
HitsSet hits;
string reply;

    if(!m_BatchScriptSourced) {
       reply = eval("source_dis sev_batch.dis");
       m_BatchScriptSourced=1;
	}
    if(module!="")  {

		// Sourcing query script only if it is passed and was not already sourced
	    if(script!="") {
			vector<string>::iterator i;
			for(i=m_SourcedScripts.begin();i!=m_SourcedScripts.end();i++) {
				if(*i==script) break;
			}
			if(i==m_SourcedScripts.end()) {
		        reply = eval((string)"source_dis "+script);
                m_SourcedScripts.push_back(script);
			}
		}
	    if(command!="") {
			if(arguments!="") {
				reply = eval("printformat \"%s\t%s\t%s\t%s\" astname line astpath; "+command+" {"+module+"} "+arguments);
			} else {
		        reply = eval("printformat \"%s\t%s\t%s\t%s\" astname line astpath; "+command+" {"+module+"}");
			}

		    int index = reply.find_last_of("% ")-1;
			if(index>=0) {
			   reply = reply.substr(0,index);
			}
 		    index = reply.find_last_of("\n");
			if(index>=0) {
			   reply = reply.substr(0,index);
			}
			if(reply.length()>0) {
   		        string pname = eval("printformat \"%s\" pname; pname {"+module+"}");
                int index = pname.find_last_of("% ")-1;
		        if(index>=0) {
		           pname = pname.substr(0,index);
				}
	            index = pname.find_last_of("\n");
	            if(index>=0) {
	               pname = pname.substr(0,index);
				}
				if(!type) {
					hits = parseNormalHits(reply,pname);
				} else {
			        // We will save file using it's pname
					hits = parseStatisticalHits(reply,pname);
				}
			}      
			if(m_PreviousModule!=module) {
				if(m_PreviousModule!="") {
					reply = eval("src_unload "+m_PreviousModule);
				}
				m_PreviousModule = module;
			}
		}
	}
	return hits;
}

HitsSet ServerInterface::parseNormalHits(string data,string pname) {
string name="";
string line="";
string ast="";
HitsSet hits;
int numberPassed=0;
int    fieldIndex=0;
string fieldValue="";


    for(int i=0;i<data.length();i++) {
		if(!numberPassed && data[i]==' ') {
			while(data[i]==' ' && i<data.length()) i++;
			numberPassed = 1;
		}
		if(data[i]=='\n') {
	        if(fieldValue.length()>0) {
                switch(fieldIndex) {
	                case 0 : name   = fieldValue; break;
	                case 1 : line   = fieldValue; break;
	                case 2 : ast    = fieldValue; break;
				}
		        fieldIndex++;
			    fieldValue = "";
			}
	        if(fieldIndex>2) {
		       int iLine = atoi(line.c_str());
		       if(name.length()==0) {
			      name = pname+":"+line;
			   }
		       Hit* newHit = new Hit(name,pname,iLine,ast);
		       hits.push_back(newHit);
			}
			numberPassed = 0;
			fieldIndex   = 0;
            name="";
            line="";
            ast="";
		}
		if(numberPassed) {
		    if(data[i]=='\t') {
			    switch(fieldIndex) {
			        case 0 : name   = fieldValue; break;
			        case 1 : line   = fieldValue; break;
			        case 2 : ast    = fieldValue; break;
				}
			    fieldValue="";
			    fieldIndex++;
			    continue;
			}
		    fieldValue+=data[i];
		}
	}

	// Processing end of data block
	if(fieldValue.length()>0) {
        switch(fieldIndex) {
	        case 0 : name   = fieldValue; break;
	        case 1 : line   = fieldValue; break;
	        case 2 : ast    = fieldValue; break;
		}
		fieldIndex++;
		fieldValue = "";
	}
	if(fieldIndex>2) {
		int iLine = atoi(line.c_str());
		if(name.length()==0) {
			name = pname+":"+line;
		}
		Hit* newHit = new Hit(name,pname,iLine,ast);
		hits.push_back(newHit);
	}
    return hits;
}



HitsSet ServerInterface::parseStatisticalHits(string data,string pname) {
HitsSet hits;
string field;
int fieldCounter = 0;

    Hit* newHit = new Hit("statistics",pname,-1,"");
    for(int i = 0; i<data.length();i++) {
		if(data[i]=='\n') {
			if(field.length()>0) {
			    switch(fieldCounter) {
			        case 0 : (*newHit->getAttributes())["Minimum"]=field;
					    	 break;
			        case 1 : (*newHit->getAttributes())["Maximum"]=field;
					    	 break;
			        case 2 : (*newHit->getAttributes())["Average"]=field;
					    	 break;
			        case 3 : (*newHit->getAttributes())["Deviation"]=field;
					    	 break;
   		            case 4 : (*newHit->getAttributes())["U1"]=field;
					         break;
		            case 5 : (*newHit->getAttributes())["U2"]=field;
					         break;
				}
				fieldCounter++;
			}
 	        if(fieldCounter>0)
	            hits.push_back(newHit);
		    else
				delete newHit;
			newHit = new Hit("statistics",m_PreviousModule,-1,"");
			field="";
			fieldCounter=0;
			continue;
		}
		if(data[i]==' ') {
			if(field.length()>0) {
			    switch(fieldCounter) {
			        case 0 : (*newHit->getAttributes())["Minimum"]=field;
					    	 break;
			        case 1 : (*newHit->getAttributes())["Maximum"]=field;
					    	 break;
			        case 2 : (*newHit->getAttributes())["Average"]=field;
					    	 break;
			        case 3 : (*newHit->getAttributes())["Deviation"]=field;
					    	 break;
   		            case 4 : (*newHit->getAttributes())["U1"]=field;
					         break;
		            case 5 : (*newHit->getAttributes())["U2"]=field;
					         break;
				}
			    fieldCounter++;
			}
			field="";
			continue;
		}
		field+=data[i];

	}
	if(field.length()>0) {
		switch(fieldCounter) {
		    case 0 : (*newHit->getAttributes())["Minimum"]=field;
					 break;
		    case 1 : (*newHit->getAttributes())["Maximum"]=field;
					 break;
		    case 2 : (*newHit->getAttributes())["Average"]=field;
					 break;
		    case 3 : (*newHit->getAttributes())["Deviation"]=field;
					 break;
		    case 4 : (*newHit->getAttributes())["U1"]=field;
					 break;
		    case 5 : (*newHit->getAttributes())["U2"]=field;
					 break;
		}
		fieldCounter++;
	}
	if(fieldCounter>0)
	   hits.push_back(newHit);
	else
  	   delete newHit;
	return hits;
}

void ServerInterface::disconnect() {
	if(isConnected()) {
        closesocket(m_Socket);
        m_Socket = -1;
	}
}
