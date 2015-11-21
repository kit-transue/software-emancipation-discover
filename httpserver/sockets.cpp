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
#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#include <winsock.h>
#include <io.h>
#else
   #ifndef hp10
      #include <sysent.h>
   #endif
   #include <ctype.h>
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
#include "servers.h"

static int do_shutdown = 0;
static int do_debug    = 0;
static int httpPort    = 80;

#define DBG(stmt) if(do_debug) { stmt; }



void ParseArguments(int argc, char **argv) {
    int i = 1;
    while(i < argc) {
        if(strcmp(argv[i], "-stop") == 0) {
	        do_shutdown = 1;
		} else  {
			if(strcmp(argv[i], "-debug") == 0) {
	                do_debug = 1;
			} else {
				if(strcmp(argv[i], "-port") == 0) {
					i++;
					if(i<argc) {
						int port = atoi(argv[i]);
						if(port>0) httpPort = port;
					}
				}
			}
		}
	    i++;
    }
}


//--------------------------------------------------------------------------------------------
// This function will create HTTP listen socket (usually at port 80) to process all HTTP
// requests
//--------------------------------------------------------------------------------------------
int CreateListener(int port) {
    int sock;
    struct sockaddr_in name;

    /* Create the socket. */
    sock = socket (PF_INET, SOCK_STREAM, 0);
    if (sock < 0) return -1;
    unsigned int set_option = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (const char *)&set_option, sizeof(set_option));

    /* Give the socket a name. */
    name.sin_family      = AF_INET;
    name.sin_port        = htons(port);
    name.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind (sock, (struct sockaddr *) &name, sizeof (name)) < 0)
	    return -1;
    if(listen(sock, 5) < 0)
	   return -1;
	return sock;

}
//--------------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------------------------
// This function will check if data available in the selected socket
//------------------------------------------------------------------------------------------------------
int CheckSocket(int socket) {
timeval timeout;
timeout.tv_sec  = 1;
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
//------------------------------------------------------------------------------------------------------


int read_line(int sock, char* buf, int maxLen) {
char ch,prev=0;
int readed = 0;

   do {
      int res = recv(sock, &ch, 1, 0);
	  if(ch=='\n' && prev == '\r') break;
      if(res<1) {
		  buf[readed]=0;
		  return 0;
	  }
	  if(ch!=0) buf[readed++] = ch;
	  if(readed>=maxLen) break;
	  prev = ch;
   } while(1);
   buf[readed-1]=0;
   return 1;

}


static char* GetKey(char* str) {
static char key[1024];
int j = 0;

    for(int i = 0; i<1023; i++) {
        if(str[i]==0)   break;
		if(str[i]==':') break;
		key[j++]=str[i];
	}
	while(j>0 && key[j-1]==' ') j--;
	key[j]=0;
	return key;
}

static char* GetValue(char* str) {
static char value[1024];
int j=0;
int in_value = 0;

    for(int i = 0; i<1023; i++) {
		if(str[i]==0)   {
	        while(j>0 && value[j-1]==' ') j--;
			value[j]=0;
			return value;
		}
		if(str[i]==':') {
			in_value = 1;
			i++;
	        while(i<1023 && str[i]==' ' && str[i]!=0) i++;
		}
        if(in_value == 1)   value[j++]=str[i];
	}
	while(j>0 && value[j-1]==' ') j--;
	value[j]=0;
	return value;
}
   

//-----------------------------------------------------------------------------------
// This static function will read from the client HTTP command with destination
// and http version
//-----------------------------------------------------------------------------------
static int read_command(int clientConnection,
						char* method, char* destination, char* version) {
char buf[1024];
int i,j;

   if(read_line(clientConnection,buf,1024)==0) {
	   return 0;
   }

   // Parsing request to extract request type and model name
   int token = 0;
   j=0;
   for(i=0;i<strlen(buf);i++) {
	   if(buf[i]==' ') {
	  	    switch(token) {
               // Method name 
	           case 0 : method[j]=0;
			            break;   
               // Destination
	           case 1 : destination[j]=0;
			            break;
               // Version
	           case 2 : version[j]=0;
			            break;
			}
		    token++;
		    j=0;
		    continue;
	   }
	   switch(token) {
          // Method name 
	      case 0 : method[j++]=buf[i];
			       break;   
          // Destination
	      case 1 : destination[j++]=buf[i];
			       break;
          // Version
	      case 2 : version[j++]=buf[i];
			       break;
	   }
   }
  
   switch(token) {
       // Method name 
       case 0 : method[j]=0;
                break;   
       // Destination
       case 1 : destination[j]=0;
                break;
       // Version
	   case 2 : version[j]=0;
	            break;
   }
   return 1;
}
//-----------------------------------------------------------------------------------

static int cmp(char* model, char* key) {
int modelLength;
int keyLength;
int i;

    modelLength=strlen(model);
	keyLength = strlen(key);
	if(modelLength!=keyLength) return 0;
	for(i=0;i<modelLength;i++) {
		if(toupper(model[i])!=toupper(key[i])) return 0;
	}
	return 1;
}



//-----------------------------------------------------------------------------------
// This static function will read HTTP request headers and will return data block 
// size or 0 if not available.
//-----------------------------------------------------------------------------------
static int read_headers(int clientConnection,int &discoverRequest) {
int data_length = 0;
char buf[1024];
   
   discoverRequest = 0;
   do {
	   if(read_line(clientConnection,buf,1024)==0) break;
	   if(strlen(buf)==0) break;
	   if(cmp(GetKey(buf),"USER-AGENT")) {
		   char* value = GetValue(buf);
		   if(cmp(value,"STOPDISCOVER")) 
			   discoverRequest = 2;
		   
	   }
	   if(cmp(GetKey(buf),"CONTENT-LENGTH")) {
		   char* value = GetValue(buf);
		   data_length = atoi(value);
           DBG(printf("Request length : %d (%s)\n",data_length,value));
	   }
	   if(cmp(GetKey(buf),"CONTENT-TYPE")) {
		   char* value = GetValue(buf);
		   if(cmp(value,"APPLICATION/DISCOVER")) 
			   discoverRequest = 1;
		   
	   }
   } while(1);
   return data_length;
}
//------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------
static char* GetModelName(char* destination) {
int i,j;
static char modelname[1024];
    
    // Trying to get server source root - it is space-separated from the doc URL
    for(i=strlen(destination)-1;i>=0;i--)  if(destination[i]=='@') break;
	if(i<0) i=0;
	if(i==0) { // If no server source root is in the query
	     for(i=strlen(destination)-1;i>=0;i--) if(destination[i]=='/') break;
	} else {
		 i--;
	     for(;i>=0;i--) if(destination[i]=='/') break;
	}
	i++;
	j=0;
	for(;i<strlen(destination);i++) {
		if(destination[i]=='@') break;
		modelname[j++]=destination[i];
	}
	modelname[j]=0;
	return modelname;

}
//------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------
static char* GetServerSrcRoot(char* destination) {
int i,j;
static char root[1024];
    
    // Trying to get server source root - it is space-separated from the doc URL
    for(i=strlen(destination)-1;i>=0;i--)  if(destination[i]=='@') break;
	if(i==0) { // If no server source root is in the query
		 return NULL;
	} else {
		 i++;
		 j=0;
	     for(;i<strlen(destination);i++) root[j++]=destination[i];
	}
	root[j]=0;
	return root;

}
//------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------------------------
// This function will read http request from the client, will unwrap access command
// from the request, will evaluate this command on the model server, will wrap the
// evaluation results in the HTTP reply and will send this reply to the client.
//------------------------------------------------------------------------------------------------------
char* WrapReply(char* reply, int len);
void ProcessCommandSendReply(int clientConnection) {
char method[1024];
char destination[1024];
char version[1024];
int discoverRequest;
char* httpReply;
char* error_not_found = "HTTP/1.1 404 Not Found\r\n\r\n";

   //---------- Reading HTTP request -------------------
   if(read_command(clientConnection,method,destination,version)==0) return;



   //------------ Read headers --------------------------
   int data_length = read_headers(clientConnection,discoverRequest);

   // Request to stop this server
   if(discoverRequest==2) {
	   do_shutdown = 1;
	   return;
   }


   //----------- Read data --------------------------------
   char* data_buffer;
   if(data_length>0) {
	   data_buffer = new char [data_length+1];
	   if(data_buffer==NULL) {
          printf("\n\n*** Error allocating memory!\n\n");

	   }
	   int len = data_length;
	   int readed = 0;
	   while(len>0) {   
	      int packet = recv(clientConnection,data_buffer+readed,data_length-readed,0);
		  if(packet<0) break;
		  len-=packet;
		  readed+=packet;
	   }

	   if(len>0) {
           printf("*** Client disconnected.\n\n");
		   return;
	   }
       DBG(printf("Readed from the client : %d\n",data_length));
       data_buffer[data_length]=0;
   }
   //-------------------------------------------------------

   if(discoverRequest==1) { // Request from the DeveloperXPress
	   if(data_length>0) {
           DBG(printf("Request : %s\nSource  : %s\nVersion : %s\nCommand : %s\nClient  : Discover\n",
	               method,destination,version,data_buffer));
	   } else {
           DBG(printf("Request : %s\nSource  : %s\nVersion : %s\nCommand : Empty Command\nClient  : Discover\n",
	               method,destination,version));
	   }
   } else { // Request from the WEB browser
       DBG(printf("Request : %s\nSource  : %s\nVersion : %s\nClient  : WEB browser\n\n",
            method,destination,version));
   }

   if(discoverRequest==1) { // Request from the discover client
	   char* model = GetModelName(destination);
       ServerConnection* server = ServerConnection::EstablishConnection(model);
	   if(server!=NULL) {
		   if(data_length>0) { // If we manage to get command from the client
               char* server_reply = server->EvalCommand(data_buffer);
		       if(server_reply!=NULL) {
				   // OK, server returned valid command results.
                   httpReply = WrapReply(server_reply,strlen(server_reply));
                   DBG(printf("Reply length : %d\n\n\n",strlen(server_reply)));
                   send(clientConnection,httpReply,strlen(httpReply),0);
			   } else {
				   // ERROR, server returned empty results. Server is down?
				   ServerConnection::RemoveConnection(model);
                   send(clientConnection,error_not_found,strlen(error_not_found),0);
                   printf("*** Removing dead model server %s.n",model);
			   }
		   } else {
			   // Empty command was receved from the client.
			   // Maybe client connection is broken ?
               httpReply = WrapReply("",0);
               send(clientConnection,httpReply,strlen(httpReply),0);
               DBG(printf("Reply length : 0 (empty command receved)\n\n\n"));
		   }
	   } else {
		   // No server with the given name was found.
		   char* error = "HTTP/1.1 404 Not Found\r\n\r\n";
           send(clientConnection,error,strlen(error),0);
           printf("*** Unable to locate model server %s.\n",model);
	   }
   } else { // Request from the WEB browser
	   // We will create Application/Discover reply in the format:
	   // Host        : <host ip>
	   // Model       : <model name>
	   // Source-Root : <server source root>
	   // <Error message>
	   // If the browser can understand Application/Discover MIME header
	   // it will open the DeveloperXPress. If not, it will show this page
	   // with the error message.
	   char* hostIp;
	   char buf[1024];
	   struct hostent *ent;
	   gethostname(buf,1024);
  	   ent = gethostbyname(buf);
	   if(ent!=NULL) {
          hostIp = inet_ntoa(*(in_addr *)*ent->h_addr_list);
	   } else {
		  hostIp = "127.0.0.1";
	   }
	   char* modelName = GetModelName(destination);
       char* message = "To view this HTTP source you need Discover browser.";
	   char* root = GetServerSrcRoot(destination);
	   char reply[5000];
	   strcpy(reply,"Host  : ");
	   strcat(reply,hostIp);
	   strcat(reply,"\r\n");
	   strcat(reply,"Model : ");
	   strcat(reply,modelName);
	   strcat(reply,"\r\n");
	   if(root!=NULL) {
	      strcat(reply,"Source-Root : ");
	      strcat(reply,root);
	      strcat(reply,"\r\n");
	   }
	   strcat(reply,message);
	   strcat(reply,"\r\n");
	   strcat(reply,"\r\n");
       char* httpReply = WrapReply(reply,strlen(reply));
       send(clientConnection,httpReply,strlen(httpReply),0);
   }

}
//------------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------------
// This main loop will wait for the client trying to connect to the given HTTP socket (httpSocket)
// will accept client request (creating client socket "clientConnection"), will read the client HTTP
// request from the client socket, will process this request on the server and will send the reply to the
// client. Will close client socket after sending the reply.
//------------------------------------------------------------------------------------------------------
void RequestsLoop(int httpSocket) {
   struct sockaddr s;
   int s_size = sizeof(s);
   do {
	   if(CheckSocket(httpSocket)) { // Waiting for the client request
	      int clientConnection = accept(httpSocket, &s, &s_size);
		  char val = 1;
	      if(clientConnection >= 0) {
	          // This function will read the client HTTP request and will process it on the
			  // Discover server. Will send the reply to the client.
	          ProcessCommandSendReply(clientConnection);
			  // We do not need this client connection anymore.
			  shutdown(clientConnection,2);
			  closesocket(clientConnection);
		  }
	  }
   } while(!do_shutdown);
}
//------------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------------
// This static function will check if the port occupied.
//------------------------------------------------------------------------------------------------------
static int IsPortInUse(int port) {
u_long addr;
struct sockaddr_in self_addr;
    
    addr                = inet_addr("127.0.0.1");
    struct hostent *ent = gethostbyaddr((char *)&addr, sizeof (addr), AF_INET);
    if(ent == NULL)
	return 0;
    self_addr.sin_family = AF_INET;
    self_addr.sin_port   = htons(port);
    self_addr.sin_addr   = *(in_addr *)*ent->h_addr_list;
    /* Create the socket. */
    int sock = socket (PF_INET, SOCK_STREAM, 0);
    if (sock < 0) return 0;
    /* Give the socket a name. */
    int running = 1;
    if(connect(sock,  (struct sockaddr *)&self_addr, sizeof (sockaddr_in)) < 0)
	     running = 0;
    closesocket(sock);
    return running;
}
//------------------------------------------------------------------------------------------------------



//------------------------------------------------------------------------------------------------------
// This static function will check if the port occupied.
//------------------------------------------------------------------------------------------------------
static int SendStopCommand(int port) {
char* command = 
"POST stop HTTP/1.1\r\n\
Accept: */*\r\n\
Accept-Language: en-us\r\n\
Content-Type: application/discover\r\n\
Accept-Encoding: gzip,deflate\r\n\
User-Agent: StopDiscover\r\n\
Host: 127.0.0.1\r\n\
Content-Length: 0\r\n\r\n";

u_long addr;
struct sockaddr_in self_addr;
    
    addr                = inet_addr("127.0.0.1");
    struct hostent *ent = gethostbyaddr((char *)&addr, sizeof (addr), AF_INET);
    if(ent == NULL)
	return 0;
    self_addr.sin_family = AF_INET;
    self_addr.sin_port   = htons(port);
    self_addr.sin_addr   = *(in_addr *)*ent->h_addr_list;
    /* Create the socket. */
    int sock = socket (PF_INET, SOCK_STREAM, 0);
    if (sock < 0) return 0;
    /* Give the socket a name. */
    if(connect(sock,  (struct sockaddr *)&self_addr, sizeof (sockaddr_in)) < 0)
	     return 0;
    send(sock,command,strlen(command),0);
    closesocket(sock);
	return 1;
    
}
//------------------------------------------------------------------------------------------------------



int main(int argc, char **argv) {
int httpListener;
#ifdef _WIN32
    WORD    wVersionRequested;  
    WSADATA wsaData; 
    int     err; 
   
    wVersionRequested = MAKEWORD(1, 1); 
    err               = WSAStartup(wVersionRequested, &wsaData);
    if (err != 0) {
	    printf(" No sockets available.\n");
	     return -1;
    }
#endif 
    ParseArguments(argc, argv);
    if(do_shutdown) {
		SendStopCommand(httpPort);
		return 0;
	}

	if(IsPortInUse(httpPort)==1) {
	   printf("Port %d is already in use.\nDiscover HTTP server is unable to start.\n",httpPort); 
	   return -1;

	}


    httpListener = CreateListener(httpPort);
    if(httpListener<0) {
	   printf("Unable to bind to the HTTP port 80.\nOn UNIX you must log in as a root.\n"); 
	   return -1;
	} else {
	   printf ("Listener %d created.\n",httpListener);
	   RequestsLoop(httpListener);
	   closesocket(httpListener);
	}
	return 0;
}
