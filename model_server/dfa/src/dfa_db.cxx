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
#include "dfa_db.h"

#ifndef ISO_CPP_HEADERS
#include "iostream.h"
#else /* ISO_CPP_HEADERS */
#include <iostream>
using namespace std;
#endif /* ISO_CPP_HEADERS */

#include "cLibraryFunctions.h"
#include "db.h"
#include "ddict.h"
#include "dfa.h"
#include "dfa_graph.h"
#include "msg.h"
#include "objArr_Int.h"
#include "scopeMgr.h"
#include "smt.h"
#include "symbolArr.h"
#include "symbolPtr.h"

init_relation(astRoot_of_smtHeader,1,relationMode::D,smtHeader_of_astRoot,1,0);
init_relational(astRoot,appTree);
init_relational(astXref,Relational);

objTree *db_get_node_from_id(int id);
int db_get_restore_version();

static objArr_Int *conversion_array;
static objNameSet ast_xrefs;

static astNode *dfa_restore_tree(db_buffer &dst, astNode *parent, astNode *prev, db_buffer &src);
static void dfa_save_tree(astNode *node, db_buffer &dst);
static astNode *dfa_make_restore_node(db_buffer &, tree_code, astNode *, astNode *, unsigned int, unsigned int, unsigned int);

smtTree * els_cn_na(smtHeader*h, int cn);

astRoot::astRoot(smtHeader *head) 
  : buffer(0), dfa_root(NULL), tokens(NULL), _size(0)
{
  Initialize(astRoot::astRoot(smtHeader*));
  put_relation(astRoot_of_smtHeader,head,this);

  xref = checked_cast(astXref,ast_xrefs.lookup(head->get_filename()));
    
  if (!xref) {
    symbolPtr sym = head;
    symbolPtr xsym = sym.get_xrefSymbol();
    if (xsym.isnotnull())
      sym = xsym;
    xref = new astXref(sym, this);
  } else
    xref->set_ast_table(this);
}

astRoot * 
astRoot::get_create_astRoot(smtHeader *head, 
			    db_buffer *buf)
{
  Initialize(astRoot::get_create_astRoot);

  astRoot *root = checked_cast(astRoot,get_relation(astRoot_of_smtHeader,head));
      if (root)
    root->reset();
  else
    root = new astRoot(head);
				 
  if (buf) {
    root->buffer = buf;
    root->_size = buf->size();
    root->dfa_root = (astNode *) ((*buf)[0] + astNode::get_node_size(MC_ERROR_NODE));
  }

  return root;
}

void 
astRoot::reset()
{
  Initialize(astRoot::reset);

  delete buffer;
  _size = 0;
  dfa_root = 0;

  Obj* el;
  ForEach(el, dataCells)
    delete el;

  dataCells.removeAll();
  data.reset();
  smts.arr.removeAll();
  smts.removeAll();

  if (tokens) {
    delete tokens;
    tokens = NULL;
  }
}

astRoot::~astRoot()
{
  Initialize(astRoot::~astRoot);

  reset();
  if (xref)
    xref->notify_unload();
}

app * 
astRoot::get_main_header()
{
  Initialize(astRoot::get_main_header);

  return 
    checked_cast(app,get_relation(smtHeader_of_astRoot,this));
}

astRoot::astRoot(const astRoot &)
{
  msg("astRoot::astRoot(const astRoot &) called", warning_sev) << eom;
}

astRoot * 
astRoot::db_restore(app *h,
		    appTree *parent,
		    db_app &data,
		    db_buffer &src)
{
  Initialize(astRoot::db_restore);

  smtHeader *head = checked_cast(smtHeader,h);
  astRoot *root = get_create_astRoot(head);

  root->_size = data.val;
  root->buffer = new db_buffer;
  if (db_get_restore_version() < 5) {
    root->_size = (root->_size * 4) / 3;
    conversion_array = new objArr_Int;
  } else
    conversion_array = NULL;
  root->buffer->provide(root->_size);
  dfa_make_restore_node(*root->buffer, MC_ERROR_NODE, 0, 0, 1, 1, 0);
  astNode* dfa_root = dfa_restore_tree(*root->buffer, 0, 0, src);
  if (conversion_array)
    delete conversion_array;
  byte* oldbuf = (*root->buffer)[0];
  root->buffer->compress();
  byte* newbuf = (*root->buffer)[0];
  root->dfa_root = (astNode*) ((byte*)dfa_root + (newbuf - oldbuf));
  root->_size = root->buffer->size();
  dfa_print_tree(root->buffer, head);

  return root;
}

static astNode * 
dfa_restore_tree(db_buffer &dst, 
		 astNode *parent, 
		 astNode *prev, 
		 db_buffer &src)
{
  Initialize(dfa_restore_tree);

  astNode *node = NULL;

  byte temp[8];
  src.get(temp, 8);
  unsigned int code = temp[0];
  unsigned int start = (((temp[1] << 8) | temp[2]) <<8 ) | temp[3];
  unsigned int length = (((temp[4] << 8) | temp[5]) << 8) | temp[6];
    
  unsigned int is_last = (temp[7] & 0x80) >> 7;
  unsigned int is_leaf = (temp[7] & 0x40) >> 6;
  unsigned int level = temp[7] & 0x3f;

  int value = 0;
  double double_value;

  node = dfa_make_restore_node(dst, (tree_code) code, parent, prev, is_last, is_leaf, level);
  node->set_start(start);
  node->set_length(length);

  switch(code) {

  case MC_INTEGER_CST:
    src.get_int(value);
    ((dfa_integer_cst *) node)->set_value(value);
    break;
	
  case DFA_REF_DFA:
    src.get_int(value);
    if (conversion_array) {
      for(int ii=conversion_array->size()-1; ii>=0; ii--)
	if ((char*)node+value <= (char*) (*conversion_array)[ii])
	  value -= 4;
	else
	  break;
    }
    ((dfa_ref_dfa *) node)->set_decl(value);
    break;
	
  case DFA_REF_DD:
    src.get_int(value);
    if (value) {
      ddElement *dd = checked_cast(ddElement,db_get_node_from_id(value));
      ((dfa_ref_dd *) node)->set_dd(dd);
    }
    break;

  case MC_REAL_CST:
    src.get_double(double_value);
    ((dfa_real_cst *) node)->set_value(double_value);
    break;
	
  case MC_FUNCTION_DECL:
  case MC_CONST_DECL:
  case MC_TYPE_DECL:
  case TYPE_DECL_MODIFIED:
  case MC_VAR_DECL:
  case MC_PARM_DECL:
  case MC_FIELD_DECL:
  case TEMPLATE_FCN:
  case TEMPLATE_CLASS:
  case TEMPLATE_STRUCT:
  case TEMPLATE_UNION:
  case TEMPLATE_STATIC_DATA_MBR:
    src.get_int(value);
    if (value) {
      ddElement *dd = checked_cast(ddElement,db_get_node_from_id(value));
      ((dfa_ref_dd *) node)->set_dd(dd);
    }

    src.get(temp, 4);
    value = (((temp[0] << 8) | temp[1]) <<8 ) | temp[2];
    ((dfa_decl *) node)->set_id(value);
    if (temp[3] & 0x80)
      ((dfa_decl *) node)->set_is_def();
    break;
	
  default:
    if (astNode::is_typed_expression(code))
      if (conversion_array)
	conversion_array->insert_last((int)node);
      else
	{
	  src.get_int(value);
	  if (value) 
	    {
	      ddElement *type = checked_cast(ddElement,db_get_node_from_id(value));
	      node->set_type(type);
	    }
	}
    break;
  }

  if (!is_leaf) {
    astNode *prev = NULL;
    do {
      prev = dfa_restore_tree(dst, node, prev, src);
    } while (!prev->get_is_last());
  }

  return node;
}
 
static astNode *
dfa_make_restore_node(db_buffer &dst, 
		      tree_code code, 
		      astNode *parent, 
		      astNode *prev, 
		      unsigned int is_last, 
		      unsigned int is_leaf, 
		      unsigned int level)
{
  Initialize(dfa_make_restore_node);
    
  astNode *node = (astNode *) dst.grow(astNode::get_node_size(code));
  node->set_code(code);
  node->set_type((ddElement*)0);
  node->set_is_last(is_last);
  node->set_is_leaf(is_leaf);
  node->set_level(level);

  if (is_last && parent)
    node->set_parent(parent);
  if(prev)
    prev->set_next(node);

  return node;
}

void 
astRoot::db_save(app *, 
		 appTree *nd, 
		 db_app &dba, 
		 db_buffer &dbb)
{
    Initialize(astRoot::db_save);
    astRoot *node = checked_cast(astRoot,nd);
    dba.val = node->_size;
    dfa_save_tree(node->dfa_root,dbb);
    return;
}

static void 
dfa_save_tree(astNode *node, 
	      db_buffer &dst)
{
  Initialize(dfa_save_tree);
  byte temp[8];

  unsigned int code = node->get_code();
  unsigned int value = node->get_start();

  temp[0] = code;
  temp[1] = (value >> 16) & 0xff;
  temp[2] =(value >> 8) & 0xff;
  temp[3] = value & 0xff;

  value = node->get_length();

  temp[4] = (value >> 16) & 0xff;
  temp[5] = (value >> 8) & 0xff;
  temp[6] = value & 0xff;
  temp[7] = node->get_level(); // to copy is_last, is_leaf, and type

  if (node->get_is_last())
    temp[7] |= 0x80;
  if (node->get_is_leaf())
    temp[7] |= 0x40;
  
  dst.put(temp, 8);

  value = 0;
  double double_value;
  ddElement *dd;
 
  switch (code) {

  case MC_INTEGER_CST:
    value = ((dfa_integer_cst *) node)->get_value();
    dst.put_int(value);
    break;
	
  case DFA_REF_DFA:
    value = ((dfa_ref_dfa *) node)->get_decl_int();
    dst.put_int(value);
    break;

  case DFA_REF_DD:
    value = 0;
    dd = ((dfa_ref_dd *) node)->get_dd();
    if (dd)
      value = dd->get_id();
    dst.put_int(value);
    break;
	
  case MC_REAL_CST:
    double_value = ((dfa_real_cst *) node)->get_value();
    dst.put_double(double_value);
    break;
	
  case MC_FUNCTION_DECL:
  case MC_CONST_DECL:
  case MC_TYPE_DECL:
  case TYPE_DECL_MODIFIED:
  case MC_VAR_DECL:
  case MC_PARM_DECL:
  case MC_FIELD_DECL:
  case TEMPLATE_FCN:
  case TEMPLATE_CLASS:
  case TEMPLATE_STRUCT:
  case TEMPLATE_UNION:
  case TEMPLATE_STATIC_DATA_MBR:
    value = 0;
    dd = ((dfa_ref_dd *) node)->get_dd();
    if (dd)
      value = dd->get_id();
    dst.put_int(value);
    value = ((dfa_decl *) node)->get_id();

    temp[0] = (value >> 16) & 0xff;
    temp[1] = (value >> 8) & 0xff;
    temp[2] = value & 0xff;
    temp[3] = 0;

    if (((dfa_decl *) node)->get_is_def())
      temp[3] |= 0x80;
    dst.put(temp,4);
    break;
	
  default:
    if (astNode::is_typed_expression(code))
      {
	dd = node->get_type();
	if (dd)
	  value = dd->get_id();
	dst.put_int(value);
      }
    break;
  }

  for (astNode *ch = node->get_first(); 
       ch; 
       ch = ch->get_next())
    dfa_save_tree(ch, dst);
}

astXref::astXref(const symbolPtr &mod, 
		 astRoot *ast) 
  : module(mod), ast_table(ast)
{
  Initialize(astXref::astXref(const symbolPtr&,astRoot*));

  ast_xrefs.insert(this);
}

astXref * 
ast_get_astXref(const char *ln)
{
  Initialize(ast_get_astXref(const char *));
  astXref *axr = (astXref *) ast_xrefs.lookup(ln);

  if (!axr) {
    projNode *project = 0;
    projNode *proj;
    projModule *module=0;
    
    for (int i = 1; proj = projList::search_list->get_proj(i); i++) {
      if(strcmp(proj->get_ln(), "/ParaDOCS") && strcmp(proj->get_ln(), "/tmp"))
	{
	  project = proj->get_pmod_project_from_ln (ln);
	  if (project) {
	    module = project->find_module(ln);
	    if (module)
	      break;
	  }
	}
    }
    
    if (module) {
      symbolPtr symbol = module;
      symbol = symbol.get_xrefSymbol();

      if (symbol.isnotnull())
	axr = new astXref(symbol, NULL);
    }
  }

  return axr;
}

astXref * 
ast_get_astXref(symbolPtr &symbol)
{
  Initialize(ast_get_astXref(symbolPtr &));

  const char *ln = symbol.get_name();
  astXref *axr = (astXref *) ast_xrefs.lookup(ln);

  if (!axr) {
    projNode *project = symbol.get_projNode();

    if (project) {
      projModule *module = project->find_module(ln);
      symbolPtr symbol = module;
      symbol = symbol.get_xrefSymbol();

      if (symbol.isnotnull())
	axr = new astXref(symbol, NULL);
    }
  }

  return axr;
}

astRoot * 
astXref::get_ast_table(bool mode)
{
  Initialize(astXref::get_ast_table);

  if (!ast_table)
    (void) load();

  return ast_table;
}

astRoot * 
astXref::load()
{
  Initialize(astXref::load);
  dfaLoader incremental;
  app *head = module;
  return ast_table;
}

char const * 
astXref::get_name() const
{
  Initialize(astXref::get_name);
    
  return 
    module.get_name();
}

astXref::astXref(const astXref &)
{
  msg("astXref::astXref(const astXref &) should not be called", warning_sev) << eom;
}

astXref::~astXref()
{
  Initialize(astXref::~astXref);
}

void 
astXref::notify_unload()
{
  Initialize(astXref::notify_unload);

  ast_table = NULL;
  module = module.get_xrefSymbol();
  if (module.isnull())
    ast_xrefs.remove(*this);
}

int 
astXref::get_offset(astNode *nd)
{
  Initialize(astXref::get_offset(astNode*));

  return 
    (char *) nd - (char *) ast_table->get_astNode();
}

astNode * 
astXref::get_node_from_offset(int offset)
{
  astNode *node = NULL;
  astRoot *ar = get_ast_table();

  if (ar) {
    astNode *root = ar->get_astNode();
    if (root)
      node = (astNode *) ((char *) root + offset);
  }

  return node;
}

int 
astXref::generate_array(astNode *root, 
			symbolArr &res)
{
  Initialize(astXref::generate_array(astNode*,symbolArr&));

  if (root) {
    symbolPtr sym(root, this);
    res.insert_last(sym);

    for (astNode *ch = root->get_first();
	 ch;
	 ch = ch->get_next())
      generate_array(ch,res);
  }

  return res.size();
}

symbolPtr 
astXref::get_module()
{
  return module;
}

static int 
compare_codes(int astcode, int ddkind)
{
  int retval = 1;
  
  switch (ddkind) {
  case DD_FUNC_DECL:
    retval = (astcode != MC_FUNCTION_DECL);
    break;
  case DD_VAR_DECL:
    retval = (astcode != MC_VAR_DECL);
    break;
  case DD_CLASS:
    retval = (astcode != MC_TYPE_DECL &&
              astcode != TYPE_DECL_MODIFIED);
    break;
  case DD_TEMPLATE:
    retval = (astcode != TEMPLATE_FCN &&
	      astcode != TEMPLATE_CLASS &&
	      astcode != TEMPLATE_STRUCT &&
	      astcode != TEMPLATE_UNION &&
	      astcode != TEMPLATE_STATIC_DATA_MBR);
    break;
    
  default:
    break;
  }

  return retval;
}

static int 
get_astNodes_internal(astXref *that, 
		      ddElement *dd, 
		      astNode *root, 
		      symbolArr &result)
{
  int retval = 0;

  if (!compare_codes(root->get_code(), dd->get_kind()) &&
      ((dfa_decl *) root)->get_dd() == dd &&
      ((dfa_decl *) root)->get_is_def()) {
    result.insert_last(symbolPtr(root, that));
  } else {
    for (astNode *ch = root->get_first();
	 ch;
	 ch = ch->get_next())
      get_astNodes_internal(that, dd, ch, result);
  }

  retval = result.size();
  return retval;
}

int 
astXref_get_astNodes(astXref *axr, 
		     ddElement *dd, 
		     symbolArr &result)
{
  return 
    axr->get_astNodes(dd, result);
}

int 
astXref::get_astNodes(ddElement *dd, 
		      symbolArr &result)
{
  int retval = 0;
  astRoot *ar = get_ast_table();

  if (ar) { 
    astNode *root = ar->get_astNode();
    if (root) {
      switch (dd->get_kind()) {

      case DD_MODULE:
	result.insert_last(symbolPtr(root, this));
	break;
		
      case DD_VAR_DECL:
      case DD_CLASS:
 	  case DD_TEMPLATE:

	get_astNodes_internal(this, dd, root, result);
	break;

      case DD_FUNC_DECL: {
	symbolArr arr;
	get_astNodes_internal(this, dd, root, arr);

	if (arr.size() > 0) {
	  const symbolPtr &decl = arr[0];
	  astNode *node = astNode_of_sym(decl);
	  astNode *parent = node->get_parent();
	  if (parent->get_code() == DFA_FUNCTION_DEF) {
	    astXref *arf = decl.get_astXref();
	    result.insert_last(symbolPtr(parent, arf));
	  } else {
	    result.insert_last(symbolPtr(decl));
	  }
	}

	break;
      }

      default:
	break;
      }
    }
  }
    
  retval = result.size();
  return retval;
}

astXref * 
app_get_astXref(app *head)
{
  astXref *retval = NULL;
  astRoot *ar = (astRoot *) get_relation(astRoot_of_smtHeader, head);

  if (ar)
    retval = ar->get_xref();

  return retval;
}

astNode * 
astNode_of_sym(const symbolPtr &sym)
{
  astNode *retval = NULL;
  
  if (sym.is_ast()) {
    astXref *axr = sym.get_astXref();
    int real_offset = sym.get_offset() & 0x00ffffff;
    retval = axr->get_node_from_offset (real_offset);
  } else if (sym.is_dataCell()) {
    dataCell *cell = dataCell_of_symbol(sym, false);

    if (cell) {
      symbolPtr symbol = cell->get_symbol();
      retval = astNode_of_sym(symbol);
      dataCell_offload(cell, false);
    }
  }

  return retval;
}

int 
astNode_get_char_offset(const symbolPtr &sym)
{
  int retval = 0;
  astNode *node = astNode_of_sym(sym);

  if (node)
    retval = node->get_start();

  return retval;
}

int 
astNode_get_line_number(const symbolPtr &sym)
{
  int retval = 0;
  astNode *node = astNode_of_sym(sym);
  astXref *axr = sym.get_astXref();

  if (axr) {
    astRoot *ar = axr->get_ast_table();
    if (ar) {
      smtHeader *sh = (smtHeader *) ar->get_main_header();
      if (sh) {
	sh->last_token  = 0;
	int start = node->get_start();
	smtTree *smt = sh->cn_na(start);
	if (smt)
	  retval = smt->get_linenum();
      }
    }
  }

  return retval;
}

// display string in browser list
char * 
astNode_get_src_info(symbolPtr &msym)
{
  static genString info;

  if (msym.is_ast()) {
    astXref *axr = msym.get_astXref();
    symbolPtr mod = axr->get_module(); 

    char const *mname = mod.get_name();
    if (!mname)
      mname = "<null>";
    int off = msym.get_offset() & 0x00ffffff;
    info.printf("[%s : %d]", mname, off);
  } else
    info = msym.get_name();

  return 
    (char *) info;
}

void 
symbolPtr_sym_of_astNode(const symbolPtr &that, 
			 symbolPtr &result)
{
  if (that.is_dataCell()) {
    dataCell *cell = dataCell_of_symbol(that, false);

    if (cell) {
      symbolPtr symbol = cell->get_symbol();

      if (symbol.is_ast())
	symbolPtr_sym_of_astNode(symbol, result);

      dataCell_offload(cell, false);
    }
  } else if (that.is_ast()) {
    astXref *axr = that.get_astXref();
    astNode *node = astNode_of_sym(that);

    if (node) {
      ddSymbol *dd = NULL;
      switch (node->get_code()) {

      case DFA_ROOT:
	result = axr->get_module();
	result = result.get_xrefSymbol();
	break;
		
      case DFA_REF_DFA: {
	dfa_decl *dcl = ((dfa_ref_dfa *) node)->get_decl();
	symbolPtr sym(dcl,axr);
	result = sym.sym_of_astNode();
	break;
      }
		
      case DFA_FUNCTION_DEF: {
	astNode *fdecl = NULL;
	astNode *spec = node->get_first();

	if (spec)
	  fdecl = spec->get_next();
	if (fdecl && fdecl->get_code()==MC_FUNCTION_DECL)
	  dd = (ddSymbol *) ((dfa_ref_dd *) fdecl)->get_dd();
	if (dd)
	  result = dd->get_xrefSymbol();
      }
	break;
		
      case MC_CALL_EXPR: {
	astNode *fun = node->get_first();
	if (fun && fun->get_code() == DFA_REF_DD)
	  node = fun;
	else
	  break;
      }
	/* fall throw */

      case DFA_REF_DD:
      case MC_FIELD_DECL:
      case MC_VAR_DECL:
      case MC_PARM_DECL:
      case MC_FUNCTION_DECL:
      case MC_TYPE_DECL:
      case TYPE_DECL_MODIFIED:
      case TEMPLATE_FCN:
      case TEMPLATE_CLASS:
      case TEMPLATE_STRUCT:
      case TEMPLATE_UNION:
      case TEMPLATE_STATIC_DATA_MBR:
	dd = (ddSymbol *) ((dfa_ref_dd *) node)->get_dd();
	if (dd)
	  result = dd->get_xrefSymbol();
	break;
		
      default:
	break;
      }
    }
  }
}

int 
astXref_get_offset(astNode *nd, 
		   astXref *axr)
{
  return 
    axr->get_offset(nd);
}

symbolPtr 
astXref_get_module(astXref *axr)
{
  return 
    axr->get_module();
}

int 
astNode_start(astNode *node)
{
  return 
    node->get_start();
}

int 
astNode_length(astNode *node)
{
  return 
    node->get_length();
}

typedef astNode* (astNode::*Navigate)();

static int 
tree_navigate(Navigate mp, 
	      const symbolPtr &curr, 
	      symbolPtr &dest)
{
  int retval = 0; // failure
    
  if (curr.is_ast()) {
    astNode *curnode = astNode_of_sym(curr);
    if (curnode) {
      astNode *dest_node = (curnode->*mp)();

      if (dest_node) {
	astXref *axr = curr.get_astXref();
	symbolPtr res(dest_node, axr);
	dest = res;
	retval = 1; // success
      }
    }
  }
    
  if (!retval)
    dest = NULL_symbolPtr;

  return retval;
}
    
int 
tree_get_parent(const symbolPtr &curr, 
		symbolPtr &parent)
{
  return 
    tree_navigate(&astNode::get_parent, curr, parent);
}

int 
tree_get_next(const symbolPtr &curr, 
	      symbolPtr &next)
{
  return 
    tree_navigate(&astNode::get_next, curr, next);
}

int 
tree_get_previous(const symbolPtr &curr, 
		  symbolPtr &prev)
{
  return 
    tree_navigate(&astNode::get_prev, curr, prev);
}

int 
tree_get_first(const symbolPtr &curr, 
	       symbolPtr &first)
{
  return 
    tree_navigate(&astNode::get_first, curr, first);
}

symbolPtr
astNode_get_node(char const *lname,
		 int offset)
{
  astXref *axr = ast_get_astXref(lname);
  symbolPtr node;

  if (axr) {
    offset |= 0x40000000;  // mark offset as ast, not xref
    node.put_info((scopeNode *) axr, offset);
  }

  return node;
}

//
// AST MERGE CODE
//

struct Ast_Smt_Table {
  astNode* ast;
  smtTree* smt_start;
  smtTree* smt_end;
};

genArr(Ast_Smt_Table);
static genArrOf(Ast_Smt_Table) conversion_table;

static int ast_merge(smtHeader * h, astNode* ast)
{
  Ast_Smt_Table *slot =  conversion_table.grow();
  slot->ast = ast;
  int start = ast->get_start();
  int end = start + ast->get_length() - 1;
  slot->smt_start = els_cn_na(h, start);
  slot->smt_end =  els_cn_na(h, end);

  for (astNode *ch = ast->get_first(); 
       ch; 
       ch = ch->get_next())
    ast_merge(h, ch);

  return 0;
}

int start_ast_merge(smtHeader *h)
{
  Initialize(start_ast_merge(smtHeader*));
  conversion_table.reset();
  astRoot *root = checked_cast(astRoot,get_relation(astRoot_of_smtHeader,h));
  if (root) {
    smtTree * smt = checked_cast(smtTree,h->get_root());
    int ttablth_old = h->ttablth;
    h->ttablth = smt->tnmax;
    ast_merge(h, root->get_astNode());
    h->ttablth = ttablth_old;
  }
  return 0;
}

int end_ast_merge()
{
  int sz =  conversion_table.size();

  for (int ii = 0; ii < sz; ++ii) {
    Ast_Smt_Table *slot = conversion_table[ii];
    if (slot->smt_start && slot->smt_end && 
	slot->smt_start->get_id() > 0 &&  slot->smt_end->get_id() > 0 ) {
      int start = slot->smt_start->istart();
      slot->ast->set_start(start);
      int end = slot->smt_end->istart() + slot->smt_end->ilength();
      int len = end - start;
      slot->ast->set_length(len);
    } else {
      slot->ast->set_start(0);
      slot->ast->set_length(0);
    }
  }

  return 0;
}
