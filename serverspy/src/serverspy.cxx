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
#include <stdlib.h>
#include <time.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/types.h>
#include <string.h>
#include <sys/stat.h>
#include "nameServCalls.h"
#ifdef _WIN32
    #include <windows.h>
    #include <winsock.h>
    #include <io.h>
#else
   #include <signal.h>
   #include <arpa/inet.h>
   #include <netinet/tcp.h>
   #include <netdb.h>
   #include <sys/socket.h>
   #include <fcntl.h>
   #include <netinet/in.h>
   #include <unistd.h>
   #include <sys/types.h>
   #include <sys/wait.h>
   #ifdef sun4
      #include <poll.h>
      #include <sys/time.h>
   #endif
   #ifdef irix6
      #include <sys/poll.h>
   #endif
   #ifdef hp700
      #include <sys/poll.h>
      extern "C" int select(int, int*, int*, int*, const struct timeval*);
   #endif
#endif
#include "SocketComm.h"
#include "startprocess.h"

using namespace MBDriver;

#define ALL_COUNT       0
#define USER_COUNT      1
#define HOST_COUNT      2
#define ALL_LIST        3
#define USER_LIST       4
#define HOST_LIST       5
#define ALL_STOP        6
#define CLIENT_STOP     7
#define USER_STOP       8
#define HOST_STOP       9
#define REBOOT          10
#define UNKNOWN         11

#ifndef _WIN32
#define closesocket(sock) close(sock)
#endif

char *pdf_str[1024];
int num_pdfs      = 0;
char *logfile_str = NULL;
char *prefs_str   = NULL;
char *service_str = NULL;
char *admin_str   = NULL;
char *parent_str  = NULL;
char *last_command_ptr;
static int ID_Counter=0;

static MBDriver::pid_t current_server=null_pid;
static sockaddr_in model_connection_descriptor;
static int TimeoutValue = -1;

int do_shutdown = 0;
int do_stop     = 0;
int do_debug    = 0;
struct sockaddr_in *parent = NULL;

#define DBG(stmt) if(do_debug) { stmt; }

int  StartModelServer(void);
int  RestartModelServer(void);
void AbortService(void);
char*  InternalServiceName(void);

// this function makes sure that a whole buffer gets sent
int sendSafe(int sock,char* buf,int len) {
	int nSent = 0;
	int nCur = 0;
	while((nCur = send(sock,buf+nSent,len-nSent,0))>0 && (nSent+=nCur)<len);
	return nSent;
}

int sendBuffer(int socket, char *buf)
{
    DISCommand *cmd = new DISCommand(buf, STDOUT_DISH_STREAM_CMD_TYPE);
    SendCommand(socket, cmd);
    char *reply;
    unsigned char result_code;
    int success = ReceiveReply(socket, &reply, result_code);
    if(success == 1) {
       write(1, reply, strlen(reply));
           delete [] reply;
        }
    delete cmd;
    return success;
}

//------------------------------------------------------------------------------
// Each time Developer Express client connect to the model server it 
// sources its server-side script.
//------------------------------------------------------------------------------
void SourceDisliteStartupScript(int socket) {
    char ScriptFile[1024];
    char *p;

    p=getenv("PSETHOME");
    if(p!=NULL) {
        sprintf(ScriptFile,"%s/lib//dislite.dis",p);
    } else {
        sprintf(ScriptFile,"c:/discover/lib/dislite.dis");
    } 

    FILE *file = fopen(ScriptFile, "r");
    if(!file){
        return;
    }
    struct stat st;
    stat(ScriptFile, &st);
    int size  = st.st_size;
    char *buf = new char[size+1];
    int sz    = fread(buf, 1, size, file);
    if(sz == 0){
        return;
    }
    buf[sz] = '\0';
    sendBuffer(socket,buf);
    char * fr =  "set_printformat 0";
    sendBuffer(socket,fr);
    delete [] buf;
    fclose(file);
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Will terminate process with the given ID
//------------------------------------------------------------------------------
void terminate_service_process(pid_t pid) {
    int aa,bb;
    terminate_process(pid);
    while(NameServerGetService(InternalServiceName(),aa,bb)==1);
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// This function will keep track of whether there is a server
// currently available.  Returns 0 for no, 1 for yes.  If given
// an argument, sets the server status to the argument.
//------------------------------------------------------------------------------
int server_is_ready(int setval=-1) {
    static int ready = 0;

    if (setval == 0 || setval == 1) ready = setval;

    return ready;
}

//------------------------------------------------------------------------------
// This function will check the status of the model_server process.  
// Return values are -1, no model_server started;  0, model_server
// started and exited;  1, model_server starting up;  2, model_server
// up and registered.
//------------------------------------------------------------------------------
int check_server_status(int init=0) {
    static int query_count = 0;

    if (init) query_count = 0;

    int service_port;
    int tcp_addr;

    if (current_server == NULL) return -1;

#ifdef _WIN32
    DWORD code;
    GetExitCodeProcess(current_server,&code);
    if(code!=STILL_ACTIVE) return 0;
#else
    if(waitpid(current_server,NULL,WNOHANG)==-1) return 0;
#endif

    // Check if registered in the name server
    int res = NameServerGetService(InternalServiceName(),service_port,tcp_addr);
    if(res==1) {
        return 2;
    } else {
        query_count++;
        // 3000 times GetService fails - timeout (0.25 sec per call,
        // call every second, so 60 minutes)
        if(query_count>=20000) {
            terminate_service_process(current_server);
            query_count = 0;
            return 0;
        }
        return 1;
    }
}

//------------------------------------------------------------------------------
// This function will start the server and exit without waiting for it
// to complete startup.  Returns 0 to indicate failure, 1 to indicate 
// that process was started, 2 to indicate that the process started and
// is already registered.
//------------------------------------------------------------------------------
int start_new_server(const char *path, const char *const argv[]) {
    int aa,bb;
    NameServerGetService(InternalServiceName(),aa,bb);

    current_server = start_process(path, argv);
    return check_server_status(1);
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// This function will start the name_server.  Returns 0 to indicate 
// failure, 1 to indicate that process was started.
//------------------------------------------------------------------------------
int start_name_server() {
    char *cmd;
    char *p;
    static const char *argv[] = { NULL };
    
    p=getenv("PSETHOME");
    cmd = new char[(p!=NULL ? strlen(p) : 0) + 20];
    if(p!=NULL) {
        sprintf(cmd,"%s/bin/nameserv",p);
    } else {
        sprintf(cmd,"nameserv");
    }

    int result = start_process(cmd, argv) != 0;
    delete [] cmd;
    return result;
}
//------------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////

//##############################################################################
// This class represents the administrator record in the administrators 
// linked list.  Any administrator record contain administrator name, 
// administrator TCP address and port.
//##############################################################################
class AdminDescriptor {
public:
    AdminDescriptor( char *name, 
                     int socket,
                     int port,
                     int tcp_addr);

    ~AdminDescriptor();


    static void             AddNewAdmin(AdminDescriptor *sd);
    static void             RemoveAdmin(char *name);
    static void             RemoveAdmin(AdminDescriptor *p);
    static AdminDescriptor *LookupName(char *name);

    char *m_Name;
    int   m_Socket;
    int   m_Port;
    int   m_TCP;
    
    AdminDescriptor *m_Next;
    static AdminDescriptor *admin_list;
    static AdminDescriptor *admin_list_last;
};

AdminDescriptor *AdminDescriptor::admin_list      = NULL;
AdminDescriptor *AdminDescriptor::admin_list_last = NULL;

//------------------------------------------------------------------------------
// Constructor will create new admin node with the admin name
//------------------------------------------------------------------------------
AdminDescriptor::AdminDescriptor(char *n, 
                                 int socket,
                                 int port, 
                                 int addr)
{
    this->m_Name   = strdup(n);
    this->m_Socket = socket;
    this->m_Port   = port;
    this->m_TCP    = addr;
    this->m_Next   = NULL;
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Destructor will free the space allocated for the new client name
//------------------------------------------------------------------------------
AdminDescriptor::~AdminDescriptor() {
    free(m_Name);
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// This method will add new client to the clients linked list
//------------------------------------------------------------------------------
void AdminDescriptor::AddNewAdmin(AdminDescriptor *sd) {
    if(admin_list == NULL){
        admin_list      = sd;
        admin_list_last = sd;
    } else {
        admin_list_last->m_Next = sd;
        admin_list_last        = sd;
    }
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
// This function will return a pointer to the client record in the 
// clients linked list or NULL of no client with the given name found.
//------------------------------------------------------------------------------
AdminDescriptor *AdminDescriptor::LookupName(char *name) {
    AdminDescriptor *cur = admin_list;
    while(cur != NULL){
        if(strcmp(cur->m_Name, name) == 0)
            return cur;
        cur = cur->m_Next;
    }
    return NULL;
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// This function will remove the client with the given name.
//------------------------------------------------------------------------------
void AdminDescriptor::RemoveAdmin(char *name) {
    AdminDescriptor *prev = NULL;
    AdminDescriptor *cur  = admin_list;
    while(cur != NULL) {
        if(strcmp(cur->m_Name, name) == 0) {
            closesocket(cur->m_Socket);
            if(prev == NULL) admin_list = cur->m_Next;
            else prev->m_Next = cur->m_Next;
            if(admin_list_last == cur)
                admin_list_last = prev;
            delete cur;
            return;
        }
        prev = cur;
        cur  = cur->m_Next;
    }
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// This function will remove client with the given address
//------------------------------------------------------------------------------
void AdminDescriptor::RemoveAdmin(AdminDescriptor *p) {
    AdminDescriptor *prev = NULL;
    AdminDescriptor *cur  = admin_list;
    while(cur != NULL) {
        if(p==cur) {
            closesocket(cur->m_Socket);
            if(prev == NULL)
                admin_list = cur->m_Next;
            else
                prev->m_Next = cur->m_Next;
            if(admin_list_last == cur)
                admin_list_last = prev;
            delete cur;
            return;
        }
        prev = cur;
        cur  = cur->m_Next;
    }
}
//------------------------------------------------------------------------------


//##############################################################################
// This class represents the client record in the clients linked list.
// Any client record contain client name, client TCP address and port 
// and server TCP address and port.
//##############################################################################
class ClientDescriptor {
public:
    ClientDescriptor(int id, 
                     int client_socket,
                     int client_port,
                     int client_tcp_addr,
                     int server_socket,
                     int server_port,
                     int server_tcp_addr,
                     int type,
                     char* user);

    ~ClientDescriptor();

    void UpdateServer(int port, int tcp_addr);

    static void             AddNewClient(ClientDescriptor *sd);
    static void             RemoveClient(int id);
    static void             RemoveClient(ClientDescriptor *p);
    static void             RemoveAllClients(void);
    static void             RemoveStateClients(void);
    static ClientDescriptor *LookupID(int id);

    static int m_NumberClients;

    int   m_ID;
    int   m_ClientSocket;
    int   m_ClientPort;
    int   m_ClientTCP;
    int   m_ServerSocket;
    int   m_ServerPort;
    int   m_ServerTCP;
        char* m_UserName;

        int   m_Processing;
        char* m_LastCommand;
    int   m_LastCommandLength;
        int   m_Timeout;

        int   m_Commands;
        int m_Type;

    ClientDescriptor *m_Next;
    static ClientDescriptor *sd_list;
    static ClientDescriptor *sd_list_last;
};

ClientDescriptor *ClientDescriptor::sd_list      = NULL;
ClientDescriptor *ClientDescriptor::sd_list_last = NULL;
int ClientDescriptor::m_NumberClients            = 0;

//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Constructor will create new client with the client name
//------------------------------------------------------------------------------
ClientDescriptor::ClientDescriptor(int id, 
                                   int client_socket,
                                   int client_port, 
                                   int client_addr,
                                   int server_socket,
                                   int server_port,
                                   int server_addr,
                                   int type,
                                   char* user)
{
    this->m_UserName       =  strdup(user);
    this->m_ID             = id;
    this->m_ClientSocket   = client_socket;
    this->m_ClientPort     = client_port;
    this->m_ClientTCP      = client_addr;
    this->m_ServerSocket   = server_socket;
    this->m_ServerPort     = server_port;
    this->m_ServerTCP      = server_addr;
    this->m_Processing     = 0;
    this->m_Timeout        = 0;
    this->m_Commands       =0;
    this->m_LastCommand     = NULL;
    this->m_LastCommandLength = 0;
    this->m_Next           = NULL;
    this->m_Type           = type;

}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Destructor will free the space allocated for the new client name
//------------------------------------------------------------------------------
ClientDescriptor::~ClientDescriptor() {
        if(m_LastCommand!=NULL) free(m_LastCommand);
        m_LastCommand=NULL;
        if(m_UserName!=NULL) free(m_UserName);
        m_UserName=NULL;
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// This will change server port and TCP in the case the application 
// restart server to recover from error.
//------------------------------------------------------------------------------
void ClientDescriptor::UpdateServer(int p, int addr) {
    this->m_ServerPort  = p;
    this->m_ServerTCP   = addr;
}
//------------------------------------------------------------------------------

void LogConnectionEvent ( ClientDescriptor *, int );

//------------------------------------------------------------------------------
// This method will add new client to the clients linked list
//------------------------------------------------------------------------------
void ClientDescriptor::AddNewClient(ClientDescriptor *sd) {
    if(sd_list == NULL){
        sd_list      = sd;
        sd_list_last = sd;
    } else {
        sd_list_last->m_Next = sd;
        sd_list_last         = sd;
    }
    m_NumberClients++;
    if (sd->m_Type != -1) {
        LogConnectionEvent( sd, 1 );
    }
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// This function will return a pointer to the client record in the 
// clients linked list or NULL if no client with the given name found.
//------------------------------------------------------------------------------
ClientDescriptor *ClientDescriptor::LookupID(int id)
{
    ClientDescriptor *cur = sd_list;
    while(cur != NULL){
            if(cur->m_ID==id) return cur;
            cur = cur->m_Next;
    }
    return NULL;
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// This function will remove the client with the given name.
//------------------------------------------------------------------------------
void ClientDescriptor::RemoveClient(int id) {
    ClientDescriptor *prev = NULL;
    ClientDescriptor *cur  = sd_list;
    while(cur != NULL) {
        if(cur->m_ID == id) {
            if (m_NumberClients > 0) {
                m_NumberClients--;
            }
            if (cur->m_Type != -1) {
                LogConnectionEvent( cur, 0 );
            }
            closesocket(cur->m_ClientSocket);
            closesocket(cur->m_ServerSocket);
            if(prev == NULL)
                sd_list = cur->m_Next;
            else
                prev->m_Next = cur->m_Next;
            if(sd_list_last == cur)
                sd_list_last = prev;
            delete cur;
            return;
        }
        prev = cur;
        cur  = cur->m_Next;
    }
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// This function will remove all clients which depend upon state
//------------------------------------------------------------------------------
void ClientDescriptor::RemoveStateClients() {
    ClientDescriptor *prev = NULL;
    ClientDescriptor *cur  = sd_list;
    while(cur != NULL) {
                if(cur->m_Type!=1) { // If this client depends on the state
                if (cur->m_Type != -1) {
                        LogConnectionEvent( cur, 0 );
                        }
                closesocket(cur->m_ClientSocket);
                closesocket(cur->m_ServerSocket);

                if (m_NumberClients > 0) m_NumberClients--;
        
                if(prev == NULL) sd_list = cur->m_Next;
            else prev->m_Next = cur->m_Next;
                if(sd_list_last == cur) sd_list_last = prev;
                } else {
                prev = cur;
                }
        cur  = cur->m_Next;
    }
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
// This function will remove client with the given address
//------------------------------------------------------------------------------
void ClientDescriptor::RemoveClient(ClientDescriptor *p) {
    ClientDescriptor *prev = NULL;
    ClientDescriptor *cur  = sd_list;
    while(cur != NULL) {
        if(p==cur) {
            if (m_NumberClients > 0) {
                m_NumberClients--;
            }
            if (cur->m_Type != -1) {
                LogConnectionEvent( cur, 0 );
            }
            closesocket(cur->m_ClientSocket);
            closesocket(cur->m_ServerSocket);
            if(prev == NULL)
                sd_list = cur->m_Next;
            else
                prev->m_Next = cur->m_Next;
            if(sd_list_last == cur)
                sd_list_last = prev;
            delete cur;
            return;
        }
        prev = cur;
        cur  = cur->m_Next;
    }
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// This function will remove all clients.
//------------------------------------------------------------------------------
void ClientDescriptor::RemoveAllClients(void) {
    ClientDescriptor *p;
    ClientDescriptor *cur  = sd_list;
    while(cur != NULL) {
                p=cur;
                if (m_NumberClients > 0) {
                    m_NumberClients --;
                }
                if (p->m_Type != -1) {
                    LogConnectionEvent( p, 0 );
                }
                cur  = cur->m_Next;
                closesocket(p->m_ClientSocket);
                if(p->m_LastCommand!=NULL)  {
                        free(p->m_LastCommand);
                        p->m_LastCommand=NULL;
                }
                if(p->m_UserName!=NULL) {
                    free(p->m_UserName);
                        p->m_UserName=NULL;
                }
                delete p;
    }
    sd_list=sd_list_last=NULL;
    m_NumberClients = 0;

}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
// This function will return the name visible for all clients
//------------------------------------------------------------------------------
char*  ExternalServiceName(void) {
static char name [1024];

        if(service_str != NULL && strlen(service_str)!=0) {
                strcpy(name,"Dislite:");
                strcat(name,service_str);
                return name;
        } else {
                return "Dislite:model_service";
        }
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// This function will return a name visible only for proxy
//------------------------------------------------------------------------------
char*  InternalServiceName(void) {
static char name [1024];

        if(service_str != NULL && strlen(service_str)!=0) {
                strcpy(name,"Dislite:.");
                strcat(name,service_str);
                return name;
        } else {
                return "Dislite:.model_service";
        }
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// This function will return the name which we will need to pass to 
// the model_server startup command. The model_server will automatically 
// add "Dislite:" to the name of the service and this way startup 
// service name will be converted to the internal service name.
//------------------------------------------------------------------------------
char*  StartupServiceName(void) {
static char name [1024];

        if(service_str != NULL && strlen(service_str)!=0) {
                strcpy(name,".");
                strcat(name,service_str);
                return name;
        } else {
                return ".model_service";
        }
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// This function will return the name of the administrators channel.
//------------------------------------------------------------------------------
char*  AdminName(void) {
static char name [1024];

        if(admin_str != NULL && strlen(admin_str)!=0) {
                strcpy(name,"Proxy:");
                strcat(name,admin_str);
                return name;
        } else {
                strcpy(name,"Proxy:");
                strcat(name,service_str);
                return name;
        }
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// This function will write to the logfile when clients connect or
// disconnect.  
//------------------------------------------------------------------------------
void LogConnectionEvent ( ClientDescriptor *cd, int connect ) {
    if ( logfile_str != NULL && cd != NULL ) {
        FILE *file = fopen(logfile_str, "a");
        if (file) {
            char now[32];
            time_t s   = time(NULL);
            char *t    = ctime (&s);
            for( int i=0; i<32 && t[i]; i++) {
                now[i] = t[i];
                if ( t[i] == '\n' ) {
                    now[i] = '\0';
                    break;
                }
            }
            now[31] = '\0';
            fprintf(file, "%s %s : user %s (ID %d) %s.  %d clients connected.\n", now, ExternalServiceName(), cd->m_UserName, cd->m_ID, (connect ? "connected" : "disconnected"), ClientDescriptor::m_NumberClients);
            fclose(file);
        }
    }
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// This function will check if data available in the selected socket
//------------------------------------------------------------------------------
int CheckSocket(int socket) {
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
void WaitForAnyInput(int client_sock, int admin_sock) {
    timeval timeout;
    timeout.tv_sec  = 1;
    timeout.tv_usec = 0;
    fd_set sock_set;
    int nfsd = 0;
#ifndef _WIN32
    nfsd = FD_SETSIZE;
#endif

    FD_ZERO(&sock_set);

    if (server_is_ready()) {
        FD_SET(client_sock, &sock_set);
        ClientDescriptor *cli = ClientDescriptor::sd_list;
        while(cli != NULL){
            FD_SET(cli->m_ClientSocket, &sock_set);
            FD_SET(cli->m_ServerSocket, &sock_set);
            cli = cli->m_Next;
        }
    }
    FD_SET(admin_sock, &sock_set);
    AdminDescriptor *admin = AdminDescriptor::admin_list;
    while(admin != NULL){
        FD_SET(admin->m_Socket, &sock_set);
        admin = admin->m_Next;
    }
#ifdef hp700
    select(nfsd,(int *)&sock_set,NULL,NULL,&timeout);
#else
    select(nfsd,&sock_set,NULL,NULL,&timeout);
#endif
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// This procedure will create a socket and set it to close on exec.
//------------------------------------------------------------------------------
int create_socket() {
    int sock = socket (PF_INET, SOCK_STREAM, 0);

#ifndef _WIN32
    if (sock >= 0) {
        fcntl(sock, F_SETFD, 1);
    }
#endif

    return sock;
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// This procedure will create a listen socket for client requests 
// channel and for admin requests channel
//------------------------------------------------------------------------------
int CreateListener() {
    int sock;
    struct sockaddr_in name;
    /* Create the socket. */
    sock = create_socket();
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
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// This function will try to connect to the existing model server.
//------------------------------------------------------------------------------
int ConnectToModelServer(void) {
int model_socket;

    // Create the connection-oriented socket
    model_socket = create_socket();
    if (model_socket< 0) return -1;

    int tmp_switch = 1;
    setsockopt(model_socket, IPPROTO_TCP, TCP_NODELAY, (char *)&tmp_switch, sizeof(tmp_switch));

    // Try to get connection. We will use model_socket_name global 
    // data structure which will contain up-to-date information about 
    // model server. This information is filled every time the server 
    // starts or during recovery procedure.
    if(connect(model_socket,(struct sockaddr *)&model_connection_descriptor, sizeof (sockaddr_in)) < 0) {
        return -1;
    }
    return model_socket;

}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// This function will create new client record in the clients linked list.
//------------------------------------------------------------------------------
void CreateNewClient(int id, int client_socket, int client_port, int client_tcp, 
                                                     int model_socket, int model_port, int model_tcp, int type, char* user) {

     ClientDescriptor* descr;
         descr = new ClientDescriptor(id,client_socket,client_port,client_tcp,model_socket,model_port,model_tcp,type,user);
         descr->AddNewClient(descr);

}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// This function will create new administrator record in the administrators 
// linked list.
//------------------------------------------------------------------------------
void CreateNewAdmin(char* name, int socket, int port, int tcp) {
    AdminDescriptor* descr;
    descr = new AdminDescriptor(name,socket,port,tcp);
    descr->AddNewAdmin(descr);
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Call this function to terminate message loop and exit this application
//------------------------------------------------------------------------------
void AbortService(void) {
    ClientDescriptor::RemoveAllClients();
    NameServerUnRegisterService(ExternalServiceName());
    NameServerUnRegisterService(AdminName());
    do_shutdown=1;
}
//------------------------------------------------------------------------------

void GetLogin(int client_socket,int& client_type, char* username,char* password) {
    int  i=0;
    int  field = 0;
    char buf[1024];
    char type[200];
    int  type_count = 0;
    int  user_count = 0;
    int  password_count = 0;
    char ch;

    int readed=0;
    do {
        int res = recv(client_socket, &ch, 1, 0);
        if(res!=1) break;
        buf[readed++] = ch;
    } while(ch!=0 || readed<=6);
    if(readed>0) {
        for(i=6;i<readed;i++) {
            if (buf[i]=='\t') {
                field++;
                continue;
            }
            switch(field) {
                case 0 : break;
                case 1 : type[type_count++]=buf[i]; break;
                case 2 : username[user_count++]=buf[i]; break;
                case 3 : password[password_count++]=buf[i]; break;
            }
        }

        // Send reply to the client
        char val;
        val = 0;
        send(client_socket,&val,1,0);
        val = DATA_FINAL_PACKET;
        send(client_socket,&val,1,0);
        val = 0;
        send(client_socket,&val,1,0);
        char text[1024];
        char prompt[2];
        prompt[0]= '%';
        prompt[1]=0;
        sprintf(text,"Proxy: User <%s> was connected to the model server.\nReady to process requests.\n%s",username,prompt);
        SendInteger(client_socket,strlen(text)+1);
        sendSafe(client_socket,text,strlen(text)+1);
    }
    type[type_count]=0;
    username[user_count]=0;
    password[user_count]=0;
    if (type_count == 0) {
        client_type = -1;
    } else {
        client_type = 0;
        if(strcmp(type,"dislite")==0) client_type = 1;
    }
}



int SendStop(int socket) {
unsigned char num = 0;
unsigned char type = 4;
char* cmd = "stop_server -y";
char tmp = 0;


    int success  = send(socket, (const char *)&num, 1, 0);
    if(success <= 0) return 0;
    success = send(socket, (const char *)&type, 1, 0);
    if(success <= 0) return 0;
    success = SendInteger(socket, strlen(cmd) + 1);
    if(success <= 0) return 0;
    success = SendBuffer(socket, cmd, strlen(cmd));
    if(success <= 0) return 0;
    success = send(socket, &tmp, 1, 0);
    if(success <= 0) return 0;
        return 1;
}
//-----------------------------------------------------------------------------

//------------------------------------------------------------------------------
// This function will run every time the new client try to connect to 
// the server administrator. It will create socket connection between the 
// client and the currently running pmod server and will start client log.
//------------------------------------------------------------------------------
void ProcessClientRequest(int client_socket, sockaddr *addr) {

    // This function will try to connect to the model server
    int model_socket = ConnectToModelServer();

    // If attempt to connect to the model server fails, we will restart 
    // our model server.
    if(model_socket == -1) {
        DBG(fprintf(stderr, "Server dead. Restarting ..."); fflush(stderr));
        if(RestartModelServer()==0) {
            DBG(fprintf(stderr, "\nFailed.\nCan't restart server. Quitting...\n"); fflush(stderr));
            AbortService();
        }
        return;
    }

    // Create new record in the clients records list. This record will 
    // be used to read data from the client, send this data to the 
    // server, read server reply and send it back to the client. 
    int model_port  = model_connection_descriptor.sin_port;
    int model_tcp   = model_connection_descriptor.sin_addr.s_addr;
    int client_port = ((sockaddr_in *)addr)->sin_port;
    int client_tcp  = ((sockaddr_in *)addr)->sin_addr.s_addr;

    // Login
    // Format: login\t type \t user
    int client_type;
    char user[500];
    char password[500];
    GetLogin(client_socket,client_type,user,password);

    // For GUI client we will need to source startup script
    if(client_type == 1) SourceDisliteStartupScript(model_socket);

    // We will not use password right now, but we may in future
    CreateNewClient(ID_Counter++,
                    client_socket,
                    client_port,
                    client_tcp,
                    model_socket,
                    model_port,
                    model_tcp,
                    client_type,
                    user);
    DBG(fprintf(stderr, "New client connected.\n"); fflush(stderr));
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// This function will run every time the new administrator try to 
// connect to the server. It will process administrator input and will 
// execute all administration instructions.
//------------------------------------------------------------------------------
void ProcessAdminRequest(int socket, sockaddr *addr) {
    int port = ((sockaddr_in *)addr)->sin_port;
    int tcp  = ((sockaddr_in *)addr)->sin_addr.s_addr;
    DBG(fprintf(stderr, "Creating new admin ...\n"); fflush(stderr));
    CreateNewAdmin( "admin",socket,port,tcp);                           
    DBG(fprintf(stderr, "New administrator connected.\n"); fflush(stderr));
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// This function will read block of data from the client and will 
// forward it to the server. If all data forwarded we are all set and 
// client socket will not reply that there is some data to read. If the 
// data is too big to fit in block the rest of data will be translated 
// on the next iteration.
//------------------------------------------------------------------------------
int TranslateOneQuery(int client,int server, char** cmd, int& cmd_len) {
    static char buf[2000];
    int readed;

    DBG(fprintf(stderr, "Translating query..."); fflush(stderr));
    // Redirecting query from the client to the server
    int len = 2000; 
    readed = recv(client, buf, len, 0);        
    if(readed <= 0) return 0;
    cmd_len = readed;
    sendSafe(server,buf,readed);
    *cmd = (char *)malloc(cmd_len);
    memcpy(*cmd,buf,cmd_len);
    DBG(fprintf(stderr, "Done.\n"); fflush(stderr));
    return 1;
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int Resend(int server, char* buf, int len) {
    DBG(fprintf(stderr, "Resending command..."); fflush(stderr));
    int b = sendSafe(server,buf,len);
    DBG(fprintf(stderr, "Done.\n"); fflush(stderr));
    return 1;
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// This function will read block of data from the server and will forward it to 
// the client. If all data forwarded we are all set and server socket will not
// reply that there is some data to read. If the data is too big to fit in block
// the rest of data will be translated on the next ineration.
//------------------------------------------------------------------------------
int TranslateOneReply(int client,int server) {
    char buf[2000];
    int readed;

    DBG(fprintf(stderr, "Translating reply..."); fflush(stderr));
    // Redirecting query from the client to the server
    int len = 2000;     
    readed = recv(server, buf, len, 0);
    // Client closed the connection - we will remove it from the clients list
    if(readed<=0) return 0;
    sendSafe(client,buf,readed);
    DBG(fprintf(stderr, "Done.\n"); fflush(stderr));
    return 1;
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// This function will parse the administrator command and will return 
// command code + will set param pointer to point to the optional parameter.
//------------------------------------------------------------------------------
int AdminParseCommand(char* command, void** param) {
    char* p;
    static char parsed[10][100];
    static int cliID;
    int cmd_count=0;
    int i=0;

    p=command;
    while (*p!=0) {
        if(*p==' ') {
            if(cmd_count<9) {
                parsed[cmd_count][i]=0;
                cmd_count++;
                while(*p==' ') p++;
                i=0;
                continue;
            }
        }
        if(i<99)
        parsed[cmd_count][i++]=*p;
        p++;
    }
    parsed[cmd_count][i]=0;
    cmd_count++;

    if(strcmp(parsed[0],"reboot")==0) {
        return REBOOT;
    }

    if(strcmp(parsed[0],"list")==0) {
        if(cmd_count==1) {
            return ALL_LIST;
        } else {
            if((strcmp(parsed[1],"-host")==0) && (cmd_count>2)) {
                *param = (void *)parsed[2];
                return HOST_LIST;
            }
            if((strcmp(parsed[1],"-user")==0) && (cmd_count>2)) {
                *param = (void *)parsed[2];
                return USER_LIST;
            }
            *param = (void *)parsed[1];
            return USER_LIST;
        }
    }

    if(strcmp(parsed[0],"count")==0) {

        if(cmd_count==1) {
            return ALL_COUNT;
        } else {
            if((strcmp(parsed[1],"-host")==0) && (cmd_count>2)) {
                *param = (void *)parsed[2];
                return HOST_COUNT;
            }
            if((strcmp(parsed[1],"-user")==0) && (cmd_count>2)) {
                *param = (void *)parsed[2];
                return USER_COUNT;
            }
            *param = (void *)parsed[1];
            return USER_COUNT;
        }
    }

    if(strcmp(parsed[0],"stop")==0) {
        if(cmd_count==1) {
            return ALL_STOP;
        } else {
            if((strcmp(parsed[1],"-host")==0) && cmd_count>2) {
                *param = (void *)parsed[2];
                return HOST_STOP;
            }
            if((strcmp(parsed[1],"-user")==0) && cmd_count>2) {
                *param = (void *)parsed[2];
                return USER_STOP;
            }
            cliID= atoi(parsed[1]);
            *param = (void *)&cliID;
            return CLIENT_STOP;
        }
    }
    return UNKNOWN;
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void AdminReportClientsAmount(int admin) {
char buf[100];

    ClientDescriptor *cli = ClientDescriptor::sd_list;
    int count=0;
    while(cli != NULL){
        count++;
        cli = cli->m_Next;
    }
    sprintf(buf,"%d\n",count);
    sendSafe(admin,buf,strlen(buf));
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void AdminReportUserClientsAmount(int admin,char* user) {
char buf[100];

    ClientDescriptor *cli = ClientDescriptor::sd_list;
    int count=0;
    while(cli != NULL){
        if(strcmp(cli->m_UserName,user)==0)  count++;
        cli = cli->m_Next;
    }
    sprintf(buf,"%d\n",count);
    sendSafe(admin,buf,strlen(buf));
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
void AdminReportHostClientsAmount(int admin,char* host) {
    char buf[100];
    struct hostent * hostdescr;
    int val;

    ClientDescriptor *cli = ClientDescriptor::sd_list;
    int count=0;
    while(cli != NULL){
        val = cli->m_ClientTCP;
        hostdescr = gethostbyaddr((const char*)&val,4,AF_INET);
        if(strcmp(hostdescr->h_name,host)==0)  count++;
        cli = cli->m_Next;
    }
    sprintf(buf,"%d\n",count);
    sendSafe(admin,buf,strlen(buf));
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void AdminReportClientsList(int admin) {
    char buf[1000];
    char reply[10000];
    struct hostent * hostdescr;

    ClientDescriptor *cli = ClientDescriptor::sd_list;
    int count=0;
    reply[0]=0;
    strcpy(reply,"ID\tHOST\t\tUSER\n");
    while(cli != NULL){
        count++;
        int val;
        val = cli->m_ClientTCP;
        hostdescr = gethostbyaddr((const char*)&val,4,AF_INET);
        if ( hostdescr!=NULL) {
            sprintf(buf,"%d\t%s\t%s",cli->m_ID,hostdescr->h_name,cli->m_UserName);
        } else {
            sprintf(buf,"%d\tunknown\t%s",cli->m_ID,cli->m_UserName);
        }
        strcat(reply,buf);
        strcat(reply,"\n");
        cli = cli->m_Next;
    }
    sendSafe(admin,reply,strlen(reply));
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void AdminReportUserClientsList(int admin, char* user) {
    char buf[100];
    char reply[10000];
    struct hostent * hostdescr;

    ClientDescriptor *cli = ClientDescriptor::sd_list;
    int count=0;
    reply[0]=0;
    strcpy(reply,"ID\tHOST\t\tUSER\n");
    while(cli != NULL){
        if( strcmp(cli->m_UserName,user)==0) {
           count++;
           int val;
           val = cli->m_ClientTCP;
           hostdescr = gethostbyaddr((const char*)&val,4,AF_INET);
           sprintf(buf,"%d\t%s\t%s",cli->m_ID,hostdescr->h_name,user);
           strcat(reply,buf);
           strcat(reply,"\n");
        }
        cli = cli->m_Next;
    }
    sendSafe(admin,reply,strlen(reply));
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void AdminReportHostClientsList(int admin, char* host) {
    char buf[100];
    char reply[10000];
    struct hostent * hostdescr;

    ClientDescriptor *cli = ClientDescriptor::sd_list;
    int count=0;
    reply[0]=0;
    strcpy(reply,"ID\tHOST\t\tUSER\n");
    while(cli != NULL){
        int val;
        val = cli->m_ClientTCP;
        hostdescr = gethostbyaddr((const char*)&val,4,AF_INET);
        if( strcmp(hostdescr->h_name,host)==0) {
           count++;
           sprintf(buf,"%d\t%s\t%s",cli->m_ID,hostdescr->h_name,cli->m_UserName);
           strcat(reply,buf);
           strcat(reply,"\n");
        }
        cli = cli->m_Next;
    }
    sendSafe(admin,reply,strlen(reply));
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// This function will remove client with the given ID.
//------------------------------------------------------------------------------
void AdminRemoveClient(int admin, int id) {
    char reply[10000];


    if(ClientDescriptor::LookupID(id)==NULL) {
        sprintf(reply,"ID=%d - no such client.\n",id);
    } else {
        ClientDescriptor::RemoveClient(id);
        sprintf(reply,"Client %d disconnected from the server.\n",id);
    }
    sendSafe(admin,reply,strlen(reply));
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// This function is called by administrator to disconnect all clients invoked
// by the specified user
//------------------------------------------------------------------------------
void AdminRemoveUserClients(int admin, char* user) {
    char reply[10000];
    char buf[100];

    ClientDescriptor *cli = ClientDescriptor::sd_list;
    reply[0]=0;
    while(cli != NULL){
        if(strcmp(cli->m_UserName,user)==0) {
            sprintf(buf,"Client %d disconnected.\n",cli->m_ID);
            strcat(reply,buf);
            ClientDescriptor::RemoveClient(cli->m_ID);
            cli = ClientDescriptor::sd_list;
            continue;
        }
        cli = cli->m_Next;
    }
    sendSafe(admin,reply,strlen(reply));
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// This function is called by administrator to disconnect all clients 
// operating from the specified host (if this host is going to shut 
// down, for example)
//------------------------------------------------------------------------------
void AdminRemoveHostClients(int admin, char* host) {
    char reply[10000];
    char buf[100];
    struct hostent * hostdescr;

    ClientDescriptor *cli = ClientDescriptor::sd_list;
    reply[0]=0;
    while(cli != NULL){
        hostdescr = gethostbyaddr((char *)&cli->m_ClientTCP,4,AF_INET);
        if(strcmp(hostdescr->h_name,host)==0) {
            sprintf(buf,"Client %d disconnected.\n",cli->m_ID);
            strcat(reply,buf);
            ClientDescriptor::RemoveClient(cli->m_ID);
            cli = ClientDescriptor::sd_list;
            continue;
        }
        cli = cli->m_Next;
    }
    sendSafe(admin,reply,strlen(reply));
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// This function is called if administrator sends wrong command in 
// administrator socket channel.
//------------------------------------------------------------------------------
void AdminUnknown(int admin) {
    char* message = "Unknown command.\nAvailable commands:\nlist [-user <name> | -host<name>]\ncount\nreboot\nstop [-user <name> | -host <name>]\nquit\n";
    sendSafe(admin,message,strlen(message));
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// This function is called by administrator using administrator socket 
// channel to restart server.
//------------------------------------------------------------------------------
void AdminReboot(int admin) {
    char* message1 = "Rebooting server.\n";
    sendSafe(admin,message1,strlen(message1));
    RestartModelServer();
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// This function will process single query from the administrator channel.
//------------------------------------------------------------------------------
int ProcessAdministratorCommand(int admin) {
    char buf[1025];
    void* param;
    int readed;

    DBG(fprintf(stderr, "Processing administrator request..."); fflush(stderr));
    int len = 1024;     
    readed = recv(admin, buf, len, 0);
        // Administrator closed the connection - we will remove it from the clients list
    if(readed<=0) return 0;

    buf[readed]=0;
    switch(AdminParseCommand(buf,&param)) {
        // Statistics
        case ALL_COUNT  :
            AdminReportClientsAmount(admin);
            break;
        case USER_COUNT :
            AdminReportUserClientsAmount(admin,(char *) param);
            break;
        case HOST_COUNT :
            AdminReportHostClientsAmount(admin,(char *) param);
            break;
        // List of the clients connected
        case ALL_LIST   :
            AdminReportClientsList(admin);
            break;
        case USER_LIST  :
            AdminReportUserClientsList(admin,(char *) param);
            break;
        case HOST_LIST  :
            AdminReportHostClientsList(admin,(char *) param);
            break;
        // Stops clients or all server
        case ALL_STOP   :
            AbortService();
            break;
        case CLIENT_STOP:
            AdminRemoveClient(admin,*((int *)param));
            break;
        case USER_STOP  :
            AdminRemoveUserClients(admin,(char *) param);
            break;
        case HOST_STOP  :
            AdminRemoveHostClients(admin,(char *) param);
            break;
        // Restarts the server
                case REBOOT     :
            AdminReboot(admin);
            break;
        default         :
            AdminUnknown(admin);
            break;
    }
    return 1;
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// This function will inspect clients list in order to determine 
// which clients need to query server. The data will be forwarded from 
// the client socket into model server socket and reply will be 
// forwarded from model server socket to the client socket.
//------------------------------------------------------------------------------
void ProcessClients(void) {
    ClientDescriptor *cur = ClientDescriptor::sd_list;
    while(ClientDescriptor::sd_list!=NULL && cur != NULL) {
        if(CheckSocket(cur->m_ClientSocket)) {
            // If some client send a request we need to forward this request
            // to the server
            if(cur->m_LastCommand!=NULL) {
                free(cur->m_LastCommand);
                cur->m_LastCommand=NULL;
            }
            if(TranslateOneQuery(cur->m_ClientSocket,cur->m_ServerSocket,&(cur->m_LastCommand),cur->m_LastCommandLength)==0) {
                // If an error was generated during the attempt to read
                // from the client socket we will remove this client
                // from the clients list and we will close connected
                // server port
                ClientDescriptor::RemoveClient(cur);
                return;
            } else {
                cur->m_Processing     = 1;
                cur->m_Timeout        = 0;
            }
        }
        if(CheckSocket(cur->m_ServerSocket)) {
            // If server send reply to this client request we need to
            // forward this reply to the client
            if(TranslateOneReply(cur->m_ClientSocket,cur->m_ServerSocket)==0) {
                // If an error was generated during the attempt to
                // read the data from the server this means that the server
                // is dead and we need to restart it.
                DBG(fprintf(stderr, "Server dead. Restarting ..."); fflush(stderr));
                if(RestartModelServer() == 0) {
                    DBG(fprintf(stderr, "Fail.\n Can't restart server. Quitting...\n"); fflush(stderr));
                    AbortService();
                }
                DBG(fprintf(stderr, "Done.\n"); fflush(stderr));
                return;
            }
            cur->m_Processing     = 0;
            cur->m_Timeout        = 0;
        }
        if(cur->m_Processing==1 && TimeoutValue!=-1) {
            if(cur->m_Timeout == TimeoutValue) {
                DBG(fprintf(stderr, "Server not responding. Restarting ..."); fflush(stderr));
                // Remove current client 
                ClientDescriptor::RemoveClient(cur);
                if(RestartModelServer() == 0) {
                    DBG(fprintf(stderr, "Fail.\n Can't restart server. Quitting...\n"); fflush(stderr));
                    AbortService();
                }
                DBG(fprintf(stderr, "Done.\n"); fflush(stderr));
                return;
            }
            cur->m_Timeout++;
        }
        cur = cur->m_Next;
    }
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// This function will process all administrator requests
//------------------------------------------------------------------------------
void ProcessAdministrators(void) {

    AdminDescriptor *cur = AdminDescriptor::admin_list;
    while(cur != NULL){
                if(CheckSocket(cur->m_Socket)) {
                        // If some client send a request we need to forward this request
                        // to the server
                        if(ProcessAdministratorCommand(cur->m_Socket)==0) {
                                // If an error was generated during the attempt to read
                                // from the admin socket we will remove this administrator
                                // from the administrators list and we will colse connected
                                // server port
                            AdminDescriptor::RemoveAdmin(cur);
                                return;
                                
                        }
        }
            cur = cur->m_Next;
    }
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// This function will register the external service and reconnect any
// connected clients once the model server has come up.
//------------------------------------------------------------------------------
int RegisterServer (int clients_socket) {
    struct sockaddr_in assigned;
    int len = sizeof(assigned);
    if(getsockname(clients_socket, (struct sockaddr *)&assigned, &len) == 0){
        NameServerRegisterService(ExternalServiceName(), ntohs(assigned.sin_port));
        server_is_ready(1);

        int service_port, tcp_addr;
        NameServerGetService(InternalServiceName(), service_port, tcp_addr);
        model_connection_descriptor.sin_family      = AF_INET;
        model_connection_descriptor.sin_port        = htons(service_port);
        model_connection_descriptor.sin_addr.s_addr = htonl(tcp_addr);

        ClientDescriptor *cur = ClientDescriptor::sd_list;
        while(cur != NULL) {
            // Reconnect each client to the new model server
            int model_port      = model_connection_descriptor.sin_port;
            int model_tcp       = model_connection_descriptor.sin_addr.s_addr;
            closesocket(cur->m_ServerSocket);
            // Connect to this new copy
            int model_socket = ConnectToModelServer();
            // Attempt to connect to the server fails.
            if(model_socket < 0 ) {
                DBG(fprintf(stderr, "Can't connect!\n"); fflush(stderr));
                return 0;
            }
            cur->m_ServerSocket = model_socket;
            cur->m_ServerPort   = model_port;
            cur->m_ServerTCP    = model_tcp;
            cur->m_Timeout      =0;
            if(cur->m_Type == 1) {
                SourceDisliteStartupScript(model_socket);
            }
            if(cur->m_Processing==1) {
                DBG(fprintf(stderr, "Resending last command.\n"); fflush(stderr));
                Resend(model_socket,cur->m_LastCommand,cur->m_LastCommandLength);
            }
            cur = cur->m_Next;
        }
        return 1;
    } else {
        return 0;
    }
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Loop while not shutting down. Will listen clients socket and 
// administrator socket, will process clients and administrator 
// requests if any.
//------------------------------------------------------------------------------
void RequestsLoop(int admin_socket, int clients_socket) {
    struct sockaddr s;
    int s_size = sizeof(s);

    do {
        if (!server_is_ready()) {
            switch (check_server_status()) {
                case 1:
                    break;
                case 2:
                    if (RegisterServer(clients_socket) == 0) {
                        fprintf(stderr, "Unable to register model service.\n");
                        fflush(stderr);
                        AbortService();
                    }
                    break;
                case -1: case 0: default:
                    fprintf(stderr, "Unable to start model server.\n");
                    fflush(stderr);
                    AbortService();
                    break;
            }
        }
        WaitForAnyInput(clients_socket, admin_socket);
        if (server_is_ready() && CheckSocket(clients_socket)) {
            int cli_connection = accept(clients_socket, &s, &s_size);
            int tmp_switch = 1;
            setsockopt(cli_connection, IPPROTO_TCP, TCP_NODELAY, (char *)&tmp_switch, sizeof(tmp_switch));
            if(cli_connection >= 0) {
                DBG(fprintf(stderr, "Accepting client request\n"); fflush(stderr));
                ProcessClientRequest(cli_connection, &s);
            }
        }
        // Checking for the administrator requests
        if(CheckSocket(admin_socket)) {
            int admin_connection = accept(admin_socket, &s, &s_size);
            int tmp_switch = 1;
            setsockopt(admin_connection, IPPROTO_TCP, TCP_NODELAY, (char *)&tmp_switch, sizeof(tmp_switch));
            if(admin_connection >= 0) {
                DBG(fprintf(stderr, "Accepting administrator request\n"); fflush(stderr));
                ProcessAdminRequest(admin_connection, &s);
            }
        }
        // Processing requests from administrators
        ProcessAdministrators();

        // Performing data translstion from the existing client sockets
        // to the existing model sockets
        if (server_is_ready()) ProcessClients();

    } while(!do_shutdown);
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
// This function will parse arguments and will fill this global variables:
// pdf_str - array of paths to PDF files
// prefs_str - the path and name of the PREFS file
// service_str - the visible name of the clients service
// admin_str - the visible name of the administrators service
//------------------------------------------------------------------------------
int ParseArguments(int argc, char **argv) {
    int i = 1;
    while(i < argc) {

        if(strcmp(argv[i], "help") == 0)  return 0;
        if(strcmp(argv[i], "-help") == 0) return 0;

        if(strcmp(argv[i], "-timeout") == 0 && i + 1 < argc) {
            TimeoutValue = atoi(argv[i+1]);
            i++;
        }
        if(strcmp(argv[i], "-logfile") == 0 && i + 1 < argc) {
            logfile_str = strdup(argv[i+1]);
            i++;
        }
        if(strcmp(argv[i], "-pdf") == 0) {
            while (i+1 < argc) {
                if (*(argv[i+1]) == '-' || strcmp(argv[i+1], "help") == 0) {
                    break;
                }
                else {
                    if (num_pdfs < 1024) {
                        pdf_str[num_pdfs] = strdup(argv[i+1]);
                        num_pdfs++;
                    }
                    i++;
                }
            }
        }
        if(strcmp(argv[i], "-prefs") == 0 && i + 1 < argc) {
            prefs_str = strdup(argv[i+1]);
            i++;
        }
        if(strcmp(argv[i], "-service") == 0 && i + 1 < argc) {
            service_str = strdup(argv[i+1]);
            i++;
        }
        if(strcmp(argv[i], "-admin") == 0 && i + 1 < argc) {
            admin_str = strdup(argv[i+1]);
            i++;
        }
        if(strcmp(argv[i], "-debug") == 0) {
            do_debug = 1;
        }

        i++;
    }
    if(i>1) return 1;
    else return 0;
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// This function will start model server with given pdf, prefs and service name
//------------------------------------------------------------------------------
int StartModelServer(void) {
    char *cmd;
    const char **argv;
    int service_port;
    int tcp_addr;
    char *execDir;
    char *p;
    char *service_name;
    char *full_logfile_str = NULL;

    if(NameServerGetService(InternalServiceName(), service_port, tcp_addr)==1) {
        model_connection_descriptor.sin_family      = AF_INET;
        model_connection_descriptor.sin_port        = htons(service_port);
        model_connection_descriptor.sin_addr.s_addr = htonl(tcp_addr);
        return 2;
    }

    server_is_ready(0);
    p=getenv("PSETHOME");
    execDir = new char[(p!=NULL ? strlen(p) : 0) + 10];
    if(p!=NULL) {
        sprintf(execDir,"%s/bin",p);
    } else {
        sprintf(execDir,"");
    }

    cmd = new char[strlen(execDir) + 20];
    sprintf(cmd, "%s/model_server", execDir);

    argv = new const char *[num_pdfs + 20];
    int argn = 0;

    if (num_pdfs == 0 && prefs_str == NULL) {
        service_name=InternalServiceName();
    } else {
        service_name=StartupServiceName();
    }

    argv[argn++] = "-service";
    argv[argn++] = service_name;

    if (num_pdfs > 0) {
                argv[argn++] = "-pdf";
                for (int i=0; i<num_pdfs; i++) {
                        argv[argn++] = pdf_str[i];
                }
    } 

    if (prefs_str != NULL) {
                argv[argn++] = "-prefs";
                argv[argn++] = prefs_str;
    }

    if(logfile_str!=NULL) {
	    argv[argn++] = "-log_file";
	    full_logfile_str = new char[strlen(logfile_str) + 1 + strlen(service_name) + 1];
	    strcpy(full_logfile_str, logfile_str);
	    strcat(full_logfile_str, ".");
	    strcat(full_logfile_str, service_name);
	    argv[argn++] = full_logfile_str;
    }

    argv[argn] = NULL;

    int result = (start_new_server(cmd, argv)!=0);
    delete [] argv;
    delete [] cmd;
    delete [] execDir;
    delete [] full_logfile_str;
    return result;
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int InitialStartup(void) {
int i;
int service_port;
int tcp_addr;

    // Model server is already running
    if(NameServerGetService(ExternalServiceName(), service_port, tcp_addr)==1) {
        return 2;
    }
    // Server spy detected the hidden service which PID is unknown.
    if(NameServerGetService(InternalServiceName(), service_port, tcp_addr)==1) {
        DBG(fprintf(stderr, "Old model server detected. Shutting down..."); fflush(stderr));
	model_connection_descriptor.sin_family      = AF_INET;
	model_connection_descriptor.sin_port        = htons(service_port);
	model_connection_descriptor.sin_addr.s_addr = htonl(tcp_addr);
	int socket = ConnectToModelServer();
	if(socket<0) {
	    DBG(fprintf(stderr, "Can't connect.\n")); 
	    fflush(stderr);
	    return 3; // Cannot connect to the model server
	}
	if(SendStop(socket)==0) {
	    DBG(fprintf(stderr, "Can't send stop command.\n"));
	    fflush(stderr);
	    return 3; // Cannoot stop server
	}
	for(i=0;i<1000;i++)
	    if(NameServerGetService(InternalServiceName(),service_port,tcp_addr)!=1) break;
	if(i==1000) {
	    DBG(fprintf(stderr, "Timeout.\n")); 
	    fflush(stderr);
	    return 3; 
	}
        DBG(fprintf(stderr, "Done\n")); 
	fflush(stderr);
    }
    return StartModelServer();

}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
// This function will start model server with given pdf, prefs and service name
//------------------------------------------------------------------------------
int RestartModelServer(void) {
    // Start new copy of the model server
    if(current_server!=NULL) terminate_service_process(current_server);
        ClientDescriptor::RemoveStateClients();
    return StartModelServer();
}
//------------------------------------------------------------------------------

void PrintHelp(void) {
    fprintf(stderr, "Discover model server proxy.\n\n");
    fprintf(stderr, "Available options:\n");
    fprintf(stderr, " -pdf     <filename>  - pdf file name\n");
    fprintf(stderr, " -prefs   <filename>  - prefs file name\n");
    fprintf(stderr, " -timeout <seconds,-1 = infinite> - single command timeout\n");
    fprintf(stderr, " -service <name> - service name\n");
    fprintf(stderr, " -admin   <name> - administrator service name (admintool.exe)\n");
    fprintf(stderr, " -debug\n");
    fflush(stderr);
}

int main(int argc, char **argv) 
{
#ifdef _WIN32
    WORD    wVersionRequested;  
    WSADATA wsaData; 
    int     err; 
   
    wVersionRequested = MAKEWORD(1, 1); 
    err               = WSAStartup(wVersionRequested, &wsaData);
    if (err != 0) {
        fprintf(stderr, "No sockets available.\n");
        fflush(stderr);
        return -1;
    }
#endif 
    int admin_socket, clients_socket;
    if(ParseArguments(argc, argv)==0) {
        PrintHelp();
        return 1;
    }
    if(do_stop) AbortService();
    else {
        // Clients channel
        clients_socket = CreateListener();
        if(clients_socket < 0) {
            fprintf(stderr, "Unable to create clients requests socket.\n");
            fflush(stderr);
            return -1;
        }

        // Administrators channel
        admin_socket = CreateListener();
        if(admin_socket < 0) {
            fprintf(stderr, "Unable to create administrator requests socket.\n");
            fflush(stderr);
            return -1;
        }

        // First, make sure that nameserv is up and accepting connections
        int test_sock = ConnectToNameServer();
        if (test_sock < 0) {
            // Couldn't connect, so try to start nameserv
            if(start_name_server()!=0) {
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
            fprintf(stderr, "Unable to contact or start nameserver.  Please run nameserv by hand.\n");
            fflush(stderr);
            return -1;
        }

        switch(InitialStartup()) {
            case 0 :
                fprintf(stderr, "Unable to start model server.\n");
                fflush(stderr);
                return 0;
            case 1 :
                fprintf(stderr, "New model server starting up.\n");
                fflush(stderr);
                break;
            case 2 :
                fprintf(stderr, "Model server is already running.\nExiting.\n");
                fflush(stderr);
                return 0;
            case 3 :
                fprintf(stderr, "Dead model server detected which can't be killed by serverspy.\nExiting.\n");
                fflush(stderr);
                return 0;
        }

        // Next, register administrators service
    struct sockaddr_in admin;
    int len = sizeof(admin);
        int res = -1;
        if(getsockname(admin_socket, (struct sockaddr *)&admin, &len) == 0) {
            res = NameServerRegisterService(AdminName(), ntohs(admin.sin_port));
        }
        if (res == 1) {
            DBG(fprintf(stderr, "Successfully registered admin service.\n"); fflush(stderr));
        } else {
            fprintf(stderr, "Unable to register admin service.\n");
            fflush(stderr);
            return -1;
        }


        // Requests loop
        DBG(fprintf(stderr, "Ready to process requests.\n"); fflush(stderr));
        RequestsLoop(admin_socket, clients_socket);
        DBG(fprintf(stderr, "Terminating application.\n"); fflush(stderr));
        terminate_service_process(current_server);
        if (clients_socket != 0) closesocket(clients_socket);
        if (admin_socket != 0)   closesocket(admin_socket);
    }
    return 0;
}

