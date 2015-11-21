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
#ifndef _DFA_FLOW_H_
#define _DFA_FLOW_H_

#include "Relational.h"
#include "objOper.h"
#include "symbolPtr.h"
#include "genError.h"
#include "general.h"
#include "machdep.h"
#include "dfa_db.h"
#include "objArr.h"
#include "ddict.h"
#include "xrefSymbol.h"

#define DFA_MAX_LEVEL            3

#include <dis_iosfwd.h>

class intArr;
class GDGraph;
class dataCell;
class DataFlag;
class DataFlagSet;
class DataRelation;
class DataChain;

typedef unsigned char GDType;

#define GDT_LOCAL           0x1
#define GDT_GLOBAL          0x2
#define GDT_UPDATED         0x4

typedef unsigned int dataCell_type;

#define dataCell_Regular    0x1
#define dataCell_Argument   0x2
#define dataCell_Result     0x4
#define dataCell_Abstract   0x8
#define dataCell_Mask1      0x0F

#define dataCell_Field      0x10
#define dataCell_Parent     0x20
#define dataCell_JavaObject 0x40
#define dataCell_AstNode    0x80
#define dataCell_Mask2      0x0FF

#define dataCell_NewFlag    0x100
#define dataCell_OldFlag    0x200
#define dataCell_UpFlag     0x400
#define dataCell_DownFlag   0x800
#define dataCell_Mask3      0x0FFF

#define dataCell_Checked    0x1000
#define dataCell_Constant   0x2000
#define dataCell_Shared     0x4000

enum GDFormat {
  GDF_ZOMBIE,
  GDF_ACTIVE,
  GDF_PASSIVE
};

enum DataQuery {
  DFA_INFLOW,
  DFA_OUTFLOW
};

typedef unsigned int GDFlag;

#define GDR_IRREDUCIBLE    0x1
#define GDR_ALIAS          0x2
#define GDR_GLOBAL         0x4
#define GDR_MERGE          0x8
#define GDR_MERGE_UP       0x10
#define GDR_MERGE_STRAIGHT 0x20
#define GDR_MERGE_SOURCES  0x40
#define GDR_MERGE_TARGETS  0x80
#define GDR_FIELD_SOURCE   0x100
#define GDR_FIELD_TARGET   0x200
#define GDR_ALIAS_SOURCES  0x400
#define GDR_ALIAS_TARGETS  0x800
#define GDR_FIELD_ALIAS    0x1000

#define DML_UNKNOWN        (char) -2
#define DML_CONSTANT       (char) -1

extern char const *dataCell_types[];

void dataCell_print(const symbolPtr &, ostream &, int);
int dataCell_get_line_number(const symbolPtr &);
int dataCell_get_char_offset(const symbolPtr &);
int parse_args(symbolPtr &symbol, char * &arguments, genArrOf(int) &indices);
dataCell *dataCell_of_symbol(const symbolPtr &symbol, bool load);
void dataCell_offload(dataCell *&cell, bool mode);
const symbolPtr &dataCell_get_symbol(dataCell *cell);
void dataCell_print(dataCell *cell, ostream &os, int level);

declare_rel(dataCell,src,dataCell,trg);
declare_rel(dataCell,cell,dataCell,alias);

class dataCell : public Relational {

public:

  dataCell()
    : level_(0), maxLevel_(DML_UNKNOWN), argno_(0), type_(0),
        graph_(NULL), field_(NULL), depth_(0) {
	++dataCell::newCount_;
      }

  dataCell(int level, unsigned int argno, dataCell_type type)
    : level_(level), maxLevel_(DML_UNKNOWN), argno_(argno), type_(type),
        graph_(NULL), field_(NULL), depth_(0) {
	++dataCell::newCount_;
      }

  dataCell(const dataCell &that);
  dataCell(unsigned char const *&start, GDGraph *graph);
  virtual ~dataCell();

  inline const symbolPtr &get_symbol() const;
  inline const symbolPtr &get_astnode() const;
  inline void set_symbol(const symbolPtr &symbol);
  inline void set_typeSymbol(const symbolPtr &symbol);
  inline void set_astnode(const symbolPtr &symbol);
  inline int get_level() const;
  inline void set_level(int level);
  inline unsigned int get_argno() const;
  inline void set_argno(unsigned int argno);
  inline dataCell_type get_type() const;
  inline void set_type(dataCell_type type);
  inline void add_type(dataCell_type type);
  inline void rem_type(dataCell_type type);
  inline GDGraph *get_graph() const;
  inline void set_graph(GDGraph *graph);
  inline bool isAbstract() const;
  inline bool isValid() const;
  inline dataCell *getField() const;
  inline int get_maxLevel() const;
  inline void set_maxLevel(int level);
  inline unsigned int get_depth() const;
  inline void set_depth(unsigned int depth);
  inline void cleanSelf();

  bool isChecked();
  bool shiftLevel(int adjustment);
  bool cellCompare(dataCell *cell, bool mode = true,
		   bool rigid = true, bool ignore = false);
  bool cellCompare2(dataCell *cell, int &adjustment,
		    bool rigid, bool ignore);
  void cellCompare3(dataCell *cell, int &adjustment);
  bool transform(unsigned int adjustment, dataCell *field,
		 objSet &dataSet, GDGraph *graph);
  bool processLevel(bool &ref);
  void get_typeSymbol(symbolPtr &symbol);
  bool pass() const;
  bool isConstArg();

  virtual const symbolPtr &get_xrefSymbol() const;
  virtual void set_xrefSymbol(const symbolPtr &symbol);
  virtual symbolPtr getContext() const;
  virtual void setContext(const symbolPtr &symbol);
  virtual char const *get_name() const;
  virtual void print(ostream &os = cout, int lev = 0) const;
  virtual unsigned int getSize() const;
  virtual dataCell *copy() const;
  virtual void dump(ostream &os);
  virtual bool isNodeDataCell() const;
  virtual bool isCallDataCell() const;

  static dataCell *newCell(unsigned char const *&start, GDGraph *graph);
  static dataCell *newCell(symbolPtr &symbol, bool mode);
  static bool symbolCompare(const symbolPtr &source,
			    const symbolPtr &target);
  static bool setField(dataCell *&cell, dataCell *field, bool mode = true);
  static inline unsigned int DB_SIZE();

  friend ostream &operator<<(ostream &os, dataCell *cell);

  define_relational(dataCell,Relational);
  define_ptr(dataCell,src,dataCell,trg);
  define_ptr(dataCell,trg,dataCell,src);
  define_ptr(dataCell,cell,dataCell,alias);
  define_ptr(dataCell,alias,dataCell,cell);

protected:

  char const *get_name(char const *name) const;

  symbolPtr symbol_;
  char level_;
  char maxLevel_;
  unsigned char argno_;
  dataCell_type type_;
  GDGraph *graph_;
  dataCell *field_;
  symbolPtr class_;
  unsigned char depth_;

public:

  static unsigned int newCount_;
  static unsigned int deleteCount_;
  static unsigned int newAliasCount_;
  static unsigned int oldAliasCount_;
  static unsigned int checkedCount_;

};

class nodeDataCell : public dataCell {

public:

  nodeDataCell() : dataCell() {
    add_type(dataCell_AstNode);
  }

  nodeDataCell(const nodeDataCell &that)
    : dataCell(that), xref_(that.xref_) {}

  nodeDataCell(const dataCell &that) : dataCell(that) {
    add_type(dataCell_AstNode);
  }

  nodeDataCell(unsigned char const *&start, GDGraph *graph);

  virtual const symbolPtr &get_xrefSymbol() const;
  virtual void set_xrefSymbol(const symbolPtr &symbol);
  virtual unsigned int getSize() const;
  virtual dataCell *copy() const;
  virtual void dump(ostream &os);
  virtual bool isNodeDataCell() const;

  static inline unsigned int DB_SIZE();

  define_relational(nodeDataCell,dataCell);

protected:

  symbolPtr xref_;

};

class callDataCell : public nodeDataCell {

public:

  callDataCell() : nodeDataCell() {}
  callDataCell(const callDataCell &that)
    : nodeDataCell(that), context_(that.context_) {}

  callDataCell(const dataCell &that);
  callDataCell(unsigned char const *&start, GDGraph *graph);

  virtual symbolPtr getContext() const;
  virtual void setContext(const symbolPtr &symbol);
  virtual unsigned int getSize() const;
  virtual dataCell *copy() const;
  virtual void dump(ostream &os);
  virtual bool isCallDataCell() const;

  static inline unsigned int DB_SIZE();

  define_relational(callDataCell,nodeDataCell);

protected:

  symbolPtr context_;

};

generate_descriptor(dataCell,Relational);
defrel_many_to_many(dataCell,src,dataCell,trg);
defrel_many_to_one(dataCell,cell,dataCell,alias);

class DataFlag {

public:

  DataFlag(unsigned int source = 0, unsigned int target = 0, GDFlag flag = 0)
    : source_(source), target_(target), flag_(flag) {}

  inline bool addFlag(GDFlag flag);
  inline GDFlag getFlag() const;
  inline void clearFlag();

private:

  friend class DataFlagSet;
  friend class DataRelation;
  friend class DataChain;

  unsigned int source_;
  unsigned int target_;
  GDFlag flag_;

};

class DataFlagSet : public Set {

public:

  virtual ~DataFlagSet();

  bool addFlag(dataCell *source, dataCell *target, GDFlag flag);
  GDFlag getFlag(dataCell *source, dataCell *target);
  void clearFlag(dataCell *source, dataCell *target);
  void reset(bool mode);

  virtual unsigned int hash_element(const Object *ob) const;
  virtual bool isEqualObjects(const Object &ob1, const Object &ob2) const;

};

class DataRelation : public Obj {

public:

  DataRelation(GDGraph *graph, const DataFlag &data)
    : graph_(graph), data_(data) {}

  DataRelation(dataCell *source, dataCell *target, GDFlag flag);
  virtual ~DataRelation();

  void makeConcrete(symbolArr &dataSet);
  dataCell *getSource() const;
  dataCell *getTarget() const;
  inline GDFlag getFlag() const;

private:

  friend class DataChain;

  GDGraph *graph_;
  DataFlag data_;

};

class DataChain : public Set {

public:

  virtual ~DataChain();

  bool expandRelation(DataRelation *relation);
  DataRelation *getUnexpanded();
  DataRelation *getIrreducible();
  bool addRelation(DataRelation *relation);
  unsigned int processRelations(dataCell *source, dataCell *target,
				GDGraph *graph);
  unsigned int construct(const symbolPtr &srcSymbol,
			 const symbolPtr &trgSymbol,
			 GDGraph *graph);

  virtual unsigned int hash_element(const Object *ob) const;
  virtual bool isEqualObjects(const Object &ob1, const Object &ob2) const;

  inline bool unexpanded() const;
  inline bool irreducible() const;

protected:

  void expansionStep(dataCell *source, dataCell *target,
		     GDGraph *srcGraph, GDGraph *trgGraph, GDFlag flag);

private:

  objSet unexpandedSet_;
  objSet irreducibleSet_;

};

inline
const symbolPtr &
dataCell::get_symbol() const
{
  return symbol_;
}

inline
const symbolPtr &
dataCell::get_astnode() const
{
  return symbol_;
}

inline
void
dataCell::set_symbol(const symbolPtr &symbol)
{
  symbol_ = symbol;
}

inline
void
dataCell::set_typeSymbol(const symbolPtr &symbol)
{
  class_ = symbol;
}

inline
void
dataCell::set_astnode(const symbolPtr &symbol)
{
  symbol_ = symbol;
}

inline
int
dataCell::get_level() const
{
  return level_;
}

inline
void
dataCell::set_level(int level)
{
  level_ = level;
}

inline
unsigned int
dataCell::get_argno() const
{
  return argno_;
}

inline
void
dataCell::set_argno(unsigned int argno)
{
  argno_ = argno;
}

inline
dataCell_type
dataCell::get_type() const
{
  return type_;
}

inline
void
dataCell::set_type(dataCell_type type)
{
  type_ = type;
}

inline
void
dataCell::add_type(dataCell_type type)
{
  type_ |= type;
}

inline
void
dataCell::rem_type(dataCell_type type)
{
  type_ &= ~type;
}

inline
GDGraph *
dataCell::get_graph() const
{
  return graph_;
}

inline
void
dataCell::set_graph(GDGraph *graph)
{
  graph_ = graph;
}

inline
bool
dataCell::isAbstract() const
{
  bool abstract = false;

  if (type_ & dataCell_Abstract)
    abstract = true;

  return abstract;
}

inline
bool
dataCell::isValid() const
{
  return
    symbol_.isnotnull();
}

inline
dataCell *
dataCell::getField() const
{
  return field_;
}

inline
int
dataCell::get_maxLevel() const
{
  int level = (int) maxLevel_;

  if (maxLevel_ == DML_CONSTANT)
    level = 0;

  return level;
}

inline
void
dataCell::set_maxLevel(int level)
{
  maxLevel_ = (char) level;
}

inline
unsigned int
dataCell::get_depth() const
{
  return
    (unsigned int) depth_;
}

inline
void
dataCell::set_depth(unsigned int depth)
{
  depth_ = (unsigned char) depth;
}

inline
void
dataCell::cleanSelf()
{
  add_type(dataCell_Checked);
  src_rem_trgs(this);
  trg_rem_srcs(this);
  cell_rem_alias(this);
}

inline
bool
DataFlag::addFlag(GDFlag flag)
{
  bool check = true;

  if (flag_ & flag)
    check = false;
  else
    flag_ |= flag;

  return check;
}

inline
GDFlag
DataFlag::getFlag() const
{
  return flag_;
}

inline
void
DataFlag::clearFlag()
{
  flag_ = 0;
}

inline
GDFlag
DataRelation::getFlag() const
{
  return
    data_.getFlag();
}

inline
bool
DataChain::unexpanded() const
{
  return
    (bool) unexpandedSet_.size();
}

inline
bool
DataChain::irreducible() const
{
  return
    (bool) irreducibleSet_.size();
}

#endif                  // _DFA_FLOW_H
