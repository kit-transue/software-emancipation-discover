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
#include <fstream.h>
#else /* ISO_CPP_HEADERS */
#include <fstream>
using namespace std;
#endif /* ISO_CPP_HEADERS */

#include "_Xref.h"
#include "api_internal.h"
#include "cLibraryFunctions.h"
#include "charstream.h"
#include "cmd.h"
#include "ddKind.h"
#include "ddict.h"
#include "dfa.h"
#include "dfa_db.h"
#include "dfa_graph.h"
#include "genError.h"
#include "genString.h"
#include "general.h"
#include "machdep.h"
#include "msg.h"
#include "objArr.h"
#include "proj.h"
#include "smtRegion.h"
#include "xref_queries.h"
#include "xxinterface.h"

#define DFA_QUERY_INCREMENT 10

extern objSet *ast_codes;
extern objSet *ast_categories;
extern bool hashArrInit;

int dfa_fix_code_name();

class codeAttribute : public intAttribute {

public:

  codeAttribute(char const *name, int k);
  virtual int test(symbolPtr &sym) const;
  virtual void print(ostream & = cout, int level = 0) const;

  int code;

  define_relational(codeAttribute, intAttribute);

};

int
codeAttribute_get_code(Relational *rel)
{
  Initialize(codeAttribute_get_code(Relation *));

  return 
    ((codeAttribute *) rel)->code;
}

RelClass(codeAttribute);

codeAttribute::codeAttribute(char const *name,
			     int k)
  : intAttribute(name), code(k) 
{
  Initialize(codeAttribute::codeAttribute(char const *, int));

  if (!ast_codes)
    ast_codes = new objSet;

  ast_codes->insert(this);
}

init_relational(codeAttribute, intAttribute);

void 
codeAttribute::print(ostream &str, 
		     int) const
{
  Initialize(codeAttribute::print(ostream &, int) const);

  str << get_name() << ' ' << astNode_code_name(code);
}

int 
symbolPtr_get_code(const symbolPtr &sym)
{
  Initialize(symbolPtr_get_code(const symbolPtr &));
  int retval = -1;

  if (sym.is_ast()) {
    astNode *node = astNode_of_sym(sym);

    if (node)
      retval = node->get_code();
  }

  return retval;
}

int 
codeAttribute::test(symbolPtr &sym) const
{
  Initialize(codeAttribute::test(symbolPtr &) const);

  return 
    code == symbolPtr_get_code(sym);
}

char const * ATT_code(symbolPtr &sym)
{
  Initialize(ATT_code(symbolPtr &));
  int code = symbolPtr_get_code(sym);

  return 
    astNode_code_name(code);
}

int 
ATT_ast_offset(symbolPtr &sym)
{
  Initialize(ATT_ast_offset(symbolPtr &));

  if (!sym.is_ast())
    return 0;

  int off = 0;
  astNode *node = astNode_of_sym(sym);

  if (node)
    off = node->get_start();

  return off;
}

int 
ATT_ast_token(symbolPtr &sym)
{
  Initialize(ATT_ast_token(symbolPtr &));

  if (!sym.is_ast())
    return 0;

  int is_token = 0;
  astNode *node = astNode_of_sym(sym);

  if (node)
    is_token = !(node->get_first());

  return is_token;
}

int 
ATT_ast_length(symbolPtr &sym)
{
  Initialize(ATT_ast_length(symbolPtr &));

  if (!sym.is_ast())
    return 0;

  int off = 0;
  astNode *node = astNode_of_sym(sym);

  if (node)
    off = node->get_length();

  return off;
}

extern projModule *symbol_get_module(symbolPtr &sym);

int 
ATT_is_loaded(symbolPtr &sym)
{
  Initialize(ATT_is_loaded(symbolPtr &));
  projModule *mod = symbol_get_module(sym);

  return 
    mod ? mod->is_loaded() : 0;
}

char const *ATT_ast_info(symbolPtr &sym);
char const *ATT_ast_text(symbolPtr &sym);
char const *ATT_ast_spaces_before(symbolPtr &sym);
char const *ATT_ast_spaces_after(symbolPtr &sym);

int 
ATT_level(symbolPtr &sym)
{
  Initialize(ATT_level(symbolPtr &));
  int level = 0;

  if (sym.is_dataCell()) {
    dataCell *cell =  dataCell_of_symbol(sym, false);

    if (cell) {
      dataCell *field = cell;
      dataCell *temp = cell;

      while (temp = temp->getField())
	field = temp;
	
      level = field->get_level();
      dataCell_offload(cell, false);
    }
  } else {
    symbolPtr symbol = sym.get_xrefSymbol();

    if (symbol.isnotnull()) {
      symbolArr types;
      symbol.get_local_link(has_type, types);
      int size = 0;

      while (types.size() > size) {
	size = types.size();
	symbol = types[size - 1];

	if (symbol->get_attribute(POINTER, 1) ||
	    symbol->get_attribute(REF, 1))
	  ++level;

	symbol.get_local_link(has_type, types);
      }
    }
  }

  return level;
}

int
ATT_argno(symbolPtr &symbol)
{
  Initialize(ATT_argno(symbolPtr &));
  int argno = -1;

  if (symbol.is_dataCell()) {
    dataCell *cell =  dataCell_of_symbol(symbol, false);

    if (cell) {
      argno = cell->get_argno();
      dataCell_offload(cell, false);
    }
  }

  return argno;
}

int
ATT_global(symbolPtr &symbol)
{
  Initialize(ATT_global(symbolPtr &));
  int retval = 0;

  if (symbol.is_xrefSymbol()) {
    if (symbol.get_kind() == DD_LOCAL)
      retval = 0;
    else
      retval = 1;
  } else if (symbol.is_dataCell()) {
    dataCell *cell = dataCell_of_symbol(symbol, false);

    if (cell) {
      if (!cell->isAbstract())
	retval = 0;
      else if (!cell->isNodeDataCell())
	retval = 1;
      else if (cell->isAbstract() &&
	       !(cell->get_type() & dataCell_Regular))
	retval = 2;

      dataCell_offload(cell, false);
    } else
      retval = 0;
  }

  return retval;
}

int
ATT_depth(symbolPtr &symbol)
{
  Initialize(ATT_depth(symbolPtr &));
  int retval = 0;

  if (symbol.is_dataCell()) {
    dataCell *cell = dataCell_of_symbol(symbol, false);

    if (cell) {
      retval = cell->get_depth();
      dataCell_offload(cell, false);
    }
  }

  return retval;
}

static int 
codeAttribute_init()
{
  Initialize(codeAttribute_init());

  dfa_fix_code_name();
  symbolAttribute::api_init_attrs();

  for (int ii = 0; ii < LAST_DFA_TREE_CODE; ++ii)
    new codeAttribute(astNode_code_name(ii), ii);

  new codeAttribute("namespace_def", MC_NAMESPACE_DECL);
  new funcIntAttribute("is_loaded", ATT_is_loaded);
  new funcStringAttribute("ast_info", ATT_ast_info);
  new funcStringAttribute("ast_text", ATT_ast_text);
  new funcStringAttribute("ast_spaces_before", ATT_ast_spaces_before);
  new funcStringAttribute("ast_spaces_after", ATT_ast_spaces_after);
  new funcStringAttribute("ast_code", ATT_code);
  new funcIntAttribute("ast_token", ATT_ast_token);
  new funcIntAttribute("ast_offset", ATT_ast_offset);
  new funcIntAttribute("ast_length", ATT_ast_length);
  new funcIntAttribute("level", ATT_level);
  new funcIntAttribute("argno", ATT_argno);
  new funcIntAttribute("global", ATT_global);
  new funcIntAttribute("depth", ATT_depth);

  return 0;
}

char const *ATT_name(symbolPtr &sym);
char const *ATT_cname(symbolPtr &sym);
char const *ATT_filename(symbolPtr &sym);

int tree_get_first(const symbolPtr &curr, symbolPtr &next);
int tree_get_next(const symbolPtr &curr, symbolPtr &next);

static char const *level_indent = "   ";
static int 
dfa_print_pattern(ostream &os, 
		  symbolPtr &sym, 
		  int name_opt, 
		  int truncate, 
		  int reg_st, 
		  int reg_en,
		  int multiline,
		  int level)
{
  Initialize(dfa_print_pattern(ostream &, symbolPtr &, int, int, int, int, int, int));

  if (sym.get_kind() != DD_AST)
    return -1;

  symbolPtr child;
  int childish = tree_get_first(sym, child);

  if (childish) {
    if(multiline)for(int iii=0; iii<level;++iii) os << level_indent;
    int code = symbolPtr_get_code(sym);
    os << astNode_code_name(code);
  } else {
    bool skip = false;

    if (truncate) {
      astNode *node = astNode_of_sym(sym);
      int node_st = node->get_start();
      int node_en = node_st + node->get_length();

      if (node_st > reg_en || node_en < reg_st)
	skip = true;
    }

    if (!skip) {
      if (name_opt == 1) {
	char const *name = ATT_cname(sym);

	if (name && name[0])
	  os << " \"" << name << "\"";
      } else if (name_opt == 2) {
	char const *name = ATT_name(sym);

	if (name && name[0])
	  os << " name==\"" << name << "\"";
      }
    }
  }

  if (childish) {
    ++level;
    os << "->";

    ocharstream curos;

    for (;;) {
      curos.reset();
      int curchildish = dfa_print_pattern(curos, child, name_opt, 
					  truncate, reg_st, reg_en, multiline, level);
      curos << ends;

      if (curchildish > 0) { 
	os << '(';
	if(multiline) os << '\n';
	else os << ' ';
	os << curos.ptr();
	if(multiline){
          os << '\n';
	  for(int iii=0; iii<level;++iii) os << level_indent;
	}
        os<< ')';
      } else  {
	os << curos.ptr();
      }
      tree_get_next(child, child);

      if (child.isnull()) 
	break;

      os << ',';
    }
  }

  return childish;
}

static int 
dfa_find_ast(astNode *root, 
	     int reg_st, 
	     int reg_en, 
	     astNode *&n1)
{
  Initialize(dfa_find_ast(astNode *, int, int, astNode *&));

  int start = root->get_start();
  int end   = start + root->get_length();
 
  if ((start >= reg_en) || (end <= reg_st))
    return 0;
    
  astNode *sn = NULL, *en = NULL;
  astNode *first = root->get_first();
  astNode *cur = first;

  //start
  for (; cur; cur = cur->get_next()) {
    int cur_en = cur->get_start() + cur->get_length();

    if (cur_en >= reg_st) {
      sn = cur;
      break;
    }
  }

  if (!sn) {
    n1 = root;
    return 1;
  }
 
  //end
  for (; cur; cur = cur->get_next()) {
    int cur_st = cur->get_start();

    if (cur_st > reg_en) {
      break;
    }

    en = cur;
  }

  IF(!en)
    return 0;

  int sns = sn->get_start();
  int ene = en->get_start() + en->get_length();

  if (sns > reg_st || ene < reg_en) {
    n1 = root;
    return 1;
  }

  if (sn == en)
    return 
      dfa_find_ast(sn, reg_st, reg_en, n1);

  if (sn == first && en->get_next() == NULL) {
    n1 = root;
    return 1;
  }

  int no_nodes = 1;  
  n1 = sn;

  for (cur=sn; cur!=en; cur=cur->get_next())
    no_nodes++;

  return no_nodes;
}

static int 
dfa_print_pattern(ostream &os, 
		  symbolPtr &sym, 
		  int reg_st, 
		  int reg_en,
		  int multiline)
{
  Initialize(dfa_print_pattern(ostream &, symbolPtr &, int, int, int));

  int opt = 1; //cname
  symbolPtr astsym = sym.get_astNode();
  astNode *root = astNode_of_sym(astsym);

  if (!root)
    return 0;

  astNode *n1 = NULL;
  int no_nodes = dfa_find_ast(root, reg_st, reg_en, n1);

  if (no_nodes == 0)
    return no_nodes;

  astXref *axr = astsym.get_astXref();

  if (no_nodes == 1) {
    if (!n1->get_first()) {
      astNode *par = n1->get_parent();

      if (par) {
        int code = n1->get_code();
	os << astNode_code_name(code) << "<-";
        n1 = par;
      }
    }

    symbolPtr cursym(n1, axr);
    dfa_print_pattern(os, cursym, opt, 1, reg_st, reg_en, multiline, 0);
  } else {
    for (int ii = 0; ii < no_nodes; ++ii) {
      if (ii>0){
	os << ',';
	if(multiline) os << '\n';
	else os << ' ';
      }
      os << "(";
      symbolPtr cursym(n1, axr);
      dfa_print_pattern(os, cursym, opt, 1, reg_st, reg_en, multiline, 0);    
      os << ")";
      n1 = n1->get_next();
    }
  }

  return no_nodes;
}

static int 
patternCmd(ClientData cd, 
	   Tcl_Interp *interp, 
	   int argc, 
	   char const *argv[])
{
  Initialize(patternCmd(ClientData, Tcl_interp, int, char const **));

  dfaLoader incremental;
  Interpreter *i = (Interpreter *) cd;
  symbolArr res;

  int ind = 1;
  int ml = 0;
  if((argc > 1) && (strcmp(argv[1], "-f")==0)){
    ind ++;
    ml = 1;
  }
  if (i->ParseArguments(ind, argc, argv, res) != 0)
    return TCL_ERROR;

  ocharstream os;
  int sz = res.size();

  for (int ii = 0; ii < sz; ++ii) {
    dfa_print_pattern(os, res[ii], 1, 0, 0, 0, ml, 0);
    os << endl;
  }

  os << ends;
  Tcl_SetResult(i->interp, os.ptr(), TCL_VOLATILE);
  return TCL_OK;
}

static int 
pattern_regionCmd(ClientData cd, 
		  Tcl_Interp *interp, 
		  int argc, 
		  char const *argv[])
{
  Initialize(pattern_regionCmd(ClientData, Tcl_Interp *, int, char const **));

  dfaLoader incremental;
  Interpreter *i = (Interpreter *) cd;
  int ind = 1;
  int ml = 0;

  if ((argc > 1) && (strcmp(argv[1], "-f") == 0)) {
    ind++;
    ml = 1;
  }

  if (argc < ind + 2)
    return TCL_ERROR;

  int start = OSapi_atoi(argv[ind]);
  int end   = OSapi_atoi(argv[ind + 1]);
  symbolArr res;

  if (i->ParseArguments(ind+2, argc, argv, res) != 0)
    return TCL_ERROR;

  ocharstream os;
  int sz = res.size();

  for (int ii = 0; ii < sz; ++ii) {
    dfa_print_pattern(os, res[ii], start, end, ml);

    if (ml)
      os << endl;
    else
      os << ' ';
  }

  os << ends;
  Tcl_SetResult(i->interp, os.ptr(), TCL_VOLATILE);
  return TCL_OK;
}

class astclassAttribute : public  intAttribute {

public:

  astclassAttribute(char const *name, ...);
  virtual int test(symbolPtr &sym) const;
  virtual void print(ostream & = cout, int level = 0) const;

  char mask[LAST_DFA_TREE_CODE];

  define_relational(astclassAttribute, intAttribute);
};

RelClass(astclassAttribute);

init_relational(astclassAttribute, intAttribute);

void 
astclassAttribute::print(ostream &str, 
			 int) const
{
  Initialize(astclassAttribute::print(ostream &, int) const);
  str << get_name();
}

int 
astclassAttribute::test(symbolPtr &sym) const
{
  Initialize(astclassAttributr::test(symbolPtr &) const);

  int retval = 0;
  int code = symbolPtr_get_code(sym);

  if (code >= 0)
    retval = mask[code];

  return retval;
}

astclassAttribute::astclassAttribute(char const *str, ...) 
    : intAttribute(str)
{
  Initialize(astclassAttribute::astclassAttribute(char const *, ...));

  for (int ii = 0; ii < LAST_DFA_TREE_CODE; ++ii)
    mask[ii] = 0;

  va_list ap;
  va_start(ap,str);
  int code = va_arg(ap, int);

  while (code >= 0) {
    mask[code] = 1;
    code = va_arg(ap, int);
  }

  va_end(ap);
  
  if (!ast_categories)
    ast_categories = new objSet;

  ast_categories->insert(this);
}

#define DECL					\
  MC_FUNCTION_DECL,				\
	    MC_LABEL_DECL,			\
	    MC_CONST_DECL,			\
	    MC_TYPE_DECL,			\
	    MC_VAR_DECL,			\
	    MC_PARM_DECL,			\
	    MC_RESULT_DECL,			\
	    MC_FIELD_DECL,			\
	    TEMPLATE_DECL,			\
	    THUNK_DECL,				\
	    NAMESPACE_DECL

#define UEXPR					\
  FIX_TRUNC_EXPR,				\
	    FIX_CEIL_EXPR,			\
	    FIX_FLOOR_EXPR,			\
	    FIX_ROUND_EXPR,			\
	    FLOAT_EXPR,				\
	    MC_NEGATE_EXPR,			\
	    MC_ABS_EXPR,			\
	    FFS_EXPR,				\
	    MC_BIT_NOT_EXPR,			\
	    CARD_EXPR,				\
	    MC_CONVERT_EXPR,			\
	    NOP_EXPR,				\
	    NON_LVALUE_EXPR,			\
	    MC_CONJ_EXPR,			\
	    MC_REALPART_EXPR,			\
	    MC_IMAGPART_EXPR,			\
	    PARENS,				\
	    UNARY_PLUS

#define BEXPR					\
  MC_PLUS_EXPR,					\
	    MC_MINUS_EXPR,			\
	    MC_MULT_EXPR,			\
	    TRUNC_DIV_EXPR,			\
	    CEIL_DIV_EXPR,			\
	    FLOOR_DIV_EXPR,			\
	    ROUND_DIV_EXPR,			\
	    TRUNC_MOD_EXPR,			\
	    CEIL_MOD_EXPR,			\
	    FLOOR_MOD_EXPR,			\
	    ROUND_MOD_EXPR,			\
	    MC_RDIV_EXPR,			\
	    EXACT_DIV_EXPR,			\
	    EXPON_EXPR,				\
	    MC_MIN_EXPR,			\
	    MC_MAX_EXPR,			\
	    MC_LSHIFT_EXPR,			\
	    MC_RSHIFT_EXPR,			\
	    MC_LROTATE_EXPR,			\
	    MC_RROTATE_EXPR,			\
	    MC_BIT_IOR_EXPR,			\
	    MC_BIT_XOR_EXPR,			\
	    MC_BIT_AND_EXPR,			\
	    MC_BIT_ANDTC_EXPR,			\
	    IN_EXPR,				\
	    RANGE_EXPR,				\
	    MC_COMPLEX_EXPR,			\
	    MC_DIV_EXPR,			\
	    ALSHIFT_ASSIGN_EXPR,		\
	    ARSHIFT_ASSIGN_EXPR,		\
	    BIT_AND_ASSIGN_EXPR,		\
	    BIT_IOR_ASSIGN_EXPR,		\
	    BIT_XOR_ASSIGN_EXPR,		\
	    MINUS_ASSIGN_EXPR,			\
	    MULT_ASSIGN_EXPR,			\
	    PLUS_ASSIGN_EXPR,			\
	    RDIV_ASSIGN_EXPR,			\
	    TRUNC_DIV_ASSIGN_EXPR,		\
	    TRUNC_MOD_ASSIGN_EXPR,		\
	    LRSHIFT_EXPR,			\
	    LRSHIFT_ASSIGN_EXPR

#define LEXPR					\
  MC_LT_EXPR,					\
	    MC_LE_EXPR,				\
	    MC_GT_EXPR,				\
	    MC_GE_EXPR,				\
	    MC_EQ_EXPR,				\
	    MC_NE_EXPR,				\
	    SET_LE_EXPR

#define CNST					\
  MC_INTEGER_CST,				\
	    MC_REAL_CST,			\
	    MC_COMPLEX_CST,			\
	    STRING_CST

#define TYPE					\
  MC_VOID_TYPE,					\
	    MC_INTEGER_TYPE,			\
	    MC_REAL_TYPE,			\
	    MC_COMPLEX_TYPE,			\
	    MC_ENUMERAL_TYPE,			\
	    MC_BOOLEAN_TYPE,			\
	    CHAR_TYPE,				\
	    MC_POINTER_TYPE,			\
	    OFFSET_TYPE,			\
	    MC_REFERENCE_TYPE,			\
	    METHOD_TYPE,			\
	    FILE_TYPE,				\
	    MC_ARRAY_TYPE,			\
	    SET_TYPE,				\
	    MC_RECORD_TYPE,			\
	    MC_UNION_TYPE,			\
	    MC_QUAL_UNION_TYPE,			\
	    MC_FUNCTION_TYPE,			\
	    LANG_TYPE,				\
	    TEMPLATE_TYPE_PARM,			\
	    UNINSTANTIATED_P_TYPE

#define STORAGE_REF				\
  MC_COMPONENT_REF,				\
	    MC_BIT_FIELD_REF,			\
	    MC_INDIRECT_REF,			\
	    OFFSET_REF,				\
	    BUFFER_REF,				\
	    MC_ARRAY_REF,			\
	    CP_OFFSET_REF,			\
	    SCOPE_REF,				\
	    MEMBER_REF,				\
	    JAVA_COMPONENT_REF

#define STMT					\
  MC_LABEL_STMT,				\
	    MC_GOTO_STMT,			\
	    MC_RETURN_STMT,			\
	    MC_EXPR_STMT,			\
	    WITH_STMT,				\
	    LET_STMT,				\
	    MC_IF_STMT,				\
	    EXIT_STMT,				\
	    MC_SWITCH_STMT,			\
	    MC_WHILE_STMT,			\
	    MC_FOR_STMT,			\
	    MC_DO_STMT,				\
	    MC_COMPOUND_STMT,			\
	    ASM_STMT

#define OTHER_EXPR				\
  MC_CONSTRUCTOR,				\
	    COMPOUND_EXPR,			\
	    MC_MODIFY_EXPR,			\
	    INIT_EXPR,				\
	    TARGET_EXPR,			\
	    MC_COND_EXPR,			\
	    BIND_EXPR,				\
	    MC_CALL_EXPR,			\
	    METHOD_CALL_EXPR,			\
	    WITH_CLEANUP_EXPR,			\
	    CLEANUP_POINT_EXPR,			\
	    WITH_RECORD_EXPR,			\
	    MC_TRUTH_ANDIF_EXPR,		\
	    MC_TRUTH_ORIF_EXPR,			\
	    MC_TRUTH_AND_EXPR,			\
	    MC_TRUTH_OR_EXPR,			\
	    MC_TRUTH_XOR_EXPR,			\
	    MC_TRUTH_NOT_EXPR,			\
	    MC_SAVE_EXPR,			\
	    RTL_EXPR,				\
	    MC_ADDR_EXPR,			\
	    REFERENCE_EXPR,			\
	    ENTRY_VALUE_EXPR,			\
	    MC_PREDECREMENT_EXPR,		\
	    MC_PREINCREMENT_EXPR,		\
	    MC_POSTDECREMENT_EXPR,		\
	    MC_POSTINCREMENT_EXPR,		\
	    DELETE_EXPR,			\
	    VEC_DELETE_EXPR,			\
	    MC_UNSAVE_EXPR,			\
	    TYPE_EXPR,				\
	    NEW_EXPR,				\
	    VEC_NEW_EXPR,			\
	    THROW_EXPR,				\
	    VEC_INIT_CONSTRUCTOR,		\
	    BASE_INIT_EXPR,			\
	    DYNAMIC_INIT,			\
	    CONDITION,				\
	    CTOR_INIT,				\
	    DFA_NULL,				\
	    SUPER,				\
	    DIMENSIONS,				\
	    EASY_NEW,				\
	    JAVA_VEC_NEW,			\
	    INSTANCEOF,				\
	    ANONYMOUS_CLASS,			\
	    THIS

#define EXPR UEXPR,BEXPR,LEXPR,OTHER_EXPR,STORAGE_REF

#define REF DFA_REF_DD, DFA_REF_DFA
#define Init_astclAt(a,b) new astclassAttribute(a,b,-1)

int 
Init_ast_part()
{
  Initialize(Init_ast_part());
  codeAttribute_init();
    
  Init_astclAt("ast_decl", DECL);
  Init_astclAt("ast_uexpr", UEXPR);
  Init_astclAt("ast_bexpr", BEXPR);
  Init_astclAt("ast_lexpr", LEXPR);
  Init_astclAt("ast_stmt", STMT);
  Init_astclAt("ast_expr", EXPR);
  Init_astclAt("ast_id", REF);
  Init_astclAt("ast_const", CNST);

  return 0;
}

static void 
tree_usage(Interpreter *i)
{
  Initialize(tree_usage(Interpreter *));
  ostream& os = i->GetOutputStream();

  os << "usage: tree [navigate find filter] <tree pattern> <set>\n";
  os << "       tree print <set>\n";
  os << "       tree verify <tree pattern>\n";
  os << "       tree ignore [0 1]" << endl;
} 

int instance_get_ast(const symbolPtr &sym, symbolArr &results);

void cli_defines_or_uses(symbolArr &src, symbolArr &res, ddSelector &sel, int def_flag);

static void 
tree_convert_symbols(int ignore,
		     symbolArr &arr, 
		     symbolArr &in_arr, 
		     symbolArr &bad_syms)
{
  Initialize(tree_convert_symbols(int, symbolArr &, symbolArr &, symbolArr &));

  int sz = arr.size();
  symbolArr projs;

  for (int ii = 0; ii < sz; ++ii) {
    symbolPtr sym = arr[ii];
    ddKind kind = sym.get_kind();

    if (kind == DD_PROJECT || kind == DD_SCOPE) {
      projs.insert_last(sym);
      continue;
    } else if (kind == DD_INSTANCE) {
      instance_get_ast(sym, in_arr);
      continue;
    } else if (kind == DD_AST) {
      in_arr.insert_last(sym);
      continue;
    } else if (kind == DD_MODULE) {
      symbolPtr xsym = sym.get_xrefSymbol();

      if (xsym.isnull()) {
	bad_syms.insert_last(sym);
	continue;
      } else
	sym = xsym;

      // Make sure that the module has a pset file:

      char const *pset = NULL;
      projModule *mod = symbol_get_module(sym);
      if (mod) {
        pset = OSPATH((char *)mod->paraset_file_name());
        if (pset) {
	  if (OSapi_access(pset, R_OK) != 0)
	    pset = NULL;
	}
      }
      if (!pset) {
	bad_syms.insert_last(sym);
	continue;
      }	
    }

    symbolPtr astsym = sym.get_astNode();

    if (astsym.isnull()) {
      bad_syms.insert_last(sym);
      continue;
    } 

    in_arr.insert_last(astsym);
  }

  int no_bad = bad_syms.size();
  if (no_bad && ignore == 0)
    return;

  if (projs.size()) {
    symbolArr res;
    ddSelector sel;
    sel.add(DD_MODULE);
    cli_defines_or_uses(projs, res, sel, 1);
    int sz = res.size();

    for (int ii = 0; ii < sz; ++ii) {
      symbolPtr astsym = res[ii].get_astNode();

      if (astsym.isnotnull())
	in_arr.insert_last(astsym);
      else 
	;//	bad_syms.insert_last(res[ii]);
    }
  }
}

expr *tree_strip(expr *tree);

static expr *
tree_cmd_prepare(Interpreter *i, 
		 int ignore, 
		 int arg_idx, 
		 int argc, 
		 char const *argv[], 
		 symbolArr &in_arr)
{
  Initialize(tree_cmd_prepare(Interpreter *, int, int, int, char const *, symbolArr &));

  if (argc < 3) {
    tree_usage(i);
    return NULL;
  }

  char const *e = argv[arg_idx];
  expr *tree    = api_parse_expression(e);

  if (!tree)
    return NULL;

  symbolArr arr;
  if (i->ParseArguments(arg_idx + 1, argc, argv, arr) != 0) {
    delete tree;
    return NULL;
  }

  symbolArr bad_syms;
  tree_convert_symbols(ignore, arr, in_arr, bad_syms);
  int no_bad = bad_syms.size();

  if (no_bad && ignore == 0) {
    ostream &os = i->GetOutputStream();
    os << " Cannot convert to ast " << no_bad << " symbols:" << endl;

    for (int ii = 0; ii < no_bad; ++ii)
      os << bad_syms[ii].get_name() << endl;

    delete tree;
    tree = 0;
  }

  return tree;  
}

static expr *
tree_cmd_verify(Interpreter *i, 
		int argc, 
		char const *argv[])
{
  Initialize(tree_cmd_verify(Interpreter *, int argc, char const **));

  if (argc < 3) {
    tree_usage(i);
    return NULL;
  }

  char const *e = argv[2];
  expr *tree = api_parse_expression(e);
  return tree;
}

static int tree_ignore_default = 1;

static int tree_cmd_ignore(Interpreter *i, int argc, char const *argv[])
{
  int retval = TCL_ERROR;
  if (argc == 3) {
    char const *e = argv[2];
    int len = strlen(e);
    if (len == 1) {
      int val = *e - '0';
      if (val == 0 || val ==1) {
	tree_ignore_default = val;
	void cli_set_error_default(int def);
	cli_set_error_default(val);
	retval = TCL_OK;
      }
    }
  }
  if(retval == TCL_ERROR)
    tree_usage(i);

  return retval;
}

#define CMD(t) if(strcmp(quote(t),str)==0) return cmd_##t

enum treeSubCmd { cmd_wrong, cmd_navigate, cmd_find, cmd_filter, cmd_print, cmd_dump, cmd_ignore, cmd_verify, cmd_nodes };

static treeSubCmd 
tree_cmd_type(char const *str)
{
  Initialize(tree_cmd_type(char const *));

  CMD(navigate);
  CMD(find);
  CMD(filter);
  CMD(print);
  CMD(dump);
  CMD(verify);
  CMD(ignore);

  return cmd_wrong;
}

static void 
tree_find(expr *t, 
	  symbolPtr &sym, 
	  symbolArr *results_arr, 
	  bool recurse)
{
  Initialize(tree_find(expr *, symbolPtr &, symbolArr *, bool));
  int res = t->value(sym);

  if (res && sym.isnotnull())
    results_arr->insert_last(sym);

  if (recurse) {
    symbolPtr cur;
    for (int st = tree_get_first(sym, cur); 
	 st; 
	 st = tree_get_next(cur, cur))
      tree_find(t, cur, results_arr, true);
  }
}

int astNode_print(int opt, const symbolPtr &sym,ostream &os, int level);
int astNode_dump(const symbolPtr &sym,ostream &os);

static int 
tree_printCmd(ClientData cd, 
	      Tcl_Interp *interp, 
	      int argc, 
	      char const *argv[])
{
  Initialize(tree_printCmd(ClientData, Tcl_interp, int, char const **));

  Interpreter *i = (Interpreter *) cd;
  symbolArr arr;

  if (i->ParseArguments(2, argc, argv, arr) != 0)
    return TCL_ERROR;

  symbolArr bad_syms;
  symbolArr in_arr;
  tree_convert_symbols(0, arr, in_arr, bad_syms);
  int sz = in_arr.size();
  ostream &os = i->GetOutputStream();

  for (int ii = 0; ii < sz; ++ii)
    astNode_print(0, in_arr[ii], os, 0);

  return TCL_OK;
}


static int 
tree_dumpCmd(ClientData cd, 
	      Tcl_Interp *interp, 
	      int argc, 
	      char const *argv[])
{
  Initialize(tree_dumpCmd(ClientData, Tcl_interp, int, char const **));

  Interpreter *i = (Interpreter *) cd;
  symbolArr arr;

  if (i->ParseArguments(2, argc, argv, arr) != 0)
    return TCL_ERROR;

  symbolArr bad_syms;
  symbolArr in_arr;
  tree_convert_symbols(0, arr, in_arr, bad_syms);
  int sz = in_arr.size();
  ostream &os = i->GetOutputStream();
  for (int ii = 0; ii < sz; ++ii)
    astNode_dump(in_arr[ii], os);
  return TCL_OK;
}

int
treeCmd(ClientData cd, 
	Tcl_Interp *interp, 
	int argc, 
	char const *argv[])
{
  Initialize(treeCmd(ClientData, Tcl_Interp *, int, char const **));

  dfaLoader incremental;
  static int has_tpm_license = 0;
  Interpreter *i = (Interpreter *) cd;

  if (has_tpm_license == 0) {
    if (_lo(LIC_TPM)) {    // check out
      Tcl_SetResult(i->interp, (char *)"0", TCL_VOLATILE);
      _lm(LIC_TPM);    // error msg  
      return TCL_ERROR;
    } else 
      has_tpm_license = 1;
  }

  if (argc < 2) {
    tree_usage(i);
    return TCL_ERROR;
  }

  char const *subcmd   = argv[1];
  treeSubCmd cmd = tree_cmd_type(subcmd);

  if (cmd == cmd_wrong) {
    Tcl_AppendResult(i->interp, subcmd,  ": wrong tree subcommand\n", NULL);
    tree_usage(i);
    return TCL_ERROR;
  } else {
	  if (cmd == cmd_print) {
           return  tree_printCmd(cd, interp, argc, argv);
	  } else  {
	       if (cmd == cmd_dump) {
               return  tree_dumpCmd(cd, interp, argc, argv);
		   } else  {
		      if (cmd == cmd_verify) {
                  expr *tree = tree_cmd_verify(i, argc, argv);
                  if (tree != NULL) {
                      Tcl_AppendResult(interp, "1", NULL);
                      delete tree;
				  } else {
                      Tcl_AppendResult(interp, "0", NULL);
				  }
                  return TCL_OK;
			  } else if (cmd == cmd_ignore) {
                  return tree_cmd_ignore(i, argc, argv);
			  }
		   }
	  }
  }
  int arg_idx = 2;
  int ignore  = tree_ignore_default;

  if (argc > 2 && !strcmp(argv[2], "-ignore")) {
    ignore = 1;
    arg_idx++;
  }

  symbolArr in_arr;
  expr *tree = tree_cmd_prepare(i, ignore, arg_idx, argc, argv, in_arr);

  if (!tree) {
    if (ignore)
      return TCL_OK;
    else
      return TCL_ERROR;
  }
    
  DI_object results;
  DI_object_create(&results);
  symbolArr *results_arr = get_DI_symarr(results);

  int sz = in_arr.size();
  bool recurse = (cmd == cmd_find);
  expr *t = (cmd == cmd_navigate) ? tree_strip(tree) : tree;

  for (int ii = 0; ii < sz; ++ii)
    tree_find(t, in_arr[ii], results_arr, recurse);

  if (sz > 1)
    results_arr->unsort();

  delete tree;
  i->SetDIResult(results);
  return TCL_OK;
}

static int 
ast_is_declaration(const symbolPtr &sym)
{
  Initialize(ast_is_declaration(const symbolPtr &));
  const symbolPtr *psym = NULL;

  if (sym.is_dataCell()) {
    dataCell *data = dataCell_of_symbol(sym, false);

    if (data) {
      psym = &(data->get_astnode());
      dataCell_offload(data, false);
    }
  } else if(sym.is_ast()) {
    psym = &sym;
  } else {
    return 0;
  }

  astNode *node = astNode_of_sym(*psym);
  if (!node)
    return 0;

  int retval = 0;
  astNode *parent = node->get_parent();

  if (!parent)
    return 0;// FALSE

  int code = parent->get_code();

  switch (code) {
  case MC_FIELD_DECL:	
  case MC_CONST_DECL:
  case MC_VAR_DECL:
  case MC_PARM_DECL:
  case MC_TYPE_DECL:
  case MC_FUNCTION_DECL:
    retval = (parent->get_first() == node);
    break;
  }

  return retval;
}

symbolPtr 
dfa_decl_of_ast(symbolPtr &ast)
{
  Initialize(dfa_decl_of_ast(symbolPtr &));

  symbolPtr decl;
  ddElement *dd;
  dfa_decl *dcl;
  astNode *node = astNode_of_sym(ast);

  if (!node)
    return decl;

  int code = node->get_code();

  switch (code) {
  case MC_PARM_DECL:
    return ast;
  case MC_VAR_DECL:
    dd = ((dfa_decl *) node)->get_dd();
    if (dd->get_kind() == DD_LOCAL)
      return ast;
    else 
      return dd->get_xrefSymbol();
    break;
  case DFA_REF_DFA:
    dcl = ((dfa_ref_dfa *) node)->get_decl();
    return symbolPtr(dcl, ast.get_astXref());
    break;
  default:
    return ast.get_xrefSymbol();
  }
}

static bool
node_get_instance(const symbolPtr &sym,
		  symbolPtr &inst)
{
  Initialize(node_get_instance(const symbolPtr &, symbolPtr &));
  astNode *node = astNode_of_sym(sym);

  if (!node)
    return false;

  int code = node->get_code();

  switch (code) {
  case MC_COMPONENT_REF:
  case JAVA_COMPONENT_REF:
    //         component_ref 3569 6 : `window'
    //           indirect_ref 3569 6 : `window'
    //           | local_id 3569 6 : `window'
    //           global_id 3569 6 : `window'
    //
    // search for global_id
    node = node->get_first();
    if (!node)
      return false;
    node = node->get_next();
    break;

  case DFA_REF_DD:
  case DFA_REF_DFA:
    break;

  case MC_FIELD_DECL:	
  case MC_CONST_DECL:
  case MC_VAR_DECL:
  case MC_PARM_DECL:
  case MC_FUNCTION_DECL:
  case MC_CALL_EXPR:
  case NEW_EXPR:
    node = node->get_first();
    break;

  default:
    node = NULL;
  }

  if(!node)
    return false;
  int off = node->get_start();
  if (off < 0)
    return false;

  int len = node->get_length();
  if(len == 0)
    return false;

  char const *str = ATT_filename((symbolPtr &) sym);
  char *tmpstr = new char[strlen(str)+1];
  strcpy(tmpstr, str);
  inst = MetricElement::get_instance(tmpstr, off);
  delete tmpstr;

  return
    inst.isnotnull();
}

static void 
node_get_instance(const symbolArr &arr, 
		  symbolArr &resarr)
{
  Initialize(node_get_instance(const symbolArr &, symbolArr &));

  int sz = arr.size();
  symbolPtr inst;  

  for (int ii = 0; ii < sz; ++ii) {
    const symbolPtr sym = arr[ii].get_astNode();

    if (sym.isnull())
      continue;

    if (node_get_instance(sym, inst))
      resarr.insert_last(inst);
  }
}

static int 
data_instanceCmd(ClientData cd, 
		 Tcl_Interp *interp, 
		 int argc, 
		 char const *argv[])
{
  Initialize(data_instanceCmd(ClientData, Tcl_Interp *, int, char const **));

  Interpreter *i = (Interpreter *) cd;
  symbolArr arr;

  if (i->ParseArguments(1, argc, argv, arr) != 0)
    return TCL_ERROR;

  DI_object resobj;
  DI_object_create(&resobj);
  symbolArr &resarr = *get_DI_symarr(resobj);  

  node_get_instance(arr, resarr);

  resarr.unsort();
  i->SetDIResult(resobj);
  return TCL_OK;
}

static int 
data_nodeCmd(ClientData cd, 
	     Tcl_Interp *interp, 
	     int argc, 
	     char const *argv[])
{
  Initialize(data_nodeCmd(ClientData, Tcl_Interp *, int, char const **));

  Interpreter *i = (Interpreter *) cd;
  symbolArr arr;

  if (i->ParseArguments(1, argc, argv, arr) != 0)
    return TCL_ERROR;

  int sz = arr.size();

  DI_object resobj;
  DI_object_create(&resobj);
  symbolArr &resarr = *get_DI_symarr(resobj);  

  for (int ii = 0; ii < sz; ++ii) {
    symbolPtr sym = arr[ii].get_astNode();

    if (sym.isnotnull())
      resarr.insert_last(sym);
  }

  resarr.unsort();
  i->SetDIResult(resobj);
  return TCL_OK;
}

static int 
data_symbolCmd(ClientData cd, 
	       Tcl_Interp *interp, 
	       int argc, 
	       char const *argv[])
{
  Initialize(data_symbolCmd(ClientData, Tcl_Interp *, int, char const **));

  Interpreter *i = (Interpreter *) cd;
  symbolArr arr;

  if (i->ParseArguments(1, argc, argv, arr) != 0)
    return TCL_ERROR;

  int sz = arr.size();

  DI_object resobj;
  DI_object_create(&resobj);
  symbolArr &resarr = *get_DI_symarr(resobj);  

  for (int k = 0; k < sz; ++k) {
    symbolPtr sym = arr[k];
    symbolPtr symbol;

    if (sym.is_dataCell()) {
      dataCell *cell = dataCell_of_symbol(sym, false);

      if (cell) {
	symbol = cell->get_xrefSymbol();
	dataCell_offload(cell, false);
      }
    } else
      symbol = sym.get_xrefSymbol();

    if (symbol.isnotnull())
      resarr.insert_last(symbol);
  }

  resarr.unsort();
  i->SetDIResult(resobj);
  return TCL_OK;
}

static void
find_ast_decl(symbolPtr &symbol,
	      symbolPtr &node,
	      symbolArr &results)
{
  Initialize(find_local_decl(symbolPtr &, symbolPtr &, symbolArr &));

  if (!node.is_ast())
    node = node.get_astNode();

  if (node.isnull())
    return;

  if (!symbol.is_xrefSymbol())
    symbol = symbol.get_xrefSymbol();

  if (symbol.isnull())
    return;

  int code = symbolPtr_get_code(node);

  if (code == MC_VAR_DECL || code == MC_PARM_DECL) {
    symbolPtr nodeSymbol = node.get_xrefSymbol();

    if (!nodeSymbol.sym_compare(symbol))
      results.insert_last(node);
  }

  symbolPtr current;

  for (tree_get_first(node, current);
       current.isnotnull();
       tree_get_next(current, current))
    find_ast_decl(symbol, current, results);
}

static void 
dfa_data_decl(symbolPtr &sym, 
	      symbolArr &resarr)
{
  Initialize(dfa_data_decl(symbolPtr &, symbolArr &));

  if (sym.is_xrefSymbol()) {
    if (sym.get_kind() == DD_LOCAL) {
      symbolArr fileArr;
      sym.get_link(ref_file, fileArr);
      symbolPtr fileSymbol;

      ForEachS(fileSymbol, fileArr) {
	bool isLoaded = ATT_is_loaded(fileSymbol);
	app *head = fileSymbol.get_def_app();

	if (!head)
	  continue;

	astXref *axr = app_get_astXref(head);

	if (axr) { 
	  astRoot *root = axr->get_ast_table();

	  if (root) {
	    astNode *node = root->get_astNode();
	    symbolPtr nodeSymbol(node, axr);
	    find_ast_decl(sym, nodeSymbol, resarr);
	  }
	}

	if (!isLoaded) {
	  msg("Unloading $1 ... ") << head->get_name() << eom;
	  cout.flush();
	  obj_unload(head);
	  msg("done\n") << eom;
	  cout.flush();
	}
      }
    } else
      resarr.insert_last(sym);
  } else if (sym.is_dataCell()) {
    dataCell *cell = dataCell_of_symbol(sym, false);

    if (cell) {
      symbolPtr symbol = cell->get_symbol();
      dfa_data_decl(symbol, resarr);
      dataCell_offload(cell, false);
    }
  } else {
    symbolPtr ast = sym.get_astNode();

    if (ast.isnotnull())
      ast = dfa_decl_of_ast(ast);
    if (ast.isnotnull())
      resarr.insert_last(ast);
  }
}

void
symbol_get_context(const symbolPtr &symbol,
		   symbolArr &results)
{
  Initialize(symbol_get_context(const symbolPtr &, symbolPtr &));

  if (symbol.is_ast()) {
    symbolPtr node = symbol;
    symbolPtr scope;

    for (; node.isnotnull(); tree_get_parent(node, node)) {
      int code = symbolPtr_get_code(node);

      if ((code == MC_VAR_DECL || code == MC_TYPE_DECL || code == MC_CONST_DECL ||
	   code == MC_NAMESPACE_DECL || code == TEMPLATE_DECL) &&
	  scope.isnull())
	scope = node;
      else if (code == DFA_FUNCTION_DEF || code == DFA_ROOT) {
	scope = node;
	break;
      }
    }

    dfa_data_decl(scope, results);
  } else if (symbol.is_dataCell()) {
    dataCell *cell = dataCell_of_symbol(symbol, false);

    if (cell) {
      if (!cell->isAbstract()) {
	symbolPtr node = cell->get_symbol();
	symbol_get_context(node, results);
      } else {
	symbolPtr scope;
	GDGraph *graph = cell->get_graph();
	graph = graph->getParent();
	graph->getSymbol(scope);
	dfa_data_decl(scope, results);
      }

      dataCell_offload(cell, false);
    }
  }
}
bool api_def_less_kind(ddKind kind);
static int 
symbol_contextCmd(Interpreter*i, int argc, char const *argv[])
{
  symbolArr arr;

  if (i->ParseArguments(1, argc, argv, arr) != 0)
    return TCL_ERROR;

  int sz = arr.size();

  DI_object resobj;
  DI_object_create(&resobj);
  symbolArr &resarr = *get_DI_symarr(resobj);  

  for (int ii = 0; ii < sz; ++ii) {
    symbolPtr &sym = arr[ii];
    ddKind kind = sym.get_kind();
    switch(kind){
    case DD_AST:
    case DD_DATA:
      {
	symbolArr scopes;
	symbol_get_context(sym, scopes);
        if(scopes.size())
	  resarr.insert_last(scopes);
      }
      break;
    case DD_INSTANCE:
      {
	symbolPtr sc = sym.scope();
	if(sc.xrisnotnull())
	  resarr.insert_last(sc);
      }
      break;
    default:
      if(!api_def_less_kind(kind))
	resarr.insert_last(sym);
      break;
    }
  }

  resarr.unsort();
  i->SetDIResult(resobj);
  return TCL_OK;
}
static int 
data_declCmd(ClientData cd, 
	     Tcl_Interp *interp, 
	     int argc, 
	     char const *argv[])
{
  Initialize(data_declCmd(ClientData, Tcl_Interp *, int, char const **));

  Interpreter *i = (Interpreter *) cd;
  symbolArr arr;

  if (i->ParseArguments(1, argc, argv, arr) != 0)
    return TCL_ERROR;

  int sz = arr.size();

  DI_object resobj;
  DI_object_create(&resobj);
  symbolArr &resarr = *get_DI_symarr(resobj);  

  for (int ii = 0; ii < sz; ++ii) {
    symbolPtr &sym = arr[ii];
    dfa_data_decl(sym, resarr);
  }

  resarr.unsort();
  i->SetDIResult(resobj);
  return TCL_OK;
}

static void 
data_usage(Interpreter *i, 
	   char const *cmd = 0)
{
  Initialize(data_usage(Interpreter *, char const *));
  ostream &os = i->GetOutputStream();

  if (cmd)
    os << cmd << ": wrong data subcommand" << endl;

  os << "usage: data [cells context inflow outflow decl field find node instance symbol trace unique] <arguments>" << endl;
}

void api_instances(symbolPtr &s, symbolArr &arr);

static int
data_cellsCmd(ClientData cd, 
	      Tcl_Interp *interp, 
	      int argc, 
	      char const *argv[])
{
  Initialize(data_cellsCmd(ClientData, Tcl_Interp *, int, char const **));

  Interpreter *i = (Interpreter *) cd;
  symbolArr arr;

  if (i->ParseArguments(1, argc, argv, arr) != 0)
    return TCL_ERROR;

  DI_object resobj;
  DI_object_create(&resobj);
  symbolArr &results = *get_DI_symarr(resobj);

  for (int k = 0; k < arr.size(); ++k) {
    symbolPtr symbol = arr[k];
    GDGraph *graph = NULL;

    if (symbol.is_xrefSymbol()) {
      ddKind kind = symbol.get_kind();

      if (kind == DD_MODULE || kind == DD_FUNC_DECL) {
	graph = GDSet::search(symbol);

	if (graph)
	  graph->getDataCells(symbol, results, true);
      } else
	api_instances(symbol, arr);
    } else if (symbol.is_dataCell())
      results.insert_last(symbol);
    else if (symbol.is_ast() || symbol.is_instance()) {
      symbolPtr node = symbol;

      if (symbol.is_instance())
	node = node.get_astNode();

      if (node.isnotnull()) {
	graph = GDSet::search(node);

	if (graph)
	  graph->getDataCells(node, results, true);
      }
    }

#if 0
    if (graph)
      graph->offloadGraph(false);
#endif
  }

  results.unsort();
  i->SetDIResult(resobj);
  return TCL_OK;
}

static int
data_flowCmd(ClientData cd,
	     Tcl_Interp *interp,
	     int argc,
	     char const *argv[],
	     DataQuery query)
{
  Initialize(data_flowCmd(ClientData, Tcl_Interp *, int, char const **, DataQuery));

  if (argc < 2)
    return TCL_ERROR;

  static int count = 0;
  Interpreter *i = (Interpreter *) cd;
  symbolArr dataArr, scopeArr;

  if (i->ParseArguments(1, 2, argv, dataArr) != 0)
    return TCL_ERROR;

  if (i->ParseArguments(2, argc, argv, scopeArr) != 0)
    return TCL_ERROR;

  DI_object resultObj;
  DI_object_create(&resultObj);
  symbolArr &resultSet = *get_DI_symarr(resultObj);
  unsigned int dataSize = dataArr.size();
  unsigned int scopeSize = scopeArr.size();
  objSet defnSet;
  Obj *defn;

  if (dataSize == 0 || scopeSize == 0) {
    i->SetDIResult(resultObj);
    return TCL_OK;
  }

  for (unsigned int k = 0; k < scopeSize; ++k) {
    symbolPtr &graphSymbol = scopeArr[k];
    defn = GDSet::search(graphSymbol, defnSet, false);
    GDGraph *graph = GDSet::search(graphSymbol, false);

    if (graph && !defnSet.includes(graph))
      defnSet.insert(graph);
  }

  ForEach(defn, defnSet) {
    GDGraph *graph = (GDGraph *) defn;
    GDGraph *parent = graph->getParent();
    parent->loadGraph(true);
    graph->closeGraph(true);
    bool abstract = true;

    for (unsigned int m = 0; m < dataSize; ++m) {
      symbolPtr &symbol = dataArr[m];
      dataCell *queryCell = NULL;
      GDGraph *queryParent = NULL;
      symbolPtr keySymbol;
      symbolArr symArr;

      if (symbol.is_xrefSymbol())
	dfa_data_decl(symbol, symArr);
      else {
	symArr.insert_last(symbol);

	if (symbol.is_dataCell()) {
	  queryCell = dataCell_of_symbol(symbol, false);

	  if (queryCell) {
	    GDGraph *queryGraph = queryCell->get_graph();

	    if (queryGraph) {
	      queryParent = queryGraph->getParent();

	      if (!queryCell->isAbstract()) {
		queryCell = queryGraph->findDataCell(queryCell);
		abstract = false;
	      }
	    }
	  }
	}
      }

      unsigned int symSize = symArr.size();

      for (unsigned int n = 0; n < symSize; ++n) {
	symbolPtr &cellSymbol = symArr[n];

	if (!queryCell)
	  graph->dataFlow(cellSymbol, resultSet, keySymbol,
			  queryCell, query);
	else {
	  objSet querySet;
	  queryCell->transform(0, NULL, querySet, graph);
	  Obj *obj;

	  ForEach(obj, querySet) {
	    dataCell *realQueryCell = (dataCell *) obj;
	    graph->dataFlow(cellSymbol, resultSet, keySymbol,
			    realQueryCell, query);
	  }
	}
      }

      if (queryCell && queryParent != parent) {
	if (abstract)
	  dataCell_offload(queryCell, false);
#if 0
	else
	  queryParent->offloadGraph(false);
#endif
      }
    }

#if 0
    parent->offloadGraph(false);
#endif

    if (count++ % DFA_QUERY_INCREMENT == 0) {
      msg(".") << eom;
      cout.flush();
    }
  }

  resultSet.unsort();
  i->SetDIResult(resultObj);
  return TCL_OK;
}

static int
data_findCmd(ClientData cd,
	     Tcl_Interp *interp,
	     int argc,
	     char const *argv[])
{
  Initialize(data_findCmd(ClientData, Tcl_Interp *, int, char const **));

  if (argc < 2)
    return TCL_ERROR;

  static int count = 0;
  Interpreter *i = (Interpreter *) cd;
  symbolArr dataArr, scopeArr;

  if (i->ParseArguments(1, 2, argv, dataArr) != 0)
    return TCL_ERROR;

  if (i->ParseArguments(2, argc, argv, scopeArr) != 0)
    return TCL_ERROR;

  DI_object resultObj;
  DI_object_create(&resultObj);
  symbolArr &resultSet = *get_DI_symarr(resultObj);
  unsigned int dataSize = dataArr.size();
  unsigned int scopeSize = scopeArr.size();
  objSet defnSet;
  Obj *defn;

  if (dataSize == 0 || scopeSize == 0) {
    i->SetDIResult(resultObj);
    return TCL_OK;
  }

  for (unsigned int k = 0; k < scopeSize; ++k) {
    symbolPtr &graphSymbol = scopeArr[k];
    defn = GDSet::search(graphSymbol, defnSet, false);
    GDGraph *graph = GDSet::search(graphSymbol, false);

    if (graph && !defnSet.includes(graph))
      defnSet.insert(graph);
  }

  ForEach(defn, defnSet) {
    GDGraph *graph = (GDGraph *) defn;
    GDGraph *parent = graph->getParent();
    parent->loadGraph(true);

    for (unsigned int m = 0; m < dataSize; ++m) {
      symbolPtr &symbol = dataArr[m];

      if (symbol.is_dataCell()) {
	dataCell *queryCell = dataCell_of_symbol(symbol, false);

	if (queryCell) {
	  objSet dataSet;
	  queryCell->transform(0, NULL, dataSet, graph);
	  GDGraph *queryGraph = queryCell->get_graph();
	  Obj *data;

	  ForEach(data, dataSet) {
	    dataCell *realQueryCell = (dataCell *) data;
	    dataCell *cell = graph->findDataCell(realQueryCell);

	    if (cell) {
	      symbolPtr cellSymbol(cell);
	      resultSet.insert_last(cellSymbol);
	    }
	  }

	  if (queryGraph->getParent() != parent)
	    dataCell_offload(queryCell, false);
	}
      } else {
	objArr cellArr;
	symbolArr declArr;
	dfa_data_decl(symbol, declArr);
	unsigned int declSize = declArr.size();

	for (unsigned int k = 0; k < declSize; ++k) {
	  symbolPtr &decl = declArr[k];
	  DataSet *bucket = graph->find(decl);

	  if (!bucket)
	    continue;

	  unsigned int cellSize = bucket->size();

	  for (unsigned int m = 0; m < cellSize; ++m) {
	    dataCell *cell = (*bucket)[m];
	    symbolPtr cellSymbol(cell);
	    resultSet.insert_last(cellSymbol);
	  }
	}
      }
    }

#if 0
    parent->offloadGraph(false);
#endif
  }

  resultSet.unsort();
  i->SetDIResult(resultObj);
  return TCL_OK;
}

static int
data_traceCmd(ClientData cd, 
	      Tcl_Interp *interp, 
	      int argc, 
	      char const *argv[])
{
  Initialize(data_traceCmd(ClientData, Tcl_Interp *, int, char const **));

  if (argc < 3)
    return TCL_OK;

  Interpreter *i = (Interpreter *) cd;
  symbolArr results;

  if (i->ParseArguments(1, argc, argv, results) != 0)
    return TCL_ERROR;

  DI_object resultObj;
  DI_object_create(&resultObj);
  symbolArr &resultSet = *get_DI_symarr(resultObj);
  int size = results.size();

  if (size > 1) {
    DataChain search;
    symbolPtr srcSymbol = results[0];
    symbolPtr trgSymbol = results[1];
    objSet graphSet;
    Obj *defn;

    for (int k = 2; k < size; ++k) {
      symbolPtr funcSymbol = results[k];
      defn = GDSet::search(funcSymbol, graphSet, false);
      GDGraph *graph = GDSet::search(funcSymbol, false);

      if (graph && !graphSet.includes(graph))
	graphSet.insert(graph);
    }

    ForEach(defn, graphSet) {
      GDGraph *graph = (GDGraph *) defn;
      graph->loadGraph(true);
      search.construct(srcSymbol, trgSymbol, graph);
#if 0
      graph->offloadGraph(false);
#endif
    }

    while (search.unexpanded()) {
      DataRelation *relation = search.getUnexpanded();

      if (relation)
	search.expandRelation(relation);
    }

    while (search.irreducible()) {
      DataRelation *relation = search.getIrreducible();

      if (relation)
	relation->makeConcrete(resultSet);
    }
  }

  resultSet.unsort();
  i->SetDIResult(resultObj);
  return TCL_OK;
}

static int
data_fieldCmd(ClientData cd, 
	      Tcl_Interp *interp, 
	      int argc, 
	      char const *argv[])
{
  Initialize(data_fieldCmd(ClientData, Tcl_Interp *, int, char const **));

  if (argc < 1)
    return TCL_OK;

  Interpreter *i = (Interpreter *) cd;
  symbolArr results;

  if (i->ParseArguments(1, argc, argv, results) != 0)
    return TCL_ERROR;

  DI_object resultObj;
  DI_object_create(&resultObj);
  symbolArr &resultSet = *get_DI_symarr(resultObj);
  int size = results.size();

  for (int k = 0; k < size; ++k) {
    symbolPtr symbol = results[k];

    if (symbol.is_dataCell()) {
      dataCell *cell = dataCell_of_symbol(symbol, false);

      if (!cell)
	continue;

      dataCell *field = cell;
      dataCell *temp = cell;

      while (temp) {
	field = temp;
	temp = temp->getField();
      }

      symbolPtr fieldSymbol = field->get_symbol();
      resultSet.insert_last(fieldSymbol);
      dataCell_offload(cell, false);
    } else
      dfa_data_decl(symbol, resultSet);
  }

  resultSet.unsort();
  i->SetDIResult(resultObj);
  return TCL_OK;
}

static int
data_graphCmd(ClientData cd, 
	      Tcl_Interp *interp, 
	      int argc, 
	      char const *argv[])
{
  Initialize(data_graphCmd(ClientData, Tcl_Interp *, int, char const **));

  if (argc < 1)
    return TCL_OK;

  Interpreter *i = (Interpreter *) cd;
  symbolArr results;

  if (i->ParseArguments(1, argc, argv, results) != 0)
    return TCL_ERROR;

  DI_object resultObj;
  DI_object_create(&resultObj);
  symbolArr &resultSet = *get_DI_symarr(resultObj);
  int size = results.size();

  for (int k = 0; k < size; ++k) {
    symbolPtr symbol = results[k];

    if (symbol.is_dataCell()) {
      GDGraph *graph = symbol.get_graph();
      symbolPtr graphSymbol;

      if (call_get_graph(graph))
	graph->getAstSymbol(graphSymbol, false);
      else
	graph->getSymbol(graphSymbol);

      resultSet.insert_last(graphSymbol);
    }
  }

  resultSet.unsort();
  i->SetDIResult(resultObj);
  return TCL_OK;
}

static int
data_contextCmd(ClientData cd, 
		Tcl_Interp *interp, 
		int argc, 
		char const *argv[])
{
  Initialize(data_contextCmd(ClientData, Tcl_Interp *, int, char const **));

  if (argc < 1)
    return TCL_OK;

  Interpreter *i = (Interpreter *) cd;
  symbolArr results;

  if (i->ParseArguments(1, argc, argv, results) != 0)
    return TCL_ERROR;

  DI_object resultObj;
  DI_object_create(&resultObj);
  symbolArr &resultSet = *get_DI_symarr(resultObj);
  int size = results.size();

  for (int k = 0; k < size; ++k) {
    symbolPtr symbol = results[k];

    if (symbol.is_dataCell()) {
      dataCell *cell = dataCell_of_symbol(symbol, false);

      if (cell) {
	const symbolPtr &context = cell->getContext();
	resultSet.insert_last(context);
	dataCell_offload(cell, false);
      }
    }
  }

  resultSet.unsort();
  i->SetDIResult(resultObj);
  return TCL_OK;
}

static int
data_uniqueCmd(ClientData cd, 
	       Tcl_Interp *interp, 
	       int argc, 
	       char const *argv[])
{
  Initialize(data_uniqueCmd(ClientData, Tcl_Interp *, int, char const **));

  if (argc < 1)
    return TCL_OK;

  Interpreter *i = (Interpreter *) cd;
  symbolArr results;

  if (i->ParseArguments(1, argc, argv, results) != 0)
    return TCL_ERROR;

  DI_object resultObj;
  DI_object_create(&resultObj);
  symbolArr &resultSet = *get_DI_symarr(resultObj);
  int size = results.size();
  objArr cellSet, graphSet;

  for (int k = 0; k < size; ++k) {
    symbolPtr symbol = results[k];

    if (!symbol.is_dataCell()) {
      resultSet.insert_last(symbol);
      continue;
    }

    dataCell *cell = dataCell_of_symbol(symbol, true);
    GDGraph *graph = cell->get_graph();
    cellSet.insert_last(cell);
    graphSet.insert_last(graph);
  }

  size = cellSet.size();

  while (size > 0) {
    dataCell *cell = (dataCell *) cellSet[--size];
    symbolPtr cellSymbol(cell);
    resultSet.insert_last(cellSymbol);
    cellSet.remove(cell);

    for (int m = size; m > 0; --m) {
      dataCell *nextCell = (dataCell *) cellSet[m - 1];

      if (cell->cellCompare(nextCell))
	cellSet.remove(nextCell);
    }

    size = cellSet.size();
  }

#if 0
  size = graphSet.size();

  for (int n = 0; n < size; ++n) {
    GDGraph *graph = (GDGraph *) graphSet[n];
    graph->offloadGraph(false);
  }
#endif

  resultSet.unsort();
  i->SetDIResult(resultObj);
  return TCL_OK;
}

static int 
dataCmd(ClientData cd, 
	Tcl_Interp *interp, 
	int argc, 
	char const *argv[])
{
  Initialize(dataCmd(ClientData, Tcl_Interp *, int, char const **));

  dfaLoader incremental;
  Interpreter *i = (Interpreter *) cd;

  if (argc == 1) {
    data_usage(i);
    return TCL_OK;
  }

  char const *cmd = argv[1];
  
  if (!strcmp(cmd, "decl")) {
    return 
      data_declCmd(cd, interp, argc - 1, argv + 1);
  } else if (!strcmp(cmd, "node")) {
    return 
      data_nodeCmd(cd, interp, argc - 1, argv + 1);
  } else if (!strcmp(cmd, "instance")) {
    return 
      data_instanceCmd(cd, interp, argc - 1, argv + 1);
  } else if (!strcmp(cmd, "symbol")) {
    return 
      data_symbolCmd(cd, interp, argc - 1, argv + 1);
  } else if (!strcmp(cmd, "cells")) {
    return
      data_cellsCmd(cd, interp, argc - 1, argv + 1);
  } else if (!strcmp(cmd, "inflow")) {
    return
      data_flowCmd(cd, interp, argc - 1, argv + 1, DFA_INFLOW);
  } else if (!strcmp(cmd, "outflow")) {
    return
      data_flowCmd(cd, interp, argc - 1, argv + 1, DFA_OUTFLOW);
  } else if (!strcmp(cmd, "trace")) {
    return
      data_traceCmd(cd, interp, argc - 1, argv + 1);
  } else if (!strcmp(cmd, "field")) {
    return
      data_fieldCmd(cd, interp, argc - 1, argv + 1);
  } else if (!strcmp(cmd, "graph")) {
    return
      data_graphCmd(cd, interp, argc - 1, argv + 1);
  } else if (!strcmp(cmd, "unique")) {
    return
      data_uniqueCmd(cd, interp, argc - 1, argv + 1);
  } else if (!strcmp(cmd, "find")) {
    return
      data_findCmd(cd, interp, argc - 1, argv + 1);
  } else if (!strcmp(cmd, "context")) {
    return
      data_contextCmd(cd, interp, argc - 1, argv + 1);
  } else {
    data_usage(i, cmd);
    return TCL_ERROR;
  }
}

static int
dfa_unloadCmd(ClientData cd,
	      Tcl_Interp *interp,
	      int argc,
	      char const *argv[])
{
  Initialize(dfa_unloadCmd(ClientData, Tcl_Interp, int, char const **));
  int size = GDSet::tableSet_.size();

  for (int k = size; k > 0; --k) {
    GDSet *hashtable = (GDSet *) GDSet::tableSet_[k - 1];

    if (hashtable)
      delete hashtable;
  }

  return TCL_OK;
}

static int
graphRelationsCmd(ClientData cd,
		  Tcl_Interp *interp,
		  int argc,
		  char const *argv[])
{
  Initialize(graphRelationsCmd(ClientData, Tcl_Interp *, int, char const **));

  Interpreter *i = (Interpreter *) cd;
  symbolArr results;

  if (i->ParseArguments(1, argc, argv, results) != 0)
    return TCL_ERROR;

  ostream &outStream = i->GetOutputStream();
  int size = results.size();
  objSet graphSet;
  Obj *defn;

  for (int k = 0; k < size; ++k) {
    symbolPtr symbol = results[k];
    defn = GDSet::search(symbol, graphSet, false);
    GDGraph *graph = GDSet::search(symbol, false);

    if (graph && !graphSet.includes(graph))
      graphSet.insert(graph);
  }

  ForEach(defn, graphSet) {
    GDGraph *graph = (GDGraph *) defn;
    graph->loadGraph(true);
    graph->printRelations(outStream);
#if 0
    graph->offloadGraph(false);
#endif
  }

  return TCL_OK;
}

static int
graphCreateCmd(ClientData cd,
	       Tcl_Interp *interp,
	       int argc,
	       char const *argv[])
{
  Initialize(graphCreateCmd(ClientData, Tcl_Interp *, int, char const **));

  Interpreter *i = (Interpreter *) cd;
  ostream &os = i->GetOutputStream();
  symbolArr results;

  if (i->ParseArguments(1, argc, argv, results) != 0)
    return TCL_ERROR;

  for (int k = 0; k < results.size(); k++) {
    symbolPtr symbol = results[k];
    ddKind kind = symbol.get_kind();

    if (kind != DD_FUNC_DECL)
      return TCL_ERROR;

    if (!GDSet::Hashtable_)
      GDSet::loadStandardFunctions();

    GDGraph *graph = GDSet::Hashtable_->find(symbol, false);

    if (graph)
      return TCL_ERROR;

    scopeNode *scope = symbol.get_scope();
    GDHeader *header = new GDHeader(symbol.get_offset());
    graph = new GDGraph(header, scope);
    GDSet::Hashtable_->add(graph, symbol);
//    graph->setStatus(GDT_GLOBAL);
  }

  return TCL_OK;
}

static int
graphAddCmd(ClientData cd,
	    Tcl_Interp *interp,
	    int argc,
	    char const *argv[])
{
  Initialize(graphAddCmd(ClientData, Tcl_Interp *, int, char const **));

  Interpreter *i = (Interpreter *) cd;
  ostream &os = i->GetOutputStream();

  if (argc < 6) {
    os << "Usage: graph add <source argument> <source level> <target argument> <target level> [<function>]" << endl;
    return TCL_OK;
  }

  symbolArr results;
  if (i->ParseArguments(5, argc, argv, results) != 0)
    return TCL_ERROR;

  if (results.size() == 0)
    return TCL_OK;

  if (!GDSet::Hashtable_)
    GDSet::loadStandardFunctions();

  symbolPtr symbol = results[0];
  GDGraph *graph = GDSet::Hashtable_->find(symbol, false);

  if (!graph)
    return TCL_ERROR;

  int srcLevel, trgLevel, srcArgno, trgArgno;

  if (Tcl_GetInt(interp, argv[1], &srcArgno) != TCL_OK ||
      Tcl_GetInt(interp, argv[3], &trgArgno) != TCL_OK ||
      Tcl_GetInt(interp, argv[2], &srcLevel) != TCL_OK ||
      Tcl_GetInt(interp, argv[4], &trgLevel) != TCL_OK) {
    os << "Illegal arguments." << endl;
    return TCL_ERROR;
  }

  int maxArgno = srcArgno > trgArgno ? srcArgno : trgArgno;
  GDHeader *header = graph->getHeader();

  if (maxArgno > header->getArgno())
    header->setArgno(maxArgno);

  dataCell_type srcType = dataCell_Argument;
  dataCell_type trgType = dataCell_Argument;

  if (srcArgno < 0) {
    srcType = dataCell_Result;
    srcArgno = 0;
  }

  if (trgArgno < 0) {
    trgType = dataCell_Result;
    trgArgno = 0;
  }

  dataCell *source = new dataCell(srcLevel, srcArgno,
				  srcType | dataCell_Abstract);
  dataCell *target = new dataCell(trgLevel, trgArgno,
				  trgType | dataCell_Abstract);

  hashArrInit = false;
  source->set_symbol(symbol);
  target->set_symbol(symbol);
  graph->loadGraph(true);
  graph->addRelation(source, target, GDR_GLOBAL | GDR_IRREDUCIBLE,
		     true, true, true);
  graph->offloadGraph(false);
  hashArrInit = true;
  return TCL_OK;
}

static int
graphComputeCmd(ClientData cd,
		Tcl_Interp *interp,
		int argc,
		char const *argv[])
{
  Initialize(graphComputeCmd(ClientData, Tcl_Interp *, int, char const **));

  Interpreter *i = (Interpreter *) cd;
  symbolArr results;

  if (i->ParseArguments(1, argc, argv, results) != 0)
    return TCL_ERROR;

  int size = results.size();
  objSet graphSet;
  Obj *defn;

  for (int k = 0; k < size; ++k) {
    symbolPtr symbol = results[k];
    defn = GDSet::search(symbol, graphSet, false);
    GDGraph *graph = GDSet::search(symbol, false);

    if (graph && !graphSet.includes(graph))
      graphSet.insert(graph);
  }

  ForEach(defn, graphSet) {
    GDGraph *graph = (GDGraph *) defn;
    graph->closeGraph(true, true);
  }

  return TCL_OK;
}

static int
graphLoadCmd(ClientData cd,
	     Tcl_Interp *interp,
	     int argc,
	     char const *argv[])
{
  Initialize(graphLoadCmd(ClientData, Tcl_Interp *, int, char const **));

  Interpreter *i = (Interpreter *) cd;
  symbolArr results;

  if (i->ParseArguments(1, argc, argv, results) != 0)
    return TCL_ERROR;

  int size = results.size();
  objSet graphSet;
  Obj *defn;

  for (int k = 0; k < size; ++k) {
    symbolPtr symbol = results[k];
    defn = GDSet::search(symbol, graphSet, false);
    GDGraph *graph = GDSet::search(symbol, false);

    if (graph && !graphSet.includes(graph))
      graphSet.insert(graph);
  }

  ForEach(defn, graphSet) {
    GDGraph *graph = (GDGraph *) defn;
    graph->loadGraph(true, true, true);
  }

  return TCL_OK;
}

static int
graphUnloadCmd(ClientData cd,
	       Tcl_Interp *interp,
	       int argc,
	       char const *argv[])
{
  Initialize(graphUnloadCmd(ClientData, Tcl_Interp *, int, char const **));

  Interpreter *i = (Interpreter *) cd;
  symbolArr results;

  if (i->ParseArguments(1, argc, argv, results) != 0)
    return TCL_ERROR;

  int size = results.size();
  objSet graphSet;
  Obj *defn;

  for (int k = 0; k < size; ++k) {
    symbolPtr symbol = results[k];
    defn = GDSet::search(symbol, graphSet, false);
    GDGraph *graph = GDSet::search(symbol, false);

    if (graph && !graphSet.includes(graph))
      graphSet.insert(graph);
  }

  ForEach(defn, graphSet) {
    GDGraph *graph = (GDGraph *) defn;
    graph->offloadGraph(false, NULL, true);
  }

  return TCL_OK;
}

static int
graphCmd(ClientData cd, 
	 Tcl_Interp *interp, 
	 int argc, 
	 char const *argv[])
{
  Initialize(graphCmd(ClientData, Tcl_Interp *, int, char const **));

  dfaLoader incremental;
  Interpreter *i = (Interpreter *) cd;
  ostream &os = i->GetOutputStream();

  if (argc == 1) {
    os << "Usage: graph [add create compute relations] <arguments>" << endl;
    return TCL_OK;
  }

  char const *cmd = argv[1];
  
  if (!strcmp(cmd, "relations")) {
    return
      graphRelationsCmd(cd, interp, argc - 1, argv + 1);
  } else if (!strcmp(cmd, "create")) {
    return
      graphCreateCmd(cd, interp, argc - 1, argv + 1);
  } else if (!strcmp(cmd, "add")) {
    return
      graphAddCmd(cd, interp, argc - 1, argv + 1);
  } else if (!strcmp(cmd, "compute")) {
    return
      graphComputeCmd(cd, interp, argc - 1, argv + 1);
  } else if (!strcmp(cmd, "load")) {
    return
      graphLoadCmd(cd, interp, argc - 1, argv + 1);
  } else if (!strcmp(cmd, "unload")) {
    return
      graphUnloadCmd(cd, interp, argc - 1, argv + 1);
  } else {
    os << "Wrong graph subcommand.\nUsage: graph [add compute create load unload] <arguments>" << endl;
    return TCL_OK;
  }
}

int 
ast_get_instances(const symbolPtr &ast, 
		  symbolArr &results)
{    
  Initialize(ast_get_instances(const symbolPtr &, symbolArr &));

  symbolArr res;
  astNode *node = astNode_of_sym(ast);

  if (!node)
    return 0;

  res.insert_last(ast);
  node_get_instance(res, results);
  return 1;
}

extern int (*ast_instances)(const symbolPtr &ast, symbolArr &results);
extern bool (*ast_instance)(const symbolPtr &ast, symbolPtr &inst);

extern int (*ast_declaration)(const symbolPtr &ast);

struct tpmChangeFile {

  tpmChangeFile()
    : file(NULL), os(NULL), tpm(NULL), mod(NULL), count(-1), status(IDLE) {}

  bool init(char const *f, char const *t, projModule *m);
  bool open();
  int close();
  void put(char const *);
  genString file;
  ostream *os;
  genString tpm;
  projModule *mod;
  int count;
  enum {IDLE, INIT, OPEN} status;   
  ocharstream cmd_buff;  // for cmd_validate()
} current_output;

bool
tpmChangeFile::init(char const *f,
		    char const *t,
		    projModule *m)
{
  Initialize(tpmChangeFile::init(char const *, char const *, projModule *));

  if (status != IDLE)
      return false;

  file = f;
  tpm = t;
  mod = m;
  count = 0;
  status = INIT;

  return true;

}

void
tpmChangeFile::put(char const *dif)
{
  Initialize(tpmChangeFile::put(char const *));

  if (status == IDLE)
    return;
  if (status == INIT) 
    open();

  char ch;
  for(char const*p=dif; ch=*p; ++p){
    (*os) << ch;
    if (ch == '\n')
      (*os) << "> ";  // continuation line
  }
  (*os) << endl;
  ++ count;

  CMD_GROUP(SMT){
    cmd_buff << dif << endl;
  }

}

bool
tpmChangeFile::open()
{
  Initialize(tpmChangeFile::open());
  Assert(status == INIT);
  os = new ofstream(file, ios::out);
  status = OPEN;
  count = 0;
  char const *comm = "# ";
  (*os) << comm << "ds: ";
  char ch;
  char *p = (char *) tpm;

  // skip unimportant characters
  int space = ' ';
  for (; (ch = *p); ++p)
    if (ch > space)
      break;
  for (; (ch = *p); ++p) {
    if (ch >= space)
      (*os) << ch;
  }

  (*os) << endl;
  (*os) << comm << "fn: " << mod->get_phys_filename() << endl;
  (*os) << comm << "ps: " << mod->paraset_file_name() << endl;

  CMD_GROUP(SMT){
    cmd_buff.reset();
  }

  return ! os->bad();
}

extern "C"  char *smt_truncate_filename(char *name);

int
tpmChangeFile::close()
{
  Initialize(tpmChangeFile::close());

  int c = count;
  delete os;
  os = NULL;

  CMD_GROUP(SMT){
    char *fname = smt_truncate_filename(file);
    cmd_buff << ends;
    char *the_ptr = cmd_buff.ptr();
    cmd_validate(fname, the_ptr);
    cmd_buff.reset();
  }


  status = IDLE;
  count = 0;

  file = NULL;
  tpm = NULL;
  mod = NULL;
  count = 0;

  return c;
}

  
static int
diff_startCmd(Interpreter *i,
	      int argc,
	      char const **argv)
{
  Initialize(diff_startCmd(Interpreter *, int, char const *));

  // get a license for TPM remediation
  static bool has_tpm_remed_license = false;
  static bool dont_warn_again = false;

  if ( ! has_tpm_remed_license && ! dont_warn_again )
    {
      if ( _lf(LIC_TPM_REMED) == 0 )
	{
	  if ( _lo(LIC_TPM_REMED) )
	    {
	      _lm(LIC_TPM_REMED);
	      return TCL_ERROR;
	    }
	  else
	    has_tpm_remed_license = true;
	}
      else
	{
	  _lm(LIC_TPM_REMED);
	  dont_warn_again = true;
	  return TCL_ERROR;
	}
    }

  IF(argc < 3)
    return TCL_ERROR;

  char const *file = argv[1];
  char const *tpm = argv[2];
  symbolArr arr;

  if (i->ParseArguments(3, argc, argv, arr) != 0)
    return TCL_ERROR;

  int sz = arr.size();
  //Assert(sz == 1);
  if(sz != 1) {
    genString err;
    err.printf("Error: wrong selection.");    
    Tcl_SetResult(i->interp, err, TCL_VOLATILE);
    return TCL_ERROR;
  }
  projModule *mod = symbol_get_module(arr[0]);
  //Assert(mod);
  if(!mod) {
    genString err;
    err.printf("Error: wrong module.");    
    Tcl_SetResult(i->interp, err, TCL_VOLATILE);
    return TCL_ERROR;
  }
  
  current_output.init(file, tpm, mod);

  return TCL_OK;  
}

static int
diff_finishCmd(Interpreter *i,
	       int argc,
	       char const **argv)
{
  Initialize(diff_finishCmd(Interpreter *, int, char const *));

  int count = current_output.close();
  char buf[9];
  sprintf(buf, "%d", count);
  Tcl_SetResult(i->interp, buf, TCL_VOLATILE);

  return TCL_OK;
}

static int
diff_process_replacement(Interpreter *i,
			 int off,
			 int len,
			 char const *replacement)
{
  Initialize(diff_process_replacement(Interpreter *, int, int, char const *));

  // find appropriate separator
  static char const *separators = "/|%!^:;'\"`~+=-*#@&\1\2\3";
  int ch;

  for (char const *p = separators; ch = *p; ++p) {
    if (strchr(replacement, ch) == 0)
      break;
  }

  Assert(ch);
  genString res;
  res.printf("%d,%d,%c%s%c", off, len, ch, replacement, ch);
  current_output.put(res);

  Tcl_SetResult(i->interp, res, TCL_VOLATILE);
  return TCL_OK;
}

static int
src_replaceCmd(Interpreter *i,
	       int argc,
	       char const **argv)
{
  Initialize(src_replaceCmd(Interpreter *, int, char const **));

  IF(argc==1)
    return TCL_ERROR;

  char const *replacement = argv[1];
  symbolArr arr;

  if (i->ParseArguments(2, argc, argv, arr) != 0)
    return TCL_ERROR;

  int sz = arr.size();
  int off, len;

  if (sz == 1) {
    off = ATT_ast_offset(arr[0]);
    len = ATT_ast_length(arr[0]);
  } else if (sz == 2) {
    off = ATT_ast_offset(arr[0]);
    int end = ATT_ast_offset(arr[1]) + ATT_ast_length(arr[1]);
    len = end - off;
  } else
    return TCL_ERROR;

  return
    diff_process_replacement(i, off, len, replacement);
}

static char const *
diff_process_format(Interpreter *i,
		    int argc,
		    char const **argv,
		    symbolArr &arr)
{
  Initialize(diff_process_format(Interpreter *, int, char const **, symbolArr &));

  static ocharstream buf;
  static symbolAttribute *name = symbolAttribute::get_by_name("name");
  static symbolAttribute *cname = symbolAttribute::get_by_name("cname");
  static symbolAttribute *text = symbolAttribute::get_by_name("ast_text");
  static symbolAttribute *before = symbolAttribute::get_by_name("ast_spaces_before");
  static symbolAttribute *after = symbolAttribute::get_by_name("ast_spaces_after");

  char const *format = argv[1];
  int cur_ind = 2;
  buf.reset();
  char ch;

  for (char const *p = format; (ch = *p); ++p) {
    if (ch == '$') {
      char const *np = NULL;
      char const *val = Tcl_ParseVar(i->interp, p, &np);
      Assert(np);
      p = np - 1;
      buf << val;
    } else if (ch == '\\') {
      ch = * ++p;
      if (ch == 0)
	break;
      switch(ch){
      case '\\':
	buf << '\\';
	break;
      case 't':
	buf << '\t';
	break;
      case 'r':
	buf << '\r';
	break;
      case 'n':
	buf << '\n';
	break;
      case 'f':
	buf << '\f';
	break;
      default: 
	buf << ch;
	break;
      }
    } else if (ch == '%') {
      ch = * ++p;
      if (ch == 0)
	break;
      else if (ch == 's') {
	Assert(cur_ind < argc);
	char const*arg = argv[cur_ind];
        buf << arg;
	++cur_ind;
        continue;
      } 

      symbolAttribute *att = NULL;
      switch(ch){
      case 'a':
	att = after;
	break;
      case 'b':
	att = before;
	break;
      case 'c':
	att = cname;
	break;
      case 'n':
	att = name;
	break;
      case 't':
	att = text;
	break;
      default:
	att = NULL;
	break;
      }
      if (att) {
	Assert(cur_ind < argc);
        symbolArr arg;
	if (i->ParseArguments(cur_ind, cur_ind+1, argv, arg) != 0)
	  return NULL;
	Assert(arg.size() == 1);
	buf << att->value(arg[0]);
	++cur_ind;
      } else {
	buf << ch;
      }
    } else {
      buf << ch;
    }
  }

  buf << ends;

  if (i->ParseArguments(cur_ind, argc, argv, arr) != 0)
    return NULL;

  return
    buf.ptr();
}
 
static int
src_freplaceCmd(Interpreter *i,
		int argc,
		char const **argv)
{
  Initialize(src_replaceCmd(Interpreter *, int, char const **));

  IF(argc==1)
    return TCL_ERROR;

  symbolArr arr;
  int ind = 0;
  char const *replacement = diff_process_format(i, argc, argv, arr);

  IF(!replacement) 
    return TCL_ERROR;

  int sz = arr.size();
  int max_ind = sz - 1;
  int off, len;

  if (ind == max_ind) {
    off = ATT_ast_offset(arr[ind]);
    len = ATT_ast_length(arr[ind]);
  } else if (ind == (max_ind - 1)) {
    off = ATT_ast_offset(arr[ind]);
    int end = ATT_ast_offset(arr[max_ind]) + ATT_ast_length(arr[max_ind]);
    len = end - off;
  } else
    return TCL_ERROR;

  return
    diff_process_replacement(i, off, len, replacement);
}

static int
src_deleteCmd(Interpreter *i,
	      int argc,
	      char const **argv)
{
  Initialize(src_deleteCmd(Interpreter *, int, char const **));

  char const*replacement = "";
  symbolArr arr;

  if (i->ParseArguments(1, argc, argv, arr) != 0)
    return TCL_ERROR;

  int sz = arr.size();
  int off, len;

  if (sz == 1) {
    off = ATT_ast_offset(arr[0]);
    len = ATT_ast_length(arr[0]);
  } else if (sz == 2) {
    off = ATT_ast_offset(arr[0]);
    int end = ATT_ast_offset(arr[1]) + ATT_ast_length(arr[1]);
    len = end - off;
  } else
    return TCL_ERROR;

  return
    diff_process_replacement(i, off, len, replacement);
}

static int
src_insert_beforeCmd(Interpreter *i,
		     int argc,
		     char const **argv)
{
  Initialize(src_diffCmd(Interpreter *, int, char const **));

  IF(argc == 1)
    return TCL_ERROR;

  char const *replacement = argv[1];
  symbolArr arr;

  if (i->ParseArguments(2, argc, argv, arr) != 0)
    return TCL_ERROR;

  int sz = arr.size();
  int off, len;

  if (sz == 1) {
    off = ATT_ast_offset(arr[0]);
    len = 0;
  } else
    return TCL_ERROR;

  return
    diff_process_replacement(i, off, len, replacement);
}

static int
src_finsert_beforeCmd(Interpreter *i,
		      int argc,
		      char const **argv)
{
  Initialize(src_finsert_beforeCmd(Interpreter *, int, char const **));

  IF(argc == 1)
    return TCL_ERROR;

  int ind = 0;
  symbolArr arr;
  char const *replacement = diff_process_format(i, argc, argv, arr);

  IF(!replacement)
    return TCL_ERROR;

  int sz = arr.size();
  int max_ind = sz -1;
  int off, len;

  if (ind == max_ind) {
    off = ATT_ast_offset(arr[ind]);
    len = 0;
  } else
    return TCL_ERROR;

  return
    diff_process_replacement(i, off, len, replacement);
}

static int
src_insert_afterCmd(Interpreter *i,
		    int argc,
		    char const **argv)
{
  Initialize(src_diffCmd(Interpreter *, int, char const **));

  IF(argc == 1)
    return TCL_ERROR;

  char const *replacement = argv[1];
  symbolArr arr;

  if (i->ParseArguments(2, argc, argv, arr) != 0)
    return TCL_ERROR;

  int sz = arr.size();
  int off, len;

  if (sz == 1) {
    off = ATT_ast_offset(arr[0]) + ATT_ast_length(arr[0]);
    len = 0;
  } else
    return TCL_ERROR;

  return
    diff_process_replacement(i, off, len, replacement);
}

static int
src_finsert_afterCmd(Interpreter *i,
		     int argc,
		     char const **argv)
{
  Initialize(src_diffCmd(Interpreter *, int, char const **));

  IF(argc == 1)
    return TCL_ERROR;

  symbolArr arr;
  int ind = 0;
  char const *replacement = diff_process_format(i, argc, argv, arr);

  IF(!replacement)
    return TCL_ERROR;

  int sz = arr.size();
  int max_ind = sz - 1;
  int off, len;

  if (ind == max_ind) {
    off = ATT_ast_offset(arr[ind]) + ATT_ast_length(arr[ind]);
    len = 0;
  } else
    return TCL_ERROR;

  return
    diff_process_replacement(i, off, len, replacement);
}

static int add_commands()
{
  Initialize(add_commands());

  ast_instances = ast_get_instances;
  ast_instance  = node_get_instance;
  ast_declaration = ast_is_declaration;

  new cliCommandInfo("tree", treeCmd);
  new cliCommandInfo("data", dataCmd);
  new cliCommandInfo("dfa_unload", dfa_unloadCmd);
  new cliCommandInfo("pattern", patternCmd);
  new cliCommandInfo("pattern_region", pattern_regionCmd);
  new cliCommandInfo("graph", graphCmd);
  new cliCommand("diff_replace", src_replaceCmd);
  new cliCommand("diff_replacef", src_freplaceCmd);
  new cliCommand("diff_delete", src_deleteCmd);
  new cliCommand("diff_insert_before", src_insert_beforeCmd);
  new cliCommand("diff_insert_after", src_insert_afterCmd);
  new cliCommand("diff_insertf_before", src_finsert_beforeCmd);
  new cliCommand("diff_insertf_after", src_finsert_afterCmd);
  new cliCommand("diff_start", diff_startCmd);
  new cliCommand("diff_finish", diff_finishCmd);
  new cliCommand("get_context_symbol", symbol_contextCmd);

  return 0;
}

static int astNode_init_expressions_internal(char *array, ...) 
{
  va_list ap;
  va_start(ap,array);
  int code = va_arg(ap, int);

  while (code >= 0) {
    array[code] = 1;
    code = va_arg(ap, int);
  }

  va_end(ap);
  
  return 1;
}

int astNode_init_expressions(char* array)
{
  return astNode_init_expressions_internal(array, EXPR, -1);
}

static int add_commands_dummy = add_commands();
