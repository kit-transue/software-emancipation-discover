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
#ifndef __disbuild__h__
#define __disbuild__h__

#ifndef ISO_CPP_HEADERS
#include <stdio.h>
#else /* ISO_CPP_HEADERS */
#include <cstdio>
namespace std {};
using namespace std;
#endif /* ISO_CPP_HEADERS */

class objArr;
class projNode;
class projModule;
class genString;

#define LST_F "all.pmod.lst"
#define PRS_F "all.pmod.prs"
#define NIF_F "all.pmod.nif"
#define REM_F "all.update.rem"

#define IFF_EXT "iff"

enum disbuildExit {
    mbQUIT,
    mbWARNING,
    mbERROR,
    mbCRASH,
    mbABORT
};

const char *proj_get_env_disbuild ();
const char *disbuild_get_tmpdir();

const char *get_roots_filename ();
const char *get_pmods_filename ();
const char *get_rules_filename ();

FILE *disbuild_open_model_file (const char *fname, const char *mode);

int disbuild_stop_after_failure ();
//int disbuild_is_cancelled ();

objArr& disbuild_get_pmod_subprojects ();

int disbuild_split_line (char const *arr[], char *line, int params);

projNode * disbuild_get_top_project(const char *nm);

char disbuild_s_or_space(int sz);
void disbuild_get_iff_name (projModule *mod, genString& iff);

void disbuild_exit (int = mbQUIT, const char* = 0, const char* = 0);

#endif
