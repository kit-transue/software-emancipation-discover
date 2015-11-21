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
#include <msg.h>

#undef MAX
#include <systemMessages.h>
#include <genString.h>
#include <genTmpfile.h>
#include <customize.h>
#include <xref.h>

#include <xref_priv.h>
#include <machdep.h>
#include <shell_calls.h>
#include <driver_mode.h>

const int  customize_get_write_flag();
const char* customize_install_root();

extern const char *get_PDF();

extern int proj_count;
extern int source_file_count;
extern int total_file_count;
extern int pset_exists_count; 
extern int pset_up_to_date_count;
extern int entry_in_pmod_count;
extern int entry_in_pmod_uptodate_count ;


static void check_all_files(projNode *pn, projNode *projnode);

void pr_bad_proj_name(const char *projname)
{
    Initialize(pr_bad_proj_name);

    msg("Error: Project \"$1\" can not be found in the PDF \"$2\".\n") << projname << eoarg << get_PDF() << eom;
    msg("The cause might be one of the following two reasons:\n") << eom;
    msg("1. Project name \"$1\" is not defined in this PDF.\n") << projname << eom;
    msg("2. The root directory does not exist.\n") << eom;
}

void dd_get_stat(char* projname,char** visible_projects)
{

    Initialize(dd_get_stat);

    // add all root projects in the scan list
    projList::search_list->scan_projects("/");

    // get to project node from project name
    projNode *projnode = projNode::find_project (projname);
    if (!projnode) {
        pr_bad_proj_name(projname);
        return;
    }

    // set the projnode to be current project
    projNode::set_home_proj (projnode);

    // set visible projects
    if (visible_projects[0] != NULL) {
	symbolArr all_projs, visible_projs;
        projList::search_list->get_search_list(all_projs);
	for (int ii = 0; visible_projects[ii]; ++ii) {
	    symbolPtr s;
	    {
		ForEachS(s, all_projs) {
		    char const *name = s.get_name ();
		    if (name && !strcmp(name, visible_projects[ii])) {
			visible_projs.insert_last(s);
			break;
		    }
		}
	    }
	}
        projList::search_list->set_search_list(visible_projs);
    }

    // get the xref from project node
    Xref *xref = projnode->get_xref();
    if (!xref || !xref->is_writable()) {
	//  either no xref, or we don't have write-access to it

        // suppress the message if the user is in -batch, doing things like
	//  -checkpmod, -p, etc.
	if (customize_get_write_flag()) {
	    msg("Error: Could not write to pmod files for project $1\n") << projname << eom;
	    return;
	}
    }

    // Make sure that we scan this project first
    projList::search_list->scan_project_first (projnode);
     
    
    check_all_files(projnode, projnode);
    msg("Total number of projects in pdf = $1\n") << proj_count << eom;
    msg("Total number of files in project = $1\n") << total_file_count << eom;
    msg("Total number of source files in project = $1\n") << source_file_count << eom;
    msg("Number of files with psets = $1\n") << pset_exists_count << eom;
    msg("Number of files with up to date psets = $1\n") << pset_up_to_date_count << eom;
    msg("Number of files with entries in pmod = $1\n") << entry_in_pmod_count << eom;
    msg("Number of files with up to date entries in pmod = $1\n\n\n") << entry_in_pmod_uptodate_count << eom;
}

static int file_size(char* name)
{
    Initialize(file_size);
    int handle = OSapi_open(name, O_RDONLY);
    if (handle<0) return -1;
#ifndef SEEK_END
#define SEEK_END 2
#endif
    off_t size = OSapi_lseek(handle, 0L, SEEK_END);
    OSapi_close(handle);
    return size;
}


static void check_all_files(projNode *pn, projNode *projnode)
{
    Initialize(check_all_files);
    
    proj_count++;
    
    if (is_projNode(projnode))
        projnode->refresh();
    
    symbolArr& proj_contents = projnode->rtl_contents();
    symbolPtr sym;
    
    ForEachS(sym, proj_contents)
    {
        char const *qq = sym.get_name();
        if (sym->get_kind() == DD_MODULE)
        {   
            total_file_count++;
            projModule *proj_module = checked_cast(projModule,sym);
	    if(proj_module->language() != FILE_LANGUAGE_UNKNOWN)
            { 
		source_file_count++;
		char const *path = proj_module->get_name();
		genString full_path;
		proj_module->get_phys_filename(full_path);
		
		int pset_exists = proj_module->is_paraset_file();
		int pset_up_to_date = 0;
		if(pset_exists)
		    pset_up_to_date = (proj_module->outdated_pset()) ? 0 : 1;  
		pset_exists_count +=pset_exists;
		pset_up_to_date_count +=pset_up_to_date;
		fsymbolPtr entry = projNode::lookup_xrefSymbol_in_cur_proj(DD_MODULE,path);
		int entry_in_pmod = 0;
		if((entry->xrisnotnull()))
		    entry_in_pmod = 1;
		int entry_in_pmod_uptodate;
		if(entry_in_pmod)
		    if(Xref_file_is_newer(path,(char *)full_path)->xrisnull())
			entry_in_pmod_uptodate = 0;
		    else
			entry_in_pmod_uptodate = 1;
		else
		    entry_in_pmod_uptodate = 0;
		entry_in_pmod_count += entry_in_pmod;
		entry_in_pmod_uptodate_count += entry_in_pmod_uptodate;
		
		msg(" Filename:                  $1\n") << (char *)full_path << eom;
		msg(" Source lines:              ") << eom;

		genString exe = customize_install_root();
		exe += "/bin/paratotal";
		int tmp_stat = vsysteml (exe, (const char *)full_path, NULL);

		if (pset_exists)
		{
		    genString com_str =  full_path;
		    com_str += ".pset";      
		    msg(" .pset size:                $1\n") << file_size(com_str) << eom;
		}
		else
		    msg(" .pset size:                .pset does not exist\n") << eom;
		if (pset_exists)
		{
		    if (pset_up_to_date)
			msg(" .pset up to date:          yes\n") << eom;
		    else
			msg(" .pset up to date:          no\n") << eom;
		}
		if (entry_in_pmod)
		{
		    msg(" File has entry in .pmod:   yes\n") << eom;
		    if (entry_in_pmod_uptodate)
			msg(" Entry in .pmod up to date: yes\n") << eom;
		    else
			msg(" Entry in .pmod up to date: no\n") << eom;
		}
		msg("") << eom;
	    }
	}
	else if (sym->get_kind() == DD_PROJECT)
	{
	    appPtr app_head = sym.get_def_app();
	    if(app_head && is_projHeader(app_head))
	    {
		projNode *child_proj = projNodePtr(app_head->get_root());
		check_all_files(pn, child_proj);
	    }
	}
    }
}



extern "C" void dd_print_xref (char *projname)
{
    Initialize (dd_print_xref);

    if (!projname)  {
        pr_bad_proj_name(projname);
        return;
    }

    // get to project node from project name
    projNode *projnode = projNode::find_project (projname);
    if (!projnode) return;
    projNode::set_home_proj (projnode);

    // get the xref from project node
    Xref *xref = projnode->get_xref();
    if (!xref) {
	msg("Error: Could not open pmod for project $1\n") << projname << eom;
	return;
    }

//    projList::search_list->scan_project_first (projnode);
//    print_XREF ();
      print_XREF(xref);
}

void dd_merge_xref (int count, char const **src_projs, char const *projname)
{
    Initialize (dd_merge_xref);

    if (!projname)  {
        pr_bad_proj_name(projname);
        return;
    }

    // get to project node from project name
    projNode *projnode = projNode::find_project (projname);
    if (!projnode) return;
    projNode::set_home_proj (projnode);

    // get the xref from project node
    Xref *xref = projnode->get_xref();
    if (!xref) {
	msg("Error: Could not open cross reference file for project $1\n") << projname << eom;
	return;
    }

    projList::search_list->scan_project_first (projnode);
    int did = 0;
    for (int i = 0; i < count; i++) {
	projNode *src_proj = projNode::find_project (src_projs[i]);

	if (!src_proj) {
	    msg("Error: The project $1 not found in project definition file\n") << src_projs[i] << eom;
	    continue;
	}

	if (!src_proj->get_xref(1)) {
	    msg("Error: The project $1 does not contain cross refrence\n") << src_projs[i] << eom;
	    continue;
	}

	Xref *src_xref = src_proj->get_xref();
	src_xref->merge (xref);
	did = 1;
    }
    if (did)
      xref->save_lxref_by_whole();

}

void dd_tidy_xref (char const *projname)
{
    Initialize (dd_tidy_xref);

    if (!projname) return;

    // get to project node from project name
    projNode *projnode = projNode::find_project (projname);
    if (!projnode) {
        pr_bad_proj_name(projname);
        return;
    }

    projNode::set_home_proj (projnode);

    // get the xref from project node
    Xref *xref = projnode->get_xref();
    if (!xref) {
	msg("Error: Could not open cross reference file for project $1\n") << projname << eom;
	return;
    }

    projList::search_list->scan_project_first (projnode);

    xref->tidy_xref ();
}
