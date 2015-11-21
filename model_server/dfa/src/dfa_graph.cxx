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
#include <cstdlib>
namespace std {};
using namespace std;
#include <ctime>
#include <sstream>

#include <cLibraryFunctions.h>
#include <msg.h>
#include <fileEntry.h>
#include <dfa_graph.h>
#include <_Xref.h>
#include <customize.h>
#include <xxinterface.h>
#include <driver_mode.h>

#define DFA_RELATION_INCREMENT 10000
#define DFA_VIRTUAL_INCREMENT  10

static bool dfa_loop = false;
char const *arena = OSapi_getenv("PSET_ARENA");
char const *testName = OSapi_getenv("PSET_TESTNAME");
char const *dfa_max_global = OSapi_getenv("DFA_MAX_GLOBAL");
unsigned int DFA_MAX_GLOBAL = dfa_max_global ? atoi(dfa_max_global) : 0;
bool hashArrInit = true;

bool GDSet::DFA_ = false;
GDSet *GDSet::Hashtable_ = NULL;
objArr GDSet::tableSet_;
objArr GDGraph::currentSet_;
bool GDGraph::DFAloop_ = false;
unsigned int GDGraph::count_ = 0;
unsigned int GDGraph::newRelations_ = 0;
unsigned int GDGraph::rejectedRelations_ = 0;
unsigned int GDSet::count_ = 0;
unsigned int GDSet::total_ = 0;
unsigned int rejectedRelations = 0;
bool GDSet::errorMessage_ = true;
extern unsigned int DFA_NESTED_FIELDS_LIMIT;

static objSet currentSet;
static GDSet *currentTable = NULL;
static unsigned int cc_rel = 0;
static unsigned int cd_rel = 0;
static unsigned int dc_rel = 0;
static unsigned int fc_rel = 0;
static unsigned int tc_rel = 0;
static unsigned int dd_rel = 0;
static unsigned int fd_rel = 0;
static unsigned int td_rel = 0;
static unsigned int ot_rel = 0;
static unsigned int m0_rel = 0;
static unsigned int m1_rel = 0;
static unsigned int m2_rel = 0;
#ifdef XXX_DFAglobal
static bool DFAglobal = false;
#endif
static int DFAthreshold = 0;
static symbolArr stubArr;

init_relational(GDGraph,Relational);
init_relational(GDSet,Relational);
init_rel_or_ptr(projNode,project,0,GDSet,table,0);
init_rel_or_ptr(GDGraph,graph,0,GDGraph,call,relationMode::D);
init_rel_or_ptr(GDGraph,defn,0,GDGraph,call,0);
init_rel_or_ptr(GDSet,table,0,GDGraph,graph,0);

double my_time();
int proc_size(int pid);
extern "C" void psarr(symbolArr *sa);
symbolPtr astNode_get_node(char const *lname, int offset);
bool isDFAinit();

extern "C"
void
dfa_debug_hook(char const *name)
{
  msg("dfa_debug_hook on $1", warning_sev) << name << eoarg << eom;
}
#ifdef XXX_DFAglobal
bool
isDFAglobal()
{
  return DFAglobal;
}
#endif
static void
showDFAinfo(unsigned int &count,
	    unsigned int funcSize,
	    int mode)
{
  Initialize(showDFAinfo(unsigned int &, unsigned int, int));

  genString message, procInfo;
  int procSize = proc_size(0);

  if (mode > 0)
    message.printf("Iteration %d", ++count);
  else if (mode == 0)
    message.printf("Local Phase");
  else
    message.printf("Finished");

  if (funcSize != 0) 
    procInfo.printf("%d Mbytes, %d new relations, %d rejected relations, %d denied relations, %d new cells, %d rejected cells, %d new aliases, %d rejected aliases, loop size: %d",
		    procSize, GDGraph::newRelations_, rejectedRelations,
		    GDGraph::rejectedRelations_, dataCell::newCount_,
		    dataCell::deleteCount_, dataCell::newAliasCount_,
		    dataCell::oldAliasCount_, funcSize);
  else
    procInfo.printf("%d Mbytes, %d new relations, %d rejected relations, %d denied relations, %d new cells, %d rejected cells, %d new aliases, %d rejected aliases",
		    procSize, GDGraph::newRelations_, rejectedRelations,
		    GDGraph::rejectedRelations_, dataCell::newCount_,
		    dataCell::deleteCount_, dataCell::newAliasCount_,
		    dataCell::oldAliasCount_);

  GDGraph::newRelations_ = 0;
  GDGraph::rejectedRelations_ = 0;
  dataCell::newCount_ = 0;
  dataCell::deleteCount_ = 0;
  dataCell::newAliasCount_ = 0;
  dataCell::oldAliasCount_ = 0;
  dataCell::checkedCount_ = 0;
  rejectedRelations = 0;

  msg ("showDFAinfo: $1 : $2", normal_sev)
	<< message.str() << eoarg
	<< procInfo.str() << eoarg << eom;
}

void
uses_functions(const symbolPtr &symbol,
	       symbolArr &results)
{
  Initialize(uses_functions(const symbolPtr &, symbolArr &));

  symbolArr callsArr;
  symbol->get_link(is_using, callsArr);
  unsigned int size = callsArr.size();

  for (unsigned int k = 0; k < size; ++k) {
    symbolPtr &call = callsArr[k];

    if (call.get_kind() == DD_FUNC_DECL) {
      symbolArr defs;
      call.get_def_symbols(defs, 2);
      results.insert_last(defs);
    }
  }
}

static void
calls_functions(const symbolPtr &symbol,
		symbolArr &results)
{
  Initialize(calls_functions(const symbolPtr &, symbolArr &));
  GDGraph *graph = GDSet::search((symbolPtr &) symbol, true);

  if (!graph)
    return;

  symbolSet resultSet;
  graph->loadGraph(true);
  graph->getCallTree(resultSet);
  graph->offloadGraph(false);
  results = resultSet;
}
#ifdef XXX_cleanFields
static void
cleanFields(GDGraph *graph,
	    dataCell *cell)
{
  Initialize(cleanFields(GDGraph *, dataCell *));
  DataSet *bucket = graph->find(cell);

  if (!bucket)
    return;

  unsigned int size = bucket->size();

  for (unsigned int k = 0; k < size; ++k) {
    dataCell *next = (*bucket)[k];

    if (!next || !next->isValid() || next->isChecked())
      continue;

    if (next->get_depth() < cell->get_depth())
      continue;

    if (next->get_depth() == cell->get_depth() &&
	next->get_level() <= cell->get_level())
      continue;

    if (!next->cellCompare(cell, true, true, true))
      continue;

    next->add_type(dataCell_Checked);
  }
}
#endif
void
GDGraph::getCallTree(symbolSet &resultSet)
{
  Initialize(GDGraph::getCallTree(symbolSet &));

  if (!currentTable)
    currentTable = getHashtable(true);

  Obj *callSet = graph_get_calls(this);
  Obj *call;

  if (!callSet)
    return;

  ForEach(call, *callSet) {
    GDGraph *callGraph = (GDGraph *) call;
    symbolPtr callSymbol;
    callGraph->getSymbol(callSymbol);
    DataSet *bucket = callGraph->find(callSymbol);

    if (!bucket)
      continue;

    Obj *defnObj = call_get_defns(callGraph);
    objSet defnSet;
    Obj *defn;

    if (!defnObj)
      defnObj = GDSet::search(callSymbol, defnSet, true);

    if (!defnObj)
      continue;

    ForEach(defn, *defnObj) {
      GDGraph *defnGraph = (GDGraph *) defn;
      defn_put_call(defnGraph, callGraph);
      GDSet *defnTable = defnGraph->getHashtable(true);

      if (currentTable != defnTable && !currentSet.includes(defnGraph)) {
	currentSet.insert(defnGraph);

	if (defnGraph->getStatus() != GDT_GLOBAL ||
	    (defnTable->getStatus() & GDT_UPDATED))
	  ++GDSet::total_;
      }

      if (defnGraph->getStatus() != GDT_GLOBAL ||
	  (defnTable->getStatus() & GDT_UPDATED)) {
	symbolPtr defnSymbol;
	defnGraph->getSymbol(defnSymbol);
	resultSet.insert(defnSymbol);
      }
    }

    callGraph->getCallTree(resultSet);
  }
}

void
GDGraph::closeGraph(bool abstract,
		    bool override)
{
  Initialize(GDGraph::closeGraph(bool, bool));
  GDSet *hashtable = getHashtable(true);

  if (!hashtable)
    return;

  GDType status = hashtable->getStatus();

  if (!is_model_build() && (!status || (status & GDT_LOCAL)))
    return;

  if (status == GDT_GLOBAL && getStatus() == GDT_GLOBAL && !override)
    return;

  if (override)
    update(GDT_LOCAL);

  if (!abstract) {
    closeRelations(abstract, override);
    return;
  }

  GDGraph::currentSet_.removeAll();
  GDGraph::currentSet_.insert_last(this);
  unsigned int check = 1;
  unsigned int count = 0;
  unsigned int size = 1;
  dfa_loop = false;
  GDGraph::setDFAloop(false);

  symbolPtr symbol;
  getSymbol(symbol);
 
  if (GDSet::isDFApass() && !call_get_graph(this)) {
    msg("DFA: Started working on $1", normal_sev) << symbol.get_name() << eoarg << eom;
  }

  while (check != 0) {
    check = 0;
    unsigned int funcSize = 0;

    for (int k = GDGraph::currentSet_.size(); k > 0; --k) {
      GDGraph *graph = (GDGraph *) GDGraph::currentSet_[k - 1];

      if (!graph)
	continue;

      if (graph->getStatus() != GDT_GLOBAL) {
	if (override && graph->newCells_.size() == 0 &&
	    graph->oldCells_.size() == 0)
	  graph->reshuffle();

	check += graph->closeRelations(abstract, override);

	if (!call_get_graph(graph))
	  ++funcSize;
      }
    }

    GDGraph::setDFAloop(dfa_loop);

    if (is_model_build() && GDSet::isDFApass() && GDGraph::isDFAloop()) {
      if (arena)
	showDFAinfo(count, funcSize, 1);
    }

    for (int m = GDGraph::currentSet_.size(); m > 0; --m) {
      GDGraph *graph = (GDGraph *) GDGraph::currentSet_[m - 1];

      if (!graph)
	continue;

      if (graph->getStatus() != GDT_GLOBAL)
	graph->renewDownCells();
    }
  }

  GDGraph::setDFAloop(false);
  size = GDGraph::currentSet_.size();

  for (unsigned int n = size; n > 0; --n) {
    GDGraph *graph = (GDGraph *) GDGraph::currentSet_[n - 1];

    if (graph && graph->getStatus() != GDT_GLOBAL) {
      graph->setStatus(GDT_GLOBAL);
      graph->setRecursion(false);
      GDSet *current = graph->getHashtable(true);
      GDGraph *projGraph = current->getProjGraph();

      if (current == GDSet::Hashtable_)
	continue;

      graph->reshuffle();

      if (call_get_graph(graph))
	continue;

      if (!projGraph) {
	projNode *project = table_get_project(current);
	symbolPtr projSymbol = project;
	GDHeader *projHeader = new GDHeader(0, 0, 0);
	projGraph = new GDGraph(projHeader, scope_);
	current->add(projGraph, projSymbol);
      } else
	projGraph->loadGraph(true);

      if (projGraph != graph)
	projGraph->combineRelations(graph, abstract);

      graph->offloadGraph(false);

      if (GDSet::isDFApass()) {
	symbolPtr graphSymbol;
	graph->getSymbol(graphSymbol);

	msg("DFA: Finished working on $1", normal_sev) << graphSymbol.get_name() << eoarg << eom;

	GDSet::progressMessage();
      }
    }
  }

  dfa_loop = false;
  GDGraph::currentSet_.removeAll();
}

bool
GDGraph::dataFlow(symbolPtr &symbol,
		  symbolArr &results,
		  symbolPtr &keySymbol,
		  dataCell *queryCell,
		  DataQuery query)
{
  Initialize(GDGraph::dataFlow(symbolPtr &, symbolArr &, symbolPtr &, dataCell *, DataQuery));

  bool check = false;
  bool mode = true;
  bool doCalls = false;
  symbolPtr module;
  GDGraph *queryGraph = NULL;
  symbolPtr graphSymbol;
  getSymbol(graphSymbol);
  GDGraph *func = getParent();
  symbolPtr funcSymbol;
  func->getSymbol(funcSymbol);
  GDSet *hashtable = getHashtable(true);
  DataSet *bucket = NULL;

  if (symbol.is_ast() || symbol.is_instance()) {
    if (symbol.is_instance())
      symbol = symbol.get_astNode();

    if (symbol.isnull())
      return false;

    keySymbol = dfa_decl_of_ast(symbol);
  } else if (symbol.is_xrefSymbol())
    keySymbol = symbol;
  else if (symbol.is_dataCell()) {
    if (!queryCell)
      return false;

    bool recompute = keySymbol.isnull();

    if (recompute)
      keySymbol = queryCell->get_symbol();

    if (!queryCell->isAbstract()) {
      if (queryGraph != this)
	doCalls = true;
      else if (recompute)
	keySymbol = dfa_decl_of_ast(keySymbol);

      mode = false;
    } else if (!(queryCell->get_type() & dataCell_Regular)) {
      if (keySymbol.is_ast())
	keySymbol = queryCell->get_xrefSymbol();

      if (call_get_graph(queryGraph) && func != this &&
	  queryGraph != this) {
	if (!dataCell::symbolCompare(funcSymbol, keySymbol))
	  doCalls = true;
      }
    }
  }

  if (!mode) {
    loadGraph(mode);
    closeGraph(mode);
  }

  if (!doCalls) {
    if (queryCell)
      bucket = find(queryCell);
    else
      bucket = find(keySymbol);

    if (bucket && symbol.is_ast() && keySymbol.is_xrefSymbol() &&
	keySymbol.get_kind() == DD_FUNC_DECL)
      bucket = bucket->find(symbol, false);

    if (bucket) {
      objArr dataArr;

      if (hashtable->getProjGraph() == this && queryCell) {
	if (queryCell->isNodeDataCell())
	  bucket->getCallCells(symbol, dataArr);
	else if (!(queryCell->get_type() & dataCell_Regular))
	  dataArr = bucket->getDefnCells();
	else
	  dataArr = bucket->getCells();
      } else if (queryCell)
	dataArr = bucket->getCells();
      else
	dataArr = bucket->getAllCells();

      unsigned int size = dataArr.size();

      for (unsigned int k = 0; k < size; ++k) {
	dataCell *cell = (dataCell *) dataArr[k];

	if (!cell || !cell->isValid() || cell->isChecked() ||
	    cell->isAbstract() != mode)
	  continue;

	if (symbol.is_dataCell() &&
	    !cell->cellCompare(queryCell, true, mode))
	  continue;

	objArr dataArr;
	Obj *dataSet = NULL;
	dataCell *alias = (dataCell *) cell_get_alias(cell);

	if (alias) {
	  dataArr.insert(alias);
	  cell = alias;
	}

	if (query == DFA_INFLOW)
	  dataSet = trg_get_srcs(cell);
	else if (query == DFA_OUTFLOW)
	  dataSet = src_get_trgs(cell);

	if (dataSet)
	  dataArr.insert(dataSet);

	for (unsigned int m = 0; m < dataArr.size(); ++m) {
	  dataCell *nextCell = (dataCell *) dataArr[m];
	  check = true;

	  if (nextCell != cell) {
	    Obj *aliasSet = alias_get_cells(nextCell);

	    if (aliasSet)
	      dataArr.insert(aliasSet);
	  }

	  if (nextCell->isAbstract()) {
	    symbolPtr newSymbol(nextCell);
	    results.insert_last(newSymbol);
	  } else {
	    objSet nextSet;
	    findRelatedCells(nextCell, nextSet, true, true);
	    Obj *next;

	    ForEach(next, nextSet) {
	      dataCell *newCell = (dataCell *) next;
	      symbolPtr newSymbol(newCell);
	      results.insert_last(newSymbol);
	    }
	  }
	}
      }
    }
  }

  results.unsort();
  return check;
}

void
GDGraph::loadGraph(bool abstract,
		   bool mode,
		   bool recurse)
{
  Initialize(GDGraph::loadGraph(bool, bool, bool));

  if ((!start_ && abstract) || GDGraph::isDFAloop())
    return;

  if (recurse && call_get_graph(this)) {
    GDGraph *parent = getParent();
    parent->loadGraph(abstract, mode, false);
    return;
  }

  if (mode_ == GDM_NONE || mode_ == GDM_OFFLOADED) {
    if (!GDSet::isDFApass() && !call_get_graph(this)) {
      GDSet *hashtable = getHashtable(true);

      if (hashtable != GDSet::Hashtable_) {
	if (hashtable->getProjGraph() == this) {
	  projNode *project = table_get_project(hashtable);
	  msg("Loading the global DFA graph for the project $1.", normal_sev) << project->get_name() << eom;
	}
      }
    }

    mode_ = GDM_ABSTRACT;
    loadRelations();

    if (mode)
      loadCalls(true);
  }

  if (getStatus() != GDT_GLOBAL && start_) {
    GDGraph *parent = getParent();

    if (!call_get_graph(this)) {
      free((char *) start_);
      start_ = NULL;
    } else if (parent->start_ == NULL)
      start_ = NULL;
  }

  if (getStatus() != GDT_GLOBAL)
    setFormat(GDF_ACTIVE);

  if (is_model_build())
    reshuffle();

  if (!abstract && mode_ != GDM_CONCRETE && mode_ != GDM_COMPLETE) {
    mode_ = GDM_CONCRETE;

    symbolPtr graphNode;
    getAstSymbol(graphNode, false);

    if (graphNode.isnull())
      return;

    GDSet *hashtable = getHashtable(true);
    int code = symbolPtr_get_code(graphNode);
    objSet dummy;

    if (code == MC_FUNCTION_DECL || code == MC_CALL_EXPR ||
	code == NEW_EXPR) {
      for (symbolPtr funcNode = graphNode; 
	   funcNode.isnotnull();
	   tree_get_parent(funcNode, funcNode)) {
	int code = symbolPtr_get_code(funcNode);

	if (code == DFA_FUNCTION_DEF || code == DFA_ROOT) {
	  hashtable->generateDataFlow(funcNode, dummy, false);
	  hashtable->cleanDataCells();
	  break;
	}
      }
    } else {
      symbolPtr fileSymbol;
      getFileSymbol(fileSymbol);
      symbolPtr fileNode = fileSymbol.get_astNode();
      hashtable->generateDataFlow(fileNode, dummy, false);
      hashtable->cleanDataCells();
    }
  }
}

void
GDGraph::loadRelations()
{
  Initialize(GDGraph::loadRelations());

  if (!start_)
    return;

  GDSet *hashtable = getHashtable(true);
  unsigned int graphSize = header_->getSize();
  unsigned int cellNumber = header_->getCellNumber();
  unsigned char const *start = start_ + GDHeader::DB_SIZE();
  unsigned char const *end = start_ + graphSize;
  genArr(uint) holes;
  genArr(int) cells;
  unsigned int offset = 0;
  unsigned int index = 0;

  if (!hashArr_ && hashArrInit && hashtable->getProjGraph() != this) {
    hashArr_ = (unsigned int *) malloc(cellNumber * sizeof(unsigned int));
    offset = 1;
  }

  for (unsigned int k = 0; k < cellNumber; ++k) {
    dataCell *cell = dataCell::newCell(start, this);
    bool check = addDataCell(cell, true);

    if (!check) {
      holes.append(&k);
      int cellID = -1;

      if (cell)
	cellID = cell->get_id() - 1;

      cells.append(&cellID);
    } else {
      --dataCell::newCount_;

      if (hashArr_ && offset > 0) {
	*(hashArr_ + index) = offset - 1;
	offset += cell->getSize();
	++index;
      }
    }
  }

  while (start < end) {
    GDInteger src(start);
    start += GDInteger::DB_SIZE();
    GDInteger trg(start);
    start += GDInteger::DB_SIZE();
    unsigned int srcID = ((unsigned int) src) - 1;
    unsigned int trgID = ((unsigned int) trg) - 1;
    unsigned int hsize = holes.size();

    for (unsigned int m = hsize; m > 0; --m) {
      if (srcID > *holes[m - 1])
	--srcID;
      else if (srcID == *holes[m - 1]) {
	srcID = *cells[m - 1];
	break;
      }
    }

    for (unsigned int n = hsize; n > 0; --n) {
      if (trgID > *holes[n - 1])
	--trgID;
      else if (trgID == *holes[n - 1]) {
	trgID = *cells[n - 1];
	break;
      }
    }

    unsigned char flag_1 = *start++;
    unsigned char flag_2 = *start++;
    GDFlag flag = (GDFlag) ((flag_1 << 8) | flag_2);

    dataCell *source = (dataCell *) dataCells_[srcID];
    dataCell *target = (dataCell *) dataCells_[trgID];
    src_put_trg(source, target);

    if (flag & GDR_ALIAS) {
      cell_put_alias(source, target);
      flag &= ~GDR_ALIAS;
    }

    addFlag(source, target, flag);
  }
}

void
GDGraph::loadCalls(bool mode)
{
  Initialize(GDGraph::loadCalls(bool));

  if (!start_)
    return;

  Obj *callSet = graph_get_calls(this);
  Obj *call;

  if (!callSet) {
    unsigned int graphSize = header_->getSize();
    unsigned int callNum = header_->getCallNumber();
    unsigned char const *start = start_ + graphSize;

    for (int k = 0; k < callNum; ++k) {
      GDHeader *header = new GDHeader(start);
      GDGraph *graph = new GDGraph(header, scope_, start, format_);
      graph_put_call(this, graph);

      if (mode)
	graph->loadGraph(true, true, false);
      else
	graph->loadCalls(mode);

      unsigned int totalSize = header->getTotalSize();
      start += totalSize;
    }
  } else if (mode) {
    ForEach(call, *callSet) {
      GDGraph *graph = (GDGraph *) call;

      if (graph)
	graph->loadGraph(true, true, false);
    }
  }
}

void
GDGraph::setFormat(GDFormat format,
		   bool mode)
{
  Initialize(GDGraph::setFormat(GDFormat, bool));
  format_ = format;

  if (mode) {
    Obj *callSet = graph_get_calls(this);
    Obj *call;

    ForEach(call, *callSet) {
      GDGraph *graph = (GDGraph *) call;
      graph->setFormat(format, mode);
    }
  }
}

unsigned int
GDGraph::closeRelations(bool abstract,
			bool override)
{
//  Initialize(GDGraph::closeRelations(bool, bool));

  if (!is_model_build() && !override)
    return 0;

  unsigned int check = 0;
  GDType status = getStatus();
  GDSet *hashtable = getHashtable(true);
  GDGraph *func = (GDGraph *) call_get_graph(this);
  symbolPtr symbol;
  getSymbol(symbol);

  if (override)
    update(GDT_LOCAL);

  if (testName && strstr(symbol.get_name(), testName))
    dfa_debug_hook(testName);

  if (!abstract) {
    if (mode_ != GDM_COMPLETE) {
      loadGraph(abstract);
      Obj *callSet = graph_get_calls(this);
      Obj *call;

      ForEach(call, *callSet) {
	GDGraph *graph = (GDGraph *) call;
	graph->closeRelations(abstract, override);
	combineRelations(graph, abstract);
      }

      mergeRelations(abstract);
    }

    if (mode_ == GDM_CONCRETE)
      mode_ = GDM_COMPLETE;
  } else {
    if ((getStatus() == GDT_GLOBAL)) {
      if (hashtable && !(hashtable->getStatus() & GDT_UPDATED))
	return 0;
    }

    loadGraph(true);

    if (func) {
      Obj *defnObj = call_get_defns(this);
      objSet defnSet;

      if (!defnObj)
	defnObj = GDSet::search(symbol, defnSet, true);

      if (defnObj) {
	Obj *defn;

	ForEach(defn, *defnObj) {
	  GDGraph *graph = (GDGraph *) defn;
	  defn_put_call(graph, this);
	  check += recursionStep(graph, override);
	}
      }
    }

    Obj *callSet = graph_get_calls(this);
    Obj *call;

    ForEach(call, *callSet) {
      GDGraph *graph = (GDGraph *) call;
      check += recursionStep(graph, override);
    }

    if (getStatus() != GDT_GLOBAL)
      check += mergeRelations(true);

    if (!isRecursive()) {
      GDGraph *projGraph = hashtable->getProjGraph();
      setStatus(GDT_GLOBAL);

      if (projGraph != this)
	reshuffle();

      if (!func) {
	if (!projGraph) {
	  projNode *project = table_get_project(hashtable);
	  symbolPtr projSymbol = project;
	  GDHeader *projHeader = new GDHeader(0, 0, 0);
	  projGraph = new GDGraph(projHeader, scope_);
	  hashtable->add(projGraph, projSymbol);
	} else
	  projGraph->loadGraph(true);

	if (hashtable != GDSet::Hashtable_) {
	  if (projGraph != this)
	    projGraph->combineRelations(this, abstract);

	  offloadGraph(false);
	}

	if (GDSet::isDFApass()) {
	  msg("DFA: Finished working on $1", normal_sev) << symbol.get_name() << eoarg << eom;

	  if (hashtable != GDSet::Hashtable_)
	    GDSet::progressMessage();
	}
      }
    } else
      update(GDT_LOCAL);
  }

  return check;
}

unsigned int
GDGraph::recursionStep(GDGraph *graph,
		       bool override)
{
//  Initialize(GDGraph::recursionStep(GDGraph *, bool));

  if (!graph)
    return 0;

  unsigned int check = 0;
  GDType status = graph->getStatus();

  if (status != GDT_GLOBAL || override) {
    symbolPtr symbol;
    graph->getSymbol(symbol);

    if (call_get_graph(graph)) {
      DataSet *bucket = graph->find(symbol);

      if (!bucket)
	return 0;
    }

    if (!GDGraph::currentSet_.includes(graph)) {
      GDGraph::currentSet_.insert_last(graph);
      ++check;

      if (override)
	update(GDT_LOCAL);

      if (is_model_build() && !call_get_graph(graph)) {
	msg("DFA: Started working on $1", normal_sev) << symbol.get_name() << eoarg << eom;
      }
    } else {
      graph->setRecursion(true);
      setRecursion(true);

      if (!GDGraph::isDFAloop()) {
	unsigned int size = GDGraph::currentSet_.size();

	for (unsigned int k = 0; k < size; ++k) {
	  GDGraph *temp = (GDGraph *) GDGraph::currentSet_[k];

	  if (graph == temp) {
	    dfa_loop = true;
	    break;
	  }
	}
      }
    }
  }

  if (!graph->isRecursive())
    graph->closeRelations(true, override);

  if (graph->isRecursive())
    setRecursion(true);

  if (status != GDT_GLOBAL || getStatus() != GDT_GLOBAL) {
    update(GDT_LOCAL);
    check += combineRelations(graph, true);
  }

  return check;
}

void
GDGraph::setAlias(dataCell *source,
		  dataCell *target,
		  int mode)
{
  Initialize(GDGraph::setAlias(dataCell *, dataCell *, int));

  dataCell *realSource = (dataCell *) cell_get_alias(source);
  dataCell *realTarget = (dataCell *) cell_get_alias(target);

  if (!realSource)
    realSource = source;

  if (!realTarget)
    realTarget = target;

  if (realSource == realTarget)
    return;

  Obj *srcSet = alias_get_cells(realSource);
  Obj *trgSet = alias_get_cells(realTarget);
  objArr srcArr, trgArr;

  if (srcSet)
    srcArr.insert(srcSet);

  if (trgSet)
    trgArr.insert(trgSet);

  GDGraph *func = call_get_graph(this);
  unsigned int srcSize = srcArr.size();
  unsigned int trgSize = trgArr.size();
  unsigned int srcDepth = realSource->get_depth();
  unsigned int trgDepth = realTarget->get_depth();
  dataCell_type srcType = realSource->get_type();
  dataCell_type trgType = realTarget->get_type();
  bool srcPass = (func || !realSource->isNodeDataCell());
  bool trgPass = (func || !realTarget->isNodeDataCell());
  bool order = true;
  int realmode = mode;

  if (mode >= 0) {
    realmode = 1;

    if (srcPass && !trgPass)
      order = false;
    else if (!srcPass && trgPass)
      order = true;
    else {
      bool srcCall = (realSource->isNodeDataCell() &&
		      !(srcType & dataCell_Regular));
      bool trgCall = (realTarget->isNodeDataCell() &&
		      !(trgType & dataCell_Regular));

      if (srcCall && !trgCall)
	order = true;
      else if (!srcCall && trgCall)
	order = false;
      else if (srcSize < trgSize)
	order = true;
      else if (srcSize > trgSize)
	order = false;
      else if (srcDepth >= trgDepth)
	order = true;
      else
	order = false;
    }
  }

  if (order) {
    if (!trgSet && mode_ != GDM_OFFLOADED)
      ++dataCell::newAliasCount_;

    if (srcSet && mode_ != GDM_OFFLOADED)
      ++dataCell::oldAliasCount_;

    cell_put_alias(realSource, realTarget);
    unsigned int srcSize = srcArr.size();

    for (unsigned int k = 0; k < srcSize; ++k) {
      dataCell *srcCell = (dataCell *) srcArr[k];
      cell_put_alias(srcCell, realTarget);
    }

    moveRelations(realSource, realTarget, realmode);
  } else {
    if (!srcSet && mode_ != GDM_OFFLOADED)
      ++dataCell::newAliasCount_;

    if (trgSet && mode_ != GDM_OFFLOADED)
      ++dataCell::oldAliasCount_;

    cell_put_alias(realTarget, realSource);
    unsigned int trgSize = trgArr.size();

    for (unsigned int m = 0; m < trgSize; ++m) {
      dataCell *trgCell = (dataCell *) trgArr[m];
      cell_put_alias(trgCell, realSource);
    }

    moveRelations(realTarget, realSource, realmode);
  }
}

void
GDGraph::moveRelations(dataCell *cell,
		       dataCell *alias,
		       int mode)
{
  Initialize(GDGraph::moveRelations(dataCell *, dataCell *, int));

  if (!cell || !alias || cell == alias)
    return;

  Obj *sourceSet = trg_get_srcs(cell);
  Obj *targetSet = src_get_trgs(cell);
  objArr sourceArr, targetArr;

  if (sourceSet)
    sourceArr.insert(sourceSet);

  if (targetSet)
    targetArr.insert(targetSet);

  src_rem_trgs(cell);
  trg_rem_srcs(cell);
  unsigned int srcSize = sourceArr.size();
  unsigned int trgSize = targetArr.size();

  for (unsigned int k = 0; k < srcSize; ++k) {
    dataCell *source = (dataCell *) sourceArr[k];
    dataCell *newAlias = (dataCell *) cell_get_alias(alias);

    if (!newAlias)
      newAlias = alias;

    if (source == newAlias)
      continue;

    GDFlag srcFlag = clearFlag(source, cell);
    addRelation(source, newAlias, srcFlag | GDR_ALIAS_TARGETS,
		true, false, false, mode);
  }

  for (unsigned int m = 0; m < trgSize; ++m) {
    dataCell *target = (dataCell *) targetArr[m];
    dataCell *newAlias = (dataCell *) cell_get_alias(alias);

    if (!newAlias)
      newAlias = alias;

    if (target == newAlias)
      continue;

    GDFlag trgFlag = clearFlag(cell, target);
    addRelation(newAlias, target, trgFlag | GDR_ALIAS_SOURCES,
		true, false, false, mode);
  }

  dataCell *realCell = cell;
  dataCell *realAlias = (dataCell *) cell_get_alias(alias);

  if (!realAlias)
    realAlias = alias;

  if (alias != realAlias) {
    GDFlag firstFlag = clearFlag(cell, alias);
    addRelation(realCell, realAlias, firstFlag | GDR_ALIAS_TARGETS,
		true, false, false, mode);

    realCell = cell;
    GDFlag secondFlag = clearFlag(alias, cell);
    addRelation(realAlias, realCell, secondFlag | GDR_ALIAS_SOURCES,
		true, false, false, mode);
  } else {
    src_put_trg(cell, realAlias);
    src_put_trg(realAlias, cell);
  }
}

void
GDGraph::cleanRelations()
{
  Initialize(GDGraph::cleanRelations());

  if (!getStatus())
    return;

  unsigned int size = dataCells_.size();

  for (unsigned int k = 0; k < size; ++k) {
    dataCell *source = (dataCell *) dataCells_[k];

    if (!source || !source->isValid() || source->isChecked())
      continue;

    objArr dataSet;

    {
      Obj *targetSet = src_get_trgs(source);
      Obj *trg;

      ForEach(trg, *targetSet) {
	dataCell *target = (dataCell *) trg;
	GDFlag flag = clearFlag(source, target);

	if (flag & GDR_IRREDUCIBLE)
	  dataSet.insert(target);
      }
    }

    src_rem_trgs(source);
    unsigned int dataSize = dataSet.size();

    for (unsigned int m = 0; m < dataSize; ++m) {
      dataCell *cell = (dataCell *) dataSet[m];

      if (cell && source && cell->isValid() && !cell->isChecked())
	addRelation(source, cell, GDR_IRREDUCIBLE, true, false, false);
    }
  }

  setStatus(0);
}

void
GDGraph::getDataCells(symbolPtr &symbol,
		      symbolArr &results,
		      bool mode)
{
  Initialize(GDGraph::getDataCells(symbolPtr &, symbolArr &, bool));

  bool abstract = mode;
  symbolPtr keySymbol = symbol;
  dataCell *keyCell = NULL;
  symbolPtr node;
  int code = 0;

  if (symbol.is_ast() || symbol.is_instance()) {
    node = symbol.get_astNode();

    if (node.isnull())
      return;

    astXref *axr = node.get_astXref();
    symbolPtr module = axr->get_module();
    module = module.get_xrefSymbol();

    symbolPtr fileSymbol;
    getFileSymbol(fileSymbol);

    if (module != fileSymbol)
      return;

    code = symbolPtr_get_code(node);

    if (code == MC_VAR_DECL || code == MC_PARM_DECL) 
      keySymbol = node;
    else
      keySymbol = dfa_decl_of_ast(node);
  } else if (symbol.is_dataCell()) {
    keyCell = dataCell_of_symbol(symbol, true);

    if (!keyCell)
      return;
  }

  DataSet *bucket = NULL;
  loadGraph(true);

  if (!abstract) {
    loadGraph(abstract);
    closeGraph(abstract);
  }

  if (keyCell)
    bucket = find(keyCell);
  else
    bucket = find(keySymbol);

  if (bucket) {
    unsigned int size = bucket->size();

    for (unsigned int k = 0; k < size; ++k) {
      dataCell *cell = (dataCell *) (*bucket)[k];
      bool check = false;

      if (!cell || !cell->isValid() || cell->isChecked())
	continue;

      if (abstract && !cell->isAbstract())
	continue;

      symbolPtr cellSymbol = cell->get_symbol();

      if ((abstract || cell->isAbstract()) &&
	  (symbol.is_ast() || symbol.is_instance()) &&
	  cell->isNodeDataCell() && !(cell->get_type() & dataCell_Regular))
	cellSymbol = cell->get_xrefSymbol();

      if (symbol.is_xrefSymbol()) {
	if (!cellSymbol.is_xrefSymbol())
	  continue;

	if (abstract || cell->isAbstract())
	  check = dataCell::symbolCompare(symbol, cellSymbol);
	else {
	  const symbolPtr &cellNode = dfa_decl_of_ast(cellSymbol);
	  check = dataCell::symbolCompare(symbol, cellNode);
	}
      } else if (symbol.is_ast() || symbol.is_instance()) {
	if (abstract || cell->isAbstract())
	  check = !keySymbol.sym_compare(cellSymbol);
	else
	  check = !node.sym_compare(cellSymbol);
      } else if (symbol.is_dataCell()) {
	if (keyCell->isAbstract())
	  check = keyCell->cellCompare(cell, false, true);
	else
	  check = keyCell->cellCompare(cell, false, false);
      }

      if (check) {
	symbolPtr cellSymbol(cell);
	results.insert_last(cellSymbol);
      }
    }
  }

  Obj *callSet = graph_get_calls(this);
  Obj *call;

  ForEach(call, *callSet) {
    GDGraph *graph = (GDGraph *) call;

    if (graph)
      graph->getDataCells(symbol, results, abstract);
  }

  results.unsort();
}

void
GDGraph::getAllDataCells(symbolArr &results,
			 bool abstract)
{
  Initialize(GDGraph::getAllDataCells(symbolArr &, bool));

  loadGraph(true);
  unsigned int size = dataCells_.size();

  for (unsigned int k = 0; k < size; ++k) {
    dataCell *cell = (dataCell *) dataCells_[k];

    if (cell && cell->isValid() && !cell->isChecked() &&
	cell->isAbstract() == abstract) {
      symbolPtr symbol(cell);
      results.insert_last(symbol);
    }
  }

  Obj *callSet = graph_get_calls(this);
  Obj *call;

  ForEach(call, *callSet) {
    GDGraph *graph = (GDGraph *) call;

    if (graph)
      graph->getAllDataCells(results, abstract);
  }
}

void
GDGraph::printRelations(ostream &outStream)
{
  Initialize(GDGraph::printRelations(ostream &));

  outStream << "==================================================" << endl;
  symbolPtr symbol, node;
  getSymbol(symbol);
  getAstSymbol(node);
  int code = symbolPtr_get_code(node);

  if (code == MC_CALL_EXPR || code == NEW_EXPR)
    outStream << "Function call:       ";
  else if (code == MC_FUNCTION_DECL)
    outStream << "Function definition: ";
  else if (node.isnotnull())
    outStream << "Static scope:        ";
  else
    outStream << "                     ";

  if (symbol.isnotnull())
    outStream << symbol.get_name() << endl;

  outStream << "==================================================" << endl;
  unsigned int size = dataCells_.size();

  for (unsigned int k = 0; k < size; ++k) {
    dataCell *source = (dataCell *) dataCells_[k];

    if (!source || !source->isValid() || source->isChecked() ||
	!source->isAbstract())
      continue;

    Obj *targetSet = src_get_trgs(source);
    Obj *trg;

    ForEach(trg, *targetSet) {
      dataCell *target = (dataCell *) trg;

      if (!target || !target->isAbstract() || source == target)
	continue;

      genString arrow;
      GDFlag flag = getFlag(source, target);

      if (!source->isNodeDataCell() && !target->isNodeDataCell())
	flag |= GDR_GLOBAL;

      if (OSapi_getenv("DIS_FLOW_DEBUG"))
	arrow.printf(" =%c%c%c%c%c%c%c%c%c=> ",
		     (flag & GDR_IRREDUCIBLE) ? 'i' : '-',
		     (flag & GDR_GLOBAL) ? 'g' : '-',
		     (flag & GDR_MERGE) ? 'm' : '-',
		     (flag & GDR_MERGE_UP) ? 'u' : '-',
		     (flag & GDR_MERGE_STRAIGHT) ? 'p' : '-',
		     (flag & GDR_MERGE_SOURCES) ? 's' : '-',
		     (flag & GDR_MERGE_TARGETS) ? 't' : '-',
		     (flag & GDR_FIELD_SOURCE) ? 'l' : '-',
		     (flag & GDR_FIELD_TARGET) ? 'r' : '-');
      else
	arrow.printf(" => ");

      genString srcInfo, trgInfo;

      if (source->isNodeDataCell() &&
	  (source->get_type() & dataCell_Regular))
	srcInfo.printf(" [L]");
      else if (source->isNodeDataCell())
	srcInfo.printf(" [C]");
      else if (source->get_type() & dataCell_Regular)
	srcInfo.printf(" [G]");
      else
	srcInfo.printf(" [D]");

      if (target->isNodeDataCell() &&
	  (target->get_type() & dataCell_Regular))
	trgInfo.printf(" [L]");
      else if (target->isNodeDataCell())
	trgInfo.printf(" [C]");
      else if (target->get_type() & dataCell_Regular)
	trgInfo.printf(" [G]");
      else
	trgInfo.printf(" [D]");

      outStream << source->get_name() << srcInfo.str() << arrow.str()
	        << target->get_name() << trgInfo.str() << endl;
    }
  }

  Obj *callSet = graph_get_calls(this);
  Obj *call;

  ForEach(call, *callSet) {
    GDGraph *graph = (GDGraph *) call;

    if (graph)
      graph->printRelations(outStream);
  }
}

extern "C"
void
graph_print(GDGraph *graph)
{
  graph->printRelations(cout);
}

extern "C"
void
dfa_print(char const *name)
{
  symbolPtr symbol = api_lookup_xrefSymbol(DD_FUNC_DECL, name);
  symbol = symbol.get_def_symbol();
  projNode *project = symbol.get_projNode();

  if (!project) {
    msg("The DFA information for $1 does not exist.", error_sev)
	<< name << eoarg << eom;
    return;
  }

  GDSet *hashtable = project_get_table(project);

  if (!hashtable) {
    msg("The DFA information for $1 does not exist.", error_sev)
	<< name << eoarg << eom;
    return;
  }

  GDGraph *graph = hashtable->find(symbol, true);
  graph->printRelations(cout);
}

unsigned int
GDGraph::mergeRelations(bool abstract)
{
  Initialize(GDGraph::mergeRelations(bool));

  if (abstract && call_get_graph(this))
    return 0;

  bool mode = false;
  unsigned int check = 0;
  unsigned int count = 0;
  unsigned int maxCount = 0;
  GDSet *hashtable = getHashtable(true);

  if (hashtable && hashtable->getProjGraph() == this)
    mode = true;

  while (true) {
    unsigned int temp = 0;

    if (oldCells_.size() == 0) {
      if (newCells_.size() == 0)
	return check;

      renewOldCells();
    }

    symbolPtr first, second;
    // indSet_ doesn't change when merging

    ForEachT(first, indSet_) {
      if (first.get_kind() != DD_FUNC_DECL)
	continue;

      int index = indSet_.get_index(first);
      DataSet *bucket = *dataSet_[index];

      if (bucket)
	temp += bucket->mergeRelations(this, abstract);
    }

    ForEachT(second, indSet_) {
      if (second.get_kind() == DD_FUNC_DECL)
	continue;

      int index = indSet_.get_index(second);
      DataSet *bucket = *dataSet_[index];

      if (bucket)
	temp += bucket->mergeRelations(this, abstract);
    }

    renewOldCells();
    check += temp;

    if (mode) {
      if (arena)
	showDFAinfo(count, 0, 1);

      if (++maxCount == DFA_MAX_GLOBAL)
	return check;
    }
  }

  return check;
}

unsigned int
GDGraph::iterationStep(dataCell *firstCell,
		       dataCell *secondCell,
		       int adjustment,
		       GDFlag flag,
		       bool abstract)
{
  Initialize(GDGraph::iterationStep(dataCell *, dataCell *, int, GDFlag, bool));

  if (!firstCell || !secondCell || !firstCell->isValid() ||
      !secondCell->isValid())
    return 0;

  unsigned int check = 0;
  dataCell *srcField = firstCell->getField();
  dataCell *trgField = secondCell->getField();
  int srcLevel = firstCell->get_level();
  int trgLevel = secondCell->get_level();

  while (srcField && trgField) {
    srcLevel = srcField->get_level();
    trgLevel = trgField->get_level();
    srcField = srcField->getField();
    trgField = trgField->getField();
  }

  if ((adjustment > 0 && !srcField && trgField) ||
      (adjustment < 0 && srcField && !trgField))
    return 0;

  GDSet *hashtable = getHashtable(true);
  GDGraph *projGraph = hashtable->getProjGraph();

  if (adjustment != 0 && !srcField && !trgField &&
      (srcLevel > 0 || trgLevel > 0) && projGraph == this)
    return 0;

  objArr sourceArr, targetArr;
  Obj *sourceSet = cell_get_alias(firstCell);
  Obj *targetSet = cell_get_alias(secondCell);

  if (!sourceSet) {
    if ((flag & GDR_MERGE_STRAIGHT) || (flag & GDR_MERGE_SOURCES))
      sourceSet = trg_get_srcs(firstCell);
    else if (flag & GDR_MERGE_TARGETS)
      sourceSet = src_get_trgs(firstCell);
  }

  if (!targetSet) {
    if ((flag & GDR_MERGE_STRAIGHT) || (flag & GDR_MERGE_TARGETS))
      targetSet = src_get_trgs(secondCell);
    else if (flag & GDR_MERGE_SOURCES)
      targetSet = trg_get_srcs(secondCell);
  }

  if (sourceSet)
    sourceArr.insert(sourceSet);

  if (targetSet)
    targetArr.insert(targetSet);

  unsigned int srcSize = sourceArr.size();
  unsigned int trgSize = targetArr.size();

  if (srcSize == 0 || trgSize == 0)
    return 0;

  bool srcCheck = true;
  bool trgCheck = true;

  for (unsigned int k = 0; k < srcSize; ++k) {
    dataCell *source = (dataCell *) sourceArr[k];

    if (!source || !source->isValid() || source->isChecked() ||
	(abstract && !source->isAbstract()))
      continue;

    dataCell *srcCell = source;
    dataCell *firstAlias = (dataCell *) cell_get_alias(firstCell);
    dataCell *srcAlias = (dataCell *) cell_get_alias(srcCell);

    if (srcAlias && !srcCheck)
      continue;

    if (firstAlias)
      source = firstAlias;
    else if (srcAlias && srcCheck) {
      source = srcAlias;
      srcCheck = false;
    }

    bool srcPass = source->pass();

    for (unsigned int m = 0; m < trgSize; ++m) {
      dataCell *target = (dataCell *) targetArr[m];

      if (!target || !target->isValid() || target->isChecked() ||
	  (abstract && !target->isAbstract()))
	continue;

      dataCell *trgCell = target;
      dataCell *secondAlias = (dataCell *) cell_get_alias(secondCell);
      dataCell *trgAlias = (dataCell *) cell_get_alias(trgCell);

      if (trgAlias && (!trgCheck || srcAlias))
	continue;

      if (secondAlias)
	target = secondAlias;
      else if (trgAlias && trgCheck) {
	target = trgAlias;
	trgCheck = false;
      }

      if (source == target && !srcField && !trgField)
	continue;

      if (srcCell->isNodeDataCell() && trgCell->isNodeDataCell() &&
	  !(srcCell->get_type() & dataCell_Regular) &&
	  !(trgCell->get_type() & dataCell_Regular))
	continue;

      bool trgPass = target->pass();

      if (!srcPass && !trgPass)
	continue;

      if (projGraph == this) {
	if (!(srcCell->get_type() & dataCell_Regular) &&
	    !(trgCell->get_type() & dataCell_Regular) &&
	    ((!srcCell->isNodeDataCell() && !trgCell->isNodeDataCell()) ||
	     (srcCell->isNodeDataCell() && trgCell->isNodeDataCell()))) {
	  ++rejectedRelations;
	  continue;
	}

	if ((firstCell->get_type() & dataCell_Regular) &&
	    (secondCell->get_type() & dataCell_Regular) &&
	    !(srcCell->get_type() & dataCell_Regular) &&
	    !(trgCell->get_type() & dataCell_Regular)) {
	  ++rejectedRelations;
	  continue;
	}
      }

      bool warn = false;

      if (projGraph == this &&
	  !(firstCell->get_type() & dataCell_Regular))
	warn = true;

      check += processRelation(source, target, srcField, trgField,
			       flag, adjustment, abstract, warn);

      if (secondAlias)
	break;
    }

    if (firstAlias)
      break;
  }

  return check;
}

unsigned int
GDGraph::processRelation(dataCell *source,
			 dataCell *target,
			 dataCell *srcField,
			 dataCell *trgField,
			 GDFlag flag,
			 int adjustment,
			 bool abstract,
			 bool warn)
{
  Initialize(GDGraph::processRelation(dataCell *, dataCell *, dataCell *, dataCell *, GDFlag, int, bool, bool));

  if ((adjustment > 0 && !srcField && trgField) ||
      (adjustment < 0 && srcField && !trgField))
    return 0;

  unsigned int check = 0;
  GDFlag realFlag = flag;
  dataCell *realSource = source;
  dataCell *realTarget = target;
  bool srcNew = false;
  bool trgNew = false;
  objSet dataSet, aliasSet, saveSet;
  Obj *data, *save, *data2, *save2;

  if (srcField && !trgField) {
    dataCell *temp = srcField;

    while (temp->getField())
      temp = temp->getField();

    int level = temp->get_level();

    if (level < 0) {
      if (!source->transform(0 - level, NULL, saveSet, this))
	return 0;

      srcNew = true;
      srcField = new dataCell(*srcField);
      srcField->shiftLevel(0 - level);
    } else
      saveSet.insert(source);
  } else if (!srcField && trgField) {
    dataCell *temp = trgField;

    while (temp->getField())
      temp = temp->getField();

    int level = temp->get_level();

    if (level < 0) {
      if (!target->transform(0 - level, NULL, saveSet, this))
	return 0;

      trgNew = true;
      trgField = new dataCell(*trgField);
      trgField->shiftLevel(0 - level);
    } else
      saveSet.insert(target);
  } else if (adjustment > 0)
    saveSet.insert(source);
  else if (adjustment < 0)
    saveSet.insert(target);

  if (adjustment > 0 || (srcField && !trgField)) {
    if (!target->transform(adjustment, srcField, dataSet, this)) {
      if (srcNew)
	delete srcField;

      if (trgNew)
	delete trgField;

      return 0;
    }

    if (srcField)
      realFlag |= GDR_FIELD_TARGET;
  } else if (adjustment < 0 || (!srcField && trgField)) {
    if (!source->transform(0 - adjustment, trgField, dataSet, this)) {
      if (srcNew)
	delete srcField;

      if (trgNew)
	delete trgField;

      return 0;
    }

    if (trgField)
      realFlag |= GDR_FIELD_SOURCE;
  }

  ForEach(data, dataSet) {
    bool order = true;

    if (adjustment < 0 || (!srcField && trgField))
      realSource = (dataCell *) data;
    else if (adjustment > 0 || (srcField && !trgField)) {
      realTarget = (dataCell *) data;
      order = false;
    } else
      break;

    ForEach(save, saveSet) {
      if (adjustment < 0 || (!srcField && trgField))
	realTarget = (dataCell *) save;
      else if (adjustment > 0 || (srcField && !trgField))
	realSource = (dataCell *) save;
      else
	break;

      if ((!realSource && order) || (!realTarget && !order))
	break;

      if (!realSource || !realTarget)
	continue;

      if (warn && (realSource->get_type() & dataCell_Regular) &&
	  (realTarget->get_type() & dataCell_Regular))
	continue;

      dataCell *srcAlias = (dataCell *) cell_get_alias(realSource);
      dataCell *trgAlias = (dataCell *) cell_get_alias(realTarget);

      if (srcAlias || trgAlias) {
	if ((order && srcAlias) || (!order && trgAlias))
	  break;
	else
	  continue;
      }

      check += addRelation(realSource, realTarget, realFlag,
			   abstract, false, false);
    }
  }

  if (warn && (realSource->get_type() & dataCell_Regular) &&
      (realTarget->get_type() & dataCell_Regular)) {
    if (srcNew)
      delete srcField;

    if (trgNew)
      delete trgField;

    return check;
  }

  if (adjustment == 0 && !srcField && !trgField)
    check += addRelation(realSource, realTarget, realFlag,
			 abstract, false, false);

  ForEach(data2, dataSet) {
    dataCell *cell = (dataCell *) data2;

    if (cell && !cell->get_graph())
      delete cell;
  }

  ForEach(save2, saveSet) {
    dataCell *cell = (dataCell *) save2;

    if (cell && !cell->get_graph())
      delete cell;
  }

  if (srcNew)
    delete srcField;

  if (trgNew)
    delete trgField;

  return check;
}

static void
bailout(dataCell *&source,
	dataCell *&target,
	bool srcClean,
	bool trgClean)
{
  Initialize(bailout(dataCell *&, dataCell *&, bool, bool));

  if (srcClean) {
    delete source;
    source = NULL;
  }

  if (trgClean) {
    delete target;
    target = NULL;
  }
}

unsigned int
GDGraph::addRelation(dataCell *&source,
		     dataCell *&target,
		     GDFlag flag,
		     bool abstract,
		     bool srcClean,
		     bool trgClean,
		     int mode)
{
  Initialize(GDGraph::addRelation(dataCell *&, dataCell *&, GDFlag, bool, bool, bool, int));

  if (!source || !target)
    return 0;

  if (!source->isValid() || !target->isValid() ||
      source->isChecked() || target->isChecked()) {
    bailout(source, target, srcClean, trgClean);
    return 0;
  }

  unsigned int check = 0;
  symbolPtr srcSymbol = source->get_symbol();
  symbolPtr trgSymbol = target->get_symbol();
  ddKind srcKind = srcSymbol.get_kind();
  ddKind trgKind = trgSymbol.get_kind();
  bool extra = false;

  if (trgKind == DD_NUMBER || trgKind == DD_STRING ||
      trgKind == DD_ENUM_VAL) {
    bailout(source, target, srcClean, trgClean);
    return 0;
  }

  if ((target->get_type() & dataCell_Constant) && !target->getField()) {
    bailout(source, target, srcClean, trgClean);
    return 0;
  }

  if (flag != GDR_IRREDUCIBLE && flag != GDR_MERGE_UP &&
      (srcKind == DD_NUMBER || srcKind == DD_STRING ||
       srcKind == DD_ENUM_VAL)) {
    bailout(source, target, srcClean, trgClean);
    return 0;
  }

  if (flag != GDR_IRREDUCIBLE && srcKind != DD_NUMBER) {
    dataCell *srcField = source;
    dataCell *temp = source;

    while (temp = temp->getField())
      srcField = temp;

    dataCell *trgField = target;
    temp = target;

    while (temp = temp->getField())
      trgField = temp;

    int srcMaxLevel = srcField->get_maxLevel();
    int trgMaxLevel = trgField->get_maxLevel();

    if (srcMaxLevel >= 0 && trgMaxLevel >= 0) {
      int srcLevel = srcField->get_level();
      int trgLevel = trgField->get_level();

      if (srcMaxLevel - srcLevel != trgMaxLevel - trgLevel) {
	++GDGraph::rejectedRelations_;
	bailout(source, target, srcClean, trgClean);
	return 0;
      }
    } else
      extra = true;
  }

  GDSet *hashtable = getHashtable(true);
  projNode *project = table_get_project(hashtable);
  GDGraph *projGraph = hashtable->getProjGraph();

  if (hashtable != GDSet::Hashtable_) {
    if (!projGraph) {
      symbolPtr projSymbol = project;
      GDHeader *projHeader = new GDHeader(0, 0, 0);
      projGraph = new GDGraph(projHeader, scope_);
      hashtable->add(projGraph, projSymbol);
    } else
      projGraph->loadGraph(true);
  }

  if (mode == 0) {
    if (srcClean || source->get_graph() != this)
      addDataCell(source, srcClean);

    if (!source)
      return 0;

    if (trgClean || target->get_graph() != this)
      addDataCell(target, trgClean);

    if (!target)
      return 0;

    if (extra && flag != GDR_IRREDUCIBLE) {
      dataCell *srcField = source;
      dataCell *temp = source;

      while (temp = temp->getField())
	srcField = temp;

      dataCell *trgField = target;
      temp = target;

      while (temp = temp->getField())
	trgField = temp;

      int srcMaxLevel = srcField->get_maxLevel();
      int trgMaxLevel = trgField->get_maxLevel();

      if (srcMaxLevel >= 0 && trgMaxLevel >= 0) {
	int srcLevel = srcField->get_level();
	int trgLevel = trgField->get_level();

	if (srcMaxLevel - srcLevel != trgMaxLevel - trgLevel) {
	  ++GDGraph::rejectedRelations_;
	  return 0;
	}
      }
    }

    dataCell *srcAlias = (dataCell *) cell_get_alias(source);
    dataCell *trgAlias = (dataCell *) cell_get_alias(target);

    if (srcAlias)
      source = srcAlias;

    if (trgAlias)
      target = trgAlias;

    if (source == target)
      return 0;

    GDFlag oldFlag = getFlag(source, target);

    if (!oldFlag)
      check = 1;
  } else
    check = 1;

  if ((check != 0 || mode_ == GDM_OFFLOADED || mode != 0) &&
      source != target) {
    src_put_trg(source, target);
    GDFlag reverseFlag = 0;
    dataCell_type srcType = source->get_type();
    dataCell_type trgType = target->get_type();

    if (GDSet::isDFApass() && projGraph == this) {
      if (((srcType & dataCell_Constant) && !source->getField() &&
	   target->isNodeDataCell() && !(trgType & dataCell_Regular)))
	reverseFlag = GDR_MERGE_UP | GDR_ALIAS;
      else
	reverseFlag = getFlag(target, source);
    } else if (!GDSet::isDFApass() &&
	       ((source->isNodeDataCell() && !(srcType & dataCell_Regular)) ||
		(target->isNodeDataCell() && !(trgType & dataCell_Regular))) &&
	       !(srcType & dataCell_Shared) && !(trgType & dataCell_Shared) &&
	       !(srcKind == DD_NUMBER || srcKind == DD_STRING ||
		 srcKind == DD_ENUM_VAL))
      reverseFlag = GDR_IRREDUCIBLE | GDR_ALIAS;
    else
      reverseFlag = getFlag(target, source);

    if (reverseFlag) {
      if (mode < 0 && (flag & GDR_ALIAS_SOURCES))
	setAlias(target, source, mode);
      else
	setAlias(source, target, mode);
    }

    if (GDSet::isDFApass() && check != 0 &&
	mode_ != GDM_OFFLOADED) {
      if (mode == 0) {
	++GDGraph::newRelations_;
	++m0_rel;

	if (arena) {
	  int left = 0;
	  int right = 0;

	  if (!(srcType & dataCell_Regular)) {
	    if (source->isNodeDataCell())
	      left = 1;
	    else
	      left = -1;
	  }

	  if (!(trgType & dataCell_Regular)) {
	    if (target->isNodeDataCell())
	      right = 1;
	    else
	      right = -1;
	  }

	  if (left > 0 && right > 0)
	    ++cc_rel;
	  else if (left > 0 && right < 0)
	    ++cd_rel;
	  else if (left > 0 && right == 0)
	    ++fc_rel;
	  else if (left < 0 && right < 0)
	    ++dd_rel;
	  else if (left < 0 && right > 0)
	    ++dc_rel;
	  else if (left < 0 && right == 0)
	    ++fd_rel;
	  else if (left == 0 && right > 0)
	    ++tc_rel;
	  else if (left == 0 && right < 0)
	    ++td_rel;
	  else
	    ++ot_rel;
	}

	if (m0_rel % DFA_RELATION_INCREMENT == 0) {
	  if (arena) {
	    int procSize = proc_size(0);

	    msg("$1 MBytes, call-to-call: $2 defn-to-call: $3 call-to-defn: $4 defn-to-defn: $5 from-call: $6 to-call: $7 from-defn: $8 to-defn: $9 other: $10 mode 0: $11 mode 1: $12 mode -1: $13", normal_sev) 
		<< procSize << eoarg
		<< cc_rel << eoarg
		<< dc_rel << eoarg
		<< cd_rel << eoarg
		<< dd_rel << eoarg 
		<< fc_rel << eoarg
		<< tc_rel << eoarg
		<< fd_rel << eoarg
		<< td_rel << eoarg
		<< ot_rel << eoarg 
		<< m0_rel << eoarg
		<< m1_rel << eoarg
		<< m2_rel << eoarg << eom;

	    cc_rel = 0; dc_rel = 0; cd_rel = 0; dd_rel = 0; fc_rel = 0;
	    tc_rel = 0; dd_rel = 0; fd_rel = 0; td_rel = 0; ot_rel = 0;
	  }

	  m0_rel = 0; m1_rel = 0; m2_rel = 0;
	}
      } else if (mode > 0 && arena)
	++m1_rel;
      else if (mode < 0 && arena)
	++m2_rel;
    }
  }

  addFlag(source, target, flag);

  if ((check != 0 || mode_ == GDM_OFFLOADED) && source != target) {
    bool combine = false;

    if (flag & GDR_MERGE_UP)
      combine = true;

    markNewCell(source, combine);
    markNewCell(target, combine);
    GDGraph *func = call_get_graph(this);

    if (!(source->get_type() & dataCell_UpFlag) &&
	(func || (!source->isNodeDataCell() && !target->isNodeDataCell()))) {
      upCells_.insert_last(source);
      source->add_type(dataCell_UpFlag);
    }
  }

  return check;
}

bool
GDGraph::markNewCell(dataCell *cell,
		     bool flag)
{
  Initialize(GDGraph::markNewCell(dataCell *, bool));

  if (!flag && cell->get_type() & dataCell_NewFlag)
    return false;

  newCells_.insert_last(cell);
  cell->add_type(dataCell_NewFlag);
  dataCell *temp = cell;
  DataSet *bucket = NULL;

  while (temp) {
    symbolPtr symbol = temp->get_symbol();

    if (temp->isNodeDataCell() &&
	!(temp->get_type() & dataCell_Regular))
      symbol = temp->get_xrefSymbol();

    if (!bucket)
      bucket = this->find(symbol);
    else
      bucket = bucket->find(symbol, false);

    if (bucket)
      bucket->markNew(true);

    temp = temp->getField();
  }

  return true;
}

unsigned int
GDGraph::combineRelations(GDGraph *graph,
			  bool abstract)
{
  Initialize(GDGraph::combineRelations(GDGraph *, bool));

  if (!graph)
    return 0;

  unsigned int check = 0;
  symbolPtr symbol;
  graph->getSymbol(symbol);

  if (graph->getStatus() == GDT_GLOBAL)
    graph->loadGraph(true, false);
  else
    graph->loadGraph(true, true);

  GDSet *hashtable = getHashtable();
  GDGraph *projGraph = hashtable->getProjGraph();
  GDGraph *func = call_get_graph(graph);
  unsigned int srcSize = graph->downCells_.size();

  for (unsigned int k = 0; k < srcSize; ++k) {
    dataCell *source = (dataCell *) graph->downCells_[k];

    if (!source || !source->isAbstract() ||
	!source->isValid() || source->isChecked())
      continue;

    if (projGraph == this) {
      if (source->isNodeDataCell() &&
	  (source->get_type() & dataCell_Regular))
	continue;

      source->isConstArg();
    } else if (source->isNodeDataCell() && !func)
      continue;

    Obj *targetSet = src_get_trgs(source);
    Obj *trg;

    if (projGraph == this && source->isNodeDataCell() &&
	!(source->get_type() & dataCell_Regular)) {
      dataCell *alias = cell_get_alias(source);

      if (alias && alias->isNodeDataCell() &&
	  (alias->get_type() & dataCell_Regular))
	targetSet = src_get_trgs(alias);
    }

    ForEach(trg, *targetSet) {
      dataCell *target = (dataCell *) trg;

      if (!target || !target->isAbstract() ||
	  target->isChecked() || !target->isValid())
	continue;

      if (projGraph == this) {
	if (target->isNodeDataCell() &&
	    (target->get_type() & dataCell_Regular))
	  continue;

	target->isConstArg();
      } else if (target->isNodeDataCell() && !func)
	continue;

      if (!func && projGraph != this &&
	  (source->get_type() & dataCell_Regular) &&
	  (target->get_type() & dataCell_Regular))
	continue;

      if (source == target)
	continue;

      if (projGraph == this) {
	if (!(source->get_type() & dataCell_Regular) &&
	    !(target->get_type() & dataCell_Regular) &&
	    ((!source->isNodeDataCell() && !target->isNodeDataCell()) ||
	     (source->isNodeDataCell() && target->isNodeDataCell()))) {
	  ++rejectedRelations;
	  continue;
	}
      }

      objSet srcSet, trgSet;
      source->transform(0, NULL, srcSet, this);
      target->transform(0, NULL, trgSet, this);
      Obj *src, *trg, *src2, *trg2;

      ForEach(src, srcSet) {
	dataCell *srcCell = (dataCell *) src;
	dataCell *realSource = srcCell;
	bool srcCheck = false;

	if (projGraph == this && !srcCell->get_graph()) {
	  srcCell->set_graph(graph);
	  srcCheck = true;
	}

	ForEach(trg, trgSet) {
	  dataCell *trgCell = (dataCell *) trg;
	  dataCell *realTarget = trgCell;
	  bool trgCheck = false;

	  if (projGraph == this && !trgCell->get_graph()) {
	    trgCell->set_graph(graph);
	    trgCheck = true;
	  }

	  check += addRelation(realSource, realTarget, GDR_MERGE_UP,
			       true, false, false);

	  if (trgCheck)
	    trgCell->set_graph(NULL);
	}

	if (srcCheck)
	  srcCell->set_graph(NULL);
      }

      ForEach(src2, srcSet) {
	dataCell *srcCell = (dataCell *) src2;

	if (!srcCell->get_graph()) {
	  delete srcCell;
	  srcCell = NULL;
	}
      }

      ForEach(trg2, trgSet) {
	dataCell *trgCell = (dataCell *) trg2;

	if (!trgCell->get_graph()) {
	  delete trgCell;
	  trgCell = NULL;
	}
      }

      srcSet.remove_all();
      trgSet.remove_all();

      if (!source)
	break;
    }
  }

  if (graph->getStatus() == GDT_GLOBAL && !call_get_graph(graph))
    graph->offloadGraph(false);

  return check;
}

DataSet *
GDGraph::find(const symbolPtr &symbol)
{
  Initialize(GDGraph::find(const symbolPtr &));
  int index = indSet_.get_index(symbol);

  if (index < 0)
    return NULL;

  return
    *dataSet_[index];
}

DataSet *
GDGraph::find(dataCell *cell)
{
  Initialize(GDGraph::find(dataCell *));

  if (!cell || !cell->isValid() || cell->isChecked())
    return NULL;

  symbolPtr symbol = cell->get_symbol();

  if (cell->isNodeDataCell() &&
      !(cell->get_type() & dataCell_Regular))
    symbol = cell->get_xrefSymbol();

  DataSet *bucket = find(symbol);

  if (!bucket)
    return NULL;

  dataCell *field = cell->getField();

  while (field) {
    symbol = field->get_symbol();
    bucket = bucket->find(symbol, false);

    if (!bucket)
      return NULL;

    field = field->getField();
  }

  return bucket;
}

bool
GDGraph::addDataCell(dataCell *&cell,
		     bool clean)
{
  Initialize(GDGraph::addDataCell(dataCell *&, bool));

  if (!cell)
    return false;

  bool mode = clean;

  if (!cell->isValid() || cell->isChecked()) {
    if (mode)
      delete cell;

    cell = NULL;
    return false;
  }

  if (!clean && cell->get_graph() == this)
    return false;

  GDSet *hashtable = getHashtable(true);

  if (hashtable->getProjGraph() == this && cell->get_graph() != this &&
      cell->isNodeDataCell()) {
    callDataCell *newCell = new callDataCell(*cell);

    if (mode)
      delete cell;
    else
      mode = true;

    cell = newCell;
  }

  symbolPtr symbol = cell->get_symbol();

  if (GDSet::isDFApass() && !cell->isNodeDataCell() &&
      symbol.get_scope() != scope_) {
    if (mode) {
      bool check = repairCell(cell);

      if (!check) {
	delete cell;
	cell = NULL;
	return false;
      }
    } else {
      mode = true;
      cell = cell->copy();
      bool check = repairCell(cell);

      if (!check) {
	delete cell;
	cell = NULL;
	return false;
      }
    }

    symbol = cell->get_symbol();
  }

  symbolPtr graphSymbol;
  getSymbol(graphSymbol);
  GDGraph *func = (GDGraph *) call_get_graph(this);

  if (func && !cell->isNodeDataCell() &&
      !(cell->get_type() & dataCell_Regular)) {
    symbolPtr funcSymbol;
    func->getSymbol(funcSymbol);

    if (symbol != graphSymbol && symbol != funcSymbol) {
      if (mode)
	delete cell;

      cell = NULL;
      return false;
    }

    if (symbol != funcSymbol) {
      symbolPtr graphNode;
      getAstSymbol(graphNode);
      nodeDataCell *newCell = new nodeDataCell(*cell);

      if (mode)
	delete cell;
      else
	mode = true;

      cell = newCell;
      cell->set_symbol(graphNode);
      cell->set_xrefSymbol(graphSymbol);
    }
  }

  symbol = cell->get_symbol();

  if (cell->isNodeDataCell() &&
      !(cell->get_type() & dataCell_Regular))
    symbol = cell->get_xrefSymbol();

  int index = indSet_.get_index(symbol);

  if (index < 0) {
    indSet_.insert_last(symbol);
    DataSet *bucket = new DataSet(true);
    dataSet_.append(&bucket);
    bool check = bucket->insert(cell, this, mode);

    if (!cell || !check)
      return false;
  } else {
    DataSet *bucket = *dataSet_[index];

    if (!bucket) {
      if (mode)
	delete cell;

      cell = NULL;
      return false;
    }

    bool check = bucket->insert(cell, this, mode);

    if (!cell || !check || !cell->isValid() || cell->isChecked())
      return false;
  }

  cell->set_graph(this);
  dataCell *field = cell->getField();

  while (field) {
    field->set_graph(this);

    if (!GDSet::isDFApass() && !isDFAinit()) {
      const symbolPtr &fieldSymbol = field->get_symbol();
      index = indSet_.get_index(fieldSymbol);
      DataSet *fieldBucket = NULL;

      if (index < 0) {
	indSet_.insert_last(fieldSymbol);
	fieldBucket = new DataSet(false);
	dataSet_.append(&fieldBucket);
      } else
	fieldBucket = *dataSet_[index];

      if (fieldBucket && !fieldBucket->getFlag())
	fieldBucket->insert(cell, this, false);
    }

    field = field->getField();
  }

  dataCells_.insert_last(cell);
  cell->put_id(dataCells_.size());
  updateBaseLevels(cell);
  ++dataCell::newCount_;
  return true;
}

GDGraph *
GDGraph::getParent() const
{
//  Initialize(GDGraph::getParent() const);

  GDGraph *graph = (GDGraph *) this;
  GDGraph *temp = (GDGraph *) this;

  while (temp = call_get_graph(temp))
    graph = temp;

  return graph;
}

void
GDGraph::updateBaseLevels(dataCell *cell)
{
  Initialize(GDGraph::updateBaseLevels(dataCell *));

  if (!GDSet::isDFApass() || call_get_graph(this) || cell->getField() ||
      !(cell->get_type() & dataCell_Argument) || cell->isNodeDataCell())
    return;

  GDSet *hashtable = getHashtable(true);

  if (hashtable == GDSet::Hashtable_ || hashtable->getProjGraph() == this)
    return;

  int level = cell->get_level();
  unsigned int argno = cell->get_argno();

  if (!base_) {
    unsigned int totalArgno = header_->getArgno() + 1;
    base_ = (int *) malloc(totalArgno * sizeof(int));

    for (unsigned int k = 0; k < totalArgno; ++k) {
      if (k == argno)
	setBaseLevel(k, level);
      else
	setBaseLevel(k, DFA_MAX_LEVEL);
    }
  } else if (getBaseLevel(argno) > level)
    setBaseLevel(argno, level);
}

dataCell *
GDGraph::findDataCell(dataCell *cell,
		      bool mode,
		      bool recurse)
{
  Initialize(GDGraph::findDataCell(dataCell *, bool, bool));

  if (!cell || !cell->isValid() || cell->isChecked())
    return NULL;

  loadGraph(true);
  bool abstract = cell->isAbstract();
  DataSet *bucket = NULL;
  dataCell *realCell = cell;
  bool found = false;

  if (cell->get_type() & dataCell_Regular) {
    const symbolPtr &symbol = cell->get_symbol();
    bucket = find(symbol);
  } else {
    const symbolPtr &symbol = cell->get_xrefSymbol();
    bucket = find(symbol);
  }

  if (!bucket) {
#if 0
    if (!GDSet::isDFApass())
      offloadGraph(false);
#endif

    return NULL;
  }

  dataCell *field = cell->getField();

  while (field && bucket) {
    symbolPtr fieldSymbol = field->get_symbol();
    bucket = bucket->find(fieldSymbol, false);
    field = field->getField();
  }

  if (bucket) {
    objArr dataSet = bucket->getCells();
    unsigned int size = dataSet.size();

    for (unsigned int k = 0; k < size; ++k) {
      dataCell *temp = (dataCell *) dataSet[k];

      if (!temp || !temp->isValid() || temp->isChecked() ||
	  !temp->isAbstract())
	continue;

      if (cell->cellCompare(temp, mode, abstract)) {
	realCell = temp;
	found = true;
	break;
      }
    }
  }

  if (recurse) {
    unsigned int depth = realCell->get_depth();

    if (depth == 0) {
      int level = realCell->get_level();

      if (level > 0) {
	dataCell *newCell = realCell->copy();
	newCell->set_level(0);
	dataCell *testCell = findDataCell(newCell, mode, false);
	delete newCell;

	if (testCell) {
	  objSet dataSet;
	  testCell->transform(level, NULL, dataSet, this);

	  if (dataSet.size() > 0) {
	    Obj *data;

	    ForEach(data, dataSet) {
	      dataCell *nextCell = (dataCell *) data;
	      GDGraph *graph = nextCell->get_graph();

	      if (!found && graph) {
		realCell = nextCell;
		found = true;
	      } else if (!found && !graph) {
		addDataCell(nextCell, true);
		realCell = nextCell;
		found = true;
	      } else if (!graph) {
		delete nextCell;
		nextCell = NULL;
	      }
	    }

	    dataSet.remove_all();
	  }
	}
      }
    } else {
      for (unsigned int m = 0; m < depth; ++m) {
	dataCell *newCell = realCell->copy();
	field = cell;

	for (unsigned int n = 0; n <= m; ++n)
	  field = field->getField();

	for (unsigned int l = 0; l < depth - m; ++l)
	  dataCell::setField(newCell, NULL);

	dataCell *temp = newCell;

	while (temp->getField())
	  temp = temp->getField();

	int level = temp->get_level();
	temp->set_level(0);
	dataCell *testCell = findDataCell(newCell, mode, false);
	delete newCell;

	if (testCell) {
	  dataCell *alias = cell_get_alias(testCell);

	  if (alias) {
	    newCell = alias->copy();
	    bool check = newCell->shiftLevel(level);

	    if (check)
	      delete newCell;
	    else {
	      check = dataCell::setField(newCell, field, false);

	      if (check)
		delete newCell;
	      else {
		addDataCell(newCell, true);

		if (newCell) {
		  objArr targetArr;
		  Obj *targetSet = src_get_trgs(alias);
		  targetArr.insert(targetSet);
		  unsigned int trgSize = targetArr.size();

		  for (unsigned int i = 0; i < trgSize; ++i) {
		    dataCell *target = (dataCell *) targetArr[i];
		    Obj *dataSet = alias_get_cells(target);

		    if (dataSet)
		      targetArr.insert(dataSet);
		  }

		  trgSize = targetArr.size();

		  for (unsigned int j = 0; j < trgSize; ++j) {
		    dataCell *target = (dataCell *) targetArr[j];
		    target = target->copy();
		    check = target->shiftLevel(level);

		    if (check)
		      continue;

		    check = dataCell::setField(target, field, false);

		    if (!check)
		      addDataCell(target, true);

		    if (target)
		      addRelation(newCell, target, GDR_FIELD_ALIAS);
		  }

		  objArr sourceArr;
		  Obj *sourceSet = trg_get_srcs(alias);
		  sourceArr.insert(sourceSet);
		  unsigned int srcSize = sourceArr.size();

		  for (unsigned int ii = 0; ii < srcSize; ++ii) {
		    dataCell *source = (dataCell *) sourceArr[ii];
		    Obj *dataSet = alias_get_cells(source);

		    if (dataSet)
		      sourceArr.insert(dataSet);
		  }

		  srcSize = sourceArr.size();

		  for (unsigned int jj = 0; jj < srcSize; ++jj) {
		    dataCell *source = (dataCell *) sourceArr[jj];
		    source = source->copy();
		    check = source->shiftLevel(level);

		    if (check)
		      continue;

		    check = dataCell::setField(source, field, false);

		    if (!check)
		      addDataCell(source, true);

		    if (source)
		      addRelation(source, newCell, GDR_FIELD_ALIAS);
		  }

		  if (!found) {
		    realCell = newCell;
		    found = true;
		  }
		}
	      }
	    }
	  }
	}
      }
    }
  }

  if (found)
    return realCell;

  return NULL;
}

void
GDGraph::findRelatedCells(dataCell *cell,
			  objSet &dataSet,
			  bool abstract,
			  bool mode)
{
  Initialize(GDGraph::findRelatedCells(dataCell *, objSet &, bool, bool));

  loadGraph(abstract);
  DataSet *bucket = NULL;

  if (!abstract)
    closeGraph(abstract);

  if (cell->get_type() & dataCell_Regular) {
    const symbolPtr &symbol = cell->get_symbol();
    bucket = find(symbol);
  } else {
    const symbolPtr &symbol = cell->get_xrefSymbol();
    bucket = find(symbol);
  }

  if (!bucket)
    return;

  unsigned int size = bucket->size();

  for (unsigned int k = 0; k < size; ++k) {
    dataCell *temp = (dataCell *) (*bucket)[k];

    if (!temp || !temp->isValid() || temp->isChecked() ||
	temp->isAbstract() != abstract)
      continue;

    if (cell->get_argno() != temp->get_argno())
      continue;

    dataCell_type cellType = cell->get_type() & dataCell_Mask1;
    dataCell_type tempType = temp->get_type() & dataCell_Mask1;
    cellType &= ~dataCell_Abstract;
    tempType &= ~dataCell_Abstract;
 
    if (cellType != tempType)
      continue;

    if (mode && (cell->get_level() != temp->get_level() ||
		 cell->get_depth() != temp->get_depth()))
      continue;

    dataSet.insert(temp);
  }
}

void
GDGraph::offloadGraph(bool mode,
		      GDGraph *graph,
		      bool recurse)
{
  Initialize(GDGraph::offloadGraph(bool, GDGraph *, bool));

  if (this == graph || mode_ == GDM_OFFLOADED)
    return;

  symbolPtr test;
  getSymbol(test);

  if (recurse && call_get_graph(this)) {
    GDGraph *parent = getParent();
    parent->offloadGraph(mode, graph, false);
    return;
  }

  GDSet *hashtable = getHashtable(true);

  if (!mode && hashtable->getProjGraph() == this)
    return;

  if (!mode && getFormat() == GDF_ACTIVE &&
      !call_get_graph(this)) {

    settleHeader();

    ostringstream outStream;
    outStream << this;

    if (start_)
      free((char *) start_);

    size_t len = outStream.str().size();
    start_ = (unsigned char *)malloc(len);
    outStream.str().copy((char *)start_, len);
  }

  if (mode) {
    header_->setCallNumber(0);
    header_->setSize(0);
    header_->setTotalSize(0);
  }

  mode_ = GDM_OFFLOADED;
  setFormat(GDF_PASSIVE);
  setRecursion(false);

  indSet_.removeAll();
  unsigned int size = dataCells_.size();

  for (unsigned int k = 0; k < size; ++k) {
    dataCell *cell = (dataCell *) dataCells_[k];
    delete cell;
  }

  dataCells_.removeAll();
  newCells_.removeAll();
  oldCells_.removeAll();
  upCells_.removeAll();
  downCells_.removeAll();
  int bsize = dataSet_.size();

  for (int m = 0; m < bsize; ++m) {
    DataSet *bucket = *dataSet_[m];
    delete bucket;
  }

  dataSet_.reset();
  flags_.reset(true);
  Obj *callSet = graph_get_calls(this);
  Obj *callObj;

  ForEach(callObj, *callSet) {
    GDGraph *call = (GDGraph *) callObj;

    if (call && call != graph)
      call->offloadGraph(mode, graph,  false);
  }
}

void
GDGraph::settleHeader()
{
  Initialize(GDGraph::settleHeader());

  unsigned int graphSize = GDHeader::DB_SIZE();
  unsigned int size = dataCells_.size();
  unsigned int count = 0;

  for (unsigned int k = 0; k < size; ++k) {
    dataCell *source = (dataCell *) dataCells_[k];

    if (!source || !source->isValid() || source->isChecked() ||
	!source->isAbstract() || (!src_get_trgs(source) &&
				  !trg_get_srcs(source) && is_model_build()))
      continue;

    unsigned int srcSize = source->getSize();
    graphSize += srcSize;
    ++count;

    Obj *targetSet = src_get_trgs(source);
    Obj *trg;

    ForEach(trg, *targetSet) {
      dataCell *target = (dataCell *) trg;

      if (!target || !target->isValid() || target->isChecked() ||
	  !target->isAbstract() || source == target)
	continue;

      graphSize += 2 * GDInteger::DB_SIZE() + 2 * sizeof(unsigned char);
    }
  }

  unsigned int totalSize = graphSize;
  Obj *callSet = graph_get_calls(this);
  Obj *call;

  ForEach(call, *callSet) {
    GDGraph *graph = (GDGraph *) call;
    graph->settleHeader();
    GDHeader *header = graph->getHeader();
    totalSize += header->getTotalSize();
  }

  header_->setSize(graphSize);
  header_->setTotalSize(totalSize);
  header_->setCellNumber(count);
}

bool
GDGraph::repairCell(dataCell *cell)
{
  Initialize(GDGraph::repairCell(dataCell *));

  bool check = false;
  symbolPtr graphSymbol;
  getSymbol(graphSymbol);
  projNode *project = graphSymbol.get_projNode();
  Xref *xr = project->get_xref();
  // was:
  // XrefTable *lxref = xr->get_lxref();
  // should be:
  // (void)xr->get_lxref();
  xr->get_lxref();
  //
  symbolArr results;

  symbolPtr symbol = cell->get_symbol();
  symbol = symbol.get_xrefSymbol();
  ddKind cellKind = symbol.get_kind();
  char const *cellName = symbol.get_name();

  if (xr) {
    symbolPtr temp = xr->lookup_local(results, cellKind, cellName, 0);
    unsigned int size = results.size();

    for (unsigned int k = 0; k < size; ++k) {
      temp = results[k];

      if (dataCell::symbolCompare(symbol, temp)) {
	symbol = temp;
	check = true;
	break;
      }
    }
  }

  if (check) {
    cell->set_symbol(symbol);
    dataCell *field = cell->getField();

    if (field) {
      const symbolPtr &fieldSymbol = field->get_symbol();

      if (fieldSymbol.get_scope() != scope_)
	check = repairCell(field);
    }
  }

  if (!check) {
    symbol.put_info(NULL, 0);
    return false;
  }

  unsigned int offset = symbol.get_offset();
  symbol.put_info(scope_, offset);
  return check;
}

ostream &
operator<<(ostream &outStream,
	   GDGraph *graph)
{
  Initialize(operator<<(ostream &, GDGraph *));

  if (!graph)
    return outStream;

  GDHeader *header = graph->getHeader();
  GDFormat format = graph->getFormat();

  if (format == GDF_PASSIVE && graph->start_) {
    int size = header->getTotalSize();
    outStream.write((char const *) graph->start_, size);
  } else if (format != GDF_ZOMBIE) {
    if (!graph->start_ && !call_get_graph(graph))
      graph->settleHeader();

    outStream.write((char const *) header, GDHeader::DB_SIZE());
    unsigned int size = graph->dataCells_.size();
    unsigned int *indexArr = (unsigned int *) malloc(size * sizeof(unsigned int));
    unsigned int index = 0;

    if (!indexArr) {
      msg ("Couldn't allocate memory.", catastrophe_sev) << eom;
      return outStream;
    }

    for (unsigned int k = 0; k < size; ++k) {
      dataCell *cell = (dataCell *) (graph->dataCells_)[k];

      if (!cell || !cell->isValid() || cell->isChecked() ||
	  !cell->isAbstract() || (!src_get_trgs(cell) &&
				  !trg_get_srcs(cell) && is_model_build())) {
	*(indexArr + k) = 0;
	continue;
      }

      *(indexArr + k) = ++index;
      outStream << cell;
    }

    for (unsigned int m = 0; m < size; ++m) {
      dataCell *source = (dataCell *) (graph->dataCells_)[m];

      if (!source || !source->isValid() || source->isChecked() ||
	  !source->isAbstract() || (!src_get_trgs(source) &&
				    !trg_get_srcs(source) && is_model_build()))
	continue;

      GDInteger srcID = *(indexArr + m);
      Obj *targetSet = src_get_trgs(source);
      Obj *trg;

      ForEach(trg, *targetSet) {
	dataCell *target = (dataCell *) trg;

	if (!target || !target->isValid() || target->isChecked() ||
	    !target->isAbstract() || source == target)
	  continue;

	int temp = target->get_id() - 1;
	GDInteger trgID = *(indexArr + temp);
	GDFlag flag = graph->getFlag(source, target);
	dataCell *alias = (dataCell *) cell_get_alias(source);

	if (alias == target)
	  flag |= GDR_ALIAS;

	unsigned char flag_1 = (unsigned char) ((flag >> 8) & 0xFF);
	unsigned char flag_2 = (unsigned char) (flag & 0xFF);
	outStream << srcID << trgID << flag_1 << flag_2;
      }
    }

    free((char *) indexArr);
    unsigned char const *start = NULL;

    if (graph->start_)
      start = graph->start_ + header->getSize();

    Obj *callSet = graph_get_calls(graph);
    Obj *callObj;

    ForEach(callObj, *callSet) {
      GDGraph *call = (GDGraph *) callObj;

      if (!call)
	continue;

      if (start)
	call->start_ = start;
      else if (call->start_) {
	free((char *) call->start_);
	call->start_ = NULL;
      }

      GDHeader *callHeader = call->getHeader();
      start += callHeader->getTotalSize();
      outStream << call;
    }
  }

  return outStream;
}

void
GDGraph::getSymbol(symbolPtr &symbol,
		   GDSet *hashtable) const
{
  Initialize(GDGraph::getSymbol(symbolPtr &, GDSet *) const);
  unsigned int offset = header_->getSymbolID();

  if (offset != 0)
    symbol.put_info(scope_, offset);
  else {
    GDSet *table = hashtable;

    if (!table)
      table = graph_get_table(this);

    if (table) {
      projNode *project = table_get_project(table);
      symbol = project;
    }
  }
}

void
GDGraph::getAstSymbol(symbolPtr &symbol,
		      bool mode) const
{
  Initialize(GDGraph::getAstSymbol(symbolPtr &, bool) const);

  getFileSymbol(symbol);
  char const *name = symbol.get_name();
  unsigned int offset = header_->getNodeID();
  symbol = astNode_get_node(name, offset);
}

void
GDGraph::getFileSymbol(symbolPtr &symbol) const
{
  Initialize(GDGraph::getFileSymbol(symbolPtr &) const);
  unsigned int offset = header_->getFileID();

  if (offset == 0)
    symbol.put_info(NULL, 0);
  else
    symbol.put_info(scope_, offset);
}

GDGraph *
GDGraph::findCall(const symbolPtr &symbol)
{
  Initialize(GDGraph::findCall(const symbolPtr &));
  GDGraph *graph = NULL;

  loadGraph(true);
  Obj *callSet = graph_get_calls(this);
  Obj *callObj;

  ForEach(callObj, *callSet) {
    GDGraph *call = (GDGraph *) callObj;
    symbolPtr callSymbol;
    call->getAstSymbol(callSymbol);

    if (dataCell::symbolCompare(symbol, callSymbol)) {
      graph = call;
      break;
    }

    graph = call->findCall(symbol);

    if (graph)
      break;
  }

#if 0
  offloadGraph(false);
#else
  if (GDSet::isDFApass())
    offloadGraph(false);
#endif

  return graph;
}

void
GDGraph::reshuffle()
{
  Initialize(GDGraph::reshuffle());

  setOldCells();
  setDownCells();
  resetNewCells();
  resetUpCells();
  unsigned int size = dataSet_.size();

  for (unsigned int k = 0; k < size; ++k) {
    DataSet *bucket = *dataSet_[k];

    if (bucket)
      bucket->reshuffle(true);
  }
}

void
GDGraph::setOldCells()
{
  Initialize(GDGraph::setOldCells());

  oldCells_ = dataCells_;
  unsigned int size = oldCells_.size();

  for (unsigned int k = 0; k < size; ++k) {
    dataCell *cell = (dataCell *) oldCells_[k];
    cell->add_type(dataCell_OldFlag);
  }
}

void
GDGraph::setDownCells()
{
  Initialize(GDGraph::setDownCells());

  downCells_ = dataCells_;
  unsigned int size = downCells_.size();

  for (unsigned int k = 0; k < size; ++k) {
    dataCell *cell = (dataCell *) downCells_[k];
    cell->add_type(dataCell_DownFlag);
  }
}

void
GDGraph::resetNewCells()
{
  Initialize(GDGraph::resetNewCells());
  unsigned int size = newCells_.size();

  for (unsigned int k = 0; k < size; ++k) {
    dataCell *cell = (dataCell *) newCells_[k];
    cell->rem_type(dataCell_NewFlag);
  }

  newCells_.removeAll();
}

void
GDGraph::resetUpCells()
{
  Initialize(GDGraph::resetUpCells());
  unsigned int size = upCells_.size();

  for (unsigned int k = 0; k < size; ++k) {
    dataCell *cell = (dataCell *) upCells_[k];
    cell->rem_type(dataCell_UpFlag);
  }

  upCells_.removeAll();
}

void
GDGraph::resetOldCells()
{
  Initialize(GDGraph::resetOldCells());
  unsigned int size = oldCells_.size();

  for (unsigned int k = 0; k < size; ++k) {
    dataCell *cell = (dataCell *) oldCells_[k];
    cell->rem_type(dataCell_OldFlag);
  }

  oldCells_.removeAll();
}

void
GDGraph::renewOldCells()
{
  Initialize(GDGraph::renewOldCells());
  unsigned int oldSize = oldCells_.size();

  for (unsigned int k = 0; k < oldSize; ++k) {
    dataCell *oldCell = (dataCell *) oldCells_[k];
    oldCell->rem_type(dataCell_OldFlag);
  }

  oldCells_ = newCells_;
  newCells_.removeAll();
  unsigned int newSize = oldCells_.size();

  for (unsigned int m = 0; m < newSize; ++m) {
    dataCell *newCell = (dataCell *) oldCells_[m];
    newCell->rem_type(dataCell_NewFlag);
    newCell->add_type(dataCell_OldFlag);
  }

  unsigned int size = dataSet_.size();

  for (int n = 0; n < size; ++n) {
    DataSet *bucket = *dataSet_[n];

    if (bucket)
      bucket->reshuffle(false);
  }
}

void
GDGraph::renewDownCells()
{
  Initialize(GDGraph::renewDownCells());
  unsigned int oldSize = downCells_.size();

  for (unsigned int k = 0; k < oldSize; ++k) {
    dataCell *oldCell = (dataCell *) downCells_[k];
    oldCell->rem_type(dataCell_DownFlag);
  }

  downCells_ = upCells_;
  upCells_.removeAll();
  unsigned int newSize = downCells_.size();

  for (unsigned int m = 0; m < newSize; ++m) {
    dataCell *newCell = (dataCell *) downCells_[m];
    newCell->rem_type(dataCell_UpFlag);
    newCell->add_type(dataCell_DownFlag);
  }
}

dataCell *
GDGraph::operator[](int ID)
{
  Initialize(GDGraph::operator[](int));

  if (ID < 0)
    return NULL;

  dataCell *cell = NULL;
  loadGraph(true);

  if (ID < dataCells_.size())
    cell = (dataCell *) dataCells_[ID];

  if (!cell) {
    loadGraph(false);
    closeGraph(false);

    if (ID < dataCells_.size())
      cell = (dataCell *) dataCells_[ID];
  }

  return cell;
}

unsigned char const *
GDGraph::getStart(unsigned int ID)
{
  Initialize(GDGraph::getStart(unsigned int));
  unsigned char const *start = start_;

  if (!hashArr_ && ID != 0 && !GDSet::isDFApass()) {
    if (mode_ == GDM_NONE || mode_ == GDM_OFFLOADED) { 
      loadGraph(true);
      offloadGraph(false);
    } else {
      offloadGraph(false);
      loadGraph(true);
    }
  }

  start += *(hashArr_ + ID);
  return start;
}

GDGraph::GDGraph(ifstream &inStream,
		 scopeNode *scope)
  : scope_(scope), format_(GDF_PASSIVE), mode_(GDM_NONE),
    recursion_(false), base_(NULL), hashArr_(NULL), indSet_(is_model_build())
{
  Initialize(GDGraph::GDGraph(ifstream &, scopeNode *));
  unsigned char *start = (unsigned char *) malloc(GDHeader::DB_SIZE());

  if (start) {
    inStream.read((char *) start, GDHeader::DB_SIZE());
    header_ = new GDHeader(start);
    unsigned int size = header_->getTotalSize();
    unsigned char *tmp = (unsigned char *)malloc(size);

    if (tmp) {
      memcpy(tmp, start, GDHeader::DB_SIZE());
      inStream.read((char *) tmp + GDHeader::DB_SIZE(),
		    size - GDHeader::DB_SIZE());
      start_ = tmp;
      free((char *) start);
    }
  }
}

GDSet::GDSet(projNode *project,
	     bool mode)
  : status_(0), set_(), array_(), projGraph_(NULL),
    currGraph_(NULL), currFile_(NULL)
{
  Initialize(GDSet::GDSet(projNode *));

  if (mode && _lh(LIC_DFA)) {
    if (_lo(LIC_DFA)) {
      if (GDSet::errorMessage_) {
	_lm(LIC_DFA);
	GDSet::errorMessage_ = false;
      }

      status_ = 0;
      return;
    }
  }

  int sym_res = projNode_set_pmod_names(project);

  if (sym_res == 0 || !project->sym_name) {
    status_ = 0;
    return;
  }

  genString fileName = project->sym_name->get_name();
  ((char *) fileName)[fileName.length() - 3] = '\0';
  fileName += "dfa";

#ifdef _WIN32
  ifstream inStream(OSPATH((char *) fileName),
		    ios::in | ios::binary);
#else
  ifstream inStream(OSPATH((char *) fileName),
		    ios::in );
#endif

  project_put_table(project, this);
  GDSet::tableSet_.insert_last(this);

  if (!inStream || inStream.eof()) {
    status_ = 0;
    return;
  }

  unsigned char token;
  inStream.read((char *) &token, sizeof(unsigned char));

  if (inStream.eof()) {
    status_ = 0;
    return;
  }

  unsigned char status;
  inStream.read((char *) &status, sizeof(unsigned char));
  status_ = (GDType) status;

  unsigned int count = 0;
  scopeNode *scope = (scopeNode *) project->get_xref();
  // was:
  // XrefTable *lxref = ((Xref *) scope)->get_lxref();
  // should be:
  // (void)((Xref *) scope)->get_lxref();
  ((Xref *) scope)->get_lxref();
  unsigned char *start = (unsigned char *) malloc(GDInteger::DB_SIZE());

  if (start) {
    inStream.read((char *) start, GDInteger::DB_SIZE());

    if (!inStream.eof()) {
      GDInteger number(start);
      count = (unsigned int) number;
    }

    free((char *) start);
  } else {
    status_ = 0;
    msg("Couldn't allocate memory.", catastrophe_sev) << eom;
    return;
  }

  if (count > 0) {
    msg("Restoring the DFA information for the project $1", normal_sev) << project->get_name() << eom;

    if (token != PMOD_DFA_TOKEN) {
      char const *realFileName = realOSPATH((char *) fileName.str());
      if (is_model_build())
	msg("The $1 file is in the outdated format or corrupted.\nAttempting to regenerate the DFA information for the project $2.", warning_sev) << realFileName << eoarg << project->get_name() << eom;
      else {
	msg("The $1 file is in the outdated format or corrupted.\nThe DFA information for the project $2 is lost.", warning_sev) << realFileName << eoarg << project->get_name() << eom;
      }

      status_ = 0;
      return;
    } else if ((!status_ || (status_ & GDT_LOCAL)) && !is_model_build()) {
      msg("The DFA information for the project $1 is incomplete. Please run the DFA pass.", warning_sev) << project->get_name() << eoarg << eom;
      status_ = 0;
      return;
    } else if ((status_ & GDT_UPDATED) && !is_model_build())
      msg("The DFA information for the project $1 is partially incomplete. The DFA queries might run slower.", warning_sev) << project->get_name() << eoarg << eom;
    for (unsigned int k = 0; k < count && !inStream.eof(); ++k) {
      GDGraph *graph = new GDGraph(inStream, scope);
      symbolPtr symbol;
      graph->getSymbol(symbol, this);
      this->add(graph, symbol);
      symbolPtr fileSymbol;
      graph->getFileSymbol(fileSymbol);

      if (fileSymbol.isnotnull() && !fileSet_.includes(fileSymbol))
	fileSet_.insert_last(fileSymbol);
    }
  }
}

GDGraph::~GDGraph()
{
  Initialize(GDGraph::~GDGraph());
  offloadGraph(true);

  if (header_) {
    delete header_;
    header_ = NULL;
  }

  if (start_ && !call_get_graph(this)) {
    free((char *) start_);
    start_ = NULL;
  }

  if (base_) {
    free((char *) base_);
    base_ = NULL;
  }

  if (hashArr_) {
    free((char *) hashArr_);
    hashArr_ = NULL;
  }
}

GDSet::~GDSet()
{
  Initialize(GDSet::~GDSet());

  GDSet::tableSet_.remove(this);
  unsigned int size = array_.size();

  if (projGraph_) {
    projGraph_->offloadGraph(true);
    obj_delete(projGraph_);
    projGraph_ = NULL;
  }

  for (unsigned int k = 0; k < size; ++k) {
    GDGraph *graph = *array_[k];

    if (graph) {
      graph->offloadGraph(true);
      obj_delete(graph);
      graph = NULL;
    }
  }
}

void
GDSet::saveData()
{
  Initialize(GDSet::saveData());

  projNode *project = table_get_project(this);

  if (project && status_ && project->sym_name) {
    genString fileName = project->sym_name->get_name();
    ((char *) fileName)[fileName.length() - 3] = '\0';
    fileName += "dfa";

#ifdef _WIN32
    ofstream outStream(OSPATH((char *) fileName), ios::out | ios::binary);
#else
    ofstream outStream(OSPATH((char *) fileName), ios::out);
#endif

    if (outStream) {
      outStream << this;
      outStream.close();
    }
  }
}

void
GDSet::saveDFA(GDSet *hashtable,
	       GDType status,
	       bool mode)
{
  Initialize(GDSet::saveDFA(GDSet *, GDType, bool));

  if (hashtable)
    hashtable->setStatus(status);

  unsigned int size = GDSet::tableSet_.size();

  for (unsigned int k = size; k > 0; --k) {
    GDSet *current = (GDSet *) GDSet::tableSet_[k - 1];

    if (!current)
      continue;

    GDType currStatus = current->getStatus();

    if (currStatus && currStatus != GDT_GLOBAL &&
	current != hashtable)
      current->update(status);

    if (currStatus != GDT_GLOBAL || current == hashtable)
      current->saveData();

    if (mode) {
      delete current;
      current = NULL;
    }
  }

#if 0
  if (hashtable)
    _li(LIC_DFA);
#endif

  if (arena) {
    unsigned int dummy = 0;
    showDFAinfo(dummy, 0, -1);
  }
}

GDGraph *
GDSet::find(symbolPtr &symbol,
	    bool mode)
{
  Initialize(GDSet::find(symbolPtr &, bool));

  if (!GDSet::Hashtable_)
    GDSet::loadStandardFunctions();

  GDGraph *graph = NULL;
  int index = set_.get_index(symbol);

  if (index < 0 && mode)
    GDSet::createStub(symbol);

  if (index >= 0)
    graph = *array_[index];

  return graph;
}

GDGraph *
GDSet::search(symbolPtr &symbol,
	      bool mode)
{
  Initialize(GDSet::search(symbolPtr &, bool));

  if (!GDSet::Hashtable_)
    GDSet::loadStandardFunctions();

  GDGraph *graph = NULL;

  if (symbol.get_kind() == DD_PROJECT) {
    projNode *project = symbol.get_projNode();
    GDSet *hashtable = project_get_table(project);

    if (!hashtable)
      hashtable = new GDSet(project);

    graph = hashtable->getProjGraph();
  } else if (symbol.is_xrefSymbol()) {
    projNode *project = symbol.get_projNode();

    if (!project)
      return NULL;

    GDSet *hashtable = project_get_table(project);

    if (!hashtable)
      hashtable = new GDSet(project);

    graph = hashtable->find(symbol, false);

    if (!graph)
      graph = GDSet::Hashtable_->find(symbol, mode);
  } else if (symbol.is_ast() || symbol.is_instance()) {
    if (GDSet::isDFApass())
      return NULL;

    symbolPtr node = symbol;

    if (symbol.is_instance())
      node = symbol.get_astNode();

    if (node.isnull())
      return NULL;

    int code = symbolPtr_get_code(node);
    symbolPtr graphSymbol, callSymbol, temp;

    for (temp = node;
	 temp.isnotnull();
	 tree_get_parent(temp, temp)) {
      int tempCode = symbolPtr_get_code(temp);

      if (tempCode == MC_FUNCTION_DECL || tempCode == DFA_FUNCTION_DEF) {
	graphSymbol = temp.get_xrefSymbol();
	break;
      } else if (tempCode == MC_CALL_EXPR || tempCode == NEW_EXPR) {
	callSymbol = temp.get_xrefSymbol();
	break;
      }
    }

    if (graphSymbol.isnull()) {
      if (callSymbol.isnull()) {
	const symbolPtr &xrsym = symbol.get_xrefSymbol();

	if (xrsym.isnull())
	  return NULL;

	graphSymbol = xrsym->get_def_file();
	GDGraph *graph = GDSet::search(graphSymbol, mode);
	return graph;
      }

      for (temp = node;
	   temp.isnotnull();
	   tree_get_parent(temp, temp)) {
	if (symbolPtr_get_code(temp) == DFA_FUNCTION_DEF) {
	  graphSymbol = temp.get_xrefSymbol();
	  break;
	}
      }

      if (graphSymbol.isnull()) {
	astXref *axr = node.get_astXref();
	graphSymbol = axr->get_module();
	graphSymbol = graphSymbol.get_xrefSymbol();
      }
    }

    GDGraph *func = GDSet::search(graphSymbol, mode);

    if (callSymbol.isnotnull() && func)
      graph = func->findCall(symbol);
    else
      graph = func;
  }

  return graph;
}

Obj *
GDSet::search(symbolPtr &symbol,
	      objSet &defnSet,
	      bool mode)
{
  Initialize(GDSet::search(symbolPtr &, objSet &, bool));

  if (is_model_build() && !GDSet::isDFApass())
    return NULL;

  if (!GDSet::Hashtable_)
    GDSet::loadStandardFunctions();

  if (testName && strstr(symbol.get_name(), testName))
    dfa_debug_hook(testName);

  symbolArr allDefSymbols;
  unsigned int size = symbol.get_def_symbols(allDefSymbols, 2);

  for (unsigned int k = 0; k < size; ++k) {
    symbolPtr defSymbol = allDefSymbols[k];
    GDGraph *defn = GDSet::search(defSymbol, mode);

    if (defn)
      defnSet.insert(defn);
  }

  if (size == 0) {
    GDGraph *graph = GDSet::search(symbol, mode);

    if (graph)
      defnSet.insert(graph);
  }

  return &defnSet;
}

void
GDSet::closeGraphs()
{
  Initialize(GDSet::closeGraphs());

  if (status_ == GDT_GLOBAL)
    return;

  update(GDT_LOCAL);
  symbolArr funcOld, funcNew;
  genArr(GDPointer) fileArr;
  unsigned int size = array_.size();
  GDSet::total_ = size;
  GDSet::count_ = 0;
  DFAthreshold = 0;

  for (unsigned int k = 0; k < size; ++k) {
    GDGraph *graph = *array_[k];
    symbolPtr symbol;
    graph->getSymbol(symbol);

    if (symbol.get_kind() == DD_FUNC_DECL)
      funcOld.insert_last(symbol);
    else
      fileArr.append(&graph);

    if (graph->getStatus() == GDT_GLOBAL &&
	!(status_ & GDT_UPDATED) && graph->getMode() == GDM_NONE)
      GDSet::progressMessage();
  }

  funcOld.sort();

  msg("DFA: Closing the Call Graphs.", normal_sev) << eom;
  timer::init(1, "Closing the call graphs", "");

  currentTable = NULL;
  stubArr.removeAll();
  currentSet.remove_all();
  call_graph_closure(funcOld, funcNew, calls_functions);
  currentTable = NULL;
  currentSet.remove_all();
  unsigned int funcSize = funcNew.size();
  unsigned int fileSize = fileArr.size();

  timer::showtime(1, "Closing the call graphs", "");

  if (projGraph_)
    projGraph_->loadGraph(true);

  for (unsigned int m = 0; m < funcSize; ++m) {
    symbolPtr funcSymbol = funcNew[m];
    GDGraph *funcGraph = GDSet::search(funcSymbol, false);

    if (funcGraph && (funcGraph->getStatus() != GDT_GLOBAL ||
		      (status_ & GDT_UPDATED)))
      funcGraph->closeGraph(true);
  }

  for (unsigned int n = 0; n < fileSize; ++n) {
    GDGraph *fileGraph = *fileArr[n];

    if (fileGraph && (fileGraph->getStatus() != GDT_GLOBAL ||
		      (status_ & GDT_UPDATED)))
      fileGraph->closeGraph(true);
  }

  if (projGraph_) {
#ifdef XXX_DFAglobal
    DFAglobal = true;
#endif
    unsigned int dummy = 0;
    showDFAinfo(dummy, 0, 0);
    time_t curr = (time_t)my_time();
    msg("PROGRESS ESTIMATE: $1GLOBAL PHASE ", normal_sev) << ctime(&curr) << eom;
    projGraph_->closeGraph(true);
#ifdef XXX_DFAglobal
    DFAglobal = false;
#endif
  }

  setStatus(GDT_GLOBAL);
  GDSet::total_ = 0;
  GDSet::count_ = 0;
  DFAthreshold = 0;
}

void
GDSet::add(GDGraph *graph,
	   symbolPtr &symbol)
{
  Initialize(GDSet::add(GDGraph *, symbolPtr &));

  if (symbol.get_kind() == DD_PROJECT) {
    projNode *project = symbol.get_projNode();

    if (table_get_project(this) != project)
      return;

    projGraph_ = graph;
  } else {
    int index = set_.get_index(symbol);

    if (index < 0) {
      set_.insert_last(symbol);
      array_.append(&graph);
    } else
      *array_[index] = graph;
  }

  table_put_graph(this, graph);
}

void
GDSet::cleanTable(symbolPtr &symbol)
{
  Initialize(GDSet::cleanTable(symbolPtr &));
  symbolPtr temp;

  ForEachT(temp, set_) {
    int index = set_.get_index(temp);
    GDGraph *graph = *array_[index];

    if (graph) {
      symbolPtr fileSymbol;
      graph->getFileSymbol(fileSymbol);

      if (fileSymbol == symbol)
	graph->setFormat(GDF_ZOMBIE);
    }
  }
}

void
GDSet::cleanDataCells()
{
  Initialize(GDSet::cleanDataCells());
  unsigned int size = dataCells_.size();

  for (unsigned int k = 0; k < size; ++k) {
    dataCell *cell = (dataCell *) dataCells_[k];

    if (cell && !cell->get_graph()) {
      delete cell;
      dataCells_[k] = NULL;
    }
  }

  dataCells_.removeAll();
}

void
GDSet::loadStandardFunctions()
{
  Initialize(GDSet::loadStandardFunctions());

  if (GDSet::Hashtable_)
    return;

  GDSet::Hashtable_ = new GDSet();
  Interpreter *i = InitializeCLIInterpreter();

  if (i)
    i->SourceStandardScript("externFunctions.dis");
  else
    msg("No interpreter to source externFunctions.dis", warning_sev) << eom;
}

ofstream &
operator<<(ofstream &outStream,
	   GDSet *hashtable)
{
  Initialize(operator<<(ofstream &, GDSet *));

  unsigned char status = hashtable->status_;
  unsigned int size = hashtable->array_.size();
  unsigned int count = 0;

  if (hashtable->projGraph_)
    ++count;

  for (unsigned int k = 0; k < size; ++k) {
    GDGraph *graph = *(hashtable->array_)[k];

    if (graph && graph->getFormat() != GDF_ZOMBIE)
      ++count;
  }

  projNode *project = table_get_project(hashtable);

  if (count > 0) {
    msg("Saving the DFA information for the project $1", normal_sev)
	<< project->get_name() << eoarg << eom;

    unsigned char token = PMOD_DFA_TOKEN;
    outStream.write((char *) &token, sizeof(unsigned char));
    outStream.write((char *) &status, sizeof(unsigned char));

    GDInteger number = count;
    outStream.write((char *) &number, GDInteger::DB_SIZE());

    if (hashtable->projGraph_)
      outStream << hashtable->projGraph_;

    for (unsigned int m = 0; m < size; ++m) {
      GDGraph *graph = *(hashtable->array_)[m];

      if (graph && graph->getFormat() != GDF_ZOMBIE)
	outStream << graph;
    }
  }

  return outStream;
}

bool
GDSet::createStub(symbolPtr &symbol)
{
  Initialize(GDSet::createStub(symbolPtr &));

  if (symbol.isnull() || !is_model_build() || !GDSet::isDFApass())
    return false;

  if (!stubArr.includes(symbol)) {
    stubArr.insert_last(symbol);
    msg("The DFA information for $1 is not found. Generating a stub.", warning_sev) << symbol.get_name() << eoarg << eom;

    if (testName && strstr(symbol.get_name(), testName))
      dfa_debug_hook(testName);
  }

  return false;
}

void
GDSet::progressMessage()
{
  Initialize(GDSet::progressMessage());

  if (!GDSet::isDFApass() || GDSet::total_ == 0)
    return;

  unsigned int oldRatio = (100 * GDSet::count_) / GDSet::total_;
  unsigned int newRatio = (100 * ++GDSet::count_) / GDSet::total_;

  for (int k = 5 + DFAthreshold; k < 100; k += 5) {
    if (oldRatio < k && newRatio >= k) {
      time_t curr = (time_t)my_time();
      msg("PROGRESS ESTIMATE: $1LOCAL PHASE: Approximately $2% done.", normal_sev) << ctime(&curr) << eoarg << newRatio << eom;
      DFAthreshold = k;
      break;
    }
  }
}

unsigned int
DataSet::mergeRelations(GDGraph *graph,
			bool abstract,
			int mode)
{
  Initialize(DataSet::mergeRelations(GDGraph *, bool, int));

  if (!doMerge())
    return 0;

  unsigned int check = 0;
  GDGraph *func = call_get_graph(graph);

  if (!func && mode != 0)
    return 0;

  DataSet *firstBucket = (DataSet *) this;
  DataSet *secondBucket = (DataSet *) this;
  dataCell *cell = NULL;

  if (mode > 0) {
    cell = (dataCell *) dataCells_[0];

    if (cell) {
      secondBucket = func->find(cell);

      if (!secondBucket)
	return 0;
    }
  } else if (mode < 0) {
    cell = (dataCell *) dataCells_[0];

    if (cell) {
      firstBucket = func->find(cell);

      if (!firstBucket)
	return 0;
    }
  }

  GDSet *hashtable = graph->getHashtable(true);
  GDGraph *projGraph = hashtable->getProjGraph();
  unsigned int firstSize = (firstBucket->dataCells_).size();
  unsigned int subSize = dataSet_.size();
  unsigned int secondSize = 0;
  GDFlag flag = GDR_MERGE;

  for (unsigned int k = 0; k < firstSize; ++k) {
    dataCell *firstCell = (dataCell *) (firstBucket->dataCells_)[k];

    if (!firstCell || (firstCell->isAbstract() != abstract) ||
	!firstCell->isValid() || firstCell->isChecked())
      continue;

    if (projGraph == graph && firstCell->isCallDataCell())
      continue;

    if (projGraph == graph)
      secondSize = (secondBucket->dataCells_).size();
    else
      secondSize = (secondBucket->allCells_).size();

    bool firstPass = firstCell->pass();
    dataCell *secondCell = NULL;

    for (unsigned int m = 0; m < secondSize; ++m) {
      if (projGraph == graph)
	secondCell = (dataCell *) (secondBucket->dataCells_)[m];
      else
	secondCell = (dataCell *) (secondBucket->allCells_)[m];

      if (!secondCell || (!secondCell->isAbstract() && abstract) ||
	  !secondCell->isValid() || secondCell->isChecked())
	continue;

      bool secondPass = secondCell->pass();
      int adjustment = 0;

      if (!firstPass && !secondPass)
	continue;

      if (projGraph == graph &&
	  !(firstCell->get_type() & dataCell_Regular) &&
	  firstCell->isCallDataCell() == secondCell->isCallDataCell())
	continue;

      if (firstCell != secondCell) {
	if (!func && projGraph != graph &&
	    firstCell->isNodeDataCell() != secondCell->isNodeDataCell())
	  continue;
	else if (!firstCell->cellCompare2(secondCell, adjustment, true, false))
	  continue;
      }

      check += graph->iterationStep(firstCell, secondCell, adjustment,
				    flag | GDR_MERGE_STRAIGHT, abstract);

      if (firstCell != secondCell)
	check += graph->iterationStep(secondCell, firstCell, 0 - adjustment,
				      flag | GDR_MERGE_STRAIGHT, abstract);

      if (adjustment > 0) {
	check += graph->iterationStep(firstCell, secondCell, adjustment,
				      flag | GDR_MERGE_SOURCES, abstract);
	check += graph->iterationStep(secondCell, firstCell, 0 - adjustment,
				      flag | GDR_MERGE_TARGETS, abstract);
      } else if (adjustment < 0) {
	check += graph->iterationStep(secondCell, firstCell, 0 - adjustment,
				      flag | GDR_MERGE_SOURCES, abstract);
	check += graph->iterationStep(firstCell, secondCell, adjustment,
				      flag | GDR_MERGE_TARGETS, abstract);
      }
    }
  }

  for (unsigned int n = 0; n < subSize; ++n) {
    DataSet *bucket = *dataSet_[n];

    if (bucket)
      check += bucket->mergeRelations(graph, abstract, mode);
  }

  return check;
}

bool
DataSet::insert(dataCell *&cell,
		GDGraph *graph,
		bool clean)
{
  Initialize(DataSet::insert(dataCell *&, GDGraph *, bool));

  if (!cell)
    return false;

  bool mode = clean;
  DataSet *bucket = (DataSet *) this;
  const symbolPtr &cellSymbol = cell->get_symbol();
  GDGraph *func = call_get_graph(graph);

  if (flag_) {
    symbolPtr symbol = cellSymbol;
    dataCell *field = cell->getField();

    while (field && bucket) {
      symbol = field->get_symbol();
      bucket = bucket->find(symbol, true);
      field = field->getField();
    }

    if (cell->get_graph() == graph) {
      if (mode)
	delete cell;

      cell = NULL;
      return false;
    }
  }

  if (!bucket) {
    if (!flag_)
      return false;

    if (mode)
      delete cell;

    cell = NULL;
    return false;
  }

  bool comp = true;

  if (func)
    comp = false;

  for (int k = 0; k < bucket->dataCells_.size(); ++k) {
    dataCell *temp = (dataCell *) bucket->dataCells_[k];

    if (!temp)
      continue;

    if (cell == temp)
      return false;

    if (cell->get_maxLevel() == (int) DML_UNKNOWN &&
	cell->get_argno() == temp->get_argno() &&
	((cell->get_type() & dataCell_Mask1) ==
	 (temp->get_type() & dataCell_Mask1))) {
      symbolPtr tempType, cellType;
      temp->get_typeSymbol(tempType);
      cell->get_typeSymbol(cellType);
      int maxLevel = temp->get_maxLevel();
      int cellLevel = cell->get_maxLevel();

      if (!mode && ((maxLevel != cellLevel &&
		     maxLevel != (int) DML_UNKNOWN) ||
		    tempType != cellType)) {
	cell = cell->copy();
	mode = true;
      }

      cell->set_typeSymbol(tempType);

      if (maxLevel != (int) DML_UNKNOWN)
	cell->set_maxLevel(maxLevel);

      if (cell->isChecked()) {
	if (mode)
	  delete cell;

	cell = NULL;
	return false;
      }
    }

    if (cell->cellCompare(temp, comp)) {
      if (!flag_)
	return false;

      if ((cell->get_type() & dataCell_Constant) &&
	  !(temp->get_type() & dataCell_Constant))
	temp->add_type(dataCell_Constant);

      if (mode)
	delete cell;

      cell = temp;
      return false;
    }
  }

  if (!mode && flag_)
    cell = cell->copy();

  bucket->dataCells_.insert_last(cell);
  DataSet *temp = bucket;

  while (temp) {
    temp->allCells_.insert_last(cell);
    temp = temp->getParent();
  }

  if (!func && flag_ && !(cell->get_type() & dataCell_Regular)) {
    if (!cell->isNodeDataCell())
      bucket->defnCells_.insert_last(cell);
    else {
      int index = bucket->callSet_.get_index(cellSymbol);
      objArr *callArr = NULL;

      if (index < 0) {
	bucket->callSet_.insert_last(cellSymbol);
	callArr = new objArr();
	bucket->callDataSet_.append(&callArr);
      } else
	callArr = *(bucket->callDataSet_[index]);

      if (callArr)
	callArr->insert_last(cell);
    }
  }

  return true;
}

DataSet *
DataSet::find(symbolPtr &symbol,
	      bool mode)
{
  Initialize(DataSet::find(symbolPtr &, bool));

  DataSet *bucket = NULL;
  int index = indSet_.get_index(symbol);

  if (index < 0 && mode) {
    bucket = new DataSet(true);
    indSet_.insert_last(symbol);
    bucket->setParent(this);
    dataSet_.append(&bucket);
  } else if (index >= 0)
    bucket = *dataSet_[index];

  return bucket;
}

void
DataSet::getCallCells(const symbolPtr &symbol,
		      objArr &results) const
{
  Initialize(DataSet::getCallCells(const symbolPtr &, objArr &) const);
  int index = callSet_.get_index(symbol);

  if (index >= 0) {
    objArr *dataArr = *(callDataSet_[index]);

    if (dataArr)
      results.insert_all(*dataArr);
  }
}

void
DataSet::reshuffle(bool flag)
{
//  Initialize(DataSet::reshuffle(bool));

  if (flag)
    markOld_ = true;
  else
    markOld_ = markNew_;

  markNew_ = false;
  unsigned int size = dataSet_.size();

  for (unsigned int k = 0; k < size; ++k) {
    DataSet *bucket = *dataSet_[k];

    if (bucket)
      bucket->reshuffle(flag);
  }
}

DataSet::~DataSet()
{
  Initialize(DataSet::~DataSet());

  unsigned int size = dataSet_.size();
  unsigned int callSize = callDataSet_.size();

  for (unsigned int k = 0; k < size; ++k)
    delete *dataSet_[k];

  for (unsigned int m = 0; m < callSize; ++m)
    delete *callDataSet_[m];
}
