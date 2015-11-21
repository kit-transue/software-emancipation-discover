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
// enable_reparse_buttons.C
//------------------------------------------
// synopsis:
// Enable or disable the "Reparse" buttons for the viewers of an smtHeader
//------------------------------------------

// include files

#include "smt.h"
#include "viewerShell.h"
#include "ddict.h"
#include "xref.h"
#include "ldrOODT.h"
#include "symbolTreeHeader.h"
#include <view.h>
// function definitions


// Check if a particular view as represented by the ldr
// depends on the specified smt header.

static bool view_contains(ldrPtr ldr_head, smtHeaderPtr smth)
{
    Initialize(view_contains);
	
    appPtr app_head = checked_cast(app, get_relation(app_of_ldr, ldr_head));
    if(app_head) {
	if(is_smtHeader(app_head)) {
	    return(smth == checked_cast(smtHeader, app_head));
	}
	if(is_symbolTreeHeader(app_head)) {
	    RTLNode *rtl_node = checked_cast (RTLNode, app_head->get_root ());
	    symbolArr &contents = rtl_node->rtl_contents();
	    int index, size = contents.size();
	    for(index = 0; index < size; ++index) {
		fsymbolPtr  xsym = contents[index].get_xrefSymbol();
		if(xsym.is_loaded()) {
		    ddElement *def_dd = xsym->get_def_dd();
		    if(def_dd) {
			appPtr app = def_dd->get_main_header();
			if(is_smtHeader(app) && (smtHeader *)app == smth)
			    return true;
		    }
		}
	    }
	}
    }
    return false;
}

// Find ldrOODT views that depend on the specified smt header and add
// the header to the pending list of the corresponding ldrOODT.

void
add_to_pending_lists(smtHeader *smth)
{
	Initialize(add_to_pending_lists);
	viewerShell *vs;
	int i, j;

	for(i=0; vs = viewerShell::get_vs_by_idx(i); i++) {
		RTLNodePtr rtlnode = checked_cast(RTLNode, vs->get_rtl()->get_root());
		symbolArr &view_array = rtlnode->rtl_contents();
		int size = view_array.size();
		for(j = 0; j < size; j++) {
			viewPtr v = checked_cast(view, view_array[j]);
			if(v) {
				ldrPtr ldr_head = checked_cast(ldr, get_relation(ldr_of_view, v));
				if(ldr_head && is_ldrOODT(ldr_head) && view_contains(ldr_head, smth)) {
					checked_cast(ldrOODT, ldr_head)->add_pending(smth);
				}
			}
		}
	}
}

// Find viewers that depend on the specified smt header and make
// their "Update" button reflect the enabled flag.
static void
update_Update_buttons(smtHeaderPtr smt_header, boolean enabled)
{
	Initialize(update_Update_buttons);
	viewerShell *vs;
	int i, j;

	for(i=0; vs = viewerShell::get_vs_by_idx(i); i++) {
		for(j=0; j < viewerShell::MAX_VIEWERS; j++) {
			viewPtr v;
			viewer *vr;

			if(v = vs->find_view(j)) {
				// should use index but the method does not exist
				vr = vs->viewer_of_view(v);
				ldrPtr ldr_head = checked_cast(ldr, get_relation(ldr_of_view, v));
				if(ldr_head && view_contains(ldr_head, smt_header)) {
					vr->enable_reparse(enabled);
				}
			}
		}
	}
}

void enable_reparse_buttons(smtHeaderPtr smt_header, boolean enabled)
{
    Initialize(enable_reparse_buttons);

	update_Update_buttons(smt_header, enabled);
#if(0)
    Obj& ldr_collection = *get_relation(ldr_of_app, smt_header);
    ObjPtr ldr_obj; ForEach(ldr_obj, ldr_collection)
    {
	ldrPtr ldr_header = checked_cast(ldr,ldr_obj);
	Obj& view_collection = *get_relation(view_of_ldr, ldr_header);
	ObjPtr view_obj; ForEach(view_obj, view_collection)
	{
	    viewPtr view_header = checked_cast(view,view_obj);
	    viewerShell* vs = viewerShell::viewerShell_of_view(view_header);
	    if(vs)
	    {
		viewer* v = vs->viewer_of_view(view_header);
		if(v)
		    v->enable_reparse(enabled);
	    }
	}
    }
#endif
    Return
}



/*
   START-LOG-------------------------------------------

   $Log: enable_reparse_buttons.C  $
   Revision 1.5 1999/03/29 12:25:03EST Sudha Kallem (sudha) 
   changes to support new pmod format
Revision 1.2.1.3  1992/11/23  22:37:53  wmm
typesafe casts.

Revision 1.2.1.2  1992/10/09  20:20:59  jon
RCS History Marker Fixup



   END-LOG---------------------------------------------

*/

