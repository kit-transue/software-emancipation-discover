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
// include files

#include <cLibraryFunctions.h>
#include <machdep.h>
#include <stdlib.h>
#include <genString.h>
#include <genError.h>
#include <genArr.h>
#include <msg.h>
#include <smt.h>
#include <errorBrowser.h>
#include "path.h"

extern "C"
  {
  int check_smt_asynch_data ();
  void clear_smt_asynch_data ();
  };


// type definitions

typedef genString* genStringPtr;
genArr(genStringPtr);

typedef void* (*voidFuncCharPtr)(char*);
genArr(voidFuncCharPtr);


RelClass(steDocument);

// external functions

steDocumentPtr ste_import_ascii_file(char*);

static int import_canceled = false;

//------------------------------------------
// Local file importing functions
//------------------------------------------

static void* import_C(char* fn)
{
    return smt_prog_import(fn, NULL, smt_C);
}

static void* import_Cplusplus(char* fn)
{
    return smt_prog_import(fn, NULL, smt_CPLUSPLUS);
}

extern "C" void browser_import(
    char *nothing, const char* pn, const char* file_type)
{
    Initialize(browser_import);

    if(!file_type)
        file_type = "plain";

    genString ret_path;
    project_convert_filename(pn, ret_path);
    if (!ret_path.length()) {
        msg("File/directory not found: $1", error_sev) << pn << eom;
        return;
    }

    pn = (char *)ret_path.str();

    voidFuncCharPtr import = NULL;

    if(!strcmp(file_type, "c") || !strcmp(file_type, "esql-c"))
        import = import_C;
    else if(!strcmp(file_type, "cplusplus") || !strcmp(file_type, "esql-cplusplus"))
        import = import_Cplusplus;
    else if (!strcmp(file_type, "makefile")) {
        clear_smt_asynch_data ();
        return;
    }

    if (import != import_C && import != import_Cplusplus)
        clear_smt_asynch_data ();

    genError *err = NULL;
    genString pathname = pn;
    msg("Importing file: $1", normal_sev) << (char *)pathname << eom;

    if(import && (!import_canceled))
        (*import)(pathname);
    else {
        msg("Cannot import file: $1", error_sev) << (char *)pathname << eom;
        return;
    }

    if (!check_smt_asynch_data())
        msg("Finished importing file: $1", normal_sev) << (char *)pathname << eom;
}
