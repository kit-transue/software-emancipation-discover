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
#include <machdep.h>
#include <proj.h>

#include <messages.h>

#include <cLibraryFunctions.h>
#include <pdf_tree.h>

#include <xref.h>
#include <db.h>
#include <path.h>
#include <fileCache.h>

int smt_is_loaded(smtHeader* sh);


projModule* projHeader::find_working_module (const char* ln)
{
    Initialize(projHeader::find_working_module);

    if (!ln) return NULL;

    projNode* project;
    int i;

    for (i=1, project = projList::search_list->get_proj(i);
	 project;
         project = projList::search_list->get_proj(++i)) {

	if (project->get_xref() && project->get_xref()->is_writable()) {
	    projModule* module = project->find_module(ln);
	    if (module){
	        return module;
            }
	}
    }
    return NULL;
}

// return amount of mapped projects
int projHeader::make_new_filename (const char* fn)
{
    Initialize(projHeader::make_new_filename);

    if (!fn) return 0;

    projNode* project;
    genString ln;

    int i, count = 0;

    for (i=1, project = projList::search_list->get_proj(i);
	 project;
         project = projList::search_list->get_proj(++i)) {
	project->fn_to_ln (fn, ln);
	if (ln.length()) {
	    count ++;
	    project->make_new_module ((const char *)ln, 1);
	}
    }

    return count;
}

appPtr projHeader::load_module(const char* ln)
{
    Initialize(projHeader::load_module);

    if (!ln)
	return NULL;

    appPtr app_head = projModule::get_app(ln);

    if (app_head) {
	int tp = app_head->get_type();
	//boris: 111797. Report App_SMT too if it is not a fake
	if (tp != App_SMT || smt_is_loaded((smtHeader*)app_head))
	   return app_head;
    }

    projNode * pmod_proj = projHeader::get_pmod_project_from_ln(ln);
    appPtr ah = (pmod_proj)?pmod_proj->restore_file_wo_module(ln):0;
    if (ah)
      return ah;

    projModule* module = find_module(ln);
    if (module)
	return module->load_module_internal(0);

    return NULL;
}


void projHeader::save_all_projects()
{
    Initialize(projHeader::save_all_projects);

    objArr* apps;
    Obj* el;
    projHeaderPtr pheader;

    apps = app::get_app_list();

    ForEach (el, *apps) {
	if ((checked_cast(app,el))->get_type() == App_PROJECT) {
	    pheader = checked_cast(projHeader,el);
	    db_save (pheader, 0);
        }
    }
}




