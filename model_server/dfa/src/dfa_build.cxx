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
#include <msg.h>
#ifndef ISO_CPP_HEADERS
#include "iostream.h"
#include "fstream.h"
#include "stdio.h"
#else /* ISO_CPP_HEADERS */
#include <iostream>
using namespace std;
#include <fstream>
#include <cstdio>
#endif /* ISO_CPP_HEADERS */
#include "machdep.h"
#include "general.h"
#include "genError.h"
#include "astTree.h"
#include "ast_cplus_tree.h"
#include "smt.h"
#include "ddict.h"
#include "db.h"
#include "dfa_graph.h"
#include "projHeader.h"
#include "xrefSymbol.h"
#include "customize.h"

#include "symbolStruct.h"
#include "astnodeStruct.h"
#include "locationStruct.h"
#include "relationStruct.h"
#include "symbolStruct.h"
#include "astnodeList.h"
#include "proj.h"
#ifndef ISO_CPP_HEADERS
#include <ctype.h>
#else /* ISO_CPP_HEADERS */
#include <cctype>
#endif /* ISO_CPP_HEADERS */

extern bool is_forgiving();
int dfa_find(smtHeader *,astRoot *, astNode *);
int ast_is_decl_global(astDecl *);
ddElement *lookup_ast_to_dd(astDecl *decl, ddRoot *dr);
//ddElement *dd_find_string(smtHeader *, char *, int);
ddRoot *dd_sh_get_dr(smtHeader *);
void get_pset_fn(const char *, genString &);
extern "C" astTree* get_first_child_of_block(astBlock *);
int ast_decl_is_def(astDecl *);
void get_qualify_type_name(astType *type, genString &name_str);
bool dfa_smt_in_range(smtHeader *, astNode *, int, int);
static int dfa_print_after_restore;
static ddElement *dfa_get_dd_from_int_cst(astTree *cst);
static unsigned int ast_level(astTree *ast);
const char* astNode_code_name(int code);
static int dfa_debug;
//int is_cons_or_destructor(astFunction_decl *);
int els_find_location(astnodeStruct *, int &, int &);

#define STRIP_NOPS(EXP) \
  while (EXP->TREE_CODE () == NOP_EXPR				\
	  || EXP->TREE_CODE () == MC_CONVERT_EXPR			\
	  || EXP->TREE_CODE () == NON_LVALUE_EXPR)		\
    EXP = ((astExp1*)EXP)->TREE_OPERAND (0)

static int
init_dfa_debug()
{
  Initialize(init_dfa_debug());
  int retval = -1;

  if (customize::getIntPref("DIS_internal.Dfa_Debug") > 0)
    retval = 1;
  
  return retval;
}

#define FIX_LOCATION(node_id,last_node_id)\
     do {\
	   if (last_node_id) {\
	       astNode *curnode = (astNode *) ((*buf)[node_id]);\
	       astNode *last_node = (astNode *) ((*buf)[last_node_id]);\
	       if (last_node->get_length() + last_node->get_start() > curnode->get_start() + curnode->get_length())\
		   curnode->set_length(last_node->get_length() + last_node->get_start() - curnode->get_start());\
	   }\
     } while (0)

#define SET_AST_LOCATION(node_id,ast)\
     do {\
	   int st, len;\
	   if (dfa_find_location(ast,st,len)) {\
	       astNode *curnode = (astNode *) ((*buf)[node_id]);\
	       curnode->set_start(st);\
	       curnode->set_length(len);\
	   }\
     } while (0)
				      
#define SET_SMT_LOCATION(node_id,smt)\
     do {\
	   int st = smt->tbeg, len = smt->tlth;\
	   astNode *curnode = (astNode *) ((*buf)[node_id]);\
	   curnode->set_start(st);\
	   curnode->set_length(len);\
     } while (0)				      

#define SET_ELS_LOCATION(node_id,an)\
     do {\
	   int st, len;\
	   if (els_find_location(an,st,len)) {\
	       astNode *curnode = (astNode *) ((*buf)[node_id]);\
	       curnode->set_start(st);\
	       curnode->set_length(len);\
	   }\
     } while(0)
				      

#define SET_LOCATION(node_id,st,len)\
     do {\
	   astNode *curnode = (astNode *) ((*buf)[node_id]);\
	   curnode->set_start(st);\
	   curnode->set_length(len);\
     } while(0)
				      

#define SET_INT_VALUE(node_id,val) ((dfa_integer_cst *) ((*buf)[node_id]))->set_value(val)
#define SET_REAL_VALUE(node_id,val) ((dfa_real_cst *) ((*buf)[node_id]))->set_value(val)
#define SET_START(node_id,st) ((astNode *) ((*buf)[node_id]))->set_start(st)
#define SET_LENGTH(node_id,len) ((astNode *) ((*buf)[node_id]))->set_length(len)
#define SET_DD(node_id,dd) ((dfa_ref_dd *) ((*buf)[node_id]))->set_dd(dd)
#define SET_TYPE(node_id,dd) ((astNode *) ((*buf)[node_id]))->set_type(dd)

#define SET_DECL(node_id,decl_id) do \
        { \
          dfa_decl *temp_decl=NULL; \
	  if (decl_id>0) \
	    temp_decl = (dfa_decl *) ((*buf)[decl_id]); \
          ((dfa_ref_dfa *) ((*buf)[node_id]))->set_decl(temp_decl); \
	} while (0)

#define SET_ID(node_id,val) ((dfa_decl *) ((*buf)[node_id]))->set_id(val)
#define SET_IS_DEF(node_id) ((dfa_decl *) ((*buf)[node_id]))->set_is_def()

static db_buffer *buf; 
static smtHeader *header;
static ddRoot *dr;

static astNode_id dfa_make_node(tree_code, astNode_id, astNode_id, 
				astTree * = NULL);
static astNode_id dfa_build_ast(astTree *ast, astNode_id parent, 
				astNode_id prev);
static astNode_id dfa_build_smt(smtTree *smt, astNode_id parent,
				astNode_id prev);
static ddElement *dfa_find_dd(astDecl *);
static astNode_id dfa_find_dfa(astDecl *);
static int dfa_find_location(astTree *,int &, int &);
void dfa_process_tokens(smtHeader *h, astNode *par);

static int
dfa_dump_tree(smtHeader *h,
	      astNode *dfa_root)
{
  Initialize(dfa_dump_tree(smtHeader *, astNode *));

  int retval = 0;
  static int do_dump = (int) OSapi_getenv("DIS_DFA_DUMP_TREE");

  if (do_dump) {
    const char *phys_name = h->get_phys_name();
    genString gs;
    get_pset_fn(phys_name,gs);

    if (!gs.length()) {
      gs = phys_name;
      gs += ".pset";
    }

    gs += ".dfa.dump";
    int err = projHeader::make_path((char *) gs);

    if (!err) {
      ofstream ofs((char *) gs, ios::out);

      if (ofs != NULL) { 
	retval = dfa_print_tree(h,dfa_root,ofs);
	ofs.close();
      }
    }
  }

  return retval;
}

int
dfa_prepare_instances(smtHeader *h)
{
  Initialize(dfa_prepare_instances(smtHeader *));

  int retval = 0;

  astRoot *ast_root = checked_cast(astRoot,get_relation(astRoot_of_smtHeader,h));

  if (ast_root) {
    h->last_token = NULL;
    astNode *root_node = ast_root->get_astNode();
    dfa_find(h,ast_root,root_node);
    retval = 1;
  }

  return retval;
}

astRoot *
dfa_build(smtHeader *h)
{
  Initialize(dfa_build(smtHeader *));

  smtTree *root = checked_cast(smtTree, h->get_root());
  int do_it = h->ast_exist;

  if (!do_it && is_forgiving()) {
    for (smtTree *smt = root->get_first(); smt; smt = smt->get_next()) {
      if (smt_get_ast(smt)) {
	do_it = 1;
	break;
      }
    }
  }

  if (!do_it)
    return NULL;

  header = h;
  h->last_token = NULL;
  dr = dd_sh_get_dr(h);

  buf = new db_buffer;
  astNode_id dfa_null = dfa_make_node(MC_ERROR_NODE, 0, 0);
  astNode_id dfa_root = dfa_build_smt(root, 0, 0);
  buf->compress();

  astRoot *ast_root = astRoot::get_create_astRoot(h, buf);

  buf = NULL;
  header = NULL;
  dr = NULL;

  astNode *root_node = ast_root->get_astNode();
  dfa_process_tokens(h, root_node);
  dfa_dump_tree(h, root_node);

  return ast_root;
}

static astNode_id
dfa_build_expr(astTree *exp,
	       astNode_id parent,
	       astNode_id prev)
{
  Initialize(dfa_build_expr(astTree *, astNode_id, astNode_id));

  tree_code code = exp->TREE_CODE();

  astNode_id node = NULL;
  astNode_id last = NULL;
  astNode_id first_op = NULL;

  switch (code) {
  case INIT_EXPR:
  case MC_MODIFY_EXPR:
  case MC_LT_EXPR: 
  case MC_LE_EXPR: 
  case MC_GT_EXPR: 
  case MC_GE_EXPR: 
  case MC_EQ_EXPR: 
  case MC_NE_EXPR:

  case MC_PREDECREMENT_EXPR: 
  case MC_PREINCREMENT_EXPR: 
  case MC_POSTDECREMENT_EXPR:
  case MC_POSTINCREMENT_EXPR:

#if 0 // XXX: not used in C/C++; what about Java?
  case EXPON_EXPR:
#endif
  case MC_PLUS_EXPR:
  case MC_MINUS_EXPR:
  case MC_MULT_EXPR:
#if 0 // XXX: not used in C/C++; what about Java?
  case TRUNC_DIV_EXPR:
  case CEIL_DIV_EXPR: 
  case FLOOR_DIV_EXPR:
  case ROUND_DIV_EXPR:
  case TRUNC_MOD_EXPR: 
  case CEIL_MOD_EXPR:  
  case FLOOR_MOD_EXPR: 
  case ROUND_MOD_EXPR:
#endif
  case MC_RDIV_EXPR:  
#if 0
  case EXACT_DIV_EXPR:
#endif

  case MC_LSHIFT_EXPR:
  case MC_RSHIFT_EXPR:
  case MC_LROTATE_EXPR:
  case MC_RROTATE_EXPR:

  case MC_BIT_IOR_EXPR:
  case MC_BIT_XOR_EXPR:
  case MC_BIT_AND_EXPR:
  case MC_BIT_ANDTC_EXPR:

  case MC_TRUTH_ANDIF_EXPR:
  case MC_TRUTH_ORIF_EXPR:
  case MC_TRUTH_AND_EXPR:
  case MC_TRUTH_OR_EXPR:
  case MC_TRUTH_XOR_EXPR:

  case MC_MIN_EXPR:
  case MC_MAX_EXPR:
  case MC_ARRAY_REF:

  case MC_COMPOUND_EXPR:
    node = dfa_make_node(code, parent, prev, exp);
    SET_AST_LOCATION(node,exp);	
    first_op = last = dfa_build_expr(((astExp1 *) exp)->TREE_OPERAND(0),
				     node, last);
    last = dfa_build_expr(((astExp1 *) exp)->TREE_OPERAND(1), node, last);
    break;
	
  case MC_COND_EXPR:
    node = dfa_make_node(code, parent, prev,exp);
    SET_AST_LOCATION(node,exp);	
    last = dfa_build_expr(((astExp1 *) exp)->TREE_OPERAND(0), node, last);
    last = dfa_build_expr(((astExp1 *) exp)->TREE_OPERAND(1), node, last);
    last = dfa_build_expr(((astExp1 *) exp)->TREE_OPERAND(2), node, last);
    break;

  case MC_CALL_EXPR:
  case NEW_EXPR:
    {
      node = dfa_make_node(code, parent, prev,exp);
      SET_AST_LOCATION(node,exp);	
      smtTree *tkn = header->tn_na(-exp->TREE_DEBUG());
      ddElement *dd = NULL;

      if (tkn) {
	last = dfa_make_node((tree_code) DFA_REF_DD, node, last);
	dd = smt_get_dd(tkn);

	if (!dd) {
	  smtTree *smtdcl = checked_cast(smtTree,get_relation(declaration_of_reference,tkn));

	  if (smtdcl)
	    dd = smt_get_dd(smtdcl);
	}

	SET_DD(last,dd);
	SET_START(last,tkn->tbeg);
	SET_LENGTH(last,tkn->tlth);
      } else {
	astTree *exp1 = ((astExp1 *) exp)->TREE_OPERAND(0);

	if (exp1 && exp1->TREE_CODE() == MC_ADDR_EXPR)
	  exp1 = ((astExp1*) exp1)->TREE_OPERAND(0);
	last = dfa_build_expr(exp1, node, last);
      }

      int doing_virtual = dd ? dd->is_virtual() : 0;
      int first_arg = 1;

      for (astList *lst = (astList *) ((astExp1 *) exp)->TREE_OPERAND(1);
	   lst;
	   lst = (astList *) lst->TREE_CHAIN(), first_arg = 0) {
	astTree *arg = lst->TREE_VALUE();

	if (arg) {
	  if (first_arg && arg->TREE_CODE() == MC_PLUS_EXPR) {
	    if (!doing_virtual) {
	      astTree* comp = ((astExp2 *) arg)->TREE_OPERAND(1);
	      if (comp) {
		STRIP_NOPS(comp);
		if( comp->TREE_CODE() == MC_COMPONENT_REF ||
		    comp->TREE_CODE() == JAVA_COMPONENT_REF) {
		  astDecl* delta = (astDecl*)((astExp2*)comp)->TREE_OPERAND(1);
		  if (delta && delta->TREE_CODE() == MC_FIELD_DECL) {
		    astIdentifier* id = delta->DECL_NAME();
		    if(id && id->IDENTIFIER_POINTER() &&
		       !strcmp(id->IDENTIFIER_POINTER(),"__delta"))
		      doing_virtual = 1;
		  }
		}
	      }
	    }

	    if (doing_virtual)
	      arg = ((astExp1 *) arg)->TREE_OPERAND(0);
	  }

	  astNode_id child = dfa_build_expr(arg,node,last);

	  if (child) {
	    last = child;

	    if (TREE_START_LINE(lst)) {
	      smtTree *tkn_st = header->tn_na(TREE_START_LINE(lst));
	      smtTree *tkn_en = header->tn_na(TREE_END_LINE(lst));

	      if (tkn_st && tkn_en) {
		int start = tkn_st->tbeg;
		int end = tkn_en->tbeg + tkn_en->tlth;
		astNode *curnode = (astNode *) ((*buf)[last]);
		int lastcode = (int) curnode->get_code();

		if (lastcode == DFA_REF_DD || lastcode == DFA_REF_DFA)
		  dfa_smt_in_range(header,curnode,start,end);
	      }
	    }
	  }
	}
      }
    }

    break;
	
  case MC_COMPONENT_REF: 
  case JAVA_COMPONENT_REF: 
    {
      node = dfa_make_node(code, parent, prev,exp);
      SET_AST_LOCATION(node,exp);	
      last = dfa_build_expr(((astExp1 *) exp)->TREE_OPERAND(0), node, last);
      astNode_id first = last;
      last = dfa_build_expr(((astExp1 *) exp)->TREE_OPERAND(1), node, last);
	  
      int tkn = - exp->TREE_DEBUG();
      smtTree *token = header->tn_na(tkn);

      if (token) {
	SET_START(last,token->tbeg);
	SET_LENGTH(last,token->tlth);
	astNode *first_node = (astNode *) (*buf)[first];

	if (first_node->get_code() == MC_INDIRECT_REF) {
	  astNode *curnode = (astNode *) (*buf)[node];

	  if (first_node->get_start() >= curnode->get_start() &&
	      first_node->get_length() > token->tlth) 
	    first_node->set_length(first_node->get_length() - token->tlth);
	}
      }
    }

    break;

  case MC_FIELD_DECL:	
  case MC_CONST_DECL:
  case MC_VAR_DECL:
  case MC_PARM_DECL:
  case MC_FUNCTION_DECL:
    {
      astDecl *decl = (astDecl *) exp;

      if (ast_is_decl_global(decl)) { // global decl 
	node = dfa_make_node((tree_code) DFA_REF_DD, parent, prev,exp);
	ddElement *dd = dfa_find_dd(decl);
	SET_DD(node,dd);
      } else { //local variable
	node = dfa_make_node((tree_code) DFA_REF_DFA, parent,prev,exp);
	astNode_id df_decl = dfa_find_dfa(decl);

	if (df_decl)
	  SET_DECL(node,df_decl);
      }
    }

    break;

  case CLEANUP_POINT_EXPR:
  case NOP_EXPR:
  case NON_LVALUE_EXPR:
  case MC_CONVERT_EXPR:
#if 0 // XXX
  case FIX_TRUNC_EXPR:
  case FIX_CEIL_EXPR:
  case FIX_FLOOR_EXPR:
  case FIX_ROUND_EXPR:
#endif
  case FLOAT_EXPR:
  case MC_SAVE_EXPR:
  case MC_UNSAVE_EXPR:
    node = dfa_build_expr(((astExp1 *) exp)->TREE_OPERAND(0), parent, prev);
    break;

  case MC_ABS_EXPR:
  case MC_NEGATE_EXPR:
  case MC_TRUTH_NOT_EXPR:
  case MC_BIT_NOT_EXPR:
  case MC_INDIRECT_REF:
    node = dfa_make_node(code, parent, prev,exp);
    SET_AST_LOCATION(node,exp);	
    last = dfa_build_expr(((astExp1 *) exp)->TREE_OPERAND(0), node, last);
    break;
	
  case MC_ADDR_EXPR:
    {
      astTree *op0 = ((astExp1 *) exp)->TREE_OPERAND(0);

      if (op0->TREE_CODE() == MC_STRING_CST)
	node = dfa_build_expr(op0, parent, prev);
      else {
	node = dfa_make_node(code, parent, prev,exp);
	SET_AST_LOCATION(node,exp);	
	last = dfa_build_expr(op0, node, last);
      }
    }

    break;
	
#if 0
  case TARGET_EXPR:
    {
      astTree *op1 = ((astExp2 *) exp)->TREE_OPERAND(1);

      if (!op1)
	op1 = exp->TREE_PREV();

      if(op1)
	node = dfa_build_expr(op1,parent,prev);
    }
#endif
    break;

  case MC_STRING_CST:
    node = dfa_make_node((tree_code) DFA_REF_DD, parent, prev,exp);
    SET_DD(node, dfa_find_dd((astDecl *) exp));
    SET_AST_LOCATION(node,exp);	
    break;

  case MC_INTEGER_CST: 
    {
      ddElement *dd = dfa_get_dd_from_int_cst(exp);

      if (dd) {
	node = dfa_make_node((tree_code) DFA_REF_DD, parent, prev);
	SET_DD(node,dd);
      } else {
	node = dfa_make_node(code, parent, prev);
	SET_AST_LOCATION(node,exp);
	SET_INT_VALUE(node,TREE_INT_CST_LOW((astInt_cst*)exp));
      }
    }

    break;

  case MC_REAL_CST:
    node = dfa_make_node(code, parent, prev);
    SET_AST_LOCATION(node,exp);	
    SET_REAL_VALUE(node, ((astReal_cst *) exp)->TREE_REAL_CST());
    break;
      
  case MC_CONSTRUCTOR:
    {
      for (astList *list = (astList *) ((astExp2 *) exp)->TREE_OPERAND(1);
	   list;
	   list = (astList *) list->TREE_CHAIN()) {
	astTree *comp = list->TREE_VALUE();

	if (comp) {
	  if (!node) {
	    node = dfa_make_node(code, parent, prev);
	    SET_AST_LOCATION(node,exp);	
	  }

	  astNode_id ch = dfa_build_expr(comp,node,last);

	  if(ch)
	    last = ch;
	}
      }
    }

    break;

#if 0 // XXX
  case VEC_INIT_CONSTRUCTOR:
  case WITH_CLEANUP_EXPR:
#endif
  case MC_RESULT_DECL:
#if 0 // XXX
  case RTL_EXPR:
  case BIND_EXPR:
  case MC_BIT_FIELD_REF:
  case CP_OFFSET_REF:
#endif
    break;

  default:
    if (dfa_debug == 0)
      dfa_debug = init_dfa_debug();

    if (dfa_debug > 0)
      msg("dfa_build_expr: case $1\n") << astNode_code_name(code) << eom; 

    break;
  }

  return node;
}

static astNode_id
dfa_build_ast(astTree *ast,
	      astNode_id parent,
	      astNode_id prev)
{
  Initialize(dfa_build_ast(astTree *, astNode_id, astNode_id));

#ifdef XXX_newAST_buildDFA
  tree_code code = ast->TREE_CODE();
  astNode_id node = NULL;
  astNode_id last = NULL;
  smtTree *smt = NULL;

  switch (code) {

  case LET_STMT:
    smt = ast_get_smt(ast);

    if (smt && smt->type != SMT_macrocall)
      node = dfa_build_smt(smt,parent,prev);
    else {
      astTree *ast1 = get_first_child_of_block((astBlock *) ast);

      if (ast1)
	node = dfa_build_ast(ast1, parent, prev);
    }
	    
    break;

  case MC_EXPR_STMT:
  case MC_RETURN_STMT:
    {
      node = dfa_make_node(code, parent, prev,ast);
      SET_AST_LOCATION(node,ast);
      astTree *body = ((astStmt *) ast)->STMT_BODY();

      if (body)
	last = dfa_build_expr(body, node, last); 
    }

    break;

  case MC_IF_STMT:
    {
      node = dfa_make_node(code, parent, prev);
      SET_AST_LOCATION(node,ast);
      astIf *if_stmt = (astIf *) ast;
      astStmts *then_clause = getIf_then_body(if_stmt);
      astStmts *else_clause = getIf_else_body(if_stmt);
      astExp *cond = if_stmt->STMT_COND();

      if (cond) {
	astNode_id child = dfa_build_expr(cond, node, last);

	if (child)
	  last = child;
      }

      if (then_clause) {
	astNode_id child = dfa_build_ast(then_clause, node, last);

	if (child)
	  last = child;
      }

      if (else_clause)
	last = dfa_build_ast(else_clause,node,last);

      FIX_LOCATION(node,last);
    }

    break;

  case MC_FOR_STMT:
  case MC_WHILE_STMT:
  case MC_DO_STMT:
    {
      node = dfa_make_node(code, parent, prev);
      SET_AST_LOCATION(node,ast);
      astLoop *lp = (astLoop *) ast;
      int is_do_loop = lp->IS_DO_WHILE();
      astStmts *body = lp->STMT_LOOP_BODY();
      astExp *cond = lp->STMT_LOOP_COND();

      if (!is_do_loop) {
	astList *lst = lp->STMT_LOOP_VARS();
	astTree *init = NULL;
	astTree *delta = NULL;

	if (lst->TREE_CODE() == MC_TREE_LIST) {
	  init = lst->TREE_VALUE();
	  delta = lst->TREE_PURPOSE();
	} else
	  init = lst;
	      
	if (init) {
	  if (init->TREE_CODE() == MC_VAR_DECL) {
	    astNode_id child = dfa_build_ast(init, node, last);

	    if (child)
	      last = child;
	  } else {
	    astExp *init_exp = (astExp *) init;

	    if (init->TREE_CODE() == MC_EXPR_STMT)
	      init_exp = (astExp *) ((astStmt *) init)->STMT_BODY();

	    if (init_exp) {
	      astNode_id child = dfa_build_expr(init_exp, node, last);

	      if (child)
		last = child;
	    }
	  }
	}

	if (cond) {
	  astNode_id child = dfa_build_expr(cond, node, last);

	  if (child)
	    last = child;
	}

	if (delta) {
	  astExp *delta_exp =  (astExp *) delta;

	  if (delta->TREE_CODE() == MC_EXPR_STMT)
	    delta_exp = (astExp *) ((astStmt *) delta)->STMT_BODY();

	  if (delta_exp) {
	    astNode_id child = dfa_build_expr(delta_exp, node, last); 

	    if (child)
	      last = child;
	  }
	}
	  
	if (body) {
	  astNode_id child = dfa_build_ast(body, node, last);

	  if (child)
	    last = child;
	}

	FIX_LOCATION(node,last);
      } else {
	int loop_end_line = lp->STMT_SOURCE_LINE();

	if (body) {
	  astNode_id child = dfa_build_ast(body, node, last);

	  if (child)
	    last = child;
	}

	if (cond) {
	  astNode_id child = dfa_build_expr(cond, node, last);

	  if (child)
	    last = child;
	}

	smtTree *tok_end = header->tn_na(loop_end_line);

	if (tok_end) {
	  astNode *nd = (astNode *) ((*buf)[node]);
	  nd->set_length(tok_end->tbeg - nd->get_start() + tok_end->tlth);
	}
      }
    }

    break;

  case MC_SWITCH_STMT:
    {
      node = dfa_make_node(code, parent, prev);
      SET_AST_LOCATION(node,ast);
      astCase *sw = (astCase *) ast;
      astExp *exp = (astExp *) sw->STMT_CASE_INDEX();
      last = dfa_build_expr(exp, node, last);
      astTree *body = sw->STMT_CASE_LIST();

      if (body)
	last = dfa_build_ast(body, node, last);

      FIX_LOCATION(node,last);
    }

    break;

  case MC_CONST_DECL:
  case MC_VAR_DECL:
    {
      node = dfa_make_node(code, parent, prev,ast);
      SET_AST_LOCATION(node,ast);
      ddElement *dd = dfa_find_dd((astDecl *) ast);
      SET_DD(node,dd);
      int tkn = - ast->TREE_DEBUG();
      smtTree *token = header->tn_na(tkn);

      if (token)
	SET_ID(node,token->tbeg);

      if (ast_decl_is_def(ast))
	SET_IS_DEF(node);

      ast->set_DEBUG(node);

      if (ast_is_decl_global((astDecl *) ast)) {
	last = dfa_make_node((tree_code) DFA_REF_DD, node, last, ast);
	SET_DD(last,dd);
      } else {
	last = dfa_make_node((tree_code) DFA_REF_DFA, node, last, ast);
	SET_DECL(last,node);
      }

      if (token)
	SET_SMT_LOCATION(last,token);

      if (code == MC_VAR_DECL) {
	astTree *init = ((astDecl *) ast)->DECL_INITIAL();

	if (init && init->TREE_CODE() == MC_ERROR_NODE)
	  init = NULL;

	if (!init) {
	  astType *type = ast->TREE_TYPE();

	  if (type)
	    init = DECL_CONSTRUCTOR((astDecl *) ast);

	  if (init && init->TREE_CODE() == MC_ERROR_NODE)
	    init = NULL;
	}

	if (init)
	  last = dfa_build_expr(init, node, last);
      }
    }

    break;

  case MC_FIELD_DECL:
  case MC_PARM_DECL:
    {
	node = dfa_make_node(code, parent, prev,ast);
	SET_AST_LOCATION(node,ast);
	ddElement * dd = dfa_find_dd((astDecl*)ast);
	SET_DD(node,dd);
	int tkn = - ast->TREE_DEBUG();
	smtTree * token = header->tn_na(tkn);
	if(token)
	    SET_ID(node,token->tbeg);
	if (ast_decl_is_def(ast))
	    SET_IS_DEF(node);
        if (ast_is_decl_global((astDecl*)ast))
	  {
	    last = dfa_make_node((tree_code)DFA_REF_DD,node,last,ast);
	    SET_DD(last,dd);
	  }
	else
	  {
	    last = dfa_make_node((tree_code)DFA_REF_DFA,node,last,ast);
	    SET_DECL(last,node);
	  }
	if(token)
	  SET_SMT_LOCATION(last,token);

	ast->set_DEBUG(node);
	break;
    }
      case MC_TYPE_DECL: {
	node = dfa_make_node(code, parent, prev,ast);
	SET_AST_LOCATION(node,ast);
	ddElement * dd = dfa_find_dd((astDecl*)ast);
	SET_DD(node,dd);
	int tkn = - ast->TREE_DEBUG();
	smtTree * token = header->tn_na(tkn);
	if(token)
	    SET_ID(node,token->tbeg);
	if (ast_decl_is_def(ast))
	    SET_IS_DEF(node);
        if (ast_is_decl_global((astDecl*)ast))
	  {
	    last = dfa_make_node((tree_code)DFA_REF_DD,node,last,ast);
	    SET_DD(last,dd);
	  }
	else
	  {
	    last = dfa_make_node((tree_code)DFA_REF_DFA,node,last,ast);
	    SET_DECL(last,node);
	  }
	if(token)
	  SET_SMT_LOCATION(last,token);
	ast->set_DEBUG(node);
	break;
    }

      case MC_FUNCTION_DECL: {
	  node = dfa_make_node(code, parent, prev);
	  SET_AST_LOCATION(node,ast);
	  ddElement * dd = dfa_find_dd((astDecl*)ast);
	  SET_DD(node,dd);
	  int tkn = - ast->TREE_DEBUG();
	  smtTree * token = header->tn_na(tkn);
	  if(token)
	      SET_ID(node,token->tbeg);
	  if (ast_decl_is_def(ast))
	    SET_IS_DEF(node);
	  if (ast_is_decl_global((astDecl*)ast))
	    {
	      last = dfa_make_node((tree_code)DFA_REF_DD,node,last,ast);
	      SET_DD(last,dd);
	    }
	  else
	    {
	      last = dfa_make_node((tree_code)DFA_REF_DFA,node,last,ast);
	      SET_DECL(last,node);
	    }
	  if(token)
	    SET_SMT_LOCATION(last,token);

    }
	break;

  case MC_CALL_EXPR:
  case NEW_EXPR:
    node = dfa_build_expr(ast,parent,prev);
    break;

      case EXIT_STMT:
      case MC_GOTO_STMT:
	break;

      default:
	if (dfa_debug == 0)
	  dfa_debug = init_dfa_debug();
	if (dfa_debug > 0)
	  msg("dfa_build_ast: case $1\n") << astNode_code_name(code) << eom;
	break;
    }

    return node;
#else
    return 0;
#endif
}

static astNode_id
dfa_build_smt(smtTree * smt,
	      astNode_id parent,
	      astNode_id prev)
{
  Initialize(dfa_build_smt(smtTree *, astNode_id, astNode_id));
  
  astNode_id node = NULL;
  astNode_id last = NULL;
  astTree *ast = NULL;
  
  switch (smt->get_node_type()) {

  case SMT_file: 
    {
      node = dfa_make_node((tree_code)DFA_ROOT, parent, prev);
      SET_SMT_LOCATION(node,smt);

      for (smtTree *nd = smt->get_first(); nd; nd = nd->get_next()) {
	astNode_id ch = dfa_build_smt(nd,node,last);

	if (ch)
	  last = ch;
      }

      break;
    }

  case SMT_list_decl:
    {
      node = dfa_make_node((tree_code)DFA_LIST_DECL,parent,prev);
      SET_SMT_LOCATION(node,smt);

      for (smtTree *nd = smt->get_first(); nd; nd = nd->get_next()) {
	astNode_id ch = dfa_build_smt(nd,node,last);

	if (ch)
	  last = ch;
      }
    }

    break;

  case SMT_declspec:
    {
      node = dfa_make_node((tree_code)DFA_DECLSPEC,parent,prev);
      SET_SMT_LOCATION(node,smt);

      for (smtTree *nd = smt->get_first(); nd; nd = nd->get_next()) {
	astNode_id ch = dfa_build_smt(nd,node,last);

	if (ch)
	  last = ch;
      }
    }

    break;

  case SMT_title: 
    {
      last = prev; 

      for (smtTree *nd = smt->get_first(); nd; nd = nd->get_next()) {
	astNode_id ch = dfa_build_smt(nd,parent,last);

	if (ch) {
	  last = ch;
	  node = ch;
	}
      }
    }

    break;

  case SMT_superclass :
    {
      node = dfa_make_node((tree_code)DFA_SUPERCLASS,parent,prev);
      SET_SMT_LOCATION(node,smt);
      smtTree *base = NULL;
      ddElement *ddbase = NULL;

      for (smtTree *nd = smt->get_first(); nd; nd = nd->get_next()) {
	if (ddbase = checked_cast(ddElement,get_relation(ref_dd_of_smt,nd))) {
	  base = nd;
	  break;
	}
      }

      astNode_id ch = dfa_make_node((tree_code)DFA_REF_DD,node,0);

      if (base)
	SET_SMT_LOCATION(ch,base);

      if (ddbase)
	SET_DD(ch,ddbase);
    }

    break;

  case SMT_decl:
  case SMT_stmt:
  case SMT_if:
  case SMT_nstdloop:
  case SMT_switch:
  case SMT_enum_field:
  case SMT_pdecl:
  case SMT_fdecl: 
    {
      ast = smt_get_ast(smt);

      if (ast)
	node = dfa_build_ast(ast, parent, prev);
    }

    break;

  case SMT_case_clause:
  case SMT_cbody:
  case SMT_ebody:
  case SMT_block: 
    {
      node = dfa_make_node(MC_COMPOUND_EXPR, parent, prev);
      SET_SMT_LOCATION(node,smt);

      for (smtTree *nd = smt->get_first(); nd; nd = nd->get_next()) {
	astNode_id ch = dfa_build_smt(nd,node,last);

	if (ch)
	  last = ch;
      }

      break;
    }

  case SMT_fdef: 
    {
      node = dfa_make_node((tree_code)DFA_FUNCTION_DEF, parent, prev);
      SET_SMT_LOCATION(node,smt);

      for (smtTree *nd = smt->get_first(); nd; nd= nd->get_next()) {
	if (nd->get_node_type() == SMT_fdecl) {
	  smtTree *smt_spec = nd->get_first();
	  last = dfa_make_node((tree_code)DFA_DECLSPEC,node,last);
	  smtTree *title = NULL;

	  if (smt_spec && smt_spec->get_node_type() == SMT_declspec) {
	    SET_SMT_LOCATION(last,smt_spec);
	    title = smt_spec->get_next();
	  } else
	    title = nd->get_first();

	  astNode_id fdcl = dfa_make_node(MC_FUNCTION_DECL, node, last);

	  if (!(title && title->get_node_type() == SMT_title))
	    title = NULL;

	  if (title)
	    SET_SMT_LOCATION(fdcl,title);
	  else
	    SET_SMT_LOCATION(fdcl,nd);

	  SET_IS_DEF(fdcl);
	  astTree *astfun = smt_get_ast(smt);
	  astNode_id last_ch = NULL;

	  if (astfun) {
	    ddElement *dd = dfa_find_dd((astDecl *) astfun);
	    SET_DD(fdcl,dd);
	    int tkn = - astfun->TREE_DEBUG();
	    smtTree *token = header->tn_na(tkn);

	    if (token)
	      SET_ID(fdcl,token->tbeg);

	    last_ch = dfa_make_node((tree_code)DFA_REF_DD,fdcl,last_ch,astfun);
	    SET_DD(last_ch,dd);

	    if (token)
	      SET_SMT_LOCATION(last_ch,token);
	  }

	  if (title) {
	    astNode_id ch = dfa_build_smt(title,fdcl,last_ch);
	  }

	  last = fdcl;
	} else {
	  astNode_id ch = dfa_build_smt(nd, node, last);

	  if (ch)
	    last = ch;
	}
      }

      break;
    }

  case SMT_cdecl: 
    {
      node = dfa_make_node(MC_TYPE_DECL, parent, prev);
      SET_SMT_LOCATION(node,smt);
      astTree *astcl = smt_get_ast(smt);

      if (astcl) {
	ddElement *dd = dfa_find_dd((astDecl *) astcl);
	SET_DD(node,dd);
	int tkn = - astcl->TREE_DEBUG();
	smtTree *token = header->tn_na(tkn);

	if (token)
	  SET_ID(node,token->tbeg);

	if (ast_decl_is_def((astDecl *)astcl))
	  SET_IS_DEF(node);
	    
	if (ast_is_decl_global((astDecl *) astcl)) {
	  last = dfa_make_node((tree_code)DFA_REF_DD,node,last,astcl);
	  SET_DD(last,dd);
	} else {
	  last = dfa_make_node((tree_code)DFA_REF_DFA,node,last,astcl);
	  SET_DECL(last,node);
	}

	if (token)
	  SET_SMT_LOCATION(last,token);

	for (smtTree *nd = smt->get_first(); nd; nd= nd->get_next()) {
	  astNode_id ch = dfa_build_smt(nd,node,last);

	  if (ch)
	    last = ch;
	}
      }
    }

    break;	  
      
  case SMT_edecl: 
    {
      node = dfa_make_node(MC_TYPE_DECL, parent, prev);
      SET_SMT_LOCATION(node,smt);
      astTree *astcl = smt_get_ast(smt);

      if (astcl) {
	ddElement *dd = dfa_find_dd((astDecl *) astcl);
	SET_DD(node,dd);
	int tkn = - astcl->TREE_DEBUG();
	smtTree *token = header->tn_na(tkn);

	if (token)
	  SET_ID(node,token->tbeg);

	if (ast_decl_is_def((astDecl *)astcl))
	  SET_IS_DEF(node);

	if (ast_is_decl_global((astDecl *) astcl)) {
	  last = dfa_make_node((tree_code)DFA_REF_DD,node,last,astcl);
	  SET_DD(last,dd);
	} else {
	  last = dfa_make_node((tree_code)DFA_REF_DFA,node,last,astcl);
	  SET_DECL(last,node);
	}

	if (token)
	  SET_SMT_LOCATION(last,token);

	for (smtTree *nd = smt->get_first(); nd; nd= nd->get_next()) {
	  astNode_id ch = dfa_build_smt(nd,node,last);

	  if (ch)
	    last = ch;
	}
      }
    }

    break;	  
      
  case SMT_break:
  case SMT_continue:
  case SMT_goto:
  case SMT_token:
  case SMT_comment:
  case SMT_m_include:
  case SMT_m_define:
  case SMT_m_if:
  case SMT_m_else:
  case SMT_m_endif:
  case SMT_m_undef:
  case SMT_m_pragma:
  case SMT_m_gen:
  case SMT_temp:
  case SMT_label:
    break;

  case SMT_macrocall:
  case SMT_expr:
    ast = smt_get_ast(smt);

    if (ast)
      node = dfa_build_ast(ast,parent,prev);

    break;
      
  default:
    if (dfa_debug == 0)
      dfa_debug = init_dfa_debug();

    if (dfa_debug > 0)
      msg("dfa_build_smt: case $1\n") << smt_token_name(smt->get_node_type()) << eom;

    break;
  }

  return node;
}

static astNode_id
dfa_make_node(tree_code code,
	      astNode_id parent,
	      astNode_id prev,
	      astTree *ast)
{
  Initialize(dfa_make_node(tree_code, astNode_id, astNode_id, astTree *));

  astNode_id retval = 0;
  astNode *node = (astNode *) buf->grow(astNode::get_node_size(code));
  byte *start = (*buf)[0];
  node->set_code(code);
  node->set_type((ddElement*)0);
  node->set_is_last(1);
  node->set_is_leaf(1);

  if (parent)
    node->set_parent((astNode*)(start+parent));

  if (prev)
    ((astNode *) (start + prev))->set_next(node);

  if (ast) {
    int level = ast_level(ast);
    node->set_level(level);
  }

  retval = (byte *) node - start;
  return retval;
}

static ddElement *
dfa_find_dd(astDecl *decl)
{
  Initialize(dfa_find_dd(astDecl *));
    
  ddElement *retval = 0;
  tree_code code = decl->TREE_CODE();

  if (code == MC_STRING_CST) {
    if (TREE_START_LINE(decl)) {
      smtTree *token = header->tn_na(TREE_START_LINE(decl));

      if (token && token->extype == SMTT_string)
	retval = checked_cast(ddElement,get_relation(ref_dd_of_smt,token));
    }
  } else if (code == MC_PARM_DECL || code == MC_VAR_DECL 
	     && !ast_is_decl_global(decl)) {
    smtTree *smt_local = ast_get_smt(decl);

    if (smt_local) {
      ddElement *type = checked_cast(ddElement, get_relation(semtype_of_smt_decl, smt_local));
      genString name;

      if (type) {
	name = type->get_name();
	name += " ";
      }

      smtTree *id = checked_cast(smtTree, get_relation(id_of_decl, smt_local));
      bool good = 0;

      if (id && id->tlth) {
	good = 1;
	genString nm;
	nm.put_value(&header->srcbuf[id->tbeg], id->tlth);
	name += nm;
      }

      if (good)
	retval = dd_lookup_or_create((char*)name, (appTree*)header, 0, DD_LOCAL, 0, 0);

      if (retval)
	put_relation(ref_smt_of_dd, retval, smt_local);
    }
  } else {
    smtTree *smt = ast_get_smt(decl);

    if (smt)
      retval = smt_get_dd(smt);

    if (!retval || retval->get_kind() == DD_MACRO)
      retval = lookup_ast_to_dd (decl, dr);
  }

  return retval;
}

static astNode_id
dfa_find_dfa(astDecl *decl)
{
  Initialize(dfa_find_dfa(astDecl *));

  int val= decl->TREE_DEBUG();
  int ret = val > 0 ? val : 0;
  return ret;
}

static int
dfa_find_location(astTree *ast,
		  int &start,
		  int &length)
{
  Initialize(dfa_find_location(astTree *, int &, int &));
    
  smtTree *st, *en;
  int retval = 0;
  start = -1;
  length = -1;
  char *fn = TREE_FILENAME(ast);

  if (fn && smtHeader::find(fn) == header) {
    if (TREE_START_LINE(ast)) {
      st = header->tn_na(TREE_START_LINE(ast));

      if (st) {
	start = st->tbeg;
	retval = 1;

	if (TREE_END_LINE(ast) >= TREE_START_LINE(ast)) {
	  en = header->tn_na(TREE_END_LINE(ast));

	  if (en)
	    length = en->tbeg + en->tlth - start;
	}
      }
    }
  }

  return retval;
}

static void
dfa_text_info(smtHeader *head,
	      int start,
	      int len,
	      char *text,
	      int max)
{
  Initialize(dfa_text_info(smtHeader *, int, int, char *, int));

  if (!head->srcbuf)
    head->get_root(); // load smtTree

  char const *src = &head->srcbuf[start];
  char *dst = &text[0];

  if (len < max) {
    for (int ii = 0; ii < len; ++ii)
      *dst++ = src[ii] > ' ' ? src[ii] : ' ';
  } else {
    int half = (max/2) - 3; // 32/2-3 = 13 
    int ii;
    for (ii = 0; ii < half; ++ii)
      *dst++ = src[ii] > ' ' ? src[ii] : ' ';

    *dst++ = ' ';
    *dst++ = '.';
    *dst++ = '.';
    *dst++ = '.';
    *dst++ = ' ';
    src = &head->srcbuf[start+len-half];
    for (ii = 0; ii < half; ++ii)
      *dst++ = src[ii] > ' ' ? src[ii] : ' ';
  }	    

  *dst = '\0';
}

static int
dfa_print_tree_internal(int opt,
			smtHeader *head,
			astNode *node,
			ostream &os,
			char *prefix)
{
  Initialize(dfa_print_tree_internal(int, smtHeader *, astNode *, ostream &, char *));

  tree_code code = (tree_code)node->get_code();
  os << endl;

  if (prefix)
    os << prefix;

  os << astNode_code_name(code) << ' ';

  if (opt)
    os << (void*)node << ' ' << node->get_level() << ' ';

  os << node->get_start() << ' ' << node->get_length();

  if (opt) {
    if (code == (tree_code)DFA_REF_DFA) {
      dfa_decl * dcl = ((dfa_ref_dfa*)node)->get_decl();
      os << ' ' << (void*)dcl;
    } else {
      switch (code) {

      case DFA_REF_DD:
      case MC_FIELD_DECL:
      case MC_VAR_DECL:
      case MC_PARM_DECL:
      case MC_FUNCTION_DECL:
      case MC_TYPE_DECL:
      case TYPE_DECL_MODIFIED:
      case MC_CONST_DECL:
      case DFA_CURSOR_DECL:
      case TEMPLATE_FCN:
      case TEMPLATE_CLASS:
      case TEMPLATE_STRUCT:
      case TEMPLATE_UNION:
      case TEMPLATE_STATIC_DATA_MBR:
	{
	  ddElement *dd = ((dfa_ref_dd *) node)->get_dd();

	  if (dd) {
	    os << ' ' << ddKind_name(dd->get_kind()) << ' ' << (void*)dd
	      << ' ' << dd->get_ddname();
	  }
	    
	  if (code != (tree_code) DFA_REF_DD)
	    os << ' ' << ((dfa_decl *) node)->get_id();
	}

	break;
	  
      case MC_INTEGER_CST:
	os << ' ' << ((dfa_integer_cst *) node)->get_value();
	break;
	  
      case MC_REAL_CST:
	os << ' ' << ((dfa_real_cst *) node)->get_value();
	break;
			  
      default:
	{
	  ddElement *type = node->get_type();

	  if (type) {
	    os << " Type " << ddKind_name(type->get_kind()) << ' ' << (void*)type
	      << ' ' << type->get_ddname();
	  }
	}
	break; 
      }
    }
  }

  int len = node->get_length();
  int start = node->get_start();

  if (len > 0) {
    char text[32];
    dfa_text_info(head, start, len, text, 32);
    os << " : `" << &text[0] << "'";
  }
    
  genString pref = prefix;

  if (prefix && !node->get_is_last())
    pref += "| ";
  else
    pref += "  ";

  for (astNode *ch = node->get_first(); ch; ch = ch->get_next())
    dfa_print_tree_internal(opt, head, ch, os, pref);

  return 0;
}


//--------------------------------------------------------------------------------
// This static function sends AST node with all its attributes to the given stream.
// It recursively sends all AST child nodes to the same stream.
//--------------------------------------------------------------------------------
const char*  ATT_etag(symbolPtr& sym);
static int dfa_dump_tree_internal(astXref *axr,
			                      astNode *node,
			                      ostream &os,
								  int level) {
  Initialize(dfa_dump_tree_internal(astXref *, astNode *, ostream &));

  level++;
  tree_code code = (tree_code)node->get_code();

  // Tree level
  os << level << '\t';

  // Node type
  os << astNode_code_name(code) << '\t';

  // Offset and length in the source file
  os << node->get_start() << '\t' << node->get_length() <<'\t';

  symbolPtr sym(node,axr);

  // Node name if any
  char const *name;
  name = sym.get_name();
  if(name!=NULL && strlen(name)>0) {
	  os << name << '\t';
  } else {
	  os << ' ' << '\t';
  }

  // Node etag
  const char *etag;
  etag = ATT_etag(sym);
  if(etag!=NULL && strlen(etag)>0) {
	  os << etag << '\t';
  } else {
	  os << ' ' << '\t';
  }
  os << endl;


  for (astNode *ch = node->get_first(); ch; ch = ch->get_next())
    dfa_dump_tree_internal(axr, ch, os,level);
  level--;
  return 0;
}
//--------------------------------------------------------------------------------


int
dfa_print_tree(smtHeader *head,
	       astNode *node,
	       ostream &os)
{
  Initialize(dfa_print_tree(smtHeader *, astNode *, ostream &));

  int retval = dfa_print_tree_internal(1, head, node, os, NULL);
  os << endl;
  return retval;
}

int
dfa_print_tree(db_buffer *buffer,
	       smtHeader *head)
{
  Initialize(dfa_print_tree(db_buffer *, smtHeader *));
  int retval = 0;

  if (dfa_print_after_restore) {
    astNode *dfa_root = (astNode *) ((*buffer)[0] +
				     astNode::get_node_size(MC_ERROR_NODE));
    retval = dfa_print_tree(head,dfa_root,cout);
  }

  return retval;
}

static ddElement *
dfa_get_dd_from_int_cst(astTree *cst)
{
  Initialize(dfa_get_dd_from_int_cst(astTree *));

  ddElement *retval = NULL;
  astType *tdecl= cst->TREE_TYPE();

  if (tdecl && tdecl->TREE_CODE() != MC_ENUMERAL_TYPE) {
    tdecl = NULL;
    cst = (astTree *) cst->TREE_DEBUG();

    if (cst)
      tdecl = cst->TREE_TYPE();
  } 
    
  if (tdecl && tdecl->TREE_CODE() == MC_ENUMERAL_TYPE) {
    astIdentifier *purp = NULL;

    for (astList *list =  (astList *) (tdecl->TYPE_VALUES());
	 list;
	 list= (astList *)(list->TREE_CHAIN())) {
      if (list->TREE_VALUE() == cst) {
	purp = (astIdentifier*)list->TREE_PURPOSE();
	break;
      }
    }

    if (purp) {
      char *name = purp->IDENTIFIER_POINTER();

      if (name) {
	genString tm;
	get_qualify_type_name((astType *) tdecl->context,tm);

	if (tm.length())
	  tm += "::";

	tm += name;
	retval = dr->lookup(DD_ENUM_VAL,(char*)tm);
      }
    }
  }

  return retval;
}

void dataCell_print(const symbolPtr &datasym, ostream &, int);

int
astNode_print(int opt,
	      const symbolPtr &sym,
	      ostream &os,
	      int level)
{
  Initialize(astNode_print(int, const symbolPtr &, ostream &, int));

  if (sym.is_dataCell()) {
    dataCell_print(sym, os, level);
    return 0;
  }
    
  astNode *node = astNode_of_sym(sym);
  astXref *axr = sym.get_astXref();
  app *head = (app *) axr->get_module();
  genString pref;

  for (int ii = 0; ii < level; ++ii)
    pref += "  ";

  int retval = dfa_print_tree_internal(opt, (smtHeader *) head,
				       node, os, (char *) pref);
  os << endl;
  return retval;
}

int
astNode_dump(const symbolPtr &sym,
	         ostream &os)
{
  Initialize(astNode_dump(const symbolPtr &, ostream &));

  if (sym.is_dataCell()) {
    return 0;
  }
    
  astNode *node = astNode_of_sym(sym);
  astXref *axr = sym.get_astXref();
  int retval = dfa_dump_tree_internal(axr,
				                      node, os,0);
  os << endl;
  return retval;
}


int
astNode_print(const symbolPtr &sym,
	      ostream &os,
	      int level)
{
  Initialize(astNode_print(const symbolPtr &, ostream &, int));

  return
    astNode_print(1, sym, os, level);
}

const char *ATT_kind( symbolPtr &sym);
const char *ATT_name( symbolPtr &sym);
const char *ATT_type( symbolPtr &sym);
const char *ATT_instance_info(symbolPtr &sym);
int ATT_level(symbolPtr &sym);

const char *dataCell_types[]  =
{ "", "argument", "return" };

const char *
dfa_get_type(const symbolPtr &sym)
{
  Initialize(dfa_get_type(const symbolPtr &));

  if (sym.is_dataCell()) {
    dataCell *cell =  dataCell_of_symbol(sym, false);

    if (cell) {
      dataCell_type type = cell->get_type();
      dataCell_offload(cell, false);

      if (type & dataCell_Argument)
	return "argument";
      else if (type & dataCell_Result)
	return "return";
    }
  } else if (sym.is_ast()) {
    astNode*node =  astNode_of_sym(sym);

    if (!node)
      return NULL;
    int code = node->get_code();
    
    switch (code) {

    case MC_INTEGER_CST:
      return "int";
    case MC_REAL_CST:
      return "double";
    default:
      {
	ddElement* dd = node->get_type();
	if (dd)
	  return dd->get_name();

	symbolPtr xsym = sym.get_xrefSymbol();

	if (xsym.xrisnotnull())
	  return ATT_type(xsym);
      }  
    }
  }

  return NULL;
}

extern bool get_type_sym(const symbolPtr &, symbolPtr &);

bool dfa_get_type_sym(const symbolPtr &sym, symbolPtr &type)
{
  Initialize(dfa_get_type_sym(const symbolPtr &sym, symbolPtr &type));

  bool ret = false;

  if (sym.is_dataCell()) {
    // nothing for now
  } else if (sym.is_ast()) {
    astNode *node = astNode_of_sym(sym);
    if (node) {
      ddElement *type_dd = node->get_type();
      if (!type_dd) {
	int code = node->get_code();
	ddElement *nd = NULL; 
	dfa_decl* dcl = NULL;
	switch (code) {
	case DFA_REF_DD:
	  nd = ((dfa_ref_dd *) node)->get_dd();
	  break;
	  
	case DFA_REF_DFA:
	  dcl = ((dfa_ref_dfa*)node)->get_decl();
	  if (dcl)
	    nd = dcl->get_dd();
	  break;
	  
	default:
	  break;
	}

	if (nd)
	  type_dd = checked_cast(ddElement, get_relation(semtype_of_smt_decl,nd));
      }
      if (type_dd) {
	type = type_dd->get_xrefSymbol();
	if (type.xrisnotnull())
	  ret = true;
      } else {
	symbolPtr xsym = sym.get_xrefSymbol();
	if (xsym.xrisnotnull()) {
	  if (get_type_sym(xsym, type) == true)
	    ret = true;
	}
      }
    }
  } else {
    // ??
  }

  return ret;
}

const char *
ATT_ast_info(symbolPtr &sym)
{
  Initialize(ATT_ast_info(symbolPtr &));

  static genString buf;
  ddKind kind = sym.get_kind();

  if (kind == DD_INSTANCE) {
    return ATT_instance_info(sym);
  } else if (kind == DD_DATA) {
    dataCell *cell =  dataCell_of_symbol(sym, false);

    if (cell) {
      dataCell_type tp = cell->get_type();
      int argno = cell->get_argno();
      int level = cell->get_level();
      dataCell_type type = cell->get_type();
      symbolPtr ast = sym.get_astNode();
      const char *str = ATT_ast_info(ast);
      dataCell_offload(cell, false);

      if (type & dataCell_Result)
	buf.printf("%s return", str, type);
      else if (type & dataCell_Argument)
	buf.printf("%s argument %d", str, type, argno);
      else
	buf.printf("%s level %d", str, level);
    }

    return buf;
  } else if (kind == DD_AST) {
    astNode *node = astNode_of_sym(sym);
    astXref *axr = sym.get_astXref();
    smtHeader *head = (smtHeader *) (app *) axr->get_module();
    int code = node->get_code();
    char text[32];
    char *pt = text;
    int len = node->get_length();
    int start = node->get_start();

    if (len)
      dfa_text_info(head, start, len, text, 32);
    else
      *pt = 0;

    buf.printf("%s %d %d `%s'", astNode_code_name(code), start, len, pt);
  } else {
    buf.printf("(%s) %s", ATT_kind(sym), ATT_name(sym));
  }

  return buf;
}

const char* ATT_ast_text(symbolPtr &sym)
{
  Initialize(ATT_ast_text(symbolPtr &));
  if(!sym.is_ast())
    return NULL;

  astNode *node = astNode_of_sym(sym);
  astXref *axr = sym.get_astXref();
  smtHeader *head = (smtHeader *) (app *) axr->get_module();
  int len = node->get_length();
  if(!len)
    return NULL;

  int start = node->get_start();
  
  if (!head->srcbuf)
    head->get_root(); // load smtTree
    
  static genString buf;
  buf.put_value(head->srcbuf+start, len);
  
  return buf;
}

const char* ATT_ast_spaces_before(symbolPtr &sym)
{
  if(!sym.is_ast())
    return NULL;

  astNode *node = astNode_of_sym(sym);
  astXref *axr = sym.get_astXref();
  smtHeader *head = (smtHeader *) (app *) axr->get_module();
  int len = node->get_length();
  if(!len)
    return NULL;

  if (!head->srcbuf)
    head->get_root(); // load smtTree
    
  const char* stop = head->srcbuf-1;
  const char* ptr = stop + node->get_start();
  int no_white = 0;
  while(ptr != stop){
    int ch = *ptr;
    if(!isspace(ch)) {
      ++ptr;
      break;
    }
    ++ no_white;
    if(ch=='\n'){
      if(ptr[-1] == '\r'){
	++no_white;
	--ptr;
      }
      break;
    }
    --ptr;
  }
  
  const char* retval;
  if(no_white > 0){
    static genString buf;
    buf.put_value(ptr, no_white);
    retval = buf;
  } else {
    retval = NULL;
  }
  return retval;
}

const char* ATT_ast_spaces_after(symbolPtr &sym)
{
  if(!sym.is_ast())
    return NULL;

  astNode *node = astNode_of_sym(sym);
  astXref *axr = sym.get_astXref();
  smtHeader *head = (smtHeader *) (app *) axr->get_module();
  int len = node->get_length();
  if(!len)
    return NULL;

  if (!head->srcbuf)
    head->get_root(); // load smtTree
    
  const char* stop = head->srcbuf + head->src_size;
  const char* start = head->srcbuf + node->get_start() + len;
  int no_white = 0;

  const char *ptr;
  for(ptr = start; ptr != stop; ++ptr){
    int ch = *ptr;
    if(!isspace(ch))
      break;
    if(ch=='\n'){
      if(ptr[-1] == '\r'){
	start = ptr - 1;
	no_white = 2;
      } else {
	start = ptr;
	no_white = 1;
      }
    } else {
      ++ no_white;
    }
  }

  const char* retval;
  if(no_white > 0){
    static genString buf;
    buf.put_value(ptr, no_white);
    retval = buf;
  } else {
    retval = NULL;
  }
  return retval;
}
 
static astNode *
dfa_get_ast(const symbolPtr &sym)
{
  Initialize(dfa_get_ast(const symbolPtr &));

  astNode *root = NULL;
  symbolPtr astsym = ((symbolPtr &) sym).get_astNode();

  if (astsym.is_ast())
    root = astNode_of_sym(astsym);

  return root;
}

bool
dfa_equal(astNode *n1,
	  astNode *n2)
{
  Initialize(dfa_equal(astNode *, astNode *));

  int c1 = n1->get_code();
  int c2 = n2->get_code();

  if (c1 != c2)
    return false;
  
  switch (c1) {

  case DFA_REF_DD: 
    {
      ddElement *d1 = ((dfa_ref_dd *) n1)->get_dd();
      ddElement *d2 = ((dfa_ref_dd *) n2)->get_dd();
      symbolPtr s1 = d1;
      symbolPtr s2 = d2;
      symbolPtr x1 = s1.get_xrefSymbol();
      symbolPtr x2 = s2.get_xrefSymbol();

      return
	(x1.isnotnull() && x2.isnotnull() && (x1.sym_compare(x2) == 0));
    }

  case DFA_REF_DFA:
    {
      dfa_decl *d1 = ((dfa_ref_dfa *) n1)->get_decl();
      dfa_decl *d2 = ((dfa_ref_dfa *) n2)->get_decl();

      return
	(d1 == d2);
    }

  case MC_INTEGER_CST:
    {
      int i1 = ((dfa_integer_cst *) n1)->get_value();
      int i2 = ((dfa_integer_cst *) n2)->get_value();

      return
	(i1 == i2);
    }

  case MC_REAL_CST:
    {
      double r1 = ((dfa_real_cst *) n1)->get_value();
      double r2 = ((dfa_real_cst *) n2)->get_value();

      return
	(r1 == r2);
    }

  case MC_VAR_DECL:
  case MC_PARM_DECL:
  case MC_FUNCTION_DECL: 
  case MC_FIELD_DECL:
    return n1 == n2;

  default: 
    n1 = n1->get_first();
    n2 = n2->get_first();

    for (;;) {
      if (!n1)
	return !n2;
      else if (!n2)
	return false;
      else if (!dfa_equal(n1, n2))
	return false;

      n1 = n1->get_next();
      n2 = n2->get_next();
    }
  }

  return false;
}

bool
dfa_equal(const symbolPtr &s1,
	  const symbolPtr &s2)
{
  Initialize(dfa_equal(const symbolPtr &, const symbolPtr &));

  astNode *n1 = dfa_get_ast(s1);
  astNode *n2 = NULL;

  if (!n1)
    return false;

  ddKind kind = s2.get_kind();

  switch (kind) {

  case DD_AST:
    n2 = dfa_get_ast(s2);
    return dfa_equal(n1, n2);

  case DD_INSTANCE:
    n2 = dfa_get_ast(s2);
    return n1 == n2;

  default:   // xrefSymbol
    {
      symbolPtr x1 = s1.get_xrefSymbol();
      symbolPtr x2 = s2.get_xrefSymbol();
      return 
	(x1.isnotnull() && x2.isnotnull() && (x1.sym_compare(x2) == 0));
    }
  }

  return false;
}

char const *
dfa_get_name(const symbolPtr &sym)
{
  Initialize(dfa_get_name(const symbolPtr &));
  
  if (sym.is_ast()) {
    astNode *node =  dfa_get_ast(sym);

    if (!node)
      return NULL;

    int code = node->get_code();
    static genString buf;
      
    switch(code) {

    case MC_INTEGER_CST: 
      {
	int ii = ((dfa_integer_cst *) node)->get_value();
	buf.printf("%d", ii);
	return buf;
      }

    case MC_REAL_CST: 
      {
	double rr = ((dfa_real_cst *) node)->get_value();
	buf.printf("%f", rr);
	return buf;
      } 

    default: 
      {
	char const *name = NULL;
	symbolPtr xsym = sym.get_xrefSymbol();

	if (xsym.xrisnotnull())
	  name = xsym.get_name();

	return name;
      }  
    }
  } else if (sym.is_dataCell()) {
    dataCell *cell = dataCell_of_symbol(sym, false);

    if (cell) {
      char const *name = cell->get_name();
      dataCell_offload(cell, false);
      return name;
    }
  }

  return NULL;
}

static unsigned int
ast_type_level(astType *type)
{
  Initialize(ast_type_level(astType *));
  unsigned int retval = 0;
  
  if (type) {
    tree_code code = type->TREE_CODE();
      
    switch (code) {

    case MC_POINTER_TYPE:
    case MC_REFERENCE_TYPE:
      {
	astType *ptype = type->TREE_TYPE();
	unsigned int plevel = ast_type_level(ptype);
	retval = plevel + 1;
      }

      break;

    case MC_ARRAY_TYPE:
      {
	astType *ptype = type->TREE_TYPE();
	retval = ast_type_level(ptype);
      }
      break;

    default: 
      break;
    }
  }

  return retval;
}

static unsigned int
ast_level(astTree *ast)
{
  Initialize(ast_level(astTree *));
  unsigned int retval = 0;

  if (ast) {
    astType *type = ast->TREE_TYPE();
    retval = ast_type_level(type);
  }

  return retval;
}

int els_get_ast_code(astnodeStruct *a);
ddElement *els_get_dd(int id);
int els_get_decl_id(int id);
void els_set_decl_id(int id, int val);
int els_get_identifier(astnodeStruct *);
int els_get_id_location(astnodeStruct *a);

int
els_decl_is_def(astnodeStruct *a,
		astNode_id node)
{
  Initialize(els_decl_is_def(astnodeStruct *, astNode_id));

  int retval = 1;
  astNode *nd = (astNode *) (*buf)[node];
  int code = nd->get_code();
  astNode *par;
  ddElement *dd = NULL;

  switch (code) {

  case MC_FUNCTION_DECL:
    par = nd->get_parent();

    if (par->get_code() != DFA_FUNCTION_DEF)
      retval = 0;

    break;

  case DFA_CURSOR_DECL:
    par = nd->get_parent();

    if (par->get_code() != DFA_CURSOR_DEF)
      retval = 0;

    break;

  case MC_TYPE_DECL:
  case TYPE_DECL_MODIFIED:
    if (a->identifier)
      dd = els_get_dd(a->identifier);

    if (!dd || !dd->get_is_def())
      retval = 0;

  default:
    break;
  }

  return retval;
}

static int
dd_is_type(ddKind kind)
{
  return kind == DD_SEMTYPE ||
         kind == DD_TYPEDEF || 
	 kind == DD_ENUM || 
	 kind == DD_CLASS || 
	 kind == DD_UNION; 
}

static astNode_id
dfa_build_els(astnodeStruct *an,
	      astNode_id par,
	      astNode_id prev)
{
  Initialize(dfa_build_els(astnodeStruct *an, astNode_id par, astNode_id prev));

  astNode_id node = NULL;
  astNode_id next_par = NULL;
  astNode_id last = NULL;
  ddElement *dd = NULL;
  astNode_id decl = NULL;
  int identifier = 0;
  int id_loc;

  int code = els_get_ast_code(an);

  if (code >= 0) {
    node = dfa_make_node((tree_code) code, par, prev);
    SET_ELS_LOCATION(node,an);

    switch (code) {

    case MC_VAR_DECL:
    case MC_PARM_DECL:
      identifier = els_get_identifier(an);
      dd = els_get_dd(identifier);
      SET_DD(node,dd);

      if (dd && dd->get_kind() == DD_LOCAL)
	els_set_decl_id(identifier, node);

      if (els_decl_is_def(an,node))
	SET_IS_DEF(node);

      id_loc = els_get_id_location(an);
      SET_ID(node,id_loc);
      break;

    case MC_TYPE_DECL:
    case TYPE_DECL_MODIFIED:
    case MC_FIELD_DECL:
    case MC_FUNCTION_DECL:
    case MC_STRING_CST:
    case MC_CONST_DECL:
    case DFA_CURSOR_DECL:
    case TEMPLATE_FCN:
    case TEMPLATE_CLASS:
    case TEMPLATE_STRUCT:
    case TEMPLATE_UNION:
    case TEMPLATE_STATIC_DATA_MBR:
      identifier = els_get_identifier(an);
      dd = els_get_dd(identifier);
      SET_DD(node,dd);

      if (els_decl_is_def(an,node))
	SET_IS_DEF(node);

      id_loc = els_get_id_location(an);
      SET_ID(node,id_loc);
      break;
 	  
    case DFA_REF_DD:
      dd = els_get_dd(an->identifier);
      SET_DD(node,dd);
      break;
  
    case DFA_REF_DFA:
      decl = els_get_decl_id(an->identifier);
      SET_DECL(node,decl);
      break;

    default:
      if (astNode::is_typed_expression(code))
	{
	  identifier = els_get_identifier(an);
	  if (identifier > 0)
	    dd = els_get_dd(identifier);
	  if (dd && dd_is_type(dd->get_kind()))
	    SET_TYPE(node,dd);
	}
      break;
    }
  }
  
  if (node) {
    last = NULL;
    next_par = node;
  } else {
    last = prev;
    next_par = par;
  }

  if (an->children) {
    for (int ii = 0; ii < an->children->size(); ++ii) {
      astnodeStruct *ch = (*(an->children))[ii];
      last = dfa_build_els(ch, next_par, last);
    }
  }
    
  if (!node)
    node = last;

  return node;
}

astRoot *
dfa_els_build(smtHeader *h,
	      astnodeStruct *an)
{
  Initialize(dfa_els_build(smtHeader *,astnodeStruct *));
    
  header = h;
  h->last_token = NULL;
  dr = dd_sh_get_dr(h);
  buf = new db_buffer;

  astNode_id dfa_null = dfa_make_node(MC_ERROR_NODE,0,0);
  astNode_id dfa_root = dfa_build_els(an, 0, 0);
  SET_LOCATION(dfa_root,0,h->src_size);
  buf->compress();
    
  astRoot *ast_root = astRoot::get_create_astRoot(h, buf);

  buf = NULL;
  header = NULL;
  dr = NULL;

  astNode *root_node = ast_root->get_astNode();
//  dfa_process_tokens(h, root_node);
  dfa_dump_tree(h, root_node);

  return ast_root;
}
