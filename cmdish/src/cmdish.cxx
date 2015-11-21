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
  #include <winsock.h>
  #include <io.h>
#else
  #include <netdb.h>
  #include <unistd.h>
  #include <arpa/inet.h>
  #include <netinet/in.h>
  #include <sys/socket.h>
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
   #endif
#endif

#include "nameServCalls.h"
#include "debug.h"
#include "startproc.h"

#ifndef _WIN32
  #define closesocket(sock) close(sock)
#endif


int		do_debug = 0;
FILE*	log_fd   = NULL;
char	dbgFileName[1024];


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


static char* getServiceName() {

	// Getting hostname
	char hostName[1024];
	int er_code = getHostName(hostName, 1024);
	if(er_code != 0) {
		printf("Error\n" \
               "Unable to get hostname!\n" \
               "\n");
		return NULL;
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
		printf("Error\n" \
               "Unable to get environment variable USER!\n" \
               "\n");
		return NULL;
	}

	// Constructing drivers names
    int len = strlen(hostName) + strlen(userName) + 15;
    char *servName = new char[len];
	sprintf(servName, "%s:%s:CM:Generic", hostName, userName);

	return servName;

} //getServiceName

static char* receivePackage(int sock) {

    int len = 0;
    char* package = NULL;

    ReceiveInteger(sock, &len);
    if(len > 0) {
        package = new char [len + 1];
        ReceiveBuffer(sock, package, len);
        package[len] = 0;

        int shift = 0;
        if(package[0] == '1') {
            shift++;
            if(package[1] == '\n') {
                shift++;
            }
        }

        if(shift > 0) {
            for(int i = shift; i <= len; i++) {
                package[i - shift] = package[i];
            }
        }
    } else {

        // Just return empty string, not NULL.
        package = new char [1];
        package[0] = 0;
    }

    return package;

} //ReceivePackage


static void sendPackage(int sock, char *package) {

    int len = strlen(package);
    SendInteger(sock, len);
    SendBuffer(sock, package, len);

} //SendPackage


//---------------------------------------------------------------------------
// Find the given service in name server.
// Check if this service exists and is accessible.
//    Returns: -1 - no name server,
//              0 - service is not running or is not accessible,
//              1 - service exists and is accessible.
//---------------------------------------------------------------------------
int findService(const char *service_name, int &service_port, int &tcp_addr) {

    _DBG(fprintf(log_fd, "  Checking service '%s'...\n", service_name));

    // Ask name server about this service...
    int res = NameServerGetService(service_name, service_port, tcp_addr);
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
    serv_addr.sin_port = htons(service_port);
#if 1
    serv_addr.sin_addr.s_addr = htonl(tcp_addr);
#else
    serv_addr.sin_addr.S_un.S_addr = htonl(tcp_addr);
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

} //findService
//---------------------------------------------------------------------------


int main(int argc, char* argv[]) {

    // We will return this code as a result of this program.
    // We will set non-zero value if some error occures.
    int retCode    = 0;

    int sock       = -1;
    int err        = 0;
	int	nPort      = 0;
    int nAddress   = 0;
    char *cm       = NULL;
    char *query    = NULL;
    char *reply    = NULL;
    char *errMsg   = NULL;
    char *servName = NULL;

    // Parse arguments
    for(int i = 1; i < argc - 1; i++) {
        if(!strcmp(argv[i], "-cm")) {

            int len = strlen(argv[i + 1]);
	        if(len > 0) {
		        cm = new char [len + 1];
                strcpy(cm, argv[i + 1]);
                cm[len] = 0;
	        }
        } else if(!strcmp(argv[i], "-query")) {

            int len = strlen(argv[i + 1]);
	        if(len > 0) {
		        query = new char [len + 1];
                strcpy(query, argv[i + 1]);
                query[len] = 0;
	        }
        } else if(!strcmp(argv[i], "-debug")) {

            if(i + 1 < argc && strlen(argv[i + 1]) > 0) {
                do_debug = 1;
                strcpy(dbgFileName, argv[i + 1]);
                _DBG_OPEN();
            }
        }
    }

    // Query string is strictly necessary
    if(!query) {
        printf("Error\n" \
               "Command line doesn't specify query to 'cmdriver'!\n" \
               "\n" \
               "Usage:\n" \
               "  cmdriver -query <query-string> " \
                         "[-cm <cm-name>] " \
                         "[-debug screen | <log-file>]\n" \
               "\n");
        retCode = -1;
        goto ex_;
    }

#ifdef _WIN32
    // Initialize WinSockets
    WORD    wVersionRequested;
    WSADATA wsaData;

	_DBG(fprintf(log_fd, "Initializing WinSockets...\n"));
    wVersionRequested = MAKEWORD(1, 1);
    err = WSAStartup(wVersionRequested, &wsaData);
    if(err != 0) {
	    printf("Error\n" \
               "No sockets available!\n" \
               "\n");
        retCode = err;
        goto ex_;
    }
    putenv("DIS_NLUNIXSTYLE=1");
#endif

    // Get name of service "CM:Generic"
	_DBG(fprintf(log_fd, "Getting service name...\n"));
    servName = getServiceName();

    if(servName == NULL) {
        retCode = -1;
        goto ex_;
    }
	_DBG(fprintf(log_fd, "   '%s'\n", servName));

    if(strcmp(query, "wait") == 0) {

        // Wait until driver starts.
	    _DBG(fprintf(log_fd, "Waiting until the 'cmdriver' starts...\n"));
        for(int i = 0; i < 200; i++) {
            err = findService(servName, nPort, nAddress);
            if(err == 1) {
                break;
            }
        }

        // The driver is not available.
        if(err != 1) {
			printf("Error\n" \
                   "Unable to connect to the 'cmdriver'.\n" \
                   "\n");
            retCode = -1;
        } else {
	        _DBG(fprintf(log_fd, "   The 'cmdriver' started.\n"));
            retCode = 0;
        }

        goto ex_;

    } else {

        // Find service of 'cmdriver'.
	    _DBG(fprintf(log_fd, "Finding the service...\n"));
        err = findService(servName, nPort, nAddress);
    }

	// No name server running
	if(err == -1) {

        // Try to run name server
	    _DBG(fprintf(log_fd, "Launching the name server...\n"));
		if(LaunchNameServer(START_SERVER) == 0) {
			printf("Error\n" \
                   "Can't start name server.\n" \
                   "\n");
			retCode = -1;
            goto ex_;
		}

        // Wait until name server starts
	    _DBG(fprintf(log_fd, "Waiting until the name server starts...\n"));
        int sock = -1;
        for(int i = 0; i < 200; i++) {
	        _DBG(fprintf(log_fd, "."));
            sock = ConnectToNameServer();
            if(sock != -1) {
		        break;
            }
        }
        _DBG(fprintf(log_fd, "\n"));

        // M.b. we couldn't connect to name server
        if(sock != -1) {
	        _DBG(fprintf(log_fd, "   The name server started.\n"));
            closesocket(sock);
        } else {
	        printf("Error\n" \
                   "Can't connect to name server.\n" \
                   "\n");
			retCode = -1;
            goto ex_;
        }

        // Need to start 'cmdriver'.
        err = 0;
    }

    // No 'cmdriver' running
    if(err == 0) {

        // Construct command line for starting 'cmdriver'.
	    char cmdLine[1024];
	    char *home = getenv("PSETHOME");
        strcpy(cmdLine, "\"");
        if(home != NULL) {
            strcat(cmdLine, home);
            strcat(cmdLine, "/bin/cmdriver");
        } else {
            strcat(cmdLine, "cmdriver");
        }
        strcat(cmdLine, "\"");

        // 'cmdriver' will work "forever".
//        strcat(cmdLine, " -forever");

        if(do_debug) {
            strcat(cmdLine, " -debug screen");
        }

        // Try to run 'cmdriver'.
	    _DBG(fprintf(log_fd, "Launching '%s'...\n", cmdLine));
        if(start_new_process(cmdLine, START_NO_WAIT) == -1) {
	        printf("Error\n" \
                   "Can't start 'cmdriver'.\n" \
                   "\n");
			retCode = -1;
            goto ex_;
        }

        // Wait until 'cmdriver' starts.
	    _DBG(fprintf(log_fd, "Waiting until the 'cmdriver' starts...\n"));
        for(int i = 0; i < 200; i++) {
            err = findService(servName, nPort, nAddress);
            if(err == 1) {
                break;
            }
        }
        if(err == 1) {
	        _DBG(fprintf(log_fd, "   The 'cmdriver' started.\n"));
        }
    }

    // Unfortunately we couldn't run (find) 'cmdriver'.
    if(err != 1) {
        printf("Error\n" \
               "'cmdriver' not found!\n" \
               "\n");
        retCode = (err != 0) ? err : -1;
        goto ex_;
    }

    // Create socket for connecting to 'cmdriver'.
    _DBG(fprintf(log_fd, "Creating socket for connecting to the 'cmdriver'...\n"));
    sock = socket(PF_INET, SOCK_STREAM, 0);
    if(sock < 0) {
        printf("Error\n" \
               "Unable to create socket!\n" \
               "\n");
        retCode = sock;
        goto ex_;
    }

    // Connect to 'cmdriver'.
    _DBG(fprintf(log_fd, "Connecting to the 'cmdriver'...\n"));
    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(nPort);
#if 1
    serv_addr.sin_addr.s_addr = htonl(nAddress);
#else
    serv_addr.sin_addr.S_un.S_addr = htonl(nAddress);
#endif
    err = connect(sock, (struct sockaddr *)&serv_addr, sizeof(sockaddr_in));
    if(err < 0) {
        printf("Error\n" \
               "Unable to connect to 'cmdriver' trough socket!\n" \
               "\n");
        retCode = err;
        goto ex_;
    }

    if(strcmp(query, "register") == 0 || strcmp(query, "unregister") == 0) {

        _DBG(fprintf(log_fd, "Performing query '%s'...\n", query));
        // Special queries 'register' or 'unregister'.
        sendPackage(sock, query);
        reply = receivePackage(sock);
        if(strcmp(reply, "Error") == 0) {
            // Get description of error
            errMsg = receivePackage(sock);
            printf("Error\n");
            printf(errMsg);
            int err_len = strlen(errMsg);
            if(errMsg[err_len - 1] != '\n') {
                printf("\n");
            }
            printf("\n");
            retCode = -1;
            goto ex_;
        }
        delete [] reply;
        reply = NULL;

    } else {

        // Register in the 'cmdriver' as a client.
        _DBG(fprintf(log_fd, "Registering in the 'cmdriver'...\n"));
        sendPackage(sock, "register");
        reply = receivePackage(sock);
        if(strcmp(reply, "Error") == 0) {
            // Get description of error
            errMsg = receivePackage(sock);
            printf("Error\n");
            printf(errMsg);
            int err_len = strlen(errMsg);
            if(errMsg[err_len - 1] != '\n') {
                printf("\n");
            }
            printf("\n");
            retCode = -1;
            goto ex_;
        }
        delete [] reply;
        reply = NULL;

        // Connect to particular CM system
        if(cm != NULL && retCode == 0) {

            _DBG(fprintf(log_fd, "Connecting to '%s'...\n", cm));

            int cm_len = strlen(cm);
            char *cmd = new char [cm_len + 10];
            sprintf(cmd, "connect\t%s", cm);
            sendPackage(sock, cmd);
            delete [] cmd;
            cmd = NULL;

            reply = receivePackage(sock);
            if(strcmp(reply, "Error") == 0) {
                // Get description of error
                errMsg = receivePackage(sock);
                printf("Error\n");
                printf(errMsg);
                int err_len = strlen(errMsg);
                if(errMsg[err_len - 1] != '\n') {
                    printf("\n");
                }
                printf("\n");
                retCode = -1;
            }

            delete [] reply;
            reply = NULL;
        }

        // Perform query
        if(query != NULL && retCode == 0) {
            _DBG(fprintf(log_fd, "Performing query '%s'...\n", query));
            sendPackage(sock, query);
            reply = receivePackage(sock);
            if(strcmp(reply, "Error") == 0) {
                // Get description of error
                errMsg = receivePackage(sock);
                printf("Error\n");
                printf(errMsg);
                int err_len = strlen(errMsg);
                if(errMsg[err_len - 1] != '\n') {
                    printf("\n");
                }
                printf("\n");
                retCode = -1;
            } else {
                // This is necessary result from 'cmdriver'
                printf(reply);
            }

            delete [] reply;
            reply = NULL;
        }

        // Unregister in the 'cmdriver' as a client.
        _DBG(fprintf(log_fd, "\nUnregistering from the 'cmdriver'...\n"));
        sendPackage(sock, "unregister");
        reply = receivePackage(sock);
        if(strcmp(reply, "Error") == 0) {
            // Get description of error
            errMsg = receivePackage(sock);
            retCode = -1;
            goto ex_;
        }
        delete [] reply;
        reply = NULL;
    }

ex_:
    if(sock > 0) {
        closesocket(sock);
    }
    delete [] cm;
    cm = NULL;
    delete [] query;
    query = NULL;
    delete [] reply;
    reply = NULL;
    delete [] errMsg;
    errMsg = NULL;
    delete [] servName;
    servName = NULL;

    return retCode;

} //main

//END OF FILE
