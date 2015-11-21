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
#include "WfTask.h"
#include "machdep.h"
#include "msg.h"
#include "genArrCharPtr.h"
#include "genArrInt.h"
#include <TclList.h>
#ifndef ISO_CPP_HEADERS
#include <fstream.h>
#else /* ISO_CPP_HEADERS */
#include <fstream>
using namespace std;
#endif /* ISO_CPP_HEADERS */
#include "cLibraryFunctions.h"
#include "xxinterface.h"

init_relational(WfTask, objDictionary);
init_relational(WfTaskApp, externApp);
init_rel_or_ptr(WfTask, task, 0, WfTaskApp, app, 0);

#ifdef _WIN32
int wftask_commands_flag = 0;
#endif

bool extern_canonic_name(char const* name, genString& new_name);

//
// Static data:
//

WfTaskCatalog *WfTask::catalog = NULL;
char const *WfTask::main_script = "workflow.dis";

//
// CLASS: WfTask
//

WfTask::WfTask(char const *tag, objArr &arr, bool inst, WfTaskApp *ap)
  : objDictionary(tag), curnode(0)
{
  if (inst)
    ninst = arr;
  else {
    int sz = arr.size();
    for (int ii = 0; ii < sz; ii++) {
      WfNode *node = (WfNode *)(arr[ii]);
      WfNodeInstance *ni = new WfNodeInstance(node);
      ninst.insert_last(ni);
    }
  }

  AttachFrom(0);

//  if (Init())
//    catalog->Append(this);

//  if (!ap)
//    ap = (WfTaskApp *)WfTaskApp::create(tag, NULL);

  ap->set_status(externApp::NEW);
  task_put_app(this, (WfTaskApp *)ap);
}

WfTask::~WfTask()
{
  int sz = ninst.size();
  for (int ii = 0; ii < sz; ii++)
    delete (WfNodeInstance *)ninst[ii];

  // Delete the associated app:

  WfTaskApp *ap = task_get_app(this);
  if (ap) {
    externInfo *ti = ap->info();
    ti->app_delete(ap);
  }
}

bool WfTask::Init()
{
  static bool ret = false;

  if (!ret) {
    LoadMainScript();     // make sure standard node and param definitions are initialized
    if (!catalog) {
      catalog = new WfTaskCatalog;
      if (catalog)
        if (catalog->InitHooks()) {
          catalog->AppendHook("run", (AccessCatalog::HookMP)&WfTaskCatalog::RunCmd);
          catalog->AppendHook("addnode", (AccessCatalog::HookMP)&WfTaskCatalog::InsertCmd);
          catalog->AppendHook("delnode", (AccessCatalog::HookMP)&WfTaskCatalog::RemoveCmd);
          catalog->AppendHook("where", (AccessCatalog::HookMP)&WfTaskCatalog::WhereCmd);
          catalog->AppendHook("print", (AccessCatalog::HookMP)&WfTaskCatalog::PrintCmd);
          catalog->AppendHook("setparam", (AccessCatalog::HookMP)&WfTaskCatalog::SetParamCmd);
	  catalog->AppendHook("getparam", (AccessCatalog::HookMP)&WfTaskCatalog::GetParamCmd);
          catalog->AppendHook("save", (AccessCatalog::HookMP)&WfTaskCatalog::SaveCmd);
          catalog->AppendHook("dump", (AccessCatalog::HookMP)&WfTaskCatalog::DumpCmd);
          catalog->AppendHook("move", (AccessCatalog::HookMP)&WfTaskCatalog::MoveCmd);
          catalog->AppendHook("validate", (AccessCatalog::HookMP)&WfTaskCatalog::ValidateCmd);
          catalog->AppendHook("revive", (AccessCatalog::HookMP)&WfTaskCatalog::ReviveCmd);
          catalog->AppendHook("error_status", (AccessCatalog::HookMP)&WfTaskCatalog::ErrorStatusCmd);
          ret = true;
        }
    }
    else
      ret = true;
  }

  return ret;
}

void WfTask::GetErrorString(genString &str)
{
    str = error_string;
}

void WfTask::GetErrorInfo(genString &str)
{
    str = error_info;
}

int WfTask::Run(Interpreter *i, int from, int steps)
{
    int ret = TCL_OK;

    int end      = from + steps - 1;
    int sz       = ninst.size();
    error_string = "";
    for (curnode = from; (curnode <= end) && (curnode < sz); curnode++) {
	AttachParams(curnode);
	WfNodeInstance *ni = (WfNodeInstance *)(ninst[curnode]);
	ret = ni->Run(i);
	if (ret == TCL_ERROR){
	    ni->GetLastError(error_string, error_info);
	    break;
	}
    }

    if (curnode >= sz)
	curnode = 0;       // rewind the task after completion

    if (curnode > 0)
	AttachFrom(curnode);

    NotifyApp();

    return ret;
}

int WfTask::Run(Interpreter *i)
{
  int steps = ninst.size() - curnode + 1;
  return Run(i, curnode, steps);
}

int WfTask::RunFrom(Interpreter *i, int from)
{
  int steps = ninst.size() - from + 1;
  return Run(i, from, steps);
}

int WfTask::RunFor(Interpreter *i, int steps)
{
  return Run(i, curnode, steps);
}

void WfTask::AttachFrom(int from)
{
  if (from < 0)
    from = 0;
  int sz = ninst.size();
  for (int ii = from; ii < sz; ii++)
    AttachParams(ii);
}

void WfTask::AttachParams(int index)
{
  WfNodeInstance *ni = (WfNodeInstance *)(ninst[index]);
  Obj *el;
  objArr arr;
  arr.insert_all(ni->inputs);
  arr.insert_all(ni->optionals);
  ForEach (el, arr) {
    WfParamInstance *pi = MatchParam((WfParamInstance *)el, index);
    if (pi) {
      *((WfParamInstance *)el) = *pi;
      WfSymbolArr *arr = pi_get_symarr(pi);
      if (arr)
        pi_put_symarr(((WfParamInstance *)el), arr);
    }
  }
}

WfParamInstance *WfTask::MatchParam(WfParamInstance *inst, int index)
{
  WfParamInstance *ret = NULL;

  WfParam *param = instance_get_param(inst);
  for (int ii = index - 1; (ii >= 0) && (!ret); ii--) {
    WfNodeInstance *ni = (WfNodeInstance *)(ninst[ii]);
    int tmatch = 0;
    WfParamInstance *tpi = NULL;
    objArr arr;
    int mc = param->mc;
    if (mc & WfParam::M_OUT)
      arr.insert_all(ni->outputs);
    if (mc & WfParam::M_INP)
      arr.insert_all(ni->inputs);
    bool mtag = (mc & WfParam::M_TAG) ? true : false;
    Obj *el;
    ForEach (el, arr) {
      WfParamInstance *pi = (WfParamInstance *)el;
      WfParam *pp = instance_get_param(pi);
      if (pp == param) {
        ret = pi;
	break;
      }
      else if (pp->vt->is(param->vt)) {
        tmatch++;
	if (!tpi)
	  tpi = pi;
      }
    }
    if ((tmatch == 1) && !ret && !mtag)   // found one param that has the same type, but is not identical
      ret = tpi;
  }

  return ret;
}

int WfTask::NormalizePos(int pos)
{
  int ret = pos;

  int sz = ninst.size();
  if (pos > sz)
    ret = sz - 1;
  if (pos < -1)
    ret = -1;

  return ret;
}

void WfTask::Insert(int pos, objArr &arr)
{
  pos = NormalizePos(pos);

  int sz = arr.size();
  for (int ii = sz - 1; ii >= 0; ii--) {
    WfNode *node = (WfNode *)arr[ii];
    WfNodeInstance *ni = new WfNodeInstance(node);
    ninst.insert_after(pos, ni);
    if (pos < curnode)
      curnode++;
  }

  NotifyApp();

  AttachFrom(pos + 1);    // Re-hitch the wagon, so to speak
}

#ifdef min
#  undef min
#endif /* min */
#define min(a, b) ((a) > (b) ? (b) : (a))

void WfTask::Move(int from, int to)
{
  from = NormalizePos(from);
  if (from < 0) from = 0;
  to = NormalizePos(--to);   // insert *after* this position

  WfNodeInstance *ni = (WfNodeInstance *)ninst[from];
  ninst.remove(from);
#if 0
  if (from < to)
    to = NormalizePos(++to);
#endif /* 0 */
  ninst.insert_after(to, ni);

  NotifyApp();

  AttachFrom(min(from, to));
}

void WfTask::Nullify()
{
  curnode = 0;
  ninst.removeAll();     // don't delete elements!
}

void WfTask::Revive()
{
  int sz = ninst.size();
  for (int ii = 0; ii < sz; ii++) {
    WfNodeInstance *ni = (WfNodeInstance *)ninst[ii];
    objArr parr;
    parr.insert_all(ni->inputs);
    parr.insert_all(ni->optionals);
    parr.insert_all(ni->outputs);
    Obj *el;
    ForEach (el, parr) {
      ((WfParamInstance *)el)->Revive();
    }
  }
}

bool WfTask::Validate()
{
  bool ret = false;

  bool ok = true;
  int sz = ninst.size();
  for (int ii = 0; ii < sz; ii++) {
    WfNodeInstance *ni = (WfNodeInstance *)(ninst[ii]);
    Obj *el;
    ForEach (el, ni->inputs) {
      WfParamInstance *pi = (WfParamInstance *)el;
      WfParamInstance *mpi = MatchParam(pi, ii);
      if (!((pi->Ready()) || mpi)) {
        ok = false;
        break;
      }
    }
  }

  if (ok)
    ret = true;

  return ret;
}

void WfTask::Remove(int pos)
{
  pos = NormalizePos(pos);

  WfNodeInstance *ni = (WfNodeInstance *)ninst[pos];
  ninst.remove(pos);
  delete ni;
  if (pos < curnode)
    curnode--;
  else if (pos == curnode) {
    int sz = ninst.size();
    if (curnode + 1 > sz)
      curnode = sz;
    else
      curnode++;
  }

  NotifyApp();

  AttachFrom(pos);
}

int WfTask::CurrentPosition()
{
  return curnode;
}

bool WfTask::ValidNodeIndex(int index)
{
  int sz = ninst.size();
  if ((index >= 0) && (index < sz))
    return true;
  else
    return false;
}

void WfTask::Print(TclList &list, genArrOf(int) &indices, WfPrintCode pc, bool recurse)
{
  int sz = indices.size();
  for (int ii = 0; ii < sz; ii++) {
    WfNodeInstance *ni = (WfNodeInstance *)ninst[*(indices[ii])];
    ni->Print(list, pc, recurse, true);
  }
}

void WfTask::Print(TclList &list, WfPrintCode pc, bool recurse)
{
  int sz = ninst.size();
  for (int ii = 0; ii < sz; ii++) {
    WfNodeInstance *ni = (WfNodeInstance *)ninst[ii];
    ni->Print(list, pc, recurse, true);
  }
}

bool WfTask::SetNodeParam(int nindex, char const *pname, char const *val)
{
  bool ret = false;

  objArr piarr;
  GetNodeParam(nindex, pname, piarr);
  int sz = piarr.size();
  for (int ii = 0; ii < sz; ii++) {
    WfParamInstance *pi = (WfParamInstance *)piarr[ii];
    pi->SetVal(val);
  }

  if (sz > 0)
    ret = true;

  NotifyApp();

  AttachFrom(nindex);

  return ret;
}

void WfTask::GetNodeParam(int nindex, char const *pname, objArr &piarr)
{
  int sz = ninst.size();
  if (nindex < sz && nindex >= 0) {
    WfNodeInstance *ni = (WfNodeInstance *)ninst[nindex];
    objArr arr;
    arr.insert_all(ni->inputs);
    arr.insert_all(ni->outputs);
    arr.insert_all(ni->optionals);
    Obj *el;
    ForEach (el, arr) {
      WfParam *param = instance_get_param((WfParamInstance *)el);
      if (param && (!strcmp(param->get_name(), pname)))
        piarr.insert_last((WfParamInstance *)el);
    }
  }
}

bool WfTask::PrintNodeParam(int nindex, char const *pname, TclList &list)
{
  bool ret = false;

  objArr piarr;
  GetNodeParam(nindex, pname, piarr);
  int sz = piarr.size();
  for (int ii = 0; ii < sz; ii++) {
    WfParamInstance *pi = (WfParamInstance *)piarr[ii];
    pi->Print(list, WF_DEFAULT, NULL, true);
  }

  if (sz > 0)
    ret = true;

  return ret;
}

void WfTask::NotifyApp()
{
  WfTaskApp *ap = task_get_app(this);

  if (ap)
    ap->set_status(externApp::MODIFIED);
}

bool WfTask::LoadParamDefs(objArr &arr, TclList &list)
{
  bool ret = true;

  int sz = list.Size();
  for (int ii = 0; ii < sz; ii++) {
    TclList def(list[ii]);
    WfParam *param = WfParam::Find(def[0]);
    if (param) {
      WfParamInstance *pi = new WfParamInstance(param);
      char const *init = def[2];
      if (*init == '1')
        pi->SetVal(def[3]);
      arr.insert_last(pi);
    }
    else
      { ret = false; break; }
  }

  return ret;
}

bool WfTask::LoadNodeDefs(objArr &arr, TclList &list)
{
  bool ret = true;

  int sz = list.Size();
  for (int ii = 0; ii < sz; ii++) {
    TclList def(list[ii]);
    WfNode *node = WfNode::Find(def[0]);
    if (node) {
      TclList in (def[1]);
      TclList out(def[2]);
      TclList opt(def[3]);
      bool valid = false;
      objArr inputs, outputs, optionals;
      if (LoadParamDefs(inputs, in))
        if (LoadParamDefs(outputs, out))
          if (LoadParamDefs(optionals, opt))
	    valid = true;
      if (valid) {
        WfNodeInstance *ni = new WfNodeInstance(node, inputs, outputs, optionals);
        arr.insert_last(ni);
      }
      else
        { ret = false; break; }
    }
    else 
      { ret = false; break; }
  }

  return ret;
}

bool WfTask::Load(WfTaskApp *ap)
{
  bool ret = false;

  char const *name = ap->get_name();
  char const *fname = OSPATH(ap->get_phys_name());

  msg("STDERR: Loading task: \"$1\"...") << name << eom;

  if (fname && fname[0]) {

    // Using ifstream gives us unpredictable results on NT,
    // so we've switched to the C-lib routines.

    FILE *f = fopen(fname, "r");
    if (f) {
      struct OStype_stat st;
      OSapi_stat(fname, &st);
      int fsize = (int)st.st_size;
      char *buf = new char[fsize + 1];
      int sz = OSapi_fread(buf, 1, fsize, f);
      buf[sz] = '\0';
      TclList list(buf);
      delete buf;
      OSapi_fclose(f);
      bool valid = false;
      if (list.Validate()) {
	LoadMainScript();
        objArr narr;
	if (LoadNodeDefs(narr, list)) {
          WfTask *task = new WfTask(name, narr, true, ap);
	  if (task)
	    valid = true;
        }
      }
      if (valid)
        ret = true;
      else
        msg("STDERR: Cannot load task definition file: \"$1\".") << fname << eom;
    }
    else
      msg("STDERR: Failed to open file: \"$1\".") << fname << eom;
  }
  else {
    char const *lname = ap->get_filename();
    msg("ERROR: No mapping for $1 in project $2") << lname << eoarg << "task" << eom;
  }

  return ret;
}

extern Interpreter *interpreter_instance;

int WfTask::LoadMainScript()
{
  static int ret = -1;

  if (ret == -1) {
    // This will only be executed once per session:
    if ((interpreter_instance != NULL) &&
        (interpreter_instance->SourceStandardScript(main_script) == TCL_OK))
      ret = 1;
    else
      ret = 0;
  }

  return ret;
}

bool WfTask::Save(WfTaskApp *ap)
{
  bool ret = false;

  char const *name = ap->get_name();
  char const *fname = OSPATH(ap->get_phys_name());

  msg("STDERR: Saving task: \"$1\"...") << name << eom;

  if (fname && fname[0]) {
    WfTask *task = app_get_task(ap);
    if (task) {
      genArrCharPtr vec;
      task->CycleVals(vec, true);
      WfTask task_copy = *task;
      task_copy.Consolidate(ap);
      ofstream os(fname, ios::out);
      if (!os.bad()) {
        TclList list;
        task_copy.Print(list, WF_DEFAULT, true);
	os << (char const *)list;
	if (!os.bad()) {
          ap->set_status(externApp::SAVED);
          ret = true;
        }
      }
      else
        msg("STDERR: Failed to open file: \"$1\".") << fname << eom;      
      task_copy.Nullify();  // make sure we don't delete the node instances in dtor
      task->CycleVals(vec, false);
    }
  }
  else {
    char const *lname = ap->get_filename();
    msg("ERROR: No mapping for $1 in project $2") << lname << eoarg << "task" << eom;
  }

  return ret;
}

void WfTask::CycleVals(genArrCharPtr &vec, bool backup)
{
  int npi = 0;
  int sz = ninst.size();
  for (int ii = 0; ii < sz; ii++) {
    WfNodeInstance *ni = (WfNodeInstance *)(ninst[ii]);
    objArr arr;
    arr.insert_all(ni->inputs);
    arr.insert_all(ni->outputs);
    arr.insert_all(ni->optionals);
    int ssz = arr.size();
    for (int jj = 0; jj < ssz; jj++, npi++) {
      WfParamInstance *pi = (WfParamInstance *)(arr[jj]);
      WfParam *p          = instance_get_param(pi);
      if (backup) {
        if (pi->Ready()) {
	     if (p && (p->vt->btype() == WfValueType::SET)){
	         char const *ptr = (char *)pi->GetDIVal();
		 vec.append(&ptr);
	     } else {
		 char const *val = pi->GetStrVal();
		 char *str = new char[OSapi_strlen(val) + 1];
		 char *p   = str;
		 do { *p++ = *val; } while (*val++);
		 vec.append(&str);
	     }
        }
        else {
	    char *str = new char[1];
	    *str      = '\0';
	    vec.append(&str);
        }
      }
      else {
	  char const **vp = vec[npi];
	  if (vp) {
	      if (p && (p->vt->btype() == WfValueType::SET)){
		  char const *ptr = *vp;
                  if(*ptr)
                     pi->SetDIVal((DI_object)ptr);
	      } else {
		  char const *v = *vp;
		  if (*v)
		      pi->SetValLiteral(v);
		  delete[] v;
	      }
	  }
      }
    }
  }
}

void WfTask::Consolidate(WfTaskApp *ap)
{
    int sz = ninst.size();
    for (int ii = 0; ii < sz; ii++) {

	WfNodeInstance *ni = (WfNodeInstance *)ninst[ii];

	// Unset those params which we can derive:

	{
	    Obj *el;
	    objArr arr;
	    arr.insert_all(ni->inputs);
	    arr.insert_all(ni->optionals);
	    ForEach (el, arr) {
		WfParamInstance *pi = (WfParamInstance *)el;
		WfParamInstance *mpi = MatchParam(pi, ii);
		if (mpi)
		    if (WfParamInstance::Equal(pi, mpi))
			pi->Unset();
	    }
	}

	// Convert the object tags to entity lists:

	objArr arr;
	arr.insert_all(ni->inputs);
	arr.insert_all(ni->outputs);
	arr.insert_all(ni->optionals);
	Obj *el;
	ForEach (el, arr) {
	    WfParamInstance *pi = (WfParamInstance *)el;
	    WfParam *param = instance_get_param(pi);
	    if (pi->Ready() && param && param->vt && (param->vt->btype() == WfValueType::SET)) {
		WfSymbolArr *warr = pi_get_symarr(pi);
		if (warr) {
		    symbolArr *arr = warr->arr;
		    if(arr) {
			genString ext_group_name = "";
			if(arr->size() > 0)  {
			    WfNode *node = instance_get_node(ni);
			    if(ap && node) {
				genString group_name;
				//WfTaskApp *task = task_get_app(this);
				group_name = ap->get_name();
				group_name += ".";
				group_name += node->get_name();
				group_name += ".";
				group_name += param->get_name();
				genString new_name;
				if(extern_canonic_name(group_name, new_name))
				  group_name = new_name;
				extGroupApp* app = extGroupApp::get(group_name);
				if(!app) {
				    symbolArr *garr = new symbolArr; 
				    app             = (extGroupApp*) extGroupApp::create(group_name, garr);
				    *garr           = *arr;
				} else {
				    symbolArr *garr = app->load();
				    *garr           =  *arr;
				    app->set_status(externApp::NEW);
				}
				app->externSave();
				ext_group_name = "group@";
				ext_group_name += group_name;	
			    } 
			} 
			pi->SetValLiteral(ext_group_name.str());
		    }
		}
	    }
	}
    }
}

void WfTask::SearchAndReplace(WfValueType::CODE code, char const *oldval, genString &newval, objArr &piarr)
{
    int sz = ninst.size();
    for (int ii = 0; ii < sz; ii++) {
	WfNodeInstance *ni = (WfNodeInstance *)ninst[ii];
	objArr arr;
	arr.insert_all(ni->inputs);
	arr.insert_all(ni->outputs);
	arr.insert_all(ni->optionals);
	Obj *el;
	ForEach (el, arr) {
	    WfParamInstance *pi = (WfParamInstance *)el;
	    WfParam *param = instance_get_param(pi);
	    if (param && param->vt && (param->vt->btype() == code)) {
		if (OSapi_strcmp((char *)pi->GetStrVal(), (char *)oldval)) {
		    pi->SetVal(newval);
		    piarr.insert_last(pi);
		}
	    }
	}
    }
}

void WfTask::print(ostream& os, int) const
{
  os << get_name(); 
}

//
// CLASS: WfTaskApp
//

WfTaskApp::WfTaskApp(char const *name, char const *fname, extStatus st)
  : externApp(name, fname, st)
{
  info()->get_apps()->insert(this);
}

externApp *WfTaskApp::create(char const *name, externFile *extf)
{
  char const *phys_filename;
  genString fn;

  if (extf) {
    phys_filename = extf->get_phys_filename();
  }
  else {
    externInfo *ti = type_info();
    ti->new_fname(name, fn);
    phys_filename = fn.str();
  }
  WfTaskApp *ap = new WfTaskApp(name, phys_filename);
  ap->make_savable();

  if(WfTask::Init())
    WfTask::catalog->Append(ap);

//  if (pm)
//    WfTask::Load(ap);

  return ap;
}

externInfo *WfTaskApp::type_info()
{
  static externInfo *inf = new externInfo(WfTaskApp::create, "tsk", "task", "ext");
  return inf;
}

externInfo *WfTaskApp::info() const
{
  return type_info();
}

bool WfTaskApp::externSave()
{
  bool ret = false;

  WfTask *task = app_get_task(this);
  if (task) {
    if (task->Save(this))
      ret = true;
  }

  return ret;
}

#define ifeq(x, y) if (!OSapi_strcmp(x, y))

//
// CLASS: WfTaskCatalog
//

Relational *WfTaskCatalog::Find(char const *name)
{
  WfTaskApp *app = (WfTaskApp *)AccessCatalog::Find(name);
  if(app){
    WfTask *tsk = app_get_task(app);
    if(tsk == NULL)
      WfTask::Load(app);
    tsk = app_get_task(app);
    return tsk;
  } else 
    return NULL;
}

int WfTaskCatalog::AppendItemCmd(int argc, char const *argv[], genString &result)
{
  int ret = TCL_ERROR;

  result = "";

  char const *tag = NULL;
  genArrCharPtr nodes;

  if (argc >= 3) {
    tag = argv[2];
    for (int ii = 3; ii < argc; ii++)
      nodes.append(&(argv[ii]));
    if (!Find(tag)) {
      genArrCharPtr bad;
      objArr _nodes;
      AccessCatalog::FillArr(WfNode::catalog, _nodes, nodes, bad);
      if (!bad.size()) {
	WfTaskApp *ap = (WfTaskApp *)WfTaskApp::create(tag, NULL);
	WfTask *task  = new WfTask(tag, _nodes, true, ap);
	if (task) {
	  result = tag;
          ret = TCL_OK;
	}
        else
          result.printf("%s: error: allocation failed.", argv[0]);
      }
      else
        result.printf("%s: error: unknown item: \"%s\".", argv[0], *(bad[0]));
    }
    else
      result.printf("%s: error: name already in use: \"%s\".", argv[0], tag);
  }
  else
    result.printf("usage: %s %s %s", argv[0], argv[1], "tag [ node1 node2 ... ]");

  return ret;
}

int WfTaskCatalog::SaveCmd(int argc, char const *argv[], genString &result)
{
  int ret = TCL_ERROR;

  result = "";

  if (argc == 3) {
    char const *tag = argv[2];
    WfTask *task = (WfTask *)Find(tag);
    if (task) {
      WfTaskApp *ap = task_get_app(task);
      if (ap) {
        if (ap->externSave())
          ret = TCL_OK;
      }
    }
    else
      result.printf("%s: error: unknown item: \"%s\".", argv[0], tag);
  }
  else
    result.printf("usage: %s %s %s", argv[0], argv[1], "tag");


  return ret;
}

int WfTaskCatalog::RemoveCmd(int argc, char const *argv[], genString &result)
{
  int ret = TCL_ERROR;

  result = "";

  if (argc >= 4) {
    char const *tag = argv[2];
    char const *pos = argv[3];
    WfTask *task = (WfTask *)Find(tag);
    if (task) {
      int index = OSapi_atoi(pos);
      if (task->ValidNodeIndex(index)) {
        task->Remove(index);
        ret = TCL_OK;
      }
      else
        result.printf("%s: error: index out of range: %d.", argv[0], index);
    }
    else
      result.printf("%s: error: unknown item: \"%s\".", argv[0], tag);
  }
  else
    result.printf("usage: %s %s %s", argv[0], argv[1], "tag pos");

  return ret;
}

int WfTaskCatalog::ReviveCmd(int argc, char const *argv[], genString &result)
{
  int ret = TCL_ERROR;

  result = "";

  if (argc >= 3) {
    bool valid = true;
    objArr tarr;
    for (int ii = 2; ii < argc; ii++) {
      char const *tag = argv[ii];
      WfTask *task = (WfTask *)Find(tag);
      if (task) {
        tarr.insert_last(task);
      }
      else {
	result.printf("%s: error: unknown item: \"%s\".", argv[0], tag);
        valid = false;
	break;
      }
      if (valid) {
        Obj *el;
	ForEach (el, tarr) {
          ((WfTask *)el)->Revive();
        }
        ret = TCL_OK;
      }
    }
  }
  else
    result.printf("usage: %s %s %s", argv[0], argv[1], "tag1 [ tag2 tag3 ... ]");

  return ret;
}

int WfTaskCatalog::ErrorStatusCmd(int argc, char const *argv[], genString &result)
{
    int ret = TCL_ERROR;

    result = "";

    if (argc >= 3) {
	bool valid = true;
	objArr tarr;
	for (int ii = 2; ii < argc; ii++) {
	    char const *tag = argv[ii];
	    WfTask *task = (WfTask *)Find(tag);
	    if (task) {
		tarr.insert_last(task);
	    }
	    else {
		result.printf("%s: error: unknown item: \"%s\".", argv[0], tag);
		valid = false;
		break;
	    }
	    if (valid) {
		Obj *el;
		ForEach (el, tarr) {
		    ((WfTask *)el)->GetErrorString(result);
		}
		ret = TCL_OK;
	    }
	}
    }
    else
	result.printf("usage: %s %s %s", argv[0], argv[1], "tag1 [ tag2 tag3 ... ]");
    return ret;
}

int WfTaskCatalog::ValidateCmd(int argc, char const *argv[], genString &result)
{
  int ret = TCL_ERROR;

  result = "";

  if (argc == 3) {
    char const *tag = argv[2];
    WfTask *task = (WfTask *)Find(tag);
    if (task) {
      if (task->Validate())
        result = "1";
      else
	result = "0";
      ret = TCL_OK;
    }
    else
      result.printf("%s: error: unknown item: \"%s\".", argv[0], tag);
  }
  else
    result.printf("usage: %s %s %s", argv[0], argv[1], "tag");

  return ret;
}

int WfTaskCatalog::MoveCmd(int argc, char const *argv[], genString &result)
{
  int ret = TCL_ERROR;

  result = "";

  if (argc == 5) {
    char const *tag = argv[2];
    WfTask *task = (WfTask *)Find(tag);
    if (task) {
      int from = OSapi_atoi(argv[3]);
      int to = OSapi_atoi(argv[4]);
      task->Move(from, to);
      ret = TCL_OK;
    }
    else
      result.printf("%s: error: unknown item: \"%s\".", argv[0], tag);
  }
  else
    result.printf("usage: %s %s %s", argv[0], argv[1], "tag from to");

  return ret;
}

int WfTaskCatalog::InsertCmd(int argc, char const *argv[], genString &result)
{
  int ret = TCL_ERROR;

  result = "";

  if (argc >= 5) {
    char const *tag = argv[2];
    char const *pos = argv[3];
    genArrCharPtr nodes;
    for (int ii = 4; ii < argc; ii++)
      nodes.append(&(argv[ii]));
    WfTask *task = (WfTask *)Find(tag);
    if (task) {
      genArrCharPtr bad;
      objArr _nodes;
      AccessCatalog::FillArr(WfNode::catalog, _nodes, nodes, bad);
      if (!bad.size()) {
	task->Insert(OSapi_atoi(pos), _nodes);
        ret = TCL_OK;
      }
      else
        result.printf("%s: error: unknown item: \"%s\".", argv[0], *(bad[0]));
    }
    else
      result.printf("usage: %s: error: unknown item: \"%s\".", argv[0], tag);
  }
  else {
    // Yes, the usage syntax is a little different here; since these commands
    // will now only be used internally, we'll use positional parameters for
    // convenience sake (the caller has to do less work).
    result.printf("usage: %s %s %s", argv[0], argv[1], "tag pos node1 [ node2 node3 ... ]");
  }

  return ret;
}

int WfTaskCatalog::RunCmd(int argc, char const *argv[], genString &result)
{
  int ret = TCL_ERROR;

  result = "";

  bool valid = true;
  char const *tag = NULL;
  char const *from = NULL;
  char const *num = NULL;

  for (int ii = 2; ii < argc; ii++) {
    ifeq (argv[ii], "-from")
      if (!from)
        if (ii < argc - 1)
          from = argv[++ii];
        else
          { valid = false; break; }
      else
        { valid = false; break; }
    else ifeq (argv[ii], "-n")
      if (!num)
        if (ii < argc - 1)
          num = argv[++ii];
        else
          { valid = false; break; }
      else
        { valid = false; break; }
    else if (!tag)
      tag = argv[ii];
    else
      { valid = false; break; }
  }

  if (tag && valid) {
    WfTask *task = (WfTask *)(Find(tag));
    if (task) {
      if (from && num)
        ret = task->Run(interpreter, OSapi_atoi(from), OSapi_atoi(num));
      else if (from)
        ret = task->RunFrom(interpreter, OSapi_atoi(from));
      else if (num)
	ret = task->RunFor(interpreter, OSapi_atoi(num));
      else
        ret = task->Run(interpreter);
    }
    else
      result.printf("%s: error: unknown item: \"%s\".", argv[0], tag);
  }
  else
    result.printf("usage: %s %s %s", argv[0], argv[1], "tag [ -from pos ] [ -n steps ]");

  return ret;
}

int WfTaskCatalog::WhereCmd(int argc, char const *argv[], genString &result)
{
  int ret = TCL_ERROR;

  result = "";

  if (argc == 3) {
    char const *tag = argv[2];
    WfTask *task = (WfTask *)(Find(tag));
    if (task) {
      int where = task->CurrentPosition();
      result.printf("%d", where);
      ret = TCL_OK;
    }
    else
      result.printf("%s: error: unknown item: \"%s\".", argv[0], tag);
  }
  else
    result.printf("usage: %s %s %s", argv[0], argv[1], "tag");

  return ret;
}

int WfTaskCatalog::GetParamCmd(int argc, char const *argv[], genString &result)
{
  int ret = TCL_ERROR;

  result = "";

  bool valid = true;
  char const *tag = NULL;
  char const *nidp = NULL;
  char const *pname = NULL;

  for (int ii = 2; ii < argc; ii++) {
    ifeq (argv[ii], "-n")
      if (!nidp)
        if (ii < argc - 1)
          nidp = argv[++ii];
        else
          { valid = false; break; }
      else
        { valid = false; break; }
    else ifeq (argv[ii], "-p")
      if (!pname)
        if (ii < argc - 1)
          pname = argv[++ii];
        else
          { valid = false; break; }
      else
        { valid = false; break; }
    else if (!tag)
      tag = argv[ii];
    else
      { valid = false; break; }
  }

  if (valid && tag && nidp && pname) {
    WfTask *task = (WfTask *)(Find(tag));
    if (task) {
      int nid = OSapi_atoi(nidp);
      TclList list;
      if (task->PrintNodeParam(nid, pname, list)) {
        ret = TCL_OK;
        result = list;
      }
      else
        result.printf("%s: error: invalid node/param pair: %d/\"%s\".", nid, pname);
    }
    else
      result.printf("%s: error: unknown item: \"%s\".", argv[0], tag);  
  }
  else
    result.printf("usage: %s %s %s", argv[0], argv[1], "tag -n nindex -p pname");

  return ret;
}

int WfTaskCatalog::SetParamCmd(int argc, char const *argv[], genString &result)
{
  int ret = TCL_ERROR;

  result = "";

  bool valid = true;
  char const *tag = NULL;
  char const *nidp = NULL;
  char const *pname = NULL;
  char const *val = NULL;

  for (int ii = 2; ii < argc; ii++) {
    ifeq (argv[ii], "-n")
      if (!nidp)
        if (ii < argc - 1)
          nidp = argv[++ii];
        else
          { valid = false; break; }
      else
        { valid = false; break; }
    else ifeq (argv[ii], "-p")
      if (!pname)
        if (ii < argc - 1)
          pname = argv[++ii];
        else
          { valid = false; break; }
      else
        { valid = false; break; }
    else ifeq (argv[ii], "-v")
      if (!val)
        if (ii < argc - 1)
          val = argv[++ii];
        else
          { valid = false; break; }
      else
        { valid = false; break; }
    else if (!tag)
      tag = argv[ii];
    else
      { valid = false; break; }
  }

  if (valid && tag && nidp && pname && val) {
    WfTask *task = (WfTask *)(Find(tag));
    if (task) {
      int nid = OSapi_atoi(nidp);
      if (task->SetNodeParam(nid, pname, val))
        ret = TCL_OK;
      else
        result.printf("%s: error: invalid node/param pair: %d/\"%s\".", argv[0], nid, pname);
    }
    else
      result.printf("%s: error: unknown item: \"%s\".", argv[0], tag);
  }
  else
    result.printf("usage: %s %s %s", argv[0], argv[1], "tag -n nindex -p pname -v value");

  return ret;
}

int WfTaskCatalog::DumpCmd(int argc, char const *argv[], genString &result)
{
  int ret = TCL_ERROR;

  result = "";

  if (argc >= 3) {
    char const *tag = argv[2];
    WfTask *task = (WfTask *)(Find(tag));
    if (task) {      
      genArrOf(int) indices;
      bool ok = true;
      for (int ii = 3; ii < argc; ii++) {
        int index = OSapi_atoi(argv[ii]);
	if (task->ValidNodeIndex(index)) 
          indices.append(&index);
	else {
	  result.printf("%s: error: index out of range: %d.", argv[0], index);
          ok = false;
	  break;
	}
      }
      if (ok) {
	TclList list;
        if (indices.size() > 0)
          task->Print(list, indices, WF_DUMP, true);
        else
          task->Print(list, WF_DUMP, true);
	result = list;
	ret = TCL_OK;
      }
    }
    else
      result.printf("%s: error: unknown item: \"%s\".", argv[0], tag);
  }
  else
    result.printf("usage: %s %s %s", argv[0], argv[1], "tag [ node1 node2 ... ]");

  return ret;
}


int WfTaskCatalog::PrintCmd(int argc, char const *argv[], genString &result)
{
  int ret = TCL_ERROR;

  result = "";

  bool valid = true;
  bool recurse = false;
  char const *tag = NULL;
  genArrOf(int) indices;

  for (int ii = 2; ii < argc; ii++) {
    ifeq (argv[ii], "-r")
      if (!recurse)
        recurse = true;
      else
        { valid = false; break; }
    else ifeq (argv[ii], "-n")
      if (ii < argc - 1) {
        int index = OSapi_atoi(argv[++ii]);
        indices.append(&index);
      }
      else
        { valid = false; break; }
    else if (!tag)
      tag = argv[ii];
    else
      { valid = false; break; }
  }

  if (valid && tag) {
    WfTask *task = (WfTask *)(Find(tag));
    if (task) {
      TclList list;
      bool ok = true;
      int sz = indices.size();
      if (sz > 0) {
        for (int ii = 0; ii < sz; ii++) {
	  int index = *(indices[ii]);
          if (!task->ValidNodeIndex(index)) {
	    result.printf("%s: error: index out of range: %d.", argv[0], index);
	    ok = false;
            break;
	  }
        }
        if (ok)
          task->Print(list, indices, WF_DEFAULT, recurse);
      }
      else
        task->Print(list, WF_DEFAULT, recurse);
      if (ok) {
        result = list;
	ret = TCL_OK;
      }
    }
    else
      result.printf("%s: error: unknown item: \"%s\".", argv[0], tag);
  }
  else
    result.printf("usage: %s %s %s", argv[0], argv[1], "tag [ -r ] [ -n node1 -n node2 ... ]");

  return ret;
}

int wf_task_cmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[])
{
  int    ret                  = TCL_ERROR;
  static int has_flow_license = 0;

  if(has_flow_license == 0){
      if(_lo(LIC_TASKFLOW)){    // check out
	  Tcl_SetResult(interp, (char *)"no license", TCL_VOLATILE);
	  if(strcmp(argv[1], "info"))
	      _lm(LIC_TASKFLOW);    // error msg  
	  return TCL_ERROR;
      } else 
	  has_flow_license = 1;
  }
  externInfo::load();

  if (WfTask::Init())
    ret = WfTask::catalog->ProcessCommand(cd, interp, argc, argv);

  return ret;
}

static int add_commands()
{
    new cliCommandInfo ("wftask",  wf_task_cmd);
    externInfo::add(WfTaskApp::type_info());

    return 0;
}

static int dummy = add_commands();

