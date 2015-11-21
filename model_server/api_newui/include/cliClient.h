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
#ifndef CLICLIENT_H
#define CLICLIENT_H

#include "tcl.h"
#include "Interpreter.h"
#ifndef ISO_CPP_HEADERS
#include <ctype.h>
#include <fstream.h>
#else /* ISO_CPP_HEADERS */
#include <cctype>
using namespace std;
#include <fstream>
#endif /* ISO_CPP_HEADERS */
#include "genStringPlus.h"
#ifndef _WIN32
#include "ui_INTERP_info.h"
#endif
#include <genString.h>

typedef struct cliClientData {
    cliClientData(){ level = 0; }
    ~cliClientData();
    ostream        *os;
    ostream        *auxos;
    Interpreter    *interp;
    Interpreter    *parent_interp;
    
    genString      shutdown_script;
    ostream        *osp;
    int            client_dead;
    int            client_id;
    int            pid;
    int            level;
    genStringPlus  string;
#ifdef _WIN32
    int            out_file;
    int            in_file;
    unsigned long  client_thread;
    unsigned long  main_thread;
#else
    ui_INTERP_info *hook;
#endif
} *cliClientPtr;

void handle_pipe_buffer(char *buffer, int len, cliClientData *cd);

#ifdef _WIN32
int MonitorClient(cliClientData *data, ostream& outstream);
#endif

#endif
