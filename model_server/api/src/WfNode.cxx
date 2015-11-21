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
#include <WfNode.h>
#include <machdep.h>
#include <genArrCharPtr.h>
#ifndef ISO_CPP_HEADERS
#include <ctype.h>
#else /* ISO_CPP_HEADERS */
#include <cctype>
using namespace std;
#endif /* ISO_CPP_HEADERS */
#include "charstream.h"

init_relational(WfParam, objDictionary);
init_relational(WfNode, objDictionary);

//init_rel_or_ptr(WfParam, param, 0, WfValueType, valuetype, 0);
init_rel_or_ptr(WfParam, param, 0, WfParamInstance, instance, 0);
init_rel_or_ptr(WfNode, node, 0, WfNodeInstance, instance, 0);
init_rel_or_ptr(WfValueType,base,0, WfDerivedType,derived,0);
init_rel_or_ptr(WfParamInstance,pi,0,WfSymbolArr,symarr,0);

//
// Static data:
//

#ifdef _WIN32
int wfnode_commands_flag = 0;
#endif

WfParamCatalog *WfParam::catalog = NULL;
WfNodeCatalog *WfNode::catalog = NULL;
objArr WfValueType::catalog;

//
// CLASS: WfValueType
//

WfValueType::WfValueType(char const*nm) : objDictionary(nm) 
{ 
  catalog.insert_last(this);
}

WfValueType*WfValueType::lookup(char const*name)
{ 
  return (WfValueType*)  obj_search_by_name(name, catalog);
}
char const* WfValueType::bname() const
{
  char const*bname;
  if(isDerived()){
    WfValueType* bobj = ((WfDerivedType*) this)->getBase();
    bname = bobj->get_name();
  } else {
    bname = get_name();
  }
  return bname;
}

bool WfValueType::is(char const*name) const
{
  WfValueType* bobj = lookup(name);
  return
    bobj ? is(bobj) : false;
}

bool WfValueType::is(WfValueType* bobj) const
{
  if(this == bobj)
    return true;
  else if(!isDerived())
    return false;
  else {
    WfValueType* super = ((WfDerivedType*) this)->getBase();
    return super->is(bobj);
  } 
}

int WfValueType::list(ostream& os)
{
  int sz = catalog.size();
  for(int ii=0; ii<sz; ++ii){
    WfValueType* vt = (WfValueType*) catalog[ii];
    if(ii > 0)
      os << ' ';
    char const*name = vt->get_name();
    os << name;
  }
  return sz;
}

bool WfValueType::Print(TclList &list)
{
  bool ret = true;

  CODE code = btype();
  genString buf;
  switch (code) 
  {
    case INT:    list = "int"; break;
    case BOOL:   list = "bool"; break;
    case STRING: list = "string"; break;
    case SET:    list = "set"; break;
    case ENUM:   if (((WfEnumType *)this)->get_list(buf) != TCL_ERROR) {
		   buf.printf("enum {%s}", buf.str());
		   list = buf;
		 }
		 else
		   ret = false;
		 break;
    default:     break;
  }

  return ret;
}

WfDerivedType::WfDerivedType(char const*nm, WfValueType*vt)
   : WfValueType(nm)
{
  derived_put_base(this, vt);
}

WfDerivedType::WfDerivedType(char const*nm, WfValueType*vt, char const*validator)
   : WfValueType(nm), _validate(validator)
{
  derived_put_base(this, vt);
}

WfValueType* WfDerivedType::getBase() const
{ 
  WfValueType*super = getSuper();
  while(super && super->isDerived()){
    super = ((WfDerivedType*)super)->getSuper();
  }
  return super;
}

bool WfDerivedType::validate(char const*value) const
{
  if(!getBase()->validate(value))
    return false;
  
  char const* cmd = _validate;
  if(!cmd || !cmd[0])
    return true;

  genString buf;
  buf.printf("%s %s", cmd, value);
  Interpreter *curint = GetActiveInterpreter();
  int err = curint->EvalCmd(buf);

  genString val;
  curint->GetResult(val);
  char const*ptr = val;

  if(err == TCL_ERROR){
    ostream&os = curint->GetOutputStream();
    os << ptr << endl;
    return false;
  }

  return 
    (ptr[0] != '\0') &&  (ptr[0] != '0');
}

bool WfIntType::validate(char const*value) const
{
  if(value[0]=='-')
    ++value;

  int ch;
  while(ch=*(value++))
    if(!isdigit(ch))
      return false;
  return true;
}

bool WfBoolType::validate(char const*value) const
{
  if(value[1])
    return false;

  int bval = value[0] - '0';
  return bval == 0 || bval == 1;
}

int WfEnumType::get_list(genString& val)
{
  int err = TCL_OK;
  if(!eval_flag){
    val = list;
  } else {
    Interpreter *curint = GetActiveInterpreter();
    err = curint->EvalCmd(list);

    if(err == TCL_ERROR)
      val = 0;
    else
      curint->GetResult(val);
  }
  return err;
}

//
// CLASS: WfParam
//

WfParam::WfParam(char const *tag, char const *_vname, char const *_def, char const *_info, WfValueType *_vt, int _mc)
  : objDictionary(tag), vname(_vname), def(_def), info(_info), vt(_vt), mc(_mc)
{ 
  catalog->Append(this); 
}

bool WfParam::Init()
{
  static bool ret = false;

  if (!ret) {
    if (!catalog) {
      catalog = new WfParamCatalog;
      if (catalog)
        if (catalog->InitHooks()) {
	  catalog->AppendHook("describe", (AccessCatalog::HookMP)&WfParamCatalog::DescribeCmd);
          ret = true;
        }
    }
    else
      ret = true;
  }

  return ret;
}

WfParam *WfParam::Find(char const *tag)
{
  if (catalog)
    return (WfParam *)(catalog->Find(tag));
  else
    return NULL;
}

void WfParam::print(ostream& os, int) const
{
  os << get_name(); 
}

//
// CLASS: WfNode
//

WfNode::WfNode(char const *tag, char const *_command, char const *_info, objArr &_inputs, objArr &_outputs, objArr &_optionals)
  : objDictionary(tag), command(_command), info(_info), inputs(_inputs), outputs(_outputs), optionals(_optionals)
{
  catalog->Append(this);
}

bool WfNode::Init()
{
  static bool ret = false;

  if (!ret) {
    if (!catalog) {
      catalog = new WfNodeCatalog;
      if (catalog)
        if (catalog->InitHooks()) {
	  catalog->AppendHook("describe", (AccessCatalog::HookMP)&WfNodeCatalog::DescribeCmd);
          ret = true;
        }
    }
    else
      ret = true;
  }

  return ret;
}

WfNode *WfNode::Find(char const *tag)
{
  if (catalog)
    return (WfNode *)(catalog->Find(tag));
  else
    return NULL;
}

void WfNode::print(ostream& os, int) const
{
  os << get_name();
}

int WfNode::GetInputs(objArr& arr) const { arr = inputs; return arr.size(); }
int WfNode::GetOutputs(objArr& arr) const { arr = outputs; return arr.size(); }
int WfNode::GetOptionals(objArr& arr) const { arr = optionals; return arr.size(); }

//
// CLASS: WfParamInstance
//

WfParamInstance::WfParamInstance(WfParam *param)
{
    set_value = NULL;
    param_put_instance(param, this);
    if (param->def.not_null())
	SetVal(param->def);
    else
	Unset();
}

bool WfParamInstance::Ready() const { return active; }
char const *WfParamInstance::GetStrVal() const { return str_value; }
DI_object const WfParamInstance::GetDIVal() const { return set_value; }
void WfParamInstance::Unset() { set_value = NULL; str_value = 0; active = false; }
void WfParamInstance::operator=(const WfParamInstance &pi) { set_value = pi.GetDIVal(); str_value = pi.GetStrVal(); active = pi.Ready(); }
void WfParamInstance::SetValLiteral(char const *_value) { str_value = _value; active = true; }
void WfParamInstance::SetDIVal(DI_object obj) { set_value = obj; active = true; }

void WfParamInstance::Revive()
{
    WfParam *param = instance_get_param(this);
    if (param && (param->vt->btype() == WfValueType::SET)) {
	WfSymbolArr *wa = pi_get_symarr(this);
	if (wa) {
	    symbolArr *arr = wa->arr;
	    if (arr) {
		Interpreter *i = GetActiveInterpreter();
		if (i) {
		    DI_object dobj;
		    DI_object_create(&dobj);
		    symbolArr *darr = get_DI_symarr(dobj);
		    i->ReferenceObject(dobj);
		    int sz = arr->size();
		    for (int ii = 0; ii < sz; ii++)
			darr->insert_last((*arr)[ii]);
		    SetDIVal(dobj);
		}
	    }
	}
    }
}

void WfParamInstance::SetVal(char const *_value)
{
    WfParam *param = instance_get_param(this);
    if (param && (param->vt->btype() == WfValueType::SET)) {
	symbolArr varr;
	Interpreter *i = GetActiveInterpreter();
	if (i->ParseArgString(_value, varr) == 0) {
	    DI_object dobj;
	    DI_object_create(&dobj);
	    symbolArr *darr = get_DI_symarr(dobj);
	    i->ReferenceObject(dobj);
	    int sz = varr.size();
	    for (int ii = 0; ii < sz; ii++)
		darr->insert_last(varr[ii]);
	    if (set_value != NULL){
		WfSymbolArr *wa = pi_get_symarr(this);
		if (wa) {
		    pi_rem_symarr(this);
		    delete wa;
		    i->DeleteDIObject(set_value);
		}
	    }
	    WfSymbolArr *warr = new WfSymbolArr(darr);
	    pi_put_symarr(this, warr);
	    SetDIVal(dobj);
	}
    }
    else
	SetValLiteral(_value);
}

bool WfParamInstance::Equal(WfParamInstance *p1, WfParamInstance *p2)
{
  bool ret = false;

  if (p1 && p1->Ready() && p2 && p2->Ready()) {
    WfParam *pp1 = instance_get_param(p1);
    WfParam *pp2 = instance_get_param(p2);
    if (pp1 && pp2) {
      if (pp1->vt && pp2->vt) {
        WfValueType::CODE t1 = pp1->vt->btype();
        WfValueType::CODE t2 = pp2->vt->btype();
        if (t1 == t2) {
          if (t1 == WfValueType::SET) {
            symbolArr a1, a2;
	    WfSymbolArr *w1 = pi_get_symarr(p1);
	    WfSymbolArr *w2 = pi_get_symarr(p2);
	    if (w1 && w2 && w1->arr && w2->arr) {
	      a1 = *w1->arr;
	      a2 = *w2->arr;
	      int s1 = a1.size();
	      int s2 = a2.size();
	      if (s1 == s2) {
	        a1.sort();
	        a2.sort();
		bool eq = true;
		for (int ii = 0; ii < s1; ii++) {
		  if (a1[ii] != a2[ii]) {
		    eq = false;
		    break;
		  }
		}
		if (eq)
		  ret = true;
	      }
	    }
	  } else {
	      char const *v1 = p1->GetStrVal();
	      char const *v2 = p2->GetStrVal();
	      if (!OSapi_strcmp(v1, v2))
		  ret = true;
	  }
        }
      }
    }
  }

  return ret;
}

void WfParamInstance::Reset()
{ 
  Unset();
  WfParam *param = instance_get_param(this);
  if (param) {
    if (param->def.not_null())
      SetVal(param->def);
  }
}

void WfParamInstance::Print(TclList &list, WfPrintCode pc, char const *add, bool showname)
{
  WfParam *pp = instance_get_param(this);

  TclList param;
  
  if (showname)
    param += pp->get_name();

  if (add)
    param += add;

  TclList type;
  TclList extended_type;
  WfValueType *vt = pp->vt;

  type += vt->get_name();
  if (pc == WF_DUMP)
      vt->Print(extended_type);

  TclList init;
  init += active ? "1" : "0";

  TclList val;
  if (active){
    if(pc == WF_DUMP){
      WfParam *param = instance_get_param(this);
      if (param && (param->vt->btype() == WfValueType::SET)){
	int len;
	genString tmp = Tcl_GetStringFromObj(set_value, &len);
	val           = tmp;
      } else
	val = str_value;
    } else
      val = str_value;
  }
  param += type;
  if(pc == WF_DUMP)
      param += extended_type;
  param += init;
  param += val;

  list += param;
}

//
// CLASS: WfSymbolArr
//
// Relational wrapper for symbolArr.
//
WfSymbolArr::WfSymbolArr(symbolArr *_arr) : arr(_arr) {}

//
// CLASS: WfNodeInstance
//

WfNodeInstance::WfNodeInstance(WfNode *node) : retcode(-1)
{
  node_put_instance(node, this);

  objArr arr;
  Obj *el;

  {
    node->GetInputs(arr);
    ForEach (el, arr) {
      WfParamInstance *pi = new WfParamInstance((WfParam *)el);
      inputs.insert_last(pi);
    }
  }

  {
    node->GetOutputs(arr);
    ForEach (el, arr) {
      WfParamInstance *pi = new WfParamInstance((WfParam *)el);
      outputs.insert_last(pi);
    }
  }

  {
    node->GetOptionals(arr);
    ForEach (el, arr) {
      WfParamInstance *pi = new WfParamInstance((WfParam *)el);
      optionals.insert_last(pi);
    }
  }
}

WfNodeInstance::WfNodeInstance(WfNode *node, objArr &_inputs, objArr &_outputs, objArr &_optionals)
  : retcode(-1), inputs(_inputs), outputs(_outputs), optionals(_optionals)
{
  node_put_instance(node, this);
}

WfNodeInstance::~WfNodeInstance()
{
  Obj *el;

  { ForEach (el, inputs) delete (WfParamInstance *)el; }
  { ForEach (el, outputs) delete (WfParamInstance *)el; }
  { ForEach (el, optionals) delete (WfParamInstance *)el; }
}

bool WfNodeInstance::Ready() const
{
  bool ret = true;

  Obj *el;
  ForEach (el, inputs) {
    WfParamInstance *pi = (WfParamInstance *)el;
    if (!pi->Ready()) {
      ret = false;
      break;
    }
  }

  return ret;
}
 
int WfNodeInstance::Run(Interpreter *i)
{
  int ret = TCL_ERROR;

  if (Ready()) {

    Obj *el;
    objArr params;
    params.insert_all(inputs);
    params.insert_all(optionals);

    error_string = "";
    // Set inputs and optionals:

    ForEach (el, params) {
      WfParamInstance *pi = (WfParamInstance *)el;
      WfParam *p          = instance_get_param(pi);
      char const *pname         = (p->vname);
      char const *pval;
      if (pi->Ready()) {
	  if (p && (p->vt->btype() == WfValueType::SET)){
	      Tcl_Obj *name = Tcl_NewStringObj(pname, -1);
              Tcl_IncrRefCount(name);
	      Tcl_ObjSetVar2(i->interp, name, NULL, pi->GetDIVal(), TCL_GLOBAL_ONLY);
	      Tcl_DecrRefCount(name);
	  } else {
	      pval = pi->GetStrVal();
	      TclList cmd;
	      cmd += "set";
	      cmd += pname;
	      cmd += pval;
	      i->EvalCmd(cmd);
	  }
      } else {
	  pval = "";
	  TclList cmd;
	  cmd += "set";
	  cmd += pname;
	  cmd += pval;
	  i->EvalCmd(cmd);
      }
    }

    // Evaluate script:

    WfNode *node = instance_get_node(this);
    int code = i->EvalCmd(node->command);

    // Set outputs:

    if (code != TCL_ERROR) {
      ForEach (el, outputs) {
        WfParamInstance* pi = (WfParamInstance *)el;
	WfParam *p = instance_get_param(pi);
        char const *vname = p->vname;
        char const *val = Tcl_GetVar(i->interp, vname, 0);
	if(val == NULL){
	    error_string = "Script produced no result.";
	    code         = TCL_ERROR;
	} else 
	    pi->SetVal(val);
      }
    } else {
	i->GetResult(error_string);
    }

    // Clean up:

    objArr arr;
    arr.insert_all(inputs);
    arr.insert_all(outputs);
    int sz = arr.size();
    for (int ii = 0; ii < sz; ii++) {
      WfParamInstance* pi = (WfParamInstance *)arr[ii];
      WfParam *p = instance_get_param(pi);
      char const *vname = p->vname;
      Tcl_UnsetVar(i->interp, vname, 0);
    }

    ret = code;
  }
  else {
    WfNode *node = instance_get_node(this);
    genString error;
    error.printf("wfnode (%s): error: ambiguous or missing input(s).", node->get_name());
    Tcl_SetResult(i->interp, (char *)error, TCL_VOLATILE);
  }

  retcode = ret;

  return ret;
}

void WfNodeInstance::GetLastError(genString &err_str, genString &err_info)
{
    err_str  = error_string;
    err_info = error_info;
}

void WfNodeInstance::Print(TclList &list, WfPrintCode pc, bool recurse, bool showname)
{
  TclList node;

  if (showname) {
    WfNode *n = instance_get_node(this);
    node += n->get_name();
  }

  if (recurse) {

    int sz, ii;

    TclList *olist;
    TclList in, out, opt;
    char const *add = NULL;

    if (pc == WF_DUMP)
      olist = &node;

    if (pc == WF_DUMP) add = "Input"; else olist = &in;
    sz = inputs.size();
    for (ii = 0; ii < sz; ii++) {
      WfParamInstance *pi = (WfParamInstance *)inputs[ii];
      pi->Print(*olist, pc, add, showname);
    }

    if (pc == WF_DUMP) add = "Output"; else olist = &out;
    sz = outputs.size();
    for (ii = 0; ii < sz; ii++) {
      WfParamInstance *pi = (WfParamInstance *)outputs[ii];
      pi->Print(*olist, pc, add, showname);
    }

    if (pc == WF_DUMP) add = "Optional"; else olist = &opt;
    sz = optionals.size();
    for (ii = 0; ii < sz; ii++) {
      WfParamInstance *pi = (WfParamInstance *)optionals[ii];
      pi->Print(*olist, pc, add, showname);
    }

    if (pc == WF_DEFAULT) {
      node += in;
      node += out;
      node += opt;
    }
  }

  list += node;
}

#define ifeq(x, y) if (!OSapi_strcmp(x, y))

//
// CLASS: WfNodeCatalog
//

int WfNodeCatalog::DescribeCmd(int argc, char const *argv[], genString &result)
{
  int ret = TCL_ERROR;

  result  = "";

  if (argc == 3) {
    char const *tag = argv[2];
    WfNode *node = (WfNode *)Find(tag);
    if (node) {
      if (node->info.not_null())
        result = node->info;
      ret = TCL_OK;
    }
    else
      result.printf("%s: error: unknown item: \"%s\".", argv[0], tag);
  }
  else
    result.printf("usage: %s %s %s", argv[0], argv[1], "tag");

  return ret;
}

int WfNodeCatalog::AppendItemCmd(int argc, char const *argv[], genString &result)
{
  int ret = TCL_ERROR;

  result  = "";

  bool valid = true;
  char const *tag = NULL;
  char const *script = NULL;
  char const *info = NULL;
  genArrCharPtr in;
  genArrCharPtr out;
  genArrCharPtr opt;

  for (int ii = 2; ii < argc; ii++) {
    ifeq (argv[ii], "-eval")
      if (!script)
        if (ii < argc - 1)
          script = argv[++ii];
        else
          { valid = false; break; }
      else
        { valid = false; break; }
    else ifeq (argv[ii], "-i")
      if (!info)
        if (ii < argc - 1)
          info = argv[++ii];
        else
          { valid = false; break; }
      else
        { valid = false; break; }
    else ifeq(argv[ii], "-in")
      if (ii < argc - 1)
        in.append(&(argv[++ii]));
      else
        { valid = false; break; }
    else ifeq(argv[ii], "-out")
      if (ii < argc - 1)
        out.append(&(argv[++ii]));
      else
        { valid = false; break; }
    else ifeq(argv[ii], "-opt")
      if (ii < argc - 1)
        opt.append(&(argv[++ii]));
      else
        { valid = false; break; }
    else if (!tag)
      tag = argv[ii];
    else
      { valid = false; break; }
  }

  if (valid && tag && script) {
    if (!Find(tag)) {
      genArrCharPtr bad;
      objArr _in, _out, _opt;
      {
        AccessCatalog::FillArr(WfParam::catalog, _in, in, bad);
          if (!bad.size()) {
	    AccessCatalog::FillArr(WfParam::catalog, _out, out, bad);
	      if (!bad.size())
	        AccessCatalog::FillArr(WfParam::catalog, _opt, opt, bad);
	  }
      }
      if (!bad.size()) {
        WfNode *node = new WfNode(tag, script, info, _in, _out, _opt);
        if (node) {
	   result = tag;
           ret = TCL_OK;
        }
        else
          result.printf("%s: error: cannot create object.", argv[0]);
      }
      else
        result.printf("%s: error: unknown item: \"%s\".", argv[0], (*(bad[0])));
    }
    else
      result.printf("%s: error: name already in use: \"%s\".", argv[0], tag);
  }
  else
    result.printf("usage: %s %s %s", argv[0], argv[1], "tag -eval command [ { -in | -out | -opt } <p1> ... ] [ -i info ]");

  return ret;
}

//
// CLASS: WfParamCatalog
//

int WfParamCatalog::DescribeCmd(int argc, char const *argv[], genString &result)
{
  int ret = TCL_ERROR;

  result  = "";

  if (argc == 3) {
    char const *tag = argv[2];
    WfParam *param = (WfParam *)Find(tag);
    if (param) {
      if (param->info.not_null())
        result = param->info;
      ret = TCL_OK;
    }
    else
      result.printf("%s: error: unknown item: \"%s\".", argv[0], tag);
  }
  else
    result.printf("usage: %s %s %s", argv[0], argv[1], "tag");

  return ret;
}

int WfParamCatalog::AppendItemCmd(int argc, char const *argv[], genString &result)
{
  int ret = TCL_ERROR;

  result = "";

  bool valid = true;
  char const *tag = NULL;
  char const *vname = NULL;
  char const *def = NULL;
  char const *info = NULL;
  int mc = WfParam::M_OUT;
  WfValueType *vtp = NULL;

  for (int ii = 2; ii < argc; ii++) {
    ifeq (argv[ii], "-v")
      if (!vname)
        if (ii < argc - 1)
	  vname = argv[++ii];
        else
          { valid = false; break; }
      else
	{ valid = false; break; }
    else ifeq (argv[ii], "-d")
      if (!def)
        if (ii < argc - 1)
          def = argv[++ii];
        else
	  { valid = false; break; }
      else
        { valid = false; break; }
    else ifeq (argv[ii], "-i")
      if (!info)
        if (ii < argc - 1)
          info = argv[++ii];
        else
          { valid = false; break; }
      else
        { valid = false; break; }
    else ifeq (argv[ii], "-t")
      if (!vtp)
        if (ii < argc - 1) {
          char const *vtype = argv[++ii];
          vtp = WfValueType::lookup(vtype);
        }
        else { valid = false; break; }
      else { valid = false; break; }
    else ifeq (argv[ii], "-mi")
      mc |= WfParam::M_INP;
    else ifeq (argv[ii], "-mt")
      mc |= WfParam::M_TAG;
    else ifeq (argv[ii], "-mn")
      mc = 0;
    else if (!tag)
      tag = argv[ii];
    else
      { valid = false; break; }
  }

  if (valid && tag && vname && vtp) {
    if (!Find(tag)) {
      WfParam *param = new WfParam(tag, vname, def, info, vtp, mc);
      if (param) {
        result = tag;
        ret = TCL_OK;
      }
      else
        result.printf("%s: error: allocation failed.", argv[0]);
    }
    else
      result.printf("%s: error: name already in use: \"%s\".", argv[0], tag);
  }
  else
    result.printf("usage: %s %s %s", argv[0], argv[1], "tag -v var -t type [ -d default ] [ -i info ]");

  return ret;
}

static int wf_type_new(Interpreter* i, int argc, char const **argv)
{
  int retval = TCL_ERROR;
  ostream& out = i->GetOutputStream();

  if(argc < 2){
    out << "name must be specified" << endl;
    return retval;
  }
  char const * base = argv[0];
  char const * name = argv[1];

  WfValueType *btp = WfValueType::lookup(base);
  WfValueType *ntp = WfValueType::lookup(name);

  if(!btp){
    out << "base type " << base << " does not exist" << endl;
    return retval;
  }
  if(ntp){
    out << "type " << name << " already exists" << endl;
    return retval;
  }

  char const * validation = 0;
  if(argc == 4)
    validation = argv[3];
  else if(argc > 4){
    out << argc + 2 << ": too many parameters" << endl;
    return retval;
  }

  ntp = new WfDerivedType(name, btp, validation);
  if(ntp){
    Tcl_SetResult(i->interp, (char *)name, TCL_VOLATILE);
    retval = TCL_OK;
  }
  return retval;
}
static int wf_type_enum(Interpreter* i, int argc, char const **argv)
{
  int retval = TCL_ERROR;
  ostream& out = i->GetOutputStream();

  if(argc < 2){
    out << "name must be specified" << endl;
    return retval;
  }
  char const * name = argv[0];

  WfValueType *ntp = WfValueType::lookup(name);

  if(ntp){
    out << "type " << name << " already exists" << endl;
    return retval;
  }
 
  char const * list = argv[1];
  bool eval = argc == 3;
  ntp = new WfEnumType(name, list, eval);
  if(ntp){
    Tcl_SetResult(i->interp, (char *)name, TCL_VOLATILE);
    retval = TCL_OK;
  }
  return retval;
}

static int wf_type_cmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[])
{
  Interpreter* i = (Interpreter*) cd;
  int ret = TCL_OK;

  ostream& out = i->GetOutputStream();
  if(argc < 2){
    ocharstream os;
    WfValueType::list(os);
    os << ends;
    Tcl_SetResult(i->interp, os.ptr(), TCL_VOLATILE);
    return TCL_OK;
  } 

  char const*name = argv[1];
  if (argc == 2){
    WfValueType* vt = WfValueType::lookup(name);
    if(vt){
      TclList vv;
      if (vt->Print(vv))
        Tcl_SetResult(i->interp, (char*)vv, TCL_VOLATILE);
      else
        ret = TCL_ERROR;
    }
  } else if(strcmp(name, "new")==0){
    ret = wf_type_new(i, argc-2, argv+2);
  } else if(strcmp(name, "enum")==0){
    ret = wf_type_enum(i, argc-2, argv+2);
  } else {
    out << name << ":wrong subcommand" << endl;
    ret = TCL_ERROR;
  }
  return ret;
}
int wf_param_cmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[])
{
  int ret = TCL_ERROR;

  if (WfParam::Init())
    ret = WfParam::catalog->ProcessCommand(cd, interp, argc, argv);
 
  return ret;
}

int wf_node_cmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[])
{
  int ret = TCL_ERROR;

  if (WfNode::Init())
    ret = WfNode::catalog->ProcessCommand(cd, interp, argc, argv);

  return ret;
}

char const *sc_action_names[] = {
  "None",
  "Warn",
  "Reject"
};

static int add_commands()
{
    new WfIntType("int");
    new WfBoolType("bool");
    new WfStringType("string");
    WfValueType*set = new WfSetType("set");
    new WfDerivedType("entity", set);
    new WfDerivedType("instance", set, "instance_test");
    new WfDerivedType("scope", set);
    
    new cliCommandInfo ("wftype",  wf_type_cmd);
    new cliCommandInfo ("wfnode",  wf_node_cmd);
    new cliCommandInfo ("wfparam", wf_param_cmd);

    TclList validActions;
    for (int i = 0; i < (sizeof(sc_action_names) / sizeof(char const *)); i++) {
      validActions += sc_action_names[i];
    }
    new WfEnumType("sc_action", (validActions.Value()), false);

    return 0;
}

static int dummy = add_commands();
