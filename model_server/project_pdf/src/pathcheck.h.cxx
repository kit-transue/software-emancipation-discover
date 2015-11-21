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
#include <stdio.h>
#else /* ISO_CPP_HEADERS */
#include <cstdio>
namespace std {};
using namespace std;
#endif /* ISO_CPP_HEADERS */
#include <general.h>
#include <objDb.h>
#include <pathcheck.h>
#include <proj.h>
#include <Hash.h>
#include <cLibraryFunctions.h>
#include <msg.h>
#include <machdep.h>
#include <fileEntry.h>

enum HIDE_RULE {
    UNINITIALIZED = -1,
    ROOT,
    TREE,
    PROJECT
};

/******
  o Improved "Hide File" behavior in case of identical physical file names 
    for more than one source file in a project. Specifically, introduced
    new environment variable PSET_PDF_HIDE to define/control the PDF 
    behavior in case of identical physical file names. 
    The following describes the allowable values (root is default):
	root  - first physical file name hides all identical names
		under the same root project.
	tree  - first physical file name hides all identical names
		on the same project branch. In this case the identical
		files in  sibling subprojects do not hide each other.
      project - first physical file name hides all identical names
		in the same subproject only.
******/
static enum HIDE_RULE get_hide_rule()
{
    Initialize(get_hide_rule);
    static enum HIDE_RULE hide_rule = UNINITIALIZED;
    static char const *pdf_hide = OSapi_getenv ("PSET_PDF_HIDE");

    if (hide_rule == UNINITIALIZED) {
	if (pdf_hide && strcmp (pdf_hide, "project") == 0)
	    hide_rule = PROJECT;
	else if (pdf_hide && strcmp(pdf_hide, "tree") == 0)
	    hide_rule = TREE;
	else if (pdf_hide && strcmp(pdf_hide, "root") != 0)
	    hide_rule = ROOT;
	else {
	    hide_rule = ROOT;
	    if (pdf_hide && pdf_hide[0])
		msg("PDF Error: env PSET_PDF_HIDE= $1. Allowed: root, tree, project. Using root.") << pdf_hide << eom;
	    ;
	}
    }
    return hide_rule;
}

//
//  === implicit name projModule Hash ===
//
class imModuleHash : public nameHash
{
   public:
    virtual const char* name(const Object*) const;
};

const char *imModuleHash::name(const Object *m) const
{
    Initialize(imModuleHash::name);
    const char *nm = NULL;
    if (m) {
	projModule *mod = (projModule *)m;
	nm = mod->get_implicit_name();
    }
    return nm;
}


//
//  === physical name projModule Hash ===
//
class phModuleHash : public nameHash
{
   public:
    virtual const char* name(const Object*) const;
};

const char *phModuleHash::name(const Object *m) const
{
    Initialize(phModuleHash::name);
    const char *nm = NULL;
    if (m) {
	projModule *mod = (projModule *)m;
	nm = mod->get_phys_filename();
    }
    return nm;
}


//
//  === implicit name projNode Hash ===
//
class imProjHash : public nameHash
{
   public:
    virtual const char* name(const Object*) const;
};

const char *imProjHash::name(const Object *p) const
{
    Initialize(imProjHash::name);
    const char *nm = NULL;
    if (p) {
        projNode *pn = (projNode *)p;
        nm = pn->get_name();
    }
    return nm;
}

//
//  === physical name projNode Hash ===
//
class phProjHash : public nameHash
{
   public:
    virtual const char* name(const Object*) const;
};

//
//  Hash initialization
//
phModuleHash ph_mod_hash;
imModuleHash im_mod_hash;
imProjHash   im_proj_hash;

// Returns: 1 on success
//          0 on failure
//         -1 wrong parameters
int add_module_to_hashes (projModule *mod)
{
    Initialize(add_module_to_hashes__projModule*);
    int ret = -1;
    if (mod) {
	ret = (ph_mod_hash.add (*mod)) ? 1 : 0;
	if (ret && mod->get_implicit_name())
	    ret = (im_mod_hash.add (*mod)) ? 1 : 0;
    }
    return ret;
}

// Returns: 1 on success
//          0 on failure
//         -1 wrong parameters
int remove_module_from_hashes (projModule *mod)
{
    Initialize(remove_module_from_hashes__projModule*);
    int ret = -1;
    if (mod) {
	ret = (ph_mod_hash.remove(*mod)) ? 1 : 0;
	if (ret && mod->get_implicit_name())
	    ret = (im_mod_hash.remove(*mod)) ? 1 : 0;
    }
    return ret;
}


// Returns: 1 on success
//          0 on failure
//         -1 wrong parameters
int remove_projNode_from_hashes (projNode *pn)
{
    Initialize(remove_projNode_from_hashes__projNode*);
    int ret = -1;
    if (pn) {
	Object *im_obj = im_proj_hash.remove(*pn);
	ret = ( im_obj ) ? 1 : 0 ;
    }
    return ret;
}

extern char* create_path_2DIS_no_realpath( char * path, char** dis_path );

// Search an existing projModule suitable for the specified project
// If the Project is not specified returns first found projModule
projModule *find_module_by_physname (const char *phys_name, projNode *pn, int& idx, projNode *&mod_pn)
{
    Initialize(find_module_by_physname__char*_projNode*_int&_projNode*&);
    idx = -1;
    mod_pn = NULL;
    projModule *the_mod = NULL;
    if (phys_name && *phys_name) {
	char *cvt_ptr = 0;
	char *buf =  create_path_2DIS_no_realpath( (char *)phys_name, &cvt_ptr );
	Object *found = NULL;
	for (int res = ph_mod_hash.find (buf, idx, found) ; res ;
	     res = ph_mod_hash.next (buf, idx, found) )
	 {
	     projModule *mod = (projModule *)found;
	     if (!mod)
		 continue;

	     projNode *found_pn = mod->get_project_internal ();
	     int is_found = 0;
	     if (!pn)
		 is_found = 1;
	     else {
		 if (pn == found_pn)
		     is_found = 1;
		 else {
                     // ROOT    - DEFAULT. Root project first phys_name hides all the others
                     // TREE    - First phys_name in a  project hides all the others on the same branch
                     // PROJECT - First phys name in a project hides all others on the same level
		     switch (get_hide_rule()) {
		     case PROJECT:
			 is_found = 0;
			 break;
		     case TREE:
			 if (found_pn->is_younger(pn) || pn->is_younger(found_pn))
			     is_found = 1;
			 break;
		     case ROOT:
		     default:
			 if (pn->root_project() == found_pn->root_project())
			     is_found = 1;
			 break;
		     }
		 }
	     }

	     if (is_found) {
		 the_mod = mod;
		 mod_pn = found_pn;
		 break;
	     }
	 }
	if (cvt_ptr) free (cvt_ptr);
    }
    return the_mod;
}

// Search an existing projModule by implicit name 
projModule *find_module_by_impname (const char *imp_name)
{
    Initialize(find_module_by_impname);
    int idx = -1;
    projModule *m = 0;
    if (imp_name && *imp_name) {
	Object *found = NULL;
	if ( im_mod_hash.find (imp_name, idx, found) )
	    m = (projModule*)found;
    }
    return m;
}

// Search an existing projNode suitable for the specified parent project
// If the parent project is not specified returns first found projNode or NULL
projNode *find_projNode_by_impname (const char *imp_name, int& idx, projNode *&found_parent_pn)
{
    Initialize(find_projNode_by_physname__char*_int&_projNode*&);
    idx              = -1;
    found_parent_pn  = NULL;
    projNode *the_pn = NULL;

    if (imp_name && *imp_name) {
	Object *found = NULL;
	for (int res = im_proj_hash.find (imp_name, idx, found) ; res ;
	     res = im_proj_hash.next (imp_name, idx, found) )
	 {
	     the_pn = (projNode *)found;
	     if (!the_pn) continue;
	     found_parent_pn = the_pn->find_parent ();
	     break;
	 }
    }
    return the_pn;
}


void init_module_check_tree()
{
    Initialize(init_module_check_tree);
    IF(0);
}
