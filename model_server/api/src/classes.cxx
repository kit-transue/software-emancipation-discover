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
#include <iostream.h>
#else /* ISO_CPP_HEADERS */
#include <iostream>
using namespace std;
#endif /* ISO_CPP_HEADERS */

#include "Interpreter.h"
#include "symbolPtr.h"
#include "symbolSet.h"
#include "xref_queries.h"
#include "genArr.h"
#include "xrefSymbol.h"
#include "msg.h"

static void xref_symbol(symbolPtr& sym){
  Xref * xr = sym.get_xref();
  if(xr)
    sym.put_scope(xr);
}

genArr(int);
struct classDependency {
  intArr subs;
  intArr closure;
};

struct dependencyMask {
  typedef unsigned int word;
  enum {WS=32};  
  int data_size;
  word* data;
  void closure(int ii, classDependency * cladeparr);
  void set_data(int size, classDependency * cladeparr);
  int index(int i, int j) const { return data_size * i + j ;}

  bool get(int i, int j) const;
  void set(int i, int j);

  void setup(const symbolArr& arr, const symbolSet& set);

  dependencyMask() : data(0) {}
  ~dependencyMask() {if(data) delete data;}
};

void dependencyMask::set(int i, int j)
{
  int ind = index(i,j);
  data[ind/WS] |= (1U << (ind%WS));
}
bool dependencyMask::get(int i, int j) const
{
  int ind = index(i,j);
  return data[ind/WS] & (1U << (ind%WS));
}
void dependencyMask::set_data(int size, classDependency * cladeparr)
{
  if(data)
    delete data;

  data_size = size;

  int asz = size * size;
  asz = (asz+WS-1)/WS;  // bits
  data = new word[asz];
  for(int x=0; x<asz; ++x)
    data[x] = 0;

  for(int ii=0; ii<size; ++ii){
    classDependency& deparr = cladeparr[ii];
    int*dep = deparr.closure[0];
    int dep_size = deparr.closure.size();
    for(int kk=0; kk<dep_size; ++kk){
      int jj = dep[kk];
      set(ii,jj);
    }
  }
}

void dependencyMask::closure(int ii, classDependency * cladeparr)
{
  classDependency& dep = cladeparr[ii];
  intArr& cdp = dep.closure;
  if(cdp.size())
    return;
  cdp.append(&ii);
  intArr& subs = dep.subs;
  int sz = subs.size();
  for(int ind=0; ind<sz; ++ind){
    int jj = *subs[ind];
    closure(jj, cladeparr);
    intArr& closure = cladeparr[jj].closure;
    int* closure_ptr = closure[0];
    int closure_size = closure.size();
    int* more = cdp.grow(closure_size);
    for(int kk=0; kk<closure_size; ++kk)
      more[kk] = closure_ptr[kk];
  }
}

void dependencyMask::setup(const symbolArr&arr, const symbolSet&set)
{
  int size = arr.size();

  classDependency * cladeparr = new classDependency[size];
  int ii;
  for(ii=0; ii<size; ++ii){
    symbolPtr& cl = arr[ii];
    symbolArr subs;
    cl.get_link(has_subclass, subs);
    int subsize = subs.size();
    intArr& cdp = cladeparr[ii].subs;
    for(int si=0; si<subsize; ++si){
      int jj = set.get_index(subs[si]);
      if(jj >= 0)
	cdp.append(&jj);
    }
  }

  for(ii=0; ii<size; ++ii)
    closure(ii, cladeparr);

  set_data(size, cladeparr);
  delete [] cladeparr;
}

class  objInfo {
public:
  ddKind field_kind;
  symbolSet obj_defs;
  symbolArr obj_array;
  symbolSet obj_refs;
  intArr    def_of_ref;

  symbolSet fields;
  intArr fieldObjIndex;

  objInfo(ddKind kind) : field_kind(kind), obj_refs(1), fields(1) {}
  const char*name(int ind) const { return obj_array[ind].get_name();}
  bool  obj_of_field(const symbolPtr& field, symbolPtr& obj) const;
  int setup(const symbolArr&);
};

bool objInfo::obj_of_field(const symbolPtr&field, symbolPtr&cl) const
{
  int find = fields.get_index(field);
  if(find < 0)
    return false;

  int jj = *fieldObjIndex[find];
  if(jj < 0)
    return false;

  cl = obj_array[jj];
  return true;
}
char const *ATT_filename( symbolPtr& sym);

int  objInfo::setup(const symbolArr& clarr)
{
  fields.removeAll();
  fieldObjIndex.reset();

  int size = clarr.size();
  for(int ii=0; ii<size; ++ii){
    symbolPtr& cl = clarr[ii];
    symbolArr flarr;
    cl->get_link(is_using, flarr);
    int fsz = flarr.size();
    const char* cl_name = cl.get_name();
    const char* cl_file = NULL;
    if(cl_name[0]=='<')   //<unnamed> bug workaround
      cl_file = ATT_filename(cl);

    for(int ff=0;ff<fsz; ++ff){
      symbolPtr&field = flarr[ff];
      if(field.get_kind() == field_kind){
	if(cl_file){
	  const char* field_file = ATT_filename(field);
	  if(strcmp(cl_file, field_file) != 0)  // oops
	    continue;
	}
	symbolArr field_syms;
	field.get_all_symbols(field_syms, 0);
	int no_f = field_syms.size();
	for(int f_ind=0; f_ind<no_f; ++f_ind){
	  symbolPtr &field_sym = field_syms[f_ind];
	  xref_symbol(field_sym);
	  bool is_ins = fields.insert(field_sym);
	  int new_ind = fields.get_index(field_sym);
	  if(new_ind != fieldObjIndex.size()){
	    msg("mismatch: $1 $2", warning_sev)
		<< field.get_name() << eoarg
		<< clarr[*fieldObjIndex[new_ind]].get_name() << eoarg << eom;
	  } else {
	    fieldObjIndex.append(&ii);
	  }
	}
      }
    }
    symbolArr all_syms;
    cl.get_all_symbols(all_syms, 0);
    int no_sym = all_syms.size();
    for(int sind=0; sind<no_sym; ++sind){
      symbolPtr& cl_sym = all_syms[sind];
      xref_symbol(cl_sym);
      if(obj_refs.insert(cl_sym))
	def_of_ref.append(&ii);
      else 
	msg("ambiguous symbol: $1", warning_sev) << cl.get_name() << eoarg << eom;
    }
  }
  obj_defs = clarr;
  obj_array = clarr;

  return size;
}

class classInfo {
public:
  objInfo enums;
  objInfo classes;
  dependencyMask dependencies;

  classInfo() : enums(DD_ENUM_VAL), classes(DD_FIELD) {}
  int setup(const symbolArr& clarr);
  int setup();
  bool  class_of_field(const symbolPtr& field, symbolPtr& cl) const
    { return classes.obj_of_field(field, cl);}
  bool has_field(const symbolPtr& cl, const symbolPtr&field)  const;
  void print(ostream&);
};
void cli_get_roots(symbolArr &);
void cli_defines_or_uses(symbolArr&src_arr, symbolArr&result_arr, ddSelector&sel, int def_flag);

bool classInfo::has_field(const symbolPtr& cl, const symbolPtr&field) const
{
  int ii = classes.obj_refs.get_index(cl);
  if(ii<0)
    return false;
  ii = * classes.def_of_ref[ii];

  int find = classes.fields.get_index(field);
  if(find < 0)
    return false;

  int jj = *classes.fieldObjIndex[find];
  if(jj < 0)
    return false;

  return dependencies.get(jj, ii);
}

classInfo* classInfo_create()
{
  classInfo*ci = new classInfo;
  ci->setup();
  return ci;
}

bool classInfo_has_field(classInfo*ci, const symbolPtr& cl, const symbolPtr&field)
{
  return ci->has_field(cl, field);
}

static void get_all_classes_and_enums(symbolArr& classes)
{
  symbolArr roots;
  cli_get_roots(roots);
  ddSelector sel;
  sel.add(DD_CLASS);
//  sel.add(DD_ENUM);
  cli_defines_or_uses(roots, classes, sel, 1);
}

int  classInfo::setup()
{
  symbolArr clarr;
  get_all_classes_and_enums(clarr);
  return setup(clarr);
}

int  classInfo::setup(const symbolArr& arr)
{
  symbolArr cl, en;
  int sz = arr.size();
  for(int ii=0; ii<sz; ++ii){
    const symbolPtr& sym = arr[ii];
    ddKind kind = sym.get_kind();
    if(kind==DD_CLASS)
      cl.insert_last(sym);
    else if (kind==DD_ENUM)
      en.insert_last(sym);
  }
  int res = classes.setup(cl);
  res = enums.setup(en);
  dependencies.setup(classes.obj_array, classes.obj_defs);
  return res;
}

void classInfo::print(ostream& os) {
  int sz = classes.obj_defs.size();
  os << endl;
  for(int ii=0; ii<sz; ++ii){
    os << classes.name(ii) << ": ";
    for(int jj=0; jj<sz; ++jj){
      if(dependencies.get(ii,jj))
	os << classes.name(jj) << " ";
    }
    os << endl;
  }
}

#if 1
static classInfo class_info;
static int classtableCmd(Interpreter*i, int argc, char const **argv)
{
    symbolArr arr;
    if(i->ParseArguments(1, argc, argv, arr) != 0)
	return TCL_ERROR;
    if(arr.size())
      class_info.setup(arr);
    else
      class_info.setup();
    return TCL_OK;
}

static int classprintCmd(Interpreter*i, int argc, char const **argv)
{
    ostream& os = i->GetOutputStream();
    class_info.print(os);
    return TCL_OK;
}

static int fieldprintCmd(Interpreter*i, int argc, char const **argv)
{
#if 0 // 990615 kit transue.  Hasn't done anything since introduction in /main/8
    ostream& os = i->GetOutputStream();
    objInfo& classes = class_info.classes;
#endif
    return TCL_OK;
}

static int classfieldCmd(Interpreter*i, int argc, char const **argv)
{
  symbolArr arr;
  if(i->ParseArguments(1, argc, argv, arr) != 0)
    return TCL_ERROR;
  if(arr.size() != 2)
    return TCL_ERROR;
  
  bool answer = class_info.has_field(arr[0], arr[1]);
  Tcl_SetResult(i->interp, answer ? (char *)"1" : (char *)"0", TCL_VOLATILE);
  return TCL_OK;
}

static int classoffieldCmd(Interpreter*i, int argc, char const **argv)
{
  symbolArr arr;
  if(i->ParseArguments(1, argc, argv, arr) != 0)
    return TCL_ERROR;
  
  DI_object results;
  DI_object_create(&results);
  symbolArr & result_arr = *get_DI_symarr(results);
  
  symbolPtr cl;
  int sz = arr.size();
  for(int ii=0; ii<sz; ++ii) {
    bool answer = class_info.class_of_field(arr[ii], cl);
    if(answer)
      result_arr.insert_last(cl);
  }
  
  result_arr.unsort();
  i->SetDIResult(results);
  return TCL_OK;
}

static int add_commands()
{
  new cliCommand("classtable", classtableCmd);
  new cliCommand("superclassfield", classfieldCmd);
  new cliCommand("classprint", classprintCmd);
  new cliCommand("fieldprint", fieldprintCmd);
  return 0;
}
static int dummy = add_commands();
#endif
