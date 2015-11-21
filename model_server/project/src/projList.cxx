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
// projList.C
//------------------------------------------
// synopsis:
// 
// One-of class that maintains list of projects to search.
//------------------------------------------

// INCLUDE FILES
#include <cLibraryFunctions.h>
#include <msg.h>
#include <genError.h>
#include <projList.h>
#include <RTL_externs.h>
#ifndef ISO_CPP_HEADERS
#include <ctype.h>
#else /* ISO_CPP_HEADERS */
#include <cctype>
using namespace std;
#endif /* ISO_CPP_HEADERS */

#include <pdf_tree.h>
#include <transaction.h>

void cleanup_miniBrowsers();

// VARIABLE DEFINITIONS

projList* projList::search_list;   // Initialized in projNode::proj_init().
projList* projList::full_list;   //    ditto
projNode* projList::write_proj=NULL;

static char *next_path;

// DEBUG FUNCTION
extern "C" void psarr(symbolArr*);             // debug print contents of symbolArr 


// FUNCTION DEFINITIONS

//projList::projList() : RTL(0), write_proj(NULL)
projList::projList() : RTL(0)
{}


projList::~projList()
{}


void projList::domain_add(projNodePtr proj)          // NOT class static
{
    Initialize(projList::domain_add);

    if (proj) {
	if(proj->get_xref()) {
	    domain_array.insert_last(proj);
	} else {
	    proj->refresh();

	    symbolArr& proj_contents = proj->rtl_contents();
	    symbolPtr sym;
	    ForEachS(sym, proj_contents) {
		if (sym.isnotnull() && sym.relationalp()) {
		    Relational * rel = sym;
		    if (is_projModule(rel))
			continue;
		}
		appPtr app_head = sym.get_def_app();
		if(app_head && is_projHeader(app_head))
		    domain_add(checked_cast(projNode,app_head->get_root()));
	    }
	}
    }
}


void projList::domain_reset()			// class static
{
    Initialize(projList::domain_reset);

    search_list->domain_array.removeAll();
}


void projList::domain_add(const char* name)	// class static
{
    Initialize(projList::domain_add);

    if(!strcmp(name, "/"))
    {
       	RTLPtr rtl_head = projNode::get_project_rtl();
        if(rtl_head)
	{
	    RTLNodePtr rtl_node = checked_cast(RTLNode,rtl_head->get_root());
	    if(rtl_node)
	    {
	        Obj* el;
	        ForEachS(el,rtl_node->rtl_contents())
		{
		    Relational *rel = (Relational *)el;
		    //boris: The first case only shoud stay
		    if (is_projNode(rel))
			search_list->domain_add((projNode *)rel);
	        }
	    }
	}
    }
    else
    {
        projNodePtr proj_node = projNode::find_project(name);
        if(proj_node)
            search_list->domain_add(proj_node);
    }
}

void projList::domain_first(projNodePtr proj)	// class static
{
    Initialize(projList::domain_first);

    if(proj && proj->get_xref())
	search_list->domain_array.insert_first(proj);
}


projNodePtr projList::domain(int index)		// class static
{
    Initialize(projList::domain);

    if(index >= 0  &&  index < search_list->domain_array.size())
        return checked_cast(projNode,search_list->domain_array[index]);
    else
	return NULL;
}


void projList::scan_projects(const char* name)         // NOT class static
{
    domain_reset();       // static
    domain_add(name);     // static
}


void projList::scan_project_first(projNodePtr proj)    // NOT class static
{ domain_first(proj); }   // calls only a static function

projNodePtr projList::get_scan(int index)               // NOT class static
{ return domain(index); }      // calls only a static function


void projList::writable(projNodePtr proj)               // NOT class static
{
    Initialize(projList::writable);

    write_proj = proj;
}


projNodePtr projList::writable()                        // NOT class static
{
    Initialize(projList::writable);

    return write_proj;
}


projNodePtr projList::get_proj(int index)                // NOT class static
{
    Initialize(projList::get_proj);

    if (index == 0)
	 return write_proj;

    projNodePtr pr = NULL;

    symbolArr& read_projs = checked_cast(RTLNode,get_root())->rtl_contents();
    const int size = read_projs.size();

    if(index < 0)
    {
	for(int i = size - 1; i >= 0  &&  pr == NULL; --i)
	{
            projNodePtr proj = checked_cast(projNode,read_projs[i]);
	    if(proj->get_xref()&& proj->get_xref()->is_writable())
		pr = proj;
	}
    }
    else if(index <= size)
        pr = checked_cast(projNode,read_projs[size - index]);

    return pr;
}


void projList::add_proj(projNodePtr proj)                   // NOT class static
{
    Initialize(projList::add_proj);

    rtl_add_obj(checked_cast(RTLNode,get_root()), proj);
}


void projList::rem_proj_from_all_lists(projNode *pn)        // class static
{
    Initialize(projList::rem_proj_from_all_lists);

    search_list->rem_proj(pn);
    full_list->rem_proj(pn);
}

void projList::rem_proj(projNodePtr proj)                   // NOT class static
{
    Initialize(projList::rem_proj);

    rtl_delete_obj(checked_cast(RTLNode,get_root()), proj);
}

void  projList::copy(const projList& projlis)              // NOT class static
//  copy projects from the projlis into *this 
//    (only called once from main;  the purpose is to save
//     the original list)
{
    Initialize(projList::copy);
    symbolArr& read_projs = checked_cast(RTLNode, projlis.get_root())->rtl_contents();
    symbolArr& write_projs = checked_cast(RTLNode, this->get_root())->rtl_contents();
    write_projs = read_projs;
}

int  projList::set_search_list(const symbolArr &arr)        // NOT class static
//  called from new project_scope dialog box, this function
//     hides some projects and unhides others
//     Should only be called on the search_list
{
    Initialize(projList::set_search_list);
    symbolPtr sym;
    //  rtlnode is for search_list (which ought to be "this")
    RTLNodePtr rtlnode =  checked_cast(RTLNode, this->get_root());
    //  rtlnode2 is for the projectBrowsers, which keep their own version of search_list !!!
    RTLNodePtr rtlnode2 =  checked_cast(RTLNode, projNode::get_project_rtl()->get_root());

    symbolArr base_list;
    get_search_list(base_list);

    // first we must remove all symbols from the loaded pmods
    start_transaction() {
      ForEachS(sym,base_list) {
	if (!arr.includes(sym)) {
	    Xref::remove_all_symptrs_from_xrefTable(sym);
	    Xref::xref_notifier_report_proj(-1,sym);  // hide all symbols in any pmods of this proj
	    xref_notifier_apply();   // fake an "end_transaction" to flush more often, and especially
	                              // to flush before the rtl_delete_obj in the next loop
    }    }} end_transaction();
    
    {ForEachS(sym,base_list) {
	if (!arr.includes(sym)) {
	    rtl_delete_obj(rtlnode,sym);
	    rtl_delete_obj(rtlnode2,sym);
	}
    }}
    {ForEachS(sym, arr) {
	if (!base_list.includes(sym)) {
	    rtl_add_obj(rtlnode,sym);
	    rtl_add_obj(rtlnode2,sym);
	}
    }}

    re_generate_pdf_tree_for_hiding();
    cleanup_miniBrowsers();

    return 0;      // no error checking for now, so every return is a good one
}
#ifdef  _PSET_DEBUG
symbolArr  dbgList;
extern "C" int dbgGetList(int i)
{
    Initialize(dbgGetList);
    switch (i) {
      case 0:
          projList::search_list->get_search_list(dbgList);
          break;
      case 1:
          projList::full_list->get_search_list(dbgList);
          break;
      case 2:
	  RTLNodePtr rtlnode;
	  rtlnode =  checked_cast(RTLNode, projList::search_list->get_root());
	  dbgList = rtlnode->rtl_contents(); 
	  break;
      case 3:
	  rtlnode =  checked_cast(RTLNode, projList::full_list->get_root());
	  dbgList = rtlnode->rtl_contents(); 
	  break;
      }
    psarr(&dbgList);
    cerr << "dbgList is at " << (void*) &dbgList << endl;
    return dbgList.size();
}
extern "C" int dbgSetList(int i)
{
    switch (i) {
      case 0:
          projList::search_list->set_search_list(dbgList);
          break;
      case 1:
          projList::full_list->set_search_list(dbgList);
          break;
      }
    cerr << "dbgList is at " << (void*) &dbgList << endl;
    return dbgList.size();
}
extern "C" int dbgDelete(int i)
{
    dbgList.remove_index(i);
    psarr(&dbgList);
    return dbgList.size();
}
#endif

int  projList::get_search_list(symbolArr &arr)                // NOT class static
//  get the search list, not including the home project, control project, nor doc project
//    also skip anything that's not a projNode, to avoid getting
//    non-project stuff like "loaded files"
{
    Initialize(projList::get_search_list);
    RTLNodePtr rtlnode =  checked_cast(RTLNode, this->get_root());
    symbolArr& src = rtlnode->rtl_contents(); 

    arr = src;
    int sz = arr.size();
    for (int i=sz-1; i>=0; i--) {
	if (!is_projNode(arr[i]))
	    arr.remove_index(i);
    }
    arr.remove(projNode::get_control_project());
    arr.remove(projNode::get_home_proj_top_node());

    return 0;      // no error checking for now, so every return is a good one
}

int parse_selection(char const *, symbolArr& result);

//  The following have to be called as a matched pair to build a project list
//    from a space-delimited character string.  They use a static variable, so
//    they must not be called a second time till the first is complete.
void projList::parse_names_init(const char* input)             // NOT class static
{
  Initialize(projList::parse_names_init);

   proj_contents.removeAll();
   proj_contents_next = 0;
   if( strcmp(input, "/") )
     parse_selection( input, proj_contents );

  pathlist = input;
  next_path = pathlist;                 // next_path is file static !!!!
}

const char* projList::parse_names_next(symbolPtr* psym)                       // NOT class static
{
  Initialize(projList::parse_names_next);

  char const *path = 0;
  symbolPtr sym;

  if ( proj_contents.size() )
  {
    if ( proj_contents_next < proj_contents.size() )
    {
      sym = proj_contents[proj_contents_next++];
      path = sym.get_name();
    }
  }
  else
  {
    if(next_path && *next_path )
    {
      path = next_path;  
  
      // Find end of current path.
      char *end;
      for(end = next_path; *end && !isspace(*end); ++end) {}
  
      // Find start of next path.
      for(next_path = end; isspace(*next_path); ++next_path) {}
  
      *end = '\0';
    }
  }

  if ( psym )
     *psym = sym;
  return path;
}

void projList::domain_string()                   // NOT class static
//  call parse_names_first(), then call this, to build project domain list from string
//  Since domain_add() is currently class static, this always puts the list in the same place !!!
{
    const char* proj_name;
    while(proj_name = parse_names_next()) 
       domain_add(proj_name);
}  

int get_all_xreflist_from_proj(objSet & obs, projNode* pn)
{
    Initialize(get_all_xreflist_from_proj);

    projNode* curproj=pn;
    while (curproj && !curproj->get_xref(1))
	curproj=checked_cast(projNode, curproj->find_parent());
    Xref* xr = (curproj) ? curproj->get_xref(1) : 0;
    if (xr && xr->get_lxref() && !xr->is_unreal_file()) {
	// if project has an xref (pmod)
	obs.insert(curproj);
	return obs.size();
    } else  { // project does not have an xref
	pn->refresh_projects(); // so lets get all its children
	ObjPtr os = parentProject_get_childProjects(pn);
    	ObjPtr ob;
    	ForEach(ob, *os) {
	    projNode* pr = checked_cast(projNode, ob);
	    get_all_xreflist_from_proj(obs, pr);
	}
    }
    return obs.size();
}

void get_projs_to_search(objSet & os)
{
  projNode* pr;
  ForEachProj(i,pr){
//  for (int i = 0; (pr=projList::search_list->get_proj(i)) ; ++i) { // readable later
    os |= pr;
    get_all_xreflist_from_proj(os, pr);
  }
}

/*
   START-LOG-------------------------------------------

   $Log: projList.cxx  $
   Revision 1.18 2001/11/07 14:08:14EST ktrans 
   Remove dormant project code, -verify_pdf option, and slow put w/ feedback.  See no_slow_put branch for details.
// Revision 1.33  1994/07/13  15:52:48  aharlap
// bug 7782
//
// Revision 1.32  1994/06/09  12:48:05  so
// Bug track: n/a
// remove the restriction to refresh root projects
//
// Revision 1.31  1994/05/23  21:23:36  so
// Bug track: n/a
// fast projHeader::fn_to_ln
//
// Revision 1.29  1994/05/13  13:29:05  davea
// Bug track: 7280
// Avoid crash in Atria environment;
// change the way we detect readable Xref
//
// Revision 1.28  1994/04/28  23:02:39  trung
// Bug track: 0
// project hiding, symbols_get_link, loading group
//
// Revision 1.27  1994/03/09  17:04:42  trung
// Bug track: 6597
// unload all files belong to projects being hided
//
// Revision 1.26  1994/01/28  23:21:47  davea
// bug 6173:
// remove symbolPtr from all xrefEntry related to
// a project that's being hidden.
//
// Revision 1.25  1994/01/14  14:15:07  davea
// bug 5796
// projList is still messed up in the way domain_array and
// pathlist are handled
//
// Revision 1.24  1994/01/12  00:44:49  davea
// bugs 5940, 5670, 5693, 5882,5896, 5889, and 5900
//
// Revision 1.23  1993/07/13  14:21:25  builder
// syntax error
//
// Revision 1.22  1993/07/13  14:11:34  builder
// check is_ddElement inside domain_add()
//
// Revision 1.21  1993/07/07  23:37:56  jon
// project-scoping separate search_list from full_list
//
// Revision 1.20  1993/06/11  12:15:57  andrea
// removed extraneous printf's
//
// Revision 1.19  1993/06/04  18:49:08  andrea
// model option works and allows user inputted projects as well as multiple projects
//
// Revision 1.18  1993/05/21  23:02:36  davea
// bug 3394 - avoid dereferencing null ptr if no xref
//
// Revision 1.17  1993/05/06  17:29:05  glenn
// Rewrite to use class static functions, add ability to incrementally
// specify search path with domain_* functions.
// Remove unused functions: find_writable_module, find_current_module,
// find_any_module, num_scanables, num_readables, readable.
//
   END-LOG---------------------------------------------
*/


