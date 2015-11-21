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
/*
// synopsis: 
//
// $Log: 
*/

#ifdef _WIN32
#include <string.h>
#endif

#include "astEnums.h"
#include "astTree.h"
#include "ast_cplus_tree.h"

// XXX: move back to ast_cplus_tree.h:
#define THIS_NAME "this"

#define XDECL_CONTEXT(node) (((astDecl *)node)->DECL_CONTEXT())

astTree *ast_lookup_field(astTree *type, astDecl *decl)
{
  astTree *ret = decl;
  if (!decl || !type) return 0;
 
  if (!type) return ret;
  astDecl * field = (astDecl *)type->TREE_TYPE()->TYPE_FIELDS();
  while (field) {
    if (field->DECL_NAME() == NULL_TREE
        && field->TREE_TYPE()->TREE_CODE() == MC_UNION_TYPE)
    {
      astTree *temp = ast_lookup_field(field->TREE_TYPE(), decl);
      if (temp) return temp;
    }
    if (field->DECL_NAME() == decl->DECL_NAME()) 
      return field;
    field = (astDecl *)field->TREE_CHAIN();
  }
  return ret;
}

bool ast_function_decl_is_def (astDecl * decl)
{
  bool is = false;

  if (decl == 0) return false;
  if (decl->TREE_CODE() != MC_FUNCTION_DECL) return false;

  if (decl->DECL_INITIAL())
    if (decl->DECL_INITIAL()->TREE_CODE() == LET_STMT)
      {
          if (DECL_RESULT(decl)
              && (XDECL_CONTEXT (DECL_RESULT(decl)))
              && (XDECL_CONTEXT (DECL_RESULT(decl)))->TREE_CODE() != MC_ERROR_NODE)
	  {
	    /* for compiler generated function */
	    if (decl->artificial_flag)
	 	is = false;
	    else if ( DECL_LANG_SPECIFIC(decl) && (
		DECL_IMPLICIT_INSTANTIATION(decl) ||
		DECL_EXPLICIT_INSTANTIATION(decl)) )
	      is = false;
#if 0 // XXX: these are different types; how could they possibly match?
	    else if (XDECL_CONTEXT (DECL_RESULT (decl)) == decl)
	      is = true;
#endif

#if 0 // XXX: BLOCK_SUPERCONTEXT is gcc construct
	    /* add next else if for case 
	     *
	     *   struct A { A(int x); int val; };
	     *   A::A(int x) : val(x) {}
             * 
	     */   
	    else if ((XDECL_CONTEXT (DECL_RESULT (decl)))->TREE_CODE() == MC_COMPOUND_EXPR
		     && XDECL_CONTEXT(DECL_RESULT (decl))->BLOCK_SUPERCONTEXT() == decl)
	      is = true;
#endif
	  }
	else
	  if (TREE_ASM_WRITTEN (decl))
	    is = true;
	else
	  if (TREE_ASET_FLAG (decl))
	    is = true;
      }
  return is;
}


bool ast_typedecl_is_def (tree decl)
{
  int is = false;

  if (decl == 0) return false;
  if (decl->TREE_CODE() != MC_TYPE_DECL) return false;

  astType *type = decl->TREE_TYPE();
  if (type == 0)    return false;

  switch (type->TREE_CODE())
    {
    case MC_RECORD_TYPE:
    case MC_UNION_TYPE:
    case MC_ENUMERAL_TYPE:
      if (TYPE_LANG_SPECIFIC(type) &&
       (CLASSTYPE_IMPLICIT_INSTANTIATION(type) ||
	  CLASSTYPE_EXPLICIT_INSTANTIATION(type)))
	is = false;
      else if (type->TYPE_NAME() &&
               type->TYPE_NAME()->TREE_CODE() == MC_TYPE_DECL)
	{
	  if ((type->TYPE_NAME() == decl) && type->TYPE_SIZE())
	    is = true;
	}
      else
	if ((TYPE_STUB_DECL (type) == decl) && type->TYPE_SIZE())
	  is = true;
      break;
    }
  return is;
}
#ifdef XXX_ast_typedecl_is_typedef
int ast_typedecl_is_typedef (astDecl *decl)
{
  astType *type;
  int is = 1;

  if (decl == 0) return 0;
  if (decl->TREE_CODE() != MC_TYPE_DECL) return 0;

  type = decl->TREE_TYPE();
  if (type == 0)    return 0;

  switch (type->TREE_CODE())
    {
    case MC_RECORD_TYPE:
    case MC_UNION_TYPE:
    case MC_ENUMERAL_TYPE:
      if (TYPE_STUB_DECL (type) == 0)
	/* c: typr is type of typedef, decl is typedef */
	;
      else
      if (type->TYPE_NAME() &&
	  type->TYPE_NAME()->TREE_CODE() == MC_TYPE_DECL)
	{
	  if (type->TYPE_NAME() == decl)
	    is = 0;
	  else if (!decl->DECL_ARGUMENTS() && decl->DECL_NAME() &&
		   decl->DECL_NAME()==decl->DECL_ASSEMBLER_NAME()) {
            char * name = decl->DECL_NAME()->IDENTIFIER_POINTER();
	    if (name) {
	      if (strchr(name, ':'))
		is = 0;
	    }
	  }
	}
      else
	if (TYPE_STUB_DECL (type) == decl) 
	  is = 0;
      break;
    }
  if (is)
    if (decl->TREE_PREV() == (tree) -1)
      is = 0;
  return is;
}
#endif
static int common_tentative_defs;

int set_common_tentative_defs (int val)
{
  common_tentative_defs = val;
  return val;
}

int ast_var_decl_is_def (astDecl *decl)
{
  int retval = 0;

  if (!DECL_EXTERNAL (decl))
    if (!common_tentative_defs 
	|| !TREE_PUBLIC(decl) 
	|| decl->DECL_INITIAL())
      retval = 1;

  return retval;
}

int ast_decl_is_def (astDecl *decl)
{
  if (decl == 0) return 0;
  switch (decl->TREE_CODE())
    {
    case MC_TYPE_DECL: return ast_typedecl_is_def (decl);
    case MC_FUNCTION_DECL: return ast_function_decl_is_def (decl);
    case MC_VAR_DECL: return ast_var_decl_is_def (decl);
    case MC_CONST_DECL:
    case MC_LABEL_DECL:
    case MC_FIELD_DECL:
    case MC_PARM_DECL:
      return 1;
    default:
      return 0;
    }
}
#ifdef XXX_ast_is_var_def
int ast_is_var_def (tree decl)
{
  astTree *type;
  int is = 0;
  tree t;
  astBlock *block;

  if (decl == 0) return 0;
  if (decl->TREE_CODE() != MC_VAR_DECL) return 0;

  type = decl->TREE_TYPE();
  if (type == 0)    return 0;

  block = XDECL_CONTEXT (decl);

  if (block)
    for (t = block->STMT_VARS(); t; t = t->TREE_CHAIN())
      if (decl == t)
	{
	  is = 1;
	  break;
	}
  return is;
}
#endif
/* do not take care of special case */
int ast_decl_is_global_1(tree decl)
{
  tree context;
  if (!decl) /* || !(TREE_CODE(decl) == MC_TYPE_DECL ||
		TREE_CODE(decl) == CONST_DECL)) */
      return 0;
  context = XDECL_CONTEXT(decl);
#if 0
  if (context && TREE_CODE(decl) == FUNCTION_DECL &&
      TREE_CODE(context) == LET_STMT)
  return 1;
#endif
  if (context == 0) return 1;
#if 0  // XXX: I can't figure out what TYPE_MAIN_DECL macro should be....
  else if (context->TREE_CODE() == MC_RECORD_TYPE || 
	   context->TREE_CODE() == MC_ENUMERAL_TYPE || 
	   context->TREE_CODE() == MC_UNION_TYPE) {
    tree par_decl = TYPE_MAIN_DECL(context);
    return ast_decl_is_global_1(par_decl);
  }
#endif
  else
    return 0;
}

int ast_is_decl_global(astDecl *decl)
{
  int rc;

  if (decl == 0) return 0;

  switch (decl->TREE_CODE())
    {
    case MC_VAR_DECL:
      if (DECL_EXTERNAL (decl))	rc = 1;
      else
	if (TREE_STATIC (decl))
	  if (TREE_PUBLIC (decl)) rc = 1;  /* global declaration */
	  else
	    if (XDECL_CONTEXT (decl) == 0) rc = 1;  /* static global */
	    else rc = 0;
	else rc = 0;
      break;
    case MC_TYPE_DECL:
    case MC_CONST_DECL:
      if (XDECL_CONTEXT (decl) == 0) rc = 1;
      else if (decl->DECL_ARGUMENTS() &&
	  (decl->DECL_ARGUMENTS()->TREE_CODE() == MC_IDENTIFIER_NODE) &&
               (decl->ID_NODE()->IDENTIFIER_TEMPLATE()))
	rc = 1;
      else 
	rc = ast_decl_is_global_1(decl);
      if (decl->TREE_TYPE() &&
	  decl->TREE_TYPE()->TREE_CODE() == TEMPLATE_TYPE_PARM)
	rc = 0;

      break;
    case MC_FUNCTION_DECL:
    case MC_FIELD_DECL:
      rc = ast_decl_is_global_1(decl);
      break;
    case TEMPLATE_DECL:
      rc = 1;
      break;

      default : rc = 0;
    }
  return rc;
}
#ifdef XXX_ast_is_legal_id
int
ast_is_legal_id (char *name)
{
  int i;
  int rc = 0;
  for (i = 0; i < strlen (name); ++i)
    {
      switch (* (char *) (name + i))
	{
	case 'A':  case 'B':  case 'C':  case 'D':  case 'E':
	case 'F':  case 'G':  case 'H':  case 'I':  case 'J':
	case 'K':	 case 'L':  case 'M':  case 'N':  case 'O':
	case 'P':  case 'Q':  case 'R':  case 'S':  case 'T':
	case 'U':  case 'V':  case 'W':  case 'X':  case 'Y':
	case 'Z':
	case 'a':  case 'b':  case 'c':  case 'd':  case 'e':
	case 'f':  case 'g':  case 'h':  case 'i':  case 'j':
	case 'k':  case 'l':  case 'm':  case 'n':  case 'o':
	case 'p':  case 'q':  case 'r':  case 's':  case 't':
	case 'u':  case 'v':  case 'w':  case 'x':  case 'y':
	case 'z':
	case '_':
	  ;
	  break;
	case '0': case '1': case '2': case '3': case '4':
	case '5': case '6': case '7': case '8': case '9': 
	  if (i == 0) rc = 1; 
	  break;
	default: rc = 1;
	  break;
	}
      if (rc)
	break;
    }
  return rc;
}
#endif
/* this function is for C++. It check the name of PARM_DECL,
   if it is THIS (name = this), */ 	
int
ast_dd_is_parm_this (astDecl *p)
{
  char * s; 
  astIdentifier *id = p->DECL_NAME(); 
  if (id) {
    s = id->IDENTIFIER_POINTER();
    if (!strcmp (s, THIS_NAME))
      return 1;
  }
  return 0;
}

/* this function is for C++. It check the name of FIELD_DECL,
   if it is  vptr (field for pointer to virtual table in instance)
   name (name begins as _vptr$), */ 	
int
is_vptr_name (tree p)
{
  astIdentifier *id = 0;
  char * s = 0;
  if (p->TREE_CODE() == MC_FIELD_DECL) {
      if (( id = ((astDecl *)p)->DECL_NAME())) {
	  s = id->IDENTIFIER_POINTER();
	  if (strncmp (s, VFIELD_NAME, 6) == 0)     /* "_vptr$" */
              return 1;
      }
  }
  return 0;
}

/* For S-mode: skip bese init member 
   if it is virtual table pointer initializer */
tree ast_filtr_base_init_member (astList *lst)
{
  tree co;
  astList *mem;

  if (lst && lst->TREE_CODE() == MC_TREE_LIST)
      for (mem = lst; mem; mem=mem->get_next_ast())
      {
#ifdef XXX_skip_base_init_member
	if (mem->TREE_CODE() == MC_TREE_LIST  && mem->TREE_VALUE() &&
	    (mem->TREE_VALUE()->TREE_CODE() == MC_MODIFY_EXPR) &&
	    ((co = mem->TREE_VALUE()->TREE_OPERAND(0))->TREE_CODE()==MC_COMPONENT_REF) &&
            (TREE_OPERAND(co, 0)->TREE_CODE() == MC_INDIRECT_REF) &&
            (TREE_OPERAND(TREE_OPERAND (co,0),0)->TREE_CODE()) == MC_PARM_DECL) &&
	    ast_dd_is_parm_this (TREE_OPERAND(TREE_OPERAND (co,0),0)) &&
	    is_vptr_name (TREE_OPERAND(co,1)))
	  ;
	else break;
#else
        break;
#endif

      }
  else mem = 0;

  return mem;
}
#ifdef XXX_ast_get_first_base_init_member 
tree ast_get_first_base_init_member (astDecl *fndecl)
{
  astDecl *t;
  astExp *base_init;
  tree first_mem;

  if ((t = fndecl->DECL_ARGUMENTS()) &&
      (ast_dd_is_parm_this (t)) &&
      ((base_init = (astExp *)DECL_CONSTRUCTOR (t))))
    {
      first_mem = base_init->TREE_OPERAND(1);
      first_mem = ast_filtr_base_init_member ((astList *)first_mem);
    }
  else first_mem = 0;
  return first_mem;
#endif //}
#ifdef XXX_ast_get_next_base_init_member
tree ast_get_next_base_init_member (astList *lst)
{
  astList *mem;

  if (lst && lst->TREE_CODE() == MC_TREE_LIST)
    mem = lst->get_next_ast();
  else mem = 0;

  return ast_filtr_base_init_member (mem);
#endif}
#ifdef XXX_ast_breakpoint_valid
int ast_breakpoint_valid (tree node)
{
  int ret;
  astBlock *context;

  if (!node) return 0;
  switch (node->TREE_CODE())
    {
    case MC_VAR_DECL:
      if (context = XDECL_CONTEXT (node))
	if (context->TREE_CODE() == MC_RECORD_TYPE)	  ret = 1;
	else
	  if (context->STMT_VARS() == context->STMT_BODY())  ret = 1;
	  else ret = 0;
      else ret = 0;
      break;
    case MC_LABEL_STMT:
    case MC_GOTO_STMT:
    case MC_RETURN_STMT:
    case MC_EXPR_STMT:
    case LET_STMT:
    case MC_IF_STMT:
#if 0   // is this used?
    case EXIT_STMT:
#endif 
    case MC_SWITCH_STMT:
    case MC_FOR_STMT:
    case MC_DO_STMT:
    case MC_WHILE_STMT:
    case MC_COMPOUND_STMT:
    case MC_ASM_STMT:
      ret = 1;
      break;
    default:
      ret = 0;
      break;
    }
  return ret;
#endif //}
#ifdef XXX_ast_get_block_for_stmt
tree ast_get_block_for_stmt (tree stmt)
{
  tree block;
  if (!stmt) return 0;

  switch (stmt->TREE_CODE())
    {
    case MC_CONST_DECL:
    case MC_VAR_DECL:
      block = XDECL_CONTEXT(stmt);
      break;
    case LET_STMT:
      block =  stmt;
      break;
    case MC_LABEL_STMT:
    case MC_GOTO_STMT:
    case MC_RETURN_STMT:
    case MC_EXPR_STMT:
#if 0 // XXX?
    case MC_EXIT_STMT:
#endif
    case MC_SWITCH_STMT:
    case MC_COMPOUND_STMT:
    case MC_ASM_STMT:
    case MC_IF_STMT:
    case MC_FOR_STMT:
    case MC_WHILE_STMT:
    case MC_DO_STMT:
        block = ((astStmts *)stmt)->STMT_CONTEXT();
      break;
    default:
      block = 0;
      break;
    }
  return block;
#endif //}
#ifdef XXX_get_real_typedecl
tree get_real_typedecl (tree fict)
{
  tree t = fict;
  astType *type = 0;

  if (fict->TREE_CODE() == MC_TYPE_DECL)
    {
      type = fict->TREE_TYPE();
      if (type)
	if (type->TYPE_NAME() &&
	    type->TYPE_NAME()->TREE_CODE() == MC_TYPE_DECL)
	  t = type->TYPE_NAME();
	else
	  if (TYPE_STUB_DECL (type) &&
	      TYPE_STUB_DECL (type)->TREE_CODE() == MC_TYPE_DECL)
	    t = TYPE_STUB_DECL (type);
    }
  return t;
}
#endif
#ifdef XXX_friend_decls_fixed
/* Given a TYPE with TREE_CODE == MC_RECORD_TYPE and a FRND, which is 0 or
   previous friend - FUNCTION_DECL, not member of other class, - returns   
   next friend function if any, or 0 */
tree get_friend_function (astType *type, tree frnd)
{
  int next = 0;
  astDecl *lst = 0;
  astDecl *typedecl = 0;
  tree fr = 0;

  if (type == 0) return 0;
  if (type->TREE_CODE() != MC_RECORD_TYPE) return 0;
  if (frnd && frnd->TREE_CODE() != MC_FUNCTION_DECL) return 0;
  
  typedecl = (type->TYPE_NAME() && type->TYPE_NAME()->TREE_CODE() == MC_TYPE_DECL ?
	      type->TYPE_NAME() : TYPE_STUB_DECL (type));
  if (typedecl == 0) return 0;

  for (lst = typedecl->DECL_INITIAL(); lst; lst = (astDecl *)lst->TREE_CHAIN())
    {
        fr = (lst->TREE_VALUE() &&
              lst->TREE_VALUE()->TREE_CODE() == MC_TREE_LIST ?
	    TREE_VALUE (lst->TREE_VALUE()) : 0);

      if (frnd == 0) break;
      if (next) break;
      if (frnd == fr) next = 1;
      fr = 0;
    }
  return fr;
}

tree get_friend_class (astType *type, tree frnd)
{
  int next = 0;
  astType *lst = 0;
  tree fr = 0;

  if (type == 0) return 0;
  if (type->TREE_CODE() != MC_RECORD_TYPE) return 0;
  if (frnd && frnd->TREE_CODE() != MC_TYPE_DECL) return 0;
  if (TYPE_LANG_SPECIFIC (type) == 0) return 0;

  
  for (lst = CLASSTYPE_FRIEND_CLASSES(type); lst; lst = lst->TREE_CHAIN())
    {
      fr = (lst->TREE_VALUE() && lst->TREE_VALUE()->TREE_CODE() == MC_RECORD_TYPE ?
	    lst->TREE_VALUE()->TYPE_NAME() : 0);

      if (frnd == 0) break;
      if (next) break;
      if (frnd == fr) next = 1;
      fr = 0;
    }
  return fr;
}

int
is_member_function (tree typedecl, tree fndecl)
{
  astType *type;
  if (typedecl == 0 || typedecl->TREE_CODE() != MC_TYPE_DECL) return 0;
  if ((type = typedecl->TREE_TYPE()) == 0 || 
      (type->TREE_CODE() != MC_RECORD_TYPE &&
       type->TREE_CODE() != MC_UNION_TYPE)
      )
    return 0;
  if (fndecl == 0 || fndecl->TREE_CODE() != MC_FUNCTION_DECL) return 0;

  {
    int i;
    tree decl;
    tree method_vec = CLASSTYPE_METHOD_VEC (type);
    int len = method_vec ? method_vec->TREE_VEC_LENGTH() : 0;
    
    if (method_vec == 0) return 0;
    
    if (method_vec->TREE_VEC_ELT(0) && 
	method_vec->TREE_VEC_ELT(0)->TREE_CODE() == MC_FUNCTION_DECL)
        if (fndecl->DECL_NAME() == TREE_VEC_ELT (method_vec, 0)->DECL_NAME())
	{
          for (decl = method_vec->TREE_VEC_ELT(0); decl; decl=TREE_CHAIN(decl))
	    if (decl->DECL_NAME() != fndecl->DECL_NAME()) return 0;
	    else
	      if (DECL_ASSEMBLER_NAME (decl) == DECL_ASSEMBLER_NAME (fndecl))
		return 1;
	}

    for (i = 1; i < len; i++)
      {
	for (decl = method_vec->TREE_VEC_ELT(i); decl; decl=TREE_CHAIN(decl))
	  if (decl->DECL_NAME() != fndecl->DECL_NAME()) break;
	  else
	    if (DECL_ASSEMBLER_NAME (decl) == DECL_ASSEMBLER_NAME (fndecl))
	      return 1;
      }
  }
  return 0;
}

is_member_aggr (tree typedecl, tree aggr)
{
  tree type, type2, real_aggr;
  tree field;

  if (typedecl == 0 || typedecl->TREE_CODE() != MC_TYPE_DECL) return 0;
  if ((type = typedecl->TREE_TYPE()) == 0 || 
      (type->TREE_CODE() != MC_RECORD_TYPE &&
       type->TREE_CODE() != MC_UNION_TYPE)
      )
    return 0;

  if (aggr == 0 || aggr->TREE_CODE() != MC_TYPE_DECL) return 0;
  if ((type2 = aggr->TREE_TYPE()) == 0 || 
      (type2->TREE_CODE() != MC_RECORD_TYPE &&
       type2->TREE_CODE() != MC_UNION_TYPE)
      )
    return 0;

  real_aggr = (type2->TYPE_NAME() && type2->TYPE_NAME()->TREE_CODE()==MC_TYPE_DECL)?
    type2->TYPE_NAME() : TYPE_STUB_DECL (type2);

  if (real_aggr == 0 || real_aggr->TREE_CODE() != MC_TYPE_DECL)
    return 0;

  for (field = TYPE_FIELDS (type); field; field = TREE_CHAIN (field))
    if (field == real_aggr)
      return 1;

  return 0;
}

tree ast_get_decl_of_record (tree type)
{
  tree decl = 0;

  if (type == 0) return 0;

  if (type->TREE_CODE() != MC_RECORD_TYPE ||
      type->TREE_CODE() != MC_UNION_TYPE)
    return 0;

  if (type->TYPE_NAME() &&
      type->TYPE_NAME()->TREE_CODE() == MC_TYPE_DECL)
    decl = type->TYPE_NAME();
  else
    if (TYPE_MAIN_VARIANT (type))
      decl = TYPE_STUB_DECL (TYPE_MAIN_VARIANT (type));
    else	decl = TYPE_STUB_DECL (type);

  return decl;
}
#endif
