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
#include <ctype.h>
#include <strstream.h>
#else /* ISO_CPP_HEADERS */
#include <cstdlib>
namespace std {};
using namespace std;
#include <cctype>
#endif /* ISO_CPP_HEADERS */

#include <msg.h>
#include <genError.h>
#include <representation.h>
#include <objOper.h>
#include <RTL.h>
#include <symbolTreeHeader.h>
#include <RTL_Names.h>
#include <RTL_externs.h>
#include <RTL_apl_extern.h>
#include <genString.h>
#include <xref.h>
#include <symbolSet.h>
#ifndef _symbolScope_h
#include <symbolScope.h>
#endif
#include <scopeMgr.h>
#include <driver_mode.h>

class xrefNotifier {
    symbolSet modified;
    symbolSet added;
    symbolSet deleted;
    objSet list_of_rtl;
    symbolScope** list_of_scopes;
    int scope_list_size;
    int scope_list_count;

friend objSet& get_list_of_rtl();
  public:
    xrefNotifier(): list_of_scopes(0), scope_list_size(0), scope_list_count(0) { reset(); }
    void reset ();
    void report (int, const symbolPtr&);
    void add (RTL*);
    void add(symbolScope*);
    void rem (RTL*);
    void rem(symbolScope*);
    void apply ();
    symbolTreeHeader* find_symbolTreeHeader (const symbolPtr &);
};

static xrefNotifier xref_notifier;


void xrefNotifier::reset ()
{
    Initialize (xrefNotifier::init);
    modified.set_type(1);
    added.set_type(1);
    deleted.set_type(1);

    modified.removeAll ();
    added.removeAll ();
    deleted.removeAll ();
}

void xrefNotifier::report (int flag, const symbolPtr& sym)
{
    if (!is_gui()) return;
  
    Initialize (xrefNotifier::report);

    if ((int)sym == 0)
	return;

    
    switch (flag) {
      case -1:  /* remove */
	    deleted.insert(sym);
	    modified.remove(sym);
	    added.remove(sym);
	break;

      case 0:  /* modified */
	    modified.insert(sym);
	    IF (sym.relationalp() && deleted.includes(sym))
		msg("(Intern error: we are getting xrefNotifier modify after delete)\n") << eom;
	break;

      case 1:  /* added */
	    added.insert(sym);
	    deleted.remove(sym);
	    IF (sym.relationalp() && modified.includes(sym))
		msg("(Intern error: we are getting xrefNotifier add after a modified already existed\n") << eom;
	break;
    }
}

void xrefNotifier::add (RTL* rtl)
{
    Initialize (xrefNotifier::add);

    if (!list_of_rtl.includes (rtl))
	list_of_rtl.insert (rtl);
}

void xrefNotifier::add(symbolScope* scope) {
   Initialize(xrefNotifier::add(scope));

   for (int i = 0; i < scope_list_count; i++) {
      if (list_of_scopes[i] == scope) {
	 return;
      }
   }

   if (scope_list_count >= scope_list_size) {
      scope_list_size = scope_list_count + 10;
      if (!list_of_scopes) {
	 // sun4 apparently can't handle realloc(0,...)
	 list_of_scopes = (symbolScope**) malloc(scope_list_size * sizeof(symbolScope*));
      }
      else list_of_scopes = (symbolScope**) realloc((char*) list_of_scopes, scope_list_size *
			       sizeof(symbolScope*));
   }

   list_of_scopes[scope_list_count++] = scope;
}

void xrefNotifier::rem (RTL* rtl)
{
    Initialize (xrefNotifier::rem);

    if (list_of_rtl.includes (rtl))
	list_of_rtl.remove (rtl);
}

void xrefNotifier::rem(symbolScope* scope) {
   Initialize(xrefNotifier::rem(scope));

   for (int i = 0; i < scope_list_count; i++) {
      if (list_of_scopes[i] == scope) {
	 scope_list_count--;
	 for (; i < scope_list_count; i++) {
	    list_of_scopes[i] = list_of_scopes[i + 1];
	 }
	 break;
      }
   }
}

void proj_add_to_loaded();

void xrefNotifier::apply ()
{
    Initialize (xrefNotifier::apply);

//    proj_add_to_loaded();

    bool notify_scopes = false;

    if(modified.size() || added.size() || deleted.size())
    {
	symbolArr mod, ad, del;

	symbolPtr el2;
	ForEachT (el2, deleted) {
	  if (el2.is_xrefSymbol() && el2.xrisnotnull()) {
	    del.insert_last (el2);
	    if (el2.get_kind() == DD_MODULE) {
	      notify_scopes = true;
	    }
	  } else if (el2.is_instance()) {
	    del.insert_last(el2);
	  }
	  if (!el2.is_xrefSymbol()) {
	    modified.remove(el2);
	    added.remove(el2);
	  }
	}

	symbolPtr el;
	ForEachT (el, modified) {
	  if (el.is_instance() || el.xrisnotnull())
	    mod.insert_last (el);
	}

	symbolPtr el1;
	ForEachT (el1, added) {
	    if (el1.is_instance() || el1.xrisnotnull()) {
		ad.insert_last (el1);
		if (el1.get_kind() == DD_MODULE) {
		   notify_scopes = true;
		}
	     }
	}
 

	reset ();

	if(mod.size() || ad.size() || del.size())
	{
	    objSet list_copy = list_of_rtl;
	    Obj* rtl;
	    ForEach (rtl, list_copy) {
		checked_cast (RTL, rtl)->xref_notify (mod, ad, del);
	    }
	}
	if (notify_scopes) {
	   scopeMgr_report_cm_command();
	   projNodePtr homep = projNode::get_home_proj();
	   for (int i = 0; i < scope_list_count; i++) {
	      list_of_scopes[i]->update_proj(homep);
	   }
	}
    }
}
#ifdef XXX_xrefNotifier__find_symbolTreeHeader
symbolTreeHeader* xrefNotifier::find_symbolTreeHeader (const symbolPtr& s)
{
    Initialize (xrefNotifier::find_symbolTreeHeader);

    fsymbolPtr  xref = s.get_xrefSymbol ();
    
    Obj* el;
    ForEach (el, list_of_rtl) {
	RTL* rtl = checked_cast (RTL, el);
	if (is_symbolTreeHeader (rtl) && !is_inheritanceSymTreeHdr (rtl)) {
	    RTLNode* rtl_node = checked_cast (RTLNode, rtl->get_root ());
	    symbolArr& cont = rtl_node->rtl_contents ();
	    if (cont.size () == 0)
		continue;
	    symbolPtr sym = cont[0];
	    fsymbolPtr  xr = sym.get_xrefSymbol ();
	    if (xref == xr)
		return checked_cast (symbolTreeHeader, rtl);
	}
    }
    return NULL;
}

/* External interface functions */

symbolTreeHeader* find_symbolTreeHeader (const symbolPtr& s)
{
    return xref_notifier.find_symbolTreeHeader (s);
}
#endif

#define gen_extern(x,par,arg) \
void paste(xref_notifier_,x) par { xref_notifier. x arg; }

gen_extern(apply, (), ());
gen_extern(report, (int f, const symbolPtr& s), (f, s));
gen_extern(add, (RTLPtr rtl), (rtl));
gen_extern(add, (symbolScope* scope), (scope));
gen_extern(rem, (RTLPtr rtl), (rtl));
gen_extern(rem, (symbolScope* scope), (scope));


/*
   START-LOG-------------------------------------------

   $Log: RTL_notify.cxx  $
   Revision 1.16 2001/11/07 14:07:54EST ktrans 
   Remove dormant project code, -verify_pdf option, and slow put w/ feedback.  See no_slow_put branch for details.
// Revision 1.16  1994/08/01  15:28:04  bhowmik
// Bug track: 7320
// .
//
// Revision 1.15  1994/04/15  14:19:53  davea
// bug 6649
// remove call to proj_add_to_loaded, as the loaded-files
// rtl is now handled in by the calls to set_actual_mapping.
// Also add extra debug error checking to xrefNotify
//
// Revision 1.14  1994/03/05  14:45:13  trung
// Bug track: 0
// fixes for prop to subsys, notify rtl, restoring subsys
//
// Revision 1.13  1994/02/08  21:03:07  trung
// Bug track: 0
// modify xrefNotifier::apply for symbols removed
//
// Revision 1.12  1994/01/13  15:18:54  trung
// fixes for check in
//
// Revision 1.11  1994/01/12  00:45:20  davea
// bugs 5940, 5670, 5693, 5882,5896, 5889, and 5900
//
// Revision 1.10  1994/01/08  18:32:34  trung
// Bug track: 5741
// fix this symtom for the 4th time and still going
//
// Revision 1.9  1993/10/04  21:17:38  trung
// Bug track: 4909
// fix deleting a file
//
// Revision 1.8  1993/09/02  18:30:58  mg
// loaded files
//
// Revision 1.7  1993/04/14  11:05:48  wmm
// Performance improvement: do reset() before propagation so same
// pending notifications aren't reprocessed in case of recursion.
//
// Revision 1.6  1993/03/19  00:08:04  aharlap
// cleanup for paraset compatibility
//
// Revision 1.5  1993/03/10  19:53:38  davea
// converted xrefSymbol to symbolPtr
//
// Revision 1.4  1993/03/03  16:40:08  aharlap
// fixed bug with updating call tree # 2754, # 2234
//
// Revision 1.3  1993/02/01  16:53:36  glenn
// Check size of arrays in apply before proceeding.
//
// Revision 1.2  1993/01/03  20:43:15  aharlap
// added xrefNotifier::find_symbolTreeHeader
//
   END-LOG---------------------------------------------
*/
