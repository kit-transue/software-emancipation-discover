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
#include "clipboard.h"
#include "scrapbook.h"
#include "deepCopier.h"
#include "ddict.h"
#include "_Xref.h"
#include "xrefSymbol.h"
#include "smtMarker.h"
#include "smtRegion.h"
#include "symbolSet.h"

smtTree   *dis_get_first_node_on_line(const char *fname, int lin); // defined in ste_test_smt.C
void      ste_get_subtrees (objTree *st, objTree *en, objArr& the_ar, int uppest);
void      ste_smart_view_update( smtHeaderPtr h, int with_text );
char      *ste_smt_get_native_string( smtTreePtr nd );

smtTree   *inst_get_token( symbolPtr& sym );
smtTree   *inst_get_statement( symbolPtr& sym );
smtTree   *GetSmtFromMarker(symbolPtr& sym);
smtTree   *GetSmtFromMarker(smtMarker *sm);
smtRegion *GetRegion(symbolPtr& sym);

appPtr clipboard_app = NULL;

static int SmtReplaceTextCmd(ClientData cd, Tcl_Interp *interp, int argc, char *argv[])
{
    Interpreter *i = (Interpreter *)cd;

    if(argc < 2){
	Tcl_SetResult(i->interp, (char *)"smt_replace_text: wrong number of arguments", TCL_VOLATILE);
	return TCL_ERROR;
    }
    char *new_text = argv[1];
    symbolArr res;
    if(i->ParseArguments(2, argc, argv, res) != 0)
	return TCL_ERROR;
    start_transaction(){
	start_smt_editing();
	for(int ii = 0; ii < res.size(); ii++){
	    symbolPtr sym = res[ii];
	    smtTree *smt;
	    if((smt = GetSmtFromMarker(sym)) != NULL){
		smtHeader *smt_head = (smtHeader *)smt->get_header();
		objInserter oi0(REPLACE, NULL, smt, new_text);
		objInserter ni0(oi0);
		oi0.pt = ni0.pt = (void *)1;
		smt_head->insert_obj_real(&oi0, &ni0, 0);
		smt_head->tidy_tree();
		smt_head->vrn = 1;  /* view update ! */
	    }
	}
	commit_smt_editing();
    }end_transaction();
    Tcl_SetResult(i->interp, new_text, TCL_VOLATILE);
    return TCL_OK;
}

static int SmtInsertTextAfterCmd(ClientData cd, Tcl_Interp *interp, int argc, char *argv[])
{
    Interpreter *i = (Interpreter *)cd;

    if(argc < 2){
	Tcl_SetResult(i->interp, (char *)"smt_insert_text_after: wrong number of arguments", TCL_VOLATILE);
	return TCL_ERROR;
    }
    char *new_text = argv[1];
    symbolArr res;
    if(i->ParseArguments(2, argc, argv, res) != 0)
	return TCL_ERROR;
    start_transaction(){
	start_smt_editing();
	for(int ii = 0; ii < res.size(); ii++){
	    symbolPtr sym = res[ii];
	    smtTree *smt;
	    if((smt = GetSmtFromMarker(sym)) != NULL){
		smtHeader *smt_head = (smtHeader *)smt->get_header();
		objInserter oi(AFTER, NULL, smt, new_text);
		objInserter ni(oi);
		ni.pt = (void *)0;
		oi.pt = (void *)0;
		smt_head->insert_obj_real(&oi, &ni, 0);
		smt_head->tidy_tree();
		smt_head->vrn = 1;  /* view update ! */
	    }
	}
	commit_smt_editing();
    }end_transaction();
    Tcl_SetResult(i->interp, new_text, TCL_VOLATILE);
    return TCL_OK;
}

static int SmtInsertTextBeforeCmd(ClientData cd, Tcl_Interp *interp, int argc, char *argv[])
{
    Interpreter *i = (Interpreter *)cd;

    if(argc < 2){
	Tcl_SetResult(i->interp, (char *)"smt_insert_text_before: wrong number of arguments", TCL_VOLATILE);
	return TCL_ERROR;
    }
    char *new_text = argv[1];
    symbolArr res;
    if(i->ParseArguments(2, argc, argv, res) != 0)
	return TCL_ERROR;
    start_transaction(){
	start_smt_editing();
	for(int ii = 0; ii < res.size(); ii++){
	    symbolPtr sym = res[ii];
	    smtTree *smt;
	    if((smt = GetSmtFromMarker(sym)) != NULL){
		smtHeader *smt_head = (smtHeader *)smt->get_header();
		objInserter oi(BEFORE, NULL, smt, new_text);
		objInserter ni(oi);
		ni.pt = (void *)0;
		oi.pt = (void *)1;
		smt_head->insert_obj_real(&oi, &ni, 0);
		smt_head->tidy_tree();
		smt_head->vrn = 1;  /* view update ! */
	    }
	}
	commit_smt_editing();
    }end_transaction();
    Tcl_SetResult(i->interp, new_text, TCL_VOLATILE);
    return TCL_OK;
}

static appPtr CreateRegApp(smtTree *start, smtTree *end)
{
    objArr_Int regions;
    objArr     nodes;

    Initialize(CreateRegApp);
		
    if(start == end){
	deepCopier cp;
	cp.put_starter_obj(start);
	smtTree* ret_n   = checked_cast(smtTree,obj_copy_deep (*start, &cp, 0));
	smtHeader* ret_h = checked_cast(smtHeader, ret_n->get_header());
	ret_h->tidy_tree();
	return ret_h;
    } else {
	ste_get_subtrees (start, end, nodes, 0);
	int st_index  = 0;
	int st_offset = 0;
	int en_offset = -1;
	int en_index  = nodes.size() - 1;
	regions.insert_last (st_index);
	regions.insert_last (st_offset);
	regions.insert_last (en_index);
	regions.insert_last (en_offset);
	appPtr app = clipboard::create_deep_copy (nodes, regions, 0);
	return app;
    }
}

static int RegionTextCmd(ClientData cd, Tcl_Interp *interp, int argc, char *argv[])
{
    static ocharstream os;

    Interpreter *i = (Interpreter *)cd;
    symbolArr res;
    if(i->ParseArguments(1, argc, argv, res) != 0)
	return TCL_ERROR;
    for(int ii = 0; ii < res.size(); ii++){
	symbolPtr sym     = res[ii];
	smtRegion *region = GetRegion(sym);
	if(region){
	    smtTree *smt_start = GetSmtFromMarker((smtMarker *)region);
	    smtTree *smt_end   = GetSmtFromMarker(region->end);
	    if(smt_start != NULL && smt_end != NULL){
		appPtr app = CreateRegApp(smt_start, smt_end);
		if(app){
		    os.reset();
		    os << '\0';
		    os.reset();
		    smtTree *smt = (smtTree *)app->get_root();
		    if(smt)
			smt->send_string(os);
		    os << '\0';
		    char *str = os.ptr();
		    Tcl_AppendResult(i->interp, str, NULL);
		    obj_delete(app);
		}
	    }
	}
    }
    return TCL_OK;
}

static int RegionNativeTextCmd(ClientData cd, Tcl_Interp *interp, int argc, char *argv[])
{
    Interpreter *i = (Interpreter *)cd;
    symbolArr res;
    if(i->ParseArguments(1, argc, argv, res) != 0)
	return TCL_ERROR;
    for(int ii = 0; ii < res.size(); ii++){
	symbolPtr sym     = res[ii];
	smtRegion *region = GetRegion(sym);
	if(region){
	    smtTree *smt_start = GetSmtFromMarker((smtMarker *)region);
	    smtTree *smt_end   = GetSmtFromMarker(region->end);
	    if(smt_start != NULL && smt_end != NULL){
		appPtr app = CreateRegApp(smt_start, smt_end);
		if(app){
		    smtTree *smt = (smtTree *)app->get_root();
		    if(smt){
			char *str = ste_smt_get_native_string(smt);
			Tcl_AppendResult(i->interp, str, NULL);
			delete [] str;
		    }
		    obj_delete(app);
		}
	    }
	}
    }
    return TCL_OK;
}

static int RegionCopyCmd(ClientData cd, Tcl_Interp *interp, int argc, char *argv[])
{
    Interpreter *i = (Interpreter *)cd;
    symbolArr res;
    if(i->ParseArguments(1, argc, argv, res) != 0)
	return TCL_ERROR;
    int ok = 0;
    if(res.size() > 0){
	symbolPtr sym     = res[0];
	smtRegion *region = GetRegion(sym);
	if(region){
	    smtTree *smt_start = GetSmtFromMarker(region);
	    smtTree *smt_end   = GetSmtFromMarker(region->end);
	    if(smt_start != NULL && smt_end != NULL){
		appPtr app = CreateRegApp(smt_start, smt_end);
		if(app){
		    if(clipboard_app)
			obj_delete(clipboard_app);
		    ((smtHeader *)app)->tidy_tree();
		    clipboard_app = app;
		    ok            = 1;
		}
	    }
	}
    }
    if(ok)
	Tcl_AppendResult(i->interp, "1", NULL);
    else
	Tcl_AppendResult(i->interp, "0", NULL);
    return TCL_OK;
}

static int RegionCutCmd(ClientData cd, Tcl_Interp *interp, int argc, char *argv[])
{
    Interpreter *i = (Interpreter *)cd;
    symbolArr res;
    if(i->ParseArguments(1, argc, argv, res) != 0)
	return TCL_ERROR;
    int ok = 0;
    if(res.size() > 0){
	symbolPtr sym     = res[0];
	smtRegion *region = GetRegion(sym);
	if(region){
	    smtTree *smt_start = GetSmtFromMarker(region);
	    smtTree *smt_end   = GetSmtFromMarker(region->end);
	    if(smt_start != NULL && smt_end != NULL){
		objArr nodes;
		appPtr app = CreateRegApp(smt_start, smt_end);
		if(app){
		    if(clipboard_app)
			obj_delete(clipboard_app);
		    ((smtHeader *)app)->tidy_tree();
		    clipboard_app = app;
		    ok            = 1;
		    ste_get_subtrees (smt_start, smt_end, nodes, 0);
		    if (nodes.size())
			scrapbook_instance->cut (nodes, 0);
		}
	    }
	}
    }
    if(ok)
	Tcl_AppendResult(i->interp, "1", NULL);
    else
	Tcl_AppendResult(i->interp, "0", NULL);
    return TCL_OK;
}

int RegionPasteAfter(ClientData cd, Tcl_Interp *interp, int argc, char *argv[])
{
    Initialize(RegionPasteAfter);

    Interpreter *i = (Interpreter *)cd;
    symbolArr res;
    if(i->ParseArguments(1, argc, argv, res) != 0)
	return TCL_ERROR;
    int ok = 0;
    if(res.size() > 0){
	symbolPtr sym = res[0];
	smtTree *smt;
	if((smt = GetSmtFromMarker(sym)) != NULL && clipboard_app != NULL){
	    smtTree *smt_to_insert = (smtTree *)clipboard_app->get_root();
	    if(smt_to_insert){
		deepCopier cp;
		cp.put_starter_obj(smt_to_insert);
		smtTree* ret_n   = checked_cast(smtTree,obj_copy_deep (*smt_to_insert, &cp, 0));
		smtHeader* ret_h = checked_cast(smtHeader, ret_n->get_header());
		ret_h->tidy_tree();

		start_smt_editing();
		smtHeader *trg_h = (smtHeader *)smt->get_header();
		trg_h->obj_insert_light(AFTER, ret_n, smt, (void *)0, (void *)1);
		trg_h->tidy_tree();
		trg_h->vrn = 1;  /* view update ! */
		commit_smt_editing();
		ok            = 1;
	       
		obj_delete(ret_h);
	    }
	}
    }
    if(ok)
	Tcl_AppendResult(i->interp, "1", NULL);
    else
	Tcl_AppendResult(i->interp, "0", NULL);
    return TCL_OK;
}

int RegionPasteBefore(ClientData cd, Tcl_Interp *interp, int argc, char *argv[])
{
    Initialize(RegionPasteAfter);

    Interpreter *i = (Interpreter *)cd;
    symbolArr res;
    if(i->ParseArguments(1, argc, argv, res) != 0)
	return TCL_ERROR;
    int ok = 0;
    if(res.size() > 0){
	symbolPtr sym = res[0];
	smtTree *smt;
	if((smt = GetSmtFromMarker(sym)) != NULL && clipboard_app != NULL){
	    smtTree *smt_to_insert = (smtTree *)clipboard_app->get_root();
	    if(smt_to_insert){
		deepCopier cp;
		cp.put_starter_obj(smt_to_insert);
		smtTree* ret_n   = checked_cast(smtTree,obj_copy_deep (*smt_to_insert, &cp, 0));
		smtHeader* ret_h = checked_cast(smtHeader, ret_n->get_header());
		ret_h->tidy_tree();

		start_smt_editing();
		smtHeader *trg_h = (smtHeader *)smt->get_header();
		trg_h->obj_insert_light(BEFORE, ret_n, smt, (void *)0, (void *)1);
		trg_h->tidy_tree();
		trg_h->vrn = 1;  /* view update ! */
		commit_smt_editing();
		ok            = 1;
		obj_delete(ret_h);
	    }
	}
    }
    if(ok)
	Tcl_AppendResult(i->interp, "1", NULL);
    else
	Tcl_AppendResult(i->interp, "0", NULL);
    return TCL_OK;
}

static int add_commands()
{
    new cliCommandInfo("smt_replace_text", SmtReplaceTextCmd);
    new cliCommandInfo("smt_insert_text_after", SmtInsertTextAfterCmd);
    new cliCommandInfo("smt_insert_text_before", SmtInsertTextBeforeCmd);
    new cliCommandInfo("region_text", RegionTextCmd);
    new cliCommandInfo("region_native_text", RegionNativeTextCmd);
    new cliCommandInfo("region_copy", RegionCopyCmd);
    new cliCommandInfo("region_cut", RegionCutCmd);
    new cliCommandInfo("region_paste_after", RegionPasteAfter);
    new cliCommandInfo("region_paste_before", RegionPasteBefore);

    return 0;
}

static int add_commands_dummy = add_commands();

