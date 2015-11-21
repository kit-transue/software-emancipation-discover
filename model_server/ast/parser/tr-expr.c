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

/*
//
// ------- function names ------------------------
//
//--------------------------------------------------------------------
// synopsis : traverse a given expression looking for CALL_EXPR
// 
//    
//-----------------------------------------------------------------------------
// $Log: tr-expr.c  $
// Revision 1.10 1998/08/07 15:29:55EDT wmm 
// Bug track: 15587
// Fix chunk handling for pop_save_obstack.
 * Revision 1.22  1994/07/01  18:17:07  kol
 * print message get_vfshift only if ast_hook==0 (debugging mode)
 *
 * Revision 1.21  1994/01/18  23:13:25  kol
 * traverse TRUTH_XOR_EXPR (c0511101.C from test++, also bug #5987)
 *
 * Revision 1.20  1993/12/02  14:43:39  kol
 * fix
 *
 * Revision 1.19  1993/12/02  14:33:32  kol
 * fix for current_class_decl in c-parser
 *
 * Revision 1.18  1993/12/01  19:42:15  kol
 * bug #5408
 *
 * Revision 1.17  1993/09/01  13:46:42  kol
 * fixed recently introduced bug
 *
 * Revision 1.16  1993/08/29  22:51:52  kol
 * fixes for not calling ast_call_hook for builtin_new in some cases;
 * and for not traversing save_expr second time after CALL_, NEW_EXPR also
 *
 * Revision 1.15  1993/08/27  15:06:05  kol
 * additional fix to previous change
 *
 * Revision 1.14  1993/08/27  15:02:29  kol
 * work with NEW_EXPR as with CALL_EXPR
 *
 * Revision 1.13  1993/07/28  20:35:35  kol
 * added work with SAVE_EXPR (do not traverse it repeatedly)
 *
 * Revision 1.12  1993/07/01  16:27:02  kol
 * added if (ast_hook)
 *
 * Revision 1.4  1993/02/12  20:07:11  kol
 * added case COND_EXPR to get_class
 *
 * Revision 1.3  1993/02/12  15:21:04  kol
 * added case ARRAY_REF to get_class
 *
 * Revision 1.2  1992/12/07  16:24:40  kol
 * to gcc-2.3.2
 *
 * Revision 1.1  1992/11/13  18:50:36  kol
 * Initial revision
 *
 * Revision 1.2.1.3  92/10/15  16:08:58  kol
 * *** empty log message ***
 * 
 * Revision 1.11  92/09/14  09:09:40  kol
 * bug fix
 * 
 * Revision 1.8  92/09/10  16:47:00  kol
 * *** empty log message ***
 * 
 * Revision 1.7  92/09/10  11:10:28  kol
 * code to find virtual function decl
 * 
 * Revision 1.6  92/08/04  15:07:46  kol
 * *** empty log message ***
 * 
 * Revision 1.5  92/05/25  15:32:18  kol
 * return 0 from get_func_in_call_expr 
 * 
 * Revision 1.4  92/05/22  10:39:24  kol
 * fixed bug (looking for FUNCTION_DECL in CALL_EXPR)
 * 
 * Revision 1.3  92/05/15  18:33:51  kol
 * fixed bug in WITH_CLEANUP_EXPR
 * 
 * Revision 1.2  92/05/01  10:43:56  kol
 * fixed bugs
 * 
 * Revision 1.1  92/04/21  20:40:48  kol
 * Initial revision
 * 
//
//----------------------------------------------------------
// Restrictions:
//
//----------------------------------------------------------
*/

/* include files */
#include <stdio.h>
#include "config.h"
#include "tree.h"
#include "cp-tree.h"
#include "obstack.h" 

#define VTABLE_ELT_SIZE 8

extern int ast_hook;

void init_saveexp_obstack ();
static tree * lookup_saved ();
static void push_save_obstack ();
static void push_save_to_save_obstack ();
static void pop_from_save_obstack ();
static struct obstack *saveexp_obstackp;

static traverse_base_init;

/* function declaration*/
extern tree ast_get_current_class_decl ();
void ast_set_connect_call ();
void traverse_expr ();
static void traverse_expr_here ();
void ast_global_var_hook();
#define TIMEVAR(VAR, BODY)    \
do { int otime = get_run_time (); BODY; VAR += get_run_time () - otime; } while (0)

static int traverse_time = 0;

void print_traverse_time ()
{
  OSapi_fprintf (stderr,
	   "time in traverse: %d.%06d\n",
	   traverse_time / 1000000, traverse_time % 1000000);
}
void traverse_expr (expr)
     tree expr;
{
  int start_time, this_time;
  start_time = get_run_time ();
  traverse_expr_here (expr);
  this_time = get_run_time ();
  traverse_time += this_time - start_time;
}

/* This code is for argument list inside CALL_EXPR */
/* TREE_VALUE (lst) is an argument; TREE_PURPOSE (lst) should be 0 */
/* This function sets parents and traverse full list */
void traverse_list (lst)
     tree lst;
{
  traverse_expr_here (TREE_VALUE(lst));
  if (TREE_CHAIN(lst))
    traverse_list (TREE_CHAIN (lst));
}

void traverse_expr_one (expr)
     tree expr;
{
  traverse_expr_here (TREE_OPERAND(expr,0));
}


void traverse_expr_two (expr)
     tree expr;
{
  traverse_expr_here (TREE_OPERAND(expr,0));
  traverse_expr_here (TREE_OPERAND(expr,1));  
}

void traverse_expr_three (expr)
     tree expr;
{
  traverse_expr_here (TREE_OPERAND(expr,0));  
  traverse_expr_here (TREE_OPERAND(expr,1));
  traverse_expr_here (TREE_OPERAND(expr,2));  
}

void traverse_expr_four (expr)
     tree expr;
{
  traverse_expr_here (TREE_OPERAND(expr,0));  
  traverse_expr_here (TREE_OPERAND(expr,1));
  traverse_expr_here (TREE_OPERAND(expr,2));  
  traverse_expr_here (TREE_OPERAND(expr,3));
}

static void traverse_expr_here (expr)
     tree expr;
{
  if (!expr) return;

  switch (TREE_CODE(expr))
    {
    case ERROR_MARK:
    case INTEGER_CST:
    case REAL_CST:
    case STRING_CST:
    case COMPLEX_CST:

    case LABEL_DECL: 
    case TYPE_DECL:
    case PARM_DECL:
    case RESULT_DECL:
    case TEMPLATE_DECL:
    case TEMPLATE_CONST_PARM:
    case THUNK_DECL:
      break;

    case FUNCTION_DECL:
    case CONST_DECL:
    case VAR_DECL:
    case FIELD_DECL:
      ast_global_var_hook(expr);
      break;

    case COMPONENT_REF:  
      traverse_expr_two (expr);  break;
    case BIT_FIELD_REF: traverse_expr_three (expr); break;
    case INDIRECT_REF:  
      traverse_expr_here (TREE_OPERAND(expr,0));
     break;
    case OFFSET_REF:  traverse_expr_two (expr);  break;
    case BUFFER_REF: traverse_expr_one (expr); break;
    case ARRAY_REF:  traverse_expr_two (expr);  break;
      
    case CONSTRUCTOR:  traverse_expr_two (expr); break;
      
    case COMPOUND_EXPR: 
      /* if (saveexp_obstackp == 0)	init_saveexp_obstack ();*/
      push_save_obstack (expr);
      traverse_expr_two (expr); 
      pop_from_save_obstack (expr);
      break;

    case MODIFY_EXPR:  traverse_expr_two (expr); break;
    case INIT_EXPR:  traverse_expr_two (expr); break;

    case TARGET_EXPR:  
      if (TREE_OPERAND (expr, 1) == NULL_TREE && TREE_PREV (expr))
	{
	  traverse_expr_here (TREE_OPERAND (expr, 0));
	  traverse_expr_here (TREE_PREV (expr));
	  traverse_expr_here (TREE_OPERAND(expr,2));  
	}
      else traverse_expr_three (expr); 
      break;
      
    case COND_EXPR:  
      push_save_obstack (expr);
      traverse_expr_three (expr);
      pop_from_save_obstack (expr);
      break;
    case BIND_EXPR:  /*traverse_expr_three (expr); */
      if (BLOCK_TYPE_TAGS(expr))
	traverse_expr_here(BLOCK_TYPE_TAGS(expr));
       break;

    case CALL_EXPR:  
      push_save_obstack (expr);
      traverse_expr_two (expr); ast_set_connect_call (expr);
      pop_from_save_obstack (expr);
      break;
      
    case METHOD_CALL_EXPR:  traverse_expr_four (expr); break;
    case WITH_CLEANUP_EXPR: 
      traverse_expr_here (TREE_OPERAND (expr,0));
      traverse_expr_here (TREE_OPERAND (expr,2));
      break;
    case CLEANUP_POINT_EXPR: traverse_expr_one (expr); break;
      
    case PLUS_EXPR:  
    case MINUS_EXPR: 
    case MULT_EXPR:  
    case TRUNC_DIV_EXPR:
    case CEIL_DIV_EXPR: 
    case FLOOR_DIV_EXPR:
    case  ROUND_DIV_EXPR:
    case TRUNC_MOD_EXPR: 
    case CEIL_MOD_EXPR:  
    case FLOOR_MOD_EXPR: 
    case  ROUND_MOD_EXPR:
    case RDIV_EXPR:  
    case EXACT_DIV_EXPR:  traverse_expr_two (expr); break;
      
    case FIX_TRUNC_EXPR:
    case FIX_CEIL_EXPR: 
    case FIX_FLOOR_EXPR:
    case FIX_ROUND_EXPR:  traverse_expr_one (expr); break;
      
    case FLOAT_EXPR:  traverse_expr_one (expr); break;
      
    case EXPON_EXPR:  traverse_expr_two (expr); break;
      
    case NEGATE_EXPR:  traverse_expr_one (expr); break;
      
    case MIN_EXPR:
    case MAX_EXPR:  traverse_expr_two (expr); break;
    case ABS_EXPR:
    case FFS_EXPR:  traverse_expr_one (expr); break;
      
    case LSHIFT_EXPR: 
    case RSHIFT_EXPR: 
    case LROTATE_EXPR:
    case RROTATE_EXPR:  traverse_expr_two (expr); break;
      
    case BIT_IOR_EXPR:
    case BIT_XOR_EXPR:
    case BIT_AND_EXPR:
    case BIT_ANDTC_EXPR:  traverse_expr_two (expr); break;
    case BIT_NOT_EXPR:  traverse_expr_one (expr); break;
    
    case TRUTH_ANDIF_EXPR: 
    case TRUTH_ORIF_EXPR:  
    case TRUTH_AND_EXPR:  
    case TRUTH_OR_EXPR:  traverse_expr_two (expr); break;
    case TRUTH_XOR_EXPR:  traverse_expr_two (expr); break;
    case TRUTH_NOT_EXPR:  traverse_expr_one (expr); break;
      
    case LT_EXPR: 
    case LE_EXPR: 
    case GT_EXPR: 
    case GE_EXPR: 
    case EQ_EXPR: 
    case NE_EXPR:  traverse_expr_two (expr); break;
      
    case IN_EXPR:  traverse_expr_two (expr); break;
    case SET_LE_EXPR:  traverse_expr_two (expr); break;
    case CARD_EXPR:  traverse_expr_one (expr); break;
    case RANGE_EXPR:  traverse_expr_two (expr); break;
      
    case CONVERT_EXPR:  traverse_expr_one (expr); break;
      
    case NOP_EXPR:  
    case NON_LVALUE_EXPR: traverse_expr_one (expr); break;

    case SAVE_EXPR:  
      if (lookup_saved (expr)) ;
      else {
	push_save_to_save_obstack (expr); 
	traverse_expr_two (expr); 
      }
      break;
      
    case RTL_EXPR:  /*** traverse_expr_two(expr); ***/ break;
      
    case ADDR_EXPR:  traverse_expr_one (expr); break;
      
    case REFERENCE_EXPR:  traverse_expr_one (expr); break;
      
    case ENTRY_VALUE_EXPR:  traverse_expr_one (expr); break;
    
    case COMPLEX_EXPR:  traverse_expr_two(expr); break;
    case CONJ_EXPR:  traverse_expr_one (expr); break;
    case REALPART_EXPR: 
    case IMAGPART_EXPR:  traverse_expr_one (expr); break;
      
    case PREDECREMENT_EXPR: 
    case PREINCREMENT_EXPR: 
    case POSTDECREMENT_EXPR:
    case POSTINCREMENT_EXPR:  traverse_expr_two(expr); break;

    case LABEL_EXPR:
    case GOTO_EXPR:
    case RETURN_EXPR:
    case EXIT_EXPR:
    case LOOP_EXPR:
      traverse_expr_one(expr);
      break;

    case DELETE_EXPR: 
    case VEC_DELETE_EXPR: 
      traverse_expr_two (expr); break;

    case TREE_LIST:           traverse_list (expr);    break;

    case  LABEL_STMT:
    case GOTO_STMT:
    case RETURN_STMT:
    case COMPOUND_STMT:
    case ASM_STMT:
    case EXPR_STMT:
      traverse_expr_here (STMT_BODY (expr));
      break;
    case IF_STMT: 
      traverse_expr_here (STMT_COND (expr));
      traverse_expr_here (STMT_THEN (expr));
      traverse_expr_here (STMT_ELSE (expr));
      break;
    case LOOP_STMT:
      traverse_expr_here (STMT_LOOP_VARS (expr));
      traverse_expr_here (STMT_LOOP_COND (expr));
      traverse_expr_here (STMT_LOOP_BODY (expr));
      break;
    case NEW_EXPR:  
    case VEC_NEW_EXPR:  
      push_save_obstack (expr);
      { 
	tree cexpr =0, fn = 0;
	if (TREE_OPERAND (expr, 1) && 
	    TREE_CODE (TREE_OPERAND (expr, 1)) == TREE_LIST &&
	    TREE_VALUE (TREE_OPERAND (expr, 1)) &&
	    TREE_CODE (TREE_VALUE (TREE_OPERAND (expr, 1))) == SAVE_EXPR &&
	    (cexpr = TREE_OPERAND (TREE_VALUE (TREE_OPERAND (expr, 1)), 0)) &&
	    TREE_CODE (cexpr) == CALL_EXPR &&
	    TREE_CODE (TREE_OPERAND (cexpr, 0)) == ADDR_EXPR &&
	    TREE_CODE ((fn = TREE_OPERAND (TREE_OPERAND (cexpr, 0), 0))) == FUNCTION_DECL &&
	    DECL_ASSEMBLER_NAME (fn) &&
	    IDENTIFIER_POINTER (DECL_ASSEMBLER_NAME (fn)) &&
	    strcmp (IDENTIFIER_POINTER (DECL_ASSEMBLER_NAME (fn)), "__builtin_new") == 0)
	  {
	    traverse_expr (TREE_OPERAND (expr, 0));
	    if (TREE_CHAIN (TREE_OPERAND (expr, 1)))
	      traverse_list (TREE_CHAIN (TREE_OPERAND (expr, 1)));
	  }
	else
	  traverse_expr_two (expr); 
	ast_set_connect_call (expr);
      }
      pop_from_save_obstack (expr);
      break;
    case THROW_EXPR:  traverse_expr_one (expr); break;
    case VEC_INIT_CONSTRUCTOR:
    case BASE_INIT_EXPR:
      if (TREE_OPERAND (expr, 1)) traverse_expr_here (TREE_OPERAND (expr, 1));
      break;
    case UNSAVE_EXPR:
      traverse_expr_here (TREE_OPERAND ( expr, 0));
      break;
    default: 
      if (ast_hook == 0)
      OSapi_fprintf (stderr, "PARSER: traverse_expr: TREE_CODE: %d",TREE_CODE(expr));
      break;
    }
}

extern void ast_call_hook ();

static tree get_class (exp)
     tree exp;
{
  tree type = 0, idf;

  switch (TREE_CODE (exp))
    {
    case SAVE_EXPR:
    case UNSAVE_EXPR:
    case PLUS_EXPR:
    case NOP_EXPR:
    case ARRAY_REF:
    case NON_LVALUE_EXPR:
    case ADDR_EXPR:
    case CONVERT_EXPR: 
      type = get_class (TREE_OPERAND (exp, 0)); break;
    case COND_EXPR: 
      type = get_class (TREE_OPERAND (exp, 1)); break;      
    case COMPONENT_REF:
      if (TREE_CODE (TREE_OPERAND (exp, 1)) == FIELD_DECL && 
	  (idf = DECL_NAME (TREE_OPERAND (exp, 1))) &&
	  strncmp (IDENTIFIER_POINTER (idf), VFIELD_NAME, strlen (VFIELD_NAME)) == 0)
	type = TREE_TYPE (TREE_OPERAND (exp, 0));
      else    type = get_class (TREE_OPERAND (exp, 0));
      break;
    case INDIRECT_REF: 
      if (TREE_CODE (TREE_TYPE (exp)) == RECORD_TYPE ||
	  TREE_CODE (TREE_TYPE (exp)) == UNION_TYPE)
	type = TREE_TYPE (exp);
      else type = get_class (TREE_OPERAND (exp, 0)); 
      break;

    case PARM_DECL:
    case VAR_DECL:
    case CALL_EXPR:
    case NEW_EXPR:
      if (TREE_CODE (TREE_TYPE (exp)) == POINTER_TYPE ||
	  TREE_CODE (TREE_TYPE (exp)) == REFERENCE_TYPE)
	type = TREE_TYPE (TREE_TYPE(exp));
      else
	if (TREE_CODE (TREE_TYPE (exp)) == RECORD_TYPE ||
	    TREE_CODE (TREE_TYPE (exp)) == UNION_TYPE)
	  type = TREE_TYPE (exp);
      break;
    default:
      if (ast_hook == 0)
      printf ("PARSER: get_class in tr_expr; exp==%08x\n", exp);
      type = 0;
      break;
    }
  return type;
}

static int get_vfshift (exp)
     tree exp;
{
  int shift;

  switch (TREE_CODE (exp))
    {
    case INDIRECT_REF: 
    case SAVE_EXPR:
    case UNSAVE_EXPR:
    case NOP_EXPR:
    case COMPONENT_REF:
    case NON_LVALUE_EXPR:
      shift = get_vfshift (TREE_OPERAND (exp, 0)); break;
   case PLUS_EXPR:
      shift = TREE_INT_CST_LOW (TREE_OPERAND (exp, 1));
      break;
    default:
      if (ast_hook == 0)
	printf ("PARSER: get_vfshift in tr-expr; exp==%08x\n", exp);
      shift = 0;
      break;
    }
  return shift;
}

/* This function is simply to make code smaller. */
static tree get_func_in_call_expr (exp)
     tree exp;
{
  tree  t;
  tree func = 0;

  while (TREE_CODE(exp) == UNSAVE_EXPR)
    exp = TREE_OPERAND(exp, 0);

  if (TREE_CODE (exp) == CALL_EXPR || TREE_CODE (exp) == NEW_EXPR) 
    {
      tree addr_expr = TREE_OPERAND (exp, 0);
      switch (TREE_CODE (addr_expr))
	{
	case ADDR_EXPR:
	  t = TREE_OPERAND (addr_expr, 0);
	  if (t && TREE_CODE (t) == FUNCTION_DECL)    func = t;
	  else
	    if (t && TREE_CODE (t) == NOP_EXPR)	func = TREE_OPERAND (t, 0);
	  break;
	case NON_LVALUE_EXPR:
	  func = TREE_OPERAND (addr_expr, 0);
	  break;
	case COMPONENT_REF:
	  if (TREE_CODE (TREE_OPERAND (addr_expr, 1)) == FIELD_DECL &&
	      (TREE_CODE (TREE_TYPE (TREE_OPERAND (addr_expr, 0))) == RECORD_TYPE ||
	       TREE_CODE (TREE_TYPE (TREE_OPERAND (addr_expr, 0))) == UNION_TYPE) /* &&
	      TREE_CODE(TYPE_NAME(TREE_TYPE(TREE_OPERAND(addr_expr,0))))==TYPE_DECL */)
	    if (DECL_NAME(TYPE_NAME(TREE_TYPE(TREE_OPERAND(addr_expr,0)))) &&
		strcmp(IDENTIFIER_POINTER(DECL_NAME(TYPE_NAME(TREE_TYPE(TREE_OPERAND(addr_expr,0))))), VTBL_PTR_TYPE) == 0)
	      {
		/* virtual function. */
		/* first, find class */
		tree type = get_class (TREE_OPERAND (TREE_OPERAND (addr_expr,0), 0));
		int shift = get_vfshift (TREE_OPERAND (TREE_OPERAND(addr_expr,0),0));

		if (type)
		  {
		    tree binfo = TYPE_BINFO (type);
		    tree vflist = (binfo ? TREE_VEC_ELT (binfo, 3) : 0);

		    if (vflist && TREE_CODE (vflist) == TREE_LIST)
		      {
			int i;
			for (i = 0; i<shift && vflist; i+=VTABLE_ELT_SIZE)
			  vflist = TREE_CHAIN (vflist);

			if (vflist)
			  {
			    tree aexpr = 0;
			    tree lst = 0;
			    tree val = TREE_VALUE (vflist);
			    
			    if (val) lst = TREE_OPERAND (val, 1);
			    if (lst) lst = TREE_CHAIN (lst);
			    if (lst) lst = TREE_CHAIN (lst);
			    if (lst) aexpr = TREE_VALUE (lst);
			    if (aexpr) 
			      func = TREE_OPERAND (aexpr, 0);
			  }
		      }
		  }
	      }
	  break;
	default: func = 0; break;
	}  /* end of switch */
    }
  return func;
}

/* This function is intended only for such CALL_EXPRs which contain
   FUNCTION_DECL and does not work with calls to virtual funcs 
   and pointers 
*/
extern tree my_lookup_name ();
void ast_set_connect_call (call_expr)
     tree call_expr;
{
  tree func;
  while (TREE_CODE(call_expr) == UNSAVE_EXPR)
    call_expr = TREE_OPERAND(call_expr, 0);

  if (TREE_CODE (call_expr) == CALL_EXPR ||
      TREE_CODE (call_expr) == NEW_EXPR)
    {
      func = get_func_in_call_expr (call_expr);
      if (func && TREE_CODE (func) == FUNCTION_DECL)
	/* later: need to find out func, as in exec_get_func_in_call_expr */
	{
	  tree prev = DECL_NAME (func) ? my_lookup_name (DECL_NAME (func),-2) : 0;
	  if (prev  && TREE_CODE (prev) == TREE_LIST)
	    prev = TREE_VALUE (prev);	    
	  if (ast_hook) 
	    {
	      if (traverse_base_init == 0)
		ast_call_hook (call_expr, func,prev);
	      else 
		ast_base_init_hook (DECL_CONTEXT (func), call_expr, func, prev);
	    }
	}
    }
}

tree ast_get_func_in_call_expr (exp)
     tree exp;
{
  tree func = 0;

  while (TREE_CODE(exp) == UNSAVE_EXPR)
    exp = TREE_OPERAND(exp, 0);

  if (TREE_CODE (exp) == CALL_EXPR ||
      TREE_CODE (exp) == NEW_EXPR)
    func = get_func_in_call_expr (exp);

  return func;
}

void traverse_base_init_expr (mem_list)
     tree mem_list;
{
  tree t;
  tree w_base_init;
  tree ccd = ast_get_current_class_decl ();
  w_base_init = build (BASE_INIT_EXPR, TREE_TYPE (ccd), NULL_TREE, NULL_TREE);

  TREE_OPERAND (w_base_init, 1) = mem_list;
  if (TREE_OPERAND (w_base_init, 1)) 
    DECL_CONSTRUCTOR (ccd) = w_base_init;
  
  traverse_base_init = 1;
  for (t = mem_list; t; t = TREE_CHAIN (t))
    if (TREE_VALUE (t))
      traverse_expr (TREE_VALUE (t));
  traverse_base_init = 0;
}

/************************** for SAVE_EXPR *******************/

#define obstack_chunk_alloc xmalloc
#define obstack_chunk_free free

static char *saveexp_firstobj;
static tree *savep;

void init_saveexp_obstack ()
{
  if (saveexp_obstackp == 0)
    {
      saveexp_obstackp = (struct obstack *) xmalloc (sizeof (struct obstack));
      obstack_init (saveexp_obstackp);
      saveexp_firstobj = (char*) obstack_alloc (saveexp_obstackp, 0);
    }
}

static void free_saveexp_obstack_to (addr) char * addr;  {
  obstack_free (saveexp_obstackp, addr);
}

static void push_save_obstack (exp) tree exp; {
  savep = (tree*) obstack_alloc (saveexp_obstackp, sizeof (tree)); 
  *savep = exp;
}

#define IS_IN_CHUNK(S,CHNK) ((S) > (tree*)(CHNK) && (S)<(tree*)(CHNK)->limit)  
static void pop_save_obstack ()
{
  obstack_free (saveexp_obstackp, savep);
  savep = (tree*) (IS_IN_CHUNK (savep - 1*2,saveexp_obstackp->chunk) ? savep-1*2 :
		   (tree*)saveexp_obstackp->chunk->prev ?
		   (tree*)saveexp_obstackp->chunk->prev->limit - 1*2 : 0);
}

static void push_save_to_save_obstack (exp)
     tree exp;
{
  tree t = (saveexp_firstobj ? *(tree*) saveexp_firstobj : 0);
  if (savep && t &&
      (TREE_CODE (t) == COMPOUND_EXPR || 
       TREE_CODE (t) == CALL_EXPR ||
       TREE_CODE (t) == NEW_EXPR ||
       TREE_CODE (t) == COND_EXPR))
    {
      push_save_obstack (exp);
    }
}

static void pop_from_save_obstack (exp)
     tree exp;
{
  tree t = (saveexp_firstobj ? *(tree*) saveexp_firstobj : 0);

  if (t)
    if (TREE_CODE (t) == COMPOUND_EXPR || 
	TREE_CODE (t) == CALL_EXPR ||
	TREE_CODE (t) == NEW_EXPR ||
	TREE_CODE (t) == COND_EXPR)
    do {
      t = *savep;
      pop_save_obstack ();
    } while (t != exp);
}

static tree * get_prev_save_stack (current_chunk, pp)
     struct _obstack_chunk *current_chunk;
     tree *pp;
{
  tree *s = 0;
  struct _obstack_chunk *search_chunk = current_chunk;

  do {
    if (IS_IN_CHUNK (pp -1*2, search_chunk))   break;
    search_chunk = search_chunk->prev ;
  } while (search_chunk);

  if (search_chunk)    s = pp - 1*2;

  return s;
}

static tree * lookup_saved (exp)
     tree exp;
{
  tree *s;
  tree t = (saveexp_firstobj ? *(tree*) saveexp_firstobj : 0);

/* !! < sturner, 980430: Don't test t unless an expression's on the obstack. */
  if (savep)
/* > !! */
  if (t)
    if (TREE_CODE (t) == COMPOUND_EXPR ||
	TREE_CODE (t) == CALL_EXPR ||
	TREE_CODE (t) == NEW_EXPR ||
	TREE_CODE (t) == COND_EXPR)     
      for(s = savep; s; s = get_prev_save_stack(saveexp_obstackp->chunk, s))
	if (*s == exp)  return s;
	
  return 0;
}
/************************** END for SAVE_EXPR *******************/
