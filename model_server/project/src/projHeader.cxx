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
#include <machdep.h>
#include <proj.h>
#include <pdf_tree.h>

#include <xref.h>
#include <db.h>
#include <path.h>
#include <customize.h>
#include <fileCache.h>

int projHeader::is_writable(char const *ln)
{
    Initialize(projHeader::is_writable);
    
    if (!ln) return 0;
    
    genString fn;
    projNode* project;
    int i;
    
    for (i=1, project = projList::search_list->get_proj(i);
	 project;
         project = projList::search_list->get_proj(++i)) {
	
	if (project->is_writable() &&
		project->get_xref() && project->get_xref()->is_writable()) {
	    project->ln_to_fn(ln, fn);
	    if (fn.length()) {
	        struct OStype_stat stat_buf;
	        if (fileCache_stat(fn, &stat_buf) == 0)
	            return 1;
            }
	}
    }
    return 0;
}


projNode* projHeader::fn_to_project (char const* fn, int is_prog)
{
    Initialize(projHeader::fn_to_project);

    projNode* proj = NULL;

    genString ln;
    fn_to_ln(fn, ln, &proj, is_prog, 1);

    return proj;
}

void projHeader::fn_to_ln (char const* fn, genString &ln, projNode ** pn, int is_prog)
{
    Initialize(projHeader::fn_to_ln);

    fn_to_ln(fn, ln, pn, is_prog, 0);
}

void projHeader::fn_to_ln (char const* fn, genString &ln, projNode ** pn, int is_prog, int no_hash)
{
    fn_to_ln(fn, ln, pn, is_prog, no_hash, (genString *)NULL);
}

void projHeader::fn_to_ln (char const* fn, genString &ln, projNode ** pn,
			   int /* is_prog */ , int /* no_hash */, genString *canonic_file_name )
// convert physical name to logical. If pn!=0, also return
//     the particular root project that this name occurs in
//
// fn: physical file name
// ln: return logic file name
// pn: if not zero, return root project for this file
{
    Initialize(projHeader::fn_to_ln);

    if (!projList::search_list)
        return;

    if (!fn || *fn == '\0')
        return;

    // boris: use input parameter to return converted file name
    genString *filename;
    genString *local_filename = NULL;

    if (canonic_file_name) 
	filename = canonic_file_name;
    else {
	local_filename = new genString;
	filename = local_filename;
    }

    project_convert_filename(fn, *filename);
    if (customize::getIntPref("DIS_internal.PDFCheck_Enabled") > 0) {
        if ((*filename).length())
            msg("project_convert_filename ($1) -> ($2)") << (char *)fn << eoarg << (char *)(*filename).str() << eom;
	else
            msg("project_convert_filename ($1) -> ($2)") << (char *)fn << eoarg << "(null)" << eom;

        ;
    }

    pdf_tree_fn_to_ln((char const *)(*filename), ln, pn);

    if (local_filename)
	delete local_filename;
}

void projHeader::fn_to_ln_special_rule(char const* fn, genString &ln, projNode *pn)
{
    Initialize(projHeader::fn_to_ln_special_rule);

    ln = 0;
    char const *p = strstr(fn,"/%/");
    if (!p)
        return;

    int type = OTHER_RULE;
    if (strcmp(p+3,".make") == 0)
        type = MAKE_RULE;
    else if (strcmp(p+3,".makemf") == 0)
        type = MAKEMF_RULE;
    else if (strcmp(p+3,".pmoddir") == 0)
        type = PMODDIR_RULE;
    else if (strcmp(p+3,".pset") == 0)
        type = PSET_RULE;
    projHeader::fn_to_ln_special_rule(fn, ln, type, pn);
}

void projHeader::fn_to_ln_special_rule(char const* fn, genString &ln,
				       int type, projNode * &proj)
{
  Initialize(projHeader::fn_to_ln_special_rule);

  if(proj == NULL){
      genString name = fn;
      char *p        = strchr((char *)name, '%');
      if(p)
	  *(p - 1) = 0;
      proj = projHeader::fn_to_project(name.str());
  }
  ln = 0;

  genString filename;

  project_convert_filename((char *) fn, filename);
  
  if(proj){
      projNode * cur = proj;
      while (cur && !cur->get_map())
	  cur = cur->find_parent();
      if(cur)
	  proj = cur;
      if (proj->get_map() && proj->get_map()->is_para_rule_type(type))
      {
	  if (type == PMODDIR_RULE)
	      proj->fn_to_ln(filename, ln, DIR_TYPE, 0, 1);
	  else
	      proj->fn_to_ln(filename, ln, FILE_TYPE, 0, 1);
	  if (ln.length())
	      return;
      }
  }

  if (!projList::search_list)
    return;

  for (int i = 1; proj = projList::search_list->get_proj(i); i++)
  {
    if (!proj->is_visible_proj() && proj->get_map() && proj->get_map()->is_para_rule_type(type))
    {
	if (type == PMODDIR_RULE)
	    proj->fn_to_ln(filename, ln, DIR_TYPE, 0, 1);
	else
	    proj->fn_to_ln(filename, ln, FILE_TYPE, 0, 1);
	if (ln.length())
	{
	  if ( type != MAKE_RULE )
	  {
	    char* ln_buf = 0;
	    genString ln_internal;
	    ln= create_path_2DIS ( (char*)ln.str(), &ln_buf );
	    if ( ln_buf )
	      free( ln_buf );
	  }
          return;
	}
    }
  }
  proj = 0;
}
void projHeader::ln_to_fn (char const* ln, genString &fn)
{
    Initialize(projHeader::ln_to_fn);

    if (!projList::search_list)
	return;
    
    projNode* proj, *home;
    genString filename;

    ForEachProj(i,proj){
//    for (int i=0; proj = projList::search_list->get_proj(i); i++) {
        if (i==0)
            home = proj;
        else if (home == proj)
            continue;
        proj->ln_to_fn (ln, fn);
        if (fn.length())
            return;
    }

    fn = NULL;
}

projNode *projHeader::ln_to_projNode(char const* ln)
// ln is a project name.
//
// Search search_list to find the projNode for this project name.
//
// When it is the second process for "put", the home is not set yet.
// So it starts from 1 instaed of 0.
//
// PS: Start form 0 (home) is to speedup because home is always the target projnode.
{
    Initialize(projHeader::ln_to_projNode);

    if (!ln) return NULL;

    projNode* proj;
    int start = 0;
    if(projList::search_list->get_proj(0) == NULL)
      start = 1;
    ForEachProj(i,proj){
        if(i<start)
	  continue;
        genString fn;
        proj->ln_to_fn_imp ((char*)ln, fn, DIR_TYPE);

        if (fn.length())
            return proj;

        proj->ln_to_fn_imp ((char*)ln, fn, FILE_TYPE);

        if (fn.length())
            return proj;
    }
    return NULL;
}

projNode* projHeader::non_current_project (char const* ln)
{
    Initialize (projHeader::non_current_project);

    if (!ln) return NULL;

    projNode* proj;
    projNode* cur_proj = projNode::get_current_proj();

    while (proj = childProject_get_parentProject(cur_proj))
	cur_proj = proj;

    for (int i=1; proj = projList::search_list->get_proj(i); i++) {

	if (proj == cur_proj ||
	    proj == projNode::get_control_project())
	    continue;

        genString fn;
        proj->ln_to_fn( (char *)ln, fn);
	if (fn.length())
	    return proj;
    }
    return NULL;
}


int projHeader::make_path (char const *path)
// Make a complete path for the given file     /root/dir1/dir2/file
//   or directory    /root/dir1/dir2/
// This function is equivalent in intent to mkdir -p
// It is called with a complete path for a file name,
//   and it makes sure all nodes but the last exist, creating
//   any directories that do not.
// It may also be called with just a directory string, but then
// there must be a trailing slash
// It looks like it will not give an error if the last
//   directory level exists, but is actually a file.
{
    Initialize(projectHeader::make_path);
 
    char pathname[MAXPATHLEN];
    char *ptr1;
    char *ptr2;
    struct OStype_stat stat_buf;
 
    strcpy (pathname, path);
 
    for (ptr1 = pathname+1, ptr2 = strchr(ptr1, '/');
         ptr2; ptr1 = ptr2+1, ptr2 = strchr(ptr1, '/')) {
        if (ptr2) {
            *ptr2 = 0;
 
            if (OSapi_stat(pathname, &stat_buf) != 0) {
                if (OSapi_mkdir(pathname, 0777) != 0) {
		    genString msg ="Error trying to create directory ";
		    msg += pathname;
		    OSapi_perror(msg);
                    ReturnValue(-1);
		}
            }
            *ptr2 = '/';
        }
    }
 
    ReturnValue(0);
}

int project_name_to_fn(char const *proj_name, genString &phys_name)
{
  Initialize(project_name_to_fn);
  projNode * pn = projHeader::ln_to_projNode(proj_name);
  if (pn) {
    pn->ln_to_fn_imp(proj_name, phys_name, DIR_TYPE);
    return phys_name.length();
  }
  return 0;
}

int projHeader::construct_symbolPtr(symbolPtr & sym) const
{
    Initialize(projHeader::construct_symbolPtr);

    int retval = 0;

    projNode *pr = checked_cast(projNode,get_root());
    sym = pr->lookup_symbol (DD_PROJECT, pr->get_name());
    if (sym.xrisnull()) {
	projNode *cont = projNode::get_control_project ();
	if (cont)
	    sym = cont->lookup_symbol (DD_PROJECT, pr->get_name());
    }

    if (sym.xrisnotnull())
	retval = 1;

    return retval;
}

projModule* projHeader::find_module (char const* ln)
{
    Initialize(projHeader::find_module);

    if (!ln) return NULL;

    genString real_ln;
    projNode* project = projNode::get_current_proj();
    if (project) {
        project->fn_to_ln (ln, real_ln);
	if (real_ln.length())
	    ln = real_ln;
    }

    projModule *module = NULL;
    projNode * home_project = projNode::get_home_proj();
    if (home_project) {
      module = home_project->find_module(ln);
      if (module)
	return module;
    }
    project = projNode::get_current_proj();    
    if (project && (project != home_project)) {
      module = project->find_module(ln);
      if (module)
	return module;
    }

    // do it the hard way if xref doesn't exist.
    projNode *proj;
    ForEachProj(i,proj){
//    for (int i=0; proj = projList::search_list->get_proj(i); i++) {
      if ((proj != home_project) && (module = proj->find_module(ln)))
	break;
    }

    return module;
}

void proj_create_ctl_pm()
// create pdf definition for the control project
{
    Initialize(proj_create_ctl_pm);

    projNode *ctp = projNode::get_control_project();

    if (ctp->get_map())
        return;

    genString projn = ctp->get_ln() + 1;

    genString tnm = OSapi_getenv("TMPDIR");
    if (tnm.is_null())
	tnm = "/usr/tmp";

//    projMap *pmc = db_new(projMap,(MEMBER,NO_IMPLICIT_DIR_PROJ,TWO_WAY,0,"**",0,SINGLE_CHILD,0));
    projMap *pm = db_new(projMap,(PROJECT,IMPLICIT_DIR_PROJ,TWO_WAY,projn,tnm,"/tmp",SINGLE_CHILD,0));
//    pm->set_child_project(pmc);
    pm->set_file_type(DIR_TYPE | LINK_DIR);
    ctp->set_map(pm);
    re_generate_pdf_tree_for_ctl_proj(ctp, pm, tnm);
}
