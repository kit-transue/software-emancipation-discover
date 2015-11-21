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
// file dd_db.C

#ifndef ISO_CPP_HEADERS
#include <stdlib.h>
#else /* ISO_CPP_HEADERS */
#include <cstdlib>
using namespace std;
#endif /* ISO_CPP_HEADERS */

#include "Relational.h"
#include "cLibraryFunctions.h"
#include "customize.h"
#include "db.h"
#include "disbuild_analysis.h"
#include "driver_mode.h"
#include "groupHdr.h"
#include "linkTypes.h"
#include "machdep.h"
#include "messages.h"
#include "msg.h"
#include "proj.h"
#include "xref.h"

#include "db_decls.h"
#include "db_str_desc.h"

#include "db_save_str.h"
#include "db_str_desc.h"

#include "db_read_str.h"
#include "db_str_desc.h"


extern void add_dependency_module(projNode*, symbolPtr&);
static void put_hash (ddRoot*, objTree*, app*);
int ddKind_is_local(ddKind k);
extern void get_projs_to_search(objSet & os);

static ddElement * dd_null = 0;
static const int dd_save_size = 
      ((char*)&dd_null->aname) - ((char*)&dd_null->kind);

static symbolArr changedElems;
static symbolArr analyzed_files;


static char* copy_string(char*str)
{
  int len = str ? strlen(str) : 0;

  char * ptr = new char[len + 1];
  if (len)
      OS_dependent::bcopy(str, ptr, len);
  ptr[len] = '\0';
  return ptr;
}

void ddNode::add_changed_ddElems(ddElement *node)
{
    Initialize(ddNode::add_changed_ddElems);

    changedElems.insert_last(node);
    symbolPtr file = node->get_main_header();
    file = file.get_xrefSymbol();
    IF(file.isnull())
       ;
    else
        analyzed_files.insert_last(file);
}

void ddNode::remove_changed_ddElems()
{
    Initialize(ddNode::remove_changed_ddElem);

    changedElems.removeAll();
    analyzed_files.removeAll();
}

symbolArr *dd_obtain_changed_ddElems()
{
    Initialize(dd_obtain_changed_ddElems);

    return &changedElems;
}


int get_files_to_touch(symbolArr &objs, symbolArr &fnr)
{
    Initialize(get_files_to_touch);
    objSet prs;
    get_projs_to_search(prs);  
    linkTypes lts;
    lts.add(is_defined_in);
//    lts.add(ref_file);
    symbolPtr sym;
    symbolArr intern;

    ForEachS(sym, objs) {
	symbolPtr x = sym.get_xrefSymbol();
	if (x.isnotnull())
	    intern.insert_last(x);
    }
    Xref::symbols_get_links(intern, prs, lts, fnr);
    return fnr.size();
}


void dd_do_analysis(symbolPtr& elem, symbolArr& affected_files, symbolArr& excluded)
{
    symbolPtr x = elem.get_xrefSymbol();
    if (x.isnotnull()) {
        symbolPtr sym;
        symbolArr affected_symbols;
        x->get_link_chase_typedefs(used_by, affected_symbols);
        ForEachS(sym, affected_symbols) {
            symbolPtr filesym = sym->get_def_file();
            if (filesym.isnotnull()) {
                affected_files.insert_last(filesym);
	    }
        }
    }
}

void disbuild_XREF_add_including_file (symbolPtr& hdr, projNode *&project, symbolPtr& fs)
{
    Initialize(disbuild_XREF_add_including_file);
    fs = NULL_symbolPtr;
    project = NULL;

    projModule *module = NULL;;
    app* ah = XREF_get_files_including(hdr.get_name());
    if (ah) {
	const char *ln = ah->get_filename();
	if ( !(module = projHeader::find_module(ln)) ) {
	    msg("  ** ERR: No module for src $1,\n          that includes hdr $2\n") << ln << eoarg << hdr.get_name() << eom;
	}
	delete ah;
    } else
	msg("  ** ERR: Could not find src file that includes hdr $1\n") << hdr.get_name() << eom;

    if (module) {
	symbolPtr x(module);
	fs = x.get_xrefSymbol();
	if (fs.isnotnull())
	    project = module->get_project();
	else {
	    msg("  ** ERR: No symbol for src $1,\n          that includes hdr $2\n") << module->get_name() << eoarg << hdr.get_name() << eom;
	}
    }
}

void dd_do_analysis(symbolArr* changed_ddElems, 
                      symbolArr& excluded, projNode* target_proj)
{  
  Initialize(dd_do_analysis);
  symbolArr affected_files;
  symbolPtr elem; symbolPtr hdr;

  ForEachS(elem, *changed_ddElems)
    {
      if (hdr.isnull() && elem->get_kind() == DD_MODULE) {
	  symbolPtr x = elem.get_xrefSymbol();
	  if (x.isnotnull()) hdr = x->get_def_file();
	  continue;
      }

      if (elem->get_kind() > DD_UNION || elem->get_kind() == DD_UNKNOWN)
	continue;
      
      symbolPtr x = elem.get_xrefSymbol();
      if (x.isnotnull())
	  dd_do_analysis(x, affected_files, excluded);
    }
  symbolArr files_to_touch;
  if (changed_ddElems) {
      get_files_to_touch(*changed_ddElems, files_to_touch);
  }
  affected_files.insert_last(files_to_touch);
  affected_files.usort();

  if (affected_files.size() == 0) return;

  symbolPtr src_file, file;

  ForEachS(file, affected_files) {
    if (file.is_xrefSymbol()){
      symbolPtr xr = file.get_xrefSymbol();
	  
      if(xr->xrisnull())
	continue;

      if(xr->get_kind() != DD_MODULE) {
	xr = xr->get_def_file();
	if(xr->xrisnull())
	  continue;
      }

      if(excluded.includes(xr))
	continue;

      Xref *Xr = xr.get_xref();
      projNode *project = (Xr)? Xr->get_projNode(): 0;
      if ((project == 0) && Xr)
	project = projNode::find_project(Xr);

      if (project == 0) 
	msg("Warning: Failed to get project for file:  $1\n") << xr.get_name() << eom;
      else {
	add_dependency_module(project, file);
	if (disbuild_analysis_mode() && src_file.isnull()) {
	    char const *ln = file.get_name();
	    // boris: assume that ln and phys_name extentions are the same
	    int is_header   = ! xref_not_header_file(ln); 
	    int is_included = (is_header || XREF_check_files_including(ln));
	    if(!is_included)
		src_file = file;  // Found at least one source file
	}
      }
    }
  }

  // looking for source files to include outdated header 
  // file without impacted source files
  if (disbuild_analysis_mode() && src_file.isnull() && hdr.isnotnull()) {
      projNode *project = NULL;
      symbolPtr fs;
      disbuild_XREF_add_including_file (hdr, project, fs);

      if (fs.isnotnull() && project) {
	  add_dependency_module(project, fs);
	  msg("  INF: adding random src $1, project $2,\n       that includes hdr $3\n") << fs.get_name() << eoarg << project->get_name() << eoarg << hdr.get_name() << eom;
      }
  }
}

void fine_grained_dependency_analysis(projNode *target_proj)
{
    Initialize(fine_grained_dependency_analysis);

    analyzed_files.usort();

    while(target_proj && !target_proj->get_map())
        target_proj = target_proj->find_parent();

    if (target_proj){
       symbolArr *changed_ddElems = dd_obtain_changed_ddElems();
       dd_do_analysis(changed_ddElems, analyzed_files, target_proj);
    }
    ddNode::remove_changed_ddElems(); // clean it
}

void ddNode::db_save(app*, appTree*nd, db_app& dba, db_buffer& dbb)
{
    Initialize(ddNode::db_save);

    if(! is_ddElement(nd)){
	if (is_ddRoot (nd))
	    dba.extype = DD_ROOT;
	else	
	    dba.extype = 0;
	return;
    }

    ddElement * node = (ddElement*) nd;

    dba.extype = node->get_kind();
    dba.flag   = node->get_is_def();
    
    dbb.put(node->get_ddname());
    dbb.put(node->def);
    dbb.put(node->aname);
    if (node->get_kind() == DD_MACRO)
	dbb.put(node->get_def_file());
    else if (node->get_kind() != DD_SEMTYPE && node->get_datatype() == 1) {
	dbb.put(node->get_def_file());
    }
    else
	dbb.put((char *) 0);
    // Save bitfield part of ddElement
    SAVEABLE_STRUCT_NAME(db_str_ddElement) dd_save_part;
    OSapi_bcopy(&node->kind, &dd_save_part, dd_save_size);
    StructSaver dd_saved_buf(&dd_save_part, STRUCT_SAVE_PARAMS(db_str_ddElement));
    dbb.put(dd_saved_buf.get_buffer(), dd_save_size);
//    dbb.put(&node->kind, dd_save_size);
}

ddNode* ddNode::db_restore(app*, appTree*, db_app& dba, db_buffer& dbb)
{
    Initialize(ddNode::db_restore);
  
  
    ddElement * node;
    switch(dba.extype){
      case 0:
	return db_new(ddNode,());  
      
      case DD_ROOT:
	return db_new (ddRoot, (DD_ROOT));

      default:
	node = db_new(ddElement, ());
	break;
    }

    char * str;
    dbb.get(str);
    delete node->name;
    node->name = copy_string(str);

    dbb.get(str);
    delete node->def;
    node->def = copy_string(str);

    dbb.get(str);
    delete node->aname;
    node->aname = copy_string(str);

    dbb.get(str);
    delete node->def_file;
    node->def_file = copy_string(str);
    
    SAVEABLE_STRUCT_NAME(db_str_ddElement) dd_save_part;
    char tmp[sizeof(dd_save_part)];
    dbb.get(tmp, sizeof(dd_save_part));
    StructReader::Read(tmp, &dd_save_part, STRUCT_READ_FUNCTION(db_str_ddElement));
    OSapi_bcopy(&dd_save_part, &node->kind, dd_save_size);
//    dbb.get(&node->kind, dd_save_size);
    return node;
}

void  ddNode::db_after_restore (db_buffer&)
{
    Initialize(ddNode::db_after_restore);

    ddRoot* dr = checked_cast (ddRoot, this);
    dr->db_fill ();
    dr->unset_dd_just_built_from_scratch();
    app* h = dr->get_main_header ();
    projNodePtr proj = (h) ? appHeader_get_projectNode(h) : NULL;
    if (proj == 0) {
      const char *fn = h->get_phys_name();
      if (fn)
	proj = projHeader::fn_to_project(fn, 1);
    }
    Xref* xr = (proj) ? proj->get_xref() : get_XREF();
    IF (xr == 0) {
	msg("Error: cannot get current project!") << eom;
	return;
    }
}


void ddRoot::db_fill ()
{
    Initialize (ddRoot::db_fill);

    macro_list = checked_cast (ddElement, get_first ());
    fdecl_list = checked_cast (ddElement, macro_list->get_next ());
    vdecl_list = checked_cast (ddElement, fdecl_list->get_next ());
    tdecl_list = checked_cast (ddElement, vdecl_list->get_next ());

    put_relation(macro_of_ddRoot, this, macro_list);
    put_relation(fdecl_of_ddRoot, this, fdecl_list);
    put_relation(vdecl_of_ddRoot, this, vdecl_list);
    put_relation(tdecl_of_ddRoot, this, tdecl_list);

    put_hash (this, this, checked_cast(app, get_relation(smtHeader_of_ddRoot, this)));
}

static void put_hash (ddRoot* dr, objTree* dn, app* ah)
{
    Initialize (put_hash);
    
    Assert (is_ddNode (dn));

    if (is_ddElement (dn)) {
	ddElement* ep = checked_cast (ddElement, dn);
	ep->set_xref_hash(ah);
	char const *name = ep->get_ddname ();
	if ( name && *name && !ddKind_is_local(ep->get_kind()))
	    dr->insert_hash (ep);
    }
    for (objTree* t = dn->get_first (); t; t = t->get_next ())
	put_hash (dr, t, ah);
}

