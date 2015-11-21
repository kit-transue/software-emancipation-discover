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

#include <cLibraryFunctions.h>
#include <stdio.h>
#include <string.h>
#ifdef _WIN32
#include <stdlib.h>
#include <malloc.h>
#endif

#include "astEnums.h"

#include "tree.h"

static char *standard_tree_code_name[] = {
#define MCTREECODE(SYM, NAME, CLASS, NKIDS, TYPE) NAME,
#include "discover-tree-codes.def"
#undef MCTREECODE
};

static char *cplus_tree_code_name[] = {
  "@@dummy",
  // #include "cp/tree.def"  XXX: unified in standard_tree_code_name.
};

char **paraset_tree_code_name = 0;

tree get_first_child_of_block ();

/*
//
// Description. Given a NODE, which could be _DECL or _STMT,
// this function returns NEXT child of LET_STMT.
// Usually it is the next node of the same kind. 
// So in most of cases it would be simple TREE_CHAIN (node).
//
// This function handles the following exception.
//
// For MC_VAR_DECL this function returns first statement (=block->STMT_BODY()), 
//   when there no TREE_CHAIN ().
//
*/
tree get_next_child_of_block (node, let_stmt)
     tree node;
     tree let_stmt;
     /* node could be _DECL or _STMT.  04.10.92 NY */
{
  tree t;
  /* tree let_stmt; *09.23.93*/
  
  if ((t = TREE_CHAIN (node)))
    return t;

  switch (TREE_CODE (node))
    {
    case MC_PARM_DECL:
      OSapi_fprintf (stderr, "\nError: MC_PARM_DECL in get_next_child_of_block");
      break;
    case MC_VAR_DECL:
    case MC_CONST_DECL:
      if(!let_stmt) /*09.23.93*/
	let_stmt = DECL_CONTEXT (node); 
	
      if (let_stmt)
	{
	  if (STMT_BODY (let_stmt) == STMT_VARS (let_stmt))
	    /* C++ case, decls and stmts in one chain - so, this is end of it */
	    return 0;

	  t = STMT_BODY (let_stmt);
	  if (t == get_first_child_of_block (let_stmt))
	    t = 0;
	}
      else t = 0;
      break;
    case MC_TYPE_DECL:
    case MC_FUNCTION_DECL:
      t = 0;
      break;
    case MC_LABEL_DECL: 
      /*** --- removed.04.10.92 - NY. I believe this is DEFAULT in SWITCH statement */
      /*** t = DECL_INITIAL (decl); ***/
      OSapi_fprintf (stderr, "\nError: MC_LABEL_DECL in get_next_child_of_block");
      t = 0;
      break;
      default:
      /* should be stmt, which does not have TREE_CHAIN, return 0 */
      t = 0;
      break;
    }
  return t;
}

/*
//
// Description. This function returns the FIRST node for MC_COMPOUND_EXPR.
// If block contains variables, it returns the first variable.
// If not, function returns STMT_BODY.
*/
tree
get_first_child_of_block (block)
     tree block;
{
  tree t = 0;
  tree first = 0;

  first = STMT_VARS (block);

  if (first && (first != STMT_BODY (block)))
    {
      for (t = STMT_BODY (block); t ; t = TREE_CHAIN (t))
	if (t == first)
	  {
	    first = STMT_BODY (block);
	    break;
	  }
    }
  else
    first = STMT_BODY (block);

  if (TREE_START_LINE (block) == 0 &&
      TREE_END_LINE (block) == 0 &&
      first)
    if (TREE_CODE (first) == MC_VAR_DECL)
    {
      /* skip var_decls */
      tree t;
      for (t = first; t; t=TREE_CHAIN (t))
	if (TREE_CODE (t) != MC_VAR_DECL)
	  break;
      if (t && (TREE_CODE (t) == MC_FOR_STMT
                || TREE_CODE (t) == MC_WHILE_STMT
                || TREE_CODE (t) == MC_DO_STMT))
	first = t;
    }
  else
    if (TREE_CODE (first) == LET_STMT && !TREE_CHAIN (first))
      first = get_first_child_of_block (first);
  return first;
}

void paraset_init_tree_codes ()
{
  paraset_tree_code_name = (char **) malloc (sizeof (char *) * LAST_CPLUS_TREE_CODE);

  memcpy ((char *) paraset_tree_code_name, (char *) standard_tree_code_name, 
	 sizeof (standard_tree_code_name));

  memcpy ((char *)(paraset_tree_code_name + (int) LAST_AND_UNUSED_TREE_CODE),
	  (char *)cplus_tree_code_name,
	 (LAST_CPLUS_TREE_CODE - (int)LAST_AND_UNUSED_TREE_CODE) * sizeof (char *));
}



