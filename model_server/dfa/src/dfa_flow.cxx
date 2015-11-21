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
#include <stdlib.h>
#else /* ISO_CPP_HEADERS */
#include <cstdlib>
namespace std {};
using namespace std;
#endif /* ISO_CPP_HEADERS */
#include <cLibraryFunctions.h>


#include "_Xref.h"
#include "customize.h"
#include "dfa_db.h"
#include "dfa_graph.h"
#include "driver_mode.h"
#include "elsLanguage.h"
#include "msg.h"
#include "genArr.h"

genArr(int);
objArr GDSet::currentGraphs_;
classInfo *GDSet::classInfo_ = NULL;
unsigned int dataCell::newCount_ = 0;
unsigned int dataCell::deleteCount_ = 0;
unsigned int dataCell::newAliasCount_ = 0;
unsigned int dataCell::oldAliasCount_ = 0;
unsigned int dataCell::checkedCount_ = 0;

#define DFA_MAX_TYPEDEF_SEARCH 10

static char const *field_depth = OSapi_getenv("DFA_FieldDepth");
unsigned int DFA_NESTED_FIELDS_LIMIT = (field_depth && atoi(field_depth) >= 0)
	                                ? atoi(field_depth) : 2;
static bool DFA_JAVA = false;
static bool warn = false;
static dataCell *delCell = NULL;

init_relational(dataCell,Relational);
init_relational(nodeDataCell,dataCell);
init_relational(callDataCell,nodeDataCell);
init_rel_or_ptr(dataCell,src,0,dataCell,trg,0);
init_rel_or_ptr(dataCell,cell,0,dataCell,alias,0);

#ifdef DBG
#undef DBG
#endif
#define DBG if(flow_debug)

extern char const *testName;
static int flow_debug = (int) OSapi_getenv("DIS_FLOW_DEBUG");
astXref *ast_get_astXref(char const *ln);
astXref *ast_get_astXref(symbolPtr &symbol);
symbolPtr astXref_get_module(astXref *);
char const *ATT_scope(symbolPtr &symbol);
extern "C" void dfa_debug_hook(char const *name);

dataCell::dataCell(unsigned char const *&start,
		   GDGraph *graph)
{
  Initialize(dataCell::dataCell(unsigned char const *&, GDGraph *));

  if (!graph)
    return;

  type_ = *start;
  level_ = *(start + 1);
  maxLevel_ = *(start + 2);
  argno_ = *(start + 3);
  graph_ = NULL;
  field_ = NULL;
  depth_ = 0;

  GDInteger symbolID(start + 4);
  GDInteger classID(start + 4 + GDInteger::DB_SIZE());
  scopeNode *scope = graph->getScope();
  symbol_.put_info(scope, symbolID);
  class_.put_info(scope, classID);

  start += dataCell::DB_SIZE();

  if (type_ & dataCell_Parent) {
    field_ = dataCell::newCell(start, graph);
    unsigned depth = field_->get_depth();
    set_depth(++depth);
  }
}

nodeDataCell::nodeDataCell(unsigned char const *&start,
			   GDGraph *graph)
{
  Initialize(nodeDataCell::nodeDataCell(unsigned char const *&, GDGraph *));

  if (!graph)
    return;

  type_ = *start;
  level_ = *(start + 1);
  maxLevel_ = *(start + 2);
  argno_ = *(start + 3);
  graph_ = NULL;
  field_ = NULL;
  depth_ = 0;

  GDInteger symbolID(start + 4);
  GDInteger classID(start + 4 + GDInteger::DB_SIZE());
  GDInteger xrefID(start + 4 + 2 * GDInteger::DB_SIZE());
  scopeNode *scope = graph->getScope();
  symbol_.put_info(scope, symbolID);
  class_.put_info(scope, classID);
  xref_.put_info(scope, xrefID);

  symbolPtr fileSymbol;
  graph->getFileSymbol(fileSymbol);
  astXref *axr = ast_get_astXref(fileSymbol);
  symbol_.put_info((scopeNode *) axr, symbolID);

  start += nodeDataCell::DB_SIZE();

  if (type_ & dataCell_Parent) {
    field_ = dataCell::newCell(start, graph);
    unsigned depth = field_->get_depth();
    set_depth(++depth);
  }
}

callDataCell::callDataCell(unsigned char const *&start,
			   GDGraph *graph)
{
  Initialize(callDataCell::callDataCell(unsigned char const *&, GDGraph *));

  if (!graph)
    return;

  type_ = *start;
  level_ = *(start + 1);
  maxLevel_ = *(start + 2);
  argno_ = *(start + 3);
  graph_ = NULL;
  field_ = NULL;
  depth_ = 0;

  GDInteger symbolID(start + 4);
  GDInteger classID(start + 4 + GDInteger::DB_SIZE());
  GDInteger xrefID(start + 4 + 2 * GDInteger::DB_SIZE());
  GDInteger contextID(start + 4 + 3 * GDInteger::DB_SIZE());
  scopeNode *scope = graph->getScope();
  symbol_.put_info(scope, symbolID);
  class_.put_info(scope, classID);
  xref_.put_info(scope, xrefID);
  context_.put_info(scope, contextID);

  symbolPtr fileSymbol;
  GDInteger fileID(start + 4 + 4 * GDInteger::DB_SIZE());
  fileSymbol.put_info(scope, fileID);
  astXref *axr = ast_get_astXref(fileSymbol);
  symbol_.put_info((scopeNode *) axr, symbolID);

  start += callDataCell::DB_SIZE();

  if (type_ & dataCell_Parent) {
    field_ = dataCell::newCell(start, graph);
    unsigned depth = field_->get_depth();
    set_depth(++depth);
  }
}

dataCell::dataCell(const dataCell &that)
  : symbol_(that.symbol_), level_(that.level_), argno_(that.argno_),
    type_(that.type_), graph_(NULL), field_(NULL), maxLevel_(that.maxLevel_),
    class_(that.class_), depth_(that.depth_)
{
  if (that.field_)
    field_ = that.field_->copy();
}

callDataCell::callDataCell(const dataCell &that) : nodeDataCell(that) {
  xref_ = that.get_xrefSymbol();
  context_ = that.getContext();
}

dataCell *
dataCell::newCell(unsigned char const *&start,
		  GDGraph *graph)
{
  Initialize(dataCell::newCell(unsigned char const *&, GDGraph *));
  dataCell *cell = NULL;

  if (graph) {
    dataCell_type type = *start;
    GDSet *hashtable = graph->getHashtable(true);

    if (type & dataCell_AstNode) {
      if (hashtable->getProjGraph() == graph)
	cell = new callDataCell(start, graph);
      else
	cell = new nodeDataCell(start, graph);
    } else
      cell = new dataCell(start, graph);
  }

  return cell;
}

bool
dataCell::isNodeDataCell() const
{
  return false;
}

bool
dataCell::isCallDataCell() const
{
  return false;
}

bool
nodeDataCell::isNodeDataCell() const
{
  return true;
}

bool
callDataCell::isCallDataCell() const
{
  return true;
}

dataCell *
dataCell::copy() const
{
  dataCell *cell = new dataCell(*this);
  return cell;
}

dataCell *
nodeDataCell::copy() const
{
  dataCell *cell = new nodeDataCell(*this);
  return cell;
}

dataCell *
callDataCell::copy() const
{
  dataCell *cell = new callDataCell(*this);
  return cell;
}

dataCell::~dataCell()
{
  if (field_) {
    delete field_;
    field_ = NULL;
  }
}

bool
dataCell::isChecked()
{
//  Initialize(dataCell::isChecked());

  if (type_ & dataCell_Checked)
    return true;

  if ((int) maxLevel_ >= 0 && (int) level_ > (int) maxLevel_) {
    if (!(type_ & dataCell_JavaObject) || ((int) maxLevel_ > 1))
      cleanSelf();
    else if ((int) maxLevel_ == 1 && !field_)
      cleanSelf();
  } else if ((int) level_ > DFA_MAX_LEVEL ||
	   (int) level_ < 0 - DFA_MAX_LEVEL)
    cleanSelf();
  else if (maxLevel_ == DML_CONSTANT &&
	   (field_ || (int) level_ != 0))
    cleanSelf();

  bool check = type_ & dataCell_Checked;
  return check;
}

unsigned int
dataCell::getSize() const
{
  Initialize(dataCell::getSize() const);
  unsigned int size = dataCell::DB_SIZE();

  if (field_)
    size += field_->getSize();

  return size;
}

unsigned int
nodeDataCell::getSize() const
{
  Initialize(nodeDataCell::getSize() const);
  unsigned int size = nodeDataCell::DB_SIZE();

  if (field_)
    size += field_->getSize();

  return size;
}

unsigned int
callDataCell::getSize() const
{
  Initialize(callDataCell::getSize() const);
  unsigned int size = callDataCell::DB_SIZE();

  if (field_)
    size += field_->getSize();

  return size;
}

dataCell *
dataCell::newCell(symbolPtr &symbol, bool mode)
{
  Initialize(dataCell::newCell(symbolPtr &, bool));

  dataCell *cell = NULL;
  symbolPtr node, decl;

  if (symbol.is_xrefSymbol())
    cell = new dataCell();
  else {
    node = symbol.get_astNode();

    if (mode) {
      decl = dfa_decl_of_ast(node);
      int code = symbolPtr_get_code(node);

      if (decl.is_ast() || code == MC_CALL_EXPR)
	cell = new nodeDataCell();
      else
	cell = new dataCell();
    } else
      cell = new nodeDataCell();
  }

  if (mode)
    cell->add_type(dataCell_Abstract);

  if (symbol.is_xrefSymbol()) {
    if (symbol.get_kind() == DD_ENUM_VAL)
      cell->symbol_.put_info(NULL, 0);
    else
      cell->symbol_ = symbol;
  } else if (node.isnotnull()) {
    int code = symbolPtr_get_code(node);

    if (!mode || code == MC_CALL_EXPR)
      cell->symbol_ = node;
    else
      cell->symbol_ = dfa_decl_of_ast(node);

    const symbolPtr &xref = symbol.get_xrefSymbol();
    cell->set_xrefSymbol(xref);
  }

  return cell;
}

bool
dataCell::pass() const
{
  bool pass = false;

  if ((get_type() & dataCell_OldFlag) ||
      (get_type() & dataCell_NewFlag))
    pass = true;

  return pass;
}

bool
dataCell::isConstArg()
{
  Initialize(dataCell::isConstArg());

  if (!GDSet::isDFApass() || isChecked() ||
      isNodeDataCell() || !(type_ & dataCell_Argument))
    return false;

  if (type_ & dataCell_Constant)
    return true;

  if (get_argno() == 0 && get_level() == 0 && !field_)
    return true;

  if (trg_get_srcs(this))
    return false;

  if (field_ || (int) maxLevel_ < 0 || (int) level_ == (int) maxLevel_)
    return false;

  unsigned int argno = get_argno();
  GDGraph *graph = get_graph();

  if (!graph || call_get_graph(graph))
    return false;

  if (graph->getBase()) {
    if (graph->getBaseLevel(argno) < (int) level_)
      return false;
  }

  Obj *trgSet = src_get_trgs(this);
  Obj *trg;

  ForEach(trg, *trgSet) {
    dataCell *target = (dataCell *) trg;
    Obj *srcSet = trg_get_srcs(target);

    if (srcSet && srcSet->size() > 1)
      return false;
  }

  add_type(dataCell_Constant);
  return true;
}

const symbolPtr &
dataCell::get_xrefSymbol() const
{
  return symbol_;
}

const symbolPtr &
nodeDataCell::get_xrefSymbol() const
{
  return xref_;
}

void
dataCell::set_xrefSymbol(const symbolPtr &symbol)
{
  symbol_ = symbol;
}

void
nodeDataCell::set_xrefSymbol(const symbolPtr &symbol)
{
  xref_ = symbol;
}

symbolPtr
dataCell::getContext() const
{
  symbolPtr symbol;

  if (graph_) {
    GDGraph *graph = graph_->getParent();
    graph->getSymbol(symbol);
  }

  return symbol;
}

symbolPtr
callDataCell::getContext() const
{
  return context_;
}

void
dataCell::setContext(const symbolPtr &symbol) {}

void
callDataCell::setContext(const symbolPtr &symbol)
{
  context_ = symbol;
}

char const *
dataCell::get_name() const
{
  Initialize(dataCell::get_name());

  return
    get_name(NULL);
}

char const *
dataCell::get_name(char const *name) const
{
  Initialize(dataCell::get_name(char const *));
  static genString result;

  result = "";
  int level = get_level();
  dataCell_type type = get_type();

  if (level > 0) {
    for (int k = 0; k < level; ++k)
      result += "*";
  } else if (level < 0) {
    for (int k = 0; k > level; --k)
      result += "&";
  }

  if (level)
    result += "(";

  if (name) {
    result += "(";
    result += name;
    result += ").(";
  }

  const symbolPtr &symbol = get_xrefSymbol();
  result += symbol.get_name();

  if (name)
    result += ")";

  if (type & dataCell_Argument) {
    genString str;
    str.printf("::%d", get_argno());
    result += str;
  } else if (type & dataCell_Result)
    result += "::return";

  if (level)
    result += ")";

  if (name)
    free((char *) name);

  if (!field_)
    return result;

  char const *save = OSapi_strdup((char const *) result.str());

  return
    field_->get_name(save);
}

void 
dataCell::print(ostream &os, 
		int lev) const
{
  Initialize(dataCell::print(ostream &, int));
  os << get_name() << endl;
}

ostream &
operator<<(ostream &os,
	   dataCell *cell)
{
  Initialize(operator<<(ostream &, dataCell *));

  if (cell)
    cell->dump(os);

  return os;
}

void
dataCell::dump(ostream &os)
{
  Initialize(dataCell::dump(ostream &));

  if (field_)
    add_type(dataCell_Parent);

  unsigned char type = (unsigned char) (get_type() & dataCell_Mask2);
  GDInteger symbolID = symbol_.get_offset();
  GDInteger classID = class_.get_offset();

  os << type << level_ << maxLevel_ << argno_ << symbolID << classID;

  if (field_)
    operator<<(os,field_);
}

void
nodeDataCell::dump(ostream &os)
{
  Initialize(nodeDataCell::dump(ostream &));

  if (field_)
    add_type(dataCell_Parent);

  unsigned char type = (unsigned char) (get_type() & dataCell_Mask2);
  GDInteger symbolID = symbol_.get_offset();
  GDInteger xrefID = xref_.get_offset();
  GDInteger classID = class_.get_offset();

  os << type << level_ << maxLevel_ << argno_ << symbolID << classID << xrefID;

  if (field_)
    os << field_;
}

void
callDataCell::dump(ostream &os)
{
  Initialize(callDataCell::dump(ostream &));

  if (field_)
    add_type(dataCell_Parent);

  unsigned char type = (unsigned char) (get_type() & dataCell_Mask2);
  GDInteger symbolID = symbol_.get_offset();
  GDInteger xrefID = xref_.get_offset();
  GDInteger classID = class_.get_offset();
  GDInteger contextID = context_.get_offset();

  os << type << level_ << maxLevel_ << argno_ << symbolID << classID << xrefID;

  astXref *axr = symbol_.get_astXref();
  symbolPtr fileSymbol = axr->get_module();
  fileSymbol = fileSymbol.get_xrefSymbol();
  GDInteger fileID = fileSymbol.get_offset();

  os << contextID << fileID;

  if (field_)
    os << field_;
}

bool
dataCell::cellCompare(dataCell *cell,
		      bool mode,
		      bool rigid,
		      bool ignore)
{
//  Initialize(dataCell::cellCompare(dataCell *, bool, bool, bool));

  if (argno_ != cell->argno_)
    return false;

  dataCell_type type = type_ & dataCell_Mask1;
  dataCell_type cellType = cell->type_ & dataCell_Mask1;

  if (!rigid) {
    type &= ~dataCell_Abstract;
    cellType &= ~dataCell_Abstract;
  }

  if (type != cellType)
    return false;

  if (!ignore) {
    if (level_ != cell->level_)
      return false;

    if (depth_ != cell->depth_)
      return false;

    if ((field_ && !cell->field_) || (!field_ && cell->field_))
      return false;
  } else if (field_ && cell->field_ && level_ != cell->level_)
    return false;

  if (rigid) {
    if (!mode) {
      bool check1 = isNodeDataCell() &&
	            !(get_type() & dataCell_Regular);
      bool check2 = cell->isNodeDataCell() &&
                    !(cell->get_type() & dataCell_Regular);

      if ((!check1 && !check2) || (check1 && check2)) {
	if (symbol_ != cell->symbol_)
	  return false;
      } else if (check1) {
	if (get_xrefSymbol() != cell->symbol_)
	  return false;
      } else if (check2) {
	if (symbol_ != cell->get_xrefSymbol())
	  return false;
      }
    } else {
      if (symbol_ != cell->symbol_)
	return false;
    }
  } else {
    symbolPtr node, cellNode;

    if (isNodeDataCell())
      node = dfa_decl_of_ast(symbol_);
    else
      node = symbol_;

    if (cell->isNodeDataCell())
      cellNode = dfa_decl_of_ast(cell->symbol_);
    else
      cellNode = cell->symbol_;

    if (node != cellNode)
      return false;
  }

  if (field_ && cell->field_ &&
      !field_->cellCompare(cell->field_, mode, rigid, ignore))
    return false;

  if (ignore && !field_ && !cell->field_ &&
      (level_ == cell->level_))
    return false;

  return true;
}

bool
dataCell::cellCompare2(dataCell *cell,
		       int &adjustment,
		       bool rigid,
		       bool ignore)
{
//  Initialize(dataCell::cellCompare2(dataCell *, int &, bool, bool));

  if (argno_ != cell->argno_)
    return false;

  dataCell_type type = type_ & dataCell_Mask1;
  dataCell_type cellType = cell->type_ & dataCell_Mask1;
  type &= ~dataCell_Abstract;
  cellType &= ~dataCell_Abstract;

  if (type != cellType)
    return false;

  if (rigid && !ignore) {
    bool check1 = isNodeDataCell() &&
                  !(get_type() & dataCell_Regular);
    bool check2 = cell->isNodeDataCell() &&
                  !(cell->get_type() & dataCell_Regular);

    if ((!check1 && !check2) || (check1 && check2)) {
      if (symbol_ != cell->symbol_)
	return false;
    } else if (check1) {
      if (get_xrefSymbol() != cell->symbol_)
	return false;
    } else if (check2) {
      if (symbol_ != cell->get_xrefSymbol())
	return false;
    }
  } else if (!ignore) {
    const symbolPtr &xref = get_xrefSymbol();
    const symbolPtr &cellXref = cell->get_xrefSymbol();

    if (xref != cellXref)
      return false;
  }

  adjustment = (int) level_ - (int) cell->level_;
  bool result = true;

  if (field_ && cell->field_)
    result = field_->cellCompare2(cell->field_, adjustment, rigid, false);

  return result;
}

void
dataCell::cellCompare3(dataCell *cell,
		      int &adjustment)
{
//  Initialize(dataCell::cellCompare3(dataCell *, int &));
  adjustment = (int) level_ - (int) cell->level_;

  if (field_ && cell->field_)
    field_->cellCompare3(cell->field_, adjustment);
}

bool
dataCell::shiftLevel(int adjustment)
{
  Initialize(dataCell::shiftLevel(int));

  dataCell *cell = (dataCell *) this;
  dataCell *temp = cell;

  while (temp = cell->getField())
    cell = temp;

  bool check = false;
  int maxLevel = (int) cell->maxLevel_;
  int level = cell->get_level();
  level += adjustment;
  cell->set_level(level);

  if (maxLevel >= 0 && level > maxLevel) {
    if (!(cell->get_type() & dataCell_JavaObject) || (maxLevel > 1))
      check = true;
    else if (maxLevel == 1 && !cell->getField())
      check = true;
  } else if (level > DFA_MAX_LEVEL || level < -1)
    check = true;
  else if ((char) maxLevel == DML_CONSTANT &&
	   (cell->getField() || level != 0))
    check = true;

  if (check) {
    level = level - adjustment;
    cell->set_level(level);
  }

  return check;
}

bool
dataCell::transform(unsigned int adjustment,
		    dataCell *field,
		    objSet &dataSet,
		    GDGraph *graph)
{
  Initialize(dataCell::transform(unsigned int, dataCell *, objSet &, GDGraph *));
  dataCell *cell = (dataCell *) this;

  if (field) {
    unsigned int depth = get_depth();
    unsigned int fieldDepth = field->get_depth();

    if (depth + fieldDepth + 1 > DFA_NESTED_FIELDS_LIMIT)
      return false;
  }

  if (adjustment != 0) {
    if (shiftLevel(adjustment))
      return false;
  }

  if (!field) {
    dataCell *temp = cell;

    while (temp->getField())
      temp = temp->getField();

    int level = temp->get_level();
    int maxLevel = temp->get_maxLevel();

    if (maxLevel > 0 && level == maxLevel) {
      ddKind tempKind = temp->class_.get_kind();

      if (tempKind == DD_CLASS || tempKind == DD_INTERFACE) {
	if (adjustment != 0)
	  shiftLevel(0 - adjustment);

	return false;
      }
    }

    if (level > 0 && graph) {
      GDSet *hashtable = graph->getHashtable(true);

      if (hashtable->getProjGraph() != graph ||
	  (cell->get_type() & dataCell_Regular)) {
	temp->set_level(0);
	dataCell *newCell = NULL;

	if (is_model_build() && graph_ != graph && !call_get_graph(graph_) &&
	    call_get_graph(graph) && !cell->isNodeDataCell() &&
	    !(cell->get_type() & dataCell_Regular)) {
	  dataCell *dummy = new nodeDataCell(*temp);
	  const symbolPtr &xref = cell->get_xrefSymbol();
	  dummy->set_xrefSymbol(xref);
	  symbolPtr graphNode;
	  graph->getAstSymbol(graphNode);
	  dummy->set_symbol(graphNode);
	  newCell = graph->findDataCell(dummy, false, false);
	  delete dummy;
	} else
	  newCell = graph->findDataCell(temp, false, false);

	temp->set_level(level);

	if (newCell) {
	  Obj *srcSet = trg_get_srcs(newCell);
	  Obj *src;

	  ForEach(src, *srcSet) {
	    dataCell *source = (dataCell *) src;

	    if (cell_get_alias(source))
	      continue;

	    dataCell *srcField = source;

	    while (srcField->getField())
	      srcField = srcField->getField();

	    if (srcField->get_level() >= 0)
	      continue;

	    source->transform(level, NULL, dataSet, graph);
	  }
	}

	if (dataSet.size() == 0) {
	  if (adjustment != 0)
	    cell = this->copy();

	  dataSet.insert(cell);
	}
      } else if (!cell->isCallDataCell()) {
	if (adjustment != 0)
	  cell = this->copy();

	dataSet.insert(cell);
      }
    } else {
      if (adjustment != 0)
	cell = this->copy();

      dataSet.insert(cell);
    }
  } else {
    if (dataCell::setField(cell, field, true)) {
      if (cell != this)
	delete cell;

      if (adjustment != 0)
	shiftLevel(0 - adjustment);

      ++dataCell::deleteCount_;
      return false;
    } else
      dataSet.insert(cell);
  }

  if (adjustment != 0)
    shiftLevel(0 - adjustment);

  if (dataSet.size() == 0)
    return false;

  return true;
}

void
dataCell::get_typeSymbol(symbolPtr &symbol)
{
  Initialize(dataCell::get_typeSymbol(symbolPtr &));

  if (class_.isnotnull()) {
    symbol = class_;
    return;
  }

  if (GDSet::isDFApass() || (get_type() & dataCell_Argument))
    return;

  symbol = get_xrefSymbol();
  symbolArr typeArr;
  unsigned int size = 0;
  symbol.get_local_link(has_type, typeArr);

  for (int k = 0; k < DFA_MAX_TYPEDEF_SEARCH &&
       typeArr.size() > size; ++k) {
    size = typeArr.size();
    symbol = typeArr[size - 1];
    symbol.get_local_link(has_type, typeArr);
  }

  class_ = symbol;
}

bool
dataCell::setField(dataCell *&cell,
		   dataCell *field,
		   bool mode)
{
  Initialize(dataCell::setField(dataCell *, dataCell *, bool));

  if (!cell || !cell->isValid() || cell->isChecked())
    return true;

  if (field) {
    if (field->isNodeDataCell())
      return true;

    dataCell *realCell = cell;
    dataCell *realField = field;

    if (GDSet::isDFApass()) {
      unsigned int cellDepth = realCell->get_depth();
      unsigned int fieldDepth = realField->get_depth();

      if (cellDepth + fieldDepth + 1 > DFA_NESTED_FIELDS_LIMIT)
	return true;
    }

    if (!realField->isValid() || realField->isChecked())
      return true;

    symbolPtr firstType;
    realCell->get_typeSymbol(firstType);

    if (GDSet::isDFApass() && firstType.isnotnull()) {
      dataCell *temp = realField;

      while (temp) {
	if (temp->get_type() & dataCell_Argument)
	  continue;

	symbolPtr secondType;
	temp->get_typeSymbol(secondType);
	temp = temp->getField();

	if (firstType == secondType)
	  return true;
      }
    }

    if (!realCell->field_) {
      if (GDSet::isDFApass()) {
	const symbolPtr &fieldSym = realField->get_symbol();

	if (!GDSet::classInfo_) {
	  cout.flush();
	  timer::init(1, "Processing class information", "");
	  GDSet::classInfo_ = classInfo_create();
	  timer::showtime(1, "Processing class information", "");
	}

	if (!classInfo_has_field(GDSet::classInfo_, firstType, fieldSym))
	  return true;
      }

      if (realCell->maxLevel_ == DML_UNKNOWN)
	realCell->maxLevel_ = realCell->level_;
      else if ((int) realCell->maxLevel_ >= 0 && 
	       realCell->level_ != realCell->maxLevel_)
	return true;
      else if (realCell->maxLevel_ == DML_CONSTANT &&
	       (int) realCell->level_ != 0)
	return true;

      if (mode)
	realCell = cell->copy();

      realCell->field_ = realField->copy();
      realCell->add_type(dataCell_Parent);
      realCell->field_->add_type(dataCell_Field);
      GDGraph *graph = cell->get_graph();
      realCell->field_->set_graph(graph);
    } else {
      dataCell *fieldCell = realCell->field_;
      bool check = dataCell::setField(fieldCell, realField, false);

      if (fieldCell->get_type() & dataCell_Checked)
	realCell->cleanSelf();

      if (check)
	return true;

      if (mode) {
	realCell = cell->copy();
	dataCell::setField(cell, NULL);
      }
    }

    unsigned int depth = realCell->field_->get_depth();
    realCell->set_depth(++depth);
    cell = realCell;
  } else if (cell->field_) {
    if (!cell->field_->field_) {
      cell->rem_type(dataCell_Parent);
      cell->set_depth(0);
      delete cell->field_;
      cell->field_ = NULL;
    } else {
      dataCell::setField(cell->field_, NULL);
      unsigned int depth = cell->field_->get_depth();
      cell->set_depth(++depth);
    }
  }

  bool check = cell->get_type() & dataCell_Checked;
  return check;
}

bool
dataCell::symbolCompare(const symbolPtr &source,
			const symbolPtr &target)
{
  Initialize(dataCell::symbolCompare(const symbolPtr &, const symbolPtr &));
  int check = source.sym_compare(target);

  if (check == 2 || check == -2)
    return false;
  else
    return true;
}

bool
dataCell::processLevel(bool &ref)
{
  Initialize(dataCell::processLevel(bool &));

  maxLevel_ = 0;
  ref = false;
  bool special = true;
  ddKind symKind = symbol_.get_kind();

  if (symKind == DD_NUMBER || symKind == DD_ENUM_VAL) {
    maxLevel_ = DML_CONSTANT;
    return false;
  } else if (symKind == DD_STRING) {
    maxLevel_ = (char) 1;
    return false;
  }

  if (!(type_ & dataCell_Argument)) {
    symbolPtr symbol = symbol_.get_xrefSymbol();
    symbolArr typeArr;
    symbol.get_local_link(has_type, typeArr);
    unsigned int size = 0;

    for (int k = 0; k < DFA_MAX_TYPEDEF_SEARCH &&
	 typeArr.size() > size; ++k) {
      special = false;
      size = typeArr.size();
      symbol = typeArr[size - 1];

      if (symbol->get_attribute(POINTER, 1))
	++maxLevel_;
      else if (symbol->get_attribute(REF, 1)) {
	++maxLevel_;
	ref = true;
      }

      symbol.get_local_link(has_type, typeArr);
    }

    if (!strcmp(symbol.get_name(), "void*"))
      ++maxLevel_;

    if (size > 0)
      class_ = symbol;
    else
      maxLevel_ = DML_UNKNOWN;
  } else {
    char *argString;
    genArrOf(int) indexArr;
    symbolPtr symbol = symbol_.get_xrefSymbol();
    Xref *xref = symbol->get_Xref();
    int count = parse_args(symbol, argString, indexArr);
    int argno = get_argno();

    if (!argString) {
      maxLevel_ = DML_UNKNOWN;
      return special;
    }

    if (argno == count && strstr(argString, "..."))
      add_type(dataCell_Shared);
    else if (argno > count) {
      argno = count;
      set_argno(argno);
      add_type(dataCell_Shared);
    }

    if (argno == 0) {
      symbolArr types;
      char const *scope = ATT_scope(symbol);
      if (scope) {
	char *tmp_scope = new char[strlen(scope)+1];
	strcpy (tmp_scope, scope);
	xref->lookup_local(types, DD_CLASS, tmp_scope, NULL);

	if (types.size() > 0)
	  class_ = types[0];
	else if (DFA_JAVA) {
	  xref->lookup_local(types, DD_INTERFACE, tmp_scope, NULL);

	  if (types.size() > 0)
	    class_ = types[0];
	}

	if (!DFA_JAVA)
	  maxLevel_ = 1;

	delete tmp_scope;
      }

      return special;
    }

    int position = 0;

    if (count == argno) {
      char const *p = strchr(argString, ')');

      if (p)
	position = p - argString;
      else
	position = strlen(argString);
    } else
      position = *indexArr[argno];

    if (position >= 0)
      argString[position--] = '\0';

    while (((argString[position] == ' ') || (argString[position] == ',')) &&
	   position >= 0)
      argString[position--] = '\0';

    while (((argString[position] == '*') || (argString[position] == '&') ||
	    (argString[position] == ']')) &&
	   position >= 0) {
      if (argString[position] == '&')
	ref = true;

      if (!DFA_JAVA)
	++maxLevel_;

      argString[position--] = '\0';
    }

    while (argString[position] == ' ' && position >= 0 &&
	   position >= *indexArr[argno - 1])
      argString[position--] = '\0';

    char const *name = argString + *indexArr[argno - 1];
    char const *space = strrchr(name, ' ');

    if (space)
      name = space;

    while (*name == ' ')
      ++name;

    if (!strcmp(name, "..."))
      maxLevel_ = DML_UNKNOWN;

    symbolArr types;
    xref->lookup_local(types, DD_CLASS, name, NULL);

    if (types.size() == 0) {
      if (!DFA_JAVA)
	xref->lookup_local(types, DD_TYPEDEF, name, NULL);
      else
	xref->lookup_local(types, DD_INTERFACE, name, NULL);

      if (types.size() == 0)
	return special;
    }

    class_ = types[0];
    unsigned int size = types.size();
    class_.get_local_link(has_type, types);

    for (int k = 0; k < DFA_MAX_TYPEDEF_SEARCH &&
	 types.size() > size; ++k) {
      size = types.size();
      class_ = types[size - 1];
      class_.get_local_link(has_type, types);
    }
  }

  if (DFA_JAVA) {
    ddKind typeKind = class_.get_kind();

    if (typeKind == DD_CLASS || typeKind == DD_INTERFACE)
      add_type(dataCell_JavaObject);
  }

  return special;
}

void
GDGraph::processFlow(Obj *sourceSet,
		     Obj *targetSet,
		     bool abstract)
{
  Initialize(GDGraph::processFlow(Obj *, Obj *, bool));
  Obj *src, *trg;

  ForEach(src, *sourceSet) {
    dataCell *source = (dataCell *) src;

    if (!source)
      continue;

    GDGraph *srcCall = source->get_graph();
    dataCell *srcField = source;
    dataCell *temp = source;

    while (temp = temp->getField())
      srcField = temp;

    bool srcRef = false;
    bool special = srcField->processLevel(srcRef);
    int srcMaxLevel = srcField->get_maxLevel();
    int srcLevel = srcField->get_level();
    dataCell_type srcType = srcField->get_type();

    ForEach(trg, *targetSet) {
      if (!source)
	break;

      dataCell *target = (dataCell *) trg;

      if (!target)
	continue;

      GDGraph *trgCall = target->get_graph();
      dataCell *trgField = target;
      dataCell *tmp = target;

      while (tmp = tmp->getField())
	trgField = tmp;

      bool trgRef = false;
      special |= trgField->processLevel(trgRef);
      int trgMaxLevel = trgField->get_maxLevel();
      int trgLevel = trgField->get_level();
      dataCell_type trgType = trgField->get_type();

      int diff = srcMaxLevel - srcLevel - trgMaxLevel + trgLevel;
      symbolPtr srcSymbol = srcField->get_symbol();

      if (srcMaxLevel != (int) DML_UNKNOWN &&
	  trgMaxLevel != (int) DML_UNKNOWN &&
	  diff != 0) {
	if (diff == 1 && srcRef && !trgRef)
	  trgField->set_level(--trgLevel);
	else if (diff == -1 && ((!srcRef && trgRef) ||
				((trgType & dataCell_Argument) &&
				 target->get_argno() == 0)))
	  srcField->set_level(--srcLevel);
	else if ((srcType & dataCell_Result) && srcCall &&
		 call_get_graph(srcCall)) {
	  symbolPtr funcSymbol = srcSymbol.get_xrefSymbol();

	  if (diff == 1 && funcSymbol->get_attribute(CONS_DES, 1))
	    trgField->set_level(--trgLevel);
	  else if (special) {
	    srcMaxLevel = srcLevel + trgMaxLevel - trgLevel;
	    srcField->set_maxLevel(srcMaxLevel);
	  } else {
//	    continue;
	  }
	} else {
//	  continue;
	}
      }

      dataCell_type sourceType = source->get_type();
      dataCell_type targetType = target->get_type();

      if (!(targetType & dataCell_Regular) &&
	  target->isNodeDataCell()) {
	symbolPtr typeSymbol;
	srcField->get_typeSymbol(typeSymbol);
	trgField->set_typeSymbol(typeSymbol);

	if (trgMaxLevel == (int) DML_UNKNOWN &&
	    srcMaxLevel != (int) DML_UNKNOWN) {
	  int maxLevel = srcMaxLevel - srcLevel + trgLevel;
	  trgField->set_maxLevel(maxLevel);
	}
      } else if (!(sourceType & dataCell_Regular) &&
		 source->isNodeDataCell()) {
	symbolPtr typeSymbol;
	trgField->get_typeSymbol(typeSymbol);
	srcField->set_typeSymbol(typeSymbol);

	if (srcMaxLevel == (int) DML_UNKNOWN &&
	    trgMaxLevel != (int) DML_UNKNOWN) {
	  int maxLevel = trgMaxLevel - trgLevel + srcLevel;
	  srcField->set_maxLevel(maxLevel);
	}
      }

      if ((sourceType & dataCell_Result) && srcCall != this && srcCall)
	srcCall->addRelation(source, target, GDR_IRREDUCIBLE, abstract,
			     false, false);

      if (!source || !target)
	continue;

      if ((targetType & dataCell_Result) && trgCall != this && trgCall)
	trgCall->addRelation(source, target, GDR_IRREDUCIBLE, abstract,
			     false, false);

      if (!source || !target)
	continue;

      if ((!(sourceType & dataCell_Result) || srcCall == this || !srcCall) &&
	  (!(targetType & dataCell_Result) || trgCall == this || !trgCall)) {
	addRelation(source, target, GDR_IRREDUCIBLE, abstract,
		    false, false);

	if (!source->isNodeDataCell() && (sourceType & dataCell_Argument) &&
	    source->get_argno() == 0 && source->get_level() == 0 &&
	    !source->getField())
	  addRelation(target, source, GDR_IRREDUCIBLE | GDR_ALIAS, abstract,
		      false, false);
      }
    }
  }
}

Obj *
GDSet::generateDataFlow(symbolPtr &expr, 
			objSet &resultSet,
			bool abstract)
{
//  Initialize(GDSet::generateDataFlow(symbolPtr &, objSet &, bool));

  if (!expr.is_ast())
    return &resultSet;

  if (!currFile_) {
    astXref *axr = expr.get_astXref();
    symbolPtr fileSymbol = axr->get_module();
    fileSymbol = fileSymbol.get_xrefSymbol();
    const symbolPtr &node = fileSymbol.get_astNode();

    if (node.isnull())
      return &resultSet;

    currFile_ = find(fileSymbol);

    if (!currFile_) {
      GDHeader *header = new GDHeader(fileSymbol.get_offset(),
				      node.get_offset(),
				      fileSymbol.get_offset());
      scopeNode *scope = fileSymbol.get_scope();
      currFile_ = new GDGraph(header, scope);
      add(currFile_, fileSymbol);
    }
  }

  if (!currGraph_)
    currGraph_ = currFile_;

  symbolPtr child;
  int code = symbolPtr_get_code(expr);

  switch (code) {

  case DFA_REF_DD:
  case DFA_REF_DFA:
    {
      dataCell *cell = dataCell::newCell(expr, abstract);
      cell->add_type(dataCell_Regular);
      resultSet.insert(cell);
      dataCells_.insert_last(cell);
    }

    break;

  case THIS:
    if (currGraph_) {
      symbolPtr graphSymbol;
      currGraph_->getAstSymbol(graphSymbol);
      dataCell *cell = dataCell::newCell(graphSymbol, abstract);
      cell->add_type(dataCell_Argument);
      resultSet.insert(cell);
      dataCells_.insert_last(cell);
    }

    break;

  case MC_MODIFY_EXPR:
    {
      tree_get_first(expr, child);
      objSet srcSet, trgSet;
      Obj *trg = generateDataFlow(child, trgSet, abstract);
      tree_get_next(child, child);
      Obj *src = generateDataFlow(child, srcSet, abstract);
      resultSet.insert(src);
      currGraph_->processFlow(src, trg, abstract);
    }

    break;

  case MC_PREDECREMENT_EXPR:
  case MC_PREINCREMENT_EXPR:
  case MC_POSTDECREMENT_EXPR:
  case MC_POSTINCREMENT_EXPR:
    {
      tree_get_first(expr, child);
      objSet srcSet, trgSet;
      Obj *trg = generateDataFlow(child, trgSet, abstract);
      tree_get_next(child, child);
      Obj *src = generateDataFlow(child, srcSet, abstract);
      resultSet.insert(trg);
      currGraph_->processFlow(src, trg, abstract);
    }

    break;

  case MC_ADDR_EXPR:
    {
      tree_get_first(expr, child);
      objSet local;
      Obj *sourceSet = generateDataFlow(child, local, abstract);
      Obj *src;

      ForEach(src, *sourceSet) {
	dataCell *cell = (dataCell *) src;
	GDGraph *graph = cell->get_graph();
	cell = cell->copy();
	cell->shiftLevel(-1);

	if (graph)
	  graph->addDataCell(cell, true);

	if (cell) {
	  resultSet.insert(cell);

	  if (!graph)
	    dataCells_.insert_last(cell);
	}
      }
    }

    break;

  case MC_INDIRECT_REF:
    {
      tree_get_first(expr, child);
      objSet local;
      Obj *sourceSet = generateDataFlow(child, local, abstract);
      Obj *src;

      ForEach(src, *sourceSet) {
	dataCell *cell = (dataCell *) src;
	GDGraph *graph = cell->get_graph();
	cell = cell->copy();
	cell->shiftLevel(1);

	if (graph)
	  graph->addDataCell(cell, true);

	if (cell) {
	  resultSet.insert(cell);

	  if (!graph)
	    dataCells_.insert_last(cell);
	}
      }
    }

    break;

  case MC_CALL_EXPR:
    {
      if (expr.get_xrefSymbol().isnull())
	break;

      symbolPtr fun;
      tree_get_first(expr, fun);
      const symbolPtr &symbol = fun.get_xrefSymbol();

      if (fun.isnotnull() && symbol.isnotnull() &&
	  symbol.get_kind() == DD_FUNC_DECL) {
	symbolPtr fileSymbol;
	currFile_->getFileSymbol(fileSymbol);
	scopeNode *scope = fileSymbol.get_scope();
	GDGraph *call = NULL;
	bool dtor = false;
	int argno = 1;

	if (symbol->get_attribute(METHOD, 1) &&
	    !symbol->get_attribute(STAT_ATT, 1))
	  argno = 0;

	if (symbol->get_attribute(CONS_DES, 1)) {
	  char const *name = symbol.get_name();

	  if (!strchr(name, '~'))
	    argno = 1;
	  else {
	    dtor = true;
	    argno = 0;
	  }
	}

	if (!abstract) {
	  currGraph_->loadGraph(true);
	  Obj *callSet = graph_get_calls(currGraph_);
	  Obj *callObj;

	  ForEach(callObj, *callSet) {
	    GDGraph *tmpCall = (GDGraph *) callObj;
	    GDHeader *tmpHeader = tmpCall->getHeader();

	    if (tmpHeader->getNodeID() == expr.get_offset() &&
		tmpHeader->getSymbolID() == symbol.get_offset()) {
	      call = tmpCall;

	      if (currGraph_->getFormat() == GDF_ACTIVE)
		call->setFormat(GDF_ACTIVE);

	      break;
	    }
	  }
	}

	if (!call) {
	  GDHeader *header = new GDHeader(symbol.get_offset(),
					  expr.get_offset(),
					  fileSymbol.get_offset());
	  call = new GDGraph(header, scope);
	  graph_put_call(currGraph_, call);
	  currGraph_->incCallNumber();
	}

	GDGraph *temp = currGraph_;
	currGraph_ = call;

	if (warn && delCell) {
	  dataCell *target = dataCell::newCell(expr, abstract);
	  target->add_type(dataCell_Argument);
	  dataCells_.insert(target);

	  currGraph_->processFlow(delCell, target, abstract);
	  delCell = NULL;
	  warn = false;
	} else {
	  for (tree_get_next(fun, child);
	       child.isnotnull();
	       tree_get_next(child, child), ++argno) {
	    dataCell *target = dataCell::newCell(expr, abstract);
	    target->add_type(dataCell_Argument);
	    target->set_argno(argno);
	    dataCells_.insert_last(target);

	    if (warn && !delCell)
	      delCell = target;

	    objSet local;
	    Obj *source = generateDataFlow(child, local, abstract);
	    currGraph_->processFlow(source, target, abstract);
	  }
	}

	dataCell *retCell = dataCell::newCell(expr, abstract);
	retCell->add_type(dataCell_Result);
	currGraph_->addDataCell(retCell, true);

	if (retCell)
	  resultSet.insert(retCell);

	currGraph_ = temp;
      }
    }

    break;

  case MC_FUNCTION_DECL:
    {
      astNode *node = astNode_of_sym(expr);

      if (((dfa_decl *) node)->get_is_def()) {
	symbolPtr fn_id;
	tree_get_first(expr, fn_id);
	symbolPtr symbol = fn_id.get_xrefSymbol();

	if (testName && !strcmp(symbol.get_name(), testName))
	  dfa_debug_hook(testName);

	if (symbol.isnotnull() && symbol.get_kind() == DD_FUNC_DECL) {
	  symbolPtr fileSymbol;
	  currFile_->getFileSymbol(fileSymbol);
	  currGraph_ = this->find(symbol, false);

	  if (currGraph_ && abstract &&
	      !GDSet::currentGraphs_.includes(currGraph_)) {
	    currGraph_->offloadGraph(true);
	    obj_delete(currGraph_);
	    currGraph_ = NULL;
	  }

	  if (!currGraph_) {
	    GDHeader *header = new GDHeader(symbol.get_offset(),
					    expr.get_offset(),
					    fileSymbol.get_offset());
	    scopeNode *scope = fileSymbol.get_scope();
	    currGraph_ = new GDGraph(header, scope);
	    this->add(currGraph_, symbol);
	  } else {
	    currGraph_->setArgno(0);

	    if (abstract)
	      currGraph_->setFormat(GDF_ACTIVE);
	  }

	  GDSet::currentGraphs_.insert_last(currGraph_);

	  for(tree_get_next(fn_id, child);
	      child.isnotnull();
	      tree_get_next(child, child))
	    generateDataFlow(child, resultSet, abstract);
	}
      }
    }

    break;

  case MC_RETURN_STMT:
    tree_get_first(expr, child);

    if (child.isnotnull()) {
      objSet local;
      symbolPtr fun;
      Obj *source = generateDataFlow(child, local, abstract);

      for (fun = expr; 
	   fun.isnotnull();
	   tree_get_parent(fun, fun)) {
	if (symbolPtr_get_code(fun) == DFA_FUNCTION_DEF) {
	  for (tree_get_first(fun, fun);
	       fun.isnotnull();
	       tree_get_next(fun, fun)) {
	    if (symbolPtr_get_code(fun) == MC_FUNCTION_DECL) {
	      dataCell *target = dataCell::newCell(fun, abstract);
	      target->add_type(dataCell_Result);
	      dataCells_.insert_last(target);

	      currGraph_->processFlow(source, target, abstract);
	      break;
	    }
	  }

	  break;
	}
      }
    }

    break;

  case MC_VAR_DECL:
    tree_get_first(expr, child);

    if (child.isnotnull()) {
      objSet srcSet, trgSet;
      Obj *trg = generateDataFlow(child, trgSet, abstract);
      tree_get_next(child, child);
      Obj *src = generateDataFlow(child, srcSet, abstract);
      resultSet.insert(src);

      if (currGraph_)
	currGraph_->processFlow(src, trg, abstract);
    }

    break;

  case MC_PARM_DECL:
    tree_get_first(expr, child);

    if (child.isnotnull()) {
      symbolPtr funcSymbol, funcNode;
      currGraph_->getSymbol(funcSymbol);

      if (funcSymbol.get_kind() != DD_FUNC_DECL)
	break;

      currGraph_->getAstSymbol(funcNode);
      bool ctor = false;

      if (funcSymbol->get_attribute(CONS_DES, 1)) {
	char const *name = funcSymbol.get_name();

	if (!strchr(name, '~'))
	  ctor = true;
      }

      GDHeader *header = currGraph_->getHeader();
      unsigned int argno = header->getArgno();

      for (; child.isnotnull(); tree_get_next(child, child)) {
	int aux = symbolPtr_get_code(child);

	if (aux == DFA_REF_DD || aux == DFA_REF_DFA) {
	  objSet srcSet, trgSet;
	  Obj *target = generateDataFlow(child, trgSet, abstract);

	  if (argno == 0 && ctor && DFA_JAVA)
	    ++argno;

	  if (argno == 0 && ctor) {
	    dataCell *retCell = dataCell::newCell(funcNode, abstract);
	    retCell->add_type(dataCell_Result);
	    dataCells_.insert_last(retCell);

	    currGraph_->processFlow(target, retCell, abstract);
	  } else {
	    dataCell *source = dataCell::newCell(funcNode, abstract);
	    source->add_type(dataCell_Argument);

	    if (funcSymbol->get_attribute(METHOD, 1) &&
		!funcSymbol->get_attribute(STAT_ATT, 1))
	      source->set_argno(argno);
	    else
	      source->set_argno(argno + 1);

	    dataCells_.insert_last(source);
	    currGraph_->processFlow(source, target, abstract);
	  }

	  ++argno;
	}
      }

      header->setArgno(argno);
    }

    break;

  case MC_ARRAY_REF:
    tree_get_first(expr, child);

    if (child.isnotnull()) {
      objSet srcSet, local;
      Obj *dataSet = generateDataFlow(child, srcSet, abstract);
      Obj *data;

      ForEach(data, *dataSet) {
	dataCell *cell = (dataCell *) data;

	if (!DFA_JAVA)
	  cell->shiftLevel(1);

	resultSet.insert(cell);
      }

      tree_get_next(child, child);
      generateDataFlow(child, local, abstract);
    }

    break;

  case MC_COMPONENT_REF:
  case JAVA_COMPONENT_REF:
  case MC_BIT_FIELD_REF:

    tree_get_first(expr, child);

    if (child.isnotnull()) {
      objSet srcSet, trgSet;
      Obj *dataSet = generateDataFlow(child, srcSet, abstract);
      tree_get_next(child, child);
      Obj *fieldSet = generateDataFlow(child, trgSet, abstract);
      Obj *data, *field;

      ForEach(data, *dataSet) {
	dataCell *cell = (dataCell *) data;

	if (DFA_JAVA)
	  cell->shiftLevel(1);

	ForEach(field, *fieldSet) {
	  dataCell::setField(cell, (dataCell *) field, false);
	  break;
	}

	if (cell && cell->isValid() && !cell->isChecked()) {
	  resultSet.insert(cell);
	  break;
	}
      }
    }

    break;

  case CTOR_INIT:
    if (currGraph_) {
      objSet srcSet, trgSet;
      tree_get_first(expr, child);
      symbolPtr funcSymbol, funcNode;
      currGraph_->getSymbol(funcSymbol);
      currGraph_->getAstSymbol(funcNode);
      bool ctor = false;

      if (funcSymbol->get_attribute(CONS_DES, 1)) {
	char const *name = funcSymbol.get_name();

	if (!strchr(name, '~'))
	  ctor = true;
      }

      if (ctor) {
	Obj *fieldSet = NULL;
	Obj *field;

	if (symbolPtr_get_code(child) == DFA_REF_DD)
	  fieldSet = generateDataFlow(child, trgSet, abstract);

	dataCell *target = dataCell::newCell(funcNode, abstract);
	target->add_type(dataCell_Result);
	dataCells_.insert_last(target);

	if (fieldSet) {
	  target->set_level(1);

	  ForEach(field, *fieldSet) {
	    dataCell::setField(target, (dataCell *) field, false);
	    break;
	  }

	  tree_get_next(child, child);
	}

	if (target && target->isValid() && !target->isChecked()) {
	  Obj *source = generateDataFlow(child, srcSet, abstract);
	  currGraph_->processFlow(source, target, abstract);
	}
      } else {
	for (; child.isnotnull(); tree_get_next(child, child))
	  generateDataFlow(child, resultSet, abstract);
      }
    }

    break;

  case MC_COND_EXPR:
    {
      tree_get_first(expr, child);
      objSet local;
      generateDataFlow(child, local, abstract);
      tree_get_next(child, child);
      generateDataFlow(child, resultSet, abstract);
      tree_get_next(child, child);
      generateDataFlow(child, resultSet, abstract);
    }

    break;

  case DFA_FUNCTION_DEF:
    {
      GDGraph *temp = currGraph_;

      for (tree_get_first(expr, child); 
	   child.isnotnull();
	   tree_get_next(child, child))
	generateDataFlow(child, resultSet, abstract);

      currGraph_ = temp;
    }

    break;

  case DELETE_EXPR:
    warn = true;
    delCell = NULL;

    for (tree_get_first(expr, child); 
	 child.isnotnull();
	 tree_get_next(child, child))
      generateDataFlow(child, resultSet, abstract);

    delCell = NULL;
    warn = false;
    break;

  default:
    for (tree_get_first(expr, child); 
	 child.isnotnull();
	 tree_get_next(child, child))
      generateDataFlow(child, resultSet, abstract);

    break;
  }

  return &resultSet;
}

void
GDSet::generateFlow(astRoot *rt,
		    bool abstract,
		    bool mode)
{
  Initialize(GDSet::generateFlow(astRoot *, bool, bool));

  if (!is_model_build())
    return;

  objSet cells;
  astXref *axr = rt->get_xref();
  astNode *ast = rt->get_astNode();
  symbolPtr symbol(ast, axr);
  symbolPtr fileSymbol = symbol.get_xrefSymbol();

  if (fileSymbol.isnotnull() && !fileSet_.includes(fileSymbol))
    fileSet_.insert_last(fileSymbol);
  else if (fileSymbol.isnotnull())
    cleanTable(fileSymbol);

  const symbolPtr &node = fileSymbol.get_astNode();

  if (node.isnull())
    return;

  currFile_ = this->find(fileSymbol, false);

  if (!currFile_) {
    GDHeader *header = new GDHeader(fileSymbol.get_offset(),
				    node.get_offset(),
				    fileSymbol.get_offset());
    scopeNode *scope = fileSymbol.get_scope();
    currFile_ = new GDGraph(header, scope);
    add(currFile_, fileSymbol);
  }

  DFA_JAVA = false;
  const elsLanguage *language = elsLanguage::get_els_language(fileSymbol);

  if (language && !strcmp(language->get_name(), "java"))
    DFA_JAVA = true;

  currGraph_ = currFile_;
  GDSet::currentGraphs_.removeAll();
  Obj *res = generateDataFlow(symbol, cells, abstract);
  GDSet::currentGraphs_.removeAll();
  cleanDataCells();
  int size = array_.size();

  for (unsigned int k = 0; k < size; ++k) {
    GDGraph *graph = *array_[k];

    if (graph && graph->getFormat() == GDF_ACTIVE)
      graph->offloadGraph(false);
  }

  if (mode) {
    if (!is_model_build())
      update(GDT_UPDATED);

    if (!status_)
      setStatus(GDT_LOCAL);
  }

  currFile_ = NULL;
  currGraph_ = NULL;
}

dataCell *
astXref::get_dataCell_from_id(int id)
{
  Initialize(astXref::get_dataCell_from_id(int));

  dataCell *retval = NULL;
  astRoot *ar = get_ast_table();

  if (ar) 
    retval = (dataCell *) ar->dataCells[id - 1];

  return retval;
}

dataCell *
dataCell_of_symbol(const symbolPtr &symbol,
		   bool load)
{
  Initialize(dataCell_of_symbol(const symbolPtr &, bool));

  if (!symbol.is_dataCell())
    return NULL;

  GDGraph *graph = symbol.get_graph();

  if (!graph)
    return NULL;

  dataCell *cell = NULL;
  GDMode mode = graph->getMode();
  unsigned int ID = symbol.get_offset() & 0x00FFFFFF;
  GDSet *hashtable = graph->getHashtable(true);

  if (load || (mode != GDM_NONE && mode != GDM_OFFLOADED))
    cell = (*graph)[ID - 1];
  else {
    GDHeader *header = graph->getHeader();
    unsigned int cellNumber = header->getCellNumber();

    if (ID > cellNumber)
      cell = (*graph)[ID - 1];
    else if (graph->getHashArr()) {
      unsigned char const *start = graph->getStart(ID - 1) + GDHeader::DB_SIZE();
      cell = dataCell::newCell(start, graph);
      cell->set_graph(graph);
    }
  }

  return cell;
}

void
dataCell_offload(dataCell *&cell,
		 bool mode)
{
  Initialize(dataCell_offload(dataCell *&, bool));

  if (!cell)
    return;

  GDGraph *graph = cell->get_graph();

  if (!graph)
    delete cell;
  else if (!mode && (graph->getMode() == GDM_NONE ||
		     graph->getMode() == GDM_OFFLOADED))
    delete cell;
#if 0
  else
    graph->offloadGraph(false);
#endif

  cell = NULL;
}

void
dataCell_get_module(const symbolPtr &symbol,
		    symbolPtr &module)
{
  Initialize(dataCell_get_module(const symbolPtr &, symbolPtr &));

  dataCell *cell = dataCell_of_symbol(symbol, false);

  if (!cell)
    return;

  GDGraph *graph = cell->get_graph();

  if (graph) {
    GDSet *hashtable = graph->getHashtable(true);

    if (hashtable->getProjGraph() != graph)
      graph->getFileSymbol(module);
    else {
      const symbolPtr &cellSymbol = cell->get_symbol();

      if (cell->isNodeDataCell()) {
	astXref *axr = cellSymbol.get_astXref();
	module = astXref_get_module(axr);
      } else
	module = cellSymbol->get_def_file();
    }
  }

  dataCell_offload(cell, false);
}

void
dataCell_get_graph(dataCell *cell,
		   symbolPtr &symbol)
{
  Initialize(dataCell_get_graph(dataCell *, symbolPtr &));

  if (!cell)
    return;

  GDGraph *graph = cell->get_graph();

  if (graph)
    graph->getSymbol(symbol);
}

int
dataCell_convert_from_offset(symbolPtr &data,
			     symbolPtr &symbol,
			     int offset)
{
  Initialize(dataCell_convert_from_offset(symbolPtr &, symbolPtr &, int));

  if (offset <= 0)
    return 0;

  GDGraph *graph = GDSet::search(symbol, true);

  if (!graph)
    return 0;

  dataCell *cell = (*graph)[offset - 1];

  if (!cell)
    return 0;

  symbolPtr temp(cell);
  data = temp;
  return 1;
}

int 
dataCell_get_astNode(const symbolPtr &datasym, 
		     symbolPtr &result)
{
  Initialize(dataCell_get_astNode(const symbolPtr &, symbolPtr &));
  dataCell *cell = dataCell_of_symbol(datasym, false);

  if (cell) {
    symbolPtr symbol = cell->get_symbol();
    result = symbol.get_astNode();
    dataCell_offload(cell, false);
  }

  return 
    result.isnotnull();
}

void 
dataCell_print(const symbolPtr &datasym, 
	       ostream &, 
	       int)
{
  Initialize(dataCell_print(const symbolPtr &, ostream &, int));
  dataCell *cell = dataCell_of_symbol(datasym, false);

  if (cell) {
    node_prt(cell);
    dataCell_offload(cell, false);
  }
}

symbolPtr::symbolPtr(dataCell *cell)
{
  Initialize(symbolPtr::symbolPtr(dataCell *));

  if (cell) {
    un.graph = cell->get_graph();
    offset = cell->get_id();
    offset |= 0x80000000;  // mark offset as dataCell, not xref
  } else {
    un.cp = NULL;
    offset = 0;
  }
}

symbolPtr 
dataCell_get_node(char const *lname, 
		  int offset)
{
  Initialize(dataCell_get_node(char const *, int));

  symbolPtr sym;
  astXref *axr = ast_get_astXref(lname);

  if (axr) {
    offset |= 0x80000000;  // mark offset as dataCell, not xref
    sym.put_info((scopeNode *) axr, offset);
  }

  return sym;
}

DataFlagSet::~DataFlagSet()
{
//  Initialize(DataFlagSet::~DataFlagSet());
  reset(true);
}

void
DataFlagSet::reset(bool mode)
{
//  Initialize(DataFlagSet::reset(mode));

  Iterator iter(*this);
  Object *obj;

  while (obj = ++iter) {
    DataFlag *flag = (DataFlag *) obj;

    if (!flag)
      continue;

    if (mode) {
      delete flag;
      flag = NULL;
    } else
      flag->clearFlag();
  }

  if (mode)
    removeAll();
}
    
bool
DataFlagSet::isEqualObjects(const Object &ob1,
			    const Object &ob2) const
{
//  Initialize(DataFlagSet::isEqualObjects(const Object &, const Object &) const);

  DataFlag *data1 = (DataFlag *) &ob1;
  DataFlag *data2 = (DataFlag *) &ob2;

  return
    (data1->source_ == data2->source_ &&
     data1->target_ == data2->target_);
}

unsigned int
DataFlagSet::hash_element(const Object *ob) const
{
//  Initialize(DataFlagSet::hash_element(const Object *) const);
  DataFlag *data = (DataFlag *) ob;

  return
    ((data->source_ << 16) ^ data->target_);
}

bool
DataFlagSet::addFlag(dataCell *source,
		     dataCell *target,
		     GDFlag flag)
{
//  Initialize(DataFlagSet::addFlag(dataCell *, dataCell *, GDFlag));

  bool check = false;
  unsigned int srcID = source->get_id();
  unsigned int trgID = target->get_id();

  DataFlag *temp = new DataFlag(srcID, trgID, flag);
  DataFlag *data = (DataFlag *) add(*(Object *) temp);

  if (data)
    check = data->addFlag(flag);

  if (temp && data != temp)
    delete temp;

  return check;
}

GDFlag
DataFlagSet::getFlag(dataCell *source,
		     dataCell *target)
{
//  Initialize(DataFlagSet::getFlag(dataCell *, dataCell *));

  GDFlag flag = 0;
  unsigned int srcID = source->get_id();
  unsigned int trgID = target->get_id();
  DataFlag temp(srcID, trgID, flag);
  DataFlag *data = (DataFlag *) add(*(Object *) &temp);

  if (data)
    flag = data->getFlag();

  if (flag == 0) {
    remove(*(Object *) &temp);

    if (data && data != &temp)
      delete data;
  }

  return flag;
}

void
DataFlagSet::clearFlag(dataCell *source,
		       dataCell *target)
{
//  Initialize(DataFlagSet::clearFlag(dataCell *, dataCell *));

  GDFlag flag = 0;
  unsigned int srcID = source->get_id();
  unsigned int trgID = target->get_id();
  DataFlag temp(srcID, trgID, flag);
  DataFlag *data = (DataFlag *) add(*(Object *) &temp);

  if (data)
    data->clearFlag();

  remove(*(Object *) &temp);

  if (data && data != &temp)
    delete data;
}

DataRelation::DataRelation(dataCell *source,
			   dataCell *target,
			   GDFlag flag)
{
  Initialize(DataRelation::DataRelation(dataCell *, dataCell *, GDFlag));

  graph_ = source->get_graph();
  unsigned int srcID = source->get_id();
  unsigned int trgID = target->get_id();
  DataFlag *data = new DataFlag(srcID, trgID, flag);
  data_ = *data;
  delete data;
}

DataRelation::~DataRelation() {}

dataCell *
DataRelation::getSource() const
{
  Initialize(DataRelation::getSource() const);

  graph_->loadGraph(true);
  unsigned int cellID = data_.source_;
  dataCell *cell = (*graph_)[cellID - 1];
  return cell;
}

dataCell *
DataRelation::getTarget() const
{
  Initialize(DataRelation::getTarget() const);

  graph_->loadGraph(true);
  unsigned int cellID = data_.target_;
  dataCell *cell = (*graph_)[cellID - 1];
  return cell;
}

int ATT_global(symbolPtr&);

void
DataRelation::makeConcrete(symbolArr &dataSet)
{
  Initialize(DataRelation::makeConcrete(objSet &));
  GDFlag flag = getFlag();

  if (!(flag & GDR_IRREDUCIBLE) || !graph_)
    return;

  dataCell *source = getSource();
  dataCell *target = getTarget();

  GDGraph *func = graph_->getParent();
  func->loadGraph(false);
  objSet sourceSet, targetSet;
  graph_->findRelatedCells(source, sourceSet, false, true);
  graph_->findRelatedCells(target, targetSet, false, true);
  Obj *src, *trg;

  ForEach(src, sourceSet) {
    dataCell *srcCell = (dataCell *) src;

    ForEach(trg, targetSet) {
      dataCell *trgCell = (dataCell *) trg;
      GDFlag flag = graph_->getFlag(srcCell, trgCell);

      if (flag & GDR_IRREDUCIBLE) {
	symbolPtr srcSymbol(srcCell);
	symbolPtr trgSymbol(trgCell);
	dataSet.insert_last(srcSymbol);
	dataSet.insert_last(trgSymbol);
      }
    }
  }

#if 0
  func->offloadGraph(false);
#endif
}

DataChain::~DataChain()
{
  Initialize(DataChain::~DataChain());

  Iterator iter(*this);
  Object *obj;

  while (obj = ++iter) {
    DataRelation *relation = (DataRelation *) obj;

    if (relation) {
      delete relation;
      relation = NULL;
    }
  }
}

DataRelation *
DataChain::getUnexpanded()
{
  Initialize(DataChain::getUnexpanded());

  if (unexpandedSet_.empty())
    return NULL;

  DataRelation *relation = NULL;
  Obj *obj;

  ForEach(obj, unexpandedSet_) {
    relation = (DataRelation *) obj;
    break;
  }

  unexpandedSet_.remove(relation);
  return relation;
}

DataRelation *
DataChain::getIrreducible()
{
  Initialize(DataChain::getIrreducible());

  if (irreducibleSet_.empty())
    return NULL;

  DataRelation *relation = NULL;
  Obj *obj;

  ForEach(obj, irreducibleSet_) {
    relation = (DataRelation *) obj;
    break;
  }

  irreducibleSet_.remove(relation);
  return relation;
}

bool
DataChain::addRelation(DataRelation *relation)
{
  Initialize(DataChain::addRelation(DataRelation *));

  if (!relation || includes(*relation) ||
      unexpandedSet_.includes(relation))
    return false;

  unexpandedSet_.insert(relation);
  return true;
}

unsigned int
DataChain::hash_element(const Object *ob) const
{
  Initialize(DataChain::hash_element(const Object *) const);

  DataRelation *relation = (DataRelation *) ob;
  DataFlag data = relation->data_;
  unsigned int srcID = data.source_;
  unsigned int trgID = data.target_;
  unsigned int graph = (unsigned int) relation->graph_;

  return
    (srcID ^ trgID ^ graph);
}

bool
DataChain::isEqualObjects(const Object &ob1,
			  const Object &ob2) const
{
  Initialize(DataChain::isEqualObjects(const Object &, const Object &) const);

  bool check = true;
  DataRelation *firstRelation = (DataRelation *) &ob1;
  DataRelation *secondRelation = (DataRelation *) &ob2;
  DataFlag firstData = firstRelation->data_;
  DataFlag secondData = secondRelation->data_;

  check = (firstRelation->graph_ == secondRelation->graph_ &&
	   firstData.source_ == secondData.source_ &&
	   firstData.target_ == secondData.target_ &&
	   firstData.flag_ == secondData.flag_);

  return check;
}

bool
DataChain::expandRelation(DataRelation *relation)
{
  Initialize(expandRelation(DataRelation *));

  if (!relation)
    return false;

  bool check = false;
  dataCell *source = relation->getSource();
  dataCell *target = relation->getTarget();
  GDFlag flag = relation->getFlag();
  GDGraph *graph = source->get_graph();
  GDSet *hashtable = graph->getHashtable(true);
  graph->loadGraph(true);

  if (!graph || target->get_graph() != graph) {
    delete relation;
    relation = NULL;
    return false;
  }

  if (flag & GDR_IRREDUCIBLE) {
    if (source == cell_get_alias(target)) {
      Obj *aliasSet = alias_get_cells(source);
      Obj *alias;

      ForEach(alias, *aliasSet) {
	dataCell *cell = (dataCell *) alias;

	if (cell == target)
	  continue;

	DataRelation *newRelation = new DataRelation(cell, target, flag);
	irreducibleSet_.insert(newRelation);
      }
    } else if (target == cell_get_alias(source)) {
      Obj *aliasSet = alias_get_cells(target);
      Obj *alias;

      ForEach(alias, *aliasSet) {
	dataCell *cell = (dataCell *) alias;

	if (cell == source)
	  continue;

	DataRelation *newRelation = new DataRelation(source, cell, flag);
	irreducibleSet_.insert(newRelation);
      }
    }

    if (!includes(*relation))
      irreducibleSet_.insert(relation);

    check = true;
  }

  add(*relation);

  if (flag & GDR_ALIAS_SOURCES) {
    GDFlag newFlag = flag & ~GDR_ALIAS_SOURCES;
    Obj *aliasSet = alias_get_cells(source);
    Obj *alias;

    ForEach(alias, *aliasSet) {
      dataCell *cell = (dataCell *) alias;
      DataRelation *newRelation = new DataRelation(cell, target, newFlag);
      addRelation(newRelation);
      GDFlag firstFlag = graph->getFlag(source, cell);
      GDFlag secondFlag = graph->getFlag(cell, source);

      if (firstFlag && secondFlag) {
	DataRelation *firstRelation = new DataRelation(source, cell, firstFlag);
	DataRelation *secondRelation = new DataRelation(cell, source, secondFlag);
	addRelation(firstRelation);
	addRelation(secondRelation);
      }
    }
  }

  if (flag & GDR_ALIAS_TARGETS) {
    GDFlag newFlag = flag & ~GDR_ALIAS_SOURCES;
    Obj *aliasSet = alias_get_cells(target);
    Obj *alias;

    ForEach(alias, *aliasSet) {
      dataCell *cell = (dataCell *) alias;
      DataRelation *newRelation = new DataRelation(source, cell, newFlag);
      addRelation(newRelation);
      GDFlag firstFlag = graph->getFlag(target, cell);
      GDFlag secondFlag = graph->getFlag(cell, target);

      if (firstFlag && secondFlag) {
	DataRelation *firstRelation = new DataRelation(target, cell, firstFlag);
	DataRelation *secondRelation = new DataRelation(cell, target, secondFlag);
	addRelation(firstRelation);
	addRelation(secondRelation);
      }
    }
  }

  if (flag & GDR_FIELD_SOURCE) {
    dataCell *temp = source->copy();
    dataCell::setField(temp, NULL);
    source = graph->findDataCell(temp);
    delete temp;
  }

  if (flag & GDR_FIELD_TARGET) {
    dataCell *temp = target->copy();
    dataCell::setField(temp, NULL);
    target = graph->findDataCell(temp);
    delete temp;
  }

  if (flag & GDR_MERGE) {
    if (flag & GDR_MERGE_STRAIGHT)
      expansionStep(source, target, graph, graph, GDR_MERGE_STRAIGHT);

    if (flag & GDR_MERGE_SOURCES)
      expansionStep(source, target, graph, graph, GDR_MERGE_SOURCES);

    if (flag & GDR_MERGE_TARGETS)
      expansionStep(source, target, graph, graph, GDR_MERGE_TARGETS);
  }

  if ((flag & GDR_MERGE_UP) && (hashtable->getProjGraph() != graph)) {
    Obj *callSet = graph_get_calls(graph);
    Obj *callObj;

    ForEach(callObj, *callSet) {
      GDGraph *call = (GDGraph *) callObj;
      dataCell *newSource = call->findDataCell(source);

      if (!newSource)
	continue;

      dataCell *newTarget = call->findDataCell(target);

      if (!newTarget)
	continue;

      GDFlag newFlag = call->getFlag(newSource, newTarget);

      if (newFlag) {
	DataRelation *newRelation = new DataRelation(newSource, newTarget,
						     newFlag);
	addRelation(newRelation);
      }
    }

    if (call_get_graph(graph)) {
      Obj *defnObj = call_get_defns(graph);
      objSet defnSet;
      Obj *defn;

      if (!defnObj) {
	symbolPtr symbol;
	graph->getSymbol(symbol);
	defnObj = GDSet::search(symbol, defnSet);
      }

      if (defnObj &&
	  !(source->isNodeDataCell() && (source->get_type() & dataCell_Regular)) &&
	  !(target->isNodeDataCell() && (target->get_type() & dataCell_Regular))) {
	ForEach(defn, *defnObj) {
	  GDGraph *defnGraph = (GDGraph *) defn;
	  defn_put_call(defnGraph, graph);
	  dataCell *newSource = NULL;
	  dataCell *newTarget = NULL;

	  if (source->isNodeDataCell()) {
	    dataCell *srcTemp = new dataCell(*source);
	    const symbolPtr &srcSymbol = source->get_xrefSymbol();
	    srcTemp->set_xrefSymbol(srcSymbol);
	    newSource = defnGraph->findDataCell(srcTemp);
	    delete srcTemp;
	  } else
	    newSource = defnGraph->findDataCell(source);

	  if (!newSource)
	    continue;

	  if (target->isNodeDataCell()) {
	    dataCell *trgTemp = new dataCell(*target);
	    const symbolPtr &trgSymbol = target->get_xrefSymbol();
	    trgTemp->set_xrefSymbol(trgSymbol);
	    newTarget = defnGraph->findDataCell(trgTemp);
	    delete trgTemp;
	  } else
	    newTarget = defnGraph->findDataCell(target);

	  if (!newTarget)
	    continue;

	  processRelations(newSource, newTarget, defnGraph);
	  GDFlag newFlag = defnGraph->getFlag(newSource, newTarget);

	  if (newFlag) {
	    DataRelation *newRelation = new DataRelation(newSource, newTarget,
							 newFlag);
	    addRelation(newRelation);
	  }

#if 0
	  GDGraph *parent = graph;
	  GDGraph *temp = graph;

	  while (temp = call_get_graph(temp))
	    parent = temp;

	  if (defnGraph != parent)
	    defnGraph->offloadGraph(false);
#endif
	}
      }
    }
  }

  if ((flag & GDR_MERGE_UP) && (hashtable->getProjGraph() == graph)) {
    symbolArr graphSet;

    if (source->isCallDataCell()) {
      const symbolPtr &context = source->getContext();
      graphSet.insert_last(context);
    } else if (target->isCallDataCell()) {
      const symbolPtr &context = target->getContext();
      graphSet.insert_last(context);
    } else if (!(source->get_type() & dataCell_Regular)) {
      const symbolPtr &context = source->get_xrefSymbol();
      graphSet.insert_last(context);
    } else if (!(target->get_type() & dataCell_Regular)) {
      const symbolPtr &context = target->get_xrefSymbol();
      graphSet.insert_last(context);
    } else {
      symbolPtr srcSymbol = source->get_xrefSymbol();
      symbolPtr trgSymbol = target->get_xrefSymbol();
      symbolArr srcArr, trgArr;
      srcSymbol.get_local_link(used_by, srcArr);
      trgSymbol.get_local_link(used_by, trgArr);
      unsigned int srcSize = srcArr.size();
      unsigned int trgSize = trgArr.size();

      if (srcSize != 0 && trgSize != 0) {
	symbolSet trgSet;
	trgSet = trgArr;

	for (unsigned int m = 0; m < srcSize; ++m) {
	  const symbolPtr &tmpSymbol = srcArr[m];

	  if (trgSet.includes(tmpSymbol))
	    graphSet.insert_last(tmpSymbol);
	}
      }
    }

    unsigned int size = graphSet.size();

    for (unsigned int k = 0; k < size; ++k) {
      symbolPtr local = graphSet[k];
      GDGraph *localGraph = hashtable->find(local, false);

      if (!localGraph)
	continue;

      dataCell *newSource = localGraph->findDataCell(source);

      if (!newSource)
	continue;

      dataCell *newTarget = localGraph->findDataCell(target);

      if (!newTarget)
	continue;

      processRelations(newSource, newTarget, localGraph);
    }
  }

#if 0
  graph->offloadGraph(false);
#endif

  return check;
}

unsigned int
DataChain::processRelations(dataCell *source,
			    dataCell *target,
			    GDGraph *graph)
{
  Initialize(DataChain::processRelations(dataCell *, dataCell *, GDGraph *));

  unsigned int retval = 0;
  dataCell *realSource = source;
  dataCell *realTarget = target;
  dataCell *srcAlias = cell_get_alias(source);
  dataCell *trgAlias = cell_get_alias(target);
  DataRelation *srcRelation = NULL;
  DataRelation *trgRelation = NULL;

  if (srcAlias) {
    GDFlag srcFlag = graph->getFlag(source, srcAlias);

    if (srcFlag)
      srcRelation = new DataRelation(source, srcAlias, srcFlag);

    realSource = srcAlias;
  }

  if (trgAlias) {
    GDFlag trgFlag = graph->getFlag(trgAlias, target);

    if (trgFlag)
      trgRelation = new DataRelation(trgAlias, target, trgFlag);

    realTarget = trgAlias;
  }

  if (realSource == realTarget) {
    if (srcRelation) {
      if (addRelation(srcRelation))
	++retval;
    }

    if (trgRelation) {
      if (addRelation(trgRelation))
	++retval;
    }

    return retval;
  }

  GDFlag flag = graph->getFlag(realSource, realTarget);

  if (!flag)
    return retval;

  DataRelation *relation = new DataRelation(source, target, flag);

  if (addRelation(relation))
    ++retval;

  if (srcAlias && srcAlias != target) {
    relation = new DataRelation(srcAlias, target, flag);

    if (addRelation(relation))
      ++retval;
  }

  if (trgAlias && trgAlias != source) {
    relation = new DataRelation(source, trgAlias, flag);

    if (addRelation(relation))
      ++retval;
  }

  if (srcAlias && trgAlias && srcAlias != trgAlias) {
    relation = new DataRelation(srcAlias, trgAlias, flag);

    if (addRelation(relation))
      ++retval;
  }

  if (srcRelation) {
    if (addRelation(srcRelation))
      ++retval;
  }

  if (trgRelation) {
    if (addRelation(trgRelation))
      ++retval;
  }

  return retval;
}

unsigned int
DataChain::construct(const symbolPtr &srcSymbol,
		     const symbolPtr &trgSymbol,
		     GDGraph *graph)
{
  Initialize(DataChain::construct(const symbolPtr &, const symbolPtr &, GDGraph *));

  unsigned int retval = 0;
  symbolArr srcData, trgData;
  graph->getDataCells((symbolPtr &) srcSymbol, srcData, true);
  graph->getDataCells((symbolPtr &) trgSymbol, trgData, true);
  symbolPtr src, trg;

  ForEachS(src, srcData) {
    GDGraph *call = src.get_graph();

    if (!call)
      continue;

    dataCell *source = dataCell_of_symbol(src, true);

    if (!source)
      continue;

    dataCell *realSource = source;
    dataCell *srcAlias = (dataCell *) cell_get_alias(source);
    DataRelation *srcRelation = NULL;
    bool srcCheck = false;

    if (srcAlias) {
      GDFlag srcFlag = call->getFlag(source, srcAlias);

      if (srcFlag)
	srcRelation = new DataRelation(source, srcAlias, srcFlag);

      realSource = srcAlias;
    }

    ForEachS(trg, trgData) {
      if (trg.get_graph() != call)
	continue;

      dataCell *target = dataCell_of_symbol(trg, true);

      if (!target || source == target)
	continue;

      dataCell *realTarget = target;
      dataCell *trgAlias = (dataCell *) cell_get_alias(target);
      DataRelation *trgRelation = NULL;
      bool trgCheck = false;

      if (trgAlias) {
	GDFlag trgFlag = call->getFlag(trgAlias, target);

	if (trgFlag)
	  trgRelation = new DataRelation(trgAlias, target, trgFlag);

	realTarget = trgAlias;
      }

      if (realSource == realTarget) {
	if (srcRelation) {
	  if (addRelation(srcRelation))
	    ++retval;

	  srcCheck = true;
	}

	if (trgRelation) {
	  if (addRelation(trgRelation))
	    ++retval;

	  trgCheck = true;
	}

	continue;
      }

      GDFlag flag = call->getFlag(realSource, realTarget);

      if (!flag) {
	if (trgRelation && !trgCheck) {
	  delete trgRelation;
	  trgRelation = NULL;
	}

	continue;
      }

      DataRelation *relation = new DataRelation(source, target, flag);

      if (addRelation(relation))
	++retval;

      if (srcAlias && srcAlias != target) {
	relation = new DataRelation(srcAlias, target, flag);

	if (addRelation(relation))
	  ++retval;
      }

      if (trgAlias && trgAlias != source) {
	relation = new DataRelation(source, trgAlias, flag);

	if (addRelation(relation))
	  ++retval;
      }

      if (srcAlias && trgAlias && srcAlias != trgAlias) {
	relation = new DataRelation(srcAlias, trgAlias, flag);

	if (addRelation(relation))
	  ++retval;
      }

      if (srcRelation && !srcCheck) {
	if (addRelation(srcRelation))
	  ++retval;

	srcCheck = true;
      }

      if (trgRelation && !trgCheck) {
	if (addRelation(trgRelation))
	  ++retval;

	trgCheck = true;
      }
    }
  }

  return retval;
}

void
DataChain::expansionStep(dataCell *source,
			 dataCell *target,
			 GDGraph *srcGraph,
			 GDGraph *trgGraph,
			 GDFlag flag)
{
  Initialize(expansionStep(dataCell *, dataCell *, GDGraph *, GDGraph *,  GDFlag));

  if (!source || !target)
    return;

  objSet srcSet, trgSet;
  srcGraph->findRelatedCells(source, srcSet, true, false);
  trgGraph->findRelatedCells(target, trgSet, true, false);
  Obj *src, *trg;

  ForEach(src, srcSet) {
    dataCell *srcCell = (dataCell *) src;
    int srcDiff = 0;

    source->cellCompare3(srcCell, srcDiff);

    if (srcDiff < 0)
      continue;

    dataCell *srcAlias = cell_get_alias(srcCell);
    DataRelation *srcRelation = NULL;
    bool srcCheck = false;
    Obj *firstSet;
    Obj *first;

    if (flag & GDR_MERGE_STRAIGHT) {
      if (srcAlias) {
	GDFlag srcFlag = srcGraph->getFlag(srcCell, srcAlias);

	if (srcFlag)
	  srcRelation = new DataRelation(srcCell, srcAlias, srcFlag);

	srcCell = srcAlias;
      }

      firstSet = src_get_trgs(srcCell);
    } else if (flag & GDR_MERGE_SOURCES) {
      if (srcAlias) {
	GDFlag srcFlag = srcGraph->getFlag(srcCell, srcAlias);

	if (srcFlag)
	  srcRelation = new DataRelation(srcCell, srcAlias, srcFlag);

	srcCell = srcAlias;
      }

      firstSet = src_get_trgs(srcCell);
    } else if (flag & GDR_MERGE_TARGETS) {
      if (srcAlias) {
	GDFlag srcFlag = srcGraph->getFlag(srcAlias, srcCell);

	if (srcFlag)
	  srcRelation = new DataRelation(srcAlias, srcCell, srcFlag);

	srcCell = srcAlias;
      }

      firstSet = trg_get_srcs(srcCell);
    }

    ForEach(trg, trgSet) {
      dataCell *trgCell = (dataCell *) trg;
      int trgDiff = 0;
      int adjustment = 0;

      target->cellCompare3(trgCell, trgDiff);

      if (trgDiff < 0)
	continue;

      if (srcDiff > 0 && trgDiff > 0)
	continue;

      if (srcDiff == 0)
	adjustment = trgDiff;
      else
	adjustment = srcDiff;

      dataCell *trgAlias = cell_get_alias(trgCell);
      DataRelation *trgRelation = NULL;
      bool trgCheck = false;
      Obj *secondSet;
      Obj *second;

      if (flag & GDR_MERGE_STRAIGHT) {
	if (trgAlias) {
	  GDFlag trgFlag = trgGraph->getFlag(trgAlias, trgCell);

	  if (trgFlag)
	    trgRelation = new DataRelation(trgAlias, trgCell, trgFlag);

	  trgCell = trgAlias;
	}

	secondSet = trg_get_srcs(trgCell);
      } else if (flag & GDR_MERGE_SOURCES) {
	if (adjustment == 0)
	  continue;

	if (trgAlias) {
	  GDFlag trgFlag = trgGraph->getFlag(trgCell, trgAlias);

	  if (trgFlag)
	    trgRelation = new DataRelation(trgCell, trgAlias, trgFlag);

	  trgCell = trgAlias;
	}

	secondSet = src_get_trgs(trgCell);
      } else if (flag & GDR_MERGE_TARGETS) {
	if (adjustment == 0)
	  continue;

	if (trgAlias) {
	  GDFlag trgFlag = trgGraph->getFlag(trgAlias, trgCell);

	  if (trgFlag)
	    trgRelation = new DataRelation(trgAlias, trgCell, trgFlag);

	  trgCell = trgAlias;
	}

	secondSet = trg_get_srcs(trgCell);
      }

      ForEach(first, *firstSet) {
	dataCell *firstCell = (dataCell *) first;

	ForEach(second, *secondSet) {
	  dataCell *secondCell = (dataCell *) second;
	  int diff = 0;

	  if (!firstCell->cellCompare2(secondCell, diff, false, false))
	    continue;

	  if (diff != adjustment && diff != 0 - adjustment)
	    continue;

	  GDFlag firstFlag = 0, secondFlag = 0;

	  if (flag & GDR_MERGE_STRAIGHT) {
	    firstFlag = srcGraph->getFlag(srcCell, firstCell);
	    secondFlag = trgGraph->getFlag(secondCell, trgCell);

	    if (!firstFlag || !secondFlag) {
	      if (trgRelation && !trgCheck) {
		delete trgRelation;
		trgRelation = NULL;
	      }

	      continue;
	    }

	    DataRelation *firstRelation = new DataRelation(srcCell, firstCell,
							   firstFlag);
	    DataRelation *secondRelation = new DataRelation(secondCell, trgCell,
							    secondFlag);
	    addRelation(firstRelation);
	    addRelation(secondRelation);

	    if (srcRelation && !srcCheck) {
	      addRelation(srcRelation);
	      srcCheck = true;
	    }

	    if (trgRelation && !trgCheck) {
	      addRelation(trgRelation);
	      trgCheck = true;
	    }
	  } else if (flag & GDR_MERGE_SOURCES) {
	    firstFlag = srcGraph->getFlag(srcCell, firstCell);
	    secondFlag = trgGraph->getFlag(trgCell, secondCell);

	    if (!firstFlag || !secondFlag) {
	      if (trgRelation && !trgCheck) {
		delete trgRelation;
		trgRelation = NULL;
	      }

	      continue;
	    }

	    DataRelation *firstRelation = new DataRelation(srcCell, firstCell,
							   firstFlag);
	    DataRelation *secondRelation = new DataRelation(trgCell, secondCell,
							    secondFlag);
	    addRelation(firstRelation);
	    addRelation(secondRelation);

	    if (srcRelation && !srcCheck) {
	      addRelation(srcRelation);
	      srcCheck = true;
	    }

	    if (trgRelation && !trgCheck) {
	      addRelation(trgRelation);
	      trgCheck = true;
	    }
	  } else if (flag & GDR_MERGE_TARGETS) {
	    firstFlag = srcGraph->getFlag(firstCell, srcCell);
	    secondFlag = trgGraph->getFlag(secondCell, trgCell);

	    if (!firstFlag || !secondFlag) {
	      if (trgRelation && !trgCheck) {
		delete trgRelation;
		trgRelation = NULL;
	      }

	      continue;
	    }

	    DataRelation *firstRelation = new DataRelation(firstCell, srcCell,
							   firstFlag);
	    DataRelation *secondRelation = new DataRelation(secondCell, trgCell,
							    secondFlag);
	    addRelation(firstRelation);
	    addRelation(secondRelation);

	    if (srcRelation && !srcCheck) {
	      addRelation(srcRelation);
	      srcCheck = true;
	    }

	    if (trgRelation && !trgCheck) {
	      addRelation(trgRelation);
	      srcCheck = true;
	    }
	  }
	}
      }

      if (trgRelation && !trgCheck)
	delete trgRelation;
    }

    if (srcRelation && !srcCheck)
      delete srcRelation;
  }

#if 0
  srcGraph->offloadGraph(false);
  trgGraph->offloadGraph(false);
#endif
}

const symbolPtr &
dataCell_get_symbol(dataCell *cell)
{
  return
    cell->get_symbol();
}

const symbolPtr &
dataCell_get_xrefSymbol(dataCell *cell)
{
  return
    cell->get_xrefSymbol();
}

void 
dataCell_print(dataCell *cell,
	       ostream &os,
	       int level)
{
  cell->print(os, level);
}
