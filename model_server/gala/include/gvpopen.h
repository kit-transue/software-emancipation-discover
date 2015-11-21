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
//-----------------------------------------------------------------------------
// gvpopen.h
//
// Generic sub-process control functions
//-----------------------------------------------------------------------------

#ifndef _gvpopen_h
#define _gvpopen_h

#ifndef vportINCLUDED
#include <vport.h>
#endif

#ifndef vcharINCLUDED
#include vcharHEADER
#endif

#ifndef vstdargINCLUDED
#include vstdargHEADER
#endif

typedef void notifier_function(void);

extern "C" FILE* vpopen(vchar* sh_command, vchar* type);
extern "C" FILE* vpopen_tmp(vchar* sh_command, vchar* type, vchar* outputfile, notifier_function *);
extern "C" FILE* vpopen2way(vchar* sh_command, FILE **read_pipe_ptr, FILE **write_pipe_ptr);
extern "C" int vpclose(FILE* stream);
extern "C" int vp_find_pid(FILE*);
int vsystem(const char* cmd);
extern "C" char* vpopen_sync(vchar* sh_command, vchar* type);
extern "C" void vpkill(int,int);

#endif
