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
//
// disUIHandler.h
// --------------
//

#include <msg.h>
#include <parray.h>
#include <linkTypes.h>
#include <ddSelector.h>
#include <symbolPtr.h>
#include <symbolArr.h>

#include <RTL.h>
#include <appDGraph.h>
#include <view_creation.h>

#ifndef NEW_UI
#include <gtRTL.h>
#include <gtTogB.h>
#include <gtStringEd.h>
#include <gtPushButton.h>
#include <gtDlgTemplate.h>
#endif

#include <systemMessages.h>
#include <messages.h>

#include <DAGraph.h>
#include <DGNodeData.h>
#include <DGNodeDataXREF.h>
#include <DAGHandler.h>
#include <DAGHandlerXREF.h>

#include <disUIHandler.h>

//
// EXTERNAL DECLARATIONS
//

extern "C" {
    void push_busy_cursor();
    void pop_cursor();
};

extern
    int  get_all_browser_selected_symbols(symbolArr &);

//
// INTERFACE ROUTINES
//

//
// returns: -1: error, 0: the dialog is poped up
//
extern "C" int
generate_call_graph(symbolArr & source, symbolArr & target, disUIHandler **rhand/*=NULL*/)
{
    int ret_val = -1;

    disUIHandler * handler = disUIHandler::ui_handler((linkTypes *)0, (ddSelector *)0);
    if (handler) {
	symbolArr src_funcs;
	symbolArr trg_funcs;

	if (handler->filter_symbols(source, src_funcs)>=0 &&
	    handler->filter_symbols(target, trg_funcs)>=0){
	    ret_val = handler->restart(src_funcs, trg_funcs);
#ifdef NEW_UI
	    if(ret_val == 0){
		source = src_funcs;
		target = trg_funcs;
	    }
#endif
	} else {
	    delete handler;
	    handler = (disUIHandler *)0;
	}
    }
    if( rhand )
      *rhand = handler;
    return ret_val;
}

//
// PUBLIC-STATIC:
//

//
// it should control the usage of "graph preferences"
//
disUIHandler *
disUIHandler::ui_handler(linkTypes * sym_flt, ddSelector * knd_flt)
{
    Initialize(disUIHandler::ui_handler);

    disUIHandler * ret_val = (disUIHandler *)0;

    linkTypes default_sym_flt;
    if (!sym_flt) {
	sym_flt = &default_sym_flt;
	default_sym_flt.add(is_using);
    }
    ddSelector default_knd_flt;
    if (!knd_flt) {
	knd_flt = &default_knd_flt;
	default_knd_flt.add(DD_FUNC_DECL);
    }
    ret_val = new disUIHandler(*sym_flt, *knd_flt);

    return ret_val;
}

//
// returns: -1: error, 0: the rtl will be updated
//
#ifndef NEW_UI
int
disUIHandler::update(gtRTL * gt_rtl)
{
    int ret_val = -1;

    if (gt_rtl) {
	RTL * rtl = gt_rtl->RTL();
	if (rtl) {
	    gt_rtl->regenerate_rtl(rtl);
	    ret_val = 0;
	}
    }

    return ret_val;
}

//
// returns: -1: error, 0: the array is extracted
//
int
disUIHandler::extract(gtRTL * gt_rtl, symbolArr * & contents)
{
    Initialize(disUIHandler::extract);

    int ret_val = -1;

    if (gt_rtl) {
	RTLNode * rnode = (RTLNode *)0;
	if (!(ret_val=disUIHandler::extract(gt_rtl, rnode)))
	    contents = &(rnode->rtl_contents());
    }

    return ret_val;
}

//
// returns: -1: error, 0: the node is extracted
//
int
disUIHandler::extract(gtRTL * gt_rtl, RTLNode * & node)
{
    Initialize(disUIHandler::extract);

    int ret_val = -1;

    if (gt_rtl) {
	RTL * rtl = gt_rtl->RTL();
	if (rtl) {
	    node = checked_cast(RTLNode,rtl->get_root());
	    if (node != 0)
		ret_val = 0;
	}
    }

    return ret_val;
}
#endif // NEW_UI

//
// PUBLIC:
//

disUIHandler::~disUIHandler()
{
    Initialize(disUIHandler::~disUIHandler);

#ifndef NEW_UI
    if (shell) {
	shell->popdown();
	delete shell;
    }
#else
    if (src_arr)
	delete src_arr;
    if (trg_arr)
	delete trg_arr;
#endif
    if (xref_graph)
	delete xref_graph;
}

#ifndef NEW_UI
//
// returns: !=0: the graph is already built, 0: the graph is not built yet
//
int
disUIHandler::gexists(int val)
{
    int ret_val = 0;

    if (graph()!=0 && val>=0) {
	symbolArr * src_arr = (symbolArr *)0;
	symbolArr * trg_arr = (symbolArr *)0;
	if (!disUIHandler::extract(src_syms, src_arr) &&
	    !disUIHandler::extract(trg_syms, trg_arr))
	    ret_val = graph()->xsym_exists(*src_arr, *trg_arr) >= val;
    }

    return ret_val;
}

//
// returns: -1: error, 0: the dialogbox is poped up
//
int
disUIHandler::restart(symbolArr & source, symbolArr & target)
{
    Initialize(disUIHandler::restart);

    int ret_val = -1;

    if (src_syms!=0 && trg_syms!=0 && shell!=0) {
	RTLNode * src_node = (RTLNode *)0;
	RTLNode * trg_node = (RTLNode *)0;
	if (!disUIHandler::extract(src_syms, src_node) &&
	    !disUIHandler::extract(trg_syms, trg_node)) {
	    src_syms->delete_all_items();
	    trg_syms->delete_all_items();
	    if (source.size() > 0) {
		symbolPtr x_sym;
		ForEachS(x_sym,source) {
		    if (!(src_node->rtl_includes(x_sym)))
			src_node->rtl_insert(x_sym);
		}
	    }
	    if (target.size() > 0) {
		symbolPtr x_sym;
		ForEachS(x_sym,target) {
		    if (!(trg_node->rtl_includes(x_sym)))
			trg_node->rtl_insert(x_sym);
		}
	    }
	    shell->popup(0);
	    disUIHandler::update(src_syms);
	    disUIHandler::update(trg_syms);
	    ret_val = 0;
	}
    }

    return ret_val;
}

//
// returns: -1: error, 0: the symbols are moved/transferred
//
int
disUIHandler::move_symbols(symbolArr & values, RTLNode & source, RTLNode & target)
{
    Initialize(disUIHandler::move_symbols);

    int ret_val = -1;

    if (values.size() > 0) {
	symbolPtr x_sym;
	ForEachS(x_sym,values) {
	    if (source.rtl_includes(x_sym))
		source.rtl_contents().remove(x_sym);
	    if (!target.rtl_includes(x_sym))
		target.rtl_insert(x_sym);
	}
	ret_val = 0;
    }

    return ret_val;
}

//
// returns: -1: error, >=0: the number of fetched symbols
//
int
disUIHandler::fetch_external(symbolArr & selection)
{
    Initialize(disUIHandler::fetch_external);

    int ret_val = -1;

    if (graph()) {
	symbolArr ext_sel;

	if ((ret_val=get_all_browser_selected_symbols(ext_sel)) > 0)
	    ret_val = filter_symbols(ext_sel, selection);
    }

    return ret_val;
}

//
// returns: -1: error, 0: the symbols are removed
//
int
disUIHandler::remove_symbols(symbolArr & values)
{
    Initialize(disUIHandler::remove_symbols);

    int ret_val = -1;

    if (values.size() > 0) {
	RTLNode * source = (RTLNode *)0;
	RTLNode * target = (RTLNode *)0;
	if (!disUIHandler::extract(src_syms, source) &&
	    !disUIHandler::extract(trg_syms, target)) {
	    symbolPtr x_sym;
	    ForEachS(x_sym,values) {
		if (source->rtl_includes(x_sym))
		    source->rtl_contents().remove(x_sym);
		if (target->rtl_includes(x_sym))
		    target->rtl_contents().remove(x_sym);
	    }
	    ret_val = 0;
	}
    }

    return ret_val;
}

#endif // NEW_UI

//
// returns: -1: error, >=0: the number of copied symbols
//
int
disUIHandler::filter_symbols(symbolArr & source, symbolArr & accepted)
{
    Initialize(disUIHandler::filter_symbols);

    int ret_val = -1;

    if (graph()) {
	ret_val = 0;

	parray symbols(0);
	if (graph()->builddata(source, symbols) > 0) {
	    for (int i=symbols.size()-1; i>=0; i--) {
		accepted.insert_last(((DGNodeDataXREF*)(symbols[i]))->symbol());
		ret_val++;
	    }
	    (void) DAGHandler::rmdata(&symbols);
	}
    }

    return ret_val;
}

//
// returns: -1: error, >=0: number of symbols that reduced input
//
int
disUIHandler::reduce_input(symbolArr & i_sym, symbolArr & o_sym)
{
    int ret_val = -1;

    if (graph()) {
	ret_val = 0;

	symbolArr rm_arr;
	parray    inp_data(0);
	int       inp_size = 0;
	// -- process roots
	if (graph()->builddata(i_sym, inp_data) > 0) {
	    inp_size = inp_data.size();
	    for (int x_iter=0; x_iter<inp_size; x_iter++) {
		DGNodeDataXREF * x_node = (DGNodeDataXREF *)inp_data[x_iter];
		if (x_node) {
		    if (!(graph()->graph()->lookup(x_node, 1)))
			rm_arr.insert_last(x_node->symbol());
		}
	    }
	}
	// -- process terminals
	if (graph()->builddata(o_sym, inp_data) > 0) {
	    for (int x_iter=inp_data.size()-1; x_iter>=inp_size; x_iter--) {
		DGNodeDataXREF * x_node = (DGNodeDataXREF *)inp_data[x_iter];
		if (x_node) {
		    if (!(graph()->graph()->lookup(x_node, -1)))
			rm_arr.insert_last(x_node->symbol());
		}
	    }
	}
	if ((ret_val=rm_arr.size()) > 0) {
	    (void) remove_symbols(rm_arr);
#ifndef NEW_UI
	    (void) disUIHandler::update(src_syms);
	    (void) disUIHandler::update(trg_syms);
#endif
	}
	(void) DAGHandler::rmdata(&inp_data);
    }

    return ret_val;
}

#ifndef NEW_UI
//
// returns: -1: error, >=0: ok
//
int
disUIHandler::generate(int)
{
    Initialize(disUIHandler::generate);

    int ret_val = -1;

    if (graph()) {
	symbolArr * src_arr = (symbolArr *)0;
	symbolArr * trg_arr = (symbolArr *)0;
	if (!disUIHandler::extract(src_syms, src_arr) &&
	    !disUIHandler::extract(trg_syms, trg_arr)) {
	    ret_val = graph()->xsym_build(*src_arr, *trg_arr);
	    if (ret_val > 0)
		(void) reduce_input(*src_arr, *trg_arr);
	}
    }

    return ret_val;
}


//
// returns: -1: error, 0: the view is built and displayed
//
int
disUIHandler::preview(char *)
{
    Initialize(disUIHandler::preview);

    int ret_val = -1;

    if (gexists(2)) {
	appDGraph * g_app = new appDGraph(graph());
	if (g_app) {
	    symbolPtr ghead(g_app);
	    if (view_create(ghead, Rep_DGraph)) {
		view_create_flush();
		ret_val = 0;
	    }
	    (void) g_app->graph(); // -- destroy the reference
	}
    }

    return ret_val;
}

#endif // NEW_UI

//
// returns: -1: error, >=0: ok
//
int
disUIHandler::output(char * dev_name)
{
    Initialize(disUIHandler::output);

    int ret_val = -1;

    if (gexists(2))
	ret_val = graph()->xsym_print(dev_name);

    return ret_val;
}

//
// PROTECTED:
//
#ifndef NEW_UI
void
disUIHandler::create_CB(gtPushButton * btn, gtEventPtr evnt, void * cd, gtReason rsn)
{
    Initialize(disUIHandler::create_CB);

    if (cd) {
	disUIHandler::apply_CB(btn, evnt, cd, rsn);
	disUIHandler::cancel_CB(btn, evnt, cd, rsn);
    }
}

void
disUIHandler::apply_CB(gtPushButton *, gtEventPtr, void * cd, gtReason)
{
    Initialize(disUIHandler::apply_CB);

    if (cd) {
	push_busy_cursor();

	disUIHandler * handler = (disUIHandler *)cd;
	if (!(handler->gexists(2))) {
	    if (handler->generate(0) < 1) {
		msg("No graph was built for the specified data.", error_sev) << eom;
	    }
	}
	if (handler->out_destination > 0) {
	    if (handler->out_device) {
		char * dev_name = handler->out_device->text();
		if (dev_name) {
		    (void) handler->output(dev_name);
		    gtFree(dev_name);
		}
	    }
	} else
	    (void) handler->preview(0);

	pop_cursor();
    }
}

void
disUIHandler::cancel_CB(gtPushButton *, gtEventPtr, void * cd, gtReason)
{
    Initialize(disUIHandler::cancel_CB);

    if (cd)
	delete ((disUIHandler *)cd);
}

void
disUIHandler::src2trg_CB(gtPushButton *, gtEventPtr, void * cd, gtReason)
{
    Initialize(disUIHandler::src2trg_CB);

    if (cd) {
	disUIHandler * handler = (disUIHandler *)cd;

	symbolArr selected;
	handler->src_syms->app_nodes(&selected);
	if (selected.size() > 0) {
	    RTLNode * src_node = (RTLNode *)0;
	    RTLNode * trg_node = (RTLNode *)0;
	    if (!disUIHandler::extract(handler->src_syms, src_node) &&
		!disUIHandler::extract(handler->trg_syms, trg_node)) {
		handler->move_symbols(selected, *src_node, *trg_node);
		disUIHandler::update(handler->src_syms);
	  	disUIHandler::update(handler->trg_syms);
	    }
	}
    }
}

void
disUIHandler::trg2src_CB(gtPushButton *, gtEventPtr, void * cd, gtReason)
{
    Initialize(disUIHandler::trg2src_CB);

    if (cd) {
	disUIHandler * handler = (disUIHandler *)cd;

	symbolArr selected;
	handler->trg_syms->app_nodes(&selected);
	if (selected.size() > 0) {
	    RTLNode * src_node = (RTLNode *)0;
	    RTLNode * trg_node = (RTLNode *)0;
	    if (!disUIHandler::extract(handler->src_syms, src_node) &&
		!disUIHandler::extract(handler->trg_syms, trg_node)) {
		(void) handler->move_symbols(selected, *trg_node, *src_node);
		disUIHandler::update(handler->src_syms);
	  	disUIHandler::update(handler->trg_syms);
	    }
	}
    }
}

void
disUIHandler::insert_CB(gtPushButton *, gtEventPtr, void * cd, gtReason)
{
    Initialize(disUIHandler::insert_CB);

    if (cd) {
	disUIHandler * handler = (disUIHandler *)cd;

	symbolArr selected;
	if (handler->fetch_external(selected) > 0) {
	    RTLNode * src_node = (RTLNode *)0;
	    if (!disUIHandler::extract(handler->src_syms, src_node)) {
		(void) handler->move_symbols(selected, *src_node, *src_node);
		disUIHandler::update(handler->src_syms);
	    }
	}
    }
}

void
disUIHandler::remove_CB(gtPushButton *, gtEventPtr, void * cd, gtReason)
{
    Initialize(disUIHandler::remove_CB);

    if (cd) {
	disUIHandler * handler = (disUIHandler *)cd;

        symbolArr selected;
	handler->src_syms->app_nodes(&selected);
	handler->trg_syms->app_nodes(&selected);
	if (selected.size() > 0) {
	    handler->remove_symbols(selected);
	    disUIHandler::update(handler->src_syms);
	    disUIHandler::update(handler->trg_syms);
	}
    }
}

void
disUIHandler::out_device_CB(gtToggleButton * tb, gtEventPtr, void * cd, gtReason)
{
    Initialize(disUIHandler::out_device_CB);

    if (tb!=0 && cd!=0) {
	disUIHandler * handler = (disUIHandler *)cd;
	if (tb->set()) {
	    handler->out_destination = 1;
	    handler->out_device->set_sensitive(1);
	} else {
	    handler->out_destination = 0;
	    handler->out_device->set_sensitive(0);
	}
    }
}

#endif // NEW_UI

disUIHandler::disUIHandler(linkTypes & sym_flt, ddSelector & knd_flt)
{
    Initialize(disUIHandler::disUIHandler);

#ifndef NEW_UI
    shell = gtDialogTemplate::create(NULL, "dis_ui_graph", TXT("Graph"));

    shell->add_button("create_cb", TXT("Create"), disUIHandler::create_CB, this);
    shell->add_button("apply_cb", TXT("Apply"), disUIHandler::apply_CB, this);
    shell->add_button("cancel_cb", TXT("Done"), disUIHandler::cancel_CB, this);
    shell->add_help_button();
    shell->help_context_name("Pset.Help.Graph.Dialog");

    gtBase * container = shell->container();

    gtPushButton * ins_2_src = gtPushButton::create(container, "ins_2_src",
	  TXT("Add Source"), disUIHandler::insert_CB, this);
    ins_2_src->attach_pos(gtTop, 15);
    ins_2_src->attach_pos(gtLeft, 45);

    gtPushButton * rm_src_trg = gtPushButton::create(container, "rm_src_trg",
	  TXT("Remove"), disUIHandler::remove_CB, this);
    rm_src_trg->attach(gtTop, ins_2_src);
    rm_src_trg->attach_opp_left(ins_2_src);
    rm_src_trg->attach_opp_right(ins_2_src);

    gtPushButton * src_2_trg = gtPushButton::create(container, "src_2_trg",
          TXT(" ==> "), disUIHandler::src2trg_CB, this);
    src_2_trg->attach(gtTop, rm_src_trg);
    src_2_trg->attach_opp_left(ins_2_src);
    src_2_trg->attach_opp_right(ins_2_src);

    gtPushButton * trg_2_src = gtPushButton::create(container, "trg_2_src",
	  TXT(" <== "), disUIHandler::trg2src_CB, this);
    trg_2_src->attach(gtTop, src_2_trg);
    trg_2_src->attach_opp_left(ins_2_src);
    trg_2_src->attach_opp_right(ins_2_src);

    src_syms = gtRTL::create(container, "src_syms", TXT("Source"), gtExtended, NULL, 0);
    src_syms->set_rtl(db_new(RTL,((char *)0)), 0);
    src_syms->num_rows(7);
    if (src_syms->gt_list())
    	src_syms->gt_list()->width(350);
    src_syms->attach(gtTop);
    src_syms->attach(gtLeft);
    src_syms->attach(gtRight, ins_2_src);

    trg_syms = gtRTL::create(container, "trg_syms", TXT("Target"), gtExtended, NULL, 0);
    trg_syms->set_rtl(db_new(RTL,((char *)0)), 0);
    trg_syms->num_rows(7);
    if (trg_syms->gt_list())
    	trg_syms->gt_list()->width(350);
    trg_syms->attach(gtTop);
    trg_syms->attach(gtLeft, ins_2_src);
    trg_syms->attach(gtRight);

    gtToggleButton * out_toggle = gtToggleButton::create(container, "out_toggle",
	  TXT("Save to File"), disUIHandler::out_device_CB, this);
    out_toggle->set(0, 0);
    out_toggle->attach(gtLeft);
    out_toggle->attach(gtBottom);

    out_device = gtStringEditor::create(container, "out_device", NULL);
    out_device->set_sensitive(0);
     out_device->attach(gtLeft, out_toggle);
    out_device->attach(gtRight);
    out_device->attach(gtBottom);

    trg_syms->attach(gtBottom, out_device);
    src_syms->attach(gtBottom, out_device);


    // -- filename

    src_syms->manage();
    trg_syms->manage();
    ins_2_src->manage();
    rm_src_trg->manage();
    src_2_trg->manage();
    trg_2_src->manage();
    out_toggle->manage();
    out_device->manage();
    container->manage();
#else
    src_arr	    = new symbolArr();
    trg_arr	    = new symbolArr();
#endif
    out_destination = 0;
    xref_graph      = new DAGHandlerXREF(sym_flt, knd_flt);
}

#ifdef NEW_UI

//
// returns: -1: error, 0: the symbols are removed
//
int
disUIHandler::remove_symbols(symbolArr & values)
{
    Initialize(disUIHandler::remove_symbols);

    int ret_val = -1;

    if (values.size() > 0) {
	symbolPtr x_sym;
	ForEachS(x_sym,values) {
	    if (src_arr->includes(x_sym))
		src_arr->remove(x_sym);
	    if (trg_arr->includes(x_sym))
		trg_arr->remove(x_sym);
	}
	ret_val = 0;
    }
    return ret_val;
}

//
// returns: !=0: the graph is already built, 0: the graph is not built yet
//
int
disUIHandler::gexists(int val)
{
    int ret_val = 0;

    if (graph()!=0 && val>=0) {
	ret_val = graph()->xsym_exists(*src_arr, *trg_arr) >= val;
    }

    return ret_val;
}

//
// returns: -1: error, >=0: ok
//
int
disUIHandler::generate(int)
{
    Initialize(disUIHandler::generate);

    int ret_val = -1;

    if (graph()) {
	ret_val = graph()->xsym_build(*src_arr, *trg_arr);
	if (ret_val > 0)
	    (void) reduce_input(*src_arr, *trg_arr);
    }

    return ret_val;
}

view *gra_create_view(symbolPtr sym, repType rep_type);
void open_view (view *vp);
//
// returns: -1: error, 0: the view is built and displayed
//
int
disUIHandler::preview(char *)
{
    Initialize(disUIHandler::preview);

    int ret_val = -1;

    if (gexists(2)) {
	appDGraph * g_app = new appDGraph(graph());
	if (g_app) {
	    symbolPtr ghead(g_app);
	    view *vp = gra_create_view(ghead, Rep_DGraph);
	    if (vp) {
		open_view (vp);
		ret_val = 0;
	    }
	    (void) g_app->graph(); // -- destroy the reference
	}
    }

    return ret_val;
}

int disUIHandler::restart(symbolArr & source, symbolArr & target)
{
    int ret_val = -1;

    // -- replace contents of src_arr, and trg_arr (operator =)
    *src_arr = source;
    *trg_arr = target;
    if (!gexists(2)) {
	if (generate(0) < 1) {
	    dis_message(NULL ,MSG_ERROR, "M_GRAPH_NOTBUILT");
	} else
	    ret_val = 0;
    }
    if (ret_val >= 0)
	preview(0);

    return ret_val;
}

#endif
