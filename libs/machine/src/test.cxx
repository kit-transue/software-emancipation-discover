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
extern "C" char* bcopy(char*,char*,int); //---> NT test of merge
extern "C" char* bzero(char*,int);
extern "C" char* rindex(char*,char);


#ifdef sun5 /*NT*/
#include <sysent.h>
#ifndef ISO_CPP_HEADERS
#include <signal.h>
#else /* ISO_CPP_HEADERS */
#include <csignal>
using namespace std;
#endif /* ISO_CPP_HEADERS */
#include <sys/param.h>
#include <sys/stat.h>
#include <fcntl.h>
#ifndef ISO_CPP_HEADERS
#include <string.h>
#else /* ISO_CPP_HEADERS */
#include <cstring>
#endif /* ISO_CPP_HEADERS */
#include <unistd.h>
#ifndef ISO_CPP_HEADERS
#include <setjmp.h>
#include <stdlib.h>
#else /* ISO_CPP_HEADERS */
#include <csetjmp>
#include <cstdlib>
#endif /* ISO_CPP_HEADERS */
#include <sys/ptrace.h>
#include <sys/mman.h>
#ifndef ISO_CPP_HEADERS
#include <time.h>
#else /* ISO_CPP_HEADERS */
#include <ctime>
#endif /* ISO_CPP_HEADERS */
#ifdef Solaris_5_3
#include <sys/rusage.h>
#endif
#include <sys/wait.h>
#include <sys/systeminfo.h>
#include <psetmem.h>

extern "C" char *getwd(char* pathname)
{
    return getcwd(pathname, MAXPATHLEN);
}

extern "C" char* rindex (char *s1, char c)
{
    return (char*)strrchr (s1, c);
}

extern "C" char* index (char *s1, char c)
{
    return (char*)strchr (s1, c);
}

extern "C" char* bcopy (char *s1, char* s2, int c)
{
    return (char*)memcpy (s2, s1, c);
}

extern "C" int bcmp(char *s1, char* s2, int c)
{
    return memcmp(s1, s2, c);
}

extern "C" char* bzero(char *s, int c)
{
    return (char*)memset(s, 0, c);
}
#endif
