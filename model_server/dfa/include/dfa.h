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
#ifndef __DFA_H__
#define __DFA_H__

#include <astTree.h>
#include <ast_cplus_tree.h>

// Clean up some Windows crud [on "g++.exe (GCC) 3.4.5 (mingw special)"]
#ifdef THIS
#undef THIS
#endif

class symbolPtr;

enum dfa_tree_code {
    __DUMMY1 = LAST_CPLUS_TREE_CODE,
    
    DFA_ROOT,
    DFA_REF_DFA,
    DFA_REF_DD,
    DFA_LIST_DECL,
    DFA_FUNCTION_DEF,
    DFA_DECLSPEC,
    DFA_SUPERCLASS,
    PACKAGE_BODY,
    ASSIGN_STMT,
    DIV_EXPR,
    ELSIF_CLAUSE,
    EXCEPTION_HANDLER,
    EXCEPTION_DECL,
    DFA_WHERE,
    DFA_CURSOR_DEF,
    DFA_CURSOR_DECL,
    FOR_STMT,
    WHILE_STMT,
    DO_WHILE_STMT,
    CASE_VALUES,
    SWITCH_CLAUSE,
    CATCH,
    TRY_BLOCK,
    DYNAMIC_INIT,
    NAME_CLAUSE,
    CONDITION,
    PM_FIELD_REF,
    ALSHIFT_ASSIGN_EXPR,
    ARSHIFT_ASSIGN_EXPR,
    BIT_AND_ASSIGN_EXPR,
    BIT_IOR_ASSIGN_EXPR,
    BIT_XOR_ASSIGN_EXPR,
    MINUS_ASSIGN_EXPR,
    MULT_ASSIGN_EXPR,
    PLUS_ASSIGN_EXPR,
    RDIV_ASSIGN_EXPR,
    TRUNC_DIV_ASSIGN_EXPR,
    TRUNC_MOD_ASSIGN_EXPR,
    NO_OPERAND,
    CTOR_INIT,
    CASE_LABEL,
    ADDRESS_OF_ELLIPSIS,
    TEMPLATE_FCN,
    TEMPLATE_CLASS,
    TEMPLATE_STRUCT,
    TEMPLATE_UNION,
    TEMPLATE_STATIC_DATA_MBR,
    TEMPLATE_HEADER,
    TEMPLATE_BODY,
    MICROSOFT_TRY,
    USING_DECL,
    FINALLY,
    SYNCHRONIZED,
    EASY_NEW,
    JAVA_VEC_NEW,
    INSTANCEOF,
    INITIALIZATION_BLOCK,
    LRSHIFT_EXPR,
    LRSHIFT_ASSIGN_EXPR,
    TYPE_DECL_MODIFIED,
    CLASS_LITERAL,
    DFA_NULL,
    SUPER,
    IMPLEMENTS,
    PARENS,
    UNARY_PLUS,
    DIMENSIONS,
    ANONYMOUS_CLASS,
    THROW_SPEC,
    THIS,
    TYPESPEC,
    TYPEID,
    PROXY_IMPORT,
    DFA_ASSUME,
    DFA_ASM_PSEUDOFUNCTION,
    JAVA_COMPONENT_REF,
    CROSS_JOIN_EXPR,
    FULL_JOIN_EXPR,
    INNER_JOIN_EXPR,
    LEFT_JOIN_EXPR,
    LJOIN_EXPR,
    RIGHT_JOIN_EXPR,
    RJOIN_EXPR,
    CASE_DEFAULT,
    ERROR_NODE,
    LAST_DFA_TREE_CODE
};

typedef int astNode_id;
class ddElement;
class dfa_decl;
extern int dfa_tree_length[];
extern int dfa_init_tree_length();
extern const char *astNode_code_name(int code);

class astNode {
    int next;  // offset from this to the next / parent
    unsigned int code : 8;  
    unsigned int start : 24; // offset in source 
    unsigned int length : 24; // length in source;
    unsigned int is_last : 1;
    unsigned int is_leaf : 1;
    unsigned int level : 6; // code of type node 

    astNode * get_next_internal() { return (astNode*)((char*)this + next); }
  public:
    int get_node_size();

    static int get_node_size(tree_code code);

    static int length_inited;

    astNode * get_first() { return is_leaf ? NULL : 
			     (astNode*) ((char*)this + get_node_size()); }
								  
    astNode * get_next() { return is_last ? NULL: get_next_internal(); }
    astNode * get_parent();
    astNode * get_prev();
    unsigned int get_code() { return code; }
    void set_code (unsigned int value) { code = value; } 
    unsigned int get_level() { return level; }
    void set_level (unsigned int value) { level = 0x3f & value; } 
    void set_start(int val) { start = val;}
    int get_start() { return start;}
    void set_length(int val) { length = val;}
    int get_length() { return length;}
    void set_parent(astNode*);
    void set_next(astNode *);
    void set_is_last(unsigned int val) { is_last = val; }
    void set_is_leaf(unsigned int val) { is_leaf = val; }
    int get_is_last() { return is_last;}
    int get_is_leaf() {return is_leaf;}
    inline ddElement * get_type();
    inline void set_type(ddElement* tp);
    static int is_typed_expression(int code);
};

class dfa_ref_dd : public astNode { // always leaf
    ddElement * dd;
  public:
    void set_dd (ddElement * val) { dd = val; }
    ddElement * get_dd () { return dd; }
};

class dfa_ref_dfa : public astNode { // always leaf
    int  decl; // offset from this to the dfa_decl
  public:
    void set_decl (dfa_decl * val) { decl = val ? (char*)val - (char*)this :NULL; }
    void set_decl (int val) { decl = val; }
    dfa_decl * get_decl () { return decl ? (dfa_decl*)((char*)this + decl) : NULL; }
    int get_decl_int() {return decl;}
};

class dfa_decl : public dfa_ref_dd {
    unsigned int id : 24;    // offset in source to the decl_id token  
    unsigned int is_def : 1;
    unsigned int reserv : 7;
  public:
    int get_id() { return id; }
    void set_id(int val) { id = val; }
    void set_is_def() {is_def = 1;}
    int get_is_def() {return is_def;}
};

class dfa_integer_cst : public astNode {
    int value;
  public:
    void set_value(int val) { value = val; }
    int get_value () { return value; }
};

class dfa_real_cst : public astNode {
    char   value[sizeof(double)];
  public:
    void set_value(double val);
    double get_value ();
};

#include <dis_iosfwd.h>

class db_buffer;
class smtHeader;

int dfa_print_tree(smtHeader*,astNode * node, ostream& os=cout);
int dfa_print_tree(db_buffer*,smtHeader*);

int tree_get_parent(const symbolPtr& curr, symbolPtr& parent);
int tree_get_next(const symbolPtr& curr, symbolPtr& next);
int tree_get_previous(const symbolPtr& curr, symbolPtr& prev);
int tree_get_first(const symbolPtr& curr, symbolPtr& first);
int symbolPtr_get_code(const symbolPtr& sym);

void astNode::set_type(ddElement* type)
{
  if (is_typed_expression(code))
    ((dfa_ref_dd*)this)->set_dd(type);
}

ddElement* astNode::get_type()
{
  ddElement * retval = NULL;
  if (is_typed_expression(code))
    retval = ((dfa_ref_dd*)this)->get_dd();
  return retval;
}

#endif /* __DFA_H__ */
