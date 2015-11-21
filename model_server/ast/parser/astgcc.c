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

#include <string.h>
#include "config.h"
#include "tree.h"
#include "cp/cp-tree.h"
#if 0 
#include <c_machdep.h>
#endif

#define MARK_OLD 0x1
#define TREE_MARK_OLD(NODE,TEMP) ((TEMP=(int)TREE_CHAIN (NODE), TEMP |= MARK_OLD, TREE_CHAIN(NODE) = (tree)TEMP))
#define TREE_CHAIN_MARKED(NODE,TEMP) ((TEMP=(int)TREE_CHAIN(NODE),TEMP&=~MARK_OLD, (tree)TEMP))
extern int ast_hook;
static int print_no_mark PROTO((tree));
static int mark_old_ast PROTO((tree));

typedef enum {NULLOP=0, FIRST, AFTER, REPLACE, REPLACE_RIGHT,
		CHANGE_NAME} astOperType;

extern int flag_evaluate;
extern int lineno;
extern char *input_filename, *main_input_filename;

/*** is needed for ast_is_break_legal() ***/
static tree current_stmt_in_tree;

extern tree get_identifier ();
extern tree lookup_name ();

extern tree get_current_function_decl ();
extern tree get_current_block_in_tree ();
extern void set_current_block_in_tree ();
extern tree ast_get_current_class_decl();
extern void ast_set_current_class_decl();
extern tree ast_get_current_class_type();
extern void ast_set_current_class_type();

extern char *ast_get_fname ();

extern int to_parser ();
extern tree get_last_node ();
extern tree get_preserved_last_node ();
extern tree get_id ();

/* 11.22.94 *tree build_stmt_from_str ();*/

static int check_insertion ();
static tree search_current_stmt_in_tree();
static tree search_current_stmt ();

/*
tree 
ast_get_global_decl (name)
     tree name;
{  
  return IDENTIFIER_GLOBAL_VALUE (name);
}
*/

tree
build_exp_from_str (bufferp)
     char *bufferp;
{
  int ret_code;
  char * buf;
  int which;

  buf = (char*) malloc (strlen (bufferp) + 5);

  if ((which = which_parser()) == 0)
      /* C parser */
      strcpy (buf, "((");
  else
      /* C++ parser */
      strcpy (buf, "<(");
  strcat (buf, bufferp);

  if (which == 0)
    strcat (buf, "))");
  else
    strcat (buf, ")>");

  ret_code = to_parser (buf);
  free (buf);

  if (ret_code > 0)
    return NULL_TREE;
  else
    return  get_last_node ();
}

static tree    /*ibm*/
build_node_from_str (bufferp)
     char *bufferp;
{
  int ret_code;
  char * buf;
  tree node = NULL_TREE;

  int which;

  buf = (char*) malloc (strlen (bufferp) + 8);
  if ((which = which_parser()) == 0)
      /* C parser */
      strcpy (buf, "((");
  else
      /* C++ parser */
      strcpy (buf, "<(");

  if (!strcmp (bufferp, ";"))
    strcat (buf, "\";\"");
  strcat (buf, bufferp);

  if (which == 0)
    strcat (buf, "))");
  else
    strcat (buf, ")>");

  ret_code = to_parser (buf);
  free (buf);

  if (ret_code > 0)
    ;  /* do nothing, will return node == NULL_TREE */
  else
    {
      if (which_parser () == 1)
	node = get_preserved_last_node (); 
      else
	node = get_last_node() ;
    }
  return node;
}

tree build_node_in_context (p, let, func)
     char *p; 
     tree let;
     tree func;
{
  tree node;
  tree was_let = get_current_block_in_tree ();
  tree was_func = get_current_function_decl ();
  tree was_current_class_decl = 0;
  tree was_current_class_type = 0;
  tree was_context_result = 0;

  if (func && DECL_RESULT(func) 
      && (TREE_CODE (DECL_RESULT(func)) == ERROR_MARK))
    {
      was_context_result = DECL_CONTEXT(DECL_RESULT(func));
      DECL_CONTEXT(DECL_RESULT(func)) = 0;
    }

  if (let) set_current_block_in_tree (let);
  if (func) set_current_function_decl (func);

  if (func)
    {
      if (TREE_CODE (TREE_TYPE(func)) == METHOD_TYPE)
	{
	  tree type = 0, id = 0;
	  was_current_class_decl = ast_get_current_class_decl();
	  was_current_class_type = ast_get_current_class_type ();

	  type = DECL_CONTEXT (func);
	  id = type ? TYPE_IDENTIFIER (type) : 0;
	  if (id)
	    {
	      if (TREE_TYPE (id) != type)
		{
		  if (flag_evaluate) id = copy_node (id);
		  TREE_TYPE (id) = type;
		}
	      build_push_scope (id, DECL_NAME (func));
	      ast_set_current_class_decl(DECL_ARGUMENTS(func));
	    }
	  else
	    ast_set_current_class_decl(DECL_ARGUMENTS(func));
	}
      else  /* case static CLASS_FOO::Function ()... */
      if (DECL_CONTEXT(func) && IS_AGGR_TYPE_CODE (TREE_CODE (DECL_CONTEXT(func))))
	{
	  was_current_class_type = ast_get_current_class_type ();
	  ast_set_current_class_type (DECL_CONTEXT (func));
	}
    }
  node = build_node_from_str (p);

  if (node)
  if (*tree_code_type [(int) TREE_CODE (node)] == 's')
    if (let)
      put_stmt_context (node, let);

  if (was_context_result)
    DECL_CONTEXT(DECL_RESULT(func)) = was_context_result;

  if (let) set_current_block_in_tree (was_let);
  if (func) set_current_function_decl (was_func);

  if (func)
    if (TREE_CODE (TREE_TYPE(func)) == METHOD_TYPE)  
      ast_set_current_class_decl (was_current_class_decl);
    else
      if (DECL_CONTEXT(func) && IS_AGGR_TYPE_CODE (TREE_CODE (DECL_CONTEXT(func))))
	  ast_set_current_class_type (was_current_class_type);

  return node;
}

/* is called from interpreter (through aset), evaluate and return;
   because this new AST is temporary, do not call hook */
tree build_node_in_context_no_hook (p, let, func)
     char *p; 
     tree let;
     tree func;
{
  set_ast_hook (0);
  return build_node_in_context (p, let, func);
}

static tree get_context (stmt)
     tree stmt;
{
  tree let = 0;

  if (stmt)
    if (TREE_CODE (stmt) == LET_STMT)
      let = STMT_SUPERCONTEXT (stmt);
    else
      let = STMT_CONTEXT(stmt);
  return let;
}

static tree
get_func_decl (f_name)
     char * f_name;
{
  tree func;
  tree id = get_identifier (f_name);

  /*func = ast_get_global_decl (id);*/
  func = lookup_name (id, 0);
  if (func)
    if (TREE_CODE (func) == FUNCTION_DECL)
      return func;
  return 0;
}
		
tree 
ast_get_fndecl ()
{
  char * fname = 0;
  ast_reset_get_fnames ();
  fname = ast_get_fname();
  return get_func_decl (fname);
}
    
#if 0
static tree search_function_by_superblock (block)
     tree block;
{
  tree func;
  char * name;
  ast_reset_get_fnames ();
  do
    {
      func = 0;
      name = ast_get_fname ();
      if (name)
	{
	  func = get_func_decl (name);
	  if (func)
	    if (DECL_INITIAL (func) == block) 
	      break;
	}
    }
  while (func);
  return func;
}
#endif /* 0 */

static tree ast_search_function_by_block (let_stmt)
     tree let_stmt;
{
  tree b, block;
  tree superlet_stmt, superblock = 0;

#if 1  
  for (block = STMT_BLOCK (let_stmt); block; block = BLOCK_SUPERCONTEXT (block))
    if (TREE_CODE (block) == FUNCTION_DECL)
      return block;
  return 0;
#else	
  for (b = let_stmt; b; b = STMT_SUPERCONTEXT (b))
    superlet_stmt = b;
  if (superlet_stmt)
    superblock = STMT_BLOCK (superlet_stmt);
  if (superblock)
    return BLOCK_SUPERCONTEXT (superblock);
#if 0
    return search_function_by_superblock (superblock);
#endif
  else return 0;
#endif /* 1 */
}

/* *********************************/
static tree
ast_stmt_contains_sublets (stmt)
     tree stmt;
{
  tree st;
  tree t = 0;
  enum tree_code code = TREE_CODE (stmt);
  switch (code)
    {
    case LET_STMT:
      t = stmt;
      break;
    case LOOP_STMT:
      st = STMT_LOOP_BODY (stmt);
      if (st)
	  t = ast_stmt_contains_sublets (st);
      break;
    case CASE_STMT:
      st = STMT_CASE_LIST (stmt);
      if (st)
   	  t = ast_stmt_contains_sublets (st);
      break;
    case IF_STMT:
      st = STMT_THEN (stmt);
      if (st)
   	  t = ast_stmt_contains_sublets (st);
      else
	{
	  st = STMT_ELSE (stmt);
	  if (st)
	      t = ast_stmt_contains_sublets (st);
	}
      break;
    default:
      t = NULL_TREE;
      break;
    }
  return t;
}

static tree
ast_stmt_contains_sublets_1 (stmt)
     tree stmt;
{
  tree st;
  tree t = 0;
  enum tree_code code = TREE_CODE (stmt);
  switch (code)
    {
    case LET_STMT:
      t = stmt;
      break;
    case LOOP_STMT:
      st = STMT_LOOP_BODY (stmt);
      if (st)
	  t = ast_stmt_contains_sublets_1 (st);
      break;
    case CASE_STMT:
      st = STMT_CASE_LIST (stmt);
      if (st)
   	  t = ast_stmt_contains_sublets_1 (st);
      break;
    case IF_STMT:
      st = STMT_ELSE (stmt);
      if (st)
   	  t = ast_stmt_contains_sublets_1 (st);
      else
	{
	  st = STMT_THEN (stmt);
	  if (st)
	      t = ast_stmt_contains_sublets_1 (st);
	}
      break;
    default:
      t = NULL_TREE;
      break;
    }
  return t;
}
/*********************************** */
static int add_block_to_chain (block, superblock, stmt)
     tree block;
     tree superblock;
     tree stmt;
{
  int rc = -1;
  if (block != NULL_TREE && superblock != NULL_TREE)
    {
      tree superlet = BLOCK_LET_STMT (superblock);
      tree previous_let = NULL_TREE;
      tree previous_block = NULL_TREE;
      tree t = superlet ? STMT_BODY (superlet) : NULL_TREE;

      for ( ; t && rc != 0; t = TREE_CHAIN (t))
	if (t != stmt)
	  {
	    if ((previous_let = ast_stmt_contains_sublets_1 (t)));
	    else
	      previous_let = ast_stmt_contains_sublets (t);
	    if (previous_let)
	      previous_block = STMT_BLOCK (previous_let);
	  }
	else  /* t == stmt */
	  {
	    tree next_let, next_block;
	    rc = 0;
	    if (previous_block == NULL_TREE)
	      {
		next_block = BLOCK_SUBBLOCKS (superblock);
		BLOCK_SUBBLOCKS (superblock) = block;
		next_let = STMT_SUBBLOCKS (superlet);
		STMT_SUBBLOCKS (superlet) = BLOCK_LET_STMT (block);
	      }
	    else
	      {
		next_block = BLOCK_CHAIN (previous_block);
		BLOCK_CHAIN (previous_block) = block;
	      }
	    BLOCK_CHAIN (block) = next_block;
	  }
    }
  return rc;
}

static int add_first_subblock (sublet, let)
     tree sublet;
     tree let;
{
  if (sublet == NULL_TREE || let == NULL_TREE) 
    return 0;
  TREE_CHAIN (sublet) = STMT_SUBBLOCKS (let);
  STMT_SUBBLOCKS (let) = sublet;
  return 1;
}

static int set_sublets (stmt, let)
     tree stmt;
     tree let;
{
  tree b, bl, t;
  if ((b = ast_stmt_contains_sublets_1 (stmt)))
    {
      tree prev;
      STMT_SUPERCONTEXT (b) = let;
      bl = ast_stmt_contains_sublets (stmt);
      if (bl != b)
	STMT_SUPERCONTEXT (bl) = let;
      for (t = TREE_PREV (stmt); t ; t = TREE_PREV (t))
	if ((prev = ast_stmt_contains_sublets (t))) 
	  {
	    tree temp = TREE_CHAIN (prev);
	    if (prev != t)
	      TREE_CHAIN (prev) = b;
	    if (bl != stmt)
	      TREE_CHAIN (bl) = temp;
	    break;
	  }
      if (!t)
	if (!add_first_subblock (b, let))
	  return 0;
    }
  return 1;
}

static tree ast_get_next_block (let)
     tree let;
{
  tree t;
  tree b = 0;

  b = (TREE_CHAIN (STMT_BLOCK (let)) ?
       BLOCK_LET_STMT (TREE_CHAIN (STMT_BLOCK (let))) : 0);

  return b;
}

static void ast_set_block_body (let, next)
     tree let, next;
{
  if (let) {
    if (STMT_VARS (let) == STMT_BODY (let))
      /* C++ case: (STMT_VARS == STMT_BODY) */
      STMT_VARS (let) = next;
      STMT_BODY (let) = next;
  }
}

static int
add_stmt (stmt, previous)
     tree stmt, previous;
{
  tree t;
  tree let_stmt;
  tree sub, subl;
  tree next;

  if (!previous) return 0;

  let_stmt = get_context (previous);

  if (stmt == let_stmt) return 0;
  if (!stmt) return 0; 
  if (! check_insertion (previous)) return 0;

  next  = TREE_CHAIN (previous);
  TREE_CHAIN (previous) = stmt;
  TREE_PREV (stmt) = previous;

  if (next)
    {
      TREE_CHAIN (stmt) = next;
      TREE_PREV (next) = stmt;
    }

  if (TREE_CODE (stmt) == LET_STMT)
    {
      STMT_SUPERCONTEXT (stmt) = let_stmt;
      BLOCK_SUPERCONTEXT (STMT_BLOCK (stmt)) = STMT_BLOCK (let_stmt);
    }
  STMT_CONTEXT (stmt) = let_stmt;

  if ((sub = ast_stmt_contains_sublets_1 (stmt)))
    {
      tree prev;
      STMT_SUPERCONTEXT (sub) = let_stmt;
      BLOCK_SUPERCONTEXT (STMT_BLOCK (sub)) = STMT_BLOCK (let_stmt);

      subl = ast_stmt_contains_sublets (stmt);
      if (subl != sub)
	{
	  STMT_SUPERCONTEXT (subl) = let_stmt;
	  BLOCK_SUPERCONTEXT (STMT_BLOCK (subl)) = STMT_BLOCK (let_stmt);
	}
#if 0 /* 05.14.95 * chain for LET_STMTs is chain for stmts - already */
      for (t = previous; t ; t = TREE_PREV (t))
	if ((prev = ast_stmt_contains_sublets (t))) 
	  {
	    tree temp = TREE_CHAIN (prev);
	    if (prev != t)
	      TREE_CHAIN (prev) = sub;
	    if (subl != stmt)
	      TREE_CHAIN (subl) = temp;
	    break;
	  }
      if (!t)
	if (!add_first_subblock (sub, let_stmt))
	  return 0;
#endif /* 0 */
      if (sub) 
	add_block_to_chain (STMT_BLOCK (sub), STMT_BLOCK (let_stmt), stmt);
      if (subl && subl != sub)
	add_block_to_chain (STMT_BLOCK (subl), STMT_BLOCK (let_stmt), stmt);
    }
  return 1;
}
  
int
add_first_stmt (stmt, let_stmt)
     tree stmt,  let_stmt;
{
  tree t;
  tree b;
  tree bl;

  if (stmt == let_stmt) return 0; /* {  Error (ERR_INPUT); Return; }*/
  if (!stmt) return 0; /* {   Error (ERR_INPUT);Return; }*/

  STMT_CONTEXT (stmt) = let_stmt;
  t = STMT_BODY (let_stmt);

  /* set STMT_BODY and, if C++, STMT_VARS */
  ast_set_block_body(let_stmt,stmt);

  if (t)
    {
      TREE_CHAIN(stmt) = t;
      TREE_PREV (t) = stmt;
    }
  if ((b = ast_stmt_contains_sublets_1 (stmt)))
    {
      bl = ast_stmt_contains_sublets (stmt);
      STMT_SUPERCONTEXT (b) = let_stmt; 
      STMT_SUPERCONTEXT (bl) = let_stmt; 
      if (! (TREE_CHAIN (bl)))
	TREE_CHAIN (bl) = STMT_SUBBLOCKS (let_stmt);
      STMT_SUBBLOCKS (let_stmt) = b;
    }
  return 1;
}

int
add_last_stmt (stmt, let_stmt)
     tree stmt, let_stmt;
{
  tree st = 0;
  tree last_st = 0;
  STMT_CONTEXT (stmt) = let_stmt;
  for (st = STMT_BODY(let_stmt); st; st = TREE_CHAIN(st))
    last_st = st;
  if (last_st)
    {
      if (add_stmt (stmt, last_st));
      else return 0;
    }
  else
    if (add_first_stmt (stmt, let_stmt));
    else return 0;
  return 1;
}
#if 0 /* 05.15.95 */
static int del_stmt_sublets (t)
     tree t;
{
  tree b;
  tree b1;
  tree tt;
  tree let_stmt = STMT_CONTEXT (t); 

  if (let_stmt)
    for (b = STMT_SUBBLOCKS (let_stmt), b1 = STMT_SUBBLOCKS (let_stmt); b; 
	 b = ast_get_next_block (b))
      {
	if (b == t)
	  {
	    if (b == STMT_SUBBLOCKS (let_stmt))
	      {
		tt = ast_get_next_block (b);
		STMT_SUBBLOCKS (let_stmt) = tt;
		BLOCK_SUBBLOCKS (STMT_BLOCK (let_stmt)) = 
		  STMT_BLOCK (tt);
	      }
	    else
	      if (TREE_CHAIN (b) != b1)
		TREE_CHAIN (b1) = TREE_CHAIN (b);
	    break;
	  }
	b1 = b;
      }
}
#endif /* 0 */
static int del_stmt_subblocks (t)
     tree t;
{
  tree b;
  tree b1;
  tree tt;
  tree superblock = BLOCK_SUPERCONTEXT (t); 

  if (superblock)
    for (b = BLOCK_SUBBLOCKS (superblock); b;
	 b1 = b, b = BLOCK_CHAIN (b))
      {
	if (b == t)
	  {
	    if (b == BLOCK_SUBBLOCKS (superblock))
	      {
		tt = BLOCK_CHAIN (b);
		BLOCK_SUBBLOCKS (superblock) = tt;
		if (tt)
		STMT_SUBBLOCKS (BLOCK_LET_STMT (superblock)) = BLOCK_LET_STMT (tt);
	      }
	    else
	      if (BLOCK_CHAIN (b) != b1)
		BLOCK_CHAIN (b1) = BLOCK_CHAIN (b);
	    break;
	  }
      }
}

static int del_stmt (stmt)
     tree stmt;
{
  tree t, ttt;
  tree next;
  tree prev;
  tree let_stmt;

  if (!stmt) return 0;
  
  t = stmt;
  if (TREE_CODE (t) == LET_STMT)
    let_stmt = STMT_SUPERCONTEXT (t);
  else    let_stmt = STMT_CONTEXT (t); 

  next = TREE_CHAIN (t);
  prev = TREE_PREV (t);
  if (prev)
    TREE_CHAIN (prev) = next;
  else
    {
      if (TREE_CODE (t) == LET_STMT)
	{
	  if (let_stmt)
	    if (next)
	      if (STMT_BODY (let_stmt) == t) 
		ast_set_block_body (let_stmt, next);
	      else;
	    else
	      if (STMT_BODY (let_stmt) == t) 
		ast_set_block_body (let_stmt, 0);
	      else
		if (STMT_SUBBLOCKS (let_stmt) == t)
		  STMT_SUBBLOCKS (let_stmt) = NULL_TREE;
	}
      else
	if (let_stmt)
	  if (search_current_stmt (stmt) == stmt)
	    ast_set_block_body (let_stmt, next);
    }
  if (next)
    TREE_PREV (next) = prev;

  if (TREE_CODE (t) == LET_STMT)
    del_stmt_subblocks (STMT_BLOCK (t));
  else
    {
      tree t1, t2, block;
      t1 = ast_stmt_contains_sublets (t);
      t2 = ast_stmt_contains_sublets_1 (t);
      if (t1)
	for (block = STMT_BLOCK (t1); block; block = BLOCK_CHAIN (block))
	  {
	    del_stmt_subblocks (block);
	    if (t2 == NULL_TREE || block == STMT_BLOCK (t2))
	      break;
	  }
    }
    
  TREE_PREV (t) = NULL_TREE;

  TREE_CHAIN (t) = NULL_TREE;
  STMT_CONTEXT (t) = NULL_TREE;
  if (TREE_CODE (t) == LET_STMT)
    STMT_SUPERCONTEXT (t) = NULL_TREE;
  return 1;
}

static void ast_set_block_vars (let_stmt, next)
     tree let_stmt, next;
{
  if (let_stmt) {
    if (STMT_VARS(let_stmt) == STMT_BODY(let_stmt))
      /* C++ case: (STMT_VARS == STMT_BODY) */
      STMT_BODY (let_stmt) = next;

    STMT_VARS (let_stmt) = next;
  }
}

static int
add_var (decl, let_stmt)
     tree decl, let_stmt;
{
  tree b, t, var;

  if ((!let_stmt) || (!decl)) return 0;

  for (t = var = STMT_VARS(let_stmt); t; t = TREE_CHAIN (t))
    var = t;
  if (var)
    {
      TREE_CHAIN (var) = decl;
      TREE_PREV  (decl) = var;
    }
  else
    ast_set_block_vars (let_stmt, decl);

  b = let_stmt;
  t = decl;
  if (t)
    DECL_CONTEXT (t) = b;
  return 1;
}

static int
add_first_var (decl, let_stmt)
     tree decl, let_stmt;
{
  tree var;

  if ((!let_stmt) || (!decl)) return 0;

  var = STMT_VARS(let_stmt);

  if (var)
    TREE_CHAIN (decl) = var;

  ast_set_block_vars (let_stmt, decl);

  if (decl)
    DECL_CONTEXT (decl) = let_stmt;
  return 1;
}

static int
add_var_before (decl, next)
     tree decl, next;
{
  tree previous;
  
  if ((!decl) || (!next)) return 0;
  if (next)
    {
      TREE_CHAIN (decl) = next;
      TREE_PREV (next) = decl;
      DECL_CONTEXT (decl) = DECL_CONTEXT(next);
      previous = TREE_PREV(next);
      if (previous)
	{
	  TREE_CHAIN (previous) = decl;
	  TREE_PREV  (decl) = previous;
	}
    }
  return 1;
}

static int
add_var_after (decl, previous)
     tree decl, previous;
{
  tree next;

  if ((!decl) || (!previous)) return 0;
  if ((! check_insertion (previous)))
    return 0;

  if (previous)
    {
	  TREE_CHAIN (previous) = decl;
	  TREE_PREV  (decl) = previous;
      DECL_CONTEXT (decl) = DECL_CONTEXT(previous);
      next = TREE_CHAIN(previous);
      if (next)
	{
	  TREE_CHAIN (decl) = next;
	  TREE_PREV (next) = decl;
	}
    }
  return 1;
}

static int
del_var (decl)
     tree decl;
{
  tree prev = 0;
  tree next = 0;
  tree let_stmt = 0;

  if (!decl) return 0;

  let_stmt = DECL_CONTEXT(decl);
  
  prev = TREE_PREV(decl);
  next = TREE_CHAIN(decl);

  if (prev)
      TREE_CHAIN (prev) = next;
  else
    {
      if (let_stmt)
      if (STMT_VARS(let_stmt) == decl) 
	ast_set_block_vars (let_stmt, decl);
    }
  if (next)
    TREE_PREV (next) = prev;
  
  TREE_PREV (decl) = 0;
  TREE_CHAIN (decl) = 0;

  DECL_CONTEXT(decl) = 0;
}

static int
check_insertion (prev)
     tree prev;
{
  tree t = 0;
  enum tree_code code;
  tree supercontext;

  code = TREE_CODE (prev);

  if (*tree_code_type [(int) TREE_CODE (prev)] == 's')
    switch (code)
      {
      case LET_STMT:
	supercontext = STMT_SUPERCONTEXT (prev);
	break;
	default:
	supercontext = STMT_CONTEXT (prev);
	break;
      }
  else
    if (*tree_code_type [(int) TREE_CODE (prev)] == 'd')
      supercontext = DECL_CONTEXT (prev);

  if (supercontext)
    for (t = STMT_BODY (supercontext); t; t = TREE_CHAIN (t))
      if (prev == t) break;
  if (t)
    return 1;
  else return 0;
}

static tree
containing_stmt (t, stmt)
     tree t;
     tree stmt;
{
  tree tt = 0;

  if (t == stmt)    tt = t;
  else
  switch (TREE_CODE (t))
    {
    case IF_STMT:
      if (STMT_THEN (t) == stmt) 
	tt = t;
      else
	if (tt = containing_stmt (STMT_THEN (t), stmt));
	else
	  if (STMT_ELSE (t))
	    if ((STMT_ELSE (t) == stmt)) tt = t;
	    else
	      if (tt = containing_stmt (STMT_ELSE (t), stmt) );
      break;

    case LOOP_STMT:
      if (STMT_LOOP_BODY (t))
	if (STMT_LOOP_BODY (t) == stmt) tt = t;
	else
	  tt = containing_stmt (STMT_LOOP_BODY (t), stmt);
      break;
    case CASE_STMT:
      if (STMT_CASE_LIST (t) == stmt) tt = t;
      else	tt = 0;
      break;
    default: tt = 0;
      break;
    }
  return tt;
}

static tree
get_containing_stmt (let_stmt, stmt)
     tree let_stmt, stmt;
{
  tree t;
  tree tt = 0;
  if (let_stmt)
    for (t = STMT_BODY (let_stmt); t; t = TREE_CHAIN (t))
      if (t == stmt) return t;
      else
	if (tt = containing_stmt (t, stmt)) 
	  break;
  return tt;
}

static tree search_current_stmt (stmt)
  tree stmt;
{
  tree let_stmt;
  tree t;

  let_stmt = get_context (stmt);
  
  if (!let_stmt)
    t = 0;
  else
    {
      if (!STMT_SUPERCONTEXT (let_stmt))
	t = 0;
      else
	t = get_containing_stmt (let_stmt, stmt);
    }
  return t;
} 

static int replace_function_body (ns,os)
     tree ns,  os;
{
  int rc = 0;
  tree let, block;

  if (TREE_CODE (os) == LET_STMT &&
      (let = STMT_SUPERCONTEXT (os)))
    if (! STMT_SUPERCONTEXT (let) &&
	/* this is a first block in a function */
	TREE_CODE (ns) == LET_STMT)
      {
	STMT_SUBBLOCKS (let) = ns;
	STMT_SUPERCONTEXT (ns) = let;
	STMT_CONTEXT (ns) = let;

	block = STMT_BLOCK (ns);
	BLOCK_SUBBLOCKS (STMT_BLOCK (let)) = block;
	BLOCK_SUPERCONTEXT (block) = STMT_BLOCK (let);
	rc = 1;
      }
  return rc;
}

static tree get_first_subblock (stmt)
     tree stmt;
{
  tree block = NULL_TREE;
  tree sub = ast_stmt_contains_sublets (stmt);
  if (sub)    block = STMT_BLOCK (sub);
  else 
    if ((sub = ast_stmt_contains_sublets_1 (stmt))) 
      block = STMT_BLOCK (sub);
  return block;
}

static tree get_last_subblock (stmt)
     tree stmt;
{
  tree block = NULL_TREE;
  tree sub = ast_stmt_contains_sublets_1 (stmt);

  if (sub)    block = STMT_BLOCK (sub);
  else 
    if ((sub = ast_stmt_contains_sublets (stmt))) 
      block = STMT_BLOCK (sub);
  return block;
}

static int replace_stmt (ns, os)
     tree ns,  os;
{
  int rc = 1;
  tree let;
  tree prev_block = NULL_TREE, newblock;

  let = get_context (os);

  /* find prev_block - BLOCK_CHAIN of which is oldblock */
  {
    tree oldblock = get_first_subblock (os);  

    if (oldblock) /* remove it */
      {
	tree blk;      
	for (blk = BLOCK_SUBBLOCKS (STMT_BLOCK (let)); blk;
	     blk = BLOCK_CHAIN (blk))
	  if (blk == oldblock)
	    {
	      tree lastblock = get_last_subblock (os);
	      if (prev_block)
		BLOCK_CHAIN (prev_block) = BLOCK_CHAIN (lastblock);
	      else
		BLOCK_SUBBLOCKS (STMT_BLOCK (let)) = BLOCK_CHAIN (lastblock);
	      break;
	    }
	  else prev_block = blk;
      }
  }
	
  {
    tree current_stmt  = search_current_stmt (os);

    if (current_stmt)
      {  
	switch (TREE_CODE (current_stmt))
	  {
	  case IF_STMT:
	    if (STMT_THEN (current_stmt) == os)
	      STMT_THEN (current_stmt) = ns;
	    else
	      if (STMT_ELSE (current_stmt) == os)
		STMT_ELSE (current_stmt) = ns;
	      else rc = 0; 
	    break;
	  case LOOP_STMT:
	    if (STMT_LOOP_BODY (current_stmt) == os)
	      STMT_LOOP_BODY (current_stmt) = ns;
	    else rc = 0; 
	    break;
	  case LET_STMT:
	    if (STMT_BODY (current_stmt) == os)
	      ast_set_block_body (current_stmt, ns);
	    else rc = 0; 
	    break;
	  case CASE_STMT:
	    if (STMT_CASE_LIST (current_stmt) == os)
	      STMT_CASE_LIST (current_stmt) = ns;
	    else rc = 0; 
	    break;
	  default: /* should be current_stmt==os, rc == 1 already */
	    if (current_stmt != os) 
	      {
		rc = 0;
		if (ast_hook == 0 || getenv ("PSET_PARSER_CRASH"))
		  printf ("INCREMENTAL: replace_stmt");
		goto ex;
	      }
	    break;
	  }
      }
    else
      {
	/* this is a first block in a function */
	rc = replace_function_body (ns, os);
	goto ex;
      }
  }

  if (rc == 0) goto ex;

  {
    tree next = TREE_CHAIN (os);  
    tree prev = TREE_PREV (os);   
    if (next)
      {
	TREE_CHAIN (ns) = next;
	TREE_CHAIN (os) = 0;
	if (TREE_PREV (next))
	  TREE_PREV (next) = ns;
      }
    if (prev) 
      {
	TREE_PREV  (ns) = prev;
	TREE_PREV  (os) = 0;
	TREE_CHAIN (prev) = ns;
      }
  }

  TREE_CHAIN (os) = TREE_PREV (os) = NULL_TREE;
  STMT_CONTEXT (os) = NULL_TREE;
  if (TREE_CODE (os) == LET_STMT)
    {
      STMT_SUPERCONTEXT (os) = NULL_TREE;
      BLOCK_SUPERCONTEXT (STMT_BLOCK (os)) = NULL_TREE;
    }

  STMT_CONTEXT (ns) = let;

#if 0 /* 05.15.95 */
   if (TREE_CODE (ns) != LET_STMT)
    set_sublets (ns, let);
  else  /* added 05.12.95 */
    if (current_stmt != os)
      if (TREE_CODE (current_stmt) != LET_STMT)  /* why? *051295*/
	set_sublets (current_stmt, let);
      else	;
    else      ;
#endif /* 0 */

  if ((newblock = get_first_subblock (ns)))
    {
      /* add it into chain of BLOCKs */
      tree t, sub, lastnewblock;
      if (prev_block == NULL_TREE)
	{
	  for (t = STMT_BODY (let); !containing_stmt (t, ns); t = TREE_CHAIN (t))
	    if ((sub = get_last_subblock (t)))
	      prev_block = sub;
	}
      if (prev_block)
	{
	  sub = BLOCK_CHAIN (prev_block);
	  BLOCK_CHAIN (prev_block) = newblock;
	}
      else
	{
	  sub = BLOCK_SUBBLOCKS (STMT_BLOCK (let));
	  BLOCK_SUBBLOCKS (STMT_BLOCK (let)) = newblock;
	  t = STMT_SUBBLOCKS (let);
	  STMT_SUBBLOCKS (let) = BLOCK_LET_STMT (newblock);	  
	}

      lastnewblock = get_last_subblock (ns);
      BLOCK_CHAIN (lastnewblock) = sub;

      for (sub = newblock; sub; sub = BLOCK_CHAIN (sub))
	{
	  BLOCK_SUPERCONTEXT (sub) = STMT_BLOCK (let);
	  STMT_SUPERCONTEXT (BLOCK_LET_STMT (sub)) = let;
	  if (sub == lastnewblock) break;
	}
    }
 ex:
  return rc;
}  

tree ast_insert (optype, targ, p, delstmt)
     astOperType optype;
     tree targ;
     char *p;
     tree delstmt;
{
  tree stmt, expr, block, node = 0, fndecl = 0;
  tree was_current_block, was_current_stmt;
  tree was_current_function = get_current_function_decl ();
  enum tree_code code = TREE_CODE (targ);
  int err = 0;

  was_current_block = get_current_block_in_tree ();
  was_current_stmt = current_stmt_in_tree;

  if (delstmt)
    if (del_stmt (delstmt));
    else goto ex;
  
  switch ( *tree_code_type [(int) code] )
    {
    case 's':
      switch (optype)
	{
	case REPLACE:
	  if (TREE_CODE (targ) == LET_STMT)
	    block = STMT_SUPERCONTEXT (targ);
	  else	    block = STMT_CONTEXT (targ);
	  
	  set_current_block_in_tree (block);
	  current_stmt_in_tree = search_current_stmt_in_tree(targ);
	  break;

	case FIRST:
	  block = targ;	
	  current_stmt_in_tree = targ;
	  break;

	case AFTER:
	  if (TREE_CODE (targ) == LET_STMT)
	    block = STMT_SUPERCONTEXT (targ);
	  else	    block = STMT_CONTEXT (targ);
	  current_stmt_in_tree = block;
	  break;
	}
#ifdef _STAND_ALONE
      if (targ->stmt.filename &&
	  targ->stmt.filename [strlen(targ->stmt.filename)-1] != 'h')
	{
	  char *save0 = input_filename;
	  char *save1 = main_input_filename;
	  input_filename = targ->stmt.filename;
	  main_input_filename = targ->stmt.filename;
	  restore_unit_for_incremental ();
	  input_filename = save0;
	  main_input_filename = save1;
	}

      if (current_stmt_in_tree)
	switch (*tree_code_type [(int)TREE_CODE (current_stmt_in_tree)])
	  {
	  case 's':
	    lineno = current_stmt_in_tree->stmt.linenum;
	    break;
	  }
#endif /* _STAND_ALONE */

      fndecl = ast_search_function_by_block (block);
      stmt = build_node_in_context (p, block, fndecl);

      if (stmt)
	{
	  if (*tree_code_type [(int) TREE_CODE (stmt)] == 's')
	    {
	      switch (optype)
		{
		case REPLACE:
		  if (check_insertion (targ))
		    {
		      if (add_stmt (stmt, targ))
			if (del_stmt (targ));
			else err++;
		      else err++;
		    }
		  else 
		      if (replace_stmt (stmt, targ));
		      else err++;
		  break;

		case FIRST:
		  if (add_first_stmt (stmt, targ));
		  else err++;
		  break;

		case AFTER:
		  if (check_insertion (targ))
		    {
		      if (add_stmt (stmt, targ));
		      else err++;
		    }
		  else	err++; /***     { Error (ERR_INPUT);Return; }***/
		  break;
		  default: err++;  /***  {  Error (ERR_INPUT);Return; } ***/
		  break;                                           /* ERROR */
		}
	    }
	  else err++;
	}
      if (err == 0) node = stmt;
      break;

    case 'e':
    case 'c':
      node = build_exp_from_str (p);
      break;  

    case 'd':
      if (DECL_CONTEXT (targ) == 0 || TREE_CODE (DECL_CONTEXT (targ)) != LET_STMT ||
	  STMT_VARS (DECL_CONTEXT(targ)) != IDENTIFIER_GLOBAL_VALUE(get_identifier("int")))
	err++;
      else
      if (TREE_CODE (targ) == FUNCTION_DECL)
	{
	  static tree replace_function ();
	  if (optype == REPLACE)
	    node = replace_function (targ, p);
	}
      else 
	{
	  static tree replace_global ();
	  if (optype == REPLACE)
	    node = replace_global (targ, p);
	}
      break;
#if 0
      if (TREE_CODE (targ) == TYPE_DECL &&
	  (TREE_CODE(TREE_TYPE(targ)) == RECORD_TYPE ||
	   TREE_CODE(TREE_TYPE(targ)) == UNION_TYPE))
	return (tree) (-1);

      block = DECL_CONTEXT (targ);
      set_current_block_in_tree (block);
      current_stmt_in_tree = targ;

      switch (optype)
	{
	case REPLACE:
	case AFTER:
	  break;
	case FIRST:
	case REPLACE_RIGHT:
	  default:	  node = 0; goto ex;
	}

      fndecl = ast_search_function_by_block (block);
      node = build_node_in_context (p, block, fndecl);

      if (node)
	{
	  if (*tree_code_type [(int) TREE_CODE (node)] == 'd')
	    {
	      switch (optype)
		{
		case REPLACE:
		  if (check_insertion (targ))
		    {
		      if (add_var_after (node, targ));
		      else node = 0;
		      if (del_var (targ));
		      else node =  0;
		    }
		  else node =  0;
		  break;

		case FIRST:
		  if (add_first_var (node, targ));
		  else node =  0;
		  break;
		case AFTER:
		  if (check_insertion (targ))
		    {
		      if (add_var_after (node, targ));
		      else node =  0;
		    }
		  else	node = 0;
		  break;

		  default: node = 0;
		  break;                                           /* ERROR */
		}
	    } 
	}
      break;
#endif /* 0 */

      default: err++;
    }
 ex:
  if ((err || node == NULL_TREE))
    {
      if (getenv ("PSET_PARSER_CRASH"))
	{
	  printf ("INCREMENTAL: ast_insert: FAIL \nOld ast tree: \n");
	  debug_tree (targ);
	  printf ("New string: %s\n", p);
	}
    }
  else
    {
      TREE_CHAIN (targ) = node;
      mark_old_ast (targ);
    }      
  current_stmt_in_tree = was_current_stmt;
  set_current_block_in_tree (was_current_block);
  set_current_function_decl (was_current_function);

  return node;
}

/* ---- */
static tree
search_current_stmt_in_tree (stmt)
     tree stmt;
{
  tree block;
  if (!(block = get_current_block_in_tree())) return 0;
  if (block == stmt) return 0;
  return get_containing_stmt (block, stmt);
}

static int
search_up_because_break (stmt)
     tree stmt;
{
  tree tt;
  tree block;
  /*** if (!stmt) generate_error_input ("search_up_because_break");***/
  switch (TREE_CODE(stmt))
    {
    case LOOP_STMT: return 1;
    case CASE_STMT: return 1;
    case LET_STMT:
      block = STMT_SUPERCONTEXT (stmt);
      break;
    default:
      block = STMT_CONTEXT (stmt);
      break;
    }
  tt = get_containing_stmt (block, stmt);
  if (tt)
    if (tt == stmt)
      return search_up_because_break (block);
    else
      return search_up_because_break (tt);
  else return 0;
}

int
ast_is_break_legal ()
{
  int rc = 0;
  if (current_stmt_in_tree)
    rc = search_up_because_break (current_stmt_in_tree);
  return rc;
}

static int search_up_switch (stmt)
     tree stmt;
{
  tree tt;
  tree block;

  switch (TREE_CODE(stmt))
    {
    case CASE_STMT: return 1;
    case LET_STMT:
      block = STMT_SUPERCONTEXT (stmt);
      break;
    default:
      block = STMT_CONTEXT (stmt);
      break;
    }
  tt = get_containing_stmt (block, stmt);
  if (tt)
    if (tt == stmt)
      return search_up_switch (block);
    else
      return search_up_switch (tt);
  else return 0;
}

int ast_is_case_legal ()
{
  int rc = 0;
  tree block;

  if (current_stmt_in_tree)
    rc = search_up_switch (current_stmt_in_tree);
  else if (block = get_current_block_in_tree())
    rc = search_up_switch (block);
  return rc;
}
/* -----  */

/****
tree
get_first_block_of_fname (fname)
     char* fname;
{
  tree block = 0;
  tree func = get_func_decl (fname);
  if (func)
    block = ast_get_first_block (func);
  return block;
}
******/


tree build_contexted_exp (p, block)
     char *p; 
     tree block;
{
  tree node;
  tree fndecl = 0;

  if (block)
    fndecl = ast_search_function_by_block (block);
  node = build_node_in_context (p, block, fndecl);
  return node;
}

extern int parser_global ();
extern void push_typedecl_value_list ();
extern void unset_static_global_value ();

/* 11.04.92 NY - basically, replace FUNCTION_DECL decl */
static tree replace_function (decl, p)
     tree decl;
     char *p;
{
  int rc;
  tree was_global = 0;
  tree was_id = 0;
  tree was_implicit = 0;
  tree oldparms = 0;

  tree last;
  tree fndecl = 0;
  tree was_block = get_current_block_in_tree ();
  tree was_func = get_current_function_decl ();
  tree was_current_class_decl = ast_get_current_class_decl();
  tree was_context_result = 0;

  /* should be removed from 
     - id node(s) DECL_ASSEMBLER_NAME and maybe DECL_NAME;
     - from implicit;
     - global chain;
     - if static, not TREE_PUBLIC, remove from ulst->typedecl_value_list
     - remove from get_id
     */
     
  if (IDENTIFIER_GLOBAL_VALUE (DECL_ASSEMBLER_NAME (decl)) == decl)
    {
      was_global = IDENTIFIER_GLOBAL_VALUE (DECL_ASSEMBLER_NAME (decl));
      IDENTIFIER_GLOBAL_VALUE (DECL_ASSEMBLER_NAME (decl)) = 0;
      ast_delete_fname_in_names (IDENTIFIER_POINTER (DECL_ASSEMBLER_NAME (decl)));
   }

  if (DECL_NAME (decl) && IDENTIFIER_GLOBAL_VALUE (DECL_NAME (decl)) == decl)
    {
      was_global = IDENTIFIER_GLOBAL_VALUE (DECL_NAME (decl));
      IDENTIFIER_GLOBAL_VALUE (DECL_NAME (decl)) = 0;
      ast_delete_fname_in_names (IDENTIFIER_POINTER (DECL_NAME (decl)));
   }

  if (DECL_NAME (decl) && DECL_NAME (decl) != DECL_ASSEMBLER_NAME (decl))
    {
      /* clear this decl in overloaded function list */
      tree lst;
      tree t;
      tree prev = 0;
      
      lst = IDENTIFIER_GLOBAL_VALUE (DECL_NAME (decl));
      if (lst && TREE_CODE (lst) == TREE_LIST)
	for (t = lst; t; prev = t, t=TREE_CHAIN (t))
	  if (TREE_VALUE (t) == decl)
	    {
	      if (prev) TREE_CHAIN (prev) = TREE_CHAIN (t);
	      else 
		IDENTIFIER_GLOBAL_VALUE (DECL_NAME (decl)) = TREE_CHAIN(t);
	      break;
	    }
    }

  if (IDENTIFIER_IMPLICIT_DECL (DECL_ASSEMBLER_NAME (decl)) == decl)
    {
      was_implicit = IDENTIFIER_IMPLICIT_DECL (DECL_ASSEMBLER_NAME (decl));
      SET_IDENTIFIER_IMPLICIT_DECL (DECL_ASSEMBLER_NAME (decl), 0);
    }

  was_id = get_id (IDENTIFIER_POINTER (DECL_ASSEMBLER_NAME (decl)));

  if (was_id)
    if (IDENTIFIER_GLOBAL_VALUE (was_id) == decl)
      IDENTIFIER_GLOBAL_VALUE (was_id) = 0;
    else was_id = 0;

  set_current_block_in_tree (0);
  set_current_function_decl (0);
  ast_set_current_class_decl(0);

  rc = parser_global (p);

  set_current_block_in_tree (was_block);
  set_current_function_decl (was_func);
  ast_set_current_class_decl (was_current_class_decl);

  if (rc == 0)
    {
      tree prev_global;
      tree prev = 0, node, next;

      tree t = get_identifier ("int");
      if (t) t =  IDENTIFIER_GLOBAL_VALUE (t);
      for (prev_global = 0; t; t = TREE_CHAIN (t))
	if (t == decl) break;
	else prev_global = t;

      last = get_last_node ();

      /* form chain of new global decls in right order;
	 also get (if there is only one) FUNCTION_DECL to fndecl */
      for (node = last; node; node = next)
	{
	  if (TREE_CODE (node) == FUNCTION_DECL)
	    if (fndecl == 0)    fndecl = node;
	    else fndecl = 0;
	  next = TREE_CHAIN (node);
	  /* reverse order */
	  TREE_CHAIN (node) = prev;
	  prev = node;

	  if (next && TREE_CHAIN (next) == 0)
	    /* do not include it to new chain, because it was
	       last decl in global_binding_level->names */
	    break;
	}

      /* put it to global chain */
      TREE_CHAIN (prev_global) = prev;
      TREE_CHAIN (last) = TREE_CHAIN (decl);

      if (fndecl) /* just one FUNCTION_DECL - replace decl */
	if (TREE_CODE (TREE_TYPE (fndecl)) == TREE_CODE (TREE_TYPE (decl)))
	  {
	    if (DECL_INITIAL (fndecl))
	      {
#if 0		
		OSapi_bcopy ((char*) fndecl, decl, sizeof (struct tree_decl));
#else
		memcpy (decl, (char*) fndecl, sizeof (struct tree_decl));
#endif
		for (node = prev_global; node; node = TREE_CHAIN (node))
		  if (TREE_CHAIN (node) == fndecl) break;
		if (node) 
		  {
		    TREE_CHAIN (node) = decl;
		    if (last == fndecl) last = decl;
		    fndecl = decl;
		  }
	      }
	    if (was_global)
	      IDENTIFIER_GLOBAL_VALUE(DECL_ASSEMBLER_NAME(fndecl)) = fndecl;
	    if (was_id) 
	      {
		tree id=get_id(IDENTIFIER_POINTER(DECL_ASSEMBLER_NAME(fndecl)));
		IDENTIFIER_GLOBAL_VALUE (id) = fndecl;
	      }
	    if (was_implicit) {}
	  }

      if (prev_global)
	for (node = TREE_CHAIN (prev_global); node; node = TREE_CHAIN (node))
	  if (node == last) break;
	  else
	    if ((TREE_CODE(node) == VAR_DECL ||TREE_CODE(node) == FUNCTION_DECL) &&
		TREE_STATIC (node) && !TREE_PUBLIC (node) ||
		TREE_CODE (node) == TYPE_DECL)
	      {
		push_typedecl_value_list (node);
		unset_static_global_value (node);
	      }
      return (prev_global ? TREE_CHAIN (prev_global) : 0);
    }
  else /* rc != 0 */
    {
      if (was_global)
	IDENTIFIER_GLOBAL_VALUE(DECL_ASSEMBLER_NAME(decl)) = decl;
      if (was_id) 
	{
	  IDENTIFIER_GLOBAL_VALUE (was_id) = decl;
	}
      if (was_implicit)
	SET_IDENTIFIER_IMPLICIT_DECL (DECL_ASSEMBLER_NAME (decl),was_implicit);
      return 0;
    }

#if 0
  if (TREE_STATIC (decl) && TREE_PUBLIC (decl) == 0)
    /* static function was saved in value list, remove from it */
    pop_typedecl_value_list (decl);
#endif
}

/* 11.04.92 NY - basically, replace VAR_DECL decl on the global level */
static tree replace_global (olddecl, p)
     tree olddecl;
     char *p;
{
  int rc;
  tree was_global = 0;
  tree was_id = 0;
  tree was_implicit = 0;
  tree oldparms = 0;

  tree last;
  tree fndecl = 0;
  tree was_block = get_current_block_in_tree ();
  tree was_func = get_current_function_decl ();
  tree was_current_class_decl = ast_get_current_class_decl();
  tree was_context_result = 0;

  /* should be removed from 
     - id node(s) DECL_ASSEMBLER_NAME and maybe DECL_NAME;
     - from implicit;
     - global chain;
     - if static, not TREE_PUBLIC, remove from ulst->typedecl_value_list
     - remove from get_id
     */
     
  if (IDENTIFIER_GLOBAL_VALUE (DECL_ASSEMBLER_NAME (olddecl)) == olddecl)
    {
      was_global = IDENTIFIER_GLOBAL_VALUE (DECL_ASSEMBLER_NAME (olddecl));
      IDENTIFIER_GLOBAL_VALUE (DECL_ASSEMBLER_NAME (olddecl)) = 0;
      if (TREE_CODE (olddecl) == FUNCTION_DECL)   /* */
      ast_delete_fname_in_names (IDENTIFIER_POINTER (DECL_ASSEMBLER_NAME (olddecl)));
   }

  if (DECL_NAME (olddecl) && IDENTIFIER_GLOBAL_VALUE (DECL_NAME (olddecl)) == olddecl)
    {
      was_global = IDENTIFIER_GLOBAL_VALUE (DECL_NAME (olddecl));
      IDENTIFIER_GLOBAL_VALUE (DECL_NAME (olddecl)) = 0;
      if (TREE_CODE (olddecl) == FUNCTION_DECL)   /* */
      ast_delete_fname_in_names (IDENTIFIER_POINTER (DECL_NAME (olddecl)));
   }

  if (DECL_NAME (olddecl) && DECL_NAME (olddecl) != DECL_ASSEMBLER_NAME (olddecl))
    {
      /* clear this decl in overloaded function list */
      tree lst;
      tree t;
      tree prev = 0;
      
      lst = IDENTIFIER_GLOBAL_VALUE (DECL_NAME (olddecl));
      if (lst && TREE_CODE (lst) == TREE_LIST)
	for (t = lst; t; prev = t, t=TREE_CHAIN (t))
	  if (TREE_VALUE (t) == olddecl)
	    {
	      if (prev) TREE_CHAIN (prev) = TREE_CHAIN (t);
	      else 
		IDENTIFIER_GLOBAL_VALUE (DECL_NAME (olddecl)) = TREE_CHAIN(t);
	      break;
	    }
    }

  if (IDENTIFIER_IMPLICIT_DECL (DECL_ASSEMBLER_NAME (olddecl)) == olddecl)
    {
      was_implicit = IDENTIFIER_IMPLICIT_DECL (DECL_ASSEMBLER_NAME (olddecl));
      SET_IDENTIFIER_IMPLICIT_DECL (DECL_ASSEMBLER_NAME (olddecl), 0);
    }

  was_id = get_id (IDENTIFIER_POINTER (DECL_ASSEMBLER_NAME (olddecl)));

  if (was_id)
    if (IDENTIFIER_GLOBAL_VALUE (was_id) == olddecl)
      IDENTIFIER_GLOBAL_VALUE (was_id) = 0;
    else was_id = 0;

  set_current_block_in_tree (0);
  set_current_function_decl (0);
  ast_set_current_class_decl(0);

  rc = parser_global (p);

  set_current_block_in_tree (was_block);
  set_current_function_decl (was_func);
  ast_set_current_class_decl (was_current_class_decl);

  if (rc == 0)
    {
      tree prev_global;
      tree prev = 0, node, next;

      tree t = get_identifier ("int");
      if (t) t = IDENTIFIER_GLOBAL_VALUE (t);
      for (prev_global = 0; t; t = TREE_CHAIN (t))
	if (t == olddecl) break;
	else prev_global = t;

      last = get_last_node ();

      /* form chain of new global decls in right order;
	 also get (if there is only one) FUNCTION_DECL to fndecl */
      for (node = last; node; node = next)
	{
	  if (TREE_CODE (node) == FUNCTION_DECL)
	    if (fndecl == 0)    fndecl = node;
	    else fndecl = 0;
	  next = TREE_CHAIN (node);
	  /* reverse order */
	  TREE_CHAIN (node) = prev;
	  prev = node;

	  if (next && TREE_CHAIN (next) == 0)
	    /* do not include it to new chain, because it was
	       last decl in global_binding_level->names */
	    break;
	}

      /* put it to global chain */
      TREE_CHAIN (prev_global) = prev;
      TREE_CHAIN (last) = TREE_CHAIN (olddecl);

      if (fndecl  /* just one FUNCTION_DECL - replace olddecl */ &&
	  TREE_CODE (olddecl) == FUNCTION_DECL &&
	  TREE_CODE (TREE_TYPE (fndecl)) == TREE_CODE (TREE_TYPE (olddecl)))
	{
	  if (DECL_INITIAL (fndecl))
	    {
#if 0	      
	      OSapi_bcopy ((char*) fndecl, olddecl, sizeof (struct tree_decl));
#else
	      memcpy (olddecl, (char*) fndecl, sizeof (struct tree_decl));
#endif
	      for (node = prev_global; node; node = TREE_CHAIN (node))
		if (TREE_CHAIN (node) == fndecl) break;
	      if (node) 
		{
		  TREE_CHAIN (node) = olddecl;
		  if (last == fndecl) last = olddecl;
		  fndecl = olddecl;
		}
	    }
	  if (was_global)
	    IDENTIFIER_GLOBAL_VALUE(DECL_ASSEMBLER_NAME(fndecl)) = fndecl;
	  if (was_id) 
	    {
	      tree id=get_id(IDENTIFIER_POINTER(DECL_ASSEMBLER_NAME(fndecl)));
	      IDENTIFIER_GLOBAL_VALUE (id) = fndecl;
	    }
	  if (was_implicit) {}
	}
      else
	if (TREE_CHAIN (prev_global) == last &&
	    TREE_CODE (last) == TREE_CODE (olddecl))
	  {
#if 0
	    OSapi_bcopy ((char*) last, olddecl, sizeof (struct tree_decl));
#else
	    memcpy (olddecl, (char*) last, sizeof (struct tree_decl));
#endif
	    TREE_CHAIN (prev_global) = olddecl;
	    last = olddecl;
	  }
	  
      if (prev_global)
	for (node = TREE_CHAIN (prev_global); node; node = TREE_CHAIN (node))
	  if (node == last) break;
	  else
	    if ((TREE_CODE(node) == VAR_DECL ||TREE_CODE(node) == FUNCTION_DECL) &&
		TREE_STATIC (node) && !TREE_PUBLIC (node) ||
		TREE_CODE (node) == TYPE_DECL)
	      {
		push_typedecl_value_list (node);
		unset_static_global_value (node);
	      }
      return (prev_global ? TREE_CHAIN (prev_global) : 0);
    }
  else /* rc != 0 */
    {
      if (was_global)
	IDENTIFIER_GLOBAL_VALUE(DECL_ASSEMBLER_NAME(olddecl)) = olddecl;
      if (was_id) IDENTIFIER_GLOBAL_VALUE (was_id) = olddecl;

      if (was_implicit)
	SET_IDENTIFIER_IMPLICIT_DECL (DECL_ASSEMBLER_NAME (olddecl),was_implicit);
      return 0;
    }

#if 0
  if (TREE_STATIC (olddecl) && TREE_PUBLIC (olddecl) == 0)
    /* static function was saved in value list, remove from it */
    pop_typedecl_value_list (olddecl);
#endif
}

int ast_is_decl_global (decl)
     tree decl;
{
  int rc = 0;

  if (decl == 0) goto ret;

  switch (TREE_CODE (decl))
    {
    case FUNCTION_DECL:
      if (DECL_CONTEXT (decl) == NULL_TREE)
	/* for example, "static int x;" */
	rc = 1;
      break;
    case LABEL_DECL:
      rc = 0;
      break;
    case CONST_DECL:
      /* .... should be something */
      
      break;
    case TYPE_DECL:
      /* .... should be something */
      
      break;
    case VAR_DECL:
      if (DECL_CONTEXT (decl) == NULL_TREE)
	/* for example, "static int x;" */
	rc = 1;
      else
	if (TREE_PUBLIC (decl)) /* could be global */
	  if (TREE_CODE (DECL_CONTEXT (decl)) == LET_STMT &&
	      STMT_SUPERCONTEXT (DECL_CONTEXT (decl)) == NULL_TREE)
	    rc = 1;
      break;
    case PARM_DECL:
      rc = 0;
      break;
    case RESULT_DECL:
      rc = 0;
      break;
    case FIELD_DECL:
      /* .... should be something */
      
      break;
    default:
      if (ast_hook == 0)
	printf ("parser: not _DECL node\n");
      break;
    }
 ret:
  return rc;
}

static int mark_old_ast (targ)
     tree targ;
{
  int rc  = 0;
  int temp;
  if (targ)
    {
      enum tree_code code = TREE_CODE (targ);
      tree t;
      switch ( *tree_code_type [(int) code] )
	{
	case 'x':
	  if (code == TREE_LIST)
	    {
	      mark_old_ast (TREE_PURPOSE (targ));
	      mark_old_ast (TREE_VALUE (targ));
	      mark_old_ast (TREE_CHAIN (targ));
	    }
	  else   print_no_mark (targ);
	  break;
	case 't':  print_no_mark (targ);  break;
	case 'c':  case 'd':  
	  /* *TREE_MARK_OLD (targ, temp); * 05.11.95 */
	  break;
	case 'r':  case 'e': case '1': case '2': case '<':
	  switch (tree_code_length [code])
	    {
	    case 4:  mark_old_ast (TREE_OPERAND (targ, 3));
	      /* follow through */
	    case 3:  mark_old_ast (TREE_OPERAND (targ, 2));
	      /* follow through */
	    case 2:  mark_old_ast (TREE_OPERAND (targ, 1));
	      /* follow through */
	    case 1:  mark_old_ast (TREE_OPERAND (targ, 0));
	      break;
	    }
	  if (code == CALL_EXPR || code == NEW_EXPR ||
	      code == METHOD_CALL_EXPR)
	    TREE_MARK_OLD (targ, temp);
	  break;
	case 'f':  mark_old_ast (TREE_OPERAND (targ, 0));
	  break;
	case 's':
	  switch (code)
	    {
	    case LET_STMT:
	      if (STMT_SUPERCONTEXT (targ) &&
		  STMT_SUPERCONTEXT (STMT_SUPERCONTEXT (targ)) == NULL_TREE)
		/* function body */
		;
	      if (STMT_VARS (targ) != STMT_BODY (targ))
		for (t = STMT_VARS (targ); t; t = TREE_CHAIN_MARKED (t, temp))
		  if (TREE_CODE_CLASS (TREE_CODE (t)) == 'd') 
		    TREE_MARK_OLD (t, temp); /* declared inside this block */
		  else
		    mark_old_ast (t);
	      for (t = STMT_BODY (targ); t; t = TREE_CHAIN_MARKED (t, temp))
		if (TREE_CODE_CLASS (TREE_CODE (t)) == 'd') 
		  TREE_MARK_OLD (t, temp);
		else
		mark_old_ast (t);
	      mark_old_ast (STMT_SUBBLOCKS (targ));
	      mark_old_ast (STMT_CLEANUPS (targ));
	      break;
	    case IF_STMT:
	      mark_old_ast (STMT_COND (targ));
	      mark_old_ast (STMT_THEN (targ));
	      mark_old_ast (STMT_ELSE (targ));
	      break;
	    case LOOP_STMT:
	      mark_old_ast (STMT_LOOP_VARS (targ));
	      mark_old_ast (STMT_LOOP_COND (targ));
	      mark_old_ast (STMT_LOOP_BODY (targ));
	      break;
	    case CASE_STMT:
	      mark_old_ast (STMT_CASE_INDEX (targ));
	      mark_old_ast (STMT_CASE_LIST (targ));
	      break;
	    default:
	      mark_old_ast (STMT_BODY (targ));
	      break;
	    }
	  TREE_MARK_OLD (targ, temp);
	  break;
	}
    }
  return rc;
}

static int print_no_mark (ast)
     tree ast;
{
  /* error - should not come here */
  if (getenv ("PSET_PARSER_CRASH"))
    {
      enum tree_code code = TREE_CODE (ast);
      printf ("INCREMENTAL: print_no_mark: %s\n", tree_code_name [code]);
      debug_tree (ast);
    }
  return 0;
}
  
