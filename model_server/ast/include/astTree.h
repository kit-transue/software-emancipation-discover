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
#ifndef astTree_h
#define astTree_h

#ifndef ISO_CPP_HEADERS
#include <iostream.h>
#include <stddef.h>
#else /* ISO_CPP_HEADERS */
#include <iostream>
using namespace std;
#include <cstddef>
#endif /* ISO_CPP_HEADERS */

//#include "astfunc.h"
#include "astmode.h"
#include "astEnums.h"

/* Indexed by enum tree_code, contains a character which is
   `<' for a comparison expression, `1', for a unary arithmetic
   expression, `2' for a binary arithmetic expression, `e' for
   other types of expressions, `r' for a reference, `c' for a
   and `x' for anything else (TREE_LIST, IDENTIFIER, etc).  */

class astTree;
extern "C" void ast_constructor_hook (astTree*, enum tree_code); 

extern  "C" void ast_dbg_print_tree_to (astTree const*const, ostream&);
#ifdef AST_READONLY
extern "C" int exec_get_dbg_inf (astTree*);
extern "C"void exec_set_dbg_inf (astTree*, int);
#endif /* AST_READONLY */
extern "C" void  ast_set_linenum (astTree*, int);
extern "C" int   ast_get_linenum (astTree*);

/* The definition of tree nodes fills the next several pages.  */

/* A tree node can represent a data type, a variable, an expression
   or a statement.  Each node has a TREE_CODE which says what kind of
   thing it represents.  Some common codes are:
   INTEGER_TYPE -- represents a type of integers.
   ARRAY_TYPE -- represents a type of pointer.
   VAR_DECL -- represents a declared variable.
   INTEGER_CST -- represents a constant integer value.
   PLUS_EXPR -- represents a sum (an expression).

   As for the contents of a tree node: there are some fields
   that all nodes share.  Each TREE_CODE has various special-purpose
   fields as well.  The fields of a node are never accessed directly,
   always through accessor macros.  */

/* This type is used everywhere to refer to a tree node.  */

typedef class astTree *tree;

/* Every kind of tree node starts with this structure,
   so all nodes have these fields.

   See the accessor macros, defined below, for documentation of the fields.  */

class astType;
class smtTree;

class astTree
/*
struct tree_common
*/
{
  public:
  /* !! < */
  int debug_inf;
  astTree *previous;
  /* added for S-mode */
  public:
  smtTree * smtnode;
  int start_line;
  int end_line;
  char *filen;
  /* > !! */
  
  astTree *chain;
  astType *type;

  public:

#ifdef ONLY_INT_FIELDS
  unsigned int code : 8;
#else
  enum tree_code code : 8;
#endif

  unsigned side_effects_flag : 1;
  unsigned constant_flag : 1;
  unsigned permanent_flag : 1;
  unsigned addressable_flag : 1;
  unsigned volatile_flag : 1;
  unsigned readonly_flag : 1;
  unsigned unsigned_flag : 1;
  unsigned asm_written_flag: 1;

  unsigned used_flag : 1;
  unsigned raises_flag : 1;
  unsigned static_flag : 1;
  unsigned public_flag : 1;
  unsigned private_flag : 1;
  unsigned protected_flag : 1;

  unsigned lang_flag_0 : 1;
  unsigned lang_flag_1 : 1;
  unsigned lang_flag_2 : 1;
  unsigned lang_flag_3 : 1;
  unsigned lang_flag_4 : 1;
  unsigned lang_flag_5 : 1;
  unsigned lang_flag_6 : 1;
  /* There is room for two more flags.  */

  /* !! < added for S-mode */
  unsigned smt_mapped : 1;
  /* > !! */
  unsigned aset_flag : 1;
  /* no room for more flags */
  public:
  astTree() {}
  astTree (astTree const &);
  virtual ~astTree() {}
#ifdef  _OBJ_AST_TRAVERSE_ /* NY: keep code as a description of AST traverse */
  void* operator new (size_t size);
  void operator delete(void *ptr);
#endif /* _OBJ_AST_TRAVERSE */

/* Define accessors for the fields that all tree nodes have
   (though some fields are not used for all kinds of nodes).  */
#ifndef AST_READONLY
int TREE_DEBUG () {return debug_inf;}
void set_DEBUG (int dbg) { debug_inf = dbg;}
#else
int TREE_DEBUG () { return exec_get_dbg_inf (this); }
void set_DEBUG (int dbg) { exec_set_dbg_inf (this, dbg); }
#endif /* AST_READONLY */
int TREE_DEBUG_INF () {return debug_inf;}
void set_DEBUG_INF (int dbg) { debug_inf = dbg;}

/* The tree-code says what kind of node it is.
   Codes are defined in tree.def.  */
  enum tree_code TREE_CODE () const {return (enum tree_code) code; }
  void set_TREE_CODE (enum tree_code value) {code = value;}

/* In all nodes that are expressions, this is the data type of the expression.
   In POINTER_TYPE nodes, this is the type that the pointer points to.
   In ARRAY_TYPE nodes, this is the type of the elements.  */
  astType * TREE_TYPE() const { return type; }

/* Nodes are chained together for many purposes.
   Types are chained together to record them for being output to the debugger
   (see the function `chain_type').
   Decls in the same scope are chained together to record the contents
   of the scope.
   Statement nodes for successive statements used to be chained together.
   Often lists of things are represented by TREE_LIST nodes that
   are chained together.  */

  astTree * TREE_CHAIN () const { return chain; }
  void set_TREE_CHAIN (tree next) {chain = next;}

  astTree * TREE_PREV () const {return previous;}
  void set_TREE_PREV (tree prev) {previous = prev;}

void  printTree (ostream& str = cout) const ;

void send_string (ostream& str) const {ast_dbg_print_tree_to (this, str); }

void print(ostream& str = cout) const;

};

#define TREE_LANG_FLAG_2(NODE) ((NODE)->lang_flag_2)

#define TREE_SMT_NODE(NODE) ((NODE)->smtnode)

#define TREE_START_LINE(NODE) ((NODE)->start_line)
#define TREE_END_LINE(NODE) ((NODE)->end_line)
#define TREE_FILENAME(NODE) ((NODE)->filen)
#define TREE_SMT_MAPPED(NODE) ((NODE)->smt_mapped)
#define TREE_ASET_FLAG(NODE) ((NODE)->aset_flag)

/* Nonzero if TYPE represents an aggregate (multi-component) type. */
 
#define AGGREGATE_TYPE_P(TYPE) \
  (TYPE->TREE_CODE () == ARRAY_TYPE || TYPE->TREE_CODE () == MC_RECORD_TYPE \
   || TYPE->TREE_CODE () == MC_UNION_TYPE || TYPE->TREE_CODE () == MC_QUAL_UNION_TYPE)


/* Define many boolean fields that all tree nodes have.  */

/* In a VAR_DECL, nonzero means allocate static storage.
   In a FUNCTION_DECL, nonzero if function has been defined.
   In a CONSTRUCTOR, nonzero means allocate static storage.  */
#define TREE_STATIC(NODE) ((NODE)->static_flag)

/* Nonzero for a TREE_LIST or TREE_VEC node means that the derivation
   chain is via a `virtual' declaration.  */
#define TREE_VIA_VIRTUAL(NODE) ((NODE)->static_flag)

/* In a VAR_DECL or FUNCTION_DECL,
   nonzero means name is to be accessible from outside this module.
   In an identifier node, nonzero means a external declaration
   accesible from outside this module was previously seen
   for this name in an inner scope.  */
#define TREE_PUBLIC(NODE) ((NODE)->public_flag)

/* Nonzero for TREE_LIST or TREE_VEC node means that the path to the
   base class is via a `public' declaration, which preserves public
   fields from the base class as public.  */
#define TREE_VIA_PUBLIC(NODE) ((NODE)->public_flag)

/* In a VAR_DECL, PARM_DECL or FIELD_DECL, or any kind of ..._REF node,
   nonzero means it may not be the lhs of an assignment.
   In a ..._TYPE node, means this type is const-qualified
   (but the macro TYPE_READONLY should be used instead of this macro
   when the node is a type).  */
#define TREE_READONLY(NODE) ((NODE)->/* !! <common.> !! */readonly_flag)
  
/* Value of expression is constant.
   Always appears in all ..._CST nodes.
   May also appear in an arithmetic expression, an ADDR_EXPR or a CONSTRUCTOR
   if the value is constant.  */
#define TREE_CONSTANT(NODE) ((NODE)->/* <common.> !! */constant_flag)

/* In INTEGER_TYPE or ENUMERAL_TYPE nodes, means an unsigned type.
   In FIELD_DECL nodes, means an unsigned bit field.
   The same bit is used in functions as DECL_BUILT_IN_NONANSI.  */
#define TREE_UNSIGNED(NODE) ((NODE)->/* !! <common.> !! */unsigned_flag)

/* Nonzero in a VAR_DECL means assembler code has been written.
   Nonzero in a FUNCTION_DECL means that the function has been compiled.
   This is interesting in an inline function, since it might not need
   to be compiled separately.
   Nonzero in a RECORD_TYPE, UNION_TYPE, QUAL_UNION_TYPE or ENUMERAL_TYPE
   if the sdb debugging info for the type has been written.
   In a BLOCK node, nonzero if reorder_blocks has already seen this block.  */
#define TREE_ASM_WRITTEN(NODE) ((NODE)->/* !! <common.> !! */asm_written_flag)

/* Used in classes in C++.  */
#define TREE_PRIVATE(NODE) ((NODE)->private_flag)
/* Used in classes in C++.
   In a BLOCK node, this is BLOCK_HANDLER_BLOCK.  */
#define TREE_PROTECTED(NODE) ((NODE)->protected_flag)

/* Define additional fields and accessors for nodes representing constants.  */

class astConst : public astTree 
{
  public: 
  astConst() { }
  astConst (astConst const & nd);

  void printConst (ostream& str);
};

/* In an INTEGER_CST node.  These two together make a 2-word integer.
   If the data type is signed, the value is sign-extended to 2 words
   even though not all of them may really be in use.
   In an unsigned constant shorter than 2 words, the extra bits are 0.  */
#define TREE_INT_CST_LOW(NODE) ((NODE)->int_cst_low)
#define TREE_INT_CST_HIGH(NODE) ((NODE)->int_cst_high)

class astInt_cst : public astConst
{
 public:
  HOST_WIDE_INT int_cst_low;
  HOST_WIDE_INT int_cst_high;

  public:

  astInt_cst() { }
  astInt_cst (astInt_cst const & nd);

void  printInt (ostream& str);
};

/* In REAL_CST, STRING_CST, COMPLEX_CST nodes, and CONSTRUCTOR nodes,
   and generally in all kinds of constants that could
   be given labels (rather than being immediate).  */

/* In a REAL_CST node.  */
/* We can represent a real value as either a `double' or a string.
   Strings don't allow for any optimization, but they do allow
   for cross-compilation.  */
#define REAL_VALUE_TYPE double  // XXX: what are the implications of this?

class astReal_cst : public astConst
{
  struct rtx_def *rtl;	/* acts as link to register transfer language
				   (rtl) info */
  REAL_VALUE_TYPE real_cst;

  public:

  astReal_cst() { }
  astReal_cst (astReal_cst const & nd);

  REAL_VALUE_TYPE TREE_REAL_CST () const { return real_cst;}
  void set_TREE_REAL_CST (REAL_VALUE_TYPE r) { real_cst = r;}
void   printReal (ostream& str);
};

/* In a STRING_CST */
class astString_cst : public astConst
{
  struct rtx_def *rtl;	/* acts as link to register transfer language
				   (rtl) info */
  int length;
  char *pointer; /*! genString pointer; !*/
  public:

  astString_cst() { }
  astString_cst (astString_cst const & nd);

  int TREE_STRING_LENGTH () const { return length; }
  void set_TREE_STRING_LENGTH (int l) {length = l;}
  char * TREE_STRING_POINTER() const{ return pointer;}
  void TREE_STRING_POINTER(char * p) {pointer = p;}

void   printString (ostream& str);
};


/* In a COMPLEX_CST node.  */  
class astComplex : public astConst
{
  struct rtx_def *rtl;	/* acts as link to register transfer language
				   (rtl) info */
  astReal_cst *real;
  astReal_cst *imag;
  astReal_cst* TREE_REALPART () const {return real;}
  astReal_cst* TREE_IMAGPART () const {return imag;}
  public:

  astComplex() { }
  astComplex (astComplex const & nd);
};


/* Define fields and accessors for some special-purpose tree nodes.  */

struct lang_id2
{
  astTree* label_value;
  astTree* implicit_decl;

  astTree* type_desc;
  astTree* as_list;

  astTree* error_locus;
};
extern "C" struct lang_id2* ast_dbnew_lang_id2 ();

struct astIdentifier : public astTree 
{
  int length;
  char *pointer;
  astTree* global_value;
  astTree* local_value;
  astTree* class_value;

  astTree* class_template_info;

  struct lang_id2 *x;
  public:

astIdentifier()  {}
astIdentifier(astIdentifier const &  nd);

  int IDENTIFIER_LENGTH () const { return length;}
  char *IDENTIFIER_POINTER() const { return pointer;}
void printIdentifier (ostream& str);

astTree* IDENTIFIER_GLOBAL_VALUE() const {return global_value;}

astTree* IDENTIFIER_CLASS_VALUE() const {return class_value;}	

astTree* IDENTIFIER_LOCAL_VALUE() const {return local_value;}

astTree* IDENTIFIER_LABEL_VALUE() { return (x ? x->label_value : 0);}
astTree* IDENTIFIER_TEMPLATE() {return class_template_info;}
/* Nonzero if this identifier is the prefix for a mangled C++ operator name.  */
#define IDENTIFIER_OPNAME_P(NODE) TREE_LANG_FLAG_2(NODE)

};


/* In a TREE_LIST node.  */

class astList : public astTree
{
  astTree *purpose;
  astTree *value;
  public:

  astList() { }
  astList (astList const & nd) ;

  astTree* TREE_PURPOSE() const { return purpose;}
  void set_TREE_PURPOSE (astTree* p) {purpose = p;}
  astTree* TREE_VALUE() const { return value;}
  void set_TREE_VALUE (astTree* v) {value = v;}

  astList* get_next_ast () {return (astList*) TREE_CHAIN ();}

  void printList (ostream& str); 
};

/* In a TREE_VEC node.  */

class astVec : public astTree
{
  int length;
  astTree *a[19];
  public:

  astVec() { }
  astVec (astVec const & nd) ;

  int TREE_VEC_LENGTH() { return length;}
  astTree* TREE_VEC_ELT(int i) {return a[i];}
  astTree** TREE_VEC_END() {return &(a[length]);} 
};


/* Define fields and accessors for some nodes that represent expressions.  */

/* In ordinary expression nodes.  */

class astExp : public astTree
{
#if 0 // XXX: unused
  protected:
  int complexity;
#endif
  public:

  astExp() { }
  astExp (astExp const & nd);

  virtual astTree* TREE_OPERAND(int I) = 0;
  virtual void set_TREE_OPERAND (astTree* op, int i) = 0;

#if 0 // XXX: unused
  int TREE_COMPLEXITY() { return complexity;}
#endif

  void  printExp (ostream& str = cout);
};


class astExp1 : public astExp
{
  astTree * operands[1];
  public:

  astExp1() { };
  astExp1 (astExp1 const & nd);

  astTree* TREE_OPERAND(int I) { return operands[I]; }
  void set_TREE_OPERAND (astTree* op, int i) {operands [i] = op;}

};


class astExp2 : public astExp
{
  astTree * operands[2];
  public:

  astExp2() { };
  astExp2 (astExp2 const & nd);

  astTree* TREE_OPERAND(int I) { return operands[I]; }
  void set_TREE_OPERAND (astTree* op, int i) {operands [i] = op;}
/* In a CONSTRUCTOR node.  */
  astTree* CONSTRUCTOR_ELTS() { return operands[1];}

};

class astExp3 : public astExp
{
  astTree * operands[3];
  public:

  astExp3() { };
  astExp3 (astExp3 const & nd);

  astTree* TREE_OPERAND(int I) { return operands[I]; }
  void set_TREE_OPERAND (astTree* op, int i) {operands [i] = op;}
};

class astExp4 : public astExp
{
  astTree * operands[4];
  public:

  astExp4() { };
  astExp4 (astExp4 const & nd);

  astTree* TREE_OPERAND(int I) { return operands[I]; }
  void set_TREE_OPERAND (astTree* op, int i) {operands [i] = op;}
};


/* Define fields and accessors for nodes representing declared names.  */

class astBlock;


#define DECL_LANG_FLAG_2(NODE) (((astDecl *)NODE)->lang_flag_2)

#define DECL_CONSTRUCTOR(NODE) ((NODE)->constructor)

#define DECL_LANG_SPECIFIC(NODE) ((NODE)->lang_specific)
#define DECL_VINDEX(NODE) ((NODE)->decl.vindex)

/* In a VAR_DECL or FUNCTION_DECL,
   nonzero means external reference:
   do not allocate storage, and refer to a definition elsewhere.  */
#define DECL_EXTERNAL(NODE) ((NODE)->external_flag)

/* Nonzero in a FUNCTION_DECL means this function can be substituted
   where it is called.  */
#define DECL_INLINE(NODE) ((NODE)->inline_flag)

/* In FUNCTION_DECL, holds the decl for the return value.  */
#define DECL_RESULT(NODE) ((NODE)->result)
// XXX: wouldn't it be nice if this were a virtual function with covarient
// return type?

class astDecl : public astTree
{
  public:

  char *filename;
  int linenum;
  astTree *size;
  unsigned int uid;
#ifdef ONLY_INT_FIELDS
  int mode : 8;
#else
  enum machine_mode mode : 8;
#endif

  unsigned external_flag : 1;
  unsigned nonlocal_flag : 1;
  unsigned regdecl_flag : 1;
  unsigned inline_flag : 1;
  unsigned bit_field_flag : 1;
  unsigned virtual_flag : 1;
  unsigned ignored_flag : 1;
  unsigned abstract_flag : 1;

  unsigned in_system_header_flag : 1;
  unsigned common_flag : 1;
  unsigned defer_output : 1;
  unsigned transparent_union : 1;
  unsigned static_ctor_flag : 1;
  unsigned static_dtor_flag : 1;
  unsigned artificial_flag : 1;
  unsigned weak_flag : 1;
  /* room for no more */

  unsigned lang_flag_0 : 1;
  unsigned lang_flag_1 : 1;
  unsigned lang_flag_2 : 1;
  unsigned lang_flag_3 : 1;
  unsigned lang_flag_4 : 1;
  unsigned lang_flag_5 : 1;
  unsigned lang_flag_6 : 1;
  unsigned lang_flag_7 : 1;

  astIdentifier *name;
  astBlock *context;
  astDecl *arguments;
  astTree *result;
  astTree *initial;
  astTree *abstract_origin;
  astIdentifier *assembler_name;
  astTree *section_name;
  astTree *machine_attributes;
  struct rtx_def *rtl;	/* acts as link to register transfer language
				   (rtl) info */

  /* For a FUNCTION_DECL, if inline, this is the size of frame needed.
     If built-in, this is the code for which built-in function.
     For other kinds of decls, this is DECL_ALIGN.  */
  union {
    int i;
    unsigned int u;
    enum built_in_function f;
  } frame_size;

  astTree *vindex;

  /* Points to a structure whose details depend on the language in use.  */
  struct lang_decl *lang_specific;

  astTree *constructor;

  public:

  astDecl() {}
  astDecl (astDecl const & nd) ;

  enum built_in_function DECL_FUNCTION_CODE() {return (enum built_in_function) frame_size.f;}

  astIdentifier* DECL_NAME() const {return name;}
  void set_DECL_NAME (astIdentifier* idname) {name = idname; }
  astBlock* DECL_CONTEXT () const {return context;}
  void set_DECL_CONTEXT (astBlock* block) {context = block;}
  astTree* DECL_FIELD_CONTEXT() {return (astTree*) context;}
  astDecl* DECL_ARGUMENTS() {return arguments;}  /* In FUNCTION_DECL.  */
  void set_DECL_ARGUMENTS (astDecl* arg) {arguments = arg;}
  astTree* DECL_ARG_TYPE() {return initial;}     /* In PARM_DECL.  */
  void set_DECL_ARG_TYPE (astTree* init) {initial = init;}
  astTree* DECL_INITIAL() const {return initial;}
  void set_DECL_INITIAL (astTree* init) {initial = init;}
  char* DECL_SOURCE_FILE() {return filename;}
  void SET_DECL_SOURCE_FILE(char * p) {filename = p;}
  int DECL_SOURCE_LINE() {return linenum;}
  void SET_DECL_SOURCE_LINE(int l) {linenum = l;}
  astTree* DECL_SIZE() const { return size;}

  astIdentifier* DECL_ASSEMBLER_NAME() const {return assembler_name;}

  astIdentifier * ID_NODE () const ;

  /*int  DECL_STACK_OFFSET() { return offset_frame; }*/
  void printDecl (ostream& str);
  void printDeclaration (ostream& str) const;

    char* get_name () const { return DECL_NAME ()->IDENTIFIER_POINTER();}
  void set_linenum (int line) { 
#ifndef IS_MIXED
    ast_set_linenum (this, line); 
#endif
}
  int  get_linenum () { 
#ifndef IS_MIXED
    return ast_get_linenum (this); 
#else
    return DECL_SOURCE_LINE ();
#endif
}
};

/* Define fields and accessors for nodes representing data types.  */

/* See tree.def for documentation of the use of these fields.
   Look at the documentation of the various ..._TYPE tree codes.  */

#define TYPE_LANG_SPECIFIC(NODE) ((NODE)->lang_specific)
#define BINFO_BASETYPES(NODE) ((astVec*)NODE)->TREE_VEC_ELT (4)
#define TYPE_BINFO_BASETYPES(NODE) ((astVec*)((astType*)NODE)->TYPE_BINFO())->TREE_VEC_ELT(4)
#define TYPE_METHOD_BASETYPE(NODE) (((astType *)NODE)->maxval)
#define TYPE_OFFSET_BASETYPE(NODE) (((astType *)NODE)->maxval)

/* Nonzero for _TYPE node means that this type is a pointer to member
   function type. */
#define TYPE_PTRMEMFUNC_P(NODE) (NODE->TREE_CODE() == MC_RECORD_TYPE && TYPE_LANG_SPECIFIC(NODE) && TYPE_LANG_SPECIFIC(NODE)->type_flags.ptrmemfunc_flag)

/* Get the POINTER_TYPE to the METHOD_TYPE associated with this
   pointer to member function.  TYPE_PTRMEMFUNC_P _must_ be true,
   before using this macro. */
#define TYPE_PTRMEMFUNC_FN_TYPE(NODE) (NODE->TYPE_FIELDS()->TREE_CHAIN()->TREE_CHAIN()->TREE_TYPE()->TYPE_FIELDS()->TREE_TYPE())


#define TYPE_MIN_VALUE(NODE) ((NODE)->minval)
#define TYPE_MAX_VALUE(NODE) ((NODE)->maxval)
#define TYPE_STUB_DECL(NODE) ((NODE)->chain)

class astType : public astTree
{
  public:
  astTree *values;
  astTree *size;
  astTree *attributes;
  unsigned uid;

  unsigned char precision;
#ifdef ONLY_INT_FIELDS
  int  mode : 8;
#else
  enum machine_mode mode : 8;
#endif

  unsigned string_flag : 1;
  unsigned no_force_blk_flag : 1;
  unsigned needs_constructing_flag : 1;
  unsigned transparent_union_flag : 1;
  unsigned packed_flag : 1;
  unsigned lang_flag_0 : 1;
  unsigned lang_flag_1 : 1;
  unsigned lang_flag_2 : 1;
  unsigned lang_flag_3 : 1;
  unsigned lang_flag_4 : 1;
  unsigned lang_flag_5 : 1;
  unsigned lang_flag_6 : 1;
  /* room for 4 more bits */

  unsigned int align;
  astTree *pointer_to;
  astTree *reference_to;
  int COMPAT_parse_info;  /* compat.: required for gcc 2.6.3 or before */

  union {int address; char *pointer; } symtab;
  astTree *name;

  astTree *minval;
  astTree *maxval;

  astType *next_variant;
  astType *main_variant;
  astTree *binfo;
  astTree *noncopied_parts;

  astTree *context;
  struct obstack *obstack;

  /* Points to a structure whose details depend on the language in use.  */
  struct lang_type *lang_specific;

  astTree *noncopied_parts_c;

  public:

  astType() { }
  astType (astType const & nd);

  tree TYPE_SIZE() { return size;}
  unsigned char TYPE_ALIGN()  { return align; }
  astTree* TYPE_VALUES() { return values;}
  astTree* TYPE_DOMAIN() { return values;}
  astTree* TYPE_FIELDS() { return values;}
  astTree* TYPE_METHODS() { return maxval;}
  astTree* TYPE_ARG_TYPES() { return values;}
  void set_TYPE_ARG_TYPES (tree arg_type) {values = arg_type;}
  astTree* TYPE_POINTER_TO() { return pointer_to;}
  astTree* TYPE_REFERENCE_TO() { return reference_to;}
#if 0
  astTree* TYPE_MIN_VALUE() { return minval;}
  astTree* TYPE_MAX_VALUE() { return maxval;}
#endif /* 0 */
  unsigned char TYPE_PRECISION() { return precision;}

  int TYPE_SYMTAB_ADDRESS() { return symtab.address;}
  astTree* TYPE_NAME() { return name;}
  astType* TYPE_NEXT_VARIANT() { return next_variant; }
  astType* TYPE_MAIN_VARIANT() { return main_variant;}
  astTree* TYPE_BASETYPES() { return binfo;}
  astTree* TYPE_BINFO() { return binfo;}
  astTree* TYPE_NONCOPIED_PARTS() { return noncopied_parts;}

  /*  to make error reporting functions' lives easier.  */
char*TYPE_NAME_STRING(){return((astDecl*)TYPE_NAME())->DECL_NAME()->IDENTIFIER_POINTER();}
int TYPE_NAME_LENGTH() {return((astDecl*)TYPE_NAME())->DECL_NAME()->IDENTIFIER_LENGTH();}

/* In a *_TYPE, nonzero means a built-in type.  */
unsigned int TYPE_BUILT_IN() {return lang_flag_6;}

void  printType (ostream& str);
};

inline   astIdentifier * astDecl::ID_NODE () const 
{ 
  if (name) return name;
  else
    if ((TREE_CODE () == MC_TYPE_DECL) && type &&
	type->TYPE_NAME() && (type->TYPE_NAME()->TREE_CODE() == MC_IDENTIFIER_NODE))
      return (astIdentifier*) type->TYPE_NAME();
    else return 0;
}

class astFunction_decl : public astDecl
{
/*  struct tree_decl ignore; */

  public:

  astFunction_decl() { }
  astFunction_decl (astFunction_decl const & nd);

//  char* DECL_PRINT_NAME() {return print_name;}
//  astTree* DECL_RESULT() {return result;}

void  printFunction (ostream& str);
};


/* Define fields and accessors for nodes representing statements.
   These are now obsolete for C, except for LET_STMT, which is used
   to record the structure of binding contours (and the names declared
   in each contour) for the sake of outputting debugging info.
   Perhaps they will be used once again for other languages.  */

/* For LABEL_STMT, GOTO_STMT, RETURN_STMT, COMPOUND_STMT, ASM_STMT.  */


class astStmts : public astTree
{
  protected:
  char *filename;
  int linenum;
  astBlock *context;
  public:

  astStmts() { }
  astStmts (astStmts const & nd);

  int STMT_SOURCE_LINE() {return linenum;}
  void SET_STMT_SOURCE_LINE(int l) {linenum=l;}
  char* STMT_SOURCE_FILE() {return filename;}
  astBlock* STMT_CONTEXT() const {return context;}
  void set_STMT_CONTEXT (astBlock* block) {context = block;}
  astStmts* get_next_stmt () {return (astStmts*) TREE_CHAIN ();}
  void   printStmts (ostream& str);
};

class astStmt : public astStmts
{
  protected:
  astTree *body;
  public:

  astStmt() { }
  astStmt (astStmt const & nd);

  astTree* STMT_BODY() const {return body;}
  void set_STMT_BODY(astTree* t) {body = t;}
};

/* For IF_STMT.  */
class astIf : public astStmts
{
  astExp *cond;
  astStmts *thenpart;
  astStmts *elsepart;
  public:

  astIf() { }
  astIf (astIf const & nd);

  astExp *STMT_COND() const { return cond;}
  astStmts* STMT_THEN() { return thenpart;}
  astStmts* STMT_ELSE() { return elsepart;}
  void set_STMT_COND (astExp* exp) {cond = exp;}
  void set_STMT_THEN (astStmts* stmt) {thenpart = stmt;}
  void set_STMT_ELSE (astStmts* stmt) {elsepart = stmt;}
void   printIf (ostream& str);
};

/* For LOOP_STMT.  */
class astLoop : public astStmts
{
  astList *vars;
  astExp  *cond;
  astStmts*body;
  public:

  astLoop() { }
  astLoop (astLoop const & nd);

  astList* STMT_LOOP_VARS() const { return vars;}
  void set_STMT_LOOP_VARS(astList* var) { vars = var;}
  astExp * STMT_LOOP_COND() const { return cond;}
  void set_STMT_LOOP_COND(astExp * expr) { cond = expr;}
  astStmts* STMT_LOOP_BODY() const { return body;}
  void set_STMT_LOOP_BODY (astStmts* newbody) {body = newbody;}
  astStmts* get_STMT_LOOP_INIT () 
    { if (vars) 
	return (astStmts*) vars->TREE_VALUE ();
    else return (astStmts*) NULL; }
  astStmts* get_STMT_LOOP_DELTA () 
    { if (vars)
	return (astStmts*) vars->TREE_PURPOSE ();
    else return (astStmts*) NULL; }
  void set_STMT_LOOP_INIT (astStmts* stmt) 
    { if (vars) 
	vars->set_TREE_VALUE (stmt);
    else; }
  void set_STMT_LOOP_DELTA (astStmts* stmt) 
    { if (vars)
        vars->set_TREE_PURPOSE (stmt);
    else ; }
  int IS_DO_WHILE () {
    astStmts* init = get_STMT_LOOP_INIT ();
    if (init && (init == STMT_LOOP_BODY ()))
      return 1;
    else return 0;
  }
void  printLoop (ostream& str);
};

/* For LET_STMT and WITH_STMT.  */

/* #define STMT_BODY(NODE) */

class astBlock : public astStmt
{
  astDecl *vars;
  astBlock *supercontext;
  astTree *bind_size;
  astTree *type_tags;
  astBlock *subblocks;

   public:

  astBlock() { };
  astBlock (astBlock const & nd);

  astDecl* STMT_VARS() const { return vars;}
  astBlock* STMT_SUPERCONTEXT() { return supercontext;}
  astTree* STMT_BIND_SIZE() { return bind_size;}
  astTree* STMT_TYPE_TAGS() { return type_tags;}
  astBlock* STMT_SUBBLOCKS() { return subblocks;}
  void set_STMT_SUBBLOCKS (astBlock* block) {subblocks = block;}
  void set_STMT_VARS (astDecl* var) {vars = var;}
  void set_STMT_SUPERCONTEXT (astBlock* block) {supercontext = block;}

astStmts * get_first_stmt () {return (astStmts*) body;}
void  printBlock (ostream& str);
};

/* For CASE_STMT.  */
class astCase : public astStmts
{
  astTree *index;
  astStmts*case_list;
  public:

  astCase() { };
  astCase (astCase const & nd);

  astTree* STMT_CASE_INDEX() const { return index;}
  astStmts* STMT_CASE_LIST() const { return case_list;}
  void set_STMT_CASE_LIST(astStmts* st) { case_list = st;}
void printCase (ostream& str) ;
};


/* Return the (unique) IDENTIFIER_NODE node for a given name.
   The name is supplied as a char *.  */
extern "C" astIdentifier *get_identifier (char*);
/*extern "C" tree lookup_name (tree, int);*/

extern "C" void debug_tree (tree);

/* Points to the FUNCTION_DECL of the function whose body we are reading. */
extern "C" astFunction_decl* current_function_decl; 

extern "C" astExp* build_exp_from_str (char *); 

/****** build node in given context ******************************/
extern "C" astTree* build_node_in_context (char*, astBlock*,astFunction_decl*);

/********  get If then body *******/
astStmts* getIf_then_body (astIf* stmt);

/********  get If else body *******/
astStmts* getIf_else_body (astIf* stmt);

typedef astTree* astTreePtr;
typedef astConst* astConstPtr;
typedef astInt_cst* astInt_cstPtr;
typedef astReal_cst* astReal_cstPtr;
typedef astString_cst* astString_cstPtr;
typedef astComplex* astComplexPtr;
typedef astIdentifier* astIdentifierPtr;
typedef astList* astListPtr;
typedef astVec* astVecPtr;
typedef astExp* astExpPtr;
typedef astExp1* astExp1Ptr;
typedef astExp2* astExp2Ptr;
typedef astExp3* astExp3Ptr;
typedef astExp4* astExp4Ptr;
typedef astDecl* astDeclPtr;
typedef astType* astTypePtr;
typedef astFunction_decl* astFunction_declPtr;
typedef astStmts* astStmtsPtr;
typedef astStmt* astStmtPtr;
typedef astIf* astIfPtr;
typedef astLoop* astLoopPtr;
typedef astBlock* astBlockPtr;
typedef astCase* astCasePtr;

struct astInserter {
  
  int type;  
  astTree *src_obj;      // object to insert; may be NULL;
  astTree *targ_obj;     // object to insert src_obj as first/after/replace
  char * data;            // usually a string, e.g "i=j+1"
  };
 
//extern "C" astTree * get_real_typedecl (astTree *);

#endif /* astTree_h */

#define NULL_TREE (tree) NULL

/* Define a generic NULL if one hasn't already been defined.  */

#ifndef NULL
#define NULL 0
#endif

