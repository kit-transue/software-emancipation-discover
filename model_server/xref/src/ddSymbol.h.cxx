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
#include <ddSymbol.h>
#include <msg.h>
#include <xref.h>
#include <XrefTable.h>
#include <xrefSymbol.h>

#include <disbuild_analysis.h>
#include <dd_utils.h>

init_relational(ddNode,appTree);
init_relational(ddSymbol,ddNode);  

Xref* app_get_xref(app*ah)
{
    if(!ah)
      return NULL;

    projNode* project = app_get_proj(ah);
    if (project)
	return project->get_xref();
    else
	return NULL;
}



char const *ddSymbol::get_file_name()
{
  app* sh = get_main_header();
  if (sh)
    return sh->get_filename();
  else return 0;
}

char const *ddSymbol::get_def_file()
{
  Initialize(ddSymbol::get_def_file);
  char const *fname = 0;
  if (get_is_def()) {
    app * ah = get_main_header();
    if (ah)
      fname =  ah->get_filename();
  }
  else if (get_kind() == DD_MACRO || get_kind() == DD_TYPEDEF
           || get_kind() == DD_ENUM || get_kind() == DD_ENUM_VAL
           || get_kind() == DD_CLASS)
    fname = def_file;
  else if ((get_kind() == DD_FUNC_DECL || get_kind() == DD_VAR_DECL) &&
	   !get_from_method() && is_static())
    fname = def_file;
  else if (get_kind() != DD_SEMTYPE && datatype == 1 &&	// "cloned" ref ddElement
	   get_kind() != DD_MODULE) {	// prevent proj as "def file" in pmod
     fname = def_file;
  }
  return fname;
}

void ddSymbol::set_def_file_with_good_name(char const *fn)
{
  Initialize(ddElement::set_def_file_with_good_name);

  if (fn) {
      if (def_file) 
	 delete def_file;
      def_file = new char[strlen(fn)+1];
      strcpy(def_file, fn);
  }
}

unsigned int ddSymbol::get_sw_tracking() {return filler; }

void ddSymbol::set_sw_tracking(int ii)
{
    filler = ii;
    set_lmd(get_time_of_day());
    symbolPtr sym = find_xrefSymbol();
    if (sym.xrisnotnull()) {
	// boris: 090298. Suppress writing in PMOD while disbuild analysis pass,
        //        instead printing the modified symbol list  
	if (disbuild_analysis_mode()) {
	    genString s;
	    get_display_string_from_symbol(sym, s);
	    if (s.length())
		msg("    $1\n") << (char const *)s << eom;
	} else {
	    sym->set_attribute(filler, SWT_Entity_Status , 2);
	    if (get_lmd() > 0)
		sym->set_last_mod_date((time_t) get_lmd(), sym.get_xref()->get_lxref());
	}
    }
}

symbolPtr ddSymbol::get_xrefSymbol() 
{
    return symbolPtr(get_xrefSymbol_priv());
}

// lookup Xref symbol, and self-insert if not found
xrefSymbol* ddSymbol::get_xrefSymbol_priv()
{
//  Initialize(ddSymbol::get_xrefSymbol_priv);
  if (ddKind_is_local(get_kind())) return 0;

  if(dd_sym.isnull())
    dd_sym = find_xrefSymbol();
  
  if (dd_sym.isnull())
    dd_sym =  Xref_insert_symbol(this, get_main_header());
  
  return dd_sym. operator -> ();
}

symbolPtr ddSymbol::find_xrefSymbol()
{
    return symbolPtr(find_xrefSymbol_priv());
}
// private version
// look up Xref symbol without self-insertion
xrefSymbol* ddSymbol::find_xrefSymbol_priv()
{
  Initialize(ddSymbol::find_xrefSymbol);
  if (dd_sym.isnotnull())
    return dd_sym.operator -> ();
  
  app* ah = checked_cast(app, get_main_header());
  Xref* Xr = ah? app_get_xref(ah): get_XREF();
  if (Xr == 0)
    return NULL;
  XrefTable* xr = Xr->get_lxref();
  IF (xr == 0) return
    NULL;

  return 
    xr->find_symbol(this);
}

//  this is the symbol for dd
void xrefSymbol::set_att(ddSymbol * dd)
{
    Initialize(xrefSymbol::set_att);
    if (dd == 0) return;
    app *h = dd->get_main_header();
    ddSymbol * dh = ddSymbolPtr(appHeader_get_dd(h));
    if (dh == 0) return;
    symbolPtr dhs = dh;
    dhs = dhs.get_xrefSymbol();
    if (dhs.xrisnull()) return;
    if (dd->get_is_def()) {
	add_link(is_defined_in, dhs, 1, dhs.get_xref()->get_lxref());
    }
    else {
	add_link(ref_file, dhs, 1, dhs.get_xref()->get_lxref());
    }
}

void ddNode::propagate(int flag, objArr* arr)
{
  appTree::propagate(flag,arr);
  if(flag & relationMode::U){		// Unload

  } else if(flag & relationMode::D){	// delete

  }
}
