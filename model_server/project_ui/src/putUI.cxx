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
// putUI.h.C
//------------------------------------------
// synopsis:
//    define the member functions for the put dialog box
//
//------------------------------------------
#include <cLibraryFunctions.h>
#include <msg.h>
#include <gtDlgTemplate.h>
#include <systemMessages.h>
#include <gtPushButton.h>
#include <genError.h>
#include <gtForm.h>
#include <gtLabel.h>
#include <gtSepar.h>
#include <gtTogB.h>
#include <gtList.h>
#include <gtRTL.h>
#include <gtHorzBox.h>
#include <gtTextEd.h>
#include <gtStringEd.h>
#include <path.h>
#include <proj.h>
#include <putUI.h>
#include <RTL_externs.h>
#include <psetmem.h>
#include <symbolSet.h>
#include <feedback.h>
#include <viewUIexterns.h>
#include <messages.h>
#include <cliUI.h>

#ifndef _browserShell_h
#include <browserShell.h>
#endif

#ifndef _fileCache_h
#include <fileCache.h>
#endif

#ifndef _genTmpfile_h
#include <genTmpfile.h>
#endif

#ifndef _ddKind_readable_names_h
#include <ddKind_readable_names.h>
#endif

extern FeedBack* fb_instance;

int configurator_to_icon();

static putUI *putUI_obj = 0;

// pdf files and count for selected modules for put -- begin
typedef struct
{
    genTmpfile *tnm;
    int  count;
} pdf_put_entry;

genArr(pdf_put_entry);

static genArrOf(pdf_put_entry) pdf_put_heads;
static int current_pdf_idx = 0;
static int cur_select_count = 0;
void   *make_tcl_string(char *str);
void   delete_tcl_string(void *_tcl_str);
char   *get_tcl_string(void *_tcl_str);


// pdf files and count for selected modules for put -- end


putUI *putUI::load_putUI(symbolArr& symbols)
// load all modules into the put dialog box
// popup the put dialog box if at least one module is loaded
{
    Initialize(load_putUI);

    if (putUI_obj == 0)
        putUI_obj = new putUI(NULL);

    objArr modules;
	if (putUI_obj->fill_modules(symbols, modules)){
	    symbolArr converted_modules;
	    for(int i = 0; i < modules.size(); i++)
		converted_modules.insert_last(modules[i]);
	    putUI_obj->do_put(converted_modules);
	}
    return putUI_obj;
}

putUI::putUI(gtBase* parent)
{
    Initialize(putUI::putUI);

    RTL* modules_ready_to_put = new RTL(0);
    RTL* modules_need_to_lock = new RTL(0);
    RTL* modules_3_file_merge = new RTL(0);
    RTL* modules_not_to_put   = new RTL(0);

    modules_ready_to_put_nodes = checked_cast(RTLNode,modules_ready_to_put->get_root());
    modules_need_to_lock_nodes = checked_cast(RTLNode,modules_need_to_lock->get_root());
    modules_3_file_merge_nodes = checked_cast(RTLNode,modules_3_file_merge->get_root());
    modules_not_to_put_nodes   = checked_cast(RTLNode,modules_not_to_put->get_root());

    return;
}

//----------------------------------------------

putUI::~putUI()
{
    Initialize(putUI::~putUI);
}

//----------------------------------------------


int putUI::fill_modules(symbolArr& symbols, objArr& result)
// load all modules into put dialog box
{
    Initialize(putUI::fill_modules);

    projModule* module;

    int at_least_one = 0;
    symbolPtr sym;
    ForEachS (sym,symbols)
    {
        if (sym.is_xrefSymbol())
        {
            fsymbolPtr xr = sym.get_xrefSymbol();

            if (xr->xrisnull())
                continue;

            if (xr.get_kind() != DD_MODULE)
            {
                xr = xr->get_def_file();
                if (xr->xrisnull())
                    continue;
            }

            projNode* project = projNode::find_project (xr->get_Xref());
            module = project->find_module (xr.get_name());
        }
        else if(is_projModule(sym))
            module = projModulePtr(RelationalPtr(sym));
        else if (is_commonTree(sym))
        {
            commonTreePtr common_tree = commonTreePtr(sym);
            appPtr app_head = checked_cast (app,common_tree->get_header());
            if (app_head)
                module = app_head->get_module();
            else
                continue;
        }
        else
            continue;

	if(result.includes(module))
	    continue;
	
        genString name = module->get_name();
        module = projNode::get_home_proj()->find_module(name);
        if (!module)
        {
            gtPushButton::next_help_context("Browser.Manage.Put.NotInHome");
            msg("ERROR: File \"$1\" does not exist in home project \"$2\"") << name.str() << eoarg << projNode::get_home_proj()->get_ln() << eom;
            continue;
        }

        if (module && module->is_raw_modified(3) == 0)
        {
	    result.insert_last(module);
	    at_least_one = 1;
        }
    }

    if (at_least_one) check_for_module_comment();

    return at_least_one;
}

//----------------------------------------------

// Check all modules for an initial comment.
void putUI::check_for_module_comment()
{
    Initialize (putUI::check_for_module_comment);
    bool found = false;

    // Check modules that are ready to put.
    symbolArr& readyNodes = modules_ready_to_put_nodes->rtl_contents();
    if (readyNodes.size() > 0) {
        found = get_first_module_comment(readyNodes);
	pall_button->set_sensitive(1);
        configurator_to_icon();
    }

    // Check modules that need to locked.
    if (!found) {
        symbolArr& lockedNodes = modules_need_to_lock_nodes->rtl_contents();
        if (lockedNodes.size() > 0) {
            found = get_first_module_comment(lockedNodes);
	    browser_but->set_sensitive(1);
            configurator_to_icon();
        }
    }

    // Check modules that need to merged.
    if (!found) {
        symbolArr& mergedNodes = modules_3_file_merge_nodes->rtl_contents();
        if (mergedNodes.size() > 0) {
            found = get_first_module_comment(mergedNodes);
            configurator_to_icon();
        }
    }

    // Check modules that don't need to be put.
    if (!found) {
        symbolArr& notReadyNodes = modules_not_to_put_nodes->rtl_contents();
        if (notReadyNodes.size() > 0) {
            found = get_first_module_comment(notReadyNodes);
            configurator_to_icon();
        }
    }
}

bool putUI::get_first_module_comment(symbolArr symbols)
{
    Initialize (putUI::get_first_module_comment);
    bool found = false;
    genString firstComment;
    if (symbols.size() > 0) {
        Obj* el;
        ForEachS (el,symbols) {
            projModule* module = checked_cast(projModule,el);
            module->get_module_comment(firstComment);
            if (firstComment.length() > 0) {
                found = true;
                break;
            }
        }
    }
    if (found) comment->text((char*)firstComment);
    return (found);
}



extern int get_modified_objects(symbolArr &, symbolArr & );

void report_modified_objects(FILE *fl, projModule *mod)
{
    if (!fl)
        return;
    symbolPtr mod_sym = mod;
    symbolPtr xmod    = mod_sym.get_xrefSymbol();
    if(xmod.xrisnull())
        return;
    OSapi_fprintf(fl, "%s:\n", TXT("Modified entities"));
    OSapi_fprintf(fl, "%% %s : %s\n", readable_name_of_ddKind(DD_MODULE),
	    xmod.get_name());
    symbolArr modified_objects;
    symbolArr tmp_arr;
    tmp_arr.insert_last(xmod);
    get_modified_objects(tmp_arr, modified_objects);
    symbolArr affected_files;
    symbolPtr sym;
    ForEachS(sym, modified_objects)
    {
        genString line;
	ddKind k = sym.get_kind();
	const char* kind_str;
	if (dd_is_not_internal_object(k))
	{
	    switch(k)
	    {
	      case DD_SOFT_ASSOC:
		kind_str = TXT("ASSOCIATION");
		break;
	      case DD_LINKSPEC:
		kind_str = TXT("LINK TYPE");
		break;
	      default:
		kind_str = readable_name_of_ddKind(k);
		break;
	    }
	}
	else
            kind_str = TXT("Internal Object");
     
        OSapi_fprintf(fl, "@ %s : %s\n", kind_str, sym.get_name());
        sym->get_link(ref_file, affected_files);
    }
    OSapi_fprintf(fl, "Files affected :\n");
    symbolPtr sym1;
    symbolArr reported;
    ForEachS(sym1, affected_files)
    {
        symbolPtr xsym = sym1.get_xrefSymbol();
        if (!reported.includes(xsym))
        {
            projNode *node = projHeader::ln_to_projNode(sym1.get_name());
            genString fn;
            if (node)
                node->ln_to_fn_imp(sym1.get_name(), fn, FILE_TYPE, 0, 1);
            else
                fn = sym1.get_name();
            OSapi_fprintf(fl, "# %s\n", (char *)fn);
            reported.insert_last(xsym);
        }
    }
    OSapi_fprintf(fl, "---\n");
}

char *create_one_tmp_file(char *msg)
// craete a temporary file name that contains msg
{
    Initialize(create_one_tmp_file);

    genTmpfile *tnm = 0;
    static int count = 0;
    char buf[6];
    OSapi_sprintf(buf,"m%04d",count++);
    if (tnm)
        delete tnm;
    tnm = db_new(genTmpfile,(buf));
    FILE *out = tnm->open_buffered();
    if (msg)
        OSapi_fprintf(out,"%s\n",msg);
    tnm->close();
    return (char *)tnm->name();
}

void put_psets(objArr& modules);

void putUI::do_fast_put(symbolArr& symbols, genString& bugtext, genString& text)
{
    Initialize(putUI::do_fast_put);

    objArr successfuly_put;
    Obj *el;
    ForEachS (el,symbols)
    {
        projModule* module = checked_cast(projModule,el);

        genString fn;
        module->get_target_project(0)->ln_to_fn(module->get_name(), fn);
        if (fn.length() == 0)
            continue;

        char *p = strrchr(fn,'/');
        int dir_exist = 0;
        if (p)
        {
            *p = '\0';
            struct stat buf;
            // if it is not a directory
            if (global_cache.stat(fn, &buf) == 0 && S_ISDIR(buf.st_mode))
                dir_exist = 1;
        }
        if (!dir_exist)
            continue;
	int success = module->cfg_fast_put(module->get_target_project(0), "", text);
	if(success)
	    successfuly_put.insert_last(module);
    }
    int i;
    for(i = 0; i < successfuly_put.size(); i++){
	projModule* module = checked_cast(projModule, successfuly_put[i]);
	symbolArr tmp;
	tmp.insert_last(module);
	call_cli_callback(NULL, "discover_put_module_report", "fast_put \"\" \"\"", &tmp, NULL);
/*
	genString src_file;
	genString src_paraset_file;
	genString dst_file;
	genString dst_paraset_file;
	module->get_file_names_internal (src_file, dst_file,
					 src_paraset_file, dst_paraset_file);
*/
    }
    // Create script to put pset files
    put_psets(successfuly_put);
    for(i = 0; i < successfuly_put.size(); i++){
	projModule* module = checked_cast(projModule,successfuly_put[i]);
	module->update_local_rtl_pmod();
	void* dummy = NULL;
	view_flush_rtls(dummy);
    }
}

int execute_put_start_script(symbolArr& symbols, genString& bugtext, genString text)
{
    Initialize(execute_put_start_script);

    //*********************
    // prepare a file that contains bugno
    //*********************
    genString tnm_for_bug = create_one_tmp_file(bugtext);

    //*********************
    // prepare a file that contains comment
    //*********************
    genString tnm_for_comment = create_one_tmp_file(text);

    //*********************
    // prepare a file that contsins all file names for put
    //*********************
    genTmpfile tnm_for_files("putui");
    FILE *out_file = tnm_for_files.open_buffered();
    projModule *first_mod = 0;
    Obj* nel;
    ForEachS (nel,symbols)
    {
        projModule* module = checked_cast(projModule,nel);

        genString fn;
        module->get_target_project(0)->ln_to_fn(module->get_name(), fn);
        if (fn.length() == 0)
        {
            gtPushButton::next_help_context("Browser.Manage.Put.BadPutProject");
            msg("ERROR: This file \"$1\" could not be checked into this target project \"$2\"") << module->get_name() << eoarg << module->get_target_project(0)->get_ln() << eom;
            continue;
        }

        char *p = strrchr(fn,'/');
        int dir_exist = 0;
        if (p)
        {
            *p = '\0';
            struct stat buf;
            // if it is not a directory
            if (global_cache.stat(fn, &buf) == 0 && S_ISDIR(buf.st_mode))
                dir_exist = 1;
            *p = '/';
        }
        if (!dir_exist)
        {
            gtPushButton::next_help_context("Browser.Manage.Put.BadPutProject");
            msg("ERROR: The directory for file \"$1\" does not exist in target project \"$2\"") << module->get_name() << eoarg << module->get_target_project(0)->get_ln() << eom;
            continue;
        }
        if (!first_mod)
            first_mod = module;
        module->get_project_internal()->ln_to_fn(module->get_name(), fn);
        OSapi_fprintf(out_file,"%s\n",fn.str());
        report_modified_objects(out_file, module);
    }
    tnm_for_files.close();

    int success = 0;
    if (first_mod)
        success = first_mod->put_start(tnm_for_bug, tnm_for_comment, tnm_for_files.name());

    OSapi_unlink(tnm_for_bug);
    OSapi_unlink(tnm_for_comment);
    tnm_for_files.unlink();
    return success;
}

int cli_call_put_start(symbolArr& modules, genString& bugtext, genString text);
void putUI::do_put(symbolArr symbols)
// the main routine to send all files to the monitor queue
//
// all files are saved in symbols
{
    Initialize (putUI::do_put);

    genString text = "_empty_";
    genString bugtext = "";

    if ( symbols.size() == 0 )
    {
       gtPushButton::next_help_context("Browser.Manage.Put.BadPutStart");
       msg("ERROR: $1") << "The Put operation has failed due to an empty put list" << eom;
       return;
    }

    int success = cli_call_put_start(symbols, bugtext, text);

    /* For compatibility reasons, we have to regard scripts that
     * return a number greater than 0 as successful. But, we still
     * have all the negative numbers at our disposal (blech).
     */
    if (success <= 0)
    {
        gtPushButton::next_help_context("Browser.Manage.Put.BadPutStart");
        if (success == 0) {
          msg("ERROR: $1") << "The submission monitor has rejected your file(s)." << eom;
        }
        return;
    }

    char first = 1;
    Obj* el;

    do_fast_put(symbols, bugtext, text);
    return;
}
