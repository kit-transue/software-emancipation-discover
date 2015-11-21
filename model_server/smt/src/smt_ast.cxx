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
// File smt_ast.C
#include "smt.h"

#ifndef ISO_CPP_HEADERS
#include <stdlib.h>
#include <stdlib.h>
#include <assert.h>
#else /* ISO_CPP_HEADERS */
#include <cstdlib>
namespace std {};
using namespace std;
#include <cstdlib>
#include <cassert>
#endif /* ISO_CPP_HEADERS */

#include "astTree.h"
#include "ast_cplus_tree.h"
#include "ast_hook.h"
#include "cLibraryFunctions.h"
#include "charstream.h"
#include "cmd.h"
#include "customize.h"
#include "ddict.h"
#include "machdep.h"
#include "messages.h"
#include "objOper.h"
#include "objRelation.h"
#include "smt_categories.h"
#include "timer.h"
// XXX: #include "top_widgets.h"
#include "transaction.h"
#include "xref.h"

// PD: REPLACE

extern objSet sh_set;

int xref_hook_turn_off = 0;

objArr_Int astTree_arr; /* used to keep track of ast decl which has smt and
			    is not covered by decl specs or globals */

extern "C"  char * smt_truncate_filename(char * name)
{
  if(!name) return name;
  char * nn = name;
  while(*(++name))
     if(*name == '/')
        nn = name + 1;
  return nn;
}	


// Set link betweem SMT and AST nodes

void smt_set_ast_many_to_one(smtTree* smt, astTree* ast)
  /* one smt can have many ast 
   */
{
   Initialize(smt_set_ast);

  if (smt && ast)
  {
    TREE_SMT_NODE (ast) = smt;
    ast->smt_mapped = 1;
    if(!smt->ref_ast)
      smt->ref_ast = ast;
    astTree_arr.insert_last((int)ast);

    smtHeader *h = checked_cast(smtHeader,smt->get_header());
    if (h) h->set_ast_exist(1);
  }
}


void smt_set_ast(smtTree* smt, astTree* ast)
{
   Initialize(smt_set_ast);

  if (smt && ast)
  {
    smt_unset (smt);
    ast_unset (ast);
    smt->ref_ast = ast;
    TREE_SMT_NODE (ast) = smt;
    ast->smt_mapped = 1;
    astTree_arr.insert_last((int)ast);
    smtHeader *h = checked_cast(smtHeader,smt->get_header());
    if (h) h->set_ast_exist(1);    
  }
}

// Get associated SMT node dor AST node
smtTree* ast_get_smt(astTree* ast)
{
  return ast ? TREE_SMT_NODE (ast) : 0;
}

// Get associated AST node dor SMT node
astTree* smt_get_ast(smtTree* smt)
{
  return smt ? smt->ref_ast : 0;
}

// Destroy link between SMT and AST nodes from side of SMT
void smt_unset (smtTree* smt)
{
  if (smt && smt->ref_ast)
  {
    TREE_SMT_NODE (smt->ref_ast) = 0;
    smt->ref_ast->smt_mapped = 0;
    smt->ref_ast = 0;
  }
}

// Destroy link between SMT and AST nodes from side of AST
void ast_unset (astTree* ast)
{
  if (ast && TREE_SMT_NODE (ast))
  {
    TREE_SMT_NODE (ast)->ref_ast = 0;
    ast->smt_mapped = 0;
    TREE_SMT_NODE (ast) = 0;
  }
}
