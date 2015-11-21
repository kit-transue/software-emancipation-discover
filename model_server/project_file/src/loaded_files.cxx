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
// loaded_files.C
//------------------------------------------
// synopsis:
// 
// 
//------------------------------------------

// INCLUDE FILES

#include <proj.h>
#include <RTL.h>
#include <loadedFiles.h>
#include <RTL_externs.h>
#ifndef ISO_CPP_HEADERS
#include <strstream.h>
#else /* ISO_CPP_HEADERS */
using namespace std;
#endif /* ISO_CPP_HEADERS */
#include <smt.h>

RelClass(externApp);

RelClass(steDocument);
RelClass(groupHdr);
RelClass(assocInstance);
RelClass(oodt_relationHeader);
RelClass(objRawApp);

int offer_to_unload_list(class objArr* apps, class symbolArr* mods);
projModulePtr app_get_mod(appPtr);


// Things still to work on:
//    if loaded_files is to contain STE or RAW files, add them in
//    fix File->Update

// prototype for function defined in ui/browser/browser_callbacks.C
projModulePtr projModule_of_symbol(symbolPtr& sym);

// main implementation for both Loaded-files and Needs-to-be-reparsed,
//      two pseudo projects in the browser


// There are projNode::loaded_files and projNode::unparsed_files,
//    two static instances of
//    the loadedFiles class.  This class is derived from RTL

// The code in this class gets control in one of three
//    static member functions:
//       loadedFiles::proj_add_to_loaded()
//       loadedFiles::change_reparse_flag()
//       loadedFiles::proj_remove_from_loaded()
//
//  Whenever someone
//   changes, or tries to changes member field actual_mapping of an smtHeader,
//   we get control in the first two places.  This field indicates
//   whether or not a particular loaded file has an up-to-date smt.
//  There is some optimization to avoid calling it too often, but that is
//    the general idea.
//  If we add STE and/or RAW files, then  proj_add_to_loaded() will be
//    called from somewhere else as well.

//
// In both the app and the smtHeader destructors, our static member function
//    loadedFiles::proj_remove_from_loaded()  function gets called.  This
//    is responsible for removing any trace of this app from our stuff.




init_relational(loadedFiles,RTL);

// The two pointers to instances of loadedFiles are kept in
//    static pointers maintained in projNode.  I would think it cleaner
//    to have them here, but did not want to change so much.


loadedFiles::loadedFiles(char* nm)
: RTL(nm)
{
    Initialize(loadedFiles::loadedFiles);
}

loadedFiles::~loadedFiles()
{
}

int loadedFiles_check(const char * lname)
{

  if(!lname)
    return 0;
 Initialize(loadedFiles_check);
  loadedFiles *lf = projNode::loaded_files;
  if (lf) {
    RTLNodePtr lrtl = checked_cast(RTLNode, lf->get_root());
    symbolArr& larr = lrtl->rtl_contents();

    symbolPtr sym;
    
    ForEachS(sym, larr) {
      const char * nm = sym.get_name();
      if(nm && strcmp(lname, nm) == 0){
	return 1;
      }
    }
  }
  return 0;
}

void loadedFiles_get_apps(objArr& app_headers)
{
 Initialize(loadedFiles_get_apps);
  loadedFiles *lf = projNode::loaded_files;
  if (lf) {
    RTLNodePtr lrtl = checked_cast(RTLNode, lf->get_root());
    symbolArr& larr = lrtl->rtl_contents();

    symbolPtr sym;
    
    ForEachS(sym, larr) {
      app*ah = sym.get_def_app();
      if(ah)
	app_headers.insert_last(ah);
    }
  }
}

int smt_is_loaded(smtHeader* sh);
static bool is_reasonable_file_sym(appPtr module)
// Decide whether this app is the kind that might be considered
//     for loaded_files
{
    Initialize(is_reasonable_file_sym);
    bool return_value=0;
    if (module) {
	int tp = module->get_type();
	switch (tp) {
	  case App_SMT:        // if smtHeader
	    if (smt_is_loaded((smtHeader*)module)) {
	      return_value = 1;
	    }

	    break;

	case App_SUBSYS:
	  {
	    return_value = 1;
	  }
	    break;

	  default:
	    return_value = 0;      // zero for all other types of app
	}
    }
    return return_value;
}

void loadedFiles::proj_add_to_loaded(appPtr module)   // static
//add_to_loaded
//
//add module to the list of loaded files
//  this is the function called from outside to identify a new module that is
//  File->Load ed.
{
    Initialize(loadedFiles::proj_add_to_loaded);
    loadedFiles* lf = projNode::loaded_files;
    
    if (lf && is_reasonable_file_sym(module)) {
        projModule * pm = module->get_module();
	rtl_add_obj(checked_cast(RTLNode, lf->get_root()), pm );
    }
}

//  wrapper function
void proj_remove_from_loaded(appPtr module)      
// remove_from_loaded
//
// remove module from the list of Loaded Files
    //  called from the app::~app  (destructor)
    //     which is located in obj/src/objOper.h.C
{
    Initialize(remove_from_loaded);
    loadedFiles::proj_remove_from_loaded(module);
}


void loadedFiles::proj_remove_from_loaded(appPtr module)  //  static
// remove_from_loaded
//
// remove module from the list of Loaded Files
    //  called from the app::~app  (destructor)
    //  and from the smtHeader::~smtHeader (destructor) for good measure
{
    Initialize(remove_from_loaded);
    if(module==0) return;
    
    projModule * pm = appHeader_get_projectModule(module);

    if (pm) {
	loadedFiles* lf = projNode::loaded_files;
	if (lf) 
	    rtl_delete_obj(checked_cast(RTLNode, lf->get_root()), pm);
	
	loadedFiles* uf = projNode::unparsed_files;
	if (uf) 
	    rtl_delete_obj(checked_cast(RTLNode, uf->get_root()), pm);
    }
}

void  loadedFiles::get_unparsed_files(symbolArr& uarr)          // static
// return contents of the unparsed_files rtl
{
    Initialize(loadedFiles::get_unparsed_files);
    loadedFiles* uf = projNode::unparsed_files;
    if (uf) {
	RTLNodePtr urtl = checked_cast(RTLNode, uf->get_root());
//	symbolArr& uarr = urtl->rtl_contents();
//	return uarr;
	uarr.insert_last(urtl->rtl_contents());
    }
}

void loadedFiles::get_loaded_files(symbolArr &arr)
{
  Initialize(loadedFiles::get_loaded_files);

  loadedFiles *lf = projNode::loaded_files;
  if (lf) {
    RTLNodePtr rtl = checked_cast(RTLNode, lf->get_root());
    arr.insert_last(rtl->rtl_contents());
  }
}

void  loadedFiles::change_reparse_flag(smtHeader* ah, uint flag)    // static
//  Flag that the reparse flag may have changed for this module
{
    Initialize(loadedFiles::change_reparse_flag);
    loadedFiles* uf = projNode::unparsed_files;
    if (!uf) return;
    if (!ah) return;
    app * module = (app*) ah;
    if (flag)
	rtl_add_obj(checked_cast(RTLNode, uf->get_root()), module);
    else
	rtl_delete_obj(checked_cast(RTLNode, uf->get_root()), module);
}

//   obsolete wrapper function
void proj_add_to_loaded(appPtr )
//  this is the function that used to be called from outside
//    to identify a new module that is
//  File->Load ed.   Referenced by  app::append_to_list, which is called by
    // each of the app:: constructors
{
}

boolean app_needs_to_be_saved(app * that) // should be virtual function
{
    Initialize(app_needs_to_be_saved);

    char const *name = that->get_name();
    if (!name || !*name)
	return 0;

    if( (is_smtHeader(that) && checked_cast(smtHeader,that)->parsed) ||
	(is_steDocument(that) && strcmp(name, "ste-clip-board")) ||
	is_externApp(that) ||
        //	is_assocInstance (that) ||
        //	is_oodt_relationHeader(that) ||
	is_objRawApp(that)
	) {
	if(that->is_modified() || that->is_imported()){
	    projModule *mod = that->get_module();
	    if(mod){
		if(mod->is_project_writable())
		    return 1;
		else
		    return 0;
	    }else
		return 1;
	}
    }

    return 0;
}

projNode* app::make_current()
{
    projNode* project = get_project();
    if (project) {
	projNode *old_proj = projNode::get_current_proj ();
	if (project->get_xref()&& project->get_xref()->is_writable())
	    projNode::set_current_proj (project);
	else
	    projNode::set_current_proj (projNode::get_control_project());
	return old_proj;
    } else 
	return 0;
}

projModule* app::get_module()
{
    projModule* module = NULL;
    projNode* project;
    
    if (is_objRawApp(this)) {
	for (int i=0; project = projList::search_list->get_proj(i); i++) {
            genString ln;
	    project->fn_to_ln (get_filename(), ln);
	    if (ln.length()) 
		module = project->find_module (ln);
	        if (module)
		    return module;
	}
    } else {
        module = appHeader_get_projectModule(this);
	if (!module) {
	    project = app_get_proj(this);
	    if (!project) {
		module = NULL;
	    } else {
		module = project->find_module(get_filename());
		if (module) {
		    projectModule_put_appHeader (module, this);
		    projectNode_put_appHeader (project, this);
		}
	    }
	}
    }

    return module;
}


projNode* app::get_project()
{
     return app_get_proj(this);
}


loadedFiles * create_loadedFiles(char* nm)
{
    return new loadedFiles(nm);
}
