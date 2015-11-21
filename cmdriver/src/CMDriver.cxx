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
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#include <winbase.h>
#include <winsock.h>
#include <ws2tcpip.h> // for socklen_t
#include <io.h>
#else
#include <errno.h>
#include <netdb.h>
#include <signal.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#ifdef sun5
#include <poll.h>
#include <time.h>
#include <sys/time.h>
#include <sys/systeminfo.h>
#endif
#ifdef irix6
#include <sys/poll.h>
#include <sys/systeminfo.h>
#endif
#ifdef hp700
#include <sys/poll.h>
#include <time.h>
extern "C" int select(int, int*, int*, int*, const struct timeval*);
#endif
#endif

#include "nameServCalls.h"

#include "debug.h"
#include "startproc.h"

#ifdef XXX_enable_xerces_parsing
#include "CmXml.h"
#include "CmXmlException.h"
#else
class CmXml {
public:
    CmXml(char const*) {}
    char *getCmSystems() { return dup("no_cm"); }
    bool isCmSystem(char const *) { return true; }

    char *getAttributes(char const*) { return dup(""); }
    char *getCommands(char const*) { return dup(""); }
    bool isAttribute(char const*, char const*) { return true; }
    char *translateCommand(char const*, char *[], int * = 0) { return NULL; }
    char *translateResult(char const*, char const*, char const *reply) { return dup(reply);}
    bool isCommand(char const *, char const *) { return true; }
    bool isReturnable(char const *, char const *) {return false; }
    void checkConsistency() {}
private:
    char *dup(char const *s) {
        char *res = new char[strlen(s) + 1];
        return strcpy(res, s);
    }
};

class CmXmlException {
public:
    char const* getMessage() const {return "unlikely";}
};
#endif

#include "CmXmlStringTokenizer.h"


int		do_debug = 0;
FILE*	log_fd;
char	dbgFileName[1024];

static int nClients = 0;

#ifndef _WIN32
#define closesocket(sock) close(sock)
#endif

// This is pointer to the object,
//    which holds all information concerning work with 'cm.xml'.
static CmXml *cmXml = NULL;

static const	int m_DriversNum = 3;

// this is types of CM realized in this driver
static const	int CLEARCASE = 0;
static const	int SI = 1;
static const	int GENERIC = 2;

static const char*	m_DriversSuffixes[m_DriversNum] = {
    "CM:ClearCase",
    "CM:Source Integrity",
    "CM:Generic"
};

char m_DriversNames[m_DriversNum][1024];

typedef struct {
    //				int m_DriversAddress[m_DriversNum];
    //				int m_DriversPort[m_DriversNum];
    int m_DriversListeningSocket[m_DriversNum];
} CConnectionDescriptor;
static CConnectionDescriptor Connection;
static int do_shutdown = 0;
static int work_forever = 0;

static const char *REGISTER_CMD = "register";
static const char *UNREGISTER_CMD = "unregister";
static const char *UNCHECKOUT_CMD = "uncheckout";
static const char *CHECKOUT_CMD = "checkout";
static const char *CHECKIN_CMD = "checkin";
static const char *RESERVE_CMD = "reserve";
static const char *RESYNC_CMD = "resync";
static const char *UNRESERVE_CMD = "unreserve";
static const char *LOCALLIST_CMD = "lsco";


//--------------------------------------------------------------------------------------------
// This procedure will create a listen socket for client requests channel
//--------------------------------------------------------------------------------------------
int CreateListener() {
    int sock;
    struct sockaddr_in name;
    
    // Create the socket. 
    sock = socket (PF_INET, SOCK_STREAM, 0);
    if (sock < 0)
        return -1;
    
    unsigned int set_option = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (const char *)&set_option, sizeof(set_option));

    // Give the socket a name. 
    name.sin_family      = AF_INET;
    name.sin_port        = 0;
    name.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind (sock, (struct sockaddr *) &name, sizeof (name)) < 0)
        return -1;
    if(listen(sock, 5) < 0)
        return -1;
    return sock;
}
//--------------------------------------------------------------------------------------------

//############################################################################################
// This class represents the client record in the clients linked list.
// Any client record contain client name, client TCP address and port and server TCP 
// address and port.
//############################################################################################
class ClientDescriptor {
public:
    ClientDescriptor(int id, 
                     int client_socket,
                     int client_port,
                     int client_tcp_addr,
                     int client_type);

    ~ClientDescriptor();
    static void             AddNewClient(ClientDescriptor *sd);
    static void             RemoveClient(int id);
    static void             RemoveClient(ClientDescriptor *p);
    static void             RemoveAllClients(void);
    static ClientDescriptor *LookupID(int id);
    int   m_ID;
    int   m_ClientSocket;
    int   m_ClientPort;
    int   m_ClientTCP;
    int   m_ClientType;
    ClientDescriptor *m_Next;
    static ClientDescriptor *sd_list;
    static ClientDescriptor *sd_list_last;

    // Support for CM system's name, how 'CM:Generic' interface understands it.
public:
    char*	getCmName();
    void	setCmName(char *cm_name);

private:
    char	*m_cmName;

    // Textual description of this client
public:
    char*   getDescription();

private:
    char	*m_Description;

};

ClientDescriptor *ClientDescriptor::sd_list      = NULL;
ClientDescriptor *ClientDescriptor::sd_list_last = NULL;
//------------------------------------------------------------------------------------------------------
// Constructor will create new client with the client name
//------------------------------------------------------------------------------------------------------
ClientDescriptor::ClientDescriptor(int id, 
                                   int client_socket,
                                   int client_port, 
                                   int client_addr,
                                   int client_type) 
{
    this->m_ID             = id;
    this->m_ClientSocket   = client_socket;
    this->m_ClientPort     = client_port;
    this->m_ClientTCP      = client_addr;
    this->m_ClientType     = client_type;
    this->m_Next           = NULL;
    this->m_cmName         = NULL;
    this->m_Description    = NULL;

}
//------------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------------
// Destructor will free the space allocated for the new client name
//------------------------------------------------------------------------------------------------------
ClientDescriptor::~ClientDescriptor() {
    free(this->m_cmName);
    free(this->m_Description);
}
//------------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------------
// This method will add new client to the clients linked list
//------------------------------------------------------------------------------------------------------
void ClientDescriptor::AddNewClient(ClientDescriptor *sd) {
    if(sd_list == NULL){
        sd_list      = sd;
        sd_list_last = sd;
    } else {
        sd_list_last->m_Next = sd;
        sd_list_last       = sd;
    }
}
//------------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------------
// This function will return a pointer to the client record in the clients linked list
// or NULL of no client with the given name found.
//------------------------------------------------------------------------------------------------------
ClientDescriptor *ClientDescriptor::LookupID(int id)
{
    ClientDescriptor *cur = sd_list;
    while(cur != NULL){
        if(cur->m_ID==id) return cur;
        cur = cur->m_Next;
    }
    return NULL;
}
//------------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------------
// This function will remove the client with the given name.
//------------------------------------------------------------------------------------------------------
void ClientDescriptor::RemoveClient(int id) {
    ClientDescriptor *prev = NULL;
    ClientDescriptor *cur  = sd_list;
    while(cur != NULL) {
        if(cur->m_ID == id) {
            closesocket(cur->m_ClientSocket);
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
//------------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------------
// This function will remove client with the given address
//------------------------------------------------------------------------------------------------------
void ClientDescriptor::RemoveClient(ClientDescriptor *p) {
    ClientDescriptor *prev = NULL;
    ClientDescriptor *cur  = sd_list;
    while(cur != NULL) {
        if(p==cur) {
            closesocket(cur->m_ClientSocket);
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
//------------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------------
// This function will remove client with the given address
//------------------------------------------------------------------------------------------------------
void ClientDescriptor::RemoveAllClients(void) {
    ClientDescriptor *p;
    ClientDescriptor *cur  = sd_list;
    while(cur != NULL) {
        p=cur;
        cur  = cur->m_Next;
        closesocket(p->m_ClientSocket);
        delete p;
    }
    sd_list=sd_list_last=NULL;

}
//------------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------------
// Gets the name of the current CM system, that this client connects to.
//    This name is from the list of all CM system supported by 'CM:Generic' interface.
//------------------------------------------------------------------------------------------------------
char* ClientDescriptor::getCmName() {
    return this->m_cmName;
}
//------------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------------
// Sets the name of the current CM system, that this client connects to.
//    This name is from the list of all CM system supported by 'CM:Generic' interface.
//------------------------------------------------------------------------------------------------------
void ClientDescriptor::setCmName(char *cm_name) {
    this->m_cmName = strdup(cm_name);
}
//------------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------------
// Returns textual description of this client:
//   "socket=<xxx>, type=<y>, ip=<ii>.<ii>.<ii>.<ii>, port=<zzzzz>"
//------------------------------------------------------------------------------------------------------
char* ClientDescriptor::getDescription() {

    if(this->m_Description == NULL) {

        // Getting parts of IP address
        int ip0 = this->m_ClientTCP & 255;
        int ip1 = (this->m_ClientTCP >> 8) & 255;
        int ip2 = (this->m_ClientTCP >> 16) & 255;
        int ip3 = (this->m_ClientTCP >> 24) & 255;

        // Formatting string
        char str[1024];
        sprintf(str, "socket=%i, type=%i, ip=[%i.%i.%i.%i], port=%i",
                this->m_ClientSocket, this->m_ClientType,
                ip3, ip2, ip1, ip0,
                this->m_ClientPort);

        this->m_Description = strdup(str);
    }

    return this->m_Description;

} //getDescription
//------------------------------------------------------------------------------------------------------
//############################################################################################

//--------------------------------------------------------------------------------------------
// This function will inspect clients list in order to determine which clients
// need to query server. The data will be forwarded from the client socket
// into model server socket and reply will be forwarded from model server socket
// to the client socket.
//--------------------------------------------------------------------------------------------
int MakeSocketsArray(int server[m_DriversNum], int *sockets) {
    static ClientDescriptor* hang;

    for(int i=0;i<m_DriversNum;i++)
        sockets[i]=server[i];
    int amount=m_DriversNum;
    ClientDescriptor *cur = ClientDescriptor::sd_list;
    while(ClientDescriptor::sd_list!=NULL && cur != NULL) {
        sockets[amount++]=cur->m_ClientSocket;
        cur = cur->m_Next;
    }
    return amount;
}
//--------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------------
// This function will check if data available in the selected socket
//------------------------------------------------------------------------------------------------------
int CheckSocketForRead(int socket) {
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
    int res = select(nfsd,(int *)&sock_set,NULL,NULL,&timeout);
#else
    int res = select(nfsd,&sock_set,NULL,NULL,&timeout);
#endif
    if(res > 0) {
        return 1;
    }
    return 0;
}
//------------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------------
// This function will check if data available in any socket
//------------------------------------------------------------------------------------------------------
int WaitSocket(int *sockets, int amount) {
    timeval timeout;
    timeout.tv_sec  = 8;
    timeout.tv_usec = 0;
    fd_set sock_set;
    int    nfsd = 0;
#ifndef _WIN32
    nfsd = FD_SETSIZE;
#endif

    FD_ZERO(&sock_set);
    for(int i=0;i<amount;i++) {
        FD_SET(*(sockets+i), &sock_set);
    }
#ifdef hp700
    int res = select(nfsd,(int *)&sock_set,NULL,NULL,&timeout);
#else
    int res = select(nfsd,&sock_set,NULL,NULL,&timeout);
#endif
    if(res > 0) {
        return 1;
    }
    return 0;
}
//------------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------
// This function will create new client record in the clients linked list.
//--------------------------------------------------------------------------------------------
ClientDescriptor* CreateNewClient(int id, int client_socket, int client_port,
                                  int client_tcp, int client_type) {
    ClientDescriptor* descr = NULL;
    descr = new ClientDescriptor(id,client_socket,client_port,client_tcp,client_type);
    descr->AddNewClient(descr);
    return descr;
}
//--------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------
// This function will run every time the new client try to connect to the editor
// It will create socket connection between the client and the currently 
// running pmod server and will start client log.
//--------------------------------------------------------------------------------------------
void ConnectClient(int client_socket, sockaddr *addr, int client_type) {
    static int ID_Counter = 0;
    int client_port = ((sockaddr_in *)addr)->sin_port;
    int client_tcp  = ntohl(((sockaddr_in *)addr)->sin_addr.s_addr);

    // We will not use password right now, but we may in future
    ClientDescriptor* client = CreateNewClient(ID_Counter,
                                               client_socket,
                                               client_port,
                                               client_tcp,
                                               client_type);
    char *str = client->getDescription();
    if(str != NULL) {
        _DBG(fprintf(log_fd, "  New client connected: %s.\n", str));
    } else {
        _DBG(fprintf(log_fd, "  New client connected.\n"));
    }
    ID_Counter++;
}
//--------------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------------
// This function sends the reply. It sends the length of message before sending message 
// itself.
//---------------------------------------------------------------------------------------------
static int sendReply(int socket, char const* str, char prefix = 0) {

    int len = (str != NULL) ? strlen(str) : 0;

    // M.b. we have to add prefix
    char *tmpBuf = NULL;
    if(prefix != 0) {
        if(len == 0) {
            tmpBuf = new char [2];
            tmpBuf[0] = prefix;
            tmpBuf[1] = 0;
            len = 1;
        } else {
            tmpBuf = new char [len + 3];
            tmpBuf[0] = prefix;
            tmpBuf[1] = '\n';
            tmpBuf[2] = 0;
            strcat(tmpBuf, str);
            len += 2;
        }
        str = tmpBuf;
    }

    // If nothing to send -> exit
    if(len != 0) {
#ifdef XXX_enable_xerces_parsing
        // Replace slashes
        for(int i=0;i<len;i++) {
            if(str[i]=='\\') {
                str[i]='/';
            }
        }
#endif
        SendString(socket, str);
    }

    // Delete memory, which was temporarly allocated. 
    delete [] tmpBuf;
    tmpBuf = NULL;

    return len != 0;

} //sendReply

//---------------------------------------------------------------------------------------------
// Parse command line. It divides this string into set of string
//   which correspond to different argument.
//---------------------------------------------------------------------------------------------
static char** parseCommandLine(char* lpCmdLine) {

    int  i, j, k;
    int  len = strlen(lpCmdLine);
    bool inQ = false;   // Flag: we are inside the string,
                        //   which bounded by double quotes, or not.
    int  idx = 0;       // Index of the current character of the command line
    int  token_len = 0; // Here will be the current token's length.
    int  argc = 0;      // Here will be a number of tokens in command line
    char *token = NULL; // Current token

    // Find possible number of tokens. It is upper estimation.
    for(char *str = lpCmdLine; str != NULL; str = strchr(str, ' ')) {
        argc++;
        str++;
    }

    // Allocate memory for resulting array of tokens
    char **argv = new char* [argc + 1];
    for(i = 0; i <= argc; i++) {
        argv[i] = NULL;
    }

    // Start first  token
    token = new char [len + 1];
    token[0] = 0;
    token_len = 0;
    argv[0] = token;
    argc = 1;

    // Goes thru all characters in the command line
    for(idx = 0; idx < len; idx++) {

        // If this character is double quote -> switch flag
        if(lpCmdLine[idx] == '"') {
            inQ = !inQ;
            continue;
        }

        // If current character is space and
        //   not in the string which bounded by double quotes
        // -> start new token.
        if(!inQ && lpCmdLine[idx] == ' ') {

            // Close previous token
            if(argc > 0) {
                token[token_len] = 0;
            }

            // Start new token
            token = new char [len - idx + 1];
            token[0] = 0;
            token_len = 0;
            argv[argc++] = token;
            continue;
        }

        token[token_len++] = lpCmdLine[idx];
    } //for idx

    // Close last token
    if(argc > 0) {
        token[token_len] = 0;
    }

    // Remove bad tokens (empty or blanks)
    for(k = argc - 1; k >= 0; k--) {

        // We will work with this token
        token = argv[k];
        token_len = strlen(token);

        // Check if it contains only spaces
        for(j = 0; j < token_len; j++) {
            if(token[j] != ' ') {
                break;
            }
        }

        // We have to remove this token
        if(j == token_len) {

            delete token;
            token = NULL;

            // Shift array
            for(i = k + 1; i <= argc; i++) {
                argv[i - 1] = argv[i];
            }
        }
    } //for k

    return argv;

} //parseCommandLine
//---------------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------------
// Expands all environment variables in the given command line specification.
// Function will modify source buffer. We suppose that this buffer has enough space.
//
// Returns:  0 - all right, source buffer contains processed command line;
//          !0 - error, source buffer contains error description.
//---------------------------------------------------------------------------------------------
int expandEnvVariables(char *commandLine) {

    // Calculate number of references to env. variables, i.e. number of dollar signs.
    int len = strlen(commandLine);
    int count = 0;
    for(int i = 0; i < len; i++) {
        if(commandLine[i] == '$') {
            count++;
        }
    }

    if(count == 0) {
        // Nothing to do. Source buffer doesn't contain any env. variables.
        return 0;
    }

    // Allocate temporary memory for transformation.
    int tmp_sz = len + count * 1024;
    char *expanded = new char [tmp_sz];
    memset(expanded, 0, tmp_sz);

    // Go through source command line specification, find env. variables, and replace them.
    char var_name[1024];
    char *var_value = NULL;
    char *prev = commandLine;
    char *ptr = NULL;
    int retCode = 0;
    while(prev != NULL) {

        // Find next reference to env. variable.
        ptr = strchr(prev, '$');
        if(ptr == NULL) {

            // Copy the rest part of source command line.
            strcat(expanded, prev);
            break;

        } else {

            // Extract name of env. variable.
            if(ptr[1] == '{') {

                // We have "${VARNAME}" notation here.

                // Find closing brace.
                char *close_brace = strchr(ptr + 2, '}');
                if(close_brace == NULL) {
                    strcpy(expanded,
                           "Command line specification contains reference " \
                           "to environment variable, which starts from '${', " \
                           "but doesn't contain closing curly brace '}'.");
                    retCode = -1;
                    break;
                }

                // Get env. variable name.
                tmp_sz = close_brace - ptr - 2;
                strncpy(var_name, ptr + 2, tmp_sz);
                var_name[tmp_sz] = 0;

                // Get env. variable value.
                var_value = getenv(var_name);

                // Check if variable was found.
                if(var_value == NULL) {
                    sprintf(expanded,
                            "Command line specification contains reference " \
                            "to undefined environment variable '%s'.",
                            var_name);
                    retCode = -1;
                    break;
                }

                // M.b. we have some part before reference to env. variable.
                if(ptr != prev) {
                    tmp_sz = ptr - prev;
                    strncat(expanded, prev, tmp_sz);
                }

                // Put env. variable value to destination buffer.
                strcat(expanded, var_value);

                // Go ahead.
                prev = close_brace + 1;

            } else {

                // We have "$VARNAME" notation here.

                // Find end of env. variable. We suppose that it is either '/' or '\'.
                char *close_slash = strpbrk(ptr + 1, "\\/");

                // Get env. variable name.
                if(close_slash == NULL) {
                    strcpy(var_name, ptr + 1);
                } else {
                    tmp_sz = close_slash - ptr - 1;
                    strncpy(var_name, ptr + 1, tmp_sz);
                    var_name[tmp_sz] = 0;
                }

                // Get env. variable value.
                var_value = getenv(var_name);

                // Check if variable was found.
                if(var_value == NULL) {
                    sprintf(expanded,
                            "Command line specification contains reference " \
                            "to undefined environment variable '%s'. " \
                            "M.b. program coudln't extract correctly variable name. " \
                            "It is better to use '${VARNAME}' notation.",
                            var_name);
                    retCode = -1;
                    break;
                }

                // M.b. we have some part before reference to env. variable.
                if(ptr != prev) {
                    tmp_sz = ptr - prev;
                    strncat(expanded, prev, tmp_sz);
                }

                // Put env. variable value to destination buffer.
                strcat(expanded, var_value);

                // Go ahead.
                prev = close_slash;

            } //if ptr[1]
        } //if ptr == NULL
    } //while

    strcpy(commandLine, expanded);
    delete [] expanded;
    expanded = NULL;

    return retCode;

} //expandEnvVariables
//---------------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------------
// Runs external program
//---------------------------------------------------------------------------------------------

#ifdef _WIN32

DWORD OutputError(char** pszOutputBuffer, char *pszAPI);
DWORD ReadAndHandleOutput(char** pBuffer,HANDLE hPipeRead);
DWORD PrepAndLaunchRedirectedChild(char** pBuffer,
                                   char* command,
                                   HANDLE hChildStdOut,
                                   HANDLE hChildStdIn,
                                   HANDLE hChildStdErr);
DWORD WINAPI GetAndSendInputThread(LPVOID lpvThreadParam);

DWORD Win32RunRedirectedApp(char* command, char** pBuffer, char* workingDir) {
    HANDLE hOutputReadTmp,hOutputRead,hOutputWrite;
    HANDLE hInputWriteTmp,hInputRead,hInputWrite;
    HANDLE hErrorWrite;
    SECURITY_ATTRIBUTES sa;

    // Set up the security attributes struct.
    sa.nLength= sizeof(SECURITY_ATTRIBUTES);
    sa.lpSecurityDescriptor = NULL;
    sa.bInheritHandle = TRUE;


    // Create the child output pipe.
    if(!CreatePipe(&hOutputReadTmp,&hOutputWrite,&sa,0)) {
        return OutputError(pBuffer,"CreatePipe");
    }


    // Create a duplicate of the output write handle for the std error
    // write handle. This is necessary in case the child application
    // closes one of its std output handles.
    if (!DuplicateHandle(GetCurrentProcess(),hOutputWrite,
                         GetCurrentProcess(),&hErrorWrite,0,
                         TRUE,DUPLICATE_SAME_ACCESS)) {
        return OutputError(pBuffer,"DuplicateHandle");
    }


    // Create the child input pipe.
    if (!CreatePipe(&hInputRead,&hInputWriteTmp,&sa,0)) {
        return OutputError(pBuffer,"CreatePipe");
    }


    // Create new output read handle and the input write handles. Set
    // the Properties to FALSE. Otherwise, the child inherits the
    // properties and, as a result, non-closeable handles to the pipes
    // are created.
    if (!DuplicateHandle(GetCurrentProcess(),hOutputReadTmp,
                         GetCurrentProcess(),
                         &hOutputRead, // Address of new handle.
                         0,FALSE, // Make it uninheritable.
                         DUPLICATE_SAME_ACCESS)) {
        return OutputError(pBuffer,"DupliateHandle");
    }

    if (!DuplicateHandle(GetCurrentProcess(),hInputWriteTmp,
                         GetCurrentProcess(),
                         &hInputWrite, // Address of new handle.
                         0,FALSE, // Make it uninheritable.
                         DUPLICATE_SAME_ACCESS)) {
        return OutputError(pBuffer,"DupliateHandle");
    }


    // Close inheritable copies of the handles you do not want to be
    // inherited.
    if (!CloseHandle(hOutputReadTmp)) return OutputError(pBuffer,"CloseHandle");
    if (!CloseHandle(hInputWriteTmp)) return OutputError(pBuffer,"CloseHandle");


    DWORD dwErr = PrepAndLaunchRedirectedChild(pBuffer,command,hOutputWrite,hInputRead,hErrorWrite);
    if(dwErr!=ERROR_SUCCESS) return dwErr;


    // Close pipe handles (do not continue to modify the parent).
    // You need to make sure that no handles to the write end of the
    // output pipe are maintained in this process or else the pipe will
    // not close when the child process exits and the ReadFile will hang.
    if (!CloseHandle(hOutputWrite)) return OutputError(pBuffer,"CloseHandle");
    if (!CloseHandle(hInputRead )) return OutputError(pBuffer,"CloseHandle");
    if (!CloseHandle(hErrorWrite)) return OutputError(pBuffer,"CloseHandle");


    // Read the child's output.
    dwErr = ReadAndHandleOutput(pBuffer,hOutputRead);
    if(dwErr!=ERROR_SUCCESS) return dwErr;
    // Redirection is complete

    if (!CloseHandle(hOutputRead)) return OutputError(pBuffer,"CloseHandle");
    if (!CloseHandle(hInputWrite)) return OutputError(pBuffer,"CloseHandle");
	
    return ERROR_SUCCESS;
}


///////////////////////////////////////////////////////////////////////
// PrepAndLaunchRedirectedChild
// Sets up STARTUPINFO structure, and launches redirected child.
///////////////////////////////////////////////////////////////////////
DWORD PrepAndLaunchRedirectedChild(char** pBuffer,
                                   char* command,
                                   HANDLE hChildStdOut,
                                   HANDLE hChildStdIn,
                                   HANDLE hChildStdErr) {
    PROCESS_INFORMATION pi;
    STARTUPINFO si;

    // Set up the start up info struct.
    ZeroMemory(&si,sizeof(STARTUPINFO));
    si.wShowWindow=SW_HIDE;
    si.cb = sizeof(STARTUPINFO);
    si.dwFlags = STARTF_USESTDHANDLES|STARTF_USESHOWWINDOW;
    si.hStdOutput = hChildStdOut;
    si.hStdInput  = hChildStdIn;
    si.hStdError  = hChildStdErr;

    // Launch the process that you want to redirect
    if (!CreateProcess(NULL,command,NULL,NULL,TRUE,
                       CREATE_NO_WINDOW,NULL,NULL,&si,&pi)) {
        return OutputError(pBuffer,"CreateProcess");
    }


    // Close any unnecessary handles.
    if (!CloseHandle(pi.hThread)) return OutputError(pBuffer,"CloseHandle");
    return ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////
// ReadAndHandleOutput
// Monitors handle for input. Exits when child exits or pipe breaks.
///////////////////////////////////////////////////////////////////////
#include "string"
using namespace std;
DWORD ReadAndHandleOutput(char** pBuffer,HANDLE hPipeRead)
{
    CHAR lpBuffer[256];
    string szOutput;
    DWORD nBytesRead;

    while(TRUE) {
        if (!ReadFile(hPipeRead,lpBuffer,sizeof(lpBuffer)-1,
                      &nBytesRead,NULL) || !nBytesRead) {
            if (GetLastError() == ERROR_BROKEN_PIPE)
                break; // pipe done - normal exit path.
            else
                return OutputError(pBuffer,"ReadFile"); // Something bad happened.
        }
        lpBuffer[nBytesRead]=0;
        szOutput += lpBuffer;
    }

    int nLen = szOutput.length()+1;
    *pBuffer = new char[nLen];
    memcpy(*pBuffer,szOutput.c_str(),nLen);
    return ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////
// OutputError
// OutputError the error number and corresponding message.
///////////////////////////////////////////////////////////////////////
DWORD OutputError(char** pszOutputBuffer, char *pszAPI)
{
    char* lpMsgBuf;
    static char* pcOutputFormat="ERROR: API    = %s.\n   message    = %s.\n"; 

    DWORD dwError = GetLastError();
    FormatMessage(
                  FORMAT_MESSAGE_ALLOCATE_BUFFER |
                  FORMAT_MESSAGE_FROM_SYSTEM |
                  FORMAT_MESSAGE_IGNORE_INSERTS,
                  NULL,
                  dwError,
                  0, // Default language
                  (LPTSTR) &lpMsgBuf,
                  0,
                  NULL
                  );
	
    int nLen = strlen(lpMsgBuf)+strlen(pcOutputFormat)+strlen(pszAPI)+1;
    *pszOutputBuffer = new char[nLen];
    sprintf(*pszOutputBuffer, pcOutputFormat, pszAPI, lpMsgBuf);
    // Free the buffer.
    LocalFree( lpMsgBuf );

    return dwError;
}
#endif

int runCommand(char* command, char** pBuffer, char* workingDir) {

    // Check parameters
    if(command == NULL || pBuffer == NULL) {
        return -1;
    }

    // M.b. we have to expand env. variables.
    int expand_retcode = expandEnvVariables(command);
    if(expand_retcode != 0) {

        // We couldn't expand command line specification.
        int len = strlen(command);
        *pBuffer = new char[len + 1];
        strcpy(*pBuffer, command);
        return expand_retcode;
    }

    _DBG(fprintf(log_fd, "\n<RUN>\n"));
    _DBG(fprintf(log_fd, "%s", command));
    _DBG(fprintf(log_fd, "\n</RUN>\n\n"));

    static const char*	CANT_CREATE_PIPE = "Error: Can't create pipe.";
    static const char*	CANT_CREATE_PROCESS = "Error: Can't create process.";
    static const char*	CANT_REDIRECT_IO = "Error: Can't redirect output from external process.";
    static const char*	CANT_READ_OUTPUT = "Error: Can't read output of external program.";
    static const char*	DONE = "done.";

    // Here will be output from external program
    *pBuffer = NULL;

#ifdef _WIN32
    // Running external program on Windows platform
    return Win32RunRedirectedApp(command, pBuffer, workingDir);
#else
    // Running external program on UNIX platform

    // Create pipe for communicating with external program
    int p[2];
    if(pipe(p) != 0) {
        int len = strlen(CANT_CREATE_PIPE);
        *pBuffer = new char[len + 1];
        strcpy(*pBuffer, CANT_CREATE_PIPE);
        return -1;
    }

    // We return this code
    int retCode = 0;

    // Parse source command line to set of arguments
    char **arglist = parseCommandLine(command);

    // To run external program we have to fork current process
    const pid_t pid = fork();

    if(pid == -1) {

        // We failed to create another process
        int len = strlen(CANT_CREATE_PROCESS);
        *pBuffer = new char[len + 1];
        strcpy(*pBuffer, CANT_CREATE_PROCESS);
        retCode = -1;

    } else if(pid == 0) {

        // It is child process
        setpgid(0, 0);

        // Redirecting input/output
        if(dup2(p[0], 0) == -1 || dup2(p[1], 1) == -1 || dup2(p[1], 2) == -1) {

            // We failed to redirect input/output
            int len = strlen(CANT_REDIRECT_IO);
            *pBuffer = new char[len + 1];
            strcpy(*pBuffer, CANT_REDIRECT_IO);
            retCode = -1;

        } else {

            // Run external program
            if(execvp(arglist[0], arglist) != 0) {

                // We failed to start external program
                char *errmsg = strerror(errno);
                printf("Can't execute '%s': %s", arglist[0], errmsg);
                exit(errno);
            }

            // Actually we will never be here
            exit(0);

        } //if dup2

    } else {

        // It is parent process
        int childPID = 0;
        int	status = 0;

        // Wait until child finishes
        do {
            childPID = wait(&status);
        } while((childPID != pid) && (childPID > 0));

        // Exit code of external program
        retCode = status;

        // Read output from external program
        struct stat st;
        int res = fstat(p[0], &st);
        if(res == 0) {

            int len = st.st_size;
            if(len > 0) {

                // Allocate memory and try to read
                char *buffer = new char[len + 1];
                int actual = read(p[0], buffer, len);

                if(actual < 0) {

                    // We failed to read output
                    int len = strlen(CANT_READ_OUTPUT);
                    *pBuffer = new char[len + 1];
                    strcpy(*pBuffer, CANT_READ_OUTPUT);
                    retCode = status != 0 ? status : -1;
                    delete [] buffer;

                } else {

                    // Mark end of string. Just to make sure.
                    if(actual <= len) {
                        buffer[actual] = 0;
                    } else {
                        buffer[len] = 0;
                    }
                    *pBuffer = buffer;
                }
            } //if len
        } //if res

        // Close pipe
        close(p[0]);
        close(p[1]);

    } //if pid

    // Print to log-file what we've got from external program
    _DBG(fprintf(log_fd, "\n<OUTPUT>\n"));
    if(*pBuffer != NULL) {
        char *str = *pBuffer;
        int len = strlen(str);
        if(str[len - 1] == '\n') {
            _DBG(fprintf(log_fd, "%s", str));
        } else {
            _DBG(fprintf(log_fd, "%s\n", str));
        }
    }
    _DBG(fprintf(log_fd, "\n</OUTPUT>\n\n"));

    // Delete memory which was temporarly allocated
    delete [] arglist;

    return retCode;
#endif

} //runCommand

int runCommand(char* command, char** pBuffer) {
    return  runCommand(command,pBuffer,(char*)NULL);
}

//---------------------------------------------------------------------------------------------
// This function will process given ClearCase command with it's arguments and will 
// send the reply
//---------------------------------------------------------------------------------------------
int ProcessClearCaseCommand(ClientDescriptor* client, char **args) {
    char	command[1024];
#ifdef XXX_enable_xerces_parsing
    char*	reply = NULL;
#else
    static char const empty[] = "";
    char* reply = new char[sizeof(empty)];
    strcpy(reply, empty);
#endif

    if(strcmp(args[0],REGISTER_CMD)==0) {
        nClients++;
        sendReply(client->m_ClientSocket,"done");
        _DBG(fprintf(log_fd,"ClearCase %s :done\n",args[0]));
        return 1;
    }
    if(strcmp(args[0],UNREGISTER_CMD)==0) {
        nClients--;
        if(nClients < 1 && work_forever == 0) {
            do_shutdown = 1;
        }
        sendReply(client->m_ClientSocket,"done");
        _DBG(fprintf(log_fd,"ClearCase %s :done\n",args[0]));
    }
    // Un Check Out command
    if(strcmp(args[0],UNCHECKOUT_CMD)==0) {
        sprintf(command,"cleartool.exe uncheckout -rm \"%s\"",args[1]);
        if(runCommand(command,&reply)!=0)
            sendReply(client->m_ClientSocket,"Error");
        sendReply(client->m_ClientSocket,reply);
        delete reply;
        _DBG(fprintf(log_fd,"ClearCase %s :done\n",command));
        return 1;
    }
    // Check Out command
    if(strcmp(args[0],CHECKOUT_CMD)==0) {
        if(args[2] != NULL && args[2][0] != 0)
            sprintf(command,"cleartool.exe checkout -c %s -unreserved \"%s\"",args[2],args[1]);
        else 
            sprintf(command,"cleartool.exe checkout -nc -unreserved \"%s\"",args[1]);
        if(runCommand(command,&reply)!=0)
            sendReply(client->m_ClientSocket,"Error");
        sendReply(client->m_ClientSocket,reply);
        delete reply;
        _DBG(fprintf(log_fd,"ClearCase %s :done\n",command));
        return 1;
    }
    // Check In command
    if(strcmp(args[0],CHECKIN_CMD)==0) {
        if(args[2] != NULL && args[2][0] != 0)
            sprintf(command,"cleartool.exe checkin -c %s \"%s\"",args[2],args[1]);
        else
            sprintf(command,"cleartool.exe checkin -nc \"%s\"",args[1]);
        if(runCommand(command,&reply)!=0)
            sendReply(client->m_ClientSocket,"Error");
        sendReply(client->m_ClientSocket,reply);
        delete reply;
        _DBG(fprintf(log_fd,"ClearCase %s :done\n",command));
        return 1;
    }
    // Reserv command
    if(strcmp(args[0],RESERVE_CMD)==0) {
        if(args[2] != NULL && args[2][0] != 0)
            sprintf(command,"cleartool.exe reserve -c %s \"%s\"",args[2],args[1]);
        else
            sprintf(command,"cleartool.exe reserve -nc \"%s\"",args[1]);
        if(runCommand(command,&reply)!=0)
            sendReply(client->m_ClientSocket,"Error");
        sendReply(client->m_ClientSocket,reply);
        delete reply;
        _DBG(fprintf(log_fd,"ClearCase %s :done\n",command));
        return 1;
    }
    // Unreserv command
    if(strcmp(args[0],UNRESERVE_CMD)==0) {
        if(args[2] != NULL && args[2][0] != 0)
            sprintf(command,"cleartool.exe unreserve -c %s \"%s\"",args[2],args[1]);
        else 
            sprintf(command,"cleartool.exe unreserve -nc %s",args[1]);
        if(runCommand(command,&reply)!=0)
            sendReply(client->m_ClientSocket,"Error");
        sendReply(client->m_ClientSocket,reply);
        delete reply;
        _DBG(fprintf(log_fd,"ClearCase %s :done\n",command));
        return 1;
    }
    // "get private list of checked out files" command
    if(strcmp(args[0],LOCALLIST_CMD)==0) {
        sprintf(command,"cleartool.exe lspriv -co -short -invob \"%s\"",args[1]);
        if(runCommand(command,&reply,args[1])!=0)
            sendReply(client->m_ClientSocket,"Error");
        sendReply(client->m_ClientSocket,reply);
        delete reply;
        _DBG(fprintf(log_fd,"ClearCase %s :done\n",command));
        return 1;
    }
    // We do not know this command 
    sendReply(client->m_ClientSocket,"ClearCase :unknown command");
    _DBG(fprintf(log_fd,"ClearCase Unknown command %s.\n",args[0]));
    return 0;
}
//----------------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------------
// This function will process given Source Integrity command with it's arguments and will 
// send the reply
//---------------------------------------------------------------------------------------------
int ProcessSICommand(ClientDescriptor* client, char **args) {
	
    char	command[1024];
    char*	reply = NULL;

    if(strcmp(args[0],REGISTER_CMD)==0) {
        nClients++;
        sendReply(client->m_ClientSocket,"done");
        _DBG(fprintf(log_fd,"Source Integrity %s :done\n",args[0]));
        return 1;
    }
    if(strcmp(args[0],UNREGISTER_CMD)==0) {
        nClients--;
        if(nClients < 1 && work_forever == 0) {
            do_shutdown = 1;
        }
        sendReply(client->m_ClientSocket,"done");
        _DBG(fprintf(log_fd,"Source Integrity %s :done\n",args[0]));
    }

    // Un Check Out command
    if(strcmp(args[0],UNCHECKOUT_CMD)==0) {
        sprintf(command,"si revert -g \"%s\"",args[1]);
        if(runCommand(command,&reply)!=0)
            sendReply(client->m_ClientSocket,"Error");
        sendReply(client->m_ClientSocket,reply);
        delete reply;
        _DBG(fprintf(log_fd,"Source Integrity %s :done\n",UNCHECKOUT_CMD));
        return 1;
    }

    // Check Out command
    if(strcmp(args[0],CHECKOUT_CMD)==0) {
        sprintf(command,"si co -g \"%s\"",args[1]);
        if(runCommand(command,&reply)!=0)
            sendReply(client->m_ClientSocket,"Error");
        sendReply(client->m_ClientSocket,reply);
        delete reply;

        _DBG(fprintf(log_fd,"Source Integrity %s :done\n",CHECKOUT_CMD));
        return 1;
    }

    // Check In command
    if(strcmp(args[0],CHECKIN_CMD)==0) {
        sprintf(command,"si ci -g \"%s\"",args[1]);
        if(runCommand(command,&reply)!=0)
            sendReply(client->m_ClientSocket,"Error");
        sendReply(client->m_ClientSocket,reply);
        delete reply;

        _DBG(fprintf(log_fd,"Source Integrity %s :done\n",CHECKIN_CMD));
        return 1;
    }
	
    // Reserv command
    if(strcmp(args[0],RESERVE_CMD)==0) {
        sprintf(command,"si lock -g \"%s\"",args[1]);
        if(runCommand(command,&reply)!=0)
            sendReply(client->m_ClientSocket,"Error");
        sendReply(client->m_ClientSocket,reply);
        delete reply;

        _DBG(fprintf(log_fd,"Source Integrity %s :done\n",RESERVE_CMD));
        return 1;
    }
    // Unreserv command
    if(strcmp(args[0],UNRESERVE_CMD)==0) {
        sprintf(command,"si unlock -g \"%s\"",args[1]);
        if(runCommand(command,&reply)!=0)
            sendReply(client->m_ClientSocket,"Error");
        sendReply(client->m_ClientSocket,reply);
        delete reply;

        _DBG(fprintf(log_fd,"Source Integrity %s :done\n",UNRESERVE_CMD));
        return 1;
    }
    // Resync command
    if(strcmp(args[0],RESYNC_CMD)==0) {
        sprintf(command,"si resync -g \"%s\"",args[1]);
        if(runCommand(command,&reply)!=0)
            sendReply(client->m_ClientSocket,"Error");
        sendReply(client->m_ClientSocket,reply);
        delete reply;

        _DBG(fprintf(log_fd,"Source Integrity %s :done\n",LOCALLIST_CMD));
        return 1;
    }
    // We do not know this command 
    sendReply(client->m_ClientSocket,"Error");
    sendReply(client->m_ClientSocket,"Unknown command.");
    _DBG(fprintf(log_fd,"Source Integrity Unknown command %s.\n",args[0]));
    return 0;
}
//----------------------------------------------------------------------------------------------


//---------------------------------------------------------------------------------------------
// Adds to a set of results new result at the end.
//   - Reallocate memory.
//   - Adds new line if necessary.
//---------------------------------------------------------------------------------------------
static char* combineResults(char *resultsSet, char *result) {

    // M.b. nothing to add
    if(result == NULL || result[0] == 0) {
        return resultsSet;
    }

    // Calculate new length of the set of results
    int oldSetLen = (resultsSet != NULL) ? strlen(resultsSet) : 0;
    int newSetLen = strlen(result) + oldSetLen + 3;

    // New buffer for the set
    char *newResultsSet = new char [newSetLen];
    newResultsSet[0] = 0;

    // Copy the old set to the new buffer
    if(oldSetLen > 0) {
        strcat(newResultsSet, resultsSet);

        // Add divider if necessary
        if(resultsSet[oldSetLen - 1] != '\n') {
            strcat(newResultsSet, "\n");
        }
    }

    // Add new result
    strcat(newResultsSet, result);

    // Delete the old set
    delete [] resultsSet;

    return newResultsSet;

} //combineResults
//---------------------------------------------------------------------------------------------


//---------------------------------------------------------------------------------------------
// This function will process given command from the generic CM system with it's arguments 
// and will send the reply.
//---------------------------------------------------------------------------------------------
int ProcessGenericCommand(ClientDescriptor* client, char **args) {

    int		resCode = 0; // result - error code
    char*	reply = NULL;

    _DBG(fprintf(log_fd,"Generic CM '%s' ...\n", args[0]));

    if(strcmp(args[0], REGISTER_CMD) == 0) {

        // Register one more client
        nClients++;
        sendReply(client->m_ClientSocket, "1");

    } else if(strcmp(args[0], UNREGISTER_CMD) == 0) {

        // Unregister client
        nClients--;

        // If there are no clients left -> close the driver.
        if(nClients < 1 && work_forever == 0) {
            do_shutdown = 1;
        }
        sendReply(client->m_ClientSocket, "", '1');

    } else if(strcmp(args[0], "get_cm_systems") == 0) {

        // Getting all CM systems that are supported in the driver
        try {
            reply = cmXml->getCmSystems();
            sendReply(client->m_ClientSocket, reply, '1');
            delete [] reply;
            reply = NULL;
        }

        catch(const CmXmlException& toCatch) {
            char const *msg = toCatch.getMessage();
            sendReply(client->m_ClientSocket, "Error", '1');
            sendReply(client->m_ClientSocket, msg, '1');
            resCode = -1;
        }

    } else if(strcmp(args[0], "connect") == 0) {

        // Connect to particular CM system
        if(cmXml->isCmSystem(args[1])) {
            client->setCmName(args[1]);
            sendReply(client->m_ClientSocket, "", '1');
        } else {
            sendReply(client->m_ClientSocket, "Error", '1');
            sendReply(client->m_ClientSocket, "Unknown CM system.", '1');
            resCode = -1;
        }

    } else if(strcmp(args[0], "get_attributes") == 0) {

        // Get all attributes of the current CM system
        try {
            reply = cmXml->getAttributes(client->getCmName());
            sendReply(client->m_ClientSocket, reply, '1');
            delete [] reply;
            reply = NULL;
        }

        catch(const CmXmlException& toCatch) {
            char *msg = (char*)toCatch.getMessage();
            sendReply(client->m_ClientSocket, "Error", '1');
            sendReply(client->m_ClientSocket, msg, '1');
            resCode = -1;
        }

    } else if(strcmp(args[0], "get_commands") == 0) {

        // Get all commands of the current CM system
        try {
            reply = cmXml->getCommands(client->getCmName());
            sendReply(client->m_ClientSocket, reply, '1');
            delete [] reply;
            reply = NULL;
        }

        catch(const CmXmlException& toCatch) {
            char *msg = (char*)toCatch.getMessage();
            sendReply(client->m_ClientSocket, "Error", '1');
            sendReply(client->m_ClientSocket, msg, '1');
            resCode = -1;
        }

    } else if(strcmp(args[0], "get_attr_value") == 0) {

        // Get value of this attribute
        if(args[1] == NULL || strlen(args[1]) == 0) {

            // Client doesn't specify attribute.
            sendReply(client->m_ClientSocket, "Error", '1');
            sendReply(client->m_ClientSocket,
                      "The client doesn't specify attribute.", '1');
            resCode = -1;

        } else if(!cmXml->isAttribute(client->getCmName(), args[1])) {

            // We don't know this attribute
            sendReply(client->m_ClientSocket, "Error", '1');
            sendReply(client->m_ClientSocket, "Unknown attribute.", '1');
            resCode = -1;

        } else {
            // We know this attribute

            try {
                // Get value from external program
                char *cmdLine = cmXml->translateCommand(client->getCmName(), args);
                if(cmdLine == NULL || cmdLine[0] == 0) {

                    // Wrong 'cm.xml'
                    sendReply(client->m_ClientSocket, "Error", '1');
                    sendReply(client->m_ClientSocket, "Unable to get command line specification!", '1');
                    resCode = -1;

                } else {

                    if(runCommand(cmdLine, &reply) != 0) {

                        // Command failed
                        sendReply(client->m_ClientSocket, "Error", '1');
                        sendReply(client->m_ClientSocket, reply, '1');
                        resCode = -1;

                    } else {

                        // Command completed successfully
                        char *result = cmXml->translateResult(client->getCmName(), args[1], reply);
                        sendReply(client->m_ClientSocket, result, '1');
                        delete [] result;
                        result = NULL;
                    }

                    delete [] reply;
                    reply = NULL;

                } //if cmdLine

                delete [] cmdLine;
                cmdLine = NULL;

            } //try

            catch(const CmXmlException& toCatch) {
                char *msg = (char*)toCatch.getMessage();
                sendReply(client->m_ClientSocket, "Error", '1');
                sendReply(client->m_ClientSocket, msg, '1');
                resCode = -1;
            }

        } //if isAttribute

    } else {

        // M.b. it is one of the command of the current CM system
        if(cmXml->isCommand(client->getCmName(), args[0])) {

            // Try to execute this command
            try {
                // Translate command thru CmXml
                bool needReturn = cmXml->isReturnable(client->getCmName(), args[0]);
                int filesStart = 0;
                char *cmdLine = cmXml->translateCommand(client->getCmName(), args, &filesStart);

                if(cmdLine == NULL || cmdLine[0] == 0) {

                    // Something wrong in 'cm.xml'. We can't translate this command.
                    reply = new char [strlen(args[0]) + 128];
                    sprintf(reply, "Unable to translate the command '%s'!", args[0]);

                    sendReply(client->m_ClientSocket, "Error", '1');
                    sendReply(client->m_ClientSocket, reply, '1');
                    resCode = -1;

                    delete [] reply;
                    reply = NULL;

                } else {

                    // Determine: how many times we should run this command
                    if(filesStart == -1) {

                        // We don't need to run the command for every file in the <args>.
                        if(runCommand(cmdLine, &reply) != 0) {

                            // Command failed
                            sendReply(client->m_ClientSocket, "Error", '1');
                            sendReply(client->m_ClientSocket, reply, '1');
                            resCode = -1;

                        } else {

                            // Command completed successfully
                            if(needReturn) {
                                sendReply(client->m_ClientSocket, reply, '1');
                            } else {
                                sendReply(client->m_ClientSocket, "", '1');
                            }
                        }

                        delete [] reply;
                        reply = NULL;

                    } else {
                        // We have to run the command for all files separately. 
                        //    And replace '%s' in the command line with their names.

                        // Here we will construct new command line with file name inside.
                        int len = strlen(cmdLine);
                        char *privateCmdLine = new char [len + 1024];
                        privateCmdLine[0] = 0;

                        // Here we will combine all error results
                        char *errorResults = NULL;

                        // Here we will combine all normal results
                        char *normalResults = NULL;

                        // Let's go thru all files in <args>
                        for(int i = filesStart; args[i] != NULL; i++) {

                            // Construct final executable command line
                            sprintf(privateCmdLine, cmdLine, args[i]);

                            if(runCommand(privateCmdLine, &reply) != 0) {
                                // Command failed
                                errorResults = combineResults(errorResults, reply);

                            } else {
                                // Command completed successfully
                                if(needReturn) {
                                    normalResults = combineResults(normalResults, reply);
                                }
                            }

                            delete [] reply;
                            reply = NULL;
                        } //for i

                        // Send reply to the client
                        if(errorResults != NULL) {

                            // If error occured during executing external program -> send error message(s).
                            sendReply(client->m_ClientSocket, "Error", '1');
                            sendReply(client->m_ClientSocket, errorResults, '1');
                            resCode = -1;

                        } else {

                            // If all commands completed successfully -> send normal reply(s)
                            if(needReturn) {
                                sendReply(client->m_ClientSocket, normalResults, '1');
                            } else {
                                sendReply(client->m_ClientSocket, "", '1');
                            }
                        }

                        // Delete all temporarly allocated
                        delete [] privateCmdLine;
                        privateCmdLine = NULL;
                        delete [] errorResults;
                        errorResults = NULL;
                        delete [] normalResults;
                        normalResults = NULL;

                    } //if filesStart
                }

                delete [] cmdLine;
                cmdLine = NULL;
            } //try

            catch(const CmXmlException& toCatch) {
                char *msg = (char*)toCatch.getMessage();
                sendReply(client->m_ClientSocket, "Error", '1');
                sendReply(client->m_ClientSocket, msg, '1');
                resCode = -1;
            }
			
        } else {

            // We do not know this command 
            sendReply(client->m_ClientSocket, "Error", '1');
            sendReply(client->m_ClientSocket, "Unknown command.", '1');
            _DBG(fprintf(log_fd, "Unknown Generic CM command '%s!'\n", args[0]));
            resCode = -1;

        } //if isCommand
    } //if args[0]

    if(resCode == 0) {
        _DBG(fprintf(log_fd,"Generic CM '%s': done.\n", args[0]));
    } else {
        _DBG(fprintf(log_fd,"Generic CM '%s': FAILED!\n", args[0]));
    }

    return resCode;

} //ProcessGenericCommand
//----------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------
// Client send CM an operation request
//--------------------------------------------------------------------------------------------
int ProcessClientQuery(ClientDescriptor* client) {
    char*	commandString;
    int		readed;
    char	**args;
    int		i;

    // Redirecting query from the client to the server
    commandString = ReceiveString(client->m_ClientSocket);
    if(commandString == NULL) {
        _DBG(fprintf(log_fd,"Removing dead client.\n"));
        char *str = client->getDescription();
        if(str != NULL) {
            _DBG(fprintf(log_fd,"  Client disconnected: %s.\n", str));
        } else {
            _DBG(fprintf(log_fd,"  Client disconnected.\n"));
        }
        ClientDescriptor::RemoveClient(client);
        return 1;
    }
    readed = strlen(commandString);

    char *str = client->getDescription();
    if(str != NULL) {
        _DBG(fprintf(log_fd,"Processing query from the client: %s.\n", str));
    } else {
        _DBG(fprintf(log_fd,"Processing query from the client.\n"));
    }

    // Tokenizing string query and placing all tokens into the vals matrix
    CmXmlStringTokenizer tokenizer(commandString, "\t", CmXmlStringTokenizer::ONE_SIDE_TOKEN);
    int argsTotal = tokenizer.howManyTokensLeft();
    args = new char* [argsTotal + 1];
    int argsCount = 0;
    for(i = 0; i < argsTotal; i++) {
        char *a = tokenizer.nextToken();
        if(a != NULL && a[0] != 0) {
            int len = strlen(a);
            args[argsCount] = new char [len + 1];
            strcpy(args[argsCount], a);
            argsCount++;
        }
    }
    // Add stopper
    for(i = argsCount; i <= argsTotal; i++) {
        args[i] = NULL;
    }

    // Here will be resulting op-code
    int resCode = 1;

    if(argsCount == 0) {

        _DBG(fprintf(log_fd,"Nothing to proceed.\n"));
        resCode = 0;

    } else {

        // What type of CM interface we have
        switch(client->m_ClientType) {
        case CLEARCASE:
            ProcessClearCaseCommand(client,args);
            break;
        case SI:
            ProcessSICommand(client,args);
            break;
        case GENERIC:
            ProcessGenericCommand(client,args);
            break;
        default:
            _DBG(fprintf(log_fd, "Unknown CM system with code %i.\n", client->m_ClientType));
            resCode = 0;
            break;
        }
    } //if argCount

    // Delete temporary allocated memory
    for(i = 0; i < argsCount; i++) {
        delete [] args[i];
        args[i] = NULL;
    }
    delete [] args;
    args = NULL;

    return resCode;

} //ProcessClientQuery
//--------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------
// This function will inspect clients list in order to determine which clients
// need to query server. The data will be forwarded from the client socket
// into model server socket and reply will be forwarded from model server socket
// to the client socket.
//--------------------------------------------------------------------------------------------
void ProcessClients(void) {

    ClientDescriptor *cur = ClientDescriptor::sd_list;
    while(ClientDescriptor::sd_list!=NULL && cur != NULL) {
        if(CheckSocketForRead(cur->m_ClientSocket)) {
            // Processing client request
            if(ProcessClientQuery(cur)) 
                break;
        }
        cur = cur->m_Next;
    }
}
//--------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------
// Loop while not shutting down. Will listen clients socket and administrator socket,
// will process clients and administrator requests if any.
//-------------------------------------------------------------------------------------------
void RequestsLoop(void) {
    int              sockets[3000];
    struct sockaddr  s;
    socklen_t        s_size = sizeof(s);

    do {
        int amount = MakeSocketsArray(Connection.m_DriversListeningSocket,sockets);
        WaitSocket(sockets,amount); 
        for(int i=0; i<m_DriversNum;i++) {
            if(CheckSocketForRead(Connection.m_DriversListeningSocket[i])) {
                int cli_connection = accept(Connection.m_DriversListeningSocket[i], &s, &s_size);
                int tmp_switch = 1;
                setsockopt(cli_connection, IPPROTO_TCP, TCP_NODELAY, 
                           (char *)&tmp_switch, sizeof(tmp_switch));
                if(cli_connection >= 0) {
                    _DBG(fprintf(log_fd,"Connecting new client.\n"));
                    ConnectClient(cli_connection, &s, i);
                    continue;
                }
            }
        }
        // Performing data translation from the existing client sockets
        // to the existing model sockets
        ProcessClients();
    } while(!do_shutdown);
    _DBG(fprintf(log_fd, "Requests loop finished.\n"));
}
//--------------------------------------------------------------------------------------------


//---------------------------------------------------------------------------
// Get name of this host. Encapsulate incompatibility between different OSs.
//---------------------------------------------------------------------------
static int getHostName(char *name, int len) {

#if 1
    return gethostname(name, len);
#else
    int ret = sysinfo(SI_HOSTNAME, name, len);
    if(ret > 0 && ret <= len) {
        return 0;
    } else {
        return -1;
    }
#endif

} //getHostName
//--------------------------------------------------------------------------------------------


//---------------------------------------------------------------------------
// Constructs names of drivers (services) by adding prefix:
//    <hostname>:<username>:
//---------------------------------------------------------------------------
static int initDriversNames() {

    // Getting hostname
    char hostName[1024];
    int er_code = getHostName(hostName, 1024);
    if(er_code != 0) {
        printf("Unable to get hostname!\n");
        return er_code;
    }

    // Getting user name
    char *userName = getenv("USER");
#ifdef _WIN32
    if(userName == NULL) {
        userName = getenv("USERNAME");
        if(userName == NULL) {
            char		userNameSys[1024];
            DWORD		nUserNameSize = 1024;
            if(GetUserName(userNameSys,&nUserNameSize)==TRUE)
                userName = userNameSys;
        }
    }
#endif
    if(userName == NULL) {
        printf("Unable to get environment variable USER!\n");
        return -1;
    }

    // Constructing drivers names
    for(int i = 0; i < m_DriversNum; i++) {
        sprintf(m_DriversNames[i], "%s:%s:%s", hostName, userName, m_DriversSuffixes[i]);
    }

    return 0;
} //initDriversNames
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
// Constructs object CmXml, which implements 'CM:Generic' interface.
// This object will reply on all income queries from the clients.
//---------------------------------------------------------------------------
static int initCmXml(const char *xml_file) {

    char filePath[1024];

    if(xml_file != NULL && xml_file[0] != 0) {

        // Use xml-file which was externaly specified.
        strcpy(filePath, xml_file);

    } else {

        // Use pre-defined cm.xml file.
        static const char *fileName = "cm.xml";

        char *psetHome = getenv("PSETHOME");
        if(psetHome == NULL) {
            printf("Unable to get environment variable PSETHOME.\n");
            return 1;
        }

        sprintf(filePath, "%s/lib/%s", psetHome, fileName);
    }

    try {
        cmXml = new CmXml(filePath);
        cmXml->checkConsistency();
    }
    catch(const CmXmlException &ex) {
        printf("\nFATAL ERROR!!!\n");
        printf(ex.getMessage());
        return 1;
    }

    return 0;

} //initCmXml
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
// Checks if the given service is accessible thru socket.
//    Returns: -1 - no name server,
//              0 - service is not running or is not accessible,
//              1 - service exists and is accessible.
//---------------------------------------------------------------------------
int checkService(const char *service_name) {

    _DBG(fprintf(log_fd, "  Checking service '%s'...\n", service_name));

    // Ask name server about this service...
    int	nPort, nAddress;
    int res = NameServerGetService(service_name, nPort, nAddress);
    if(res == 0 || res == -1) {

        _DBG(fprintf(log_fd, "    Service '%s' doesn't exist.\n",
                     service_name));
        return res;
    }
    _DBG(fprintf(log_fd, "    Service '%s' exists.\n", service_name));

    // Create socket...
    _DBG(fprintf(log_fd, "    Creating socket to connect through...\n"));
    int sock = socket(PF_INET, SOCK_STREAM, 0);
    if(sock < 0) {
        _DBG(fprintf(log_fd, "    Unable to create socket!\n"));
        return 0;
    }

    // Open socket...
    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(nPort);
#if 1
    serv_addr.sin_addr.s_addr = htonl(nAddress);
#else
    serv_addr.sin_addr.S_un.S_addr = htonl(nAddress);
#endif
    int e = connect(sock, (struct sockaddr *)&serv_addr, sizeof(sockaddr_in));
    closesocket(sock);
    if(e < 0) {
        _DBG(fprintf(log_fd, 
                     "    Unable to connect to '%s' trough socket!\n",
                     service_name));
        return 0;
    }

    // Service exist and is accessible.
    return 1;

} //checkService
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
// Main function will register the CM service
//---------------------------------------------------------------------------
int main(int argc, char **argv)  {

    socklen_t len;
    int i,j;
    int	nPort,nAddress;
    bool bNameServerStarted = 0;
    char *xml_file = NULL;

    // Get parameter of the command line
    for(i = 1; i < argc; i++) {

        if(!strcmp(argv[i],"-debug")) {

            // Get the name of log-file.
            if(((i+1)<argc) && strlen(argv[i+1])>0) {
                do_debug = 1;
                strcpy(dbgFileName,argv[i+1]);
                _DBG_OPEN();
                i++;
            }

        } else if(!strcmp(argv[i], "-forever")) {

            // The driver will work in "forever" mode.
            work_forever = 1;

        } else if(!strcmp(argv[i], "-xml")) {

            // The name of alternative cm.xml file.
            if(((i+1)<argc) && strlen(argv[i+1])>0) {
                xml_file = argv[i + 1];
                i++;
            }

        } else {

            // Wrong parameter. Show prompt.
            printf("Wrong parameter: %s\n\n", argv[i]);
            printf("Usage:\n");
            printf("       cmdriver [-debug <log-file> | screen]\n");
            printf("                [-xml <cm-xml-file>]\n");
            printf("                [-forever]\n");
            _DBG_CLOSE();
            return 1;
        }
    }

    // Initializing WinSocket
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
    putenv("DIS_NLUNIXSTYLE=1");
#endif 

    // Initializing drivers (services) names
    if(initDriversNames() != 0) {
        return -1;
    }

    // Initializing 'CM:Generic' interface that based on external file 'cm.xml'.
    if(initCmXml(xml_file) != 0) {
        return -1;
    }

    // Checking if we have an instance of this driver and if we
    // have name server running at all
    int res = 0;
    _DBG(fprintf(log_fd,"Checking services...\n"));
    for(j = 0; j < m_DriversNum; j++) {

        // If we found even one service that doesn't work -> restart all.
        res = checkService(m_DriversNames[j]);
        if(res == 0 || res == -1) {
            break;
        }
    } //for j

    switch(res) {
        // No name server running
    case -1: 
        if(LaunchNameServer(START_SERVER) == 0) {
            _DBG(fprintf(log_fd,"Can't start name server.\n"));
            return 0;
        }
        _DBG(fprintf(log_fd,"Name server started.\n"));
        bNameServerStarted = 1;

        // No instance of this driver
    case 0:
        // This block will register ALL CM drivers in our name server

        for(j = 0; j < m_DriversNum; j++) {
            // Any app which needs the CM service must request this socket.
            // It will create separate communication channel in responce
            Connection.m_DriversListeningSocket[j] = CreateListener();
            if(Connection.m_DriversListeningSocket[j] < 0) {
                _DBG(fprintf(log_fd,"Unable to create clients requests socket.\n"));
                for(int k = 0; k < j; k++) {
                    closesocket(Connection.m_DriversListeningSocket[k]);
                }
                return 1;
            }
            _DBG(fprintf(log_fd, "Listening socket for '%s' created.\n",m_DriversNames[j]));
				
            // Registering one CM driver in our name server
            struct sockaddr_in assigned;
            len = sizeof(assigned);

            // Try to connect several times
            for(i = 0; i < 200; i++) {

                // Connect...
                if(getsockname(Connection.m_DriversListeningSocket[j],
                               (struct sockaddr *)&assigned, &len) == 0) {
                    _DBG(fprintf(log_fd, "Registering '%s' driver.\n", m_DriversNames[j]));
                    NameServerRegisterService(m_DriversNames[j], ntohs(assigned.sin_port));
                }

                // Check if connection was established
                if(NameServerGetService(m_DriversNames[j], nPort, nAddress) == 1) {
                    break;
                }
            }

            // We failed to connect -> close all and exit
            if(i == 200) {
                _DBG(fprintf(log_fd, "Unable to register %s service.\n", m_DriversNames[j]));
                for(int k = 0; k <= j; k++) {
                    NameServerUnRegisterService(m_DriversNames[j]);
                    closesocket(Connection.m_DriversListeningSocket[k]);
                }
                return 2;
            }
        }
        break;

        // This driver is already running
    case 1:
        _DBG(fprintf(log_fd,"This driver is already running.\nAborting launch procedure.\n"));
        return 3;
    }

    // OK, here we have name server running and service registered.
    _DBG(fprintf(log_fd,"Ready to process requests.\n"));

    // Process all request in long long, m.b. infinite, loop ...
    RequestsLoop();

    // Closing, terminating, unregistering all ...
    _DBG(fprintf(log_fd,"Terminating driver.\n"));
    for(j = 0; j < m_DriversNum; j++) {
        NameServerUnRegisterService(m_DriversNames[j]);
        closesocket(Connection.m_DriversListeningSocket[j]);
    }
    if(bNameServerStarted == 1) {
        LaunchNameServer(STOP_SERVER);
    }
    _DBG_CLOSE();

    return 0;
} //main

// END OF FILE
