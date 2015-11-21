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
#include "Args.h"
#include "cLibraryFunctions.h"
#include "customize.h"
#include "genError.h"
#include "genTry.h"
#include "machdep.h"
#include "messages.h"
#include "msg.h"
#include "options.h"
#include "psetmem.h"
#include "setprocess.h"
#include "xxinterface.h"


void print_build_date();
void create_tmp_dir();
int dd_set_hook();
void init_rel_ptrs();
void load_and_refresh_projects(genString const &xref_projname);
void load_project_rtl (int num_of_pdf_files, genString* pdf_file);
void InitializeSocketServer(const char *service);
int service_loop(long);
int service_init(int argc, char**argv);
extern bool cli_exit;

int
minimal_start(char const *pdf)
{
    genString pdf_file[2];
    int num_of_pdf_files = 1;
    pdf_file[0] = pdf;

    create_tmp_dir();
#ifndef _WIN32
    struct rlimit rlm;
    getrlimit(RLIMIT_NOFILE,  &rlm);
    if (rlm.rlim_cur < 128 && rlm.rlim_max > 128) {
        rlm.rlim_cur = 128;
        setrlimit (RLIMIT_NOFILE, &rlm);
    }
#endif
    // initialize model data structures
    dd_set_hook();
    init_rel_ptrs();

    print_build_date();

#if 0
    char *argv[] = {"cov-extend", "-service", "cov-extend", "-pdf", "$ADMINDIR/pdf/build.pdf", 0};
    service_init(5, argv);
#endif
    builtin_mmgr = new MemoryManager(NULL);

    load_project_rtl( num_of_pdf_files, pdf_file);  
    load_and_refresh_projects(NULL);
#if XXX_want_service_loop
    genString tmp = "Dislite:cov-extend";
    InitializeSocketServer(tmp);

    cli_exit = false;
    return service_loop(-1l);
#endif
}

