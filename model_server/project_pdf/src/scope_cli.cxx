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
#include <machdep.h>
#include <Interpreter.h>
#include <scopeRoot.h>
#include <scopeClone.h>
#include <scopeMgr.h>
#include <scopeUnit.h>
#include <scopeMaster.h>


#ifdef _WIN32
#include <stdlib.h>
#endif

#include <xrefSymbolSet.h>
#include <genStringPlus.h>
#ifndef ISO_CPP_HEADERS
#include <ctype.h>
#else /* ISO_CPP_HEADERS */
#include <cctype>
using namespace std;
#endif /* ISO_CPP_HEADERS */

char const* readable_name_of_ddKind(ddKind k);

void (*ui_change_scope_hook)(char const *) = 0;
void (*ui_change_mode_hook)(char const *) = 0;

static int scope_global_mode;


int scope_get_global_mode()
{
    return scope_global_mode;
}
static char const *modes[] = {"off", "wide", "deep"};
static int no_modes = 3;

static int set_modeCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[])
{   
    Interpreter *i = (Interpreter *)cd;
    int retval = TCL_ERROR;
    char const *str = 0;
    if (argc == 1) {
      Tcl_SetResult(i->interp, (char *)modes[scope_global_mode], TCL_STATIC);
      retval = TCL_OK;
    } else if (argc == 2) {
        str = argv[1];
	for(int ii=0; ii<no_modes; ++ii){
	  if(strcmp(str, modes[ii])==0){
	    scope_global_mode = ii;
	    retval = TCL_OK;
	    break;
	  }
	}
    }

    if(retval != TCL_OK)
      Tcl_SetResult(i->interp, "usage: set_mode [off,wide,deep]", TCL_STATIC);
    else if (ui_change_mode_hook && str)
     (*ui_change_mode_hook)(str);
    return retval;
}


static int unitsCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[])
{   
    Interpreter *i = (Interpreter *)cd;
    DI_object res;
    DI_object_create(&res);
    symbolArr& res_arr = *get_DI_symarr(res);

    scopeMgr_fill_units (res_arr);

    i->SetDIResult(res);
    return TCL_OK;

}

static symbolArr&  root_scopes()
{
  static symbolArr root_scopes;
  if(root_scopes.size()==0){
    scopeMgr_fill_root_scopes (root_scopes, 0);
    root_scopes.sort();
  }
  return root_scopes;
}

static void scope_mark_arr(symbolArr&scarr, int v)
{
  Initialize(scope_mark_arr);
  int sz = scarr.size();
  for(int ii=0; ii<sz; ++ii){
    symbolPtr&sym = scarr[ii];
    scopeRoot*rt = checked_cast(scopeRoot, sym);
    rt->set_hidden(v);
  }
}
static void scope_mark_all(int v)
{
    scope_mark_arr(root_scopes(), v);
}

class liteScope  {
public:
  scopeRoot*root;
  symbolArr cache;
  liteScope(scopeRoot*r) : root(r) {}
};

void client_set_scope(liteScope * scope)
{
  static liteScope * current_scope = NULL;
  if(scope_global_mode && (scope != current_scope)) {
    current_scope = scope;
    if(scope){
      scope_mark_arr(root_scopes(), 1);
      scope->root->set_hidden(0);
      scope_global_mode = 2; // deep
    } else {
      scope_mark_arr(root_scopes(), 0);
      scope_global_mode = 1; // wide
    }
  }
}

static int scopesCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[])
{   
    Interpreter *i = (Interpreter *)cd;
    DI_object res;
    DI_object_create(&res);
    symbolArr& res_arr = *get_DI_symarr(res);

    scopeMgr_fill_units (res_arr);
    scopeMgr_fill_scopes (res_arr);

    i->SetDIResult(res);
    return TCL_OK;
}


static int root_scopesCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[])
{   
    Interpreter *i = (Interpreter *)cd;
    DI_object res;
    DI_object_create(&res);
    symbolArr& res_arr = *get_DI_symarr(res);
    
    int filter = 0; // all
    if (argc == 2) {
	if (strncmp(argv[1], "vis", 3) == 0)
	    filter = 1; // visible
	else if (strncmp(argv[1], "hid", 3) == 0)
	    filter = 2; // hidden
    }
 
    scopeMgr_fill_root_scopes (res_arr, filter);

    i->SetDIResult(res);
    return TCL_OK;

}

static int add_componentCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[])
{
    Initialize(add_componentCmd);

    Interpreter *i = (Interpreter *)cd;
    symbolArr add;
    if(i->ParseArguments(1, argc, argv, add) != 0)
      return TCL_ERROR;

    int sz = add.size();
    if (sz < 2) {
	Tcl_AppendResult(i->interp, "Usage: add_component <Obj> component-list", NULL);
	return TCL_ERROR;
    }
    
    symbolPtr& sym = add[0];
    if (sym.get_kind() != DD_SCOPE) {
	Tcl_AppendResult(i->interp, "Usage error: first parameter must be scope", NULL);
	return TCL_ERROR;
    }

    Relational *target_scope = (Relational *)sym;

    if ( !is_scopeRoot(target_scope) ) {
	Tcl_AppendResult(i->interp, "Usage error: first parameter must be scope", NULL);
	return TCL_ERROR;
    }

    scopeRoot *sh = (scopeRoot *)target_scope;

    ostream& os = i->GetOutputStream();
    for (int ii = 1 ; ii < sz ; ii ++) {
	scopeNode *t = NULL;
	symbolPtr& sym = add[ii];
	if (sym.is_xrefSymbol())
	    os << "Warning: component \"" << sym.get_name() << "\" is not project or scope. Ignored." << endl;
	else {
	    Relational *component = (Relational *)sym;

	    if (is_projNode(component))
		t = sh->add_component_project ((projNode *)component, os);
	    else if (is_scopeRoot(component)) {
		if (!is_scopeSLL(component) && !is_scopeUnit(component))
		    os << "Error: only sll or unit can be a component." << endl;
		else
		    t = sh->add_component_scope ((scopeRoot *)component, os);
	    }
	}
	if (t == NULL)
	    os << "Warning: symbol \"" << sym.get_name() << "\" is not added to the scope \"" << sh->get_name() << "\"" << endl;
    }
    return TCL_OK;
}

int scope_validate_args(Interpreter*i, int start, int argc, char const **argv, symbolArr&res)
{
    symbolArr scarr;
    if(i->ParseArguments(start, argc, argv, scarr) != 0)
      return TCL_ERROR;

    
    int sz = scarr.size();
    if (sz < 1) {
	Tcl_AppendResult(i->interp, "Usage error: no scope", NULL);
	return TCL_ERROR;
    }
    
    objArr to_do;
    for (int ii = 0; ii < sz; ++ii) {
	symbolPtr& sym = scarr[ii];
	if (sym.get_kind() != DD_SCOPE) {
	    Tcl_AppendResult(i->interp, "Usage error: all parameters must be scope", NULL);
	    return TCL_ERROR;
	}

	Relational *scope = (Relational *)sym;

	if ( !is_scopeRoot(scope) ) {
	    Tcl_AppendResult(i->interp, "Usage error: all parameters must be scope", NULL);
	    return TCL_ERROR;
	}

	scopeRoot *sh = (scopeRoot *)scope;
	if (master_get_clones (sh)) {
	    Tcl_AppendResult(i->interp, "Usage error: all parameters must be root scope", NULL);
	    return TCL_ERROR;
	}
	to_do.insert_last(sh);
    }
    res.insert_last(to_do);
    return TCL_OK;
}

static genStringPlus scope_string = "/";
char const* scope_get_string()
{
  return scope_string;
}
static void scope_build_string()
{
  symbolArr hidden;
  symbolArr scarr;

  scopeMgr_fill_root_scopes (hidden, 2);
  if(hidden.size() == 0){
    scope_string =  "/";
  } else {
    scopeMgr_fill_root_scopes (scarr, 1);
    int sz = scarr.size();
    scope_string = 0;
    for(int ii=0; ii<sz; ++ii){
      if(ii>0) 
	scope_string += " ";
      scope_string += scarr[ii].get_name();
    }
  }

  if(ui_change_scope_hook)
    (*ui_change_scope_hook)(scope_string);

}

static int hide_scopesCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[])
{
    Initialize(hide_scopesCmd);

    Interpreter *i = (Interpreter *)cd;
    symbolArr scarr;

    int retval = scope_validate_args(i, 1, argc, argv, scarr);
    if(retval==TCL_OK){
      scope_mark_arr(scarr, 1);
      scope_build_string();
    }

    return retval;
}

static int unhide_scopesCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[])
{
    Initialize(hide_scopesCmd);

    Interpreter *i = (Interpreter *)cd;
    symbolArr scarr;

    int retval = scope_validate_args(i, 1, argc, argv, scarr);
    if(retval==TCL_OK){
      scope_mark_arr(scarr, 0);
      scope_build_string();
    }

    return retval;
}



static int set_scopeCmd(Interpreter*i, int argc, char const *argv[])
{
    Initialize(set_scopeCmd);

    if(argc==2){
      char const *arg = argv[1];
      if(arg && arg[0]=='/' && arg[1]=='\0'){
         // set global scope
         scope_mark_all(0);
	 scope_string = "/";
	 if(ui_change_scope_hook)
	   (*ui_change_mode_hook)(scope_string);
	 return TCL_OK;
      }
    }

    symbolArr scarr;

    int retval = scope_validate_args(i, 1, argc, argv, scarr);
    if(retval==TCL_OK){
      scope_mark_all(1);
      scope_mark_arr(scarr, 0);
      scope_build_string();
    }

    return retval;
}
class scopeSet : public Set {
public:
  virtual unsigned int hash_element (const Object * ob) const;
  virtual bool isEqualObjects(const Object& ob1, const Object& ob2) const;
  liteScope* operator[](scopeRoot*) const;
};

unsigned int scopeSet::hash_element (const Object * ob) const
{
  return (unsigned int) ((liteScope*)ob)->root;
}

bool scopeSet::isEqualObjects(const Object& ob1, const Object& ob2) const
{
  const liteScope& s1 = (const liteScope&) ob1;
  const liteScope& s2 = (const liteScope&) ob2;
  return s1.root == s2.root;
}

liteScope* scopeSet::operator[](scopeRoot*root) const
{
  liteScope scope(root);
  return (liteScope*) findObjectWithKey((const Object&)scope);
}

static scopeSet lite_scopes;

static int unset_client_scopeCmd(Interpreter *i, int argc, char const *argv[])
{
    i->SetScope(NULL);
    return TCL_OK;
}

static int client_scopeCmd(Interpreter *i, int argc, char const *argv[])
{
  liteScope *scope = i->GetScope();
  char const *scope_name = scope ? scope->root->get_name() : "/";
  Tcl_SetResult(i->interp, (char *)scope_name, TCL_STATIC);
  return TCL_OK;
}
static int set_client_scopeCmd(Interpreter *i, int argc, char const *argv[])
{
    symbolArr scarr;

    int retval = scope_validate_args(i, 1, argc, argv, scarr);
    if(retval!=TCL_OK)
      return retval;
    
    if (scarr.size() != 1)
      return TCL_ERROR;

    scopeRoot *root = (scopeRoot *) (Relational*) scarr[0];
    liteScope *scope = lite_scopes[root];
    i->SetScope(scope);

    return retval;
}

static int set_scope_symbol_cacheCmd(Interpreter *i, int argc, char const *argv[])
{
    symbolArr scarr;
    if(i->ParseArguments(1, argc, argv, scarr) != 0)
      return TCL_ERROR;

    int sz = scarr.size();
    if(!sz)
      return TCL_ERROR;
      
    scopeRoot *root = (scopeRoot *) (Relational*) scarr[0];
    liteScope*scope = lite_scopes[root];
    if(scope){
      return TCL_ERROR;
    }
    // copy the rest
    scope = new liteScope(root);
    lite_scopes.add(*(Object*)scope);
    for(int ii=1; ii<sz; ++ii){
      scope->cache.insert_last(scarr[ii]);
    }

    return TCL_OK;
}

static symbolArr symbol_cache;
static int set_global_symbol_cacheCmd(Interpreter *i, int argc, char const *argv[])
{
    int res = i->ParseArguments(1, argc, argv, symbol_cache);
    return res ? TCL_ERROR : TCL_OK;
}
static int get_symbol_cacheCmd(Interpreter *i, int argc, char const *argv[])
{
  DI_object obj;
  DI_object_create(&obj);
  symbolArr& arr = *get_DI_symarr(obj);
  liteScope*scope = i->GetScope();
  
  const symbolArr& cache = scope ? scope->cache : symbol_cache;
  if (argc==1)
    arr = cache;
  else
    for(int ii=1, sz=cache.size(); ii<argc; ++ii){
      int ind = OSapi_atoi(argv[ii]);
      if(ind>0 && ind <=sz)
	arr.insert_last(cache[ind-1]);
    }
  i->SetDIResult(obj);
  return TCL_OK;
}

static int add_dllCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[])
{
    Initialize(add_dllCmd);

    Interpreter *i = (Interpreter *)cd;
    symbolArr add;
    if(i->ParseArguments(1, argc, argv, add) != 0)
      return TCL_ERROR;

    int sz = add.size();
    if (sz < 2) {
	Tcl_AppendResult(i->interp, "Usage: add_dll <exe|dll> dll-list", NULL);
	return TCL_ERROR;
    }
    
    symbolPtr& sym = add[0];
    if (sym.is_xrefSymbol()) {
	Tcl_AppendResult(i->interp, "Usage error: first parameter must be scope", NULL);
	return TCL_ERROR;
    }

    Relational *target_scope = (Relational *)sym;

    if ( !is_scopeRoot(target_scope) ) {
	Tcl_AppendResult(i->interp, "Usage error: first parameter must be scope", NULL);
	return TCL_ERROR;
    }

    scopeRoot *sh = (scopeRoot *)target_scope;

    ostream& os = i->GetOutputStream();
    for (int ii = 1 ; ii < sz ; ii ++) {
	boolean result = false;
	symbolPtr& sym = add[ii];
	if (sym.is_xrefSymbol())
	    os << "Error: only dll can be imported." << endl;
	else {
	    Relational *component = (Relational *)sym;
	    if (!is_scopeDLL(component))
		os << "Error: only dll can be imported." << endl;
	    else {
		if ((scopeRoot *)component == sh)
		    os << "Error: dll can not be imported into itself." << endl;
		else
		    result = sh->add_import ((scopeDLL *)component, os);
	    }
	}
	if (!result)
	    os << "Warning: symbol \"" << sym.get_name() << "\" is not imported in the scope \"" << sh->get_name() << "\"" << endl;
    }
    return TCL_OK;
}

static int add_exportCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[])
{
    Initialize(add_exportCmd);

    Interpreter *i = (Interpreter *)cd;
    symbolArr add;
    if(i->ParseArguments(1, argc, argv, add) != 0)
      return TCL_ERROR;

    int sz = add.size();
    if (sz < 2) {
	Tcl_AppendResult(i->interp, "Usage: add_export <dll> symbol-list", NULL);
	return TCL_ERROR;
    }
    
    symbolPtr& sym = add[0];
    if (sym.is_xrefSymbol()) {
	Tcl_AppendResult(i->interp, "Usage error: first parameter must be dll", NULL);
	return TCL_ERROR;
    }

    Relational *target_scope = (Relational *)sym;

    if ( !is_scopeDLL(target_scope) ) {
	Tcl_AppendResult(i->interp, "Usage error: first parameter must be dll", NULL);
	return TCL_ERROR;
    }

    scopeDLL *dll = (scopeDLL *)target_scope;

    ostream& os = i->GetOutputStream();
    for (int ii = 1 ; ii < sz ; ii ++) {
	symbolPtr& cur = add[ii];
	boolean result = dll->add_export(cur, os);
    }
    return TCL_OK;
}

#define DEF_LINE 1024
char line[DEF_LINE];
symbolPtr api_lookup_xrefSymbol(ddKind kind, char const *str);

char *trim_name (char *line)
{
    int in_name = 0;
    char *ret = 0;
    char c = 0;
    int ii = 0;
    while ( (c = line[ii]) ) {
	if (in_name) {
	    if ( !(isalnum(c) || c == ':' || c == '_') ) {
		line[ii] = 0;
		break;
	    }
	} else {
	    if (isalnum(c) || c == ':' || c == '_') {
		ret = &line[ii];
		in_name = 1;
	    }
	}
	ii++;
    }
    return ret;
}

static int read_def_fileCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[])
{
    Initialize(read_def_fileCmd);

    Interpreter *i = (Interpreter *)cd;
    if (argc != 3) {
	Tcl_AppendResult(i->interp, "Usage: read_def_file <dll> filename.def", NULL);
	return TCL_ERROR;
    }
    
    symbolPtr sym;
    scopeMgr_find (argv[1],sym);
    if (sym.is_xrefSymbol()) {
	Tcl_AppendResult(i->interp, "Usage error: first parameter must be dll", NULL);
	return TCL_ERROR;
    }

    Relational *target_scope = (Relational *)sym;

    if ( !is_scopeDLL(target_scope) ) {
	Tcl_AppendResult(i->interp, "Usage error: first parameter must be dll", NULL);
	return TCL_ERROR;
    }

    scopeDLL *dll = (scopeDLL *)target_scope;
    ostream& os = i->GetOutputStream();

    char const *fname = argv[2];
    FILE *fff = fopen (fname, "r");
    if (fff == 0) {
	os << "Error: Could not read file \"" << fname << "\"" << endl;
	return TCL_ERROR;
    }

    int in_export_list = 0;
    line[0] = 0;
    while ( (fgets(line, 1024, fff)) ) {
	if (in_export_list) {
	    char *symbol_name = trim_name (line);
	    if (symbol_name && symbol_name[0])
		(void)dll->add_export (os, DD_FUNC_DECL, symbol_name);
	} else {
	    if (strncmp(line,"EXPORTS", strlen("EXPORTS")) == 0 )
		in_export_list = 1;
	}
	line[0] = 0;
    }

    return TCL_OK;
}

static int scopes_ofCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[])
{
  Initialize(scope_ofCmd);
  
  Interpreter *i = (Interpreter *)cd;
  symbolArr add;
  if(i->ParseArguments(1, argc, argv, add) != 0)
    return TCL_ERROR;

  DI_object res;
  DI_object_create(&res);
  symbolArr& res_arr = *get_DI_symarr(res);

  int sz = add.size();
  for(int ii=0; ii<sz; ++ii) {
    Relational *obj = NULL;
    symbolPtr& sym = add[ii];
    if (sym.is_xrefSymbol())
      obj = sym.get_projNode();
    else
      obj = (Relational *)sym;

    if(!obj)
      continue;

    if(is_projNode(obj))
      obj = projNodePtr(obj)->root_project();
    else if ( !is_scopeRoot(obj))
      continue;

    symbolArr res;
    scopeMgr_get_scopes_of (obj, res);
    res_arr.insert_last(res);
  } 

  if(sz>1)
     res_arr.usort();

  i->SetDIResult(res);
  return TCL_OK;
}

static int scopes_inCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[])
{
    Initialize(scope_inCmd);
    DI_object res;
    DI_object_create(&res);
    symbolArr& res_arr = *get_DI_symarr(res);

    Interpreter *i = (Interpreter *)cd;
    symbolArr add;
    if(i->ParseArguments(1, argc, argv, add) != 0)
      return TCL_ERROR;

    int sz = add.size();
    if (sz != 1) {
	Tcl_AppendResult(i->interp, "Usage: scopes_in <Obj>", NULL);
	return TCL_ERROR;
    }
    
    symbolPtr& sym = add[0];
    if (sym.is_xrefSymbol()) {
	Tcl_AppendResult(i->interp, "Usage error: first parameter must be scope", NULL);
	return TCL_ERROR;
    }

    Relational *obj = (Relational *)sym;

    if ( !is_scopeRoot(obj) ) {
	Tcl_AppendResult(i->interp, "Usage error: first parameter must be scope", NULL);
	return TCL_ERROR;
    }

    scopeMgr_get_scopes_in ((scopeRoot *)obj, res_arr);

    i->SetDIResult(res);
    return TCL_OK;
}

static int importsCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[])
{
    Initialize(importsCmd);
    DI_object res;
    DI_object_create(&res);
    symbolArr& res_arr = *get_DI_symarr(res);

    Interpreter *i = (Interpreter *)cd;

    symbolArr scarr;

    int retval = scope_validate_args(i, 1, argc, argv, scarr);
    if(retval != TCL_OK){
       return retval;
    }

    int sz = scarr.size();
    
    for(int ii=0; ii<sz; ++ii){
      symbolPtr& sym = scarr[ii];
      scopeRoot* obj = checked_cast(scopeRoot,sym);
      scopeMgr_get_imported_in (obj, res_arr);
    }

    if(sz > 1)
      res_arr.usort();

    i->SetDIResult(res);
    return TCL_OK;
}

static int get_defsCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[])
{
    Initialize(get_defsCmd);

    Interpreter *i = (Interpreter *)cd;
    symbolArr add;
    if(i->ParseArguments(1, argc, argv, add) != 0)
      return TCL_ERROR;

    DI_object res;
    DI_object_create(&res);
    symbolArr& res_arr = *get_DI_symarr(res);

    int sz = add.size();
    for(int ii=0; ii<sz; ++ii){
      symbolPtr& sym = add[ii];
      sym.get_def_symbols(res_arr);
    }

    if(sz>1)
      res_arr.usort();

    i->SetDIResult(res);
    return TCL_OK;
}

static int exportsCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[])
{
    Initialize(exportsCmd);

    Interpreter *i = (Interpreter *)cd;
    symbolArr add;
    if(i->ParseArguments(1, argc, argv, add) != 0)
      return TCL_ERROR;

    DI_object res;
    DI_object_create(&res);
    symbolArr& res_arr = *get_DI_symarr(res);

    int sz = add.size();
    for(int ii=0; ii<sz; ++ii){
      symbolPtr& sym = add[ii];
      scopeRoot *obj = checked_cast(scopeRoot,sym);

      if (is_scopeDLL(obj))
	scopeMgr_get_exported_in (obj, res_arr);
    }

    if(sz>1)
      res_arr.usort();

    i->SetDIResult(res);
    return TCL_OK;
}

static int where_importedCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[])
{
    Initialize(where_importedCmd);

    Interpreter *i = (Interpreter *)cd;
    symbolArr add;
    if(i->ParseArguments(1, argc, argv, add) != 0)
      return TCL_ERROR;

    DI_object res;
    DI_object_create(&res);
    symbolArr& res_arr = *get_DI_symarr(res);

    int sz = add.size();
    for(int ii=0; ii<sz; ++ii){
      symbolPtr& sym = add[ii];
      if(sym.get_kind() != DD_SCOPE)
	continue;

      Relational *obj = sym;
      if ( !is_scopeDLL(obj))
	continue;

      Obj* execs = dll_get_execs(scopeDLLPtr(obj));
      Obj*el;
      ForEach(el, *execs)
	res_arr.insert_last(el);
    }
    res_arr.usort();
    i->SetDIResult(res);
    return TCL_OK;
}

static int new_sllCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[])
{
    Initialize(new_sllCmd);

    int num_args   = argc - 1;
    Interpreter *i = (Interpreter *)cd;
    DI_object res;
    DI_object_create(&res);

    symbolArr& res_arr = *get_DI_symarr(res);

    if(num_args < 1) {
	Tcl_AppendResult(i->interp, "must be at least one parameter", NULL);
	return TCL_ERROR;
    }

    for(int ii = 1; ii < argc; ++ii) {
	char const *cur_name = argv[ii];
	symbolPtr cur_sym;
	scopeMgr_find (cur_name, cur_sym);
	if (cur_sym.isnotnull()) {
	    i->GetOutputStream() << "Warning: scope name " << "\"" << cur_name << "\"" << " already exists" << endl;
	    continue;
	}

	scopeMgr_new_sll (cur_name, cur_sym);
	if (cur_sym.isnull()) {
	    i->GetOutputStream() << "Warning: Failed to create SLL " << "\"" << cur_name << "\"" << endl;
	    continue;
	}
	res_arr.insert_last (cur_sym);
    }

    i->SetDIResult(res);
    return TCL_OK;
}

static int new_dllCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[])
{
    Initialize(new_dllCmd);

    int num_args   = argc - 1;
    Interpreter *i = (Interpreter *)cd;
    DI_object res;
    DI_object_create(&res);

    symbolArr& res_arr = *get_DI_symarr(res);

    if(num_args < 1) {
	Tcl_AppendResult(i->interp, "must be at least one parameter", NULL);
	return TCL_ERROR;
    }

    for(int ii = 1; ii < argc; ++ii) {
	char const *cur_name = argv[ii];
	symbolPtr cur_sym;
	scopeMgr_find (cur_name, cur_sym);
	if (cur_sym.isnotnull()) {
	    i->GetOutputStream() << "Warning: scope name " << "\"" << cur_name << "\"" << " already exists" << endl;
	    continue;
	}

	scopeMgr_new_dll (cur_name, cur_sym);
	if (cur_sym.isnull()) {
	    i->GetOutputStream() << "Warning: Failed to create DLL " << "\"" << cur_name << "\"" << endl;
	    continue;
	}
	res_arr.insert_last (cur_sym);
    }

    i->SetDIResult(res);
    return TCL_OK;
}

static int new_exeCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[])
{
    Initialize(new_exeCmd);

    int num_args   = argc - 1;
    Interpreter *i = (Interpreter *)cd;
    DI_object res;
    DI_object_create(&res);

    symbolArr& res_arr = *get_DI_symarr(res);

    if(num_args < 1) {
	Tcl_AppendResult(i->interp, "must be at least one parameter", NULL);
	return TCL_ERROR;
    }

    for(int ii = 1; ii < argc; ++ii) {
	char const *cur_name = argv[ii];
	symbolPtr cur_sym;
	scopeMgr_find (cur_name, cur_sym);
	if (cur_sym.isnotnull()) {
	    i->GetOutputStream() << "Warning: scope name " << "\"" << cur_name << "\"" << " already exists" << endl;
	    continue;
	}

	scopeMgr_new_exe (cur_name, cur_sym);
	if (cur_sym.isnull()) {
	    i->GetOutputStream() << "Warning: Failed to create EXE " << "\"" << cur_name << "\"" << endl;
	    continue;
	}
	res_arr.insert_last (cur_sym);
    }

    i->SetDIResult(res);
    return TCL_OK;
}

static int home_projCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[])
{
    Initialize(home_projCmd);

    Interpreter *i = (Interpreter *)cd;   
    DI_object res;
    DI_object_create(&res);

    symbolArr& res_arr = *get_DI_symarr(res);
    projNode*hp = projNode::get_home_proj();
    if(hp)
     res_arr.insert_last(hp);     

    i->SetDIResult(res);
    return TCL_OK;
}

// set_home_proj
//
//  0 == success
// !0 == failure

static int set_home_proj(symbolPtr &sym, int force = 0)
{
  Initialize(set_home_proj);

  int ret = 1;

  projNodePtr proj = sym_to_proj(sym);

  if (proj) {
    Xref *xr = proj->get_xref();
    projNode *t;
    while (!xr && (t = proj->find_parent())) {
      proj = t;
      xr = proj->get_xref();
    }
  }

  if (proj && (proj->is_writable() || force) && (Xref::test_subproj_pmod(proj) != -1)) {
    projNodePtr old_proj = projNode::get_home_proj();
    projNode::set_home_proj(proj);
    Xref *tmp_xref = proj->get_xref();
    bool wrt_flag = 0;
    if (tmp_xref)
      wrt_flag = tmp_xref->is_writable();
    if (wrt_flag == 0 && !force)
      projNode::set_home_proj(old_proj);
    else
      ret = 0;
  }

  return ret;
}

static int set_home_proj_cmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[])
{
  Initialize(set_home_proj_cmd);

  int status = 1;   // failure

  Interpreter *i = (Interpreter *)cd;

  symbolArr sel;
  if (i->ParseArguments(1, argc, argv, sel) == 0) {
    if (sel.size() == 1) {
      symbolPtr sym = sel[0];
      status = set_home_proj(sym, 1);
    }
  }

  Tcl_SetResult(i->interp, (char *)((status == 0) ? "0" : "1"), TCL_VOLATILE);

  return TCL_OK;
}

static int add_commands()
{
  new cliCommandInfo ("units", unitsCmd);
  new cliCommandInfo ("scopes", scopesCmd);
  new cliCommandInfo ("root_scopes", root_scopesCmd);

  new cliCommandInfo ("add_component", add_componentCmd);
  new cliCommandInfo ("hide_scopes", hide_scopesCmd);
  new cliCommandInfo ("unhide_scopes", unhide_scopesCmd);
  new cliCommandInfo ("add_sll",       add_componentCmd);
  new cliCommandInfo ("add_import",    add_dllCmd);
  new cliCommandInfo ("add_export",    add_exportCmd);
  new cliCommandInfo ("read_def_file", read_def_fileCmd);

  new cliCommandInfo ("new_sll",    new_sllCmd);
  new cliCommandInfo ("create_sll", new_sllCmd);

  new cliCommandInfo ("new_dll",    new_dllCmd);
  new cliCommandInfo ("create_dll", new_dllCmd);

  new cliCommandInfo ("new_exe",    new_exeCmd);
  new cliCommandInfo ("create_exe", new_exeCmd);

  new cliCommandInfo ("scopes_of", scopes_ofCmd);
  new cliCommandInfo ("scopes_in", scopes_inCmd);
  new cliCommandInfo ("def_symbols", get_defsCmd);
  new cliCommandInfo ("imports", importsCmd);
  new cliCommandInfo ("exports", exportsCmd);
  new cliCommandInfo ("set_mode", set_modeCmd);

  new cliCommand ("set_scope", set_scopeCmd);
  new cliCommand ("client_scope", client_scopeCmd);
  new cliCommand ("set_client_scope", set_client_scopeCmd);
  new cliCommand ("unset_client_scope", unset_client_scopeCmd);
  new cliCommand ("set_global_symbol_cache", set_global_symbol_cacheCmd);
  new cliCommand ("set_scope_symbol_cache", set_scope_symbol_cacheCmd);
  new cliCommand ("get_symbol_cache", get_symbol_cacheCmd);
  new cliCommandInfo ("wimp", where_importedCmd);
  new cliCommandInfo ("home_proj", home_projCmd);
  new cliCommandInfo ("set_home_proj", set_home_proj_cmd);

  return 0;
}


static int add_commands_dummy = add_commands();
int ATT_hidden(symbolPtr&sym)
{
  Initialize(ATT_hidden);
  scopeNode * sc = NULL;
  if(sym.is_xrefSymbol()){
    sc = sym.get_scope();
  } else {
    ddKind kind = sym.get_kind();
    if(kind == DD_SCOPE){
      sc = (scopeNode*) (Relational*)sym;
    } else if(kind == DD_PROJECT) {
      projNode*proj = checked_cast(projNode,sym);
      sc = scope_mgr.unit(proj);
    }
  }

  if(!sc) return 0;

  scopeRoot* rt = (scopeRoot*)sc->get_root();

  Obj* clones = master_get_clones(rt);
  if(!clones)
    return rt->is_hidden();

  Obj*el;
  int retval = 1;
  ForEach(el, *clones){
    scopeClone*cl = (scopeClone*) el;
    scopeRoot*rr = (scopeRoot*) cl->get_root();
    if(master_get_clones(rr))   // not real root
      continue;
    if(! rr->is_hidden()){
      retval = 0;
      break;
    }
  }
  return retval;
}

int
ATT_dll(symbolPtr &sym)
{
  Initialize(ATT_dll(symbolPtr &));

  if (sym.is_xrefSymbol())
    return 0;

  Relational *rel = sym;

  if (!rel)
    return 0;

  return
    is_scopeDLL(rel);
}

int
ATT_exe(symbolPtr &sym)
{
  Initialize(ATT_exe(symbolPtr &));

  if (sym.is_xrefSymbol())
    return 0;

  Relational *rel = sym;

  if (!rel)
    return 0;

  return
    is_scopeEXE(rel) && !is_scopeDLL(rel);
}

int
ATT_writable(symbolPtr &sym)
{
  Initialize(ATT_writable(symbolPtr &));

  int retval = 0;
  projNode *proj = sym.get_projNode();

  if (proj)
    retval = proj->is_writable();

  return retval;
}

//boris: looks up first the scope if the scope_mode is not 0
//       otherwise lookup first project
void lookup_scope_or_project (char const *str, symbolPtr& scope_sym)
{
    Initialize(lookup_scope_or_project);
    projNode * proj = NULL;
    if (scope_global_mode == 0) {
	// project
	proj = projNode::find_project(str, 0);
	if(proj && strcmp(proj->get_name(), str) == 0)
	    scope_sym = proj;
	else {
	    // unit or scope
	    scopeMgr_find (str, scope_sym);
	}
    } else {
	// unit or scope
	scopeMgr_find (str, scope_sym);
	if (scope_sym.isnull()) {
	    // project
	    proj = projNode::find_project(str);
	    if(proj && !strcmp(proj->get_name(), str))
		scope_sym = proj;
	}
    }
}
