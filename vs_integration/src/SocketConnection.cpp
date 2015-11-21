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
// SocketConnection.cpp: implementation of the SocketConnection class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "VSAddIn.h"
#include "SocketConnection.h"
#include "nameServCalls.h"
#include <winsock.h>
#include <io.h>
#define SERVICE_NAME "Integrator"
#include "DSAddIn.h"

#define ACCEPT_SOCKET_MESSAGE 32600
#define CLIENT_SOCKET_MESSAGE 32601

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

static SocketConnection* connection;

#error network byte order issue: use nameServCalls.h functions instead!

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
SocketConnection::SocketConnection() {
	SocketConnection(NULL);
}

SocketConnection::SocketConnection(CCommandsObj* cmds) {

   connection = this;
   InitializeWindowsSocketSupport();
   m_Listener = CreateServerSocket();
   if(m_Listener!=-1) {
       WSAAsyncSelect(m_Listener, m_ProxyHwnd, ACCEPT_SOCKET_MESSAGE, FD_ACCEPT);
	   RegisterService(m_Listener);
   }
   m_pCommandProcessor = cmds;

}

SocketConnection::~SocketConnection()
{

}

bool SocketConnection::SendMessageToClients(CString msg) {
    POSITION pos = m_Clients.GetHeadPosition();
	while(pos!=NULL) {
		int socket = m_Clients.GetAt(pos);
		send(socket,msg.GetBuffer(10),msg.GetLength(),0);
		m_Clients.GetNext(pos);
    }
	return true;

}



//------------------------------------------------------------------------------
// This function will start the name_server.  Returns false to indicate 
// failure, true to indicate that process was started.
//------------------------------------------------------------------------------
bool SocketConnection::StartNameServer(void) {
    char cmd[1024];
    char *p;    
    p=getenv("CODE_ROVER_HOME");
	
	if(p==NULL)
		p=getenv("PSETHOME");
	
    if(p!=NULL) {
	    sprintf(cmd,"%s/bin/nameserv",p);
    } else {
	    sprintf(cmd,"nameserv");
    }
    STARTUPINFO si;
    PROCESS_INFORMATION pi;     
    memset(&si,0,sizeof(STARTUPINFO));
    si.wShowWindow=SW_HIDE;
    si.cb=sizeof(STARTUPINFO);
    if(::CreateProcess(NULL,(char *)cmd,NULL,NULL,FALSE,CREATE_NO_WINDOW,NULL,NULL,&si,&pi)==FALSE) {
  	    return false;
    } else {
	    return true;
    }
}
//------------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// This private function creates the listening socket to which all MSDEV 
// clients can connect.
//-----------------------------------------------------------------------------
int SocketConnection::CreateServerSocket(void) {
int sock;
struct sockaddr_in name;

    // Create the socket 
    sock = socket (PF_INET, SOCK_STREAM, 0);
    if (sock < 0) return -1;

    unsigned int set_option = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (const char *)&set_option, sizeof(set_option));

    /* Give the socket a name. */
    name.sin_family      = AF_INET;
    name.sin_port        = 0;
    name.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind (sock, (struct sockaddr *) &name, sizeof (name)) < 0)
	return -1;
    if(listen(sock, 5) < 0) return -1;
    return sock;
}
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// This private function registers service port and TCP address in the name 
// server under the "MSDEV:<host>:SERVICE_NAME" name.
//-----------------------------------------------------------------------------
bool SocketConnection::RegisterService(int socket) {
	// First, make sure that nameserv is up and accepting connections
	int test_sock = ConnectToNameServer();
	if (test_sock < 0) {
	    // Couldn't connect, so try to start nameserv
	    if(StartNameServer()!=0) {
		    // Wait for nameserv to start accepting connections
		    for (int count = 0; count < 3000 ; count++) {
		        test_sock = ConnectToNameServer();
		        if (test_sock >= 0) {
  			        break;
				}
			}
	    }
	}
	if (test_sock >= 0) {
	    closesocket (test_sock);
	} else {
		return false;
	}

    struct sockaddr_in socketInfo;
    int len = sizeof(socketInfo);
	int res = -1;
    if(getsockname(socket, (struct sockaddr *)&socketInfo, &len) == 0) {
        res = NameServerRegisterService(ServiceName(),ntohs(socketInfo.sin_port));
	}
	if (res == 1) {
	    return true;
	} else {
	    return false;
	}

}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// This private function returns the service name under which add-in must 
// be registered.
//-----------------------------------------------------------------------------
CString SocketConnection::ServiceName() {
static 	char	editorName[1024];
	char*		currentUser;
	char		userString[1024];
	DWORD		nUserSize = 1024;
	char*		dnStart;

	gethostname(editorName,1024);
	if((dnStart = strchr(editorName,'.'))!=NULL)
		editorName[dnStart-editorName] = 0;
	strcat(editorName,":");
	currentUser = getenv("USER");
	if(currentUser == NULL) {
		if(GetUserName(userString,&nUserSize)==TRUE)
			currentUser = userString;
	}

	if(currentUser != NULL) {
		strcat(editorName,currentUser);
		strcat(editorName,":");
	}
	strcat(editorName,"Editor:Microsoft Visual Studio");
	return editorName;
//	return "Editor:Microsoft Visual Studio"; 
/*	CString service;
	int service_port;
	int tcp_addr;
	char host[1024];
	host[0]=0;
	gethostname(host,1024);
	for(int i=0;i<100;i++) {
	   service.Format("MSDEV%d:%s:%s",i,host,SERVICE_NAME);
	   switch(NameServerGetService(service, service_port, tcp_addr)) {
		   case 1  : break;
		   case 0  : goto OK;
		   case -1 : return "";
	   }
	}
	OK:
	return service; */
}
//-----------------------------------------------------------------------------



LRESULT CALLBACK SocketWindowProc(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam) {
  if(umsg == ACCEPT_SOCKET_MESSAGE){
     struct sockaddr s;
     int s_size                     = sizeof(s);
     int new_socket                 = accept(wparam, &s, &s_size);
     connection->AddClient(new_socket);
     WSAAsyncSelect(new_socket, connection->m_ProxyHwnd, CLIENT_SOCKET_MESSAGE, FD_READ | FD_CLOSE);
     int tmp_switch = 1;
     setsockopt(new_socket, IPPROTO_TCP, TCP_NODELAY, (char *)&tmp_switch, sizeof(tmp_switch));
  }
  if(umsg == CLIENT_SOCKET_MESSAGE) {
	  connection->ProcessClientsRequests();

  }
  return DefWindowProc(hwnd, umsg, wparam, lparam);
}


//-----------------------------------------------------------------------------
// This private function will start windows socket support and will create 
// invisible window which will receve all socket messages.
//-----------------------------------------------------------------------------
void SocketConnection::InitializeWindowsSocketSupport(void) {
    WORD wVersionRequested;  
    WSADATA wsaData; 
    int err; 
   
    m_ProxyHwnd       = NULL;
    wVersionRequested = MAKEWORD(1, 1); 
    err               = WSAStartup(wVersionRequested, &wsaData); 
    if (err != 0) {
      return;
    }
    WNDCLASS socket_window;
    socket_window.style         = 0;
    socket_window.lpfnWndProc   = SocketWindowProc;
    socket_window.cbClsExtra    = 0;
    socket_window.cbWndExtra    = 0;
    socket_window.hInstance     = GetModuleHandle(NULL);
    socket_window.hIcon         = NULL;
    socket_window.hCursor       = NULL;
    socket_window.hbrBackground = NULL;
    socket_window.lpszMenuName  = NULL;
    socket_window.lpszClassName = "_MSDEV_SOCKET_WIN";
    ATOM cl   = RegisterClass(&socket_window);
    m_ProxyHwnd = CreateWindow("_MSDEV_SOCKET_WIN", "_MSDEV_SOCKET_WIN", 0, 0, 0, 0, 0,
			      NULL, NULL, GetModuleHandle(NULL), NULL);
}
//-----------------------------------------------------------------------------



//-----------------------------------------------------------------------------
// This private function will add client descriptor (socket) to the clients
// linked list.
//-----------------------------------------------------------------------------
void SocketConnection::AddClient(int socket) {
    m_Clients.AddTail(socket);
}
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// This private function will remove client descriptor (socket) to the clients
// linked list.
//-----------------------------------------------------------------------------
void SocketConnection::RemoveClient(int socket) {
    POSITION pos = m_Clients.Find(socket);
	if(pos!=NULL) {
		int socket = m_Clients.GetAt(pos);
		closesocket(socket);
		m_Clients.RemoveAt(pos);
    }

}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// This public function reads the command 
//-----------------------------------------------------------------------------
void SocketConnection::ProcessClientsRequests() {
CString cmd;

    POSITION pos = m_Clients.GetHeadPosition();
    while(pos!=NULL) {
        int socket = m_Clients.GetAt(pos);
        if(CheckSocket(socket)==true) {
			if(ReadCommand(socket,cmd)==false) {
				RemoveClient(socket);
				return;
			} else {
				ProcessCommand(socket,cmd);
			}
			return;

		}
		m_Clients.GetNext(pos);
    }

}
//-----------------------------------------------------------------------------


//------------------------------------------------------------------------------
// This function will check if data available in the selected socket
//------------------------------------------------------------------------------
bool SocketConnection::CheckSocket(int socket) {
    timeval timeout;
    timeout.tv_sec  = 0;
    timeout.tv_usec = 0;
    fd_set sock_set;
    int    nfsd = 0;
    FD_ZERO(&sock_set);
    FD_SET(socket, &sock_set);
    if(select(nfsd,&sock_set,NULL,NULL,&timeout)>0) {
        return true;
    }
    return false;
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
// This private function reads the command using our communications protocol.
//------------------------------------------------------------------------------
bool SocketConnection::ReadCommand(int socket,CString& command) {
unsigned char buf[4];
char* cmd;
int res;
int len;

	WSAAsyncSelect(socket, connection->m_ProxyHwnd, 0, 0);
	unsigned long disable_nonblocking = 0;
	ioctlsocket(socket, FIONBIO, &disable_nonblocking);
    res = recv(socket, (char*)buf, 4, 0);
	if(res!=4) return false;
    len = (unsigned)buf[0] | ((unsigned)buf[1] << 8) | ((unsigned)buf[2] << 16) | ((unsigned)buf[3] << 24);
	if(len>0) {
       cmd = new char [len+1];
	   int readed = 0;
       while((len-readed) > 0){
	       int res = recv(socket, cmd+readed, len-readed, 0);
		   if(res < 0) {
              WSAAsyncSelect(socket, connection->m_ProxyHwnd, CLIENT_SOCKET_MESSAGE, FD_READ | FD_CLOSE);
	          return false;
		   } else {
	          readed+= res;
		   }
	   }
	   cmd[readed]=0;
	   command = cmd;
	   delete cmd;
	} else {
	   command = "";
	}
    WSAAsyncSelect(socket, connection->m_ProxyHwnd, CLIENT_SOCKET_MESSAGE, FD_READ | FD_CLOSE);
    return true;
}
//------------------------------------------------------------------------------



//--------------------------------------------------------------------------------------------
// This fuction will execute the mrg_diff to create diff file.
//---------------------------------------------------------------------------------------------
void SocketConnection::MakeDiff(CString& localfile, CString& modelfile, CString& diffile) {
STARTUPINFO si;
PROCESS_INFORMATION pi;
char* root;
CString cmd;

  root = getenv("CODE_ROVER_HOME");
  if(root==NULL) 
	  root = getenv("PSETHOME");
  if((root==NULL ) || (strlen(root)==0)) {
      cmd="C:/Discover/Bin/mrg_diff";
  } else {
      cmd=root;
	  cmd+="/bin/mrg_diff";
  }
  cmd.Replace("\\","/");
  cmd.Replace("//","/");
  if(cmd.Find(" ")>0) cmd="\""+cmd+"\"";
  cmd+=" ";


  cmd+="\"";
  cmd+=modelfile;
  cmd+="\"";

  cmd+=" ";

  cmd+="\"";
  cmd+=localfile;
  cmd+="\"";

  cmd+=" ";
  cmd+="\"";
  cmd+=diffile;
  cmd+="\"";

  memset(&si,0,sizeof(STARTUPINFO));
  si.hStdInput  = GetStdHandle(STD_INPUT_HANDLE);
  si.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
  si.hStdError  = GetStdHandle(STD_ERROR_HANDLE);
  si.dwFlags    = STARTF_USESTDHANDLES;
  si.cb=sizeof(STARTUPINFO);
  putenv("DIS_NLUNIXSTYLE=1");
  if(::CreateProcess(NULL,cmd.GetBuffer(10),NULL,NULL,FALSE,CREATE_NO_WINDOW,NULL,NULL,&si,&pi)==FALSE) {
	  return;
  }
  WaitForSingleObject(pi.hProcess,INFINITE);
  CloseHandle(pi.hProcess);
  return;
}
//---------------------------------------------------------------------------------------------


//--------------------------------------------------------------------------------------------
// This fuction will execute the mrg_diff to create diff file.
//---------------------------------------------------------------------------------------------
int SocketConnection::MapLines(CString& localfile, CString& basefile, CString& diffile, int localline) {
STARTUPINFO si;
PROCESS_INFORMATION pi;
char* root;
CString cmd;

  root = getenv("CODE_ROVER_HOME");
  if(root==NULL) 
	  root = getenv("PSETHOME");
  if((root==NULL ) || (strlen(root)==0)) {
      cmd="C:/Discover/Bin/mrg_update";
  } else {
      cmd=root;
	  cmd+="/bin/mrg_update";
  }
  cmd.Replace("\\","/");
  cmd.Replace("//","/");
  if(cmd.Find(" ")>0) cmd="\""+cmd+"\"";
  cmd+=" -mapL_to_L ";


  CString num;
  num.Format("%d",localline);

  cmd+=num;
  cmd+=" ";
  cmd+="\"";
  cmd+=basefile;
  cmd+="\"";
  cmd+=" ";
  cmd+="\"";
  cmd+=diffile;
  cmd+="\"";
  cmd+=" ";
  cmd+="\"";
  cmd+=localfile;
  cmd+="\"";

  HANDLE ReadHandle;
  HANDLE WriteHandle;

  SECURITY_ATTRIBUTES sattr;
  memset(&sattr, 0, sizeof(sattr));
  sattr.nLength        = sizeof(sattr);
  sattr.bInheritHandle = TRUE;

  if(!CreatePipe(&ReadHandle, &WriteHandle, &sattr, 0)){
    return -1;
  }


  memset(&si,0,sizeof(STARTUPINFO));
  si.cb=sizeof(STARTUPINFO);
  si.hStdInput  = GetStdHandle(STD_INPUT_HANDLE);
  si.hStdOutput = WriteHandle;
  si.hStdError  = GetStdHandle(STD_ERROR_HANDLE);
  si.dwFlags    = STARTF_USESTDHANDLES;
  putenv("DIS_NLUNIXSTYLE=1");
  if(::CreateProcess(NULL,cmd.GetBuffer(10),NULL,NULL,TRUE,CREATE_NO_WINDOW,NULL,NULL,&si,&pi)==FALSE) {
	  return -1;
  }
  WaitForSingleObject(pi.hProcess,INFINITE);
  char lineStr[200];
  unsigned long readed;

  DWORD lo;
  DWORD hi;
  int count=0;
  do {
      lo=GetFileSize(ReadHandle,&hi);
	  if(hi>0 || lo>0) break;
	  Sleep(50);
	  count++;
  } while(hi==0 && lo==0 && count<5);

  if(hi==0 && lo==0) return localline;

  ReadFile(ReadHandle,lineStr,200,&readed,NULL);
  lineStr[readed-2]=0;
  CloseHandle(pi.hProcess);
  CloseHandle(ReadHandle);
  CloseHandle(WriteHandle);
  return atoi(lineStr);
}
//---------------------------------------------------------------------------------------------




//------------------------------------------------------------------------------------------
// This static function will use mrg_update to map line in the current file into line
// in the original file which was used during modelbuild.
//------------------------------------------------------------------------------------------
int SocketConnection::GetModelLine(CString& localfile, CString& basefile,int localline) {
CString diffile;

    // First, check if the given files exists.
    if(_access(localfile,04)==-1) return  0;
    if(_access(basefile,04)==-1)  return  0;

	// This function will create diff file which we will need 
	// to map lines
    diffile = basefile;
	diffile+=".diff";
	MakeDiff(localfile,basefile,diffile);
	return MapLines(basefile,localfile,diffile,localline);
}
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
// This static function will use mrg_update to map line in the original file into line
// in the modified file
//------------------------------------------------------------------------------------------
int SocketConnection::GetPrivateLine(CString& localfile, CString& basefile,int localline) {
CString diffile;

    // First, check if the given files exists.
    if(_access(localfile,04)==-1) return  0;
    if(_access(basefile,04)==-1)  return  0;

	// This function will create diff file which we will need 
	// to map lines
    diffile = basefile;
	diffile+=".diff";
	MakeDiff(localfile,basefile,diffile);
	return MapLines(localfile,basefile,diffile,localline);
}
//------------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------
void SocketConnection::sendReply(int socket,CString reply) {
CString packet;
unsigned char buf[4];
    
    packet="1 ";
	packet+=reply;
	unsigned val =  packet.GetLength();
    buf[0]  =  val & 255;
    buf[1]  = (val >> 8) & 255;
    buf[2]  = (val >> 16) & 255;
    buf[3]  = (val >> 24) & 255;
	send(socket,(char *)buf,4,0);
	send(socket,packet,val,0);
}
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------
void SocketConnection::sendEvent(int socket,CString event) {
CString packet;
unsigned char buf[4];
    
    packet="0 ";
	packet+=event;
	unsigned val =  packet.GetLength();
    buf[0]  =  val & 255;
    buf[1]  = (val >> 8) & 255;
    buf[2]  = (val >> 16) & 255;
    buf[3]  = (val >> 24) & 255;
	send(socket,(char *)buf,4,0);
	send(socket,packet,val,0);
}
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------
void SocketConnection::sendEvent(CString event) {
    POSITION pos = m_Clients.GetHeadPosition();
	if(pos==NULL) {
		AfxMessageBox("Unable to detect connected Information Model.\nPlease run the DIScover browser first.",MB_ICONSTOP|MB_OK);
    } else {
        while(pos!=NULL) {
            int socket = m_Clients.GetAt(pos);
		    sendEvent(socket,event);
		    m_Clients.GetNext(pos);
		}
	}
}
//------------------------------------------------------------------------------------------

void stripQuotes(CString& str) {
   str.TrimLeft('"');
   str.TrimRight('"');
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void SocketConnection::ProcessCommand(int socket,CString cmd) {
CString args[10];
CString token;


	// Tokenizing string query and placing all tokens into the 
	// vals matrix
	int length = cmd.GetLength();
	token = "";
	int tokens = 0;
	for(int i=0;i<length;i++) {
		if(cmd[i]=='\t') {
			args[tokens++]=token;
			token = "";
			if(tokens==10) break;
			continue;
		}
		token+=cmd[i];
	}
	if(token.GetLength()>0) args[tokens++]=token;



	// Converts given model file line into modified file line
	if(args[0]=="model_line_to_private_line") {
	   stripQuotes(args[1]);
	   stripQuotes(args[2]);
	   int line = GetPrivateLine(args[1],args[2],atoi(args[3]));
	   CString lineStr;
	   lineStr.Format("%d",line);
       sendReply(socket,lineStr);
	   return;
	}

	// Converts given model file line into modified file line
	if(args[0]=="private_line_to_model_line") {
	   stripQuotes(args[1]);
	   stripQuotes(args[2]);
	   int line = GetModelLine(args[1],args[2],atoi(args[3]));
	   CString lineStr;
	   lineStr.Format("%d",line);
       sendReply(socket,lineStr);
	   return;
	}



	// Open file using given line number
	if(args[0]=="open") {
		stripQuotes(args[1]);
		if(m_pCommandProcessor->OpenFileOnLine(args[1],atoi(args[2]))==true) {
		   sendReply(socket,"done");
		} else {
		   CString msg;
		   msg.Format("File does not exist. Check server source root. (%s)",args[1]);
		   sendReply(socket,msg);
		}
		return;
	}

	// Selects the token on the given line
	if(args[0]=="mark") {
		stripQuotes(args[1]);
		m_pCommandProcessor->MakeSelection(args[1],atoi(args[2]),atoi(args[3]),atoi(args[4]));
		sendReply(socket,CString("done"));
		return;
	}

	// Open file using given offset (not implemented under DeveloperStudio)
	if(args[0]=="openoffset") {
		sendReply(socket,CString("not implemented"));
		return;
	}

	// Selects the token using the start and end offsets (not implemented under DeveloperStudio)
	if(args[0]=="markoffset") {
		sendReply(socket,CString("not implemented"));
		return;
	}

	// Shows string from the client in the editor status window
	if(args[0]=="info") {
		m_pCommandProcessor->ShowMessage(args[1]);
        sendReply(socket,"done");
		return;
	}

	// Saves current editor selection in the buffer. We will query selection parameters
	// later and they must be unchanged.
	if(args[0]=="freeze") {
	    m_pCommandProcessor->FreezeSelection();
		sendReply(socket,CString("done"));
		return;
	}

	// Returns the name of the file on which we freeze our selection.
	if(args[0]=="file") {
		CString file = m_pCommandProcessor->GetSelectedFile();
        sendReply(socket,file);
		return;
	}

	// Returns the selected line. Must not be implemented under Emacs
	if(args[0]=="line") {
	   int line = m_pCommandProcessor->GetSelectedLine();
	   CString lineStr;
	   lineStr.Format("%d",line);
       sendReply(socket,lineStr);
	   return;
	}

	// Returns the offset to the selection. Do not exist under DeveloperStudio
	if(args[0]=="offset") {
       sendReply(socket,CString("not implemented"));
	   return;
	}

	// Returns the offset in the selected line. Must not be implemented under Emacs
	if(args[0]=="lineoffset") {
	   int column = m_pCommandProcessor->GetSelectedColumn();
	   CString columnStr;
	   columnStr.Format("%d",column);
       sendReply(socket,columnStr);
	   return;
	}

	// Returns the selection length
	if(args[0]=="length") {
	   int len = m_pCommandProcessor->GetSelectedLength();
	   CString lenStr;
	   lenStr.Format("%d",len);
       sendReply(socket,lenStr);
	   return;
	}

	// register a new client
	if(args[0]=="register") {
       sendReply(socket,CString("done"));
	   return;
	}

	// uregister an old client
	if(args[0]=="unregister") {
       sendReply(socket,CString("done"));
	   return;
	}

    // We do not know this command 
    sendReply(socket,CString("unknown command"));

	return;

}
//------------------------------------------------------------------------------

void SocketConnection::RemoveAllClients() {

    POSITION pos = m_Clients.GetHeadPosition();
	while(pos) {
		int socket = m_Clients.GetAt(pos);
		closesocket(socket);
		m_Clients.GetNext(pos);
    }
    while(m_Clients.GetCount()>0) m_Clients.RemoveHead();

}
