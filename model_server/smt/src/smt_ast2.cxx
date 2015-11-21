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

#include "machdep.h"
#include <cLibraryFunctions.h>
#include <messages.h>
#include <Args.h>
extern Args *DIS_args;

#include "smt.h"
#include "xref.h"
#include "setprocess.h"
//#include "errorReport.h"
#include "errorBrowser.h"
#include "astTree.h"
#include "proj.h"

#ifndef _WIN32
#include <sys/param.h>
#endif
#include <customize.h>
#include <genString.h>
//#include <to_cpp.h>
//#include <vpopen.h>
#include <shell_calls.h>
#include <machdep.h>
#include <fcntl.h>
#include <timer.h>
#include <globals.h>
#ifndef ISO_CPP_HEADERS
#include <errno.h>
#else /* ISO_CPP_HEADERS */
#include <cerrno>
using namespace std;
#endif /* ISO_CPP_HEADERS */
#include <psetmem.h>

objSet tokenized_headers;
int smt_discard_mapping;

void remove_from_tokenized (Obj* hd)
{
    if (tokenized_headers.includes (hd)) {
	smt_discard_mapping = 1;
	tokenized_headers.remove (hd);
    }
}


static char * macros_file;

char * smt_get_name_macros_file()
{
    if (!macros_file)
	macros_file = OSapi_tempnam(0, "macros");
    return macros_file;
}

extern "C" smtLanguage get_language();

smtTree *smt_ast_translate_node;
smtHeader *smt_ast_translate_header;
int   smt_ast_translate_line;



static genString current_directory;

char const *pset_get_current_directory ()
{
    return current_directory;
}

void pset_set_current_directory(char const *dir)
{
    current_directory = dir;
}

