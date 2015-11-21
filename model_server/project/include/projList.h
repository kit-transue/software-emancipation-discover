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
#ifndef _projList_h
#define _projList_h
//
// projList.h
//

/****************************************************************
 * synopsis:
 * 
 * This class maintains three lists of projects, of two different kinds
 *   One list is the complete list of projects in the system - full_list
 *
 *   A second is the list of visible projects in the system - search_list
 *      (up till 7/6/93, these were combined)
 *
 *   The third is the search list for column 2 of scan mode, called
 *      the domain presumably there ought to be one such list for each
 *      browser, but there is not.  There are some static functions
 *      that hardwire looking in search_list for the domain_array.
 *
 * There are two static member variables, full_list and search_list,
 *   and these are used with functions get_proj, add_proj, rem_proj to
 *   add to the appropriate list.  Also, get_search_list and
 *   set_search_list are used to modify the search_list when the user
 *   hides projects use Manage->ProjectScope.
 *
 * The write_proj is now a static variable
 *
 * The search_list is updated via a dialog box.  When the user presses
 *   the OK button, the dialog logic will call
 *   set_search_list(symbolArr&)
 *
 *
 * domain list:
 *
 * The list of projects is stored in the domain_array. The pathlist is
 * misleading and does not contain all the names of the domain array
 * objects (bug ?).
 *
 * For practical purposes it seems that pathlist and domain_array are
 *   only used from one instance: the instance search_list
 *
 * NOTE, that the write_proj is not necessarily an element on the
 *       domain_array, but still set correctly in the full_list and
 *       search_list instances.
 *
 * The static search_list field of projList should be used for all actions.
 * For example, to get all the projects on the domain list use 
 *              projList::search_list->get_scan(i)
 *    which simply calls the static function    projList::domain(i)
 * in the loop, while it is != 0  ( i < ::search_list->domain_array.size()
 * but it is private ).
 *
 ****************************************************************/

#ifndef _objArr_h
#include <objArr.h>
#endif

#ifndef _RTL_h
#include <RTL.h>
#endif

#ifndef _proj_h
#include <proj.h>
#endif


class projList : public RTL
{
  public:
    // Do not use search_list directly (it is only here for backwards
    // compatibility).  Use one of the following domain_* functions instead.
    static projList* search_list;
    static projList* full_list;

    static void		domain_reset();
    static void		domain_add(const char*);
    static void		domain_first(projNode*);
    static void		domain_remove(projNode*);
    static projNode*	domain(int);
    void                domain_string();
    void	scan_project_first_rm(projNode*);
    void	scan_project_first(projNode*);
    void	scan_projects(const char*);
    projNode*	get_scan(int index); 

    projList();
    ~projList();

    void	writable(projNode*);
    projNode*	writable();


    projNode*	get_proj(int index);	// First writable, then readables
    void	add_proj(projNode*);	// Add to readable list
    void	rem_proj(projNode*);	// Remove from readable list

    static void rem_proj_from_all_lists(projNode *pn);
    static void add_proj_for_all_lists(projNode *pn, projNode *prev);

//   pass new set of projNodes for a search list (presumably for search_list)
// every one of these had better be part of the full_list
    // 0 is returned if all is well
    int         set_search_list(const symbolArr &);
    int         get_search_list(symbolArr &);
    
//  copy the supplied list into *this
    void        copy(const projList&);

    void        parse_names_init(const char* input);
    const char* parse_names_next(symbolPtr* psym = 0);

  private:
    static projNode* write_proj;
    void *    dummy;                   // placeholder, to avoid recompile
    objArr    domain_array;
    genString pathlist;

    symbolArr proj_contents;
    int       proj_contents_next;

    void domain_add(projNode*);
};

#define ForEachProj(ii,pr) for( \
  int ii=projList::search_list->get_proj(0)?0:1; \
  (pr = projList::search_list->get_proj(ii)); ++ii)


#endif // _projList_h

