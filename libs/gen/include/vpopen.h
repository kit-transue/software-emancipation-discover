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
#ifndef _vpopen_h
#define _vpopen_h
#include <cLibraryFunctions.h>
#if 0
// vpopen.h
//------------------------------------------
// synopsis:
// Declaration of interface similar to popen, but using vfork instead of fork.
//------------------------------------------
// Restrictions:
// YO! Let's keep this one usable by C code, please.
//------------------------------------------
#endif

#ifndef ISO_CPP_HEADERS
#include <stdio.h>
#else /* ISO_CPP_HEADERS */
#include <cstdio>
using namespace std;
#endif /* ISO_CPP_HEADERS */

#ifndef _prototypes_h
#include <prototypes.h>
#endif


#ifdef __cplusplus
int vsystem(const char*);	/* system() call using vfork. */
int v_system(char*);		/* alias for preceeding function. */
#endif


BEGIN_C_DECL

int vp_find_pid PROTO((FILE*));

FILE* vpopen PROTO((
    const char* Bourne_shell_command, const char* r_or_R_or_w));

FILE* vpopen_sync PROTO((
    const char* Bourne_shell_command, const char* r_or_R_or_w));

FILE* vpopen2way PROTO((
    const char* Bourne_shell_command, FILE **read_pipe,  FILE **write_pipe));

int vpclose PROTO((FILE*));

END_C_DECL


/*
 * ------------------------------------------
 * $Log: vpopen.h  $
 * Revision 1.4 2000/07/10 23:04:21EDT ktrans 
 * mainline merge from Visual C++ 6/ISO (extensionless) standard header files
 * Revision 1.2.1.6  1994/07/27  21:49:19  aharlap
 * declared vpopen_sync()
 *
 * Revision 1.2.1.5  1993/04/06  20:07:47  glenn
 * Declare vsystem().  Make file C-compatible again.
 *
 * Revision 1.2.1.4  1993/02/05  14:02:47  davea
 * Added prototype for vpopen2way
 *
 * Revision 1.2.1.3  1992/11/11  14:53:02  oak
 * Added v_system call.
 *
 * Revision 1.2.1.2  1992/10/09  18:22:06  kol
 * moved &log
 *
 * Revision 1.2  92/08/01  14:10:40  smit
 * declare find_pid interface.
 * 
 * Revision 1.1  92/06/28  15:45:32  glenn
 * Initial revision
 * 
 *------------------------------------------
 */

#endif /* _vpopen_h */
