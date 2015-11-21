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
#include "general.h"
#include "genError.h"
#include "tcl.h"
#include "Interpreter.h"
#include "api_internal.h"
#include "genString.h"
#include "ddKind.h"
#include "objArr.h"
#include "machdep.h"
#include "cLibraryFunctions.h"
#include "_Xref.h"
#include "symbolSet.h"
#include "xref_queries.h"
#include "smtRegion.h"
#include "cmd.h"
#include <xrefSymbol.h>
#include <genMask.h>
#include "customize.h"

genArr(int);

#define CHECK_ARG_NUM(name,num)

extern "C" char* cli_error_use();
genArr(char);
void cli_get_roots(symbolArr &roots);
void dataCell_get_module(const symbolPtr &symbol, symbolPtr &module);

void api_instances(symbolPtr& s, symbolArr&);
void fill_array_with_xref_query(symbolArr&arr, int link, symbolArr&res);

void instances_in_mod(symbolPtr&sym, symbolPtr&mod, symbolArr&res);

void module_instances(symbolPtr& sym, symbolArr *res);
void convert_projects_and_modules(const symbolArr& syms, symbolArr&mods, symbolArr&locs, int other);

int Interpreter::RootsCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[])
{
    CHECK_ARG_NUM("roots", 0);
    Interpreter *i = (Interpreter *)cd;
    DI_object dest;
    DI_object_create(&dest);
    symbolArr& roots = *get_DI_symarr(dest);
    cli_get_roots(roots);
    i->SetDIResult(dest);
    return TCL_OK;
}

void cli_find_child_projects(symbolArr&src_arr, symbolArr& trg_arr);
int Interpreter::SubprojectsCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[])
{
    CHECK_ARG_NUM("subprojects", 0);
    Interpreter *i = (Interpreter *)cd;
    symbolArr res;
    if(i->ParseArguments(1, argc, argv, res) != 0)
	return TCL_ERROR;

    DI_object target;

    DI_object_create(&target);
    symbolArr & trg_arr =  *get_DI_symarr(target);
    cli_find_child_projects(res, trg_arr);

    i->SetDIResult(target);
    return TCL_OK;
}

int Interpreter::ParentCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[])
{
    CHECK_ARG_NUM("parent", 0);
    Interpreter *i = (Interpreter *)cd;
    symbolArr res;
    if(i->ParseArguments(1, argc, argv, res) != 0)
	return TCL_ERROR;
    DI_object source;
    DI_object target;
    DI_object_create(&source);
    DI_object_create(&target);
    symbolArr *source_arr = get_DI_symarr(source);
    source_arr->insert_last(res);
    DI_find_parent_project(source, target);
    DI_object_delete(source);
    i->SetDIResult(target);
    return TCL_OK;
}

int Interpreter::FilesCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[])
{
    CHECK_ARG_NUM("modules", 0);
    Interpreter *i = (Interpreter *)cd;
    symbolArr res;


    int nextArg = 1;
    int depth = 0;
    if ((argc > 2) && (strcmp(argv[1],"-level") == 0)){
         depth = OSapi_atoi(argv[2]);
         nextArg = 3;
     };

    if(i->ParseArguments(nextArg, argc, argv, res) != 0)
        return TCL_ERROR;
    DI_object source;
    DI_object target;
    DI_object_create(&source);
    DI_object_create(&target);
    symbolArr *source_arr = get_DI_symarr(source);
    source_arr->insert_last(res);
    DI_deep_find_files(source, target, depth);


    DI_object_delete(source);
    i->SetDIResult(target);
    return TCL_OK;
}

int Interpreter::ContentsCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[])
{
    CHECK_ARG_NUM("contents", 0);
    Interpreter *i = (Interpreter *)cd;
    symbolArr res;
    if(i->ParseArguments(1, argc, argv, res) != 0)
	return TCL_ERROR;
    DI_object source;
    DI_object target;
    DI_object_create(&source);
    DI_object_create(&target);
    symbolArr *source_arr = get_DI_symarr(source);
    source_arr->insert_last(res);
    DI_find_contents(source, target);
    DI_object_delete(source);
    i->SetDIResult(target);
    return TCL_OK;
}

void ddSelector::add_all()
{
  for(int dd=1; dd < NUM_OF_DDS; ++dd) 
    add((ddKind)dd);
}
void cli_defines_or_uses_not_unique(symbolArr&src_arr, 
			 symbolArr&result_arr, ddSelector&sel, int def_flag);
void cli_defines_or_uses(symbolArr&src_arr, 
			 symbolArr&result_arr, ddSelector&sel, int def_flag);

int Interpreter::UsesCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[])
{
    CHECK_ARG_NUM("uses", 0);
    ddSelector sel;
    Interpreter *i = (Interpreter *)cd;
    int argpos     = i->SetupSelector(sel, argc, argv);
    if(argpos < 0)
	return TCL_ERROR;
    symbolArr src;
    if(i->ParseArguments(argpos, argc, argv, src) != 0)
	return TCL_ERROR;

    DI_object results;
    DI_object_create(&results);
    symbolArr& res = * get_DI_symarr(results);

    cli_defines_or_uses(src, res, sel, 0);
    i->SetDIResult(results);
    return TCL_OK;
}

int Interpreter::DefinesCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[])
{
    CHECK_ARG_NUM("defines", 0);
    ddSelector sel;
    Interpreter *i = (Interpreter *)cd;
    int argpos     = i->SetupSelector(sel, argc, argv);
    if(argpos < 0)
	return TCL_ERROR;
    symbolArr src;
    if(i->ParseArguments(argpos, argc, argv, src) != 0)
	return TCL_ERROR;

    DI_object results;
    DI_object_create(&results);
    symbolArr& res = * get_DI_symarr(results);

    cli_defines_or_uses(src, res, sel, 1);
    i->SetDIResult(results);
    return TCL_OK;
}

static int defines_or_usesCmd(Interpreter *i, int argc, char const *argv[])
{
    ddSelector sel;
    int argpos     = i->SetupSelector(sel, argc, argv);
    if(argpos < 0)
	return TCL_ERROR;
    symbolArr src;
    if(i->ParseArguments(argpos, argc, argv, src) != 0)
	return TCL_ERROR;

    DI_object results;
    DI_object_create(&results);
    symbolArr& res = * get_DI_symarr(results);

    cli_defines_or_uses_not_unique(src, res, sel, 1);
    cli_defines_or_uses_not_unique(src, res, sel, 0);
    res.remove_dup_syms();
    i->SetDIResult(results);
    return TCL_OK;
}

int Interpreter::WhereCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[])
{
    CHECK_ARG_NUM("where",1);

    Interpreter *i = (Interpreter *)cd;
    if(argc < 2){
	Tcl_AppendResult(i->interp, "argument missing", NULL);
	return TCL_ERROR;
    }
    int tcl_res;
    char const *arg = argv[1];
    --argc;
    ++argv;
    if(strcmp(arg, "defined") == 0)
	tcl_res = i->WhereDefinedCmd(argc, argv);
    else if(strcmp(arg, "referenced") == 0)
	tcl_res = i->WhereReferencedCmd(argc, argv);
    else if(strcmp(arg, "used") == 0)
	tcl_res = i->WhereUsedCmd(argc, argv);
    else if(strcmp(arg, "included") == 0)
	tcl_res = i->WhereIncludedCmd(argc, argv);
    else if(strcmp(arg, "declared") == 0)
	tcl_res = i->WhereDeclaredCmd(argc, argv);
    else {
	Tcl_AppendResult(i->interp, "argument should be one of: defined, referenced, used, included", NULL);
	return TCL_ERROR;
    }
    return tcl_res;
}

void api_declarations(symbolPtr& xsym, symbolArr& res)
{
  symbolArr instances;
  api_instances(xsym, instances);
  int sz = instances.size();
  
  static symbolAttribute *astdecl = symbolAttribute::get_by_name("astdeclaration");
  for(int ii=0; ii<sz; ++ii){
    symbolPtr& inst = instances[ii];
    char const* val = astdecl->value(inst);
    if(val && val[0] != '0')
      res.insert_last(inst);
  }
}

static int get_declarationsCmd(Interpreter*i, int argc, char const *argv[])
{
    symbolArr src;
    if(i->ParseArguments(1, argc, argv, src) != 0)
	return TCL_ERROR;
    DI_object results;
    DI_object_create(&results);
    symbolArr& res_arr = * get_DI_symarr(results);

    symbolArr mods;
    symbolArr other;
    convert_projects_and_modules(src, mods, other, 1);

    int sz = other.size();
    int ii;
    for(ii=0; ii<sz; ++ii) {
      symbolPtr& sym = other[ii];
      symbolPtr xsym = sym.get_xrefSymbol();
      if(xsym.isnull())
	continue;

      if (sym.is_instance() && (xsym.get_kind() == DD_LOCAL)){
	  MetricElement inst = sym.get_MetricElement();
	  MetricElement decl;
	  if (!inst.isnull()) 
	    decl = inst.inst_get_decl();
	  if (!decl.isnull())
	    res_arr.insert_last(symbolPtr(&decl,sym.get_MetricSetPtr()));
      } else {
	api_declarations(xsym, res_arr);
      }
    }
    sz = mods.size();   
    for(ii=0; ii<sz; ++ii)
      api_declarations(mods[ii], res_arr);

    res_arr.unsort();
    i->SetDIResult(results);
    return TCL_OK;
}

void fill_array_with_xref_query(symbolArr&src_arr, const genMask& links, symbolArr&res);
int Interpreter::WhereDeclaredCmd(int argc, char const *argv[])
{
    symbolArr src;
    if(ParseArguments(1, argc, argv, src) != 0)
	return TCL_ERROR;
    DI_object results;
    DI_object_create(&results);
    symbolArr& res_arr = * get_DI_symarr(results);

    symbolArr xsyms;
    int sz = src.size();
    for(int ii=0; ii<sz; ++ii) {
      symbolPtr& sym = src[ii];
      symbolPtr xsym = sym.get_xrefSymbol();
      if(xsym.isnull())
	continue;
      if (sym.is_instance() && (xsym.get_kind() == DD_LOCAL)){
	symbolPtr scp = sym.scope();
	if(scp.isnotnull())
	  res_arr.insert_last(scp);
      } else {
	xsyms.insert_last(xsym);
      }
    }
    if(xsyms.size()){
      symbolArr decls;
      genMask links;
      links.add(SHOW_DECLARED_IN);
      fill_array_with_xref_query(xsyms, links, decls);
      res_arr.insert_last(decls);
    }
    res_arr.unsort();
    SetDIResult(results);
    return TCL_OK;
}

symbolPtr astXref_get_module(astXref*);
int Interpreter::WhereDefinedCmd(int argc, char const *argv[])
{
    symbolArr res;
    if(ParseArguments(1, argc, argv, res) != 0)
	return TCL_ERROR;
    DI_object source;
    DI_object target;
    DI_object_create(&source);
    DI_object_create(&target);
    symbolArr *source_arr = get_DI_symarr(source);
    int sz = res.size();

    for (int ii = 0; ii < sz; ++ii) {
      symbolPtr &sym = res[ii];

      if (sym.is_instance()) {
	symbolPtr scp= sym.scope();
	source_arr->insert_last(scp);
      } else if (sym.is_ast()) {
	astXref *axr = sym.get_astXref();

	if (axr) {
	  symbolPtr mod = astXref_get_module(axr);

	  if (mod.isnotnull())
	      source_arr->insert_last(mod);
	}
      } else if (sym.is_dataCell()) {
	symbolPtr mod;
	dataCell_get_module(sym, mod);

	if (mod.isnotnull())
	  source_arr->insert_last(mod);
      } else
	source_arr->insert_last(sym);
    }

    DI_where_defined(source, target);
    DI_object_delete(source);
    SetDIResult(target);
    return TCL_OK;
}

int Interpreter::WhereReferencedCmd(int argc, char const *argv[])
{
    symbolArr res;
    if(ParseArguments(1, argc, argv, res) != 0)
	return TCL_ERROR;
    DI_object source;
    DI_object target;
    DI_object_create(&source);
    DI_object_create(&target);
    symbolArr *source_arr = get_DI_symarr(source);
    source_arr->insert_last(res);
    DI_where_referenced(source, target);
    DI_object_delete(source);
    SetDIResult(target);
    return TCL_OK;
}

int Interpreter::WhereIncludedCmd(int argc, char const *argv[])
{
    symbolArr res;
    if(ParseArguments(1, argc, argv, res) != 0)
	return TCL_ERROR;
    DI_object source;
    DI_object target;
    DI_object_create(&source);
    DI_object_create(&target);
    symbolArr *source_arr = get_DI_symarr(source);
    source_arr->insert_last(res);
    DI_files_where_included(source, target);
    DI_object_delete(source);
    SetDIResult(target);
    return TCL_OK;
}

int Interpreter::WhereUsedCmd(int argc, char const *argv[])
{
    symbolArr res;
    if(ParseArguments(1, argc, argv, res) != 0)
	return TCL_ERROR;
    DI_object source;
    DI_object target;
    DI_object_create(&source);
    DI_object_create(&target);
    symbolArr *source_arr = get_DI_symarr(source);
    source_arr->insert_last(res);
    DI_where_used(source, target);
    DI_object_delete(source);
    SetDIResult(target);
    return TCL_OK;
}

int Interpreter::GetIncludeFilesCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[])
{
    CHECK_ARG_NUM("get-include-files",0);

    Interpreter *i = (Interpreter *)cd;
    symbolArr res;
    if(i->ParseArguments(1, argc, argv, res) != 0)
	return TCL_ERROR;
    DI_object source;
    DI_object target;
    DI_object_create(&source);
    DI_object_create(&target);
    symbolArr *source_arr = get_DI_symarr(source);
    source_arr->insert_last(res);
    DI_show_include_files(source, target);
    DI_object_delete(source);
    i->SetDIResult(target);
    return TCL_OK;
}

int cli_get_link_cmd(int link, ClientData cd, Tcl_Interp *, int argc, char const *argv[]);

int Interpreter::GetMemberFunctionsCmd(ClientData cd, Tcl_Interp *i, int argc, char const *argv[])
{
  return cli_get_link_cmd(SHOW_MEMBER_FUNCS, cd, i, argc, argv);
}

int Interpreter::GetTemplateCmd(ClientData cd, Tcl_Interp *i, int argc, char const *argv[])
{
  return cli_get_link_cmd(SHOW_TEMPLofINST, cd, i, argc, argv);
}

int Interpreter::GetInstantiationsCmd(ClientData cd, Tcl_Interp *i, int argc, char const *argv[])
{
  return cli_get_link_cmd(SHOW_INSTofTEMPL, cd, i, argc, argv);
}

int Interpreter::GetMemberDataCmd(ClientData cd, Tcl_Interp *i, int argc, char const *argv[])
{
  return cli_get_link_cmd(SHOW_MEMBER_DATA, cd, i, argc, argv);
}

int Interpreter::GetFriendsCmd(ClientData cd, Tcl_Interp *i, int argc, char const *argv[])
{
  return cli_get_link_cmd(SHOW_FRIENDS, cd, i, argc, argv);
}

int Interpreter::GetGeneratedCmd(ClientData cd, Tcl_Interp *i, int argc, char const *argv[])
{
  return cli_get_link_cmd(SHOW_GENERATED, cd, i, argc, argv);
}

int Interpreter::GetMacrosCmd(ClientData cd, Tcl_Interp *i, int argc, char const *argv[])
{
  return cli_get_link_cmd(SHOW_MACROS, cd, i, argc, argv);
}

int Interpreter::GetSuperClassesCmd(ClientData cd, Tcl_Interp *i, int argc, char const *argv[])
{
  return cli_get_link_cmd(SHOW_HAS_SUPERCLASS, cd, i, argc, argv);
}

int Interpreter::GetSubClassesCmd(ClientData cd, Tcl_Interp *i, int argc, char const *argv[])
{
  return cli_get_link_cmd(SHOW_HAS_SUBCLASS, cd, i, argc, argv);
}

int Interpreter::GetDeclaresCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[])
{
    CHECK_ARG_NUM("get-declares",0);

    Interpreter *i = (Interpreter *)cd;
    symbolArr res;
    if(i->ParseArguments(1, argc, argv, res) != 0)
	return TCL_ERROR;
    DI_object source;
    DI_object target;
    DI_object_create(&source);
    DI_object_create(&target);
    symbolArr *source_arr = get_DI_symarr(source);
    source_arr->insert_last(res);
    DI_show_declares(source, target);
    DI_object_delete(source);
    i->SetDIResult(target);
    return TCL_OK;
}

int Interpreter::SetCopyCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[])
{
    CHECK_ARG_NUM("set_copy",1);

    Interpreter *i = (Interpreter *)cd;
    symbolArr res;
    if(i->ParseArguments(1, argc, argv, res) != 0)
	return TCL_ERROR;
    DI_object source;
    DI_object target;
    DI_object_create(&source);
    DI_object_create(&target);
    symbolArr *source_arr = get_DI_symarr(source);
    source_arr->insert_last(res);
    DI_object_copy(source, target);
    DI_object_delete(source);
    i->SetDIResult(target);
    return TCL_OK;
}

int Interpreter::SetAddCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[])
{
    CHECK_ARG_NUM("set_add",2);

    Interpreter *i = (Interpreter *)cd;
    if(argc < 3){
	Tcl_AppendResult(interp, "wrong number of arguments", NULL);
	return TCL_ERROR;
    }
    symbolArr arg1;
    DI_object var1;
    DI_object_create(&var1);
    symbolArr *var1_arr = get_DI_symarr(var1);
    if(i->ParseArgString(argv[1], arg1) != 0 || i->ParseArgString(argv[2], *var1_arr) != 0){
	Tcl_AppendResult(i->interp, "set-add: inavlid arguments\n", NULL);
	return TCL_ERROR;
    }
    DI_object target;
    DI_object_create(&target);
    symbolArr *target_arr = get_DI_symarr(target);
    target_arr->insert_last(arg1);
    DI_object_append(var1, target);
    DI_object_delete(var1);
    i->SetDIResult(target);
    return TCL_OK;
}

int Interpreter::SetUnionCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[])
{
    Interpreter *i = (Interpreter *)cd;
    if(argc < 2){
	Tcl_AppendResult(interp, "wrong number of arguments", NULL);
	return TCL_ERROR;
    }
    symbolArr arr;
    if(i->ParseArguments(1, argc, argv, arr) != 0)
	return TCL_ERROR;
    arr.unsort(); // remove doubles

    DI_object target;
    DI_object_create(&target);
    symbolArr *target_arr = get_DI_symarr(target);
    target_arr->insert_last(arr);
    i->SetDIResult(target);
    return TCL_OK;
}

int Interpreter::SetSubtractCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[])
{
    CHECK_ARG_NUM("set_subtract",2);

    Interpreter *i = (Interpreter *)cd;
    if(argc < 3){
	Tcl_AppendResult(interp, "wrong number of arguments", NULL);
	return TCL_ERROR;
    }
    symbolArr arg1;
    DI_object var1;
    DI_object_create(&var1);
    symbolArr *var1_arr = get_DI_symarr(var1);
    if(i->ParseArgString(argv[1], arg1) != 0 || i->ParseArgString(argv[2], *var1_arr) != 0){
	Tcl_AppendResult(i->interp, "set-add: inavlid arguments\n", NULL);
	return TCL_ERROR;
    }
    DI_object target;
    DI_object_create(&target);
    symbolArr *target_arr = get_DI_symarr(target);
    target_arr->insert_last(arg1);
    DI_array_subtract(var1, target);
    DI_object_delete(var1);
    i->SetDIResult(target);
    return TCL_OK;
}

int Interpreter::SetNilCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[])
{
    CHECK_ARG_NUM("nil_set",0);

    Interpreter *i = (Interpreter *)cd;
    if(argc != 1){
	Tcl_AppendResult(i->interp, "nil_set: does not take any argumets\n", NULL);
	return TCL_ERROR;
    }
    DI_object nil_set;
    DI_object_create(&nil_set);
    i->SetDIResult(nil_set);
    return TCL_OK;
}

int Interpreter::SetIntersectCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[])
{
    CHECK_ARG_NUM("set_intersect",2);

    Interpreter *i = (Interpreter *)cd;
    if(argc < 3){
	Tcl_AppendResult(interp, "wrong number of arguments", NULL);
	return TCL_ERROR;
    }
    symbolArr arg1;
    DI_object var1;
    DI_object_create(&var1);
    symbolArr *var1_arr = get_DI_symarr(var1);
    if(i->ParseArgString(argv[1], arg1) != 0 || i->ParseArgString(argv[2], *var1_arr) != 0){
	Tcl_AppendResult(i->interp, "set-add: inavlid arguments\n", NULL);
	return TCL_ERROR;
    }
    DI_object target;
    DI_object_create(&target);
    symbolArr *target_arr = get_DI_symarr(target);
    target_arr->insert_last(arg1);
    DI_array_intersect(var1, target);
    DI_object_delete(var1);
    i->SetDIResult(target);
    return TCL_OK;
}

int Interpreter::PrintFormatCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[])
{
    CHECK_ARG_NUM("printformat",1);

    if(argc < 2){
	Tcl_AppendResult(interp, "wrong number of arguments", NULL);
	return TCL_ERROR;
    }
    Interpreter *i      = (Interpreter *)cd;
    int nextArg = 1;
    char const * format_string;
    if (strcmp(argv[nextArg], "-s") == 0) {
	i->SetSortFlag(true);
	nextArg++;
	format_string = argv[nextArg];
	nextArg++;
    } else {	
	i->SetSortFlag(false);
	format_string = argv[nextArg];
	nextArg++;
    };
    objArr fmt_attrs;
    static genArrOf(char) order;
    order.reset();
    char nextOrder = 'a';
    for(int ii = nextArg; ii < argc; ii++){
	if (strcmp(argv[ii],"-d") == 0) {
	    nextOrder = 'd';
	} 
	else 
	    if (strcmp(argv[ii],"-a") == 0) {
		nextOrder = 'a';
	    } else {
		symbolAttribute *att = symbolAttribute::get_by_name(argv[ii]);
		if(att == NULL){
		    Tcl_AppendResult(interp, "attribute '", argv[ii], "' does not exist", NULL);
		    return TCL_ERROR;
		}
		fmt_attrs.insert_last(att);
		order.append(&nextOrder);
	    };
    };
    i->current_print_format->setAttributes(fmt_attrs);
    i->current_print_format->setFormat(format_string);
    i->current_print_format->setSortOrder(order[0]);
    return TCL_OK;
}

int Interpreter::SortCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[])
{
    CHECK_ARG_NUM("sort",0);

    Interpreter *i      = (Interpreter *)cd;
    objArr srt_attrs;
    int ii = 1;
    symbolAttribute *att;
    static genArrOf(char) order;
    order.reset();
    char nextOrder = 'a';
    while ( (ii < argc) && ((strcmp(argv[ii],"-d") == 0) || 
			    (strcmp(argv[ii],"-a") == 0) || 
			    (att = symbolAttribute::get_by_name(argv[ii]))) ) {
	if (strcmp(argv[ii],"-d") == 0) {
	    nextOrder = 'd';
	} 
	else 
	    if (strcmp(argv[ii],"-a") == 0) {
		nextOrder = 'a';
	    } else {
		srt_attrs.insert_last(att);
		order.append(&nextOrder);
	    };
	ii++;
    };
    order.append("\0");
    symbolArr res;
    if(i->ParseArguments(ii, argc, argv, res) != 0) {
	Tcl_AppendResult(interp, "attribute '", argv[ii], "' does not exist", NULL);
	return TCL_ERROR;
    };

    // make default sorting by name
    if (srt_attrs.size() == 0) { 
	att = symbolAttribute::get_by_name("name");
	if (att) {
	    srt_attrs.insert_last(att);
	};
    };
    
    DI_object target;
    DI_object_create(&target);
    symbolArr *target_arr = get_DI_symarr(target);
    symbolFormat sorter("",srt_attrs);
    sorter.setSortOrder(order[0]);
    sorter.do_padding = 1;
    sorter.sort(res, *target_arr);
    i->SetDIResult(target);
    return TCL_OK;
}


int Interpreter::AddAttributeCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[])
{
    CHECK_ARG_NUM("add_attribute", 2);
//    Interpreter *i        = (Interpreter *)cd;

    if(argc != 3) {
	Tcl_AppendResult(interp, "Usage: add_attribute att-name att-expression", NULL);
	return TCL_ERROR;
    };
    char const *aname = argv[1];
    char const *att_expr = argv[2];
    int tcl_res = TCL_OK;
    
    expr *expression = api_parse_expression(att_expr);
    if(expression == NULL){
	Tcl_AppendResult(interp, "expression parse error: \n", cli_error_use(), NULL);
	return TCL_ERROR;
    }

    symbolAttribute* attr = new exprAttribute(aname,expression);
//    Tcl_CreateCommand(interp, attr->get_name(), AttributeCmd, (ClientData)attr, NULL);
    tcl_res = TCL_OK;
    return tcl_res;
}

extern int get_modified_objects(symbolArr &, symbolArr & );

int Interpreter::ModifiedEntitiesCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[])
{
    CHECK_ARG_NUM("modified_entities",0);

    Interpreter *i = (Interpreter *)cd;
    symbolArr res;
    if(i->ParseArguments(1, argc, argv, res) != 0)
	return TCL_ERROR;
    DI_object source;
    DI_object target;
    DI_object_create(&source);
    DI_object_create(&target);
    symbolArr *source_arr = get_DI_symarr(source);
    symbolArr *target_arr = get_DI_symarr(target);
    source_arr->insert_last(res);
    get_modified_objects(*source_arr, *target_arr);
    DI_object_delete(source);
    i->SetDIResult(target);
    return TCL_OK;
}

void cli_defines_or_uses(symbolArr&src, symbolArr&res, ddSelector&sel, int def_flag);

void convert_projects_and_modules(const symbolArr& syms, symbolArr&mods, symbolArr&locs, int other)
{
  symbolArr scopes;
  int sz = syms.size();
  for(int ii = 0; ii < sz; ii++){
    symbolPtr& sym = syms[ii];
    ddKind kind = sym.get_kind();
    switch(kind){
    case DD_PROJECT:
    case DD_SCOPE:
      scopes.insert_last(sym);
      break;
    case DD_MODULE:
      {
	symbolPtr xsym = sym.get_def_symbol();
	if(xsym.isnotnull())
	  mods.insert_last(xsym);
      }
      break;
    case DD_LOCAL:
      locs.insert_last(sym);
    default:
      if(other)
	locs.insert_last(sym);
      break;
   } 
  }
  int no_scopes = scopes.size();
  if(no_scopes) {
    ddSelector sel;
    sel.add(DD_MODULE);
    cli_defines_or_uses(scopes, mods, sel, 1);
  }
}

static void get_instances(int mod_expand, const symbolArr& src, symbolArr&results_arr)
{
  symbolArr mods;
  symbolArr other;
  convert_projects_and_modules(src, mods, other, 1);
  int ii, sz;
  
  sz = other.size();
  for(ii=0; ii<sz; ++ii) {
    api_instances(other[ii], results_arr);
  }
  
  sz = mods.size();   
  for(ii=0; ii<sz; ++ii) {
    if(mod_expand)
      api_instances(mods[ii], results_arr);
    else 
      module_instances(mods[ii], &results_arr);
  }
}

int Interpreter::InstancesCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[])
{
    CHECK_ARG_NUM("instances",0);

    Interpreter *i = (Interpreter *)cd;

    int start = 1;
    int mod_expand = 1;

    if(argc > 1){
      char const *arg1 = argv[1];
      int len = strlen(arg1);
      if(len >= 3 && strncmp(arg1, "-included", len) == 0){
	mod_expand = 0;
	start = 2;
      }
    }
    symbolArr src;
    if(i->ParseArguments(start, argc, argv, src) != 0)
	return TCL_ERROR;
    DI_object results;
    DI_object_create(&results);
    symbolArr& results_arr = *get_DI_symarr(results);
    
    get_instances(mod_expand, src, results_arr);

    if(results_arr.size() > 1)
      results_arr.unsort();

    i->SetDIResult(results);
    return TCL_OK;
}

static int inst_arr_num;

void reset_inst_arr_num()
{
    inst_arr_num = 0;
}

int Interpreter::ValidateInstArrCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[])
{
    CHECK_ARG_NUM("validate_inst_arr",0);

    Interpreter *i = (Interpreter *)cd;

    symbolArr res;
    if(i->ParseArguments(1, argc, argv, res) != 0)
	return TCL_ERROR;

    genString label;
    inst_arr_num ++;    
    label.printf("instances.%d",inst_arr_num);
    cmd_validate(label,res);

    return TCL_OK;
}

void declarations_in_mod(const symbolArr& arr, symbolPtr&mod, symbolArr&res);

void fill_array_with_xref_query(symbolArr&arr, int link, symbolArr&res);
int  Interpreter::DeclarationsCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[])
{
    CHECK_ARG_NUM("declarations",0);

    Interpreter *i = (Interpreter *)cd;
    symbolArr src;
    if(i->ParseArguments(1, argc, argv, src) != 0)
	return TCL_ERROR;

    symbolArr mods;
    symbolArr locs;
    convert_projects_and_modules(src, mods, locs, 0);

    DI_object results;
    DI_object_create(&results);
    symbolArr& res_arr = *get_DI_symarr(results);
 
    int sz = locs.size();
    for(int ii=0; ii<sz; ++ii){
      symbolPtr& sym = locs[ii];
      symbolArr refs;
      symbolArr arr;
      arr.insert_last(sym);
      fill_array_with_xref_query(arr, SHOW_REF_FILE, refs);

      int no_mod = refs.size();

      for(int jj=0; jj<no_mod; ++jj){
        symbolPtr&ref = refs[jj];
	declarations_in_mod(arr, ref, res_arr);
      }
    }
    ddSelector sel;
    sel.add(DD_LOCAL);
    sz = mods.size();
    for(int mm=0; mm<sz; ++mm){
      symbolPtr& mod = mods[mm];
      locs.removeAll();
      symbolArr fff;
      fff.insert_last(mod);
      cli_defines_or_uses(fff, locs, sel, 0);
      declarations_in_mod(locs, mod, res_arr);
    }

    i->SetDIResult(results);
    return TCL_OK;
}

int  Interpreter::WhereDeclaredCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[])
{
  Interpreter*i = (Interpreter*) cd;
  return i->WhereDeclaredCmd(argc, argv);
}

int  Interpreter::GetInstanceScopeCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[])
{
    CHECK_ARG_NUM("get_instance_scope",0);

    Interpreter *i = (Interpreter *)cd;
    symbolArr res;
    if(i->ParseArguments(1, argc, argv, res) != 0)
	return TCL_ERROR;
    DI_object results;
    DI_object_create(&results);
    symbolArr *results_arr = get_DI_symarr(results);
    symbolPtr sym;
    symbolSet results_set;
    ForEachS(sym, res) {
      if (sym.is_instance()) {
	symbolPtr sc = sym.scope();
	if (sc.isnotnull())
	  results_set.insert(sc);
      }
    }
    *results_arr = results_set;
    i->SetDIResult(results);
    return TCL_OK;
}

int  Interpreter::GetInstanceSymbolCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[])
{
    CHECK_ARG_NUM("get_instance_symbol",0);

    Interpreter *i = (Interpreter *)cd;
    symbolArr res;
    if(i->ParseArguments(1, argc, argv, res) != 0)
	return TCL_ERROR;
    DI_object results;
    DI_object_create(&results);
    symbolArr *results_arr = get_DI_symarr(results);
    symbolPtr sym;
    symbolSet results_set;
    ForEachS(sym, res) {
      if (sym.is_instance()) {
	symbolPtr sc = sym.sym_of_instance();
	if (sc.isnotnull())
	  results_set.insert(sc);
      }
    }
    *results_arr = results_set;
    i->SetDIResult(results);
    return TCL_OK;
}

int Interpreter::InstanceFromOffsetCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[])
{
    CHECK_ARG_NUM("instance_from_offset", 0);

    Interpreter *i = (Interpreter *)cd;

    if ((argc < 3) && ((argc % 2) == 0)) {
      Tcl_AppendResult(i->interp, "Usage: instance_from_offset <filename> <offset> ...", NULL);
      return TCL_ERROR;
    }

    DI_object results;
    DI_object_create(&results);
    symbolArr *results_arr = get_DI_symarr(results);

    for(int current_pair = 1 ; 
	current_pair < argc ; 
	current_pair+=2 ) {
	symbolPtr result = MetricElement::get_instance(argv[current_pair], 
						       OSapi_atoi(argv[current_pair+1]));
	if (result.isnull()) {
	    Tcl_AppendResult(i->interp, "Bad filename or offset", NULL);
	    return TCL_ERROR;
	}
	
	results_arr->insert_last(result);
    }
    i->SetDIResult(results);
    return TCL_OK;
}

int Interpreter::ArgumentOfCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[])
{
    CHECK_ARG_NUM("argument_of", 0);

    Interpreter *i = (Interpreter *)cd;
    symbolArr res;
    if(i->ParseArguments(1, argc, argv, res) != 0)
	return TCL_ERROR;
    DI_object target;
    DI_object_create(&target);
    symbolArr & trg_arr =  *get_DI_symarr(target);
    for(int ii = 0; ii < res.size(); ii++){
	symbolPtr sym = res[ii];
	if(sym.get_kind() == DD_STRING){
	    sym.get_link(friends_of, trg_arr);
	}
    }
    i->SetDIResult(target);
    return TCL_OK;
}

int Interpreter::FriendOfCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[])
{
    CHECK_ARG_NUM("friend_of", 0);

    Interpreter *i = (Interpreter *)cd;
    symbolArr res;
    if(i->ParseArguments(1, argc, argv, res) != 0)
	return TCL_ERROR;
    DI_object target;
    DI_object_create(&target);
    symbolArr & trg_arr =  *get_DI_symarr(target);
    for(int ii = 0; ii < res.size(); ii++){
	symbolPtr sym = res[ii];
	if(sym.get_kind() == DD_CLASS || sym.get_kind() == DD_FUNC_DECL){
	    sym.get_link(friends_of, trg_arr);
	}
    }
    i->SetDIResult(target);
    return TCL_OK;
}

extern void cli_find_packages(symbolArr&src_arr, symbolArr& trg_arr);
int Interpreter::PackagesCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[])
{
    CHECK_ARG_NUM("packages", 0);
    Interpreter *i = (Interpreter *)cd;
    symbolArr res;
    if(i->ParseArguments(1, argc, argv, res) != 0)
	return TCL_ERROR;

    DI_object target;

    DI_object_create(&target);
    symbolArr & trg_arr =  *get_DI_symarr(target);
    cli_find_packages(res, trg_arr);

    i->SetDIResult(target);
    return TCL_OK;
}


/////////////////////////////////////////////////////////////////////////////
//  Interface to QAR: Begin

//The following code deals with QAR_Driver. Since Yuri's QAEngine does
//not use this interface(pset_server/aset will not receive QA commands) it is being '#if 0'ed below. 

#if 0

#ifdef _WIN32
    #include <winsock.h>
#else
    #include <signal.h>
    #include <arpa/inet.h>
    #include <netinet/tcp.h>
    #include <netdb.h>
    #include <sys/socket.h>
    #include <fcntl.h>
    #include <netinet/in.h>
    #include <unistd.h>
    #include <sys/types.h>
    #include <sys/wait.h>
    #define closesocket(sock) close(sock)
#endif

int NameServerGetService(char const *name, int &service_port, int &tcp_addr);
int SendInteger(int, int);
int SendBuffer(int, char const *, int);
int ReceiveInteger(int, int *);
int ReceiveBuffer(int, char *, int);
int SendCommand(int socket, unsigned char cmd);

int QAR_socket_connect();
int QAR_send_command(char const *command);
int QAR_recieve_reply(genString &reply);

extern char *cli_service_name;
static int QAR_socket = 0;
static unsigned char packet_num = 0;
static int QAR_session_num = 0;
static genString QAR_service_name;
static genString QAR_root_name;

static bool
isQARbuild(char const *str)
{
  if (!str)
    return false;

  int size = strlen(str);

  if (size != strlen("??_??_????_??_??_??"))
    return false;

  for (int k = 0; k < size; ++k) {
    switch (k) {
    case 2:
    case 5:
    case 10:
    case 13:
    case 16:
      if (*str++ != '_')
        return false;
      break;
    default:
      if (!isdigit(*str++))
        return false;
      break;
    }
  }

  return true;
}

//-----------
void
QAR_deleteSession(genString &QAR_session)
{
  if (QAR_socket && !QAR_session.is_null()) {
    genString QAR_command;
    QAR_command.printf("close\t%s", QAR_session);

    if (QAR_send_command((char *) QAR_command.str()) <= 0)
      QAR_socket = 0;
    else {
      genString reply;
      QAR_recieve_reply(reply);
    }
  }
}

//-----------
int
QAR_Cmd(ClientData cd,
        Tcl_Interp *interp,
        int argc,
        char const *argv[])
{
  Interpreter *i = (Interpreter *) cd;

  if (argc < 2) {
    Tcl_SetResult(i->interp, "No subcommand to QAR is given", TCL_VOLATILE);
    return TCL_ERROR;
  }

  if (QAR_socket == 0 && QAR_socket_connect() == 0) {
    Tcl_SetResult(i->interp, "Unable to start QAR_driver service", TCL_VOLATILE);
    return TCL_ERROR;
  }

  genString QAR_command = argv[1];

  for (int iarg= 2; iarg < argc; ++iarg) {
    QAR_command += "\t";
    QAR_command += argv[iarg];
  }

  //---- session manager
  genString QAR_session;
  char const *session = i->QAR_session;

  if (!strcmp(argv[1], "open") || !strcmp(argv[1], "fopen")) {
    if (argc < 4) {
      QAR_session.printf("%s:%d", QAR_service_name, ++QAR_session_num);
      QAR_command += "\t";
      QAR_command += QAR_session;
    } else
      QAR_session = argv[3];

    if (session)
      i->QAR_session += "\t";

    i->QAR_session += QAR_session;
  } else if (!strcmp(argv[1], "fcreate")) {
    if (argc < 5) {
      QAR_session.printf("%s:%d", QAR_service_name, ++QAR_session_num);
      QAR_command += "\t";
      QAR_command += QAR_session;
    } else
      QAR_session = argv[4];

    if (session)
      i->QAR_session += "\t";

    i->QAR_session += QAR_session;
  } else if (isQARbuild(argv[2])) {
    QAR_session.printf("%s\v", argv[2]);

    if (!session)
      i->QAR_session.printf("%s%d", (char *) QAR_session, ++QAR_session_num);
    else if (!strstr(session, (char *) QAR_session))
      i->QAR_session.printf("%s\t%s%d", session, (char *) QAR_session, ++QAR_session_num);

    if (!strcmp(argv[1], "lock") || !strcmp(argv[1], "unlock")) {
      session = (char *) i->QAR_session;
      char *ID = strstr(session, (char *) QAR_session);

      if (ID) {
        ID += QAR_session.length();
        char *p = strchr(ID, '\t');

        if (p)
          *p = '\0';

        QAR_command += "\t";
        QAR_command += ID;

	if (p)
          *p = '\t';
      }
    }
  }

  if (QAR_send_command((char *) QAR_command.str()) <= 0) {
    QAR_socket = 0;
    Tcl_SetResult(i->interp, "Communication error", TCL_VOLATILE);
    return TCL_ERROR;
  }

  if (!strcmp(argv[1], "exit")) {
    QAR_socket = 0;
    return TCL_OK;
  }

  genString reply;
  int status = QAR_recieve_reply(reply);
  char const *result = reply.str();
  Tcl_SetResult(i->interp, (char *)result, TCL_VOLATILE);

  if (status <= 0)
    return TCL_ERROR;

  return TCL_OK;
}

//-------------------------------------------------------
#define DATA_PACKET                 1
#define DATA_FINAL_PACKET           2
#define STDOUT_DISH_STREAM_CMD_TYPE 4

int
QAR_socket_connect()
{
  int nPort = 0;
  int nTcpAddr = 0;
  QAR_service_name = cli_service_name;
  QAR_service_name += "_QAR";
  QAR_root_name = customize::getStrPref("QAR_Root");

  if (QAR_root_name.is_null() || QAR_root_name == "") {
    QAR_root_name = customize::getStrPref("ADMINDIR");

    if (!QAR_root_name.is_null() && QAR_root_name != "") {
#ifdef _WIN32
      QAR_root_name += "\\..\\QA";
#else
      QAR_root_name += "/../QA";
#endif
    }
  }

  if (QAR_root_name.is_null() || QAR_root_name == "")
    QAR_root_name = OSapi_getenv("QAR_Root");

  if (QAR_root_name.is_null() || QAR_root_name == "") {
    QAR_root_name = OSapi_getenv("ADMINDIR");

    if (!QAR_root_name.is_null() && QAR_root_name != "") {
#ifdef _WIN32
      QAR_root_name += "\\..\\QA";
#else
      QAR_root_name += "/../QA";
#endif
    }
  }

  if (QAR_root_name.is_null() || QAR_root_name == "")
    return 0;

  if (NameServerGetService((char *) (QAR_service_name.str()), nPort, nTcpAddr) == 0) {
    nPort = 0;
    nTcpAddr = 0;
    //--- Start QAR_driver
    genString command;
#ifdef _WIN32
    STARTUPINFO si;
    PROCESS_INFORMATION pi;     
    char buf[1024];
    genString QARserverFileDir;

    if (GetEnvironmentVariable("PSETHOME", buf, 1024) != 0) {
      QARserverFileDir = buf;
      QARserverFileDir += "\\Bin\\";
    } else
      QARserverFileDir = "C:\\Discover\\Bin\\";

    command.printf("\"%sQAR_driver.exe\" -service %s -root \"%s\"", QARserverFileDir, QAR_service_name, QAR_root_name);

    memset(&si, 0, sizeof(STARTUPINFO));
    si.wShowWindow = SW_HIDE;
    si.cb = sizeof(STARTUPINFO);

    if(::CreateProcess(NULL, (char *) command,
                       NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL,
                       &si, &pi) == FALSE)
      return 0;

    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
#else
    char *dir = getenv("PSETHOME");

    if (!dir) {
      command = "nameserv";
    } else {
      command.printf("%s/bin/QAR_driver -service %s -root %s", dir, QAR_service_name.str(), QAR_root_name.str());
    }

    char const *arglist[2];
    arglist[0] = "QAR_driver";
    arglist[1] = NULL;
//    const pid_t pid = vfork();

//    if (pid == 0) {
//      OSapi_closeChildren();
//      setpgid(0, 0);

      if (OSapi_execv((char *) command, (char *const*) arglist) < 0)
	return 0;
//    }
#endif
    //-------- wait for QAR_driver 
    for (int iTry = 0; iTry < 200; ++iTry) {
      if (NameServerGetService((char *) (QAR_service_name.str()), nPort, nTcpAddr))
        break;
#ifdef _WIN32
      Sleep(1000);
#else
      sleep(1);
#endif
    }
  }

  if (nPort && nTcpAddr) {
    struct sockaddr_in serv_addr;
    serv_addr.sin_family      = AF_INET;
    serv_addr.sin_port        = htons(nPort);
    serv_addr.sin_addr.s_addr = htonl(nTcpAddr);

    /* Create the socket. */
    int sock = socket(PF_INET, SOCK_STREAM, 0);
 
    if (sock > 0) {
      /* Give the socket a name. */
      if (connect(sock, (struct sockaddr *) &serv_addr, sizeof(sockaddr_in)) < 0)
        closesocket(sock);
      else {
        int tmp_switch = 1;
        setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (char *) &tmp_switch, sizeof(tmp_switch));
        QAR_socket = sock;    
      }
    }
  }

  return QAR_socket;
}

//-----------
int
QAR_send_command(char const *command)
{
  unsigned char packet_type = STDOUT_DISH_STREAM_CMD_TYPE;
  ++packet_num;

  if (send(QAR_socket, (char const *) &packet_num, 1, 0) <= 0)
    return 0;
 
  if (send(QAR_socket, (char const *) &packet_type, 1, 0) <= 0)
    return 0;

  int len = strlen(command) + 1;
 
  if (SendInteger(QAR_socket, len) <= 0)
    return 0;
 
  return
    SendBuffer(QAR_socket, command, len);
}

//-----------
int
QAR_recieve_reply(genString &reply)
{
  if (QAR_socket <= 0)
    return 0;

  char packet_type;
  unsigned char result_code;

  do {
    unsigned char packet_num;

    if (recv(QAR_socket, (char *) &packet_num, 1, 0) <= 0)
      return 0;

    if (recv(QAR_socket, &packet_type, 1, 0) <= 0)
      return 0;

    if (recv(QAR_socket, (char *) &result_code, 1, 0) <= 0)
      return 0;

    int len;

    if (ReceiveInteger(QAR_socket, &len) <= 0)
      return 0;

    char *tmp = new char[len];

    if (ReceiveBuffer(QAR_socket, tmp, len) <= 0) {
      delete [] tmp;
      return 0;
    } else {
      reply += tmp;
      delete [] tmp;
    }
  } while (packet_type != DATA_FINAL_PACKET);

  if (result_code != 0)
    return 0;

  return 1;
}


//---------------------------------------------
#define NS_PORT 28948

#define REGISTER_SERVER   0
#define LIST_OF_SERVERS   1
#define FIND_SERVER       2
#define UNREGISTER_SERVER 3
#define STOP              4 

/*
int
ConnectToNameServer(void)
{
  struct sockaddr_in serv_addr;
  char buf[1024];

  if (gethostname(buf, 1024) != 0)
    return -1;

  struct hostent *ent = gethostbyname(buf);

  if (ent == NULL)
    return -1;

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port   = htons (NS_PORT);
  serv_addr.sin_addr   = *(in_addr *)*ent->h_addr_list;

  // Create the socket.
  int sock = socket(PF_INET, SOCK_STREAM, 0);

  if (sock < 0)
    return sock;

  // Give the socket a name.
  if (connect(sock, (struct sockaddr *) &serv_addr, sizeof(sockaddr_in)) < 0) {
    closesocket(sock);
    return -1;
  }

  return sock;
}

// Function connects to name server and asks if for specified service
// returns 1 on success and sets up "service_port" and "tcp_addr"
// returns 0 if no service found
// returns -1 if unable to connect to name server
int
NameServerGetService(char const *name,
                     int &service_port,
                     int &tcp_addr)
{
  unsigned char len_buf[4];
  int sock = ConnectToNameServer();

  if (sock < 0)
    return -1;

  if (!SendCommand(sock, FIND_SERVER))
    return -1;

  if (!SendInteger(sock, strlen(name))) {
    closesocket(sock);
    return -1;
  }

  if (!SendBuffer(sock, (char*) name, strlen(name))) {
    closesocket(sock);
    return -1;
  }

  int res = recv(sock, (char *) len_buf, 1, 0);

  if (res <= 0) {
    closesocket(sock);
    return -1;
  }

  if (len_buf[0] == 0) {
    closesocket(sock);
    return 0;
  }

  res = recv(sock, (char *) len_buf, 4, 0);

  if (res <= 0) {
    closesocket(sock);
    return -1;
  }

  service_port = len_buf[0] | ((unsigned)len_buf[1] << 8) | ((unsigned)len_buf[2] << 16) | ((unsigned)len_buf[3] << 24);
  res          = recv(sock, (char *) len_buf, 4, 0);

  if (res <= 0) {
    closesocket(sock);
    return -1;
  }

  tcp_addr = len_buf[0] | ((unsigned)len_buf[1] << 8) | ((unsigned)len_buf[2] << 16) | ((unsigned)len_buf[3] << 24);
  closesocket(sock);
  return 1;
}
*/

#endif // #if 0

//  Interface to QAR: End
/////////////////////////////////////////////////////////////////////////////

static int add_commands()
{
  new cliCommand("get_declarations", get_declarationsCmd);
  new cliCommand("defines_or_uses", defines_or_usesCmd);
#if 0
  //This command will not work with QAEngine.
  new cliCommandInfo("qa", QAR_Cmd);
#endif
  return 0;
}

static int add_commands_dummy = add_commands();
