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
#define LARGE_MARSHAL 500000
#define SMALL_MARSHAL 2000
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/file.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <errno.h>
#include <sys/mman.h>
#include <memory.h>




#ifndef INADDR_NONE
#define INADDR_NONE 0xffffffff
#endif


extern int errno;

#ifndef linux2
extern char *sys_errlist[];
#endif

u_short portbase=0;


static int connectTCP(host,service)
    char *host;
    char *service;
{
    char *protocol="tcp";
    struct hostent *phe;
    struct servent *pse;
    struct protoent *ppe;
    struct sockaddr_in sin;
    int s, type;
    
    memset((char *)&sin,0,sizeof(sin));
    sin.sin_family = AF_INET;

    if(pse=getservbyname(service,protocol))
	sin.sin_port=pse->s_port;
    else if ((sin.sin_port=htons((u_short)atoi(service))) ==0)
        printf("can't get %s service entry\n",service);


    if(phe=gethostbyname(host))
	memcpy((char *)&sin.sin_addr,phe->h_addr, phe->h_length);
    else if((sin.sin_addr.s_addr =  inet_addr(host))==INADDR_NONE)
	printf("can't get %s host entry\n",host);

    
    if((ppe=getprotobyname(protocol))==0)
	printf("can't get %s protocol entry\n",protocol);


    if(strcmp(protocol,"udp")==0)
	type=SOCK_DGRAM;
    else
	type = SOCK_STREAM;



    s=socket(PF_INET,type, ppe->p_proto);
    if (s<0)
	printf("can't create socket: %s \n",sys_errlist[errno]);

    if(connect(s,(struct sockaddr *)&sin, sizeof(sin) )<0)
	printf("can't connect to  %s.%s: %s\n",host,service,sys_errlist[errno]);
    
    return s;
}


int DI_call_server(socket,to_server,results)
    int socket;
    char * to_server;
    char * results;
{
    static char temp[LARGE_MARSHAL];
    int i,n,retsize=0;
    int size=strlen(to_server)+1;



    write(socket,to_server,size);
    
    n=1;
    for(i=0;i<LARGE_MARSHAL && n>0;i++)
    {
	fflush(stdout);
	n=read(socket,&temp[i],1);
	if(!temp[i])break;
	if(n>0)retsize++;
    }
    

    if(n<0){printf("Error: %s\n",sys_errlist[errno]);return -1;}
    for(i=0;i<=retsize;i++)results[i]=temp[i];
    return(retsize);
}

static char parameters[LARGE_MARSHAL];


typedef int DI_object;
typedef int * DI_objectp;

typedef int * intp;
typedef char * charp;

static int this_socket;
static int param_pos;

void (*api_error_handler)();

void api_call_server(command)
    int command;
{
    static char buf[LARGE_MARSHAL];
    int i;

    sprintf(buf," %d ",command-1);

    param_pos=strlen(buf);


    strcat(buf,parameters);


    DI_call_server(this_socket,buf,buf);
    if(!strcmp(buf,"ERROR_HAS_HAPPENED"))
    {
	if(api_error_handler)(*api_error_handler)();
	exit(1);
    }

    parameters[0]=' ';
    for(i=param_pos;buf[i];i++)parameters[i-param_pos+1]=buf[i];
    parameters[i-param_pos+1]=0;

    param_pos=0;
}


void DI_object_marshal(o)
    DI_object * o;
{
    static char temp[SMALL_MARSHAL];
    sprintf(temp," %d ",*o);
    strcat(parameters,temp);
}

void DI_object_demarshal(o)
    DI_object *o;
{
    int i;
    static char strval[SMALL_MARSHAL];
    for(;parameters[param_pos]<=32;param_pos++);
    for(i=0;parameters[i+param_pos]>32;i++)
    {
	strval[i]=parameters[i+param_pos];
    }
    param_pos +=i;
    strval[i]=0;

    sscanf(strval,"%d",o);
}

void charp_marshal(c)
    charp * c;
{
    static char temp[LARGE_MARSHAL];
    int len,i;

    sprintf(temp,"%s",*c);

    len=strlen(*c);

    for(i=0;i<len;i++)
    {
	if(temp[i]<=32)temp[i]='#';
    }

    strcat(parameters," ");
    strcat(parameters,temp);
    strcat(parameters," ");
}

void charp_demarshal(c)
    charp *c;
{
    int i,len;
    static char strval[LARGE_MARSHAL];
    for(;parameters[param_pos]<=32;param_pos++);
    for(i=0;parameters[i+param_pos]>32;i++)
    {
	strval[i]=parameters[i+param_pos];
    }
    param_pos +=i;
    strval[i]=0;

    sscanf(strval,"%s",*c);

    len=strlen(*c);
    for(i=0;i<len;i++)
    {
	if((*c)[i]=='#')(*c)[i]=' ';
    }


}
    

void int_marshal(i)
    int *i;
{

    DI_object_marshal(i);

    return;
}

void int_demarshal(i)
    int *i;
{

    DI_object_demarshal(i);

    return;
}


void intp_demarshal(i)
    intp *i;
{
    
    int_demarshal(*i);
}

void intp_marshal(i)
    intp *i;
{
    int_marshal(*i);
   
}


void DI_objectp_demarshal(o)
    DI_objectp *o;
{
    intp_demarshal((intp *)o);
}

void DI_objectp_marshal(o)
    DI_objectp *o;
{
    intp_marshal((intp *)o);
}

void house_keep()
{
	parameters[0]=0;
}

int  DI_connect(host,service)
    char *host;
{
    return (this_socket=connectTCP(host,service));
}





