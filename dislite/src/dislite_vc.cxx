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
#include <afx.h>
#include <afxsock.h>
#include <stdio.h>
#include <iostream.h>
#include <io.h>
#include <fcntl.h>
#include "nameServCalls.h"

SOCKADDR_IN IntegratorAddr;
int nameserver;
int integrator;
CString event;
char command[1024];
BOOL OnEvent   = FALSE;
BOOL OnCommand = FALSE;
BOOL OnExit    = FALSE;
BOOL IntegratorCreator=FALSE;

extern int vcopen, restart;
FILE *read_pipe, *write_pipe;
HANDLE vc_event, wish_event, hWish;
HANDLE hThreads[2];

HANDLE ReadPipeReadHandle;
HANDLE WritePipeWriteHandle;


static int replyHead = 0;
static CString replyList[10];
static CRITICAL_SECTION threadRead;

//----------------------------------------------------------------------------------------
// This private function will receve integrator IP address and port.
// It will return FALSE if no integrator detected.
// It will receve TRUE and it will fill m_WhereIntegrator structure with port and IP 
// address of the integrator if everything is OK.
// RUN THIS FUNCTION DIRECTLY AFTER QueryIntegrationService
//----------------------------------------------------------------------------------------
int ReceveIntegrationService() {
char serviceName[1024];
  
    // Getting current host name  
    char hostName[1024];
    gethostname(hostName,1024);

	// Getting current user name
	char        userName[1024];
	char*		userNameEnv;
	userNameEnv = getenv("USER");
	if(userNameEnv == NULL) {
		DWORD nUserNameSize = 1024;
		GetUserName(userName,&nUserNameSize);
	} else {
		strcpy(userName,userNameEnv);
	}

	strcpy(serviceName,hostName);
	strcat(serviceName,":");
	strcat(serviceName,userName);
	strcat(serviceName,":");
	strcat(serviceName,"Editor:Microsoft Visual Studio");

	int port;
	int address;
    int res = NameServerGetService(serviceName, port, address); 
    memset(&IntegratorAddr,0,sizeof(SOCKADDR_IN));
    IntegratorAddr.sin_family=AF_INET;
    IntegratorAddr.sin_port=htons(port);
    static struct hostent *ent = gethostbyname(hostName);
    IntegratorAddr.sin_addr=*(in_addr *)*ent->h_addr_list;
    return res;
}
//----------------------------------------------------------------------------------------

static void editorConnect () {
int i;
char str[1024];
STARTUPINFO si;
PROCESS_INFORMATION pi;

    strcpy(str,"msdev");
    memset(&si,0,sizeof(STARTUPINFO));
    si.cb=sizeof(STARTUPINFO);
    if(::CreateProcess(NULL,str,NULL,NULL,FALSE,CREATE_NEW_CONSOLE,NULL,NULL,&si,&pi)==FALSE) {
        printf("Can't start Microsoft Visual Studio");
    }
    for(i=0;i<10;i++) {
      if(ReceveIntegrationService()>0) {
	      break;
      } else {
		  Sleep(1000);
	  }
    }
    if(i==20) {
      printf("Can't find Miscosoft Developer Studio.");
	  return;
    }

    integrator = socket (PF_INET, SOCK_STREAM, 0);

    // Trying to connect to the integrator TCP address/port filled by 
    // ReceveIntegrationService
    if(connect(integrator,(SOCKADDR *)&IntegratorAddr,sizeof(IntegratorAddr)) < 0) {
       printf("Can't connect to the Discover Integrator\n");
       closesocket(integrator);
	}
}


static bool SendCommand(int socket,CString &command) {
	int len = command.GetLength();
 XXX: byte ordering changed!
            // XXX: fixme
	unsigned char b[4];
	b[0] = len & 0xFF;
	b[1] = (len>>8)  & 0xFF;
	b[2] = (len>>16) & 0xFF;
	b[3] = (len>>24) & 0xFF;
	if(send(socket,(char*)b,4,0)!=4) return false;
        // /XXX>
	if(send(socket,command.GetBuffer(10),len,0)!=len) return false;
	replyHead = 0;
	return true;

}


static CString ReadString(int socket) {
CString str;

    unsigned char b[4];
    int readed=0;

	while(readed<4) {
       int res = recv(socket,(char *)b+readed,4-readed,0);
	   if(res<=0) {
		    Sleep(1000);
	        return str;
       }
	   readed+=res;
	}

 XXX: byte ordering changed!
    int len = b[0]+(b[1]>>8)+(b[2]>>16)+(b[3]>>24);
    readed=0;
    char* buf = new char [len+1];
	while(readed<len) {
	   int res = recv(socket,buf+readed,len-readed,0);
	   if(res<=0) {
  	      Sleep(1000);
		  delete buf;
		  return str;
	   }
	   readed+=res;
	}
    buf[readed]=0;
	str = buf;
	delete buf;
	return str;
}

static CString GetReply() {
	int count = 0;
	while(replyHead==0 && count++<50) Sleep(100);
	EnterCriticalSection(&threadRead);
	if(replyHead==0) {
        LeaveCriticalSection(&threadRead);
		return "";
	}
	replyHead--;
	CString res = replyList[replyHead];
    LeaveCriticalSection(&threadRead);
	return res;

}


static void integratorFreezeSelection() {
	if(SendCommand(integrator,CString("freeze"))) {
		editorConnect();
		if(SendCommand(integrator,CString("freeze"))) return;
	}
    GetReply();
}

static CString integratorGetFileName() {
	if(SendCommand(integrator,CString("file"))==false) {
		editorConnect();
		if(SendCommand(integrator,CString("file"))==false) return "";
	}
    return GetReply();
}

static int integratorGetLineNumber() {
	if(SendCommand(integrator,CString("line"))==false) {
		editorConnect();
		if(SendCommand(integrator,CString("line"))==false) return -1;
	}
    CString lineStr = GetReply();
    return atoi(lineStr);
}

static int integratorGetLineOffset() {
	if(SendCommand(integrator,CString("lineoffset"))==false) {
		editorConnect();
		if(SendCommand(integrator,CString("lineoffset"))==false) return -1;
	}
    CString colStr=GetReply();
    return atoi(colStr);
}

static void integratorOpenFile(CString filename,int line) {
CString query;
   query.Format("open\t%s\t%d",filename,line);
   if(SendCommand(integrator,query)==false) {
	   editorConnect();
	   if(SendCommand(integrator,query)==false) return;
   }
   GetReply();
   return;
}

static void integratorViewFile(CString filename,int line) {
CString query;
   query.Format("open\t%s\t%d",filename,line);
   if(SendCommand(integrator,query)==false) {
	   editorConnect();
	   if(SendCommand(integrator,query)==false) return;
   }
   GetReply();
   return;
}

static void integratorInfo(CString message) {
CString query;
   query.Format("info\t%s",message);
   if(SendCommand(integrator,query)==false) {
	   editorConnect();
   if(SendCommand(integrator,query)==false) return;
   }
   GetReply();
   return;
}



//----------------------------------------------------------------------------------------
// This is spy which will put data into stdout stream every time the data comes from 
// the socket
//----------------------------------------------------------------------------------------

DWORD WINAPI SocketTestThread(LPVOID param) {
CString reply;
  WaitForSingleObject(vc_event , INFINITE );
  ResetEvent( vc_event );
  CString str = ReadString(integrator);
  if(str.Find("0 ")==0) {
       event = str.Right(str.GetLength()-2);
  }
  if(str.Find("1 ")==0) {
	   event = "";
       if(str.Find("1 ")==0) {
           reply = str.Right(str.GetLength()-2);
	   	   EnterCriticalSection(&threadRead);
		   if(replyHead<9)
   	          replyList[replyHead++]=reply;
  	   	   LeaveCriticalSection(&threadRead);
	   } else { 
	       reply = "";
	   }
  }
  OnEvent=TRUE;
  ExitThread(0);
  return 0;
}
//-----------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------
// This is spy which will put data into stdout stream every time the data comes from 
// the socket
//----------------------------------------------------------------------------------------

DWORD WINAPI StreamTestThread(LPVOID param) {
  WaitForSingleObject(wish_event , INFINITE );
  ResetEvent( wish_event );
  command[0]=0;
  fgets( command, 1024, read_pipe );
  OnCommand=TRUE;
  ExitThread(0);
  return 0;
}


//-----------------------------------------------------------------------------------------
void EventProcessor(void) {
  char str[1024];
  CString fname;
  CString token;
  // Processing ACTIVATE BROWSER event. No selection passed. Browser must become 
  // top-most window.
  if(event=="open") {
	  sprintf(str,"activate 0 0\n");
      fprintf(write_pipe, "%s",str);
      fflush(write_pipe);
	  return;
  }

  // Processing old ctrl+click event -  we have to get selection from the editor. 
  if(event=="click") {
      integratorFreezeSelection();
      CString file =integratorGetFileName();
	  file.Replace('\\','/');
      int line     = integratorGetLineNumber();
	  int column   = integratorGetLineOffset();
      sprintf(str,"inst_def {%s} %d %d %d\n",file,0,line,column);
      fprintf(write_pipe, "%s",str);
      fflush(write_pipe);
      return;
  }

  // Processing old QUERY event -  we have to get selection from the editor. 
  if(event=="query") {
      integratorFreezeSelection();
      CString file =integratorGetFileName();
	  file.Replace('\\','/');
      int line     = integratorGetLineNumber();
	  int column   = integratorGetLineOffset();
      sprintf(str,"query {%s} %d %d %d\n",file,0,line,column);
      fprintf(write_pipe, "%s",str);
      fflush(write_pipe);
      return;
  }

  // Processing old OPEN DEFINITION event -  we have to get selection from the editor. 
  if(event=="definition") {
      integratorFreezeSelection();
      CString file =integratorGetFileName();
	  file.Replace('\\','/');
      int line     = integratorGetLineNumber();
	  int column   = integratorGetLineOffset();
      sprintf(str,"open_def {%s} %d %d %d\n",file,0,line,column);
      fprintf(write_pipe, "%s",str);
      fflush(write_pipe);
      return;
  }

  // Processing new ctrl+click event -  selection arrived as a part of the event 
  // (tab-separated)
  if(event.Find("click")==0) {
      CString token;
	  CString file;
	  int line=0;
	  int column=0;
	  int length=0;
	  int fieldIndex = 0;
	  for(int i = 0; i < event.GetLength(); i++) {
	      if(event[i]=='\t') {
		      switch(fieldIndex) {
			      case 0 : break;
				  case 1 : file   = token; break;
				  case 2 : line   = atoi(token); break;
				  case 3 : column = atoi(token); break;
				  case 4 : length = atoi(token); break;
			  }
              token="";
			  fieldIndex++;
		  } else 
		      token+=event[i];
	  }
      if(token.GetLength()>0) {
	 	 switch(fieldIndex) {
		     case 0 : break;
		     case 1 : file   = token; break;
		     case 2 : line   = atoi(token); break;
		     case 3 : column = atoi(token); break;
		     case 4 : length = atoi(token); break;
		 }
	  }
	  file.Replace('\\','/');
      sprintf(str,"inst_def {%s} %d %d %d\n",file,0,line,column);
      fprintf(write_pipe, "%s",str);
      fflush(write_pipe);
      return;
  }

  // Processing new QUERY event -  selection arrived as a part of the event 
  // (tab-separated)
  if(event.Find("query")==0) {
      CString token;
	  CString file;
	  int line=0;
	  int column=0;
	  int length=0;
	  int fieldIndex = 0;
	  for(int i = 0; i < event.GetLength(); i++) {
	      if(event[i]=='\t') {
		      switch(fieldIndex) {
			      case 0 : break;
				  case 1 : file   = token; break;
				  case 2 : line   = atoi(token); break;
				  case 3 : column = atoi(token); break;
				  case 4 : length = atoi(token); break;
			  }
              token="";
			  fieldIndex++;
		  } else 
		      token+=event[i];
	  }
      if(token.GetLength()>0) {
	 	 switch(fieldIndex) {
		     case 0 : break;
		     case 1 : file   = token; break;
		     case 2 : line   = atoi(token); break;
		     case 3 : column = atoi(token); break;
		     case 4 : length = atoi(token); break;
		 }
	  }
	  file.Replace('\\','/');
      sprintf(str,"query {%s} %d %d %d\n",file,0,line,column);
      fprintf(write_pipe, "%s",str);
      fflush(write_pipe);
      return;
  }

  // Processing new OPEN DEFINITION event -  selection arrived as a part of the event 
  // (tab-separated)
  if(event.Find("definition")==0) {
      CString token;
	  CString file;
	  int line=0;
	  int column=0;
	  int length=0;
	  int fieldIndex = 0;
	  for(int i = 0; i < event.GetLength(); i++) {
	      if(event[i]=='\t') {
		      switch(fieldIndex) {
			      case 0 : break;
				  case 1 : file   = token; break;
				  case 2 : line   = atoi(token); break;
				  case 3 : column = atoi(token); break;
				  case 4 : length = atoi(token); break;
			  }
              token="";
			  fieldIndex++;
		  } else 
		      token+=event[i];
	  }
      if(token.GetLength()>0) {
	 	 switch(fieldIndex) {
		     case 0 : break;
		     case 1 : file   = token; break;
		     case 2 : line   = atoi(token); break;
		     case 3 : column = atoi(token); break;
		     case 4 : length = atoi(token); break;
		 }
	  }
	  file.Replace('\\','/');
      sprintf(str,"open_def {%s} %d %d %d\n",file,0,line,column);
      fprintf(write_pipe, "%s",str);
      fflush(write_pipe);
      return;
  }
}
//---------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------
void CommandProcessor(void) {
  char filename[_MAX_PATH];
  char cmd[25], str[1024];
  int line;
  DWORD code = 0;
  char *p = command;
  while(isspace(*p) && *p) p++;
  int i=0;
  do {
    cmd[i] = *p;
	if(*p==0) break;
    p++; i++;
  } while(!isspace(*p));
  cmd[i] = '\0';

  if(!strncmp(cmd,"open",4)) {
      for(;isspace(*p);p++);
      if (*p=='{' || *p=='\"') {
           char ch;	
           if(*p=='{') {
	            ch = '}';
		   } else {
	            ch = '\"';
		   }
           char *begin = ++p;
           p = strchr(p, ch);
           if(!p) return;
           while(*(p-1)=='\\') {
	           p++;
	           p = strchr(p, ch);
	           if(!p) return;
		   }
           i=0;
           while(begin<p) {
	           filename[i] = *begin;
	           begin++; i++;
		   }
           filename[i] = '\0';
           sscanf(p+1, "%d", &line);
	  } else {	
           sscanf(p, "%s %d", filename, &line);  
	  }
      if(!strcmp(cmd,"open")) {
	     integratorOpenFile(filename, line);
      } else {
	     integratorViewFile(filename, line);
      }
  } else if(strcmp(cmd,"info")==0) {
           int i=0, j=0;
           char *p = command+6;
           while (*p!='}' && *p!='\0') {
               if(*p=='[' || *p==']') {
	                *(str+i+j) = '\\';
	                j++;
			   }
               *(str+i+j)= *p;
               p++;
               i++;
		   }
           *(str+i+j) = '\0';
           integratorInfo(str);
	   } else if(strcmp(cmd,"Dislite_Exit")==0 || strlen(cmd)==0) {
                  TerminateProcess(hWish, 0); 
                  CloseHandle(hWish);
                  CloseHandle(ReadPipeReadHandle);			
                  CloseHandle(WritePipeWriteHandle);
                  OnExit=TRUE;
	   } else if(strcmp(cmd,"DeveloperXPress_Restart")==0) {
	     restart = 1;
	     char dis_service[128];
		 dis_service[0]=0;
	     sscanf(command, "%s\t%s", cmd, dis_service);
	     sprintf(str, "DIS_SERVICE=%s", dis_service);
	     _putenv(str);
	     TerminateProcess(hWish, 0); 
	     CloseHandle(hWish);
	     CloseHandle(ReadPipeReadHandle);			
	     CloseHandle(WritePipeWriteHandle);
	     OnExit=TRUE;
	   } 
}
//-----------------------------------------------------------------------------------------

//
// two-way pipe
int vpopen2way(char* cmd, FILE **read_pipe_ptr, FILE **write_pipe_ptr)
{
	HANDLE StdOut = GetStdHandle(STD_OUTPUT_HANDLE);
	HANDLE StdIn  = GetStdHandle(STD_INPUT_HANDLE);
	HANDLE StdErr = GetStdHandle(STD_ERROR_HANDLE);

	STARTUPINFO sinfo;
	memset(&sinfo, 0, sizeof(sinfo));
	sinfo.cb         = sizeof(sinfo);
	sinfo.hStdInput  = StdIn;
	sinfo.hStdOutput = StdOut;
	sinfo.hStdError  = StdErr;
	sinfo.dwFlags    = STARTF_USESTDHANDLES;

	HANDLE ReadPipeWriteHandle;
	HANDLE WritePipeReadHandle;

	*read_pipe_ptr = *write_pipe_ptr = NULL;
	SECURITY_ATTRIBUTES sattr;
	memset(&sattr, 0, sizeof(sattr));
	sattr.nLength        = sizeof(sattr);
	sattr.bInheritHandle = TRUE;

	if(!CreatePipe(&ReadPipeReadHandle, &ReadPipeWriteHandle, &sattr, 0)){
		cerr << "Cannot create pipe" << endl;
		return -1;
	}
	if(!CreatePipe(&WritePipeReadHandle, &WritePipeWriteHandle, &sattr, 0)){
		cerr << "Cannot create pipe" << endl;
		return -1;
	}
	HANDLE hDup;
	if( DuplicateHandle(GetCurrentProcess(), ReadPipeReadHandle, GetCurrentProcess(), 
		&hDup, 0, FALSE, DUPLICATE_SAME_ACCESS) )
	{
		CloseHandle(ReadPipeReadHandle);
		ReadPipeReadHandle = hDup;
	}
	int read_pipe_file_handle = _open_osfhandle((long)ReadPipeReadHandle, _O_RDONLY);
	if(read_pipe_file_handle < 0){
		cerr << "Cannot convert pipe handle" << endl;
		return -1;
	}
	*read_pipe_ptr = _fdopen(read_pipe_file_handle, "r");
	if(*read_pipe_ptr == NULL){
		cerr << "Cannot open output file" << endl;
		return -1;
	}
	sinfo.hStdOutput = ReadPipeWriteHandle;
	if( DuplicateHandle(GetCurrentProcess(), WritePipeWriteHandle, GetCurrentProcess(), 
		&hDup, 0, FALSE, DUPLICATE_SAME_ACCESS))
	{
		CloseHandle(WritePipeWriteHandle);
		WritePipeWriteHandle = hDup;
	}
	int write_pipe_file_handle = _open_osfhandle((long)WritePipeWriteHandle, _O_WRONLY);
	if(write_pipe_file_handle < 0){
		cerr << "Cannot convert pipe handle" << endl;
		return -1;
	}
	*write_pipe_ptr = _fdopen(write_pipe_file_handle, "w");	
	if(*write_pipe_ptr == NULL){
		cerr << "Cannot open output file" << endl;
		return -1;
	}

	sinfo.hStdInput = WritePipeReadHandle;
	PROCESS_INFORMATION pinfo;

	if(!CreateProcess(NULL, cmd, NULL, NULL, TRUE,
		CREATE_DEFAULT_ERROR_MODE | /*DETACHED_PROCESS  |*/ NORMAL_PRIORITY_CLASS, NULL,
		NULL, &sinfo, &pinfo)){
		LPVOID lpMsgBuf;
		if(FormatMessage( 
			FORMAT_MESSAGE_ALLOCATE_BUFFER | 
			FORMAT_MESSAGE_FROM_SYSTEM | 
			FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			GetLastError(),
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
			(LPTSTR) &lpMsgBuf,
			0,
			NULL )) {
			// Display the string.
			MessageBox( NULL, (LPCTSTR)lpMsgBuf, "Error", MB_OK | MB_ICONINFORMATION );

			// Free the buffer.
			LocalFree( lpMsgBuf );
		}
		cerr << "Cannot create process : " << cmd << endl;
		CloseHandle(ReadPipeReadHandle);
		CloseHandle(ReadPipeWriteHandle);
		CloseHandle(WritePipeReadHandle);
		CloseHandle(WritePipeWriteHandle);
		*write_pipe_ptr = *read_pipe_ptr = NULL;
		return -1;
	}
	hWish = pinfo.hProcess;
	CloseHandle(pinfo.hThread);
	CloseHandle(WritePipeReadHandle);
	CloseHandle(ReadPipeWriteHandle);
	return 0;
}


int start_vc() {
  register i;
  char str[1024];
  WORD    wVersionRequested;  
  WSADATA wsaData; 
  int     err; 

  // Socket initialization
  wVersionRequested = MAKEWORD(1, 1); 
  err = WSAStartup(wVersionRequested, &wsaData); 
  if (err != 0) {
    printf("No sockets available\n");
    exit(1);
  }

  InitializeCriticalSection(&threadRead);
  char *psethome = getenv("PSETHOME");

  // Trying to connect to the "Discover:Integrator:<local host>"
  STARTUPINFO si;
  PROCESS_INFORMATION pi;
  int ret = ReceveIntegrationService();
  if(ret==-1) { // No name server detected
    memset(&si,0,sizeof(STARTUPINFO));
    si.cb=sizeof(STARTUPINFO);
    if(::CreateProcess(NULL,"nameserv",NULL,NULL,FALSE,DETACHED_PROCESS,NULL,NULL,&si,&pi)==FALSE) {
      printf("Can't start Discover name server.");
      exit(1);
    }    
  } 
  for(i=0;i<20;i++) {
    ret = ReceveIntegrationService();
    if(ret!=-1) break;
  }
  
  if(i==20 && ret==-1) {
    printf("Can't connect to the Discover Name Server\n");
    exit(1);
  } 
   
  if(ret==1) {
     // Opening integration socket
     integrator = socket (PF_INET, SOCK_STREAM, 0);

     // Trying to connect to the integrator TCP address/port filled by 
     // ReceveIntegrationService
     if(connect(integrator,(SOCKADDR *)&IntegratorAddr,sizeof(IntegratorAddr)) < 0) {
       printf("Can't connect to the Discover Integrator\n");
       closesocket(integrator);
	 }
  } 
	
  vc_event = CreateEvent( NULL, TRUE/*manual reset*/, FALSE/*initial state: non-signaled*/,
			  NULL );
  wish_event = CreateEvent( NULL, TRUE/*manual reset*/, FALSE/*initial state: non-signaled*/,
                            NULL );
  
  str[0]=0; // empty the string

  int x;
  DWORD dwThreadId;
  char *home = getenv("HOME");
  char *service = getenv("DIS_SERVICE");
  char *dis_install = getenv("DIS_INSTALL");
  char *wish = getenv("DIS_WISH");

  int bHasSpaces = strchr(wish,' ')!=NULL;  // check if path to wish has spaces

  if(bHasSpaces) strcat(str,"\"");
  strcat(str,wish);
  if(bHasSpaces) strcat(str,"\"");

  strcat(str, " -f ");
  if(dis_install) {
  	bHasSpaces = strchr(dis_install,' ')!=NULL;  // check if path to tcl scripts has spaces
	if(bHasSpaces) strcat(str,"\"");
    strcat(str, dis_install);		
  } else {
  	bHasSpaces = strchr(psethome,' ')!=NULL;  // check if path to tcl scripts has spaces
	if(bHasSpaces) strcat(str,"\"");
    strcat(str, psethome);
    strcat(str, "\\lib");
  }
  strcat(str, "\\dis_client.tcl");
  if(bHasSpaces) strcat(str,"\"");
  strcat(str, " "); // add space between parameters

  if(service) strcat(str, service);
  if(vpopen2way(str, &read_pipe, &write_pipe)==-1) {
    printf("Can't start wish\n");
    exit(1);
  }
 	
  hThreads[0]=CreateThread(NULL,0,SocketTestThread,(LPVOID)&x,0,&dwThreadId);
  hThreads[1]=CreateThread(NULL,0,StreamTestThread,(LPVOID)&x,0,&dwThreadId);
  
  SetEvent(wish_event);
  SetEvent(vc_event);
  
  while(OnExit==FALSE) {
    WaitForMultipleObjects(2,hThreads,FALSE,INFINITE);
    if(OnEvent==TRUE) {
      if(event.GetLength()>0) EventProcessor();
      OnEvent=FALSE;
      CloseHandle(hThreads[0]);
      hThreads[0]=CreateThread(NULL,0,SocketTestThread,(LPVOID)&x,0,&dwThreadId);
      SetEvent(vc_event);
    }
    if(OnCommand==TRUE) {
      CommandProcessor();
      CloseHandle(hThreads[1]);
      if(OnExit==FALSE) {
         hThreads[1]=CreateThread(NULL,0,StreamTestThread,(LPVOID)&x,0,&dwThreadId);
	     OnCommand=FALSE;
	     SetEvent(wish_event);
      }
    }
  }
  
  if (OnCommand==TRUE) {
    TerminateThread(hThreads[0], 0);
    WaitForSingleObject(hThreads[0], INFINITE);
    CloseHandle(hThreads[0]);
  }
  CloseHandle(wish_event);
  CloseHandle(vc_event);

  
  closesocket(integrator);

  OnEvent   = FALSE;
  OnCommand = FALSE;
  OnExit    = FALSE;
  IntegratorCreator=FALSE;
  DeleteCriticalSection(&threadRead);
  return 0;
}









