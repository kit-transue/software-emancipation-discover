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
#include <msg.h>
#ifndef _WIN32
#include <sys/param.h>
#endif
#ifndef ISO_CPP_HEADERS
#include <errno.h>
#else /* ISO_CPP_HEADERS */
#include <cerrno>
namespace std {};
using namespace std;
#endif /* ISO_CPP_HEADERS */

#include <fcntl.h>

#include "dfa.h"
#include "dfa_graph.h"
#include <messages.h>
#include <genTmpfile.h>
#include <customize.h>
#include <fileCache.h>
#include "fileEntry.h"
#include <xxinterface.h>

#include <objRelation.h>
#include <proj.h>
#include <xref.h>
#include <path.h>

#ifndef _pathdef_h
#include <pathdef.h>
#endif

#include <RTL.h>
#include <RTL_externs.h>
#include <loadedFiles.h>
#include <ddKind.h>

#include <vpopen.h>
#include <machdep.h>
#include <db_intern.h>
#include <pdf_tree.h>

#include <XrefTable.h>
#include <transaction.h>
#include <psetmem.h>
#include <proj_save.h>
#include <scopeMgr.h>

projModule *proj_make_module (char const *nm, projNode*parent, char const *ln)
{
  Initialize(proj_make_module);
   DBG{
    msg("name $1 parent $2 ln $3\n") << nm << eoarg << parent << eoarg << ln << eom;
   }

    Assert(parent);
    Assert(ln);
    Assert(nm);
    projModule * mod = parent->put_module(nm, ln);
    return mod;
}

void proj_get_modules(projNode *proj, objArr& modules);

void projNode::update_xref ()
{
    Initialize (projNode::update_xref);

    objArr modules;
    proj_get_modules(this, modules);

    int no_modules = modules.size();

    for(int ii = 0; ii<no_modules; ++ii) {
      projModule *module = checked_cast(projModule, modules[ii]);
      module->update_xref();
    }
}


#include <metric.h>
#include <smt.h>

void
projNode::update_metrics()
{
  Initialize(projNode::update_metrics);

  objArr modules;
  proj_get_modules(this, modules);
  int no_modules = modules.size();

  for (int ii = 0; ii < no_modules; ++ii) {
    projModule *module = checked_cast(projModule, modules[ii]);
    module->restore_module();
    appPtr app_head = module->get_app();

    if (app_head && is_smtHeader(app_head)) {
      smtHeaderPtr smt = smtHeaderPtr(app_head);

      if (!smt->foreign && smt->parsed)
	MetricSet::set_metrics_of_smtHeader(smt, true);
    }
  }
}

void
projNode::updateDFA()
{
  Initialize(projNode::updateDFA());

  if (_lh(LIC_DFA)) {
    if (_lo(LIC_DFA)) {
      _lm(LIC_DFA);
      return;
    }
  }

  GDSet *hashtable = project_get_table(this);

  if (!hashtable)
    hashtable = new GDSet(this);

  if (hashtable->getStatus()) {
    GDSet::setDFApass(true);

    if (!GDSet::Hashtable_)
      GDSet::loadStandardFunctions();

    hashtable->closeGraphs();
    GDSet::saveDFA(hashtable, GDT_GLOBAL, true);
    GDSet::setDFApass(false);
  }
}

static int
get_search_list_size()
{
  Initialize(get_search_list_size);

  symbolArr &read_projs = checked_cast(RTLNode,
			  projList::search_list->get_root())->rtl_contents();

  return
    read_projs.size();
}


/* static */
projModule * projNode::make_new_module(const char * nm, int is_ln)
{
    Initialize(projNode::make_new_module);

    projNode* proj;

    if (!nm || !*nm) 
	return NULL;

    projNode* old_proj = projList::search_list->writable();
    projList::search_list->writable(projNode::control_proj);

    // some hacking by Trung, since do not know why prev line should be there
    int already_there = get_search_list_size();
    projList::search_list->add_proj(old_proj);
    if (get_search_list_size() > already_there)
      already_there = 0;

    proj = projNode::get_current_proj();
    genString ln;
    genString ln_imp;

    genString new_name;
    genString fn;

    if (is_ln) {
	if (!proj) {
            msg("ERROR: No project found: cannot create '$1'.") << nm << eom;
            return NULL;
        }

	if (strchr(nm, '/') == NULL) {
	    new_name += proj->get_ln();
	    new_name += "/";
	    new_name += nm;
	    nm = (char*)new_name;
        } 

        ln = nm;
        proj->ln_to_fn((char*)nm, fn);
	if (fn.str())
	    nm = fn;
        proj->fn_to_ln_imp ((char*)fn, ln_imp);
    } else {
	proj = projHeader::fn_to_project (nm);
	if (!proj) {
            msg("ERROR: No project found: cannot create '$1'.") << nm << eom;
	    if (!already_there)
	      projList::search_list->rem_proj(old_proj);
	    projList::search_list->writable(old_proj);
            return NULL;
        }
        proj->fn_to_ln ((char*)nm, ln);

	if (!ln.length()) {
            genString ret_path;
            project_convert_filename((char *)nm, ret_path);
            proj->fn_to_ln ((char *)ret_path.str(), ln);
            proj->fn_to_ln_imp ((char *)ret_path.str(), ln_imp);

        } else {
            proj->fn_to_ln_imp ((char*)nm, ln_imp);
	}
    }

    if (ln_imp.length() == 0)
    {
        if (!(proj->get_map() && proj->get_map()->is_script()))
	    msg("ERROR: Could not get project name for '$1'.") << nm << eom;
	if (!already_there)
	  projList::search_list->rem_proj(old_proj);
        projList::search_list->writable(old_proj);
        return NULL;
    }

    if (!(proj = projNode::find_project(ln_imp, ln, 1))) {
	msg("ERROR: No project found for module $1.") << ln_imp.str() << eom;
	if (!already_there)
	  projList::search_list->rem_proj(old_proj);
        projList::search_list->writable(old_proj);
        return NULL;
    }

    projModule* mod =  proj_make_module (nm, proj, ln);
    if (!already_there)
      projList::search_list->rem_proj(old_proj);
    projList::search_list->writable(old_proj);

    scope_mgr.report_cm_command (CM_NEW_FILE, proj, (projNode *)NULL, (const char *)ln);

    return mod;
}
