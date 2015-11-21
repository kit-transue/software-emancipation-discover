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

/* ============================================================== */
/********   search function by name  *****************/

/**************************************************************/
/* given argument_stmt return beginning of new chain */
/* Can be used when add control stmt into existing chain of stmts.
   New control stmt then should be chained with returned stmt */
/* astStmts *get_next_stmt (astStmts*) */
/* ==============================================================*/

#ifndef ISO_CPP_HEADERS
#include <stdio.h>
#include <iostream.h>
#include <strstream.h>
#include <string.h>
#include <stdlib.h>
#else /* ISO_CPP_HEADERS */
#include <cstdio>
namespace std {};
using namespace std;
#include <iostream>
#include <cstring>
#include <cstdlib>
#endif /* ISO_CPP_HEADERS */

#ifndef PROTO
#define PROTO(ARGS) ARGS
#endif

#include "astTree.h"
#include "ast_cplus_tree.h"
#define MAX_STRING 4096



/********  get If then body *******/
astStmts* getIf_then_body (astIf* stmt)
{
  if (stmt)
    return (astStmts*) stmt->STMT_THEN();
  else
    return (astStmts*) NULL_TREE;
}

/********  get If else body *******/
astStmts* getIf_else_body (astIf* stmt)
{
  if (stmt)
    return (astStmts*) stmt->STMT_ELSE();
  else
    return (astStmts*) NULL_TREE;
}
