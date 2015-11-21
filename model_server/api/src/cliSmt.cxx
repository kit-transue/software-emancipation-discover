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
#include "cLibraryFunctions.h"
#include "machdep.h"
#include "general.h"
#include "genError.h"
#include "tcl.h"
#include "Interpreter.h"
#include "api_internal.h"
#include "charstream.h"
#include "genString.h"
#include "ddKind.h"
#include "objArr.h"
#include "objOperate.h"
#include "ddict.h"
#include "_Xref.h"
#include "xrefSymbol.h"
#include "smtMarker.h"
#include "smtRegion.h"
#include "symbolSet.h"
#include "dfa.h"
#include "dfa_db.h"

static char const *tok_syntax [] = {
    "No symbol definition.",     // ,0
    "",     // SMT_comment,1
    "",     // SMT_m_include,2
    "",     // SMT_m_define,3
    "",     // SMT_m_if,4
    "",     // SMT_m_else,5
    "",     // SMT_m_endif,6
    "",     // SMT_m_undef,7
    "",     // SMT_m_pragma,8
    "",     // SMT_m_gen,9
    "",     // SMT_file,10
    "",     // SMT_decl,11
    "",     //                        ,12
    "",     // SMT_fdecl,13
    "",     // SMT_pdecl,14
    "",     // SMT_cdecl,15
    "",     // SMT_edecl,16
    "",     // SMT_cbody,17
    "",     // SMT_ebody,18
    "",     // SMT_sdecl,19
    "",     // SMT_else,20
    "",     // SMT_gen,21
    "",     // SMT_if,22
    "",     // SMT_ifelse,23
    "",     // SMT_for,24
    "",     // SMT_while,25
    "",     // SMT_do,26
    "",     // SMT_block,27
    "",     // SMT_switch,28
    "",     // SMT_case,29
    "",     // SMT_default,30
    "",     // SMT_break,31
    "",     // SMT_continue,32
    "",     // SMT_return,33
    "",     // SMT_goto,34
    "",     // SMT_label,35
    "",     // SMT_empty,36
    "",     // SMT_expr,37
    "",     // SMT_macrostmt,38
    "",     // SMT_dowhile,39
    "",     // SMT_stmt,40
    "",     // SMT_nstdloop,41
    "",     // SMT_clause,42
    "",     // SMT_class,43
    "",     // SMT_else_clause,44
    "",     // SMT_else_if_clause,45
    "",     // SMT_then_clause,46
    "",     // SMT_case_clause,47
    "",     // SMT_superclass,48
    "",     // SMT_macrocall,49
    "",     // SMT_title,50
    "",     // SMT_typedef,51
    "",     // SMT_gdecl, 52
    "",     // SMT_ldecl, 53
    "",     // SMT_mem_decl, 54
    "",     // SMT_enum_field, 55
    "",     // SMT_list_decl, 56
    "",     // SMT_declspec, 57
    "",     // SMT_list_pdecl, 58
    "",     // SMT_subtitle,59
    "",     // SMT_fdef,60
    "",     // SMT_macroarg,61
    "",     // SMT_try_catch,62
    "",     // SMT_try_clause,63
    "",     // SMT_catch_clause,64
    "",     //                 ,65
    "",     //                 ,66
    "",     //                 ,67
    "",     //                 ,68
    "",     //                 ,69
    "",     //                 ,70
    "",     //                 ,71
    "",     //                 ,72
    "",     //                 ,73
    "",     //                 ,74
    "",     //                 ,75
    "",     //                 ,76
    "",     //                 ,77
    "",     //                 ,78
    "",     //                 ,79
    "",     //                 ,80
    "",     //                 ,81
    "",     //                 ,82
    "",     //                 ,83
    "",     //                 ,84
    "",     //                 ,85
    "",     //                 ,86
    "",     //                 ,87
    "",     //                 ,88
    "",     //                 ,89
    "",     //                 ,90
    "",     //                 ,91
    "",     //                 ,92
    "",     //                 ,93
    "",     //                 ,94
    "",     //                 ,95
    "",     //                 ,96
    "",     //                 ,97
    "",     //                 ,98
    "",     //                 ,99
    "",     //                 ,100
    "",     // SMT_token,101
    "",     // SMT_group,102
    "",     // SMT_temp,103
    "",     //                 ,104
    "",     //                 ,105
    "",     //                 ,106
    "",     //                 ,107
    "",     //                 ,108
    "",     //                 ,109
    "",     // SMTT_ref,110
    "KEYWORD",     // SMTT_kwd,111
    "",     // SMTT_ident,112
    "OPERATOR",     // SMTT_op,113
    "",     //                ,114
    "MACRO",     // SMTT_macro,115
    "CONSTANT",     // SMTT_const,116
    "STRING LITERAL",     // SMTT_string,117
    "PUNCTUATION (block start)",     // SMTT_begin,118
    "PUNCTUATION (block end)",     // SMTT_end,119
    "",     //                ,120
    "",     //                ,121
    "",     // SMTT_untok, 122
    "",     // SMTT_lb, 123
    "",     // SMTT_el, 124
    "COMMENT",     // SMTT_commentl,125
    "COMMENT",     // SMTT_comment,126
    "PUNCTUATION (grouping)",     // SMTT_grouping,127	 '('   ','   ')'
    "",     //                ,128
    "",     //                ,129
    "",     // SMT_begcomm,130
    "",     // SMT_endcomm,131
    "KEYWORD (access specifier)",     // SMTT_ppp,132
};


smtTree   *dis_get_first_node_on_line(char const *fname, int lin); // defined in ste_test_smt.C
void      ste_get_subtrees (objTree *st, objTree *en, objArr& the_ar, int uppest);
void      ste_smart_view_update( smtHeaderPtr h, int with_text );
char      *ste_smt_get_native_string( smtTreePtr nd );

smtTree   *inst_get_token( symbolPtr& sym );
smtTree   *inst_get_statement( symbolPtr& sym );
smtTree   *GetSmtFromMarker(symbolPtr& sym);
smtTree   *GetSmtFromMarker(smtMarker *sm);
smtRegion *GetRegion(symbolPtr& sym);

symbolPtr create_smtMarker(smtTree *smt)
{
    symbolPtr sym = new smtMarker(smt);
    return sym;
}

symbolPtr create_smtRegion(smtTree *t1, smtTree *t2)
{
    symbolPtr sym = new smtRegion(t1, t2);
    return sym;
}

void get_instances_from_smt(symbolPtr& s, symbolArr& res)
{
    smtTree *smt = GetSmtFromMarker(s);
    if(smt){
	smtHeader *sh = (smtHeader *)smt->get_header();
	MetricElement::get_instances(sh->get_name(), smt->tbeg, smt->tbeg + smt->tlth, res);
    }
}

void get_instances_from_region(symbolPtr& s, symbolArr& res)
{
    smtRegion *reg = GetRegion(s);
    if(reg) {
	smtTree *smt_start = GetSmtFromMarker((smtMarker *)reg);
	smtTree *smt_end   = GetSmtFromMarker((smtMarker *)reg->end);
	if(smt_start && smt_end){
	    smtHeader *sh = (smtHeader *)smt_start->get_header();
	    MetricElement::get_instances(sh->get_name(), smt_start->tbeg, smt_end->tbeg + smt_end->tlth, res);
	}
    }
}

static int GetSmtCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[])
{
    Interpreter *i = (Interpreter *)cd;
    symbolArr res;
    if(i->ParseArguments(1, argc, argv, res) != 0)
	return TCL_ERROR;
    DI_object results;
    DI_object_create(&results);
    symbolArr *results_arr = get_DI_symarr(results);
    for(int ii = 0; ii < res.size(); ii++){
	if(res[ii].is_instance()){
	    smtTree *smt = inst_get_token(res[ii]);
	    if(smt){
		symbolPtr sym = create_smtMarker(smt);
		results_arr->insert_last(sym);
	    }	    
	} else {
	    // If we have smt already then return it
	    if(GetSmtFromMarker(res[ii]) != NULL){
		results_arr->insert_last(res[ii]);
	    } else {
		symbolPtr xsym = res[ii].get_xrefSymbol();
		if(!xsym.xrisnull()){
		    if(xsym.is_loaded_or_cheap()){
			appPtr app = xsym.get_def_app();
			if(app && is_smtHeader(app)){
			    if(xsym.get_kind() == DD_MODULE){
				smtTree *root = (smtTree *)app->get_root();
				symbolPtr sym = create_smtMarker(root);
				results_arr->insert_last(sym);
			    } else {
				ddElement *dd = xsym.get_dds(app);
				if(dd){
				    smtTree *smt = (smtTree *)get_relation(def_smt_of_dd, dd);
				    if(smt){
					symbolPtr sym = create_smtMarker(smt);
					results_arr->insert_last(sym);
				    }
				}
			    }
			}
		    }
		}
	    }
	}
    }
    i->SetDIResult(results);
    return TCL_OK;
}

static char const* ATT_smt_text( symbolPtr& sym)
{
    static ocharstream os;

    smtTree *smt = GetSmtFromMarker(sym);
    if(smt) {
      os.reset();
      smt->send_string(os);
      os << '\0';
      char*txt =  os.ptr();
      if(txt)
	txt[strlen(txt)-1] = '\0';  // kill appended space
      return txt;
    }
    return 0;
}

static char const* ATT_smt_native_text( symbolPtr& sym)
{
    static char*txt=NULL;

    if(txt) {
      delete txt;
      txt = NULL;
    }
    smtTree *smt = GetSmtFromMarker(sym);
    if(smt) {
       txt = ste_smt_get_native_string(smt);
    }
    return txt;
}

static int SmtGetParentCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[])
{
    Interpreter *i = (Interpreter *)cd;
    symbolArr res;
    if(i->ParseArguments(1, argc, argv, res) != 0)
	return TCL_ERROR;
    DI_object results;
    DI_object_create(&results);
    symbolArr *results_arr = get_DI_symarr(results);
    for(int ii = 0; ii < res.size(); ii++){
	symbolPtr sym = res[ii];
	smtTree *smt;
	if((smt = GetSmtFromMarker(sym)) != NULL){
	    smtTree *new_smt = (smtTree *)smt->get_parent();
	    if(new_smt){
		symbolPtr new_sym = create_smtMarker(new_smt);
		results_arr->insert_last(new_sym);
	    }
	}
    }
    i->SetDIResult(results);
    return TCL_OK;
}

static int SmtGetFirstCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[])
{
    Interpreter *i = (Interpreter *)cd;
    symbolArr res;
    if(i->ParseArguments(1, argc, argv, res) != 0)
	return TCL_ERROR;
    DI_object results;
    DI_object_create(&results);
    symbolArr *results_arr = get_DI_symarr(results);
    for(int ii = 0; ii < res.size(); ii++){
	symbolPtr sym = res[ii];
	smtTree *smt;
	if((smt = GetSmtFromMarker(sym)) != NULL){
	    smtTree *new_smt = (smtTree *)smt->get_first();
	    if(new_smt){
		symbolPtr new_sym = create_smtMarker(new_smt);
		results_arr->insert_last(new_sym);
	    }
	}
    }
    i->SetDIResult(results);
    return TCL_OK;
}

static int SmtGetLastCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[])
{
    Interpreter *i = (Interpreter *)cd;
    symbolArr res;
    if(i->ParseArguments(1, argc, argv, res) != 0)
	return TCL_ERROR;
    DI_object results;
    DI_object_create(&results);
    symbolArr *results_arr = get_DI_symarr(results);
    for(int ii = 0; ii < res.size(); ii++){
	symbolPtr sym = res[ii];
	smtTree *smt;
	if((smt = GetSmtFromMarker(sym)) != NULL){
	    smtTree *new_smt = (smtTree *)smt->get_last();
	    if(new_smt){
		symbolPtr new_sym = create_smtMarker(new_smt);
		results_arr->insert_last(new_sym);
	    }
	}
    }
    i->SetDIResult(results);
    return TCL_OK;
}

static int SmtGetNextCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[])
{
    Interpreter *i = (Interpreter *)cd;
    symbolArr res;
    if(i->ParseArguments(1, argc, argv, res) != 0)
	return TCL_ERROR;
    DI_object results;
    DI_object_create(&results);
    symbolArr *results_arr = get_DI_symarr(results);
    for(int ii = 0; ii < res.size(); ii++){
	symbolPtr sym = res[ii];
	smtTree *smt;
	if((smt = GetSmtFromMarker(sym)) != NULL){
	    smtTree *new_smt = (smtTree *)smt->get_next();
	    if(new_smt){
		symbolPtr new_sym = create_smtMarker(new_smt);
		results_arr->insert_last(new_sym);
	    }
	}
    }
    i->SetDIResult(results);
    return TCL_OK;
}

static int SmtGetPrevCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[])
{
    Interpreter *i = (Interpreter *)cd;
    symbolArr res;
    if(i->ParseArguments(1, argc, argv, res) != 0)
	return TCL_ERROR;
    DI_object results;
    DI_object_create(&results);
    symbolArr *results_arr = get_DI_symarr(results);
    for(int ii = 0; ii < res.size(); ii++){
	symbolPtr sym = res[ii];
	smtTree *smt;
	if((smt = GetSmtFromMarker(sym)) != NULL){
	    smtTree *new_smt = (smtTree *)smt->get_prev();
	    if(new_smt){
		symbolPtr new_sym = create_smtMarker(new_smt);
		results_arr->insert_last(new_sym);
	    }
	}
    }
    i->SetDIResult(results);
    return TCL_OK;
}

static int SmtGetRootCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[])
{
    Interpreter *i = (Interpreter *)cd;
    symbolArr res;
    if(i->ParseArguments(1, argc, argv, res) != 0)
	return TCL_ERROR;
    DI_object results;
    DI_object_create(&results);
    symbolArr *results_arr = get_DI_symarr(results);
    for(int ii = 0; ii < res.size(); ii++){
	symbolPtr sym = res[ii];
	smtTree *smt;
	if((smt = GetSmtFromMarker(sym)) != NULL){
	    smtTree *new_smt = (smtTree *)smt->get_root();
	    if(new_smt){
		symbolPtr new_sym = create_smtMarker(new_smt);
		results_arr->insert_last(new_sym);
	    }
	}
    }
    i->SetDIResult(results);
    return TCL_OK;
}

static int RegionCreateCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[])
{
    Interpreter *i = (Interpreter *)cd;

    if(argc != 3){
	Tcl_SetResult(i->interp, (char *)"region_create: wrong number of arguments", TCL_VOLATILE);
	return TCL_ERROR;
    }
    symbolArr arg1;
    symbolArr arg2;
    if(i->ParseArgString(argv[1], arg1) != 0 || i->ParseArgString(argv[2], arg2) != 0){
	Tcl_AppendResult(i->interp, "region_create: invalid arguments\n", NULL);
	return TCL_ERROR;
    }
    if(arg1.size() == 0 || arg2.size() == 0){
	Tcl_AppendResult(i->interp, "region_create: empty arrays not accepted\n", NULL);
	return TCL_ERROR;
    }
    DI_object results;
    DI_object_create(&results);
    symbolArr *results_arr = get_DI_symarr(results);

    symbolPtr sym1 = arg1[0];
    symbolPtr sym2 = arg2[0];
    smtTree *smt1  = GetSmtFromMarker(sym1);
    smtTree *smt2  = GetSmtFromMarker(sym2);
    if(smt1 && smt2){
	if(smt1->get_header() != smt2->get_header()){
	    Tcl_AppendResult(i->interp, "region_create: trees should be from same module", NULL);
	    DI_object_delete(results);
	    return TCL_ERROR;
	}
	if(smt1->tbeg > smt2->tbeg){
	    symbolPtr sym = create_smtRegion(smt2, smt1);
	    results_arr->insert_last(sym);
	} else {
	    symbolPtr sym = create_smtRegion(smt1, smt2);
	    results_arr->insert_last(sym);
	}
    }
    i->SetDIResult(results);
    return TCL_OK;
}

static int RegionRestoreCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[])
{
    Interpreter *i = (Interpreter *)cd;

    if(argc < 3){
	Tcl_SetResult(i->interp, (char *)"Usage: region_restore start_pos length module", TCL_VOLATILE);
	return TCL_ERROR;
    }
    int start_pos = atoi(argv[1]);
    int length    = atoi(argv[2]);
    symbolArr modules;
    if(i->ParseArguments(3, argc, argv, modules) != 0)
	return TCL_ERROR;
    if(modules.size() == 0 || modules[0].get_kind() != DD_MODULE){
	Tcl_SetResult(i->interp, (char *)"Usage: region_restore start_pos length module", TCL_VOLATILE);
	return TCL_ERROR;
    }
    DI_object results;
    DI_object_create(&results);
    symbolArr *results_arr = get_DI_symarr(results);
    symbolPtr xsym         = modules[0].get_xrefSymbol();
    if(xsym.isnotnull()){
	appPtr app = xsym.get_def_app();
	if(app && is_smtHeader(app)){
	    smtHeader *header = (smtHeader *)app;
	    smtTree *start    = header->cp_na(start_pos);
	    smtTree *end      = header->cp_na(start_pos + length);
	    if(start && end){
		symbolPtr sym = create_smtRegion(start, end);
		results_arr->insert_last(sym);
	    }
	}
    }
    i->SetDIResult(results);
    return TCL_OK;
}

extern void smt_get_token_definition (smtTree *, genString&);

int
DescribeSmt(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[])
{
    Initialize(DescribeSmt);
    Interpreter *i = (Interpreter *)cd;

    if(argc < 1){
        Tcl_SetResult(i->interp, "Usage: smt_info <smt>", TCL_VOLATILE);
        return TCL_ERROR;
    }
    symbolArr res;
    if(i->ParseArguments(1, argc, argv, res) != 0) {
        return TCL_ERROR;
    }
    symbolPtr sym = res[0];
    smtTree *smt;
    smt = GetSmtFromMarker(sym);
    genString s;
    if(smt!=NULL) {
        smt_get_token_definition(smt,s);
    } else {
        s.printf("Can't describe selection");
    }
    Tcl_AppendResult(interp, s.str(), NULL);
    return TCL_OK;
}

static int DescribeSymbol(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[]) 
{
    Initialize(DescribeSymbol);
    Interpreter *i = (Interpreter *)cd;

    if(argc < 2){
	Tcl_SetResult(i->interp, (char *)"Usage: sym_info <set>", TCL_VOLATILE);
	return TCL_ERROR;
    }
    symbolArr res;
    if(i->ParseArguments(1, argc, argv, res) != 0) {
	return TCL_ERROR;
    }
    if(res.size() == 0){
	Tcl_SetResult(interp, (char *)"", TCL_STATIC);
	return TCL_OK;
    }
    symbolPtr sym = res[0];
    ostringstream os;
    genString txt;
    symbolPtr ddel = sym.get_xrefSymbol();
    if(ddel.isnull()) {
#if 0      // XXX: this code makes no sense.
        char *outstr = (char *)txt;
        outstr = (char *)"No symbol definition.";
        Tcl_AppendResult(interp,(char *)outstr, NULL);
#else
        Tcl_AppendResult(interp, "No symbol definition.", NULL);
#endif
        return TCL_OK;
    } else {
        ddKind kind = ddel->get_kind();
        if(kind != DD_FUNC_DECL) {
	    char const *nm = ddKind_name(kind);
	    if (nm && *nm)
	        os << nm + 3 ;
	} 
        symbolArr temp;
        ddel->get_link(has_type, temp);
        if(temp.size() > 0) {
	    symbolPtr type = temp[0].get_xrefSymbol();
	    if(!type.isnull())
		os << ' ' << type.get_name();
	}

        os << ' ' << ddel.get_name();
      
        char const *def_file = "";
        symbolPtr sym = ddel;
        sym = sym.get_xrefSymbol();
        if (!sym.isnull()) {
	    symbolPtr file = sym->get_def_file();
	    if(! file.isnull())
		def_file = file.get_name();
	}
        os << " ==> " << def_file;
      
        os << ends;
        txt = os.str().c_str();
      
    }
    char *outstr = (char *)txt;
    if(!(outstr && outstr[0]))  outstr = (char *)"No symbol definition.";
    Tcl_AppendResult(interp,(char *)outstr, NULL);
    return TCL_OK;
}

static char const * ATT_smt_type( symbolPtr& sym)
{
  ddKind kind = sym.get_kind();
  if(kind != DD_SMT)
    return NULL;
  
  smtTree *smt = GetSmtFromMarker(sym);
  if(!smt)
    return NULL;

  int type = smt->type;
  char* name = smt_token_name(type);
  return name;
}

static char const * ATT_smt_extype( symbolPtr& sym)
{
    ddKind kind = sym.get_kind();
    if(kind != DD_SMT)
	return NULL;
    
    smtTree *smt = GetSmtFromMarker(sym);
    if(!smt)
	return NULL;
    
    int type = smt->extype;
    char* name = smt_token_name(type);
    return name;
}

char const * ATT_astname (symbolPtr& sym) {
    Initialize(ATT_astname);
    static genString buff = "";
    buff = "\0";


    ddKind kind = sym.get_kind();
    if (kind == DD_AST) {
		astNode *node = astNode_of_sym(sym);
        astXref *axr = sym.get_astXref();
        symbolPtr sym(node,axr);
        char const *name = NULL;
        while(node) {
            symbolPtr sym(node,axr);
            name = sym.get_name();
			if(name && strlen(name)>0) break;
			node = node->get_parent();
		}
		if(node && name) {
			buff=name;
		}
			
	}
    return buff;
}


char const * ATT_astpath (symbolPtr& sym) {
    Initialize(ATT_astpath);
    static genString buff = "";
    buff = "\0";
    ddKind kind = sym.get_kind();
    if (kind == DD_AST) {
        astNode *node = astNode_of_sym(sym);
        astXref *axr = sym.get_astXref();
        char const *name = NULL;
        while(node) {
            tree_code code = (tree_code)node->get_code();
			buff+="(";
            buff+=astNode_code_name(code);
			buff+=")";
            symbolPtr sym(node,axr);
            name = sym.get_name();
			if(name && strlen(name)>0) {
				buff+=name;
			}
			if(node->get_parent()) buff+="->";
			node = node->get_parent();
		}
	}
    return buff;
}


static int add_commands()
{
    new cliCommandInfo("get_smt", GetSmtCmd);
    new cliCommandInfo("smt_first", SmtGetFirstCmd);
    new cliCommandInfo("smt_last", SmtGetLastCmd);
    new cliCommandInfo("smt_next", SmtGetNextCmd);
    new cliCommandInfo("smt_prev", SmtGetPrevCmd);
    new cliCommandInfo("smt_root", SmtGetRootCmd);
    new cliCommandInfo("smt_parent", SmtGetParentCmd);
    new cliCommandInfo("region_create", RegionCreateCmd);
    new cliCommandInfo("region_restore", RegionRestoreCmd);
    new cliCommandInfo("smt_info",  DescribeSmt);
    new cliCommandInfo("sym_info",  DescribeSymbol);

    new funcStringAttribute("smt_text", ATT_smt_text);
    new funcStringAttribute("smt_native_text", ATT_smt_native_text);
    new funcStringAttribute("smt_type" , ATT_smt_type);
    new funcStringAttribute("smt_extype" , ATT_smt_extype);
    new funcStringAttribute("astname", ATT_astname);
    new funcStringAttribute("astpath", ATT_astpath);


    return 0;
}

static int add_commands_dummy = add_commands();

