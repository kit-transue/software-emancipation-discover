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
#include <genError.h>
#include "ldrTypes.h"
#include <ddict.h>
#include <xref.h>
#include <ldrNode.h>
#include <symbolLdrHeader.h>
#include <symbolLdrTree.h>
#include <lde_symbol.h>

static int depth;

//  Note:  following is local prototype for get_filtered_link() in dd !!!
void get_filtered_link (ddKind, symbolPtr , linkType, symbolArr&);

static void build_one_level (objArr& curr_arr, symbolArr& build, symbolArr & c,
			     linkType lt, ddKind kind, int level, int up)
{
    Initialize (build_one_level); 

    objArr level_arr;

    Obj* el;
    ForEach (el, curr_arr) {
	symbolLdrTree* root = checked_cast (symbolLdrTree, el);
	fsymbolPtr xref     = root->get_xref ();
	symbolArr arr;
	get_filtered_link (kind, xref, lt, arr);
	if (!build.includes (xref)) {
	    build.insert_last (xref);
	    ldrTree* prev = NULL;
	    symbolPtr el1;
	    ForEachS (el1, arr) {
		fsymbolPtr  xr = el1;
		if (xr->xrisnotnull() &&
		    (kind == DD_UNKNOWN || xr.get_kind () == kind)) {
		    symbolLdrTree* ch = db_new (symbolLdrTree, (xr, lt));
		    if (!c.includes (xr))
			c.insert_last (xr);
		    if (level != depth - 1) {
                        if ( !level_arr.includes(ch) )
                              level_arr.insert(ch);
		    } else {
			symbolArr ch_arr;
			get_filtered_link (kind, xr, lt, ch_arr);
			symbolPtr el2;
			ForEachS (el2, ch_arr) {
			    fsymbolPtr  ch_xr = el2;
			    if (ch_xr->xrisnotnull() &&
				(kind == DD_UNKNOWN || ch_xr.get_kind () == kind)) {
				if (up)
				    ch->top_end = 1;
				else
				    ch->bottom_end = 1;
				break;
			    }
			}
		    }			    
		    if(prev)
			prev->put_after (ch);
		    else
			root->put_first (ch);
		    prev = ch;
		}
	    }
	} else {
	    symbolPtr el1;
	    ForEachS (el1, arr) {
		fsymbolPtr  xr = el1.get_xrefSymbol();
		if (xr->xrisnotnull() && 
			(kind == DD_UNKNOWN || xr.get_kind () == kind)) {
		    if (up)
			root->top_end = 1;
		    else
			root->bottom_end = 1;
		    break;
		}
	    }
	}   
    }
    curr_arr = level_arr;
}

static ldrFuncCallNode* build_first (symbolLdrTree* root, int level, int up)
{
   Initialize (build_first);
   
   root->set_graphlevel(level);
   root->set_role(ldrFDECL);
   char* name = root->get_xref ().get_name ();

   ldrFuncCallNode* f_symbol = db_new(ldrFuncCallNode,
	(level ? ldrFUNC_CALL : ldrFUNC_CALL_ROOT,name));

   symbolPtr sym = root->get_xref ();
   if(!sym.get_has_def_file() && sym->get_def_file().isnull())
       f_symbol->add_call_type(FUNCCALL_SYSTEM);

   if(root->recursive)
       f_symbol->add_call_type(FUNCCALL_RECURSIVE);
   
// check if lde_hdr has no display arg flag set ( = 0)

   symbolLdrHeader* hd = checked_cast (symbolLdrHeader, root->get_header ());
   if ( hd->get_disp_arg() == 0 ) 

//      strip args - default display args (flag = 1)
        f_symbol->modif_arg(name, 0);
        
   f_symbol->set_graphlevel (level);
   f_symbol->set_role(ldrFDECL);
   f_symbol->upper_tree = up;
   f_symbol->bottom_end = root->bottom_end;
   f_symbol->top_end = root->top_end;
   root->set_exit_symbol (f_symbol);
   root->set_entry_symbol (f_symbol);
   root->put_first (f_symbol);
   return f_symbol;
}

static void internal_setup_recursion_attributes(symbolLdrTree *root, symbolArr& set, objArr& set1)
{           
    Initialize(internal_setup_recursion_attributes);

    int recursive;

    fsymbolPtr xref = root->get_xref();
    if(set.includes(xref)){
	recursive = 1;
	// Also setup attribute for the original function
        for(int i = set.size() - 1; i >= 0; i --)
	    if(set[i] == xref){
		symbolLdrTree *tree = (symbolLdrTree *)set1[i];
		objTree*       first = tree->get_first();
		if(is_ldrFuncCallNode(first)){
		    ldrFuncCallNode *fc = checked_cast(ldrFuncCallNode, first);
		    fc->add_call_type(FUNCCALL_RECURSIVE);
		}
		break;
	    }
    }
    else {
	set.insert_last(xref);
	set1.insert_last(root);
	recursive = 0;
    }

    objTree* first = root->get_first();
    if(is_ldrFuncCallNode(first)){
	ldrFuncCallNode *fc = checked_cast(ldrFuncCallNode, first);
	if(recursive)
	    fc->add_call_type(FUNCCALL_RECURSIVE);
	else
	    fc->clear_call_type(FUNCCALL_RECURSIVE);

	objTree*       next = fc->get_next();
	symbolLdrTree* ch;
	for (; next; next = next->get_next ()) {
	    if(is_symbolLdrTree(next) == 1){
		ch = checked_cast (symbolLdrTree, next);
	        internal_setup_recursion_attributes(ch, set, set1);
	    }
	}
    }
}

void lde_decorate_symbol (symbolLdrTree* root, int level, int up)
{
    Initialize (lde_decorate_symbol);

    ldrFuncCallNode* first = build_first (root, level, up);
    objTree* next = first->get_next ();
    if (!next)
	return;

    ldrConnectorType conn_type = up ? ldrcUP_TREE : ldrcTREE;

    ldrConnectionNode* connect = db_new (ldrConnectionNode, (conn_type));
    connect->set_graphlevel (level);
    connect->add_input(root->get_exit_symbol ());

    symbolLdrTree* ch = NULL;
    
    for (; next; next = next->get_next ()) {
	ch = checked_cast (symbolLdrTree, next);
	lde_decorate_symbol (ch, level + 1, up);
	connect->add_output (ch->get_entry_symbol ());
    }
    ch->put_after (connect);
}

void lde_setup_recursion_attributes(symbolLdrTree *root, symbolArr& set)
{
    Initialize(lde_setup_recursion_attributes);

    objArr set1;
    internal_setup_recursion_attributes(root, set, set1);
}
  
void lde_extract_symbol (symbolLdrTree* root, linkType lt, ddKind kind, 
			   int level, int up)
{
    Initialize (lde_extract_symbol);

    fsymbolPtr  xref = root->get_xref ();
    
    if (! (kind == DD_UNKNOWN || xref.get_kind () == kind))
	return;

    symbolLdrHeader* hd = checked_cast (symbolLdrHeader, root->get_header ());
    depth = hd->get_default_no_levels () + level;
    linkType lt_par = hd->get_parent_lt ();
    symbolArr arr;
    get_filtered_link (kind, root->get_xref (), lt_par, arr);
    if (arr.size ())
	root->top_end = 1;

    symbolArr build;
    objArr curr_arr;
    curr_arr.insert (root);

    // get RTL
    RTL* rtl = checked_cast (RTL, ldr_get_app (hd));
    RTLNode* rtl_node = checked_cast (RTLNode, rtl->get_root ());
    symbolArr& cont = rtl_node->rtl_contents();

    for (int curr_level = level + 1;
	 curr_level < depth && ! curr_arr.empty ();
	 curr_level++) 
	build_one_level (curr_arr, build, cont, lt, kind, curr_level, up);

    lde_decorate_symbol (root, level, up);
}

/*
   START-LOG-------------------------------------------

   $Log: lde_symbol.cxx  $
   Revision 1.6 1999/03/29 12:10:43EST Sudha Kallem (sudha) 
   changes to support new pmod format
 * Revision 1.10  1994/08/02  17:07:35  kws
 * Bug track: 7665
 * Make root of call tree graphically identifiable
 *
 * Revision 1.9  1993/12/23  14:10:25  azaparov
 * Bug track: 5760
 * Fixed bug 5760
 *
 * Revision 1.8  1993/12/16  13:47:18  azaparov
 * Bug track: 4418
 * Fixed bug 4418
 *
 * Revision 1.7  1993/08/10  17:29:31  sergey
 * Switched from sets to array. Part of bug #4187.
 *
 * Revision 1.6  1993/04/14  18:41:55  sergey
 * Added suport for show/hide args in C++ call tree.
 *
 * Revision 1.5  1993/03/15  21:46:49  davea
 * change xrefSymbol* to fsymbolPtr
 *
 * Revision 1.4  1993/03/08  00:08:43  davea
 * replace xrefSymbol* with symbolPtr
 *
 * Revision 1.3  1993/03/04  21:03:02  aharlap
 * used get_filtered_link instead get_link()
 *
 * Revision 1.2  1992/12/30  15:57:59  aharlap
 * *** empty log message ***
 *
 * Revision 1.1  1992/12/21  23:01:01  aharlap
 * Initial revision
 *

   END-LOG---------------------------------------------

*/






