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
// iMaster.C
//------------------------------------------
// synopsis:
// ....
// description:
// ...
//------------------------------------------
// Restrictions:
// ...
//------------------------------------------
#ifndef ISO_CPP_HEADERS
#include <stdlib.h>
#include <assert.h>
#else /* ISO_CPP_HEADERS */
#include <cstdlib>
namespace std {};
using namespace std;
#include <cassert>
#endif /* ISO_CPP_HEADERS */

#include <cLibraryFunctions.h>
#include <machdep.h> 
#ifndef PROTO
#define PROTO(ARGS) ARGS
#endif
#include "astTree.h"
#include "ast_cplus_tree.h"
#include "MemSeg.h"
#include "setprocess.h"
#include "ast_hook.h"
#include "ast_shared_hook.h"
#include "ast_shared_errors.h"
#include "ast_section.h"
#include "viewerShell.h"
#include "smt.h"

#include <genError.h>

//pairlist** get_declist_ptr ();
//extern "C" void ast_shared_free (void*);
extern "C" void* get_ast_tree_ptr ();

#ifdef _WIN32
extern "C"
{
#endif
int pset_section;
#ifdef _WIN32
}
#endif

#define REPLY \
    rarray[0] = (void *)COMM_INTERP_REPLY; \
    cmgr_instance->send ((int)data->mtype, (void*)&rarray[0], \
                         sizeof(void*) * 20)


astStmts* current_stmt_in_tree; 
astBlock* current_block_in_tree; 
astFunction_decl* current_function_decl; 
astFunction_decl* last_function_decl; 
#ifdef _WIN32
extern "C"
{
#endif
void* ast_tree_ptr;
#ifdef _WIN32
}
#endif
//extern "C" char *get_startup_path ();

/* is initialized when communication from iSlave */

// Master process function declarations

#ifdef GCC2

//=========================================================//

identArr ref_ids;


static ast_shared_errors shared_errors;
ast_shared_errors* ast_get_shared_errors() { return &shared_errors; }

#endif
