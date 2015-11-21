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
#include "nameServCalls.h"
#ifdef _WIN32
#include <windows.h>
#include <winsock.h>
#include <io.h>
#else
#ifndef hp10
#include <sysent.h>
#endif
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#ifdef irix6
#include <sys/poll.h>
#endif
#endif
#include "SocketComm.h"

#ifndef _WIN32
#define closesocket(sock) close(sock)
#endif


#define BROADCAST_SERVER_PORT 28946
#define UNKNOWN      0
#define DNS_CONNECT  1
#define HOST_CONNECT 2


int is_admin(const char *nm) {
const char *p = nm;
const char *m = "Proxy:";
	while(*p!=0 && *m!=0) if(*p++!=*m++) return 0;
	return 1;
}


void listServicesEx(char* host)
{
    NameServerServiceList *list;
	int success;
	if(host==NULL)
        success = NameServerGetListOfServices(&list);
	else
        success = NameServerGetListOfServicesRemote(host,&list);

    if(success < 0) {
	    return;
    }
    NameServerServiceList *current = list;
    while(current != NULL) {
	   const char *nm = current->getName();
	   if(is_admin(nm)) {
	       printf("%s\n",nm);
	   }
	   current = current->getNext();
    }
    if(list)
	delete list;
}


int connectToService(char *host, char *service_name)
{
    int service_port, tcp_addr;
	int success;
	if(host==NULL) {
        success = NameServerGetService(service_name, service_port, tcp_addr);
	} else {
        success = NameServerGetServiceRemote(host,service_name, service_port, tcp_addr);
	}
    if(success < 0) {
	return -1;
    }
    if(success == 0) {
	return -1;
    }
    int socket = ConnectToServer(service_port, tcp_addr);
    return socket;
}


int main(int argc, char **argv) {
char command[1024];
char reply[5000];
int sock;
int broadcast_socket;
int i;
char *hosts[1024];

    for(i=0;i<1024;i++) hosts[i]=NULL;
#ifdef _WIN32
    WORD    wVersionRequested;  
    WSADATA wsaData; 
    int     err; 
   
    wVersionRequested = MAKEWORD(1, 1); 
    err               = WSAStartup(wVersionRequested, &wsaData); 
    if (err != 0) {
	    printf("no sockets available\n");
	return -1;
    }
#endif    

    if(argc<2) {
        listServicesEx(NULL);
	    return 0;
	} 

	// Service list using DNS or using exact host name
	if(strcmp(argv[1],"-list")==0) {
		if(argv[2]!=NULL) {
			for(i=2;i<argc;i++) {
                listServicesEx(argv[i]);
			}			
			return 0;
		}
		if(argv[2]==NULL) listServicesEx(NULL);
		return 0;

	}


	if(strcmp(argv[1],"-hosts")==0) {
	    // Broadcasting to detect all name servers running on subnet
        broadcast_socket = CreateBroadcast(BROADCAST_SERVER_PORT);
	    sockaddr_in srv_addr;
        srv_addr.sin_family      = AF_INET;
        srv_addr.sin_addr.s_addr = inet_addr("255.255.255.255");
        srv_addr.sin_port        = htons(BROADCAST_SERVER_PORT);
	    char buf[1024];
        const char *message = "query_host_name";
        int val = sendto(broadcast_socket,message,strlen(message),0,(sockaddr *)&srv_addr,sizeof(srv_addr));

        fd_set socket_set;
        int    nfsd = 0;
#ifndef _WIN32
        nfsd = FD_SETSIZE;
#endif
        FD_ZERO(&socket_set);
        FD_SET(broadcast_socket, &socket_set);

		timeval timeout;
		timeout.tv_sec  = 1;
		timeout.tv_usec = 0;

        int bytes;
        while (1) {
	    if(select(nfsd,&socket_set,NULL,NULL,&timeout)<=0) break;
            bytes = recvfrom(broadcast_socket,buf,1024,0,NULL,NULL);
	    buf[bytes]=0;
		printf("%s\n",buf);
	    }
        closesocket(broadcast_socket);
		return 0;
	}


	int mode = UNKNOWN;
	char *service = NULL;
	int parse_start=2;


	service = strdup(argv[1]);
	if((argv[2]!=NULL) && (strcmp(argv[2],"-connect")==0)) {
		if(argv[3]!=NULL) {
			for(i=3;i<argc;i++) {
				if(argv[i][0]=='-') break;
				hosts[i-3]=strdup(argv[i]);
			}
			parse_start = i;
			if(i>3) {
			    mode = HOST_CONNECT;
			} else {
		        mode = DNS_CONNECT;
			}
		} else {
		    mode = DNS_CONNECT;
            parse_start = 2;		
		}
	} else {
		mode = DNS_CONNECT;
		parse_start=2;
	}


	if((mode==UNKNOWN) || (mode== DNS_CONNECT)) {
         char real_name[1024];
         strcpy(real_name,"Proxy:");
	     strcat(real_name,service);
         sock = connectToService(NULL,real_name);
         if(sock<0) {
			 printf("Service not available.\n");
			 return 2;
		 }
	}
	

	if(mode == HOST_CONNECT) {
		i=0;
		while(hosts[i]!=NULL) {
           char *host = hosts[i];
	       char real_name[1024];
	       strcpy(real_name,"Proxy:");
	       strcat(real_name,service);
           sock = connectToService(host,real_name);
           if(socket>0) break;
		   i++;
		}
		if(hosts[i]==NULL) {
		    printf("Service not available.\n");
		    return 2;
		}
	}

	char *eof_flag;
	while(1) {
		printf("> ");
		eof_flag = gets(command);
		// Exit if we get EOF
		if (eof_flag == 0) break;
		else if (command && strlen(command)>0) {
			if(strcmp(command,"quit")==0) break;
			send(sock,command,strlen(command),0);
			int len;
  	        	len = recv(sock,reply,5000,0);
			// Exit if server side close the channel
			if(len<=0) break;
			reply[len]=0;
	        	printf("%s",reply);
		}
	}
	closesocket(sock);
	return 0;
}

