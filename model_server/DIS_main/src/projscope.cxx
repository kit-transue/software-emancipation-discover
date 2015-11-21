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
#include <cLibraryFunctions.h>
#ifndef _genError_h
#include <genError.h>
#endif

#ifndef _genString_h
#include <genString.h>
#endif
#include <messages.h>

#include <proj.h>
#include <projList.h>
#include <browserShell.h>
#include <save_app_dialog.h>
#include <symbolTreeHeader.h>
#include <objRelation.h>
#include <Relational.h>
#include <symbolArr.h>
#include <symbolPtr.h>
#include <objArr.h>

#include "projscope.h"

// PRE-PROCESSOR DEFINITIONS

projModule * app_get_mod(appPtr);
extern objSet& get_list_of_rtl();

symbolArr all_projs;

void load_projlists(symbolArr& visible_projs, symbolArr& hidden_projs)
{
    Initialize(load_projlists);
    projList::full_list->get_search_list(all_projs);
    int sz = all_projs.size();
    for (int i=sz-1; i>=0; i--) {
        symbolPtr sym = all_projs[i];
	projNode *pn = checked_cast(projNode, sym);
	if (!pn->is_visible_proj())
	    all_projs.remove_index(i);
    }
    
    projList::search_list->get_search_list(visible_projs);

    sz = visible_projs.size();
    for (i=sz-1; i>=0; i--) {
	projNode *pn = checked_cast(projNode, visible_projs[i]);
	if (!pn->is_visible_proj())
	    visible_projs.remove_index(i);
    }
    
    symbolPtr s;
    ForEachS(s, all_projs)
     {
	if (!visible_projs.includes(s))
	    hidden_projs.insert_last(s);
     }

}

objArr *visible_projs_hiding = 0;
void set_projs_hiding(symbolArr &visible_projs)
{
  if (visible_projs_hiding)
    delete visible_projs_hiding;
  visible_projs_hiding = new objArr;
  symbolPtr sym;
  ForEachS(sym, visible_projs) {
    visible_projs_hiding->insert_last((Obj *)sym);
  }
}

objArr *get_projs_hiding()
{
  return visible_projs_hiding;
}


void unload_projlists(symbolArr& visible_list)
{
    Initialize(unload_projlists);
    int visible_count = visible_list.size();
    symbolArr visible_projs;
    int i;

    for (i = 0; i < visible_count; i++)
    {
		 symbolPtr vlist = visible_list[i];
		 char *vname = vlist.get_name();
	symbolPtr s;
	ForEachS(s, all_projs) {
		 char* name = s.get_name();
		 if (name && !strcmp(s.get_name(), vname))
		 {
		     visible_projs.insert_last(s);
		     break;
		 }
        }
    }

    symbolArr rule_projs;
    projList::search_list->get_search_list(rule_projs);

    int sz = rule_projs.size();
    for (i=sz-1; i>=0; i--)
    {
        projNode *pn = checked_cast(projNode, rule_projs[i]);
        if (!pn->is_visible_proj())
             visible_projs.insert_last(pn);
    }

    symbolArr base_list;
    projList::search_list->get_search_list(base_list);
    objArr* modified = app::get_modified_headers();
    objArr* mod_in_proj = new objArr;
    symbolArr prs;
    symbolPtr sym;
    ForEachS(sym,base_list) {
      if (!visible_projs.includes(sym)) {
	prs.insert_last(sym);
      }
    }
    start_transaction(){
    objArr *module_in_proj = new objArr;
    Obj* ap;
    ForEach(ap, *modified) {
      if (prs.includes(app_get_proj(checked_cast(app, ap)))) {
	projModule *pm = app_get_mod(checked_cast(app, ap));
	if (pm)
	  module_in_proj->insert_last(pm);
	mod_in_proj->insert_last(ap);
      }
    }
    set_projs_hiding(visible_projs);
//    if (mod_in_proj->size())
//      offer_to_save_list(0, Flavor_Hiding, mod_in_proj, dummy_really_unload,
//			 module_in_proj);
//    else {
    if (!mod_in_proj->size()) {
      //  we must remove all symbols from the loaded pmods
      projList::search_list->set_search_list(visible_projs);
    }

}end_transaction();
    all_projs.removeAll();
}
