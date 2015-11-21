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
#include <stdio.h>
#else /* ISO_CPP_HEADERS */
#include <cstdio>
namespace std {};
using namespace std;
#endif /* ISO_CPP_HEADERS */
#include <general.h>
#include <genError.h>
#include <machdep.h>
#include "dfa.h"


static const char * dfa_tree_name[] = {

#define MCTREECODE(SYM, NAME, CLASS, NKIDS, TYPE)  NAME,
#include "discover-tree-codes.def"
#undef MCTREECODE

    "@@dummy",
    "ast_root",
    "local_id",
    "global_id",
    "list_decl",
    "function_def",
    "ast_declspec",
    "ast_superclass",
    "package_body",
    "assign_stmt",
    "div_expr",
    "elsif_clause",
    "exception_handler",
    "exception_decl",
    "where",
    "cursor_def",
    "cursor_decl",
    "for_stmt",
    "while_stmt",
    "do_while_stmt",
    "case_values",
    "switch_clause",
    "catch",
    "try_block",
    "dynamic_init",
    "name_clause",
    "condition",
    "pm_field_ref",
    "alshift_assign_expr",
    "arshift_assign_expr",
    "bit_and_assign_expr",
    "bit_ior_assign_expr",
    "bit_xor_assign_expr",
    "minus_assign_expr",
    "mult_assign_expr",
    "plus_assign_expr",
    "rdiv_assign_expr",
    "trunc_div_assign_expr",
    "trunc_mod_assign_expr",
    "no_operand",
    "ctor_init",
    "case_label",
    "address_of_ellipsis",
    "template_fcn",
    "template_class",
    "template_struct",
    "template_union",
    "template_static_data_mbr",
    "template_header",
    "template_body",
    "microsoft_try",
    "using_decl",
    "finally",
    "synchronized",
    "easy_new",
    "java_vec_new",
    "instanceof",
    "initialization_block",
    "lrshift_expr",
    "lrshift_assign_expr",
    "type_decl_modified",
    "class_literal",
    "null",
    "super",
    "implements",
    "parens",
    "unary_plus",
    "dimensions",
    "anonymous_class",
    "throw_spec",
    "this",
    "typespec",
    "typeid",
    "proxy_import",
    "assume",
    "asm_pseudofunction",
    "java_component_ref",
    "cross_join_expr",
    "full_join_expr",
    "inner_join_expr",
    "left_join_expr",
    "ljoin_expr",
    "right_join_expr",
    "rjoin_expr",
    "case_default",
    "error"
};

int astNode::length_inited = 0;

int dfa_tree_length [LAST_DFA_TREE_CODE] = { 0 };
static char astNode_expr [LAST_DFA_TREE_CODE] = { 0 };

int astNode_init_expressions(char *array);

astNode * astNode::get_parent ()
{
    Initialize(astNode::get_parent);

    astNode * retval = NULL;
    astNode * cur = this;
    for (; !cur->is_last; cur = cur->get_next_internal())
	;
    retval = cur->get_next_internal();
    if (retval == this)
      retval = NULL;
    return retval;
}

astNode * astNode::get_prev ()
{
    Initialize(astNode::get_prev);

    astNode * retval = NULL;
    astNode * par = get_parent();
    if (par) {
	astNode * nxt = NULL;
	for (astNode * prev = par->get_first(); prev; prev = nxt) {
	    nxt = prev->get_next();
	    if (nxt == this) {
		retval = prev;
		break;
	    }
	}
    }
    return retval;
}

int astNode::is_typed_expression(int code)
{
  int retval = 0;
  if (code > 0 && code < LAST_DFA_TREE_CODE)
    retval = astNode_expr[code];
  return retval;
}

int dfa_init_tree_length()
{
    astNode_init_expressions(&astNode_expr [0]);

    for(int i=0; i < LAST_DFA_TREE_CODE; ++i)
      if (astNode_expr [i])
	dfa_tree_length[i] = 16;
      else
	dfa_tree_length[i] = 12;

    dfa_tree_length[MC_INTEGER_CST] = 16;
    dfa_tree_length[MC_REAL_CST] = 20;
    dfa_tree_length[DFA_REF_DFA] = 16;
    dfa_tree_length[DFA_REF_DD] = 16;
    dfa_tree_length[MC_FUNCTION_DECL] = 20; 
    dfa_tree_length[MC_CONST_DECL] = 20; 
    dfa_tree_length[MC_TYPE_DECL] = 20;  
    dfa_tree_length[TYPE_DECL_MODIFIED] = 20;  
    dfa_tree_length[MC_VAR_DECL] = 20;
    dfa_tree_length[MC_PARM_DECL] = 20; 
    dfa_tree_length[MC_FIELD_DECL] = 20;
    dfa_tree_length[DFA_CURSOR_DECL] = 20;
    dfa_tree_length[TEMPLATE_FCN] = 20;
    dfa_tree_length[TEMPLATE_CLASS] = 20;
    dfa_tree_length[TEMPLATE_STRUCT] = 20;
    dfa_tree_length[TEMPLATE_UNION] = 20;
    dfa_tree_length[TEMPLATE_STATIC_DATA_MBR] = 20;

    astNode::length_inited = 1;
    return 0;
}

int astNode::get_node_size(tree_code code)
{ 
    if (!length_inited)
	dfa_init_tree_length();
    return dfa_tree_length[code];
}

void astNode::set_parent(astNode* par)
{
    next = (char*)par - (char*)this;
    is_last = 1;
    par->is_leaf = 0;
}

void astNode::set_next(astNode* nxt)
{
    next = (char*)nxt - (char*)this;
    is_last = 0;
}

void dfa_real_cst::set_value(double val)
{
    OSapi_bcopy(&val, &value[0], sizeof(double));
}

double dfa_real_cst::get_value()
{
    double retval;
    OSapi_bcopy(&value[0], &retval, sizeof(double));
    return retval;
}

int astNode::get_node_size() 
{ 
    if (!length_inited)
	dfa_init_tree_length();
    return dfa_tree_length[code];
}


int dfa_fix_code_name()
{
   return 0;
}

static int dfa_fix_code_name_()
{
   dfa_tree_name[MC_INTEGER_CST] = "integer_const";
   dfa_tree_name[MC_REAL_CST] = "real_const";
   dfa_tree_name[MC_COMPLEX_CST] = "complex_const";
   dfa_tree_name[MC_STRING_CST] = "string_const";
   dfa_tree_name[MC_MODIFY_EXPR] = "assign_expr";
   dfa_tree_name[MC_CONSTRUCTOR] = "ast_initializer";
   dfa_tree_name[MC_COMPOUND_EXPR] = "ast_block";
   return 0;
}

const char* astNode_code_name(int code)
{
    static int fix_dummy = dfa_fix_code_name_();
    const char* name;
    if (code < 0 || code >= LAST_DFA_TREE_CODE)
      name = "";
    else
      name = dfa_tree_name[code];
    return name;
}

