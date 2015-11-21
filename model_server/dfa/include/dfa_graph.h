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
#ifndef _DFA_GDG_
#define _DFA_GDG_

#include "dfa.h"
#include "dfa_flow.h"
#ifndef ISO_CPP_HEADERS
#include "fstream.h"
#else /* ISO_CPP_HEADERS */
#include <fstream>
using namespace std;
#endif /* ISO_CPP_HEADERS */
#include <tcl.h>
#include <Interpreter.h>
#include <objRelmacros.h>
#include <symbolSet.h>
#include <symbolArr.h>
#include <timer.h>
#include <driver_mode.h>

#define PMOD_DFA_TOKEN     (unsigned char) '='

#define DFA_NOTIFY_INCREMENT 100

class GDInteger;
class GDHeader;
class GDGraph;
class GDSet;
class DataSet;
class classInfo;

typedef DataSet * DataSetPtr;
typedef GDGraph * GDPointer;
typedef objArr  * objArrPtr;

genArr(DataSetPtr);
genArr(objArrPtr);

Interpreter *InitializeCLIInterpreter();
symbolPtr dfa_decl_of_ast(symbolPtr &ast);
int ATT_is_loaded(symbolPtr &sym);
int dis_print_verbose(char const *fmt, ...);
int projNode_set_pmod_names(projNode *proj);
unsigned int dfa_enabled();
classInfo *classInfo_create();
//void classInfo_delete(classInfo *ci);
bool classInfo_has_field(classInfo *ci, const symbolPtr &cl,
			 const symbolPtr &field);
symbolPtr api_lookup_xrefSymbol(ddKind kind, char const *name);
void call_graph_closure(symbolArr &funs, symbolArr &out,
			void(*q)(const symbolPtr &, symbolArr &));

enum GDMode {
  GDM_NONE = 0,
  GDM_ABSTRACT,
  GDM_CONCRETE,
  GDM_COMPLETE,
  GDM_OFFLOADED
};

class GDInteger {

public:

  GDInteger(unsigned int number = 0) {
    byte_1 = number >> 24;
    byte_2 = (number >> 16) & 0xFF;
    byte_3 = (number >> 8) & 0xFF;
    byte_4 = number & 0xFF;
  }

  GDInteger(unsigned char const *start)
    : byte_1(*start), byte_2(*(start + 1)),
      byte_3(*(start + 2)), byte_4(*(start + 3)) {}

  GDInteger &operator=(unsigned int number) {
    byte_1 = number >> 24;
    byte_2 = (number >> 16) & 0xFF;
    byte_3 = (number >> 8) & 0xFF;
    byte_4 = number & 0xFF;
    return *this;
  }

  operator unsigned() const {
    return
      (((((byte_1 << 8) | byte_2) << 8) | byte_3) << 8) | byte_4;
  }

  GDInteger operator++() {
    unsigned int number = ((unsigned int) *this) + 1;
    byte_1 = number >> 24;
    byte_2 = (number >> 16) & 0xFF;
    byte_3 = (number >> 8) & 0xFF;
    byte_4 = number & 0xFF;
    return *this;
  }

  GDInteger operator++(int x) {
    unsigned int number = ((unsigned int) *this) + 1;
    byte_1 = number >> 24;
    byte_2 = (number >> 16) & 0xFF;
    byte_3 = (number >> 8) & 0xFF;
    byte_4 = number & 0xFF;
    return *this;
  }

  GDInteger operator*(unsigned int factor) {
    return
      ((unsigned int) *this) * factor;
  }

  GDInteger operator+(unsigned int factor) {
    return
      ((unsigned int) *this) + factor;
  }

  friend ostream &operator<<(ostream &os, GDInteger number) {
    os << number.byte_1;
    os << number.byte_2;
    os << number.byte_3;
    os << number.byte_4;
    return os;
  }

  static inline unsigned int DB_SIZE();

private:

  unsigned char byte_1;
  unsigned char byte_2;
  unsigned char byte_3;
  unsigned char byte_4;

};

class GDHeader {

public:

  GDHeader(unsigned int symbolID, unsigned int nodeID = 0,
	   unsigned int fileID = 0)
    : argno_(0), status_(0) {
      /* below are, actually, operators */
      symbolID_ = symbolID;
      nodeID_ = nodeID;
      fileID_ = fileID;
      cellNum_ = 0;
      callNum_ = 0;
      size_ = 0;
      totalSize_ = 0;
    }

  GDHeader(unsigned char const *start)
    : symbolID_(start),
      nodeID_(start + GDInteger::DB_SIZE()),
      fileID_(start + 2 * GDInteger::DB_SIZE()),
      cellNum_(start + 3 * GDInteger::DB_SIZE()),
      callNum_(start + 4 * GDInteger::DB_SIZE()),
      size_(start + 5 * GDInteger::DB_SIZE()),
      totalSize_(start + 6 * GDInteger::DB_SIZE()),
      argno_(*(start + 7 * GDInteger::DB_SIZE())),
      status_(*(start + 7 * GDInteger::DB_SIZE() + 1)) {}

  inline unsigned int getSymbolID() const;
  inline unsigned int getNodeID() const;
  inline unsigned int getFileID() const;
  inline unsigned int getCellNumber() const;
  inline unsigned int getCallNumber() const;
  inline unsigned int getSize() const;
  inline unsigned int getTotalSize() const;
  inline unsigned int getArgno() const;
  inline GDType getStatus() const;
  inline void setSymbolID(unsigned int);
  inline void setCellNumber(unsigned int number);
  inline void setCallNumber(unsigned int number);
  inline void setSize(unsigned int number);
  inline void setTotalSize(unsigned int number);
  inline void setArgno(unsigned int number);
  inline void setStatus(GDType status);
  inline void update(GDType status);

  static inline unsigned int DB_SIZE();

private:

  GDInteger symbolID_;
  GDInteger nodeID_;
  GDInteger fileID_;
  GDInteger cellNum_;
  GDInteger callNum_;
  GDInteger size_;
  GDInteger totalSize_;
  unsigned char argno_;
  unsigned char status_;

};

class GDGraph : public Relational {

public:

  GDGraph(GDHeader *header, scopeNode *scope,
	  unsigned char const *start = NULL, GDFormat format = GDF_ACTIVE)
    : header_(header), start_(start), scope_(scope), format_(format),
      mode_(GDM_NONE), recursion_(false), hashArr_(NULL), base_(NULL),
      indSet_(is_model_build()) {}

  GDGraph(ifstream &inStream, scopeNode *scope);
  virtual ~GDGraph();

  void closeGraph(bool abstract, bool override = false);
  bool dataFlow(symbolPtr &symbol, symbolArr &results, symbolPtr &keySymbol,
		dataCell *queryCell, DataQuery query);
  unsigned int closeRelations(bool abstract, bool override = false);
  unsigned int mergeRelations(bool abstract);
  unsigned int combineRelations(GDGraph *graph, bool abstract);
  unsigned int addRelation(dataCell *&source, dataCell *&target, GDFlag flag,
			   bool abstract = true, bool srcClean = false,
			   bool trgClean = false, int mode = 0);
  unsigned int iterationStep(dataCell *firstCell, dataCell *secondCell,
			     int adjustment, GDFlag flag,
			     bool abstract = true);
  void cleanRelations();
  void printRelations(ostream &outStream);
  void getCallTree(symbolSet &resultSet);
  bool addDataCell(dataCell *&cell, bool clean);
  void setAlias(dataCell *source, dataCell *target, int mode = 0);
  void moveRelations(dataCell *cell, dataCell *alias, int mode);
  dataCell *findDataCell(dataCell *cell, bool mode = false,
			 bool recurse = true);
  bool repairCell(dataCell *cell);
  DataSet *find(dataCell *cell);
  DataSet *find(const symbolPtr &symbol);
  void findRelatedCells(dataCell *cell, objSet &dataSet,
			bool abstract, bool mode = true);
  void getDataCells(symbolPtr &symbol, symbolArr &results, bool mode);
  void getAllDataCells(symbolArr &results, bool abstract);
  GDGraph *findCall(const symbolPtr &symbol);
  void processFlow(Obj *sourceSet, Obj *targetSet, bool abstract);
  void loadGraph(bool abstract, bool mode = true, bool recurse = true);
  void offloadGraph(bool mode, GDGraph *graph = NULL, bool recurse = true);
  void getSymbol(symbolPtr &symbol, GDSet *hashtable = NULL) const;
  void getAstSymbol(symbolPtr &symbol, bool mode = true) const;
  void getFileSymbol(symbolPtr &symbol) const;
  void setFormat(GDFormat format, bool mode = false);
  void loadCalls(bool mode = true);
  void loadRelations();
  void settleHeader();
  void reshuffle();
  void setOldCells();
  void setDownCells();
  void resetNewCells();
  void resetUpCells();
  void resetOldCells();
  void renewOldCells();
  void renewDownCells();
  GDGraph *getParent() const;
  void updateBaseLevels(dataCell *cell);
  unsigned char const *getStart(unsigned int ID = 0);

  dataCell *operator[](int ID);

  inline GDHeader *getHeader() const;
  inline scopeNode *getScope() const;
  inline GDType getStatus() const;
  inline GDMode getMode() const;
  inline GDFormat getFormat() const;
  inline bool isRecursive() const;
  inline void setRecursion(bool recursion);
  inline void setArgno(unsigned int argno);
  inline void setStatus(GDType status);
  inline void setMode(GDMode mode);
  inline void update(GDType status);
  inline void incCallNumber();
  inline bool addFlag(dataCell *source, dataCell *target, GDFlag &flag);
  inline GDFlag getFlag(dataCell *source, dataCell *target);
  inline GDFlag clearFlag(dataCell *source, dataCell *target);
  inline int *getBase() const;
  inline int getBaseLevel(unsigned int argno);
  inline void setBaseLevel(unsigned int argno, int level);
  inline GDSet *getHashtable(bool mode = true) const;
  inline unsigned int *getHashArr() const;

  inline static bool isDFAloop();
  inline static void setDFAloop(bool flag);

  friend ostream &operator<<(ostream &outStream, GDGraph *graph);

  define_relational(GDGraph,Relational);

protected:

  unsigned int processRelation(dataCell *source, dataCell *target,
			       dataCell *srcField, dataCell *trgField,
			       GDFlag flag, int adjustment,
			       bool abstract, bool warn);
  unsigned int recursionStep(GDGraph *graph, bool override = false);
  bool markNewCell(dataCell *cell, bool flag = false);

private:

  GDHeader *header_;
  unsigned char const *start_;
  scopeNode *scope_;
  GDFormat format_;
  GDMode mode_;
  bool recursion_;
  unsigned int *hashArr_;
  int *base_;

  objArr dataCells_;
  symbolSet indSet_;
  objArr newCells_;
  objArr oldCells_;
  objArr upCells_;
  objArr downCells_;
  DataSetPtrArr dataSet_;
  DataFlagSet flags_;

  static objArr currentSet_;
  static bool DFAloop_;
 
  friend class DataSet;

public:

  static unsigned int count_;
  static unsigned int newRelations_;
  static unsigned int rejectedRelations_;

};

class GDSet : public Relational {

public:

  GDSet(GDType status = 0)
    : set_(), array_(), status_(status), projGraph_(NULL),
      currGraph_(NULL), currFile_(NULL) {}

  GDSet(projNode *project, bool mode = true);
  virtual ~GDSet();

  void closeGraphs();
  void generateFlow(astRoot *rt, bool abstract = true, bool mode = true);
  Obj *generateDataFlow(symbolPtr &expr, objSet &result_set,
			bool abstract);
  void add(GDGraph *graph, symbolPtr &symbol);
  GDGraph *find(symbolPtr &symbol, bool mode = false);
  void cleanTable(symbolPtr &symbol);
  void cleanDataCells();
  void saveData();

  inline void removeCell(dataCell *cell);
  inline GDType getStatus() const;
  inline void setStatus(GDType status);
  inline void update(GDType status);

  static void loadStandardFunctions();
  static GDGraph *search(symbolPtr &symbol, bool mode = false);
  static Obj *search(symbolPtr &symbol, objSet &defnSet, bool mode = false);
  static bool createStub(symbolPtr &symbol);
  static void saveDFA(GDSet *hashtable, GDType status, bool mode);
  static void progressMessage();

  inline GDGraph *getProjGraph() const;
  inline static void setDFApass(bool flag);
  inline static bool isDFApass();

  friend ofstream &operator<<(ofstream &outStream, GDSet *hashtable);

  define_relational(GDSet,Relational);

private:

  symbolSet set_;
  genArr(GDPointer) array_;
  GDType status_;
  GDGraph *projGraph_;
  GDGraph *currGraph_;
  GDGraph *currFile_;
  symbolArr fileSet_;
  objArr dataCells_;

  static objArr currentGraphs_;
  static bool errorMessage_;
  static bool DFA_;

public:

  static objArr tableSet_;
  static GDSet *Hashtable_;
  static classInfo *classInfo_;
  static unsigned int count_;
  static unsigned int total_;

};

class DataSet {

public:

  DataSet(bool flag)
    :  indSet_(is_model_build()), parent_(NULL),
        markNew_(false), markOld_(true), flag_(flag) {}

  virtual ~DataSet();

  unsigned int mergeRelations(GDGraph *graph, bool abstract, int mode = 0);
  bool insert(dataCell *&cell, GDGraph *graph, bool clean);
  DataSet *find(symbolPtr &symbol, bool mode);
  void getCallCells(const symbolPtr &symbol, objArr &results) const;
  void reshuffle(bool flag);

  inline bool doMerge() const;
  inline void markNew(bool flag);
  inline void markOld(bool flag);
  inline bool getFlag() const;
  inline DataSet *getParent() const;
  inline void setParent(DataSet *data);
  inline objArr getCells() const;
  inline objArr getAllCells() const;
  inline objArr getDefnCells() const;
  inline dataCell *operator[](int ID);
  inline int size() const;

private:

  symbolSet indSet_;
  symbolSet callSet_;
  DataSetPtrArr dataSet_;
  objArrPtrArr callDataSet_;
  objArr dataCells_;
  objArr defnCells_;
  objArr allCells_;
  DataSet *parent_;
  bool markNew_;
  bool markOld_;
  bool flag_;

};

generate_descriptor(GDGraph,Relational);
generate_descriptor(GDSet,Relational);
defrel_one_to_many(GDGraph,graph,GDGraph,call);
defrel_many_to_many(GDGraph,defn,GDGraph,call);
defrel_one_to_one(projNode,project,GDSet,table);
defrel_one_to_many(GDSet,table,GDGraph,graph);

inline
unsigned int
GDHeader::getSymbolID() const
{
  return
    (unsigned int) symbolID_;
}

inline
unsigned int
GDHeader::getNodeID() const
{
  return
    (unsigned int) nodeID_;
}

inline
unsigned int
GDHeader::getFileID() const
{
  return
    (unsigned int) fileID_;
}

inline
unsigned int
GDHeader::getCellNumber() const
{
  return
    (unsigned int) cellNum_;
}

inline
unsigned int
GDHeader::getCallNumber() const
{
  return
    (unsigned int) callNum_;
}

inline
unsigned int
GDHeader::getSize() const
{
  return
    (unsigned int) size_;
}

inline
unsigned int
GDHeader::getTotalSize() const
{
  return
    (unsigned int) totalSize_;
}

inline
unsigned int
GDHeader::getArgno() const
{
  return
    (unsigned int) argno_;
}

inline
void
GDHeader::setSymbolID(unsigned int offset)
{
  symbolID_ = offset;
}

inline
void
GDHeader::setCellNumber(unsigned int number)
{
  cellNum_ = number;
}

inline
void
GDHeader::setCallNumber(unsigned int number)
{
  callNum_ = number;
}

inline
void
GDHeader::setSize(unsigned int number)
{
  size_ = number;
}

inline
void
GDHeader::setTotalSize(unsigned int number)
{
  totalSize_ = number;
}

inline
void
GDHeader::setArgno(unsigned int argno)
{
  argno_ = (unsigned char) argno;
}

inline
GDType
GDHeader::getStatus() const
{
  return
    status_;
}

inline
void
GDHeader::setStatus(GDType status)
{
  status_ = status;
}

inline
void
GDHeader::update(GDType status)
{
  status_ |= status;
}

inline
GDHeader *
GDGraph::getHeader() const
{
  return header_;
}

inline
scopeNode *
GDGraph::getScope() const
{
  return scope_;
}

inline
GDType
GDGraph::getStatus() const
{
  return
    header_->getStatus();
}

inline
void
GDGraph::setArgno(unsigned int argno)
{
  header_->setArgno(argno);
}

inline
GDMode
GDGraph::getMode() const
{
  return mode_;
}

inline
void
GDGraph::setMode(GDMode mode)
{
  mode_ = mode;
}

inline
GDFormat
GDGraph::getFormat() const
{
  return format_;
}

inline
void
GDGraph::setStatus(GDType status)
{
  header_->setStatus(status);
}

inline
void
GDGraph::update(GDType status)
{
  header_->update(status);
}

inline
bool
GDGraph::isRecursive() const
{
  return recursion_;
}

inline
void
GDGraph::setRecursion(bool recursion)
{
  recursion_ = recursion;
}

inline
void
GDGraph::incCallNumber()
{
  unsigned int callNum = header_->getCallNumber();
  header_->setCallNumber(++callNum);
}

inline
bool
GDGraph::addFlag(dataCell *source,
		 dataCell *target,
		 GDFlag &flag)
{
  const symbolPtr &srcSymbol = source->get_symbol();
  const symbolPtr &trgSymbol = target->get_symbol();

  if (srcSymbol.is_ast() || trgSymbol.is_ast())
    flag &= ~GDR_GLOBAL;
  else
    flag |= GDR_GLOBAL;

  return
    flags_.addFlag(source, target, flag);
}

inline
GDFlag
GDGraph::getFlag(dataCell *source,
		 dataCell *target)
{
  GDFlag flag = flags_.getFlag(source, target);
  return flag;
}

inline
GDFlag
GDGraph::clearFlag(dataCell *source,
		   dataCell *target)
{
  GDFlag flag = flags_.getFlag(source, target);
  flags_.clearFlag(source, target);
  return flag;
}

inline
int *
GDGraph::getBase() const
{
  return base_;
}

inline
int
GDGraph::getBaseLevel(unsigned int argno)
{
  int level = DFA_MAX_LEVEL;

  if (base_)
    level = *(base_ + argno);

  return level;
}

inline
void
GDGraph::setBaseLevel(unsigned int argno,
		      int level)
{
  if (base_)
    *(base_ + argno) = level;
}

inline
unsigned int *
GDGraph::getHashArr() const
{
  return hashArr_;
}

inline
GDSet *
GDGraph::getHashtable(bool mode) const
{
  GDGraph *graph = (GDGraph *) this;

  if (mode)
    graph = getParent();

  GDSet *hashtable = graph_get_table(graph);

  if (!hashtable)
    hashtable = GDSet::Hashtable_;

  return hashtable;
}

inline
bool
GDGraph::isDFAloop()
{
  return GDGraph::DFAloop_;
}

inline
void
GDGraph::setDFAloop(bool flag)
{
  GDGraph::DFAloop_ = flag;
}

inline
GDGraph *
GDSet::getProjGraph() const
{
  return projGraph_;
}

inline
void
GDSet::removeCell(dataCell *cell)
{
  dataCells_.remove(cell);
}

inline
GDType
GDSet::getStatus() const
{
  return status_;
}

inline
void
GDSet::setStatus(GDType status)
{
  status_ = status;
}

inline
void
GDSet::update(GDType status)
{
  status_ |= status;
}

inline
bool
GDSet::isDFApass()
{
  return GDSet::DFA_;
}

inline
void
GDSet::setDFApass(bool flag)
{
  GDSet::DFA_ = flag;
}

inline
dataCell *
DataSet::operator[](int ID)
{
  return
    (dataCell *) allCells_[ID];
}

inline
int
DataSet::size() const
{
  return
    allCells_.size();
}

inline
DataSet *
DataSet::getParent() const
{
  return parent_;
}

inline
void
DataSet::setParent(DataSet *data)
{
  parent_ = data;
}

inline
objArr
DataSet::getCells() const
{
  return
    dataCells_;
}

inline
objArr
DataSet::getAllCells() const
{
  return
    allCells_;
}

inline
objArr
DataSet::getDefnCells() const
{
  return
    defnCells_;
}

inline
void
DataSet::markNew(bool flag)
{
  markNew_ = flag;
}

inline
void
DataSet::markOld(bool flag)
{
  markOld_ = flag;
}

inline
bool
DataSet::getFlag() const
{
  return flag_;
}

inline
bool
DataSet::doMerge() const
{
  return
    (flag_ && markOld_);
}

inline
unsigned int
GDInteger::DB_SIZE()
{
  return
    (4 * sizeof(unsigned char));
}

inline
unsigned int
GDHeader::DB_SIZE()
{
  return
    (7 * GDInteger::DB_SIZE() + 2 * sizeof(unsigned char));
}

inline
unsigned int
dataCell::DB_SIZE()
{
  return
    (2 * GDInteger::DB_SIZE() + 4 * sizeof(unsigned char));
}

inline
unsigned int
nodeDataCell::DB_SIZE()
{
  return
    (3 * GDInteger::DB_SIZE() + 4 * sizeof(unsigned char));
}

inline
unsigned int
callDataCell::DB_SIZE()
{
  return
    (5 * GDInteger::DB_SIZE() + 4 * sizeof(unsigned char));
}

#endif
