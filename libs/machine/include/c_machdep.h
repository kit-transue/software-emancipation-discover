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
#ifndef _c_machdep_h
#define _c_machdep_h

#ifndef __cplusplus

#if defined(hp10)

#include <sys/termios.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include <sys/file.h>
#include <sys/time.h>
#include <string.h>
#include <unistd.h>

#define TCGETS TCGETATTR
#define TCSETS TCSETATTR

#define OSapi_bcopy(a,b,c) memcpy (b,a,c)
#endif /* hp10 */

#if defined(irix6)
extern void bcopy(const void *, void *, int);
#define OSapi_bcopy bcopy
#endif

#ifdef sun5

#include <unistd.h>
#include <string.h>

#define getwd getcwd
#define OSapi_bcopy(a,b,c) memcpy (b,a,c)
#define OSapi_rindex(s,c) strchr(s,c)
static int OSapi_getdtablesize() {return sysconf(_SC_OPEN_MAX);}

#endif /* sun5 */

#ifndef irix6
#define OSapi_vfork vfork
#else
#define OSapi_vfork fork
#endif

#endif /* __cplusplus */

#define OSapi_bzero(a,b) memset(a,0,b)

#endif /* _c_machdep_h */
