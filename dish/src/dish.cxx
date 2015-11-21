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
#ifndef ISO_CPP_HEADERS
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <fstream.h>
#include <iostream.h>
#else /* ISO_CPP_HEADERS */
#include <cstdio>
namespace std {};
using namespace std;
#include <cstdlib>
#include <cctype>
#include <cstring>
#include <fstream>
#include <iostream>
#endif /* ISO_CPP_HEADERS */
#include <sys/types.h>

#include <sys/types.h>
#include <genArr.h>
#include <xxinterface.h>
#ifdef _WIN32
    #include <windows.h>
    #include <winsock.h>
    #include <io.h>
#else
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

#include "nameServCalls.h"

#define CONSOLE 0
#define FILEOUT 1
#define DISLITE 2

int do_debug    = 0;
FILE* log_fd = stderr;

#include <sys/stat.h>
#ifndef ISO_CPP_HEADERS
#include <signal.h>
#else /* ISO_CPP_HEADERS */
#include <csignal>
#endif /* ISO_CPP_HEADERS */
#include "SocketComm.h"

#define BZERO(ptr,len) memset(ptr, '\0', len)
#define BUF_SZ 10000

#define UNKNOWN      0
#define DNS_CONNECT  1
#define HOST_CONNECT 2
#define HTTP_CONNECT 3

genArr(char) cmd;

static const char *ofile = NULL;
static char       *ofile_lock;
static int output_mode = CONSOLE;
static int showmode = 0;


typedef struct {
    char *host;
    char *service;
    char *ip;
    char *port;
} CConnectDescr;

CConnectDescr connections_list[1024];
int connection=0;
int isHttp=0;
char *service = NULL;
char *http_host = NULL;


//----------------------------------------------------------------
// This static function will return 0 if the given host is in the
// readable name format and will return 1 if it is in decimal
// delimeted format.
// ----------------------------------------------------------------

static int IsIp(const char* host)
{
    int dcount = 0;
    int result = 1;

    for(int i = 0; i < strlen(host); ++i) {
        if(host[i]=='.') {
            dcount++;
            continue;
        }
        if(host[i] < '0' || host[i] > '9') {
            result = 0;
            break;
        }
    }

    if(dcount!=3) result = 0;
    return result;
}


//----------------------------------------------------------------
// This static function will create unsigned integer IP from decimal
// delimeted string.
// ----------------------------------------------------------------
static unsigned IpFromDecDelim(const char* host) {
    int i,j,k;
    char val[10];
    union {
	struct {
            unsigned char b1;
            unsigned char b2;
            unsigned char b3;
            unsigned char b4;
	} s_unb;
	unsigned long s_ad;
    }conv;

    k=0;
    j=0;
    for(i=0;i<strlen(host);i++) {
        if(host[i]=='.') {
            val[j]=0;
            switch(k) {
            case 0 : conv.s_unb.b1 = atoi(val); break;
            case 1 : conv.s_unb.b2 = atoi(val); break;
            case 2 : conv.s_unb.b3 = atoi(val); break;
            case 3 : conv.s_unb.b4 = atoi(val); break;
            }
            j=0;
            val[j]=0;
            k++;
            continue;
        }
        val[j++]=host[i];
    }
    val[j]=0;
    if(j>0) {
        switch(k) {
        case 0 : conv.s_unb.b1 = atoi(val); break;
        case 1 : conv.s_unb.b2 = atoi(val); break;
        case 2 : conv.s_unb.b3 = atoi(val); break;
        case 3 : conv.s_unb.b4 = atoi(val); break;
        }
    }
    return conv.s_ad;
}


//----------------------------------------------------------------
// This static function will try to connect to the HTTP server on the
// given host using given port. Will return server socket or -1 if
// failed to connect.
// ----------------------------------------------------------------
static int connectToHttpServer(char* host, int port) {
    struct sockaddr_in serv_addr;

    if(IsIp(host)==0) {
        struct hostent *ent;
        ent = gethostbyname(host);
        if(ent == NULL) return -1;
        serv_addr.sin_addr   = *(in_addr *)*ent->h_addr_list;
    } else {

#if 0 //XXX porting?
        serv_addr.sin_addr.S_un.S_addr = IpFromDecDelim(host);
#else
        serv_addr.sin_addr.s_addr = IpFromDecDelim(host);
#endif
    }
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port   = htons (port);
	
    /* Create the socket. */
    int sock = socket (PF_INET, SOCK_STREAM, 0);
    if (sock < 0) return sock;
    /* Give the socket a name. */
    if(connect(sock,  (struct sockaddr *)&serv_addr, sizeof (sockaddr_in)) < 0) {
        closesocket(sock);
        return -1;
    }
    return sock;
}


//----------------------------------------------------------------
// This static function will send access command to the given socket
// wrapping it fo fit HTTP protocol.
//
// "server" defines object which will process request on the server.
// "accessCmd" is an access command which will be processed by the server object.
// If "usePost" is set POST command will be used, if not - GET command will  be used.
// Will return 1 if everything is OK and 0 if error during transmission.
//----------------------------------------------------------------
static int sendHttpRequest(int socket, char* server, char* accessCmd,int usePost)
{
    const char* Accept          = "Accept: %s\r\n";
    const char* AcceptLanguage  = "Accept-Language: %s\r\n";
    const char* AcceptEncoding  = "Accept-Encoding: %s\r\n";
    const char* ContentType     = "Content-Type: %s\r\n";
    const char* UserAgent       = "User-Agent: %s\r\n";
    const char* Connection      = "Connection: %s\r\n";
    const char* Host            = "Host: %s\r\n";
    const char* ContentLength   = "Content-Length: %d\r\n";
    char* hostIp;
    char* package;
    char  one_header[1024];
    char  buf[1024];


    // Determining current host ip string
    struct hostent *ent;
    gethostname(buf,1024);
    ent = gethostbyname(buf);
    if(ent!=NULL) {
        hostIp = inet_ntoa(*(in_addr *)*ent->h_addr_list);
        if(hostIp==NULL) {
            hostIp = "127.0.0.1";
        }
    } else {
        hostIp = "127.0.0.1";
    }



    // Forming HTTP command
    if(accessCmd) package = new char[strlen(accessCmd)+5000];
    else          package = new char[5000];

    if(usePost)  strcpy(package,"POST ");
    else         strcpy(package,"GET "); 

    if(server)   {
        strcat(package," ");
        strcat(package,server);
        strcat(package," ");
    } else {
        strcat(package," / ");
    }

    strcat(package,"HTTP/1.1\r\n");


    // Forming HTTP MIME headers
    sprintf(one_header,Accept,"*/*");
    strcat(package,one_header);

    sprintf(one_header,AcceptLanguage,"en-us");
    strcat(package,one_header);
   
    sprintf(one_header,AcceptEncoding,"gzip,deflate");
    strcat(package,one_header);

    sprintf(one_header,ContentType,"application/discover");
    strcat(package,one_header);

    sprintf(one_header,UserAgent,"discover");
    strcat(package,one_header);

    if(accessCmd!=NULL && strlen(accessCmd)>0) {
        sprintf(one_header,ContentLength,strlen(accessCmd));
        strcat(package,one_header);
    }

    sprintf(one_header,Host,hostIp);
    strcat(package,one_header);

    sprintf(one_header,Connection,"Keep-Alive");
    strcat(package,one_header);

    strcat(package,"\r\n");
    if(accessCmd!=NULL && strlen(accessCmd)>0) {
        strcat(package,accessCmd);
    }
    if(send(socket,package, strlen(package),0)>0) return 1;
    return 0;
}
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
// This static function will read the line from the http reply. 
//------------------------------------------------------------------------------------------
static int read_line(int sock, char* buf, int maxLen) {
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
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
// This static helper function will read a key name from the HTTP MIME-header line
//------------------------------------------------------------------------------------------
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
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
// This static helper function will read a key value from the HTTP MIME-header line
//------------------------------------------------------------------------------------------
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
//------------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------------
// Will just perform case_insensitive string comparation.
//------------------------------------------------------------------------------------------
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
//------------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------
// This static function will read HTTP request headers and will return data block 
// size or 0 if not available.
//-----------------------------------------------------------------------------------
static int ReadHeaders(int socket) {
    int data_length = 0;
    char buf[1024];
   
    for(;;) {
        if(read_line(socket,buf,1024)==0) break;
        if(strlen(buf)==0) break;
        if(cmp(GetKey(buf),"CONTENT-LENGTH")) {
            char* value = GetValue(buf);
            data_length = atoi(value);
        }
    }
    return data_length;
}
//------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------
static int parseReplyCommand(char* str) {
    int from,to;
    char numStr[100];

    for(from=0;from<strlen(str);from++) {
        if(str[from]==' ') {
            from++;
            break;
        }			
    }
    for(to=from;to<strlen(str);to++) {
        if(str[to]==' ') {
            break;
        }			
    }
    int i;
    for(i = from; i<to; i++)  {
        numStr[i-from]=str[i];
    }
    numStr[i]=0;
    return atoi(numStr);
}
//------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------------
// This static function will read HTTP server reply from the given socket and will 
// unwrap Access command results from the HTTP block. Will return results of communication
// OK or will return NULL if error in communication.
//------------------------------------------------------------------------------------------
static char* receiveHttpReply(int socket) {
    char buf[1024];
    int reply_length;
    int readed;
    static char* reply = NULL;


    if(reply!=NULL) delete reply;
    reply = NULL;
    if(read_line(socket,buf,1024)) {        // Reading reply header
        int status = parseReplyCommand(buf);
        if(status==400 || status==401) return NULL;         
        reply_length = ReadHeaders(socket); // Reading reply MIME-headers
        if(reply_length>0) {                // Reading reply data itself
            reply = new char [reply_length+1]; 
            int len = reply_length;
            readed = 0;
            while(len>0) {   
                int packet = recv(socket,reply+readed,reply_length-readed,0);
                if(packet<0) break;
                len-=packet;
                readed+=packet;
            }
            // If we were unable to read data perfectly - error
            if(readed<reply_length) {
                delete reply;
                reply=NULL;
            } else {
                reply[readed]=0;
            }
        } 
        if(reply_length==0) {
            reply = new char [1];
            reply[0]=0;
            return reply;
        }
    }
    return reply;

}
//------------------------------------------------------------------------------------------



char *ExpandFilename(char *name) {
    int i,j;
    static char fname[1024];

    j=0;
    fname[0]=0;
    for(i=0;i<strlen(name);i++) {
        if(name[i]=='~') {
            char* val = getenv("HOME");
            strcat(fname,val);
            j+=strlen(val);
            continue;
        }
        fname[j++]=name[i];
    }
    return fname;
}


char * gen_array_provide(char * ptr, int len, int& capacity, int objsize)
{
    size_t old_sz = capacity * objsize;

    if(len > capacity) {
	if(capacity == 0)
	    capacity = 1;
	while(capacity < len)
	    capacity *= 2;

	size_t new_sz = capacity * objsize;
	ptr = (ptr) ? ((char*)realloc(ptr, new_sz)) : ((char*)malloc(new_sz));

	if (ptr && (new_sz > old_sz))
	    BZERO((ptr + old_sz), new_sz - old_sz);
    }
    return ptr;
}

void gen_arr_free(char*ptr)
{
    if (ptr) free(ptr); 
}			      

int is_dislite(const char *nm) {
    const char *p = nm;
    char *dislite  = "Dislite:";
    char *discover = "Discover:";
    char *m;

    if(showmode==0) m= dislite;
    else m = discover;

    while(*p!=0 && *m!=0) if(*p++!=*m++) return 0;
    if(*p=='.') return 0;
    return 1;
}
	    


void listServices(void)
{
    NameServerServiceList *list;
    int success = NameServerGetListOfServices(&list);
    if(success < 0) {
	return;
    }
    NameServerServiceList *current = list;
    while(current != NULL) {
        const char *nm = current->getName();
        if(is_dislite(nm)) {
            if(showmode == 0) {
                cout << (char *)(nm+8) << endl;
            } else {
                cout << (char *)(nm+9) << endl;
            }
        }
        current = current->getNext();
    }
    if(list)
	delete list;
}


void listServicesEx(char* host,int test) {
    int port, addr;

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

        int res = 1;
        if(test!=0) {
            if(host==NULL)
                res = NameServerGetService(nm,port,addr);
            else
                res = NameServerGetServiceRemote(host,nm,port,addr);
        }
        if((res!=0) && is_dislite(nm)) {
            if(showmode == 0) {
                cout << (char *)(nm+8) << endl;
            } else {
                cout << (char *)(nm+9) << endl;
            }
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

int LinesAmountAndTransform(char *source) {
    char* s;
    char* d;
    int amount =0;
    int len = strlen(source);

    s=source;
    d=source;
    while(*s!=0) {
        if(*s=='\n') amount++;
        if(*s!='\r') *d++=*s;
        s++;
    }
    if( (len>0) && (*(s-1)!='\n') ) {
        amount++;
        *d++='\n'; 
    }
    *d = 0;
    return amount;
}


int sendBuffer(int socket, char *buf,int forward) {
    int success = 0;
    char *reply;

    if(isHttp!=0) {
        success = 0; 
        int httpSocket = connectToHttpServer(http_host,80);
        if(httpSocket==-1) return -10;
        if(sendHttpRequest(httpSocket,service,buf,1)!=0) {
            reply = receiveHttpReply(httpSocket);
            if(reply!=NULL) success=1;
            else            success=-20;
        }
        closesocket(httpSocket);
    } else {
        DISCommand *cmd;
        cmd = new DISCommand(buf,STDOUT_DISH_STREAM_CMD_TYPE);
        SendCommand(socket, cmd);
        unsigned char result_code;
        success = ReceiveReply(socket, &reply, result_code);
        delete cmd;
    }
    int len;
    int k;
    if(success == 1) {
        switch(output_mode) {
        case CONSOLE : len=strlen(reply);
            write(1, reply, len);
            if(len==0) {
                write(1, "% ", 2);
            } else {
                for(k=len-1;k>=0;k--) if(reply[k]=='\n') break;
                k++;
                if( ((k+1)<len)       && 
                    (reply[k]=='%')   && 
                    (reply[k+1]==' ') &&
                    (k+2==len)) {
                } else {
                    write(1, "\r\n% ", 4);
                }
            }
            break;
        case FILEOUT : if(ofile) {
                FILE *f;
                f = fopen(ofile, "wb");
                fwrite(reply, strlen(reply), 1, f);
                fclose(f);
                f = fopen(ofile_lock, "w");
                fwrite(buf, 1, 1, f);
                fclose(f);
            }
            break;
        case DISLITE : if(forward==1) {
                char* out;
                out = (char *)malloc(strlen(reply)+60);
                int lines = LinesAmountAndTransform(reply);
                sprintf(out,"%d\n%s",lines,reply);
                write(1, out, strlen(out));
                free(out);
            }
            break;
        }
    }
    return success;
}

static void append_cmd(char *in, int len)
{
    char *ptr = cmd.grow(len);
    strncpy(ptr, in, len);
}

int sendCLILine(int socket, char *buf)
{
    int success = 1;
    int len     = strlen(buf);
    static unsigned int cmd_length = 0;

    if (output_mode == DISLITE && cmd.size() == 0) {
	int i = 0;
	while (i < len && buf[i] && isdigit(buf[i])) {
	    i++;
	}
	if (i>0 && isspace(buf[i]) && buf[i+1]) {
	    buf[i] = '\0';
	    cmd_length = atoi (buf);
	    buf += i+1;
	    len -= i+1;
	} else {
	    cmd_length = 0;
	}
    }

    append_cmd (buf, len);
    buf = cmd[0];
    len = cmd.size();

    if (   (cmd_length == 0 && buf[len-1] == '\n')
           || (cmd_length != 0 && len >= cmd_length) ) {
	buf[len] = '\0';
	success = sendBuffer(socket, buf, 1);
	cmd.reset();
    }
    return success;
}

void mainLoop(int socket) {
    char buf[BUF_SZ+1];
    char tbuf[BUF_SZ+1];
    int  len;

    while(len = read (0, buf, BUF_SZ)) {
        buf[len]    = '\0';
        int j=0; 
        int k=0;
        for(int i=0;i<len;i++) {
            if(buf[i]!='\r' && buf[i]!='\n')  {
                tbuf[j++]=buf[i];
                if(buf[i]!=' ') k=j;
            }
        }
        tbuf[k]=0;
        if(strcmp(tbuf,"exit")==0) {
            break;
        }
        int success = sendCLILine(socket, buf);
        if(success != 1) {
            if(success == -10) {
                cerr << "**** No HTTP server detected on the given host." << endl;
            }
            if(success == -20) {
                cerr << "**** Model with the given name is not available on the given server." << endl;
            }
            break;
        }
    }
}

int UserLogin(int socket, int gui_client) {
    char log_record[1024];
    char* usr = getenv("USER");
    if((usr!=NULL) && (strlen(usr)>0)) {
        if(gui_client == 1) {
            sprintf(log_record,"login\tdislite\t%s",usr);
        } else {
            sprintf(log_record,"login\tdish\t%s",usr); 
        }
    } else {
        if(gui_client == 1) {
            sprintf(log_record,"login\tdislite\t%s","unknown");
        } else {
            sprintf(log_record,"login\tdish\t%s","unknown");
        }
    }
    return sendBuffer(socket,log_record,0);
}


int EvalFileOnServer (int socket, char* fname) {
    FILE *file = fopen(fname, "r");
    if(!file){
        return 0;
    }
    struct stat st;
    stat(fname, &st);
    int size  = st.st_size;
    char *buf = new char[size+1];
    int sz    = fread(buf, 1, size, file);
    if(sz == 0) return -1;
    buf[sz] = '\0';
    int status = sendBuffer(socket, buf,0);
    delete [] buf;
    return status;
}



void DescribeMyself(char* host, char* ip) {
    struct hostent *ent;
    char *daddr;

    gethostname(host,1024);
    ent = gethostbyname(host);
    if(ent!=NULL) {
        daddr = inet_ntoa(*(in_addr *)*ent->h_addr_list);
        if(daddr!=NULL) {
            sprintf(ip,"%s",daddr);
        }
    }
}


void print_help(char*arg)
{
    if(arg)
        cerr << "dish: wrong option: " << arg << endl;
    cerr << "commands:" << endl
         << " dish2 -help                                   shows this help screen" << endl
         << " dish2 -hosts                                  reports all hosts with name server running" << endl
         << " dish2 -ips                                    reports all host IPs with name server running" << endl
         << " dish2                                         lists all services available on this host" << endl
         << " dish2 -list <host1> <host2> ...               lists all services available on the specified host(s)" << endl
         << " dish2 -listpset <host1> <host2> ...           lists all pset services available on the specified host(s)" << endl
         << " dish2 <service>                               connect to this host service" << endl
         << " dish2 <service> -connect <host1> <host2>      connect to the service on the specified host(s)" << endl
         << " dish2 <service> -connectpset <host1> <host2>  connect to the pset service on the specified host(s)" << endl
         << " dish2 <service> -http <host>                  connect to the model server on the given host using HTTP protocol" << endl
         << "options:" << endl
         <<  " [-s[orce]] <file> source script" << endl;
}

int main(int argc, char **argv) {
    int i;
    int socket;
    int broadcast_socket;
    int exit_status;
    char *hosts[1024];
    int gui_client = 0;
    char my_host[1024];
    char my_ip[1024];
    int done;



    showmode = 0;
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

    if(argc < 2) {
        listServices();
        return 0;
    }

    if(strcmp(argv[1],"-help")==0) {
        print_help(0);
        return 0;
    }
   
    for(i=0;i<1024;i++) hosts[i]=NULL;


    if(strcmp(argv[1],"-hosts")==0) {
        // Broadcasting to detect all name servers running on subnet
        broadcast_socket = CreateBroadcast(BROADCAST_SERVER_PORT);
        sockaddr_in srv_addr;
        srv_addr.sin_family      = AF_INET;
        srv_addr.sin_addr.s_addr = inet_addr("255.255.255.255");
        srv_addr.sin_port        = htons(BROADCAST_SERVER_PORT);
        char buf[1024];
        char *message = "query_host_name";
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
        done = 0;
        while (1) {
#ifdef hp700
            if(select(nfsd,(int *)&socket_set,NULL,NULL,&timeout)<=0) break;
#else
            if(select(nfsd,&socket_set,NULL,NULL,&timeout)<=0) break;
#endif
            bytes = recvfrom(broadcast_socket,buf,1024,0,NULL,NULL);
            buf[bytes]=0;
            done = 1;
            printf("%s\n",buf);
        }
        DescribeMyself(my_host, my_ip);
        if(done == 0) printf("%s\n",my_host);
        closesocket(broadcast_socket);
        return 0;
    }

    if(strcmp(argv[1],"-ips")==0) {
        // Broadcasting to detect all name servers running on subnet
        broadcast_socket = CreateBroadcast(BROADCAST_SERVER_PORT);
        sockaddr_in srv_addr;
        srv_addr.sin_family      = AF_INET;
        srv_addr.sin_addr.s_addr = inet_addr("255.255.255.255");
        srv_addr.sin_port        = htons(BROADCAST_SERVER_PORT);
        char buf[1024];
        char *message = "query_host_ip";
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
        done = 0;
        while (1) {
#ifdef hp700
            if(select(nfsd,(int *)&socket_set,NULL,NULL,&timeout)<=0) break;
#else
            if(select(nfsd,&socket_set,NULL,NULL,&timeout)<=0) break;
#endif
            bytes = recvfrom(broadcast_socket,buf,1024,0,NULL,NULL);
            buf[bytes]=0;
            done = 1;
            printf("%s\n",buf);
        }
    	DescribeMyself(my_host, my_ip);
        if(done == 0) printf("%s\n",my_ip);
        closesocket(broadcast_socket);
        return 0;
    }

    if(strcmp(argv[1],"-all")==0) {
        // Broadcasting to detect all name servers running on subnet
        broadcast_socket = CreateBroadcast(BROADCAST_SERVER_PORT);
        sockaddr_in srv_addr;
        srv_addr.sin_family      = AF_INET;
        srv_addr.sin_addr.s_addr = inet_addr("255.255.255.255");
        srv_addr.sin_port        = htons(BROADCAST_SERVER_PORT);
        char buf[1024];
        char *message = "query_host_all";
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
        done = 0;
        while (1) {
#ifdef hp700
            if(select(nfsd,(int *)&socket_set,NULL,NULL,&timeout)<=0) break;
#else
            if(select(nfsd,&socket_set,NULL,NULL,&timeout)<=0) break;
#endif
            bytes = recvfrom(broadcast_socket,buf,1024,0,NULL,NULL);
            buf[bytes]=0;
            done = 1;
            printf("%s\n",buf);
        }
        DescribeMyself(my_host, my_ip);
        if(done == 0) printf("{ {%s} {%s} }\n",my_ip,my_host);
        closesocket(broadcast_socket);
        return 0;
    }


    // Service list using DNS or using exact host name
    if(strcmp(argv[1],"-list")==0) {
        showmode = 0;
        if(argv[2]!=NULL) {
            for(i=2;i<argc;i++) {
                listServicesEx(argv[i],0);
            }			
            return 0;
        }
        if(argv[2]==NULL) listServicesEx(NULL,0);
        return 0;

    }

    // Service list using DNS or using exact host name
    if(strcmp(argv[1],"-listpset")==0) {
        showmode = 1;
        if(argv[2]!=NULL) {
            for(i=2;i<argc;i++) {
                listServicesEx(argv[i],0);
            }			
            return 0;
        }
        if(argv[2]==NULL) listServicesEx(NULL,0);
        return 0;

    }

	
    // Service list using DNS or using exact host name
    if(strcmp(argv[1],"-test")==0) {
        showmode = 0;
        if(argv[2]!=NULL) {
            for(i=2;i<argc;i++) {
                listServicesEx(argv[i],1);
            }			
            return 0;
        }
        if(argv[2]==NULL) listServicesEx(NULL,1);
        return 0;

    }

    // Service list using DNS or using exact host name
    if(strcmp(argv[1],"-testpset")==0) {
        showmode = 1;
        if(argv[2]!=NULL) {
            for(i=2;i<argc;i++) {
                listServicesEx(argv[i],1);
            }			
            return 0;
        }
        if(argv[2]==NULL) listServicesEx(NULL,1);
        return 0;

    }

    int mode = UNKNOWN;
    int parse_start=2;

    service = strdup(argv[1]);
    if((argv[2]!=NULL) && (strcmp(argv[2],"-connect")==0)) {
        showmode = 0;
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
        if((argv[2]!=NULL) && (strcmp(argv[2],"-connectpset")==0)) {
            showmode =1;
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
                parse_start=2;
            } 
        } else {
            if((argv[2]!=NULL) && (strcmp(argv[2],"-http")==0)) {
                showmode =1;
                if(argv[3]!=NULL) {
                    for(i=3;i<argc;i++) {
                        if(argv[i][0]=='-') break;
                        hosts[i-3]=strdup(argv[i]);
                    }
                    parse_start = i;
                    http_host = hosts[0];
                    if(i>3) {
                        mode = HTTP_CONNECT;
                        isHttp = 1;
                    } else {				     
                        cerr << "**** No HTTP server specified." << endl;
                        return 1;
                    }
                } else {
                    cerr << "**** No HTTP server specified." << endl;
                    return 1;
                }
            }
        }
    }		

    if((mode==UNKNOWN) || (mode== DNS_CONNECT)) {
        char real_name[1024];
        if(showmode == 0)
            strcpy(real_name,"Dislite:");
        else
            strcpy(real_name,"Discover:");
        strcat(real_name,service);
        socket = connectToService(NULL,real_name);
        if(socket<0) {
            cerr << "**** Service does not exist." << endl;
            return 1;
        }
    }
	

    if(mode == HOST_CONNECT) {
        i=0;
        while(hosts[i]!=NULL) {
            char *host = hosts[i];
            char real_name[1024];
            if(showmode == 0)
                strcpy(real_name,"Dislite:");
            else
                strcpy(real_name,"Discover:");
            strcat(real_name,service);
            socket = connectToService(host,real_name);
            if(socket>0) break;
            i++;
        }
        if(hosts[i]==NULL) {
            cerr << "**** Service does not exist." << endl;
            return 2;
        }
    }
    char* source_file = NULL;
    int  stop_it     = 0;
    enum argState { NONE, SRC, DIS, OFILE} state = NONE; 
    for(int ii = parse_start; ii < argc; ++ii){
        char * arg = argv[ii];
        if(*arg=='-') {
            if(strcmp(arg,"-source")==0) {
                state = SRC;
            } else 
                if(strcmp(arg,"-s")==0) {
                    state = SRC;
                } else 
                    if(strcmp(arg,"-dislite")==0) {
                        output_mode = DISLITE;
                        gui_client = 1;
                        state = NONE;
                    } else 
                        if(strcmp(arg,"-devexpress")==0) {
                            output_mode = DISLITE;
                            gui_client = 1;
                            state = NONE;
                        } else 
                            if(strcmp(arg,"-o")==0)
                                state = OFILE;
                            else 
                                if(strcmp(arg,"-exit")==0){
                                    state = NONE;
                                    exit_status = 1;
                                } else 
                                    if(strcmp(arg,"-help")==0) {
                                        print_help(0);
                                        state       = NONE;
                                        exit_status = 1;
                                    } else 
                                        if(strcmp(arg,"-init")==0) {
                                            state       = NONE;
                                            exit_status = 3;
                                        } else 
                                            if(strcmp(arg,"-stop")==0) {
                                                stop_it = 1;
                                            } else 
                                                if(strcmp(arg,"-pset")==0)  {
                                                    showmode = 1;
                                                } else {
                                                    print_help(arg);
                                                    state       = NONE;
                                                    exit_status = 1;
                                                }
        } else {
            switch (state) {
            case OFILE:
#ifndef _WIN32
                ofile      = arg;
#else
                ofile = ExpandFilename(arg);
#endif
                ofile_lock = (char*)malloc(strlen(ofile)+6);
                sprintf(ofile_lock, "%s.lock", ofile);
                state       = NONE;
                cout << "results will be in " << ofile << endl;
                output_mode = FILEOUT;
                break;
            case SRC:   source_file = strdup(arg);
                state       = NONE;
                break;
            default:    print_help(arg);
                return 1;
            }
        }
    }



    // If we are working using HTTP server the HTTP server will log in 
    // the model server by itself

    if(isHttp==0) {
        // Send  login record to the proxy or server.
        UserLogin(socket,gui_client);
    } else {
        switch(sendBuffer(socket,"\n",0)) {
        case -10:  cerr << "**** No HTTP server detected on the given host." << endl;
            free(source_file);
            return 1;
        case -20:  cerr << "**** Model with the given name is not available on the given server." << endl;
            free(source_file);
            return 1;
        }


    }


    // Stops the server and returns
    if(stop_it == 1) {
        sendBuffer(socket, "stop_server -y",0);
        closesocket(socket);
        closesocket(broadcast_socket);
        return 0;
    }


    // Source file if needed
    char message[1024];
    if((source_file != NULL) && (strlen(source_file)>0)) {
        switch(EvalFileOnServer(socket,source_file)) {
        case 0  :  sprintf(message,"File \"%s\" does not exist!",source_file);
            cout << message << endl;			          
            free(source_file);
            return 1;
        case -1 :  sprintf(message,"Error reading file \"%s\"!",source_file);
            cout << message << endl;			          
            free(source_file);
            return 1;
        case -10:  cerr << "**** No HTTP server detected on the given host." << endl;
            free(source_file);
            return 1;
        case -20:  cerr << "**** Model with the given name is not available on the given server." << endl;
            free(source_file);
            return 1;
        default :  free(source_file);

        }
    }

#ifndef _WIN32
    signal(SIGPIPE, SIG_IGN);
#endif
    mainLoop(socket);
    closesocket(socket);
    closesocket(broadcast_socket);
    return 0;
}
