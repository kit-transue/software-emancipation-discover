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
#ifndef LIB_HEADER_FILE
#define LIB_HEADER_FILE
void message( char *txt, char *typ);

#ifndef _WIN32
  #define _MAX_PATH	1024
  #define SOCKET          int
  #define INVALID_SOCKET  -1
  #define FALSE           0
  #define TRUE            1
  #define SOCKET_ERROR    -1 
  
  #define closesocket(sock) close(sock)

#endif

char * getMessage () ;
int getopt(int argc, char* const argv[], const char* optstring);

#define MAX_PENDING_CONNECTS  4       // Maximum length of the queue 
#define HOSTNAME        "localhost"   // Server name string
                                      // This should be changed
                                      // according to the server.
extern char *optarg;
extern int optind;

#define MAXLISPEXPR (_MAX_PATH * 2)
#define MAXMSGSZ MAXLISPEXPR
#define EMACSOK "OK"
#define EMACSERROR "ERROR"


#endif


