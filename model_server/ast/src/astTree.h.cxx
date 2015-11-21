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
#ifndef ISO_CPP_HEADERS
#include <string.h>
#else /* ISO_CPP_HEADERS */
#include <cstring>
namespace std {};
using namespace std;
#endif /* ISO_CPP_HEADERS */
#include "machdep.h"
#include "astTree.h"
#include "ast_cplus_tree.h"
#include "setprocess.h"
#include <ast_hook.h>
#include <cLibraryFunctions.h>
#ifdef _WIN32
#include "cmd.h"
#endif
 
#ifndef _WIN32
extern char **paraset_tree_code_name;
#else
extern "C" char **paraset_tree_code_name;
#endif
extern "C" void paraset_init_tree_codes();

#ifdef _OBJ_AST_TRAVERSE_ /* NY: keep code as a description of AST traverse */
void *astTree::operator new(size_t size)
{
    return mmgr_instance->malloc ((int)size);
}

void astTree::operator delete (void *ptr)
{
    mmgr_instance->free (ptr);
    return;
}

objTree * astTree :: get_root () const
{
  enum tree_code code;
  astBlock* block;
  /*astBlock* superblock;*/
  astBlock* global_block;
  astFunction_decl* fndecl;
  astTree* root;
  if (!this)  /*generate_error_input ("get_root")*/;
  else
    {
      code = this->TREE_CODE ();
      switch ( *tree_code_type [(int) code] )
	{
	case 's':
	  block = ((astStmts*) this)->STMT_CONTEXT ();
	  fndecl = ast_search_function_by_block (block);
	  root = fndecl;
	  break;
	case 'd': 
	  if (this->TREE_CODE () == FUNCTION_DECL)
	    return (objTree*) this;
	  if (this->TREE_CODE () == VAR_DECL)
	    {
	      block = ((astDecl*) this)->DECL_CONTEXT ();
	      global_block =  ast_get_global_block ();
	      if (block == global_block)
		root = block;
	      else
		if (block->STMT_SUPERCONTEXT () == global_block)
		  root = block;
		else
		  root = ast_search_function_by_block (block);
	    }
	  else
	    if (this->TREE_CODE () == PARM_DECL)
	      root = ast_search_function_by_arg ((astDecl*) this);
	     /*root = (astTree*) ((astTree*) this)->get_parent ();*/
	  break;

	case 'e':
	  {
	    astTree * tt;
	    astTree * t = 0; 
	    tt = (astTree*) ((astTree*) this)->get_parent ();
	    while (tt)
	      {
		tt = (astTree*) tt->get_parent ();
		if (tt)
		  t = tt;
	      }
	    
	    root = t;
	  }
	  break;

	  default:
	  /*superblock*/ root = 0;
	  break;
	}
    }
  return ((objTree*)root) /*superblock*/;
}     

//------------------------------------------------------------
// get_first, get_next, get_parent, get_first
static astDecl * ast_get_first_arg (astFunction_decl * func)
{
  return ast_get_first_parm (func);
}

/* this function is for C++. It check the name of PARM_DECL,
   if it is THIS (name = this), */ 	
static int is_parm_this (astDecl *p)
{
  char * s; 
  astIdentifier *id = p->DECL_NAME (); 
  if (id) {
    s = id->IDENTIFIER_POINTER ();
    if (!strcmp (s, THIS_NAME))
      return 1;
  }
  return 0;
}

static astFunction_decl *ast_search_method_by_this (astDecl * arg)
{
  astFunction_decl * func = 0;
  astFunction_decl **methods = 0;
  astType *type;
  astVec *method_vec = 0;
  int len = 0;

  if (!is_parm_this (arg)) return 0;
  type = arg->TREE_TYPE ();
  if (!type) return 0;
  /* get RECORD_TYPE */
  type = type->TREE_TYPE (); 
  if (!type || !IS_AGGR_TYPE_CODE (type->TREE_CODE())) return 0;

  method_vec = (astVec*) CLASSTYPE_METHOD_VEC (type);
  if (!method_vec) return 0;
  len = method_vec->TREE_VEC_LENGTH ();

  astFunction_decl **end = (astFunction_decl **)method_vec->TREE_VEC_END();
  methods = (astFunction_decl **) (end - method_vec->TREE_VEC_LENGTH());

  if (*methods != 0)
    for (func = *methods; func; func = (astFunction_decl*)func->TREE_CHAIN ())
      if (arg == func->DECL_ARGUMENTS())
	return func;
  
  func = 0;
  methods++;
  
  while (methods != end)
    {
      func = *methods;
      if (arg == func->DECL_ARGUMENTS())
	  break;
      methods++;
      func = 0;
    }

  return func;
} 

static astFunction_decl *search_function_by_first_arg (astDecl * arg)
{
  astFunction_decl * func;
  char * name;

  if (!arg) return 0;

  if (is_parm_this (arg))
    return ast_search_method_by_this (arg);

  ast_reset_get_fnames ();
  do
    {
      func = 0;
      name = ast_get_fname ();
      if (name)
	{
	  func = get_func_decl (name);
	  if (func)
	    if (ast_get_first_arg (func) == arg) 
	      break;
	}
    }
  while (func);
  return func;
}

static astFunction_decl * ast_search_function_by_arg (astDecl * arg)
{
  astDecl * b;
  astDecl * first_arg;
  for (b = arg; b; b = (astDecl*) b->TREE_PREV ())
    first_arg = b;
  if (first_arg)
    return search_function_by_first_arg (first_arg);
  else return 0;
}

objTree* astTree :: get_first () const 
{
  enum tree_code code = ((astTree*) this)->TREE_CODE();
   switch ( *tree_code_type [(int) code] )
    {
    case 's':
      if (code == LET_STMT)
	return (objTree*) get_first_stmt ((astBlock*) this);
      break;
    case 'd':
      if (code == FUNCTION_DECL)
	return (objTree*) ast_get_first_block ((astFunction_decl*)this);
      default:
      return 0;
    }
}

objTree * astTree :: get_next () const
{
  enum tree_code code = ((astTree*) this)->TREE_CODE();
  switch ( *tree_code_type [(int) code] )
    {
    case 's':
      return (objTree*) get_next_stmt ((astStmts*) this);
     case 'd':
      if (code == VAR_DECL)
	return (objTree*) ast_get_next_var ((astDecl*)this);
       default:
      return 0;
    }
}

objTree * astList :: get_first () const
{
  astTree * t;
  t = TREE_PURPOSE ();
  if (!t) 
    t = TREE_VALUE ();
  return t;
}

objTree * astList :: get_prev () const
{
  astTree * t;
  t = TREE_PREV ();

  return t;
}
//
// Description. This function returns NEXT node for declaration.
// Usually it is the next node of the same kind. 
// So in most of cases it would be simple TREE_CHAIN ().
//
// This function handles the following exceptions.
//
// - When the next node of the same kind does not exist, for
//   PARM_DECL this function returns "first block".
//   The reason for this is that FUNCTION_DECL has 2 different child: chain of arguments and block;
//   we don't have in AST node which represents full chain of arguments - first child of FUNCTION_DECL
//   is the first argument.
// - And for VAR_DECL this function returns first statement (=block->STMT_BODY()), 
//   when there no TREE_CHAIN ().
//
objTree * astDecl :: get_next () const
{
  astTree * t;
  astFunction_decl * func;
  astBlock * block;
  astDecl * decl;
  enum tree_code code = TREE_CODE ();
  
  decl = (  astDecl *)this;
  if ((t = TREE_CHAIN ()))
    ;
  else
    switch (code)
      {
      case PARM_DECL:
	func = ast_search_function_by_arg (decl);
	if (func)
	  t = ast_get_first_block (func);
	else t = 0;
	break;
      case VAR_DECL:
      case CONST_DECL:
      case TYPE_DECL:
	block = DECL_CONTEXT ();
	if (block)
	  {
	    t = get_first_stmt (block);
	    if (t == block->STMT_VARS ())
	      t = 0;
	  }
	else t = 0;
	break;
      case FUNCTION_DECL:
	t = 0;
	break;
      case LABEL_DECL: 
	/* I believe this is DEFAULT in SWITCH statement */
	t = DECL_INITIAL ();
	break;
      default:
	t = 0;
	break;
      }
  return t;
}

//
// Description. This function returns PARENT node for declaration.
// For PARM_DECL it is function.
// For VAR_DECL, CONST_DECL, TYPE_DECL it is their context.
// FUNCTION_DECL has its own get_parent.
// FIELD_DECL exists inside type (RECORD_ or UNION_) and does not exist for each VAR_DECL with this type.
// LABEL_DECL,  RESULT_DECL are not supported here (yet)
objTree * astDecl :: get_parent () const
{
  astTree * t;
  enum tree_code code = TREE_CODE ();

  switch (code)
    {
    case PARM_DECL:
      t = ast_search_function_by_arg ((  astDecl *)this);
      break;
    case VAR_DECL:
    case CONST_DECL:
    case TYPE_DECL:
      t = DECL_CONTEXT ();
      break;
    }
  return t;
}

//
// Description. This function returns the FIRST node for FUNCTION_DECL.
// It is the first argument of the function if function has arguments, 
// othewise it is the first block of the function.
//
objTree * astFunction_decl :: get_first () const
{
  astTree * t;
  astFunction_decl* func;
  
  func = (astFunction_decl *) this;
  if ((t = ast_get_first_arg (func)));
  else
    t = ast_get_first_block (func);
  return t;
}

//
// Description. This function returns the PARENT node for FUNCTION_DECL.
// It is NULL. 
objTree *   astFunction_decl :: get_parent () const
{
  return 0;
}

//
// Description. 
// Function checks, is this expression NOP_EXPR, and operand of this NOP__EXPR
// is not _EXPR node, but is _DECL,  or _CST node.
//
static int nop_expr_last (astExp * expr)
{
  enum tree_code code;
  enum tree_code cod;
  int rc = 0; 
  cod = expr->TREE_CODE ();
  switch (cod)
    {
    case INDIRECT_REF: case BUFFER_REF: case FIX_TRUNC_EXPR: case FIX_CEIL_EXPR:
    case FIX_FLOOR_EXPR: case FIX_ROUND_EXPR: case FLOAT_EXPR: case NEGATE_EXPR:
    case ABS_EXPR: case FFS_EXPR: case BIT_NOT_EXPR: case TRUTH_NOT_EXPR: 
    case CARD_EXPR: case CONVERT_EXPR: case NOP_EXPR: case ADDR_EXPR:
    case REFERENCE_EXPR: case ENTRY_VALUE_EXPR: case CONJ_EXPR: 
    case REALPART_EXPR: case IMAGPART_EXPR:
      code = (((astExp1*) expr)->TREE_OPERAND(0))->TREE_CODE ();
      switch (code)
	{
	case FUNCTION_DECL: case CONST_DECL: case TYPE_DECL: case VAR_DECL: case PARM_DECL:
	case RESULT_DECL: case INTEGER_CST: case REAL_CST: case STRING_CST:
	case COMPLEX_CST: case ERROR_MARK:
	/*case FIELD_DECL: cannot be such case */
	  rc = 1;
	  break;
	default: rc = 0;
	}
    }
  return rc;
}

// This function is simply to make code smaller.
static astFunction_decl*
  get_func_in_call_expr (astExp* exp)
{
  astExp2* addr_expr;
  astTree * t;
  astFunction_decl * func = 0;
  if (exp->TREE_CODE () == CALL_EXPR) 
    {
      addr_expr = (astExp2*) ((astExp3*) exp)->TREE_OPERAND (0);
      t = addr_expr->TREE_OPERAND (0);
      if (t)
	if (t->TREE_CODE () == FUNCTION_DECL)
	  func = (astFunction_decl*) t;
	else
	  if (t->TREE_CODE () == NOP_EXPR)
	    func = (astFunction_decl*) ((astExp1*) t)->TREE_OPERAND (0);
      return func;
    }
  else return 0;
}

//
// Description. This function returns the FIRST child node for EXPR.
// Usually it is the first operand of this expression.
// But if the first operand is NOP_EXPR, and operand of this NOP__EXPR
// is not _EXPR node, is _DECL,  or _CST node, then this function
// returns that _DECL or _CST node. And, with accordance with this,
// if THIS expression is such NOP_EXPR, function returns 0.
//
objTree *  astExp :: get_first () const
{
  astTree * t;
  astExp * expr = (  astExp *)this;    
  astFunction_decl * func;
  if (nop_expr_last (expr))
    t = 0;
  else
    {
      t = ((astExp1*) expr)->TREE_OPERAND (0);
      if (nop_expr_last ((astExp*) t))
	;
      /* The following code is for assosciativity - 08.28.91 */
      if (TREE_CODE () == CALL_EXPR)
	{
	  func = get_func_in_call_expr (expr);

	  /* changed  02.12.92 */
	  /*put_relation(CalledBy,func,expr);*/
	  ast_call_hook (expr, func);
	}
    }
  return t;
}

char* astExp :: get_name () const
{
  enum tree_code code;
  astTree * t;

  char * name = 0;

  code = TREE_CODE ();
  int rc;
  switch (code)
    {
    case INDIRECT_REF: case BUFFER_REF: case FIX_TRUNC_EXPR: case FIX_CEIL_EXPR:
    case FIX_FLOOR_EXPR: case FIX_ROUND_EXPR: case FLOAT_EXPR: case NEGATE_EXPR:
    case ABS_EXPR: case FFS_EXPR: case BIT_NOT_EXPR: case TRUTH_NOT_EXPR: 
    case CARD_EXPR: case CONVERT_EXPR: case NOP_EXPR: case ADDR_EXPR:
    case REFERENCE_EXPR: case ENTRY_VALUE_EXPR: case CONJ_EXPR: 
    case REALPART_EXPR: case IMAGPART_EXPR:
      {
	enum tree_code code;
	code = (((astExp1*) this)->TREE_OPERAND(0))->TREE_CODE ();
	switch (code)
	  {
	  case FUNCTION_DECL:case CONST_DECL:  case TYPE_DECL:  case VAR_DECL:  case PARM_DECL:
	  case RESULT_DECL:  case INTEGER_CST:  case REAL_CST:  case STRING_CST:  case COMPLEX_CST:
	  case ERROR_MARK:
	    /*case FIELD_DECL: cannot be such case */
	    rc = 1; break;
	    default: rc = 0;
	  }
	if (rc)
	  {
	    t = ((astExp1*) this)->TREE_OPERAND (0);
	    name = t->get_name ();
	  }
      }
      break;
      default:
      t = (astTree*) this->get_root ();
      name = t->get_name ();
      break;
    }
  return name;
}

//
// Description. This function returns the PREVIOUS node for EXPR.
// Usually it is previous operand of their parent expression.
// If parent expression does not have previous operand, return 0.
// If parent is not expression, function returns previous node
// of another kind (look astStmts::get_next() )
//
objTree *  astExp :: get_prev () const
{ 
  astTree * parent;
  astList * lst;
  astTree * t = 0;
  enum tree_code code;

  astExp * exp = (astExp *)this;
  
  parent = TREE_PARENT ();
  code = parent->TREE_CODE ();
  switch (code)
    {
    case LOOP_STMT:
      lst = ((astLoop*) parent)->STMT_LOOP_VARS ();
      if (lst) 
	{
	  t = lst->TREE_PURPOSE ();
	  if (!t) t = lst->TREE_VALUE ();
	}
      break;
    case IF_STMT:
      // for if-condition there is no previous node
      break;
    case CASE_STMT:
      // as a STMT_CASE_INDEX  
      break;
    case TREE_LIST:   
      // inside SWITCH, as one of the cases index
      t = parent->TREE_PREV ();
      if (t)
	{
	  /* if previous CASE is not finished by BREAK, then as follow: */
	  if (t->TREE_CODE () == TREE_LIST)
	    {
	      t = ((astList*) t)->TREE_VALUE ();
	      if (t)
		if (t->TREE_CODE () == LABEL_DECL)
		  t = ((astDecl*) t)->DECL_INITIAL ();
	    }
	}
      break;
    case COMPONENT_REF:
    case OFFSET_REF:
    case ARRAY_REF:
      // here are all more than one operand's expressions
      // for which I don't know what to return as a previous
      break;
    case CONSTRUCTOR:
    case COMPOUND_EXPR:  
    case MODIFY_EXPR:  
    case INIT_EXPR:
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
    case EXACT_DIV_EXPR:
    case EXPON_EXPR:
    case MIN_EXPR:
    case MAX_EXPR:
    case LSHIFT_EXPR: 
    case RSHIFT_EXPR:
    case LROTATE_EXPR:
    case RROTATE_EXPR: 
      
    case BIT_IOR_EXPR: 
    case BIT_XOR_EXPR: 
    case BIT_AND_EXPR: 
    case BIT_ANDTC_EXPR:
    case TRUTH_ANDIF_EXPR:
    case TRUTH_ORIF_EXPR: 
    case TRUTH_AND_EXPR:  
    case TRUTH_OR_EXPR: 
    case LT_EXPR: 
    case LE_EXPR: 
    case GT_EXPR: 
    case GE_EXPR: 
    case EQ_EXPR: 
    case NE_EXPR: 
      
    case IN_EXPR: 
    case SET_LE_EXPR:
    case RANGE_EXPR:
    case SAVE_EXPR: 
    case RTL_EXPR:
    case WRAPPER_EXPR:
    case ANTI_WRAPPER_EXPR:
    case COMPLEX_EXPR:
    case PREDECREMENT_EXPR: 
    case PREINCREMENT_EXPR: 
    case POSTDECREMENT_EXPR:
    case POSTINCREMENT_EXPR:
      if (exp == ((astExp2*) parent)->TREE_OPERAND(1))
	t = ((astExp2*) parent)->TREE_OPERAND(0);
      break;
    case NEW_EXPR:
    case COND_EXPR:
    case CALL_EXPR:
    case WITH_CLEANUP_EXPR:
      if (exp ==  (astExp*) ((astExp3*) parent)->TREE_OPERAND(2))
	t = ((astExp3*) parent)->TREE_OPERAND(1);
      else
      if (exp == (astExp*) ((astExp3*) parent)->TREE_OPERAND(1))
	t = ((astExp3*) parent)->TREE_OPERAND(0);
      break;
    case METHOD_CALL_EXPR:
      if (exp == (astExp*) ((astExp4*) parent)->TREE_OPERAND(3))
	t = ((astExp4*) parent)->TREE_OPERAND(2);
      else
      if (exp == (astExp*) ((astExp4*) parent)->TREE_OPERAND(2))
	t = ((astExp4*) parent)->TREE_OPERAND(1);
      else
      if (exp == (astExp*) ((astExp4*) parent)->TREE_OPERAND(1))
	t = ((astExp4*) parent)->TREE_OPERAND(0);
      break;
      }
  return t;
}

//
// Description. This function returns the NEXT node for EXPR.
// Usually it is the next operand of their parent expression.
// If parent expression does not have next operand, return 0.
// If parent is not an expression, function returns previous node
// of another kind (----look astStmts::get_prev() )
//
objTree *  astExp :: get_next () const 
{ 
  astTree * parent;

  astTree * t = 0;
  astTree * tt = 0;
  enum tree_code code;

  astExp * exp = (astExp *)this;
  
  parent = TREE_PARENT ();
  code = parent->TREE_CODE ();
  switch (code)
    {
    case LOOP_STMT:
      t = ((astLoop*) parent)->STMT_LOOP_BODY ();
      break;
    case IF_STMT:
      // for if-condition - then-stmt
      t = ((astIf*) parent)->STMT_THEN ();
      break;
    case CASE_STMT:
      // as a STMT_CASE_INDEX
      tt = ((astCase*) parent)->STMT_CASE_LIST ();
/*      if (tt) tt = ((astBlock*) tt)->STMT_BODY ();
      if (tt) tt = ((astList*) tt)->TREE_PURPOSE ();*/
      t = tt;
      break;
    case TREE_LIST:   
      // inside SWITCH, as one of the cases index
      /* or inside CALL_EXPR */
      tt = ((astList*) parent)->TREE_VALUE ();
      if (tt) 
	if (tt->TREE_CODE () == LABEL_DECL)
	  tt = ((astDecl*) tt)->DECL_INITIAL ();
	else
	  /* we aree here inside CALL_EXPR, TREE_OPERAND(1) */
	  if (exp == tt)
	    {
	      parent = parent->TREE_CHAIN ();
	      if (parent)
		tt = ((astList*) parent)->TREE_VALUE ();
	      else tt = 0;
	    }
      t = tt;
      break;
    case COMPONENT_REF:
    case OFFSET_REF:
    case ARRAY_REF:
      // here are all more than one operand's expressions
      // for which I don't know what to return as a previous
      break;
    case CONSTRUCTOR:
    case COMPOUND_EXPR:  
    case MODIFY_EXPR:  
    case INIT_EXPR:
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
    case EXACT_DIV_EXPR:
    case EXPON_EXPR:
    case MIN_EXPR:
    case MAX_EXPR:
    case LSHIFT_EXPR: 
    case RSHIFT_EXPR:
    case LROTATE_EXPR:
    case RROTATE_EXPR: 
      
    case BIT_IOR_EXPR: 
    case BIT_XOR_EXPR: 
    case BIT_AND_EXPR: 
    case BIT_ANDTC_EXPR:
    case TRUTH_ANDIF_EXPR:
    case TRUTH_ORIF_EXPR: 
    case TRUTH_AND_EXPR:  
    case TRUTH_OR_EXPR: 
    case LT_EXPR: 
    case LE_EXPR: 
    case GT_EXPR: 
    case GE_EXPR: 
    case EQ_EXPR: 
    case NE_EXPR: 
      
    case IN_EXPR: 
    case SET_LE_EXPR:
    case RANGE_EXPR:
    case SAVE_EXPR: 
    case RTL_EXPR:
    case WRAPPER_EXPR:
    case ANTI_WRAPPER_EXPR:
    case COMPLEX_EXPR:
    case PREDECREMENT_EXPR: 
    case PREINCREMENT_EXPR: 
    case POSTDECREMENT_EXPR:
    case POSTINCREMENT_EXPR:
      if (exp == ((astExp2*) parent)->TREE_OPERAND(0))
	t = ((astExp2*) parent)->TREE_OPERAND(1);
      break;
    case NEW_EXPR:
    case COND_EXPR:
    case CALL_EXPR:
    case WITH_CLEANUP_EXPR:
      if (exp == (astExp*) ((astExp3*) parent)->TREE_OPERAND(0))
	{
	  t = ((astExp3*) parent)->TREE_OPERAND(1);
	  if ((code == CALL_EXPR)
	     && t && (t->TREE_CODE () == TREE_LIST))
	    t = ((astList *) t)->TREE_VALUE ();
	}
      else
      if (exp == (astExp*) ((astExp3*) parent)->TREE_OPERAND(1))
	t = ((astExp3*) parent)->TREE_OPERAND(2);
      break;
    case METHOD_CALL_EXPR:
      if (exp == (astExp*) ((astExp4*) parent)->TREE_OPERAND(0))
	t = ((astExp4*) parent)->TREE_OPERAND(1);
      else
      if (exp == (astExp*) ((astExp4*) parent)->TREE_OPERAND(1))
	t = ((astExp4*) parent)->TREE_OPERAND(2);
      else
      if (exp == (astExp*) ((astExp4*) parent)->TREE_OPERAND(2))
	t = ((astExp4*) parent)->TREE_OPERAND(3);
      break;
      }
  return t;
}

//
// Description. This function returns the PREVIOUS node for STMTS.
// Usually it is TREE_PREV(). 
// If TREE_PREV () does not exist, then this function returns last declaration 
// (look astDecl::get_next() )
//
objTree *  astStmts :: get_prev () const
{ 
  astStmts * stmt;
  astTree * parent;
  enum tree_code code;
  astTree * t = 0;
  astFunction_decl * func;
  astBlock * block;
  
  if ((t = TREE_PREV ()));
  else
    {
      stmt = (astStmts *)this;
      parent = TREE_PARENT ();
      if ((parent) && (parent->TREE_CODE () != LET_STMT))
	{
	  code = parent->TREE_CODE ();
	  switch (code)
	    {
	    case IF_STMT:
	      if (stmt == ((astIf*) parent)->STMT_ELSE ())
		t = ((astIf*) parent)->STMT_THEN ();
	      else
	      if (stmt == ((astIf*) parent)->STMT_THEN ())
		t = ((astIf*) parent)->STMT_COND ();
	      break;
	    case LOOP_STMT:
	      {
		astLoop * lp;
		lp = (astLoop*) parent;
		if (stmt == lp->STMT_LOOP_BODY ())
		  t = lp->STMT_LOOP_COND ();
		else
		  {
		    astList * lst;
		    lst = lp->STMT_LOOP_VARS ();
		    if (stmt == lst->TREE_VALUE ())
		      /* stmt == init */
		      t = 0;
		    else
		      if (stmt == lst->TREE_PURPOSE ())
			/* stmt == delta */
			t = lst->TREE_VALUE ();
		  }
	      }
	      break;
	    case CASE_STMT:
	      if (stmt == ((astCase*) parent)->STMT_CASE_LIST ())
		t = ((astCase*) parent)->STMT_CASE_INDEX ();
	      break;
	    case TREE_LIST:
	      /* can be inside of LOOP_STMT  - "init" or "delta" */
	      if (stmt == ((astList*) parent)->TREE_PURPOSE ())
		t = ((astList*) parent)->TREE_VALUE ();
	      break;
	    }
	}
      else
	{
	  block = get_context (stmt);
	  if (stmt->TREE_CODE() == LET_STMT)
	    {
	      if (block)
		if (block->STMT_SUPERCONTEXT ()) 
		  {
		    if (block->STMT_VARS() != block->STMT_BODY())
		      t = get_last_var (block);  /* THIS is the block inside of first block.*/
		  }
		else
		  if (!block->STMT_CONTEXT()) t = 0;/* THIS is the UNIT BLOCK. */
		  else
		    {
		      /* block is the superblock of the function, and THIS is the first block.*/
		      /* So find last argument (look get_next for astDecl) */
		      
		      func = search_function_by_superblock (block);
		      if (func)
			t = ast_get_last_parm (func);
		    }
	      else t = 0;
	    }
	  else
	  if (block)
	    if (block->STMT_VARS() != block->STMT_BODY())
	      t = get_last_var (block);
	}
    }
  return t;
}

//
// Description. This function returns the NEXT node for a given LIST
// by checking its parent (look astStmts::get_next and get_next_stmt_from_parent).
//
static astTree *get_next_stmt_given_list_from_parent (astList * ls)
{
  astTree * parent;
  astTree * t = 0;
  enum tree_code code;

  parent = (astTree*) ls->get_parent ();
  if (!parent) return 0;

  code = parent->TREE_CODE ();
  switch (code)
    {
    case LOOP_STMT:
      if (ls == ((astLoop*) parent)->STMT_LOOP_VARS ())
	t = ((astLoop*) parent)->STMT_LOOP_COND ();
      break;
    default:        /* for example, inside SWITCH */
      t = 0; 
      break;
    }
  return t;
}

//
// Description. This function returns the NEXT node for a given STMT
// by checking its parent (look astStmts::get_next).
//
static astTree *get_next_stmt_from_parent (astStmts * stmt)
{
  astTree * parent;
  astTree * t = 0;
  enum tree_code code;

  if ((stmt->TREE_PARENT ()) &&
      (stmt->TREE_PARENT ())->TREE_CODE () == LABEL_DECL)
    /* This is a subtree of SWITCH statement */
    parent = stmt->TREE_PARENT ();
  else
    parent = (astTree*) stmt->get_parent ();

  if (!parent) return 0;

  code = parent->TREE_CODE ();
  switch (code)
    {
    case TREE_LIST:
      if (stmt == ((astList *) parent)->TREE_PURPOSE ())
	{
	  if ((t = ((astList *) parent)->TREE_VALUE ())) 
	    {
	      if (t->TREE_CODE () == LABEL_DECL)
		/* inside SWITCH statement */
		t = ((astDecl*) t)->DECL_INITIAL ();
	    }
	  else
	    if ((t = (astTree*) parent->get_next ())) ;
	    else
	      {
		t = get_next_stmt_given_list_from_parent ((astList*) parent);
	      }
	}
      else
	if (stmt == ((astList *) parent)->TREE_VALUE ())
	  if ((t = (astTree*) parent->get_next ())) ;
	  else
	    {
	      t = get_next_stmt_given_list_from_parent ((astList*) parent);
	    }
      break;
    case IF_STMT:
      if (stmt == ((astIf*) parent)->STMT_THEN ())
	t = ((astIf*) parent)->STMT_ELSE ();
      else t = 0;
      break;
    case LOOP_STMT:
      {
	astLoop * lp;
	lp = (astLoop*) parent;
	if (stmt == lp->STMT_LOOP_BODY ())
	  t = 0;
      else
	{
	  if (stmt == ((astList*) lp->STMT_LOOP_VARS ())->TREE_VALUE ())
	    t = ((astList*) lp->STMT_LOOP_VARS ())->TREE_PURPOSE ();
	  else
	  if (stmt == ((astList*) lp->STMT_LOOP_VARS ())->TREE_PURPOSE ())
	    t = lp->STMT_LOOP_COND ();
	}
      }
      break;
    case LABEL_DECL:
      {
	/* This is a subtree of SWITCH statement */
	tree tt;
	tt = (astTree*) parent->TREE_PARENT ();
	if (tt)
	  if (tt->TREE_CODE () == TREE_LIST)
	    if ((t = tt->TREE_CHAIN ())) ;
	    else ;
      }
      break;
    }
  return t;
}
//
// Description. This function returns the NEXT node for STMTS.
// Usually it is TREE_CHAIN(). 
// If TREE_CHAIN () does not exist, then this function takes parent and
// looks its code; sometimes it takes the NEXT from parent.
//
objTree *  astStmts :: get_next () const
{ 
  astStmts * stmt;
  astTree * t = 0;
  
  if ((t = TREE_CHAIN ()));
  else
    {
      stmt = (astStmts *)this;
      t = get_next_stmt_from_parent (stmt);
	
    }
  return t;
}

//
// Description. This function returns the first child node for STMTS.
// - Control statements such as IF, LOOP, SWITCH have their own
//   methods. The same about BLOCK.
//   This function returns expression.
// 
// 
//
objTree *   astStmts :: get_first () const
{
  astStmts * stmt;
  astTree * t = 0;


  enum tree_code code;

  stmt = (astStmts *) this;
  if (!stmt) return 0;
  
  code = TREE_CODE ();
  switch (code)
    {
    case EXPR_STMT:
    case GOTO_STMT:
    case RETURN_STMT:
      t = ((astStmt*) stmt)->STMT_BODY ();
      break;
    case COMPOUND_STMT:
      t = ((astStmt*) stmt)->STMT_BODY ();
      break;
    case EXIT_STMT:
      t = 0;
      break;
    }
  return t;
}

//
// Description. This function returns the PARENT node for STMTS.
// - For statements which are childs of such statements as IF, LOOP, CASE it is, of course,
//   IF, LOOP, CASE. For example, for then_part PARENT will be IF_STMT.
// - For first_block PARENT is a function.
// - Otherwise PARENT is containing block.
//
objTree * astStmts :: get_parent () const
{
  astTree * parent;
  astStmts * stmt;
  astTree * t;
  astFunction_decl* func;
  astBlock * block;
  enum tree_code code;

  stmt = (astStmts *)this;
  if (!stmt) return 0;

  parent = TREE_PARENT ();
  if (parent)
    {
      code = parent->TREE_CODE ();
      switch (code)
	{
	case TREE_LIST:
	  t = parent->TREE_PARENT (); 
	  if ((t) && t->TREE_CODE () == LOOP_STMT)
	    /*t = parent*/;
	  break;
	case LABEL_DECL:
	  /* inside SWITCH, go up to STMT_CASE_LIST */
	  parent = parent->TREE_PARENT ();
	  if (parent) t = parent->TREE_PARENT ();
	  break;
	case IF_STMT:
	case LOOP_STMT:
	case CASE_STMT:
	case LET_STMT:
	  t = parent;
	  break;
	  default:
        t = parent;
	}
    }
  else
    {
      if (TREE_CODE () == LET_STMT)
	  block = ((astBlock*) stmt)->STMT_SUPERCONTEXT ();
      else 	block = STMT_CONTEXT ();
      
/*      if (block && ((t = containing_stmt (block, stmt)) != stmt))
	return t;
      else 
*/
      if (TREE_CODE () == LET_STMT)
	{
	  if (block) 
	    if (block->STMT_SUPERCONTEXT ()) t = block;
	    else 
	      if ((func = search_function_by_superblock (block))) t = func;
	      else t = 0;
	  else 
	    if ((func = search_function_by_superblock ((astBlock *) stmt)))
	      t = (astTree *) func;
	    else t = STMT_CONTEXT ();
	}
      else
	  t = STMT_CONTEXT ();
    }
  return t;
}

//
// Description. This function returns the FIRST node for BLOCK.
// If block contains variables, it returns the first variable.
// If not, function checks STMT_BODY.
// If code of STMT_BODY is TREE_LIST (this is inside SWITCH,
// STMT_CASE_LIST), then function returns TREE_PURPOSE.
// Otherwise it returns STMT_BODY.
// /*  -----Exception is the first block of the function.    */
// /*  -----For it function returns STMT_SUBBLOCKS.          */
//
objTree *   astBlock :: get_first () const
{
  astTree * t = 0;
  astTree * tt = 0;

  t = STMT_VARS ();

  if (!t) 
    {
      t = STMT_BODY ();
      if (t)
	if (t->TREE_CODE () == TREE_LIST)
	  t = ((astList*) t)->TREE_PURPOSE ();
    }
  return t;
}

//
// Description. This function returns the PREVIOUS node for BLOCK.
// Usually it is TREE_PREV ()
// Exception is the first block of the function.
// For this block PREVIOUS should be the last argument of function (look astDecl::get_next()) const
//
//objTree * 
//  astBlock :: get_prev () const
//{
//  astTree * t = 0;
//  astTree * tt = 0;
//  astFunction_decl* func;
//  astBlock * block;
//
//  if ((t = TREE_PREV ()));
//  else                                     /* if THIS is the first block, and function has arguments, */
//    {                                      /* return last argument, otherwise 0. */
//      block = STMT_SUPERCONTEXT ();
//      if (block)
//	if (block->STMT_SUPERCONTEXT ()) t = 0;  /* THIS is the block inside of first block.*/
//	else
//	  if (block == get_global_block ()) t = 0;/* THIS is the UNIT BLOCK. */
//	  else
//	    {
//	      /* block is the superblock of the function, and THIS is the first block.*/
//	      /* So find last argument (look get_next for astDecl) */
//		
//	      func = search_function_by_superblock (block);
//	      if (func)
//		t = ast_get_last_parm (func);
//	    }
//      else t = 0;
//    }
//  return t;
//}

//
// Description. This function returns the FIRST node for LOOP.
// It return "init"; if there is no init, it returns "delta".
// If there is no delta, it returns "cond", if cond is there.
// Otherwise it returns STMT_LOOP_BODY.
//
objTree *   astLoop :: get_first () const
{
  astTree * t = 0;
  astTree * tt = 0;

  tt = STMT_LOOP_VARS ();

  if (tt) 
    {
      t = ((astList*) tt)->TREE_VALUE ();
      if (!t) 
	t = ((astList*) tt)->TREE_PURPOSE ();
    }
  if (!t) t = STMT_LOOP_COND ();
  if (!t) t = STMT_LOOP_BODY ();

  return t;
}

//
// Description. This function returns the FIRST node for CASE.
// It return "index"; if there is no index, it returns "case list".
//
objTree * 
  astCase :: get_first () const
{
  astTree * t = 0;
  astTree * tt = 0;

  t = STMT_CASE_INDEX ();
  if (!t) t = STMT_CASE_LIST ();
  return t;
}
#endif /* _OBJ_AST_TRAVERSE_ */


#ifndef _STAND_ALONE
#ifndef _WIN32
extern char *cmd_current_journal_name, *cmd_execute_journal_name;
#endif
extern "C"  char * trn(char * name)
{
  if(!name) return name;
  char * nn = name;
  while ( *(++name)
)
     if (*name == '/')        nn = name + 1;
    return nn;
}


static void lpr(const astTree* nnd, ostream& ost, int intern=1)
{
    astTree*node = (astTree*) nnd;
    if (!node) {
	ost << "0x0" << endl;
	return;
    }

    astIdentifier * id;
    
    enum tree_code node_code = node->TREE_CODE ();
    
    if (!paraset_tree_code_name)
	paraset_init_tree_codes ();
    
    int scripting = cmd_execute_journal_name != NULL
	|| cmd_current_journal_name != NULL;
  
    if (TREE_START_LINE(node) || !scripting ) {
	ost << TREE_START_LINE(node) << ' ' << TREE_END_LINE(node)
	    << ' ' << trn (TREE_FILENAME(node)) << ' ' 
	    << paraset_tree_code_name[(int)node_code]
	    << " dbg " << node->TREE_DEBUG () << " flg "
	    << (int)TREE_ASET_FLAG (node);

	switch (node_code){
	  case MC_FUNCTION_DECL:
	  case MC_VAR_DECL:
	  case MC_CONST_DECL:
	  case MC_FIELD_DECL:
	  case MC_TYPE_DECL:
	  case MC_PARM_DECL: 
	{
	    astDecl * decl = (astDecl*)node;
	    int decl_line = decl->get_linenum ();
	    ost << " dl " << decl_line;
	    
	    if (node_code == MC_PARM_DECL)
		ost << " pl " << (int)decl->TREE_PREV ();

	    id = decl->DECL_NAME ();
	    if (id){
		const char * name = decl->get_name();
		if(intern && name && name[0] == JOINER)
		    name = "$$";
		ost << " name " << name;
	    } else {
		ost << " id " << "0x0" ;
	    }
	}
	break;
	
	  case MC_IDENTIFIER_NODE:
	    id = (astIdentifier*) node;
	    ost << " pnt " << id->IDENTIFIER_POINTER ();
	    break;
	}
    } else {
	ost << 0 << ' ' << 0
	    << ' ' << trn (TREE_FILENAME(node)) << ' ' 
	    << paraset_tree_code_name[(int)node_code]
	    << " dbg " << 0 << " flg "
	    << 0;

	switch (node_code){
	  case MC_FUNCTION_DECL:
	  case MC_VAR_DECL:
	  case MC_CONST_DECL:
	  case MC_FIELD_DECL:
	  case MC_TYPE_DECL:
	  case MC_PARM_DECL: 
	{
	    astDecl * decl = (astDecl*)node;
	    ost << " dl " << 0;
	    
	    if (node_code == MC_PARM_DECL)
		ost << " pl " << 0;

	    id = decl->DECL_NAME ();
	    if (id){
		const char * name = decl->get_name();
		if(intern && name && name[0] == JOINER)
		    name = "$$";
		ost << " name " << name;
	    } else {
		ost << " id " << id ;
	    }
	}
	break;
	
	  case MC_IDENTIFIER_NODE:
	    id = (astIdentifier*) node;
	    ost << " pnt " << id->IDENTIFIER_POINTER ();
	    break;
	}
    }
    ost << endl;
}

void astTree ::print (ostream& ost) const
{ 
  lpr(this, ost, 0);
  ost << ends;
}

extern "C" void lineprint (astTree *node)
{
  lpr (node, cout, 0);
}

#endif /* STAND_ALONE */
