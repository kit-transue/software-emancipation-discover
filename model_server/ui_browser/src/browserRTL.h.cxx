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
// browserRTL.h.C
//------------------------------------------
// synopsis:
// 
// Specialized RTL that updates lists in projectQuery.
//------------------------------------------

// INCLUDE FILES

#include <symbolPtr.h>
#include <browserRTL.h>
#include <gtRTL.h>

init_relational(browserRTL, RTL);

implement_copy_member(browserRTL);

// FUNCTION DEFINITIONS

browserRTL::browserRTL()
: RTL(NULL), scope(NULL)
{
    Initialize(browserRTL::browserRTL);
#ifndef NEW_UI 
    parent_rtl = NULL;
#endif
}

browserRTL::browserRTL(const symbolArr& arr)
: RTL(NULL), scope(NULL)
{
    Initialize(browserRTL::browserRTL);
#ifndef NEW_UI
    parent_rtl = NULL;
#endif
    append(arr);
}

browserRTL::browserRTL(const ddSelector& sel)
: RTL(NULL),
  dd_selector(sel)
{
    Initialize(browserRTL::browserRTL);
#ifndef NEW_UI 
    parent_rtl = NULL;
#endif
    symbolArr result;
    scope->query(result, dd_selector);

    RTLNodePtr rtl_node = RTLNodePtr(get_root());
	    
    symbolPtr sym;
    ForEachS(sym, result) {
        rtl_node->rtl_insert(sym);
    }
}

browserRTL::browserRTL(gtRTL *rtl)
: RTL(NULL), scope(NULL)
{
    Initialize(browserRTL::browserRTL);
#ifndef NEW_UI
    parent_rtl = rtl;
#endif
}

browserRTL::browserRTL(gtRTL *rtl, const symbolArr& arr)
: RTL(NULL), scope(NULL)
{
    Initialize(browserRTL::browserRTL);
#ifndef NEW_UI
    parent_rtl = rtl;
#endif
    append(arr);
}

browserRTL::browserRTL(gtRTL *rtl, const ddSelector& sel, symbolScope *sc)
: RTL(NULL),
  dd_selector(sel)
{
    Initialize(browserRTL::browserRTL);
#ifndef NEW_UI 
    parent_rtl = rtl;
#endif
    scope      = sc;
    symbolArr result;
    scope->query(result, dd_selector);

    RTLNodePtr rtl_node = RTLNodePtr(get_root());
	    
    symbolPtr sym;
    ForEachS(sym, result) {
        rtl_node->rtl_insert(sym);
    }
}

browserRTL::~browserRTL()
{
    Initialize(browserRTL::~browserRTL);
}

void browserRTL::append(const symbolPtr& sym)
{
    Initialize(browserRTL::append);

    RTLNodePtr rtl_root = RTLNodePtr(get_root());
    symbolArr& contents = rtl_root->rtl_contents();

    contents.insert_last(sym);
}

void browserRTL::append(const symbolArr& arr)
{
    Initialize(browserRTL::append);

    RTLNodePtr rtl_root = RTLNodePtr(get_root());
    symbolArr& contents = rtl_root->rtl_contents();
    contents.insert_last(arr);
}

static bool sp_lacks_xref_symbol(symbolPtr s) {
	return (s.is_instance() || s.is_ast() || s.is_dataCell());
}
  
/* DaveA comments about xref_notify - 1/13/94
  This function called for at least three rtl's:
      browser scan mode column 2,  column 4,
      and "Group-1"
  The "added" logic doesn't do any harm for column 4 and
     "Group", because those both seem to have a dd_selector of
     zero, inhibiting any adds.
  It would probably be more efficient to use symbolSet's for
     the 3 arguments:  modified, added, and deleted
*/

void browserRTL::xref_notify(const symbolArr& mod,
			     const symbolArr& added,
			     const symbolArr& deleted)
/* Review of changes made by Pero. 
 * A. Harlap - 12.13.93 
 * The idea to use insert_last instead of remove for symbolArr is very
 * good. Only comment, that same changes should be done for method 
 * xref_notify for other classes.
 */
 
{
    Initialize(browserRTL::xref_notify);

//
//  before doing anything check if the RTL is updatable at all;
//  projList::domain is supposed to  contain the path list of all selected projects for which 
//  different RTL's are displayed. If the current (writable) project is 
//  not on the list don't bother to continue
//     DAVE'S NOTE:  domain is no longer valid by the time this function is called, so Alex
//       had to comment it out.  Bug 5994 will probably fix that, and this logic could be retested
//       and activated again.

    projNode *tmp_proj;
    for (int i = 0; tmp_proj = projList::search_list->get_scan(i); i++){
        Xref *xref = tmp_proj->get_xref();
        if (xref->is_writable())
	      break;
    }
   if (tmp_proj == NULL)
             return;               // did not find in the path list

    int touched = 0;	// A flag indicating the need to call obj_insert later.

    RTLNodePtr rtl_root = RTLNodePtr(get_root());
    symbolArr& contents = rtl_root->rtl_contents();

    symbolArr my_added;     // Make a copy to filter input
    symbolArr new_contents; 

    int added_size = added.size();
    int modified_size = mod.size();
    int deleted_size = deleted.size();
    int contents_size = contents.size();

//  filter symbols removing those from Control and Doc project
//      this should instead include only the symbols in the domain's projects
//      see bug 5994.

    projNode *ctrl_proj = projNode::get_control_project();
    Xref * ctrl_xref    = ctrl_proj->get_xref();

//  Update modules first 
    if(scope /* && !dd_selector.empty() *//*sioffe -- change*/){
	int do_updating = 0;
	for(int a = 0; a < added_size; a++) {
	    symbolPtr sym = added[a];
	    if(sym.get_kind() == DD_MODULE)
		if(!scope->is_in_scope(sym))
		    do_updating = 1;
	}
	if(do_updating)
	    scope->update_modules();
    }
//  substitution
    for (int a=0; a < added_size; a++) {
	symbolPtr sym = added[a];
	Xref  *tmp = sym.get_xref();
	if (!(tmp == ctrl_xref ||
	      sym.is_def() == 0))
	    if(scope){
		if(scope->is_in_scope(sym))
		    my_added.insert_last(sym);
	    }else
		my_added.insert_last(sym);
    }
    //   note:     the get_has_def_file() test should probably be done first, as it is cheaper
    //      than checking for a list of projects.  No problem while only two are checked.

    int loop_deleted = deleted_size;
    char *deleted_ndx = new char[deleted_size];
    int index;
    for (index=0; index < deleted_size; index++)
	deleted_ndx[index] = 0;

    for (index=0; index < contents_size; index++) {
        symbolPtr xsym = contents[index];
        if (!xsym.is_xrefSymbol())
	  touched = 1;
	else
	  xsym = xsym.get_xrefSymbol();

//  substitution
	int m = 0;
	while (!touched && m < modified_size) {
	    symbolPtr msym = mod[m];
	    if (msym.is_xrefSymbol())
	      msym = msym.get_xrefSymbol();
	    touched = (xsym == msym);
	    m++;
	}

	int deleted_found = 0, d = 0;
	if (loop_deleted > 0) {
	    while (d < deleted_size && !deleted_found) {
	      if (!deleted_ndx[d]) {
		symbolPtr dsym = deleted[d];
		if (dsym.is_xrefSymbol())
		  dsym = dsym.get_xrefSymbol();
		deleted_found = (xsym == dsym);
	      }
	      d++;
	    }
	}

	if (deleted_found) {
	    deleted_ndx[d-1] = 1;
	    loop_deleted--;
	    touched = 1;
	} else
	    new_contents.insert_last(xsym);
    }

    if (deleted_ndx)
	delete deleted_ndx;

    for(int aa = 0; aa < my_added.size(); ++aa) {
	symbolPtr& added_sym = my_added[aa];
	ddKind skind = added_sym.get_kind();
	if(dd_selector.selects(skind)) {
	    if (!new_contents.includes(added_sym)) {
		touched = 1;
		new_contents.insert_last(added_sym);
	    }
	}
    }
    //  note:  the new_contents.includes() above really ought to be checking the
    //    name, kind, and def_file, rather than just the raw symbolPtrs.  If we
    //    were to use a symbolSet, type 0, this would be implicit.  Or we could
    //    use sym_compare() in a loop.  Finally, we could just use
    //    new_contents.remove_dup_syms()

//  update -- "contents" -----------------
    contents.removeAll();
    contents.insert_last(new_contents);

    if(get_id()>0 && touched && parent_rtl)
    {
#ifndef NEW_UI
	RTLPtr old_rtl_head = parent_rtl->RTL();
	parent_rtl->regenerate_rtl (old_rtl_head);
#endif
    }
}


/*
   START-LOG-------------------------------------------

   $Log: browserRTL.h.C  $
   Revision 1.14 2001/10/30 09:09:23EST ktrans 
   Remove doc_proj/ParaDOCS subsystem
// Revision 1.11  1994/03/20  23:25:43  boris
// Bug track: #6721
// Fixed filter reset on Edit operations and "blinking" problem in viewerShell
//
// Revision 1.10  1994/01/13  23:35:46  davea
// bug 5796
// added an if !new_contents.includes, before
// adding a new symbol to the rtl.
//
// Revision 1.9  1993/12/16  18:49:55  azaparov
// Bug track: 5634
// Fixed bug 5634
//
// Revision 1.8  1993/12/13  15:57:53  pero
// Bug track: 0
// Rewriting ::xref_notify() to use insert_last() instead of remove_index() on
// symbol arrays (symbolArr).
//
// Revision 1.7  1993/11/30  15:55:59  azaparov
// Fixed bug 5080
//
// Revision 1.6  1993/09/28  20:49:20  azaparov
// Bug track: 4752
// Bug 4752 fixed
//
// Revision 1.5  1993/06/24  23:15:40  sergey
// Checked if corresponding xref is writable in xref_notify(). Improved performance and fixed bug #3713.
//
// Revision 1.4  1993/06/03  23:52:34  sergey
// Added filter of functions without definitions to xref_notify(). Fix of bug #3455.
//
// Revision 1.3  1993/04/30  20:16:59  sergey
// Filtered out modules in "added" from System project (control/doc). Fixed bug #3591.
//
// Revision 1.2  1993/03/13  01:12:02  davea
// fix symbolPtr when it's an xrefSymbol
//
// Revision 1.1  1993/02/05  05:01:09  glenn
// Initial revision
//
   END-LOG---------------------------------------------
*/
