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
#include <ParaCancel.h>
#include <cLibraryFunctions.h>
#include <driver_mode.h>
#include <genMask.h>
#include <genString.h>
#include <machdep.h>
#include <msg.h>
#include <symbolScope.h>
#include <symbolSet.h>
#include <systemMessages.h>
#include <xref_priv.h>
#include <xref_queries.h>
#include <xxinterface.h>

char const *PSET_NO_INC_SAVE_PMOD = OSapi_getenv("PSET_NO_INC_SAVE_PMOD");
char const *PSET_MSYNC_XREF = OSapi_getenv("PSET_MSYNC_XREF");

int xref_get_doing_second_pass();
char const* customize_ste_suffix();

//  this function gets "called" when you want to die...

//  external function (pre_suggest.C) which ought to be in a header, but is not
int get_all_subprojects_from_proj(objSet & obs, projNode* pn, int deep = 0);

#define XrefTable_Index(i) ((i) % header->table_max)

Xref * XREF = 0; // new Xref();
int start_call_rel_hooks;
/*--------------------------------------------*/

int init_local_search_var = linkTypes::init_local_search();

#ifdef  EXCLUSIVE_OPEN
static bool  abort_flag=0;
#endif

void fill_with_all_not_internal_dds(ddSelector& ddsel)
{ 
    static ddKind dd_kinds[] = {DD_PROJECT, DD_MODULE, DD_CLASS, DD_ENUM,
				DD_MACRO, DD_FUNC_DECL, DD_VAR_DECL, DD_TYPEDEF,
				DD_UNION, DD_SUBSYSTEM, DD_PARAM_DECL, DD_FIELD,
				DD_TEMPLATE, DD_RELATION};

    for(int i = 0; i < sizeof(dd_kinds); i++)
	ddsel.add(dd_kinds[i]);
}

int ddKind_is_local(ddKind k)
{
  return (k == DD_PARAM_DECL);
}


int dd_has_type(ddKind k)    
{
  int retval = 0;

  switch (k) 
    {
    case DD_VAR_DECL:
    case DD_FUNC_DECL:
    case DD_FIELD: 
    case DD_PARAM_DECL:
    case DD_TYPEDEF:
    case DD_LOCAL: 
    case DD_NUMBER:
    case DD_STRING:
    case DD_SEMTYPE:
      retval = 1;
      break;
      
    default:
      break;
    }

  return retval;
}

Xref* load_XREF(char const *xref_file, int perm)
{ Initialize(load_XREF(char const *,int));
  if (XREF == 0)
    XREF = new Xref(xref_file, perm,(projNode*) 0 );
  last_Xref = XREF;
  return XREF;
}
//  Note:  according to Alex, Trung says this get_XREF() is a last resort
//   Not clear why.
//  It is declared with a single default argument, which does not get
//    used anyway.  So some people call it with a char *, and others without.

Xref* get_XREF(char const *)
{ 
  Initialize(get_XREF);
  if (projList::search_list) {
    projNode* pr = projList::search_list->get_proj(0);
    if (pr && pr->get_xref()) {
      Xref* xr = pr->get_xref();
      if (xr->get_lxref() == 0) xr->init(1);
      return xr;
    }
  }
  return XREF;
}

extern "C" int node_prx (const Obj* obj)
{
    Initialize (node_prx);

    if (symbolPtr_is_symbol(obj)) {
      const xrefSymbol* xr = (const xrefSymbol*)obj;
      xr->print ();
      return 1;
    }

    return 0;
}
	

/* extern "C" int gettimeofday(struct timeval *, struct timezone *); */

int get_time_of_day()
{
    long usec;
    long sec;
    if (OSapi_time(&usec, &sec))
      return 0;
    else
      return (int)sec;
}

void copy_home_pmod_to_perm()
{
  Initialize(copy_home_pmod_to_perm);
  if (!is_model_build() || PSET_NO_INC_SAVE_PMOD) return;
  projNode *pr = projNode::get_home_proj();
  if (!pr) return;
  Xref *Xr = pr->get_xref();
  if (!Xr || Xr->is_unreal_file()) return;
  XrefTable * xref = Xr->get_lxref();
  if (xref) {
    xref->copy_pmod_to_perm();
  }
}


void XREF_set_last_modified_date (app* sh)
{
    Initialize (XREF_set_last_modified_date);

    char const *pn = sh->get_phys_name();

    projNode *proj = app_get_proj(sh);
    IF (!proj) return;

    Xref *xr = proj->get_xref();
    IF(!xr) return;
    
    char const *fn = sh->get_filename();
    xrefSymbol *file_sym = xr->lookup_module (fn);
    if (!file_sym) return;


    if (pn) file_sym->set_last_mod_date(get_last_modified_date(pn), xr->get_lxref());
}

//extern "C"  void  Test_Pmod();
void print_XREF()
{
  Initialize(print_XREF );
  projNode* pr = projList::search_list->writable();
  Xref* Xr = pr->get_xref();
  print_XREF(Xr);
}

void print_XREF(Xref* Xr)
{
  Initialize(print_XREF );
}

void get_pmod_projs_from(objArr& subproj_Arr, projNode* pn)
//  Build a list of subprojects with pmods
//  If a given (sub)project does not have a pmod, search its subprojects
//  Note that a pmod for a particular level will "hide" all the lower-level pmods
{
    Initialize(get_pmod_projs_from);
    projNode* curproj=pn;
    
    while (curproj && !curproj->get_xref(1))
	curproj=checked_cast(projNode, curproj->find_parent());
    Xref* xr = (curproj) ? curproj->get_xref(1) : 0;
    if (xr && xr->get_lxref() && !xr->is_unreal_file()) {
	// if project has an xref (pmod)
	subproj_Arr.insert_last(curproj);
	return;
    } else  { // project does not have an xref
	pn->refresh_projects();
    	Obj *os = parentProject_get_childProjects(pn);
    	ObjPtr ob;
    	ForEach(ob, *os) {
	    projNode* pr = checked_cast(projNode, ob);
	    get_pmod_projs_from(subproj_Arr, pr);
	}
    }
}


static int proj_name_cmp(const void *a, const void *b)
{
    Initialize(proj_name_cmp);

    projNode * t1 = *(projNode **) a;
    projNode * t2 = *(projNode **) b;
    return strcmp(t1->get_ln(), t2->get_ln());
}
void dd_get_pmod_subprojects(objArr& sub_projects)
{
 projNode*proj;
 for (int i = 1; proj = projList::search_list->get_proj(i); i++) {
    if(proj->is_writable() && proj->is_visible_proj() 
          && strcmp(proj->get_ln(), "/ParaDOCS") && strcmp(proj->get_ln(), "/tmp")){
      get_pmod_projs_from(sub_projects, proj);
      sub_projects.sort(proj_name_cmp);
    }
  }
}

/*static*/ int get_all_subprojects_from_proj(objSet & obs, projNode* pn, int deep)
// Starting at pn, search for all subprojects, and add them to the objSet
// If deep is true, refresh as you go.  Otherwise assume that all subprojects
//   we are interested in are already refreshed.
// Return final number of projects in the set
//    (recursive fct)
{
    Initialize(get_all_subprojects_from_proj);
    if (deep)
      pn->refresh_projects();
    Obj* os = parentProject_get_childProjects(pn);
    ObjPtr ob;
    ForEach(ob, *os) {
      obs.insert(ob);
      projNode* pr = checked_cast(projNode, ob);
      get_all_subprojects_from_proj(obs, pr, deep);
    }
    return obs.size();
}

int get_projList(objSet & ps)
// get all top-level projects
{
  projNode* pr;
  for (int i = 1; (pr=projList::search_list->get_proj(i)) ; ++i) {
    ps.insert(pr);
  }
  return ps.size();
}

bool  is_subproj_of(projNode* pr1, projNode* pr2)
{
    // return true if pr1 is a subproject of pr2
    Initialize(is_subproj_of);
    while (pr1 && pr2!=pr1) 
	pr1 = checked_cast(projNode, pr1->find_parent());
    return (pr2==pr1);
}
   
//  To make a testing version, uncomment the following 
#ifdef _PSET_DEBUG
//#define  TEST_VERSION
#endif

void new_xref_init_rtl(RTLNodePtr rtl_node, const ddSelector& selector)
{
  Initialize(new_xref_init_rtl);
  symbolArr symbols;		// this is where we accumulate the symbols
  symbols.removeAll();
  projNode* pr;
  int i,j;
  int tempsize;    // for debugging only
  objArr   subproj_Arr;
  subproj_Arr.removeAll();
  for (i = 0; (pr = projList::search_list->get_scan(i)); ++i)
      Xref::get_xreflist_from_proj(subproj_Arr, pr);

  // figure out home project
  projNode* home = projNode::get_current_proj();
  projNode* spec_home = 0;
#ifdef _HIER_HOME
  objArr home_proj_Arr;                  // list of chosen home subprojects that have pmods
  home_proj_Arr.removeAll();
#endif
  
// filter out dups in the subproj list:
//     Any subproject which has an ancestor in the same list is removed
  if (subproj_Arr.size()) {
      tempsize = subproj_Arr.size();           // for debugging only
      for (i=0; i< subproj_Arr.size();i++) {
	  j=i+1;
	  while (1) {
	      tempsize = subproj_Arr.size();        // for debugging only
	      projNode* pr1 = checked_cast(projNode, subproj_Arr[i]);
	      if (is_subproj_of(pr1,home)) {   // if he specifies home, remove it temporarily
		  spec_home = checked_cast(projNode, subproj_Arr[i]);
#ifdef _HIER_HOME
		  home_proj_Arr.insert_last(spec_home);
#endif
		  subproj_Arr.remove(i);
		  continue;
	      }
	      if (j>= subproj_Arr.size()) break;
	      projNode* pr2 = checked_cast(projNode, subproj_Arr[j]);
	      if (is_subproj_of(pr2,pr1)) {
		  subproj_Arr.remove(j);
		  continue;
	      }
	      if (is_subproj_of(pr1,pr2)) {
		  subproj_Arr.remove(i);
		  j=i+1;
		  continue;
	      }
	      j++;
	  }
      }
  }
//  cerr << "Projects found= " << subproj_Arr.size() << endl;     // for debugging only
    if (subproj_Arr.size()) {
    	for (i=0; i< subproj_Arr.size(); i++) {
	    Obj* ob= subproj_Arr[i];
	    pr = checked_cast(projNode, ob);
	    if (pr->get_xref(1)) pr->get_xref(1)->add_symbols(symbols, selector);
    	}
    }
#ifdef TEST_VERSION
    int deleted=0;
    cerr << "Original size " << symbols.size()  << endl;
  symbolPtr lasts=NULL_symbolPtr;
  xrefSymbol* lastd=0;
#endif
    Xref *homexr = (spec_home) ? spec_home->get_xref(1): 0 ;
    if (spec_home && subproj_Arr.size()) {    
	tempsize = subproj_Arr.size();           // for debugging only
	XrefTable* homexrt = homexr->get_lxref ();
      	symbolPtr sym;
	i=symbols.size();
	while (--i>=0) {
            if (ParaCancel::is_cancelled()) break;

	    sym = symbols[i];
	    symbolPtr def = sym->get_def_file();
	    if (def.xrisnotnull()) {
	    genString lname = def.get_name();  // logical name of defining file
	    xrefSymbol* sym2 = homexrt->lookup_module(lname);
	    if (sym2) {
		if (sym2->get_has_def_file(homexrt)) {
		    symbols.remove_index(i);
#ifdef TEST_VERSION
		    deleted++;
		    cerr << deleted << " Delete " << (char const *)sym.get_name() << " decl in " << (char const *)lname << endl;
		    //sym2->print();
#endif
	    	}
#ifdef  TEST_VERSION
		symbolArr ar;
		int jj;
		if (jj=sym2->get_link(is_defining,ar,1)) {
		    cerr << "--- is_defining link has count"<< jj  << endl;
		}
#endif
      }
	  }
    }
      }
  
#ifdef _HIER_HOME
#ifdef _PSET_HOME  
    cerr << "Home projects chosen: " << home_proj_Arr.size() << endl;cerr.flush();            // bugbug -- remove before submit
#endif
    ObjPtr ob;
    ForEach(ob, home_proj_Arr) {
	projNode* spec_home = checked_cast(projNode, ob);
	if (spec_home) {
	    homexr = spec_home->get_xref(1);
	    if (homexr) homexr->add_symbols(symbols, selector);   // now add the home project stuff back in
	}
    }
#else
    if (homexr) homexr->add_symbols(symbols, selector);   // now add the home project stuff back in
#endif
    // add all these symbols we just found to the rtl_node
  symbolPtr sym;
  ForEachS(sym, symbols) {
      if (ParaCancel::is_cancelled()) break;
      rtl_node->rtl_insert(sym);
  }
}

symbolPtr lookup_file_symbol_in_xref(char const *fn, Xref* xr)
//  look up file symbol in the pmod for specified xref
//  If pmod is not mapped, load it, but do not look elsewhere
{

  symbolArr res;
  symbolPtr sym;
  if (fn == 0) return NULL_symbolPtr;
    if (xr && xr->get_lxref()) {
      sym = xr->lookup_local(res, DD_MODULE, fn, fn);
      if (sym.xrisnotnull())
	  return sym;
  }
  return NULL_symbolPtr;
}

symbolPtr lookup_file_symbol(char const *fn)
{
  projNode* pr;
  symbolArr res;
  symbolPtr sym;
  if (fn == 0) return NULL_symbolPtr;

  ForEachProj(i,pr){
//  for (int i = 0; (pr=projList::search_list->get_proj(i)) ; ++i) {
    Xref* xr = pr->get_xref(1);
    if ((xr == 0) || (xr->get_lxref() == 0)) continue;
    sym = xr->lookup(res, DD_MODULE, fn, fn);
    if (sym.xrisnotnull())
      return sym;
  }
  return NULL_symbolPtr;
}

static symbolPtr  file_included_from(projNode*proj, char const *fn)
{
   Xref* xr = (proj)?proj->get_xref(1):0;
   if(!xr)
       return  NULL_symbolPtr;


   symbolPtr sym = lookup_file_symbol_in_xref(fn, xr);
   if(sym.isnull())
       return  NULL_symbolPtr;

   symbolArr bs;
   int sz = sym->get_link(included_by, bs);
   if(sz)
     return bs[0];

   return  NULL_symbolPtr;
}
extern "C" char const *get_name(Relational* node)
{ 
  return
   node->get_name();
}

symbolPtr xref_header_file_included_from(char const *fn)
{ 
    // first check current project

    projNode *proj = projNode::get_current_proj();

    symbolPtr sym = file_included_from(proj, fn);
    if(!sym.isnull())
	return sym;
    if (!xref_get_doing_second_pass()) {
	for (int i = 1; proj = projList::search_list->get_proj(i); i++) {
	    objArr sub_projects;
	    get_pmod_projs_from(sub_projects, proj);
	    Obj*el;
	    ForEach(el, sub_projects){
		proj = projNodePtr(el);
		sym = file_included_from(proj, fn);
		if(!sym.isnull())
		    return sym;
	    }
	}
    }
    return  NULL_symbolPtr;
}
// should make this a method of xref

int lookup_all_file_symbol(symbolArr& res, char const *fn)
{
    Initialize(lookup_all_file_symbol(symbolArr&,char const *));
    projNode* pr;
    xrefSymbol* sym;
    if (fn == 0) return 0;
    if (xref_get_doing_second_pass()) {
	pr = projNode::get_home_proj();
	Xref* xr = (pr)?pr->get_xref(1):0;
	XrefTable * xrt;
	if (xr && (xrt = xr->get_lxref())) {
	    sym = xrt->lookup_module(fn);
	    if (sym && !res.includes(sym))
		res.insert_last(sym);
	}
    } else {
	objSet   subproj_Set;
	ForEachProj(i,pr){
	    subproj_Set.insert(pr);
	    get_all_subprojects_from_proj(subproj_Set, pr);
	}
 
	ObjPtr ob;
	ForEach(ob, subproj_Set) {
	    pr = checked_cast(projNode, ob);
	    Xref* xr = (pr)?pr->get_xref(1):0;
	    if ((xr == 0) || (xr->get_lxref() == 0)) continue;
	    symbolArr res_tmp;
	    sym = xr->lookup(res_tmp, DD_MODULE, fn, fn);
	    if (sym && !res.includes(sym))
		res.insert_last(sym);
	}
    }
    return res.size();
}

projNode* lookup_all_proj_for_symbol(xrefSymbol* sym)
{
  Initialize(lookup_all_proj_for_symbol(xrefSymbol*));
  projNode* pr;
  if (sym == 0) return 0;
  objSet   subproj_Set;
  ForEachProj(i,pr){
//  for (int i = 0; (pr = projList::search_list->get_proj(i)); ++i) {
    subproj_Set.insert(pr);
    get_all_subprojects_from_proj(subproj_Set, pr);
  }
  ObjPtr ob;
  XrefTable* xref;
  char const *next_addr;
  ForEach(ob, subproj_Set) {
    pr = checked_cast(projNode, ob);
    Xref* xr = (pr)?pr->get_xref(1):0;
    if ((xr == 0) || (xr->get_lxref() == 0)) continue;
    if ((xref = xr->get_lxref()) &&
	(next_addr = sym->belonged_to(xref)))
      return pr;
  }
  return 0;
}

// public version of following function
symbolPtr lookup_file_symbol(ddKind kind, char const *fn)
{
    xrefSymbol* foo = lookup_file_symbol_priv(kind, fn);
    return symbolPtr(foo);
}
xrefSymbol* lookup_file_symbol_priv(ddKind kind, char const *fn)
{
  projNode* pr;
  symbolArr res;
  xrefSymbol* sym;
  if (fn == 0) return 0;
  ForEachProj(i,pr){
//  for (int i = 0; (pr=projList::search_list->get_proj(i)) ; ++i) {
    Xref* xr = pr->get_xref(1);
    if ((xr==0) || (xr->get_lxref() == 0)) continue;
    sym = xr->lookup(res, kind, fn);
    if (sym) {
       // The following code partially circumvents a bug in rename:
       // renaming a symbol leaves "droppings" -- leftover symbols
       // with the correct name but no def_file -- that can confuse
       // code that is looking for the definition of a particular name.
       if (res.size() > 1) {
	  for (size_t j = 0; j < res.size(); j++) {
	     if (res[j].get_has_def_file()) {
		return res[j];
	     }
	  }
       }
      return sym;
    }
  }
  return NULL;
}



static char const *project_root=0;

symbolPtr lookup_xrefSymbol(ddKind kind, char const *fn)
{
  projNode* pr;
  symbolArr res;
  symbolPtr sym;
  if (fn == 0) return NULL_symbolPtr;
  ForEachProj(i,pr){
//  for (int i = 0; (pr=projList::search_list->get_proj(i)) ; ++i) {
    if(project_root && strcmp(project_root, pr->get_name()))
      continue;
    Xref* xr = pr->get_xref(1);
    if ((xr==0) || (xr->get_lxref() == 0)) continue;
    sym = xr->lookup(res, kind, fn);
    if (sym.isnotnull())
      return sym;
  }
  return NULL_symbolPtr;
}
symbolPtr lookup_xrefSymbol(ddKind kind, char const *fn, char const *root)
{
  project_root = root;
  symbolPtr sym = lookup_xrefSymbol(kind, fn);
  project_root = 0;
  return sym;
}
// private version

// return 0 if no physical filename
// return 0 if physical name is not in any project
//                   or if the project has no xref
// return symbol if logical name has a symbol, and it is
//      not older than the physical file
symbolPtr Xref_file_is_newer(char const *ln, char const *fn)
{
  Initialize(Xref_file_is_newer);
  return symbolPtr(Xref_file_is_newer_priv(ln,fn));
}

// private version
xrefSymbol* Xref_file_is_newer_priv(char const *ln, char const *fn)
{
  Initialize(Xref_file_is_newer_priv);

  if (!fn) return 0;

  genString gs;
  projNode *proj = projNode::fn_to_pmod_proj (fn, gs);

  if (!proj) return 0;

  Xref *xref = proj->get_xref();
  if (!xref) return 0;

  XrefTable* xr = xref->get_lxref();
  if (!xr) return 0;

  xrefSymbol* sym = xr->lookup_module (ln);

  if (sym && !sym->is_newer(fn, xr))
    return sym;

  return 0;
}

link_iter::link_iter(xrefSymbol* s, linkType lt)
{
  Initialize(link_iter::link_iter);
  count = 0;
  s->get_link(lt, sa);
}

xrefSymbol* link_iter::get_next()
{
  Initialize(link_iter::get_next);

  if (count < sa.size())
    return ((xrefSymbol*) sa[count++]);
  else
    return NULL;
}

void get_filtered_link (ddKind kind, symbolPtr xr, 
			linkType lt, symbolArr& as)
{

    Initialize(get_filtered_link);
    symbolArr tmp;
    xr->get_link (lt, tmp, 0);                 // search global Xref as well
    symbolSet ss;
    ss = tmp;
    tmp = ss;
    as.insert_last(tmp);
}

symbolPtr get_semtype_symbol(symbolPtr sym)
{
  Initialize(get_type_symbol);
  symbolPtr ret;
  if (sym.xrisnotnull() && dd_has_type(sym.get_kind())){
    symbolArr as;
    if (sym->get_link(has_type, as, 1)) /* try local first */
      ret = as[0];
    else if (sym->get_link(has_type, as))
      ret = as[0];      
  }
  return ret;
}
char const *xref_c_proto(const symbolPtr &src)
{
  Initialize(xref_c_proto);

  symbolPtr sym = src.get_xrefSymbol();
  if (sym.xrisnull() || sym.get_kind() != DD_FUNC_DECL) 
    return NULL;

  char const *nm = sym.get_name();
  if(!nm || strchr(nm, '('))
    return NULL;

  symbolPtr pr;
  symbolArr as;
  if (sym->get_link(has_c_proto, as, 1)) /* try local first */
    pr = as[0];
  else if (sym->get_link(has_c_proto, as))
    pr = as[0];

  char const *proto = NULL;
  if (pr.xrisnotnull())
    proto = pr.get_name(); 

  return proto;
}

static int good_suffix(char const *full_path, char const *suffix)
{
    Initialize(good_suffix);

    if (full_path && suffix) {
        char const *p = strrchr(full_path,'.');

        if (p && strcmp(p, suffix) == 0)
            return 1;
    }
    return 0;
}

int xref_not_allowed_file_type(char const *full_path)
{
    Initialize(xref_not_allowed_file_type);

    char const *p = strrchr(full_path,'.');

    if (p && (strcmp(p,".rtl") == 0 || strcmp(p,".exe") == 0))
        return 0;

    return 1;
}

int xref_not_header_file(char const *full_path)
{
    Initialize(xref_not_header_file);

     if (is_c_hdr_suffix(full_path) > 0)
 	return 0;
     if (is_cplusplus_hdr_suffix(full_path) > 0)
 	return 0;

    return 1;
}

bool is_c_suffix(char const *full_path);
bool is_cplusplus_suffix(char const *full_path);

int xref_not_src_file(char const *full_path)
{
    Initialize(xref_not_src_file);

    if (is_c_suffix(full_path) > 0)
        return 0;
    if (is_cplusplus_suffix(full_path) > 0)
        return 0;

    return 1;
}

int xref_not_txt_file(char const *full_path)
{
    Initialize(xref_not_txt_file);

    if (good_suffix(full_path, customize_ste_suffix()))
        return 0;

    return 1;
}

int strcmp_ignore_space(char const *s1, char const *s2)
{
  if (s1 == 0 && s2 == 0) return 0;
  else if (s1 == 0 || s2 == 0) return 1;
  while (*s1 && *s2) {
    if (*s1 == ' ') {
      s1++;
      continue;
    }
    if (*s2 == ' ') {
      s2++;
      continue;
    }
    if (*s1 != *s2) break;
    s1++; s2++;
  }
  return ( *s1 || *s2 );
}

int strcmp_name(ddKind kind, char const *s1, char const *s2)
{
  if(kind == DD_STRING  || kind == DD_MODULE || kind == DD_PROJECT)
    return strcmp(s1, s2);
  else 
    return strcmp_ignore_space(s1, s2);
}

int not_call_hook(RelType* )
{
    return 0;
}
int  file_get_modified_object(symbolPtr sym, symbolArr & modified, time_t t);

void project_query(symbolPtr dsym, linkType link, ddKind kind, symbolArr& result)
// takes a valid  symbolPtr
// if it's a project, query the xref for links within that project
// put results in result
{
  Initialize(project_query);
  
  appPtr project = dsym.get_def_app();
  if(project && is_projHeader(project)) {
      projNode *pn = checked_cast(projNode,project->get_root());
      objArr proj_arr;
      proj_arr.insert_last(pn);
      symbolScope scope;
      scope.set_domain(proj_arr);
      ddSelector ddsel;
      if(kind != -1)
	  ddsel.add(kind);
      else{
	  fill_with_all_not_internal_dds(ddsel);
      }
      ddsel.want_used = (link == is_using);
      scope.query(result, ddsel);
  }
}


void find_overrides_local (symbolArr& src, symbolArr & dst)
{
    Initialize(find_overrides_local(symbolArr&,symbolArr&));
    
    symbolPtr sym;

    ForEachS(sym, src) {
	sym.get_overrides(dst, true);
    }
}

void find_overrides(symbolArr& src, symbolArr & dst)
{
    Initialize(find_overrides(symbolArr&,symbolArr&));

    find_overrides_local(src,dst);
}	

int  file_get_modified_object(symbolPtr sym, symbolArr & modified, time_t t)
{
  Initialize(file_get_modified_object);
  int prev_size = modified.size();
  if (sym.isnull()) return prev_size;
  symbolArr defines_sym;
  sym->get_link(is_defining, defines_sym);
  
  symbolPtr sym1;
  ForEachS(sym1, defines_sym) {
    
    SwtEntityStatus ses = (SwtEntityStatus) sym1->get_attribute(SWT_Entity_Status, 2);
    if (ses == SWT_MODIFIED || ses == SWT_NEW) {
      time_t sym1_t = sym1.get_last_mod_date();
      if (sym1_t > t)
	modified.insert_last(sym1);
    }
  }
  return modified.size() - prev_size;
}

smtHeader *dis_reload_file (char const *f_name);
int module_needs_reparse(projModule *module);
int get_modified_objects(symbolArr &selected, symbolArr &modified)
{
  Initialize(get_modified_objects);

  // check for Delta license
  static bool has_delta_lic = false;
  if ( _lo(LIC_DELTA) )
    _lm(LIC_DELTA);
  else
    has_delta_lic = true;

  symbolArr tmp;
  symbolPtr dsym;

  ForEachS(dsym, selected) {
    symbolPtr sym = dsym.get_xrefSymbol();
    if (sym.isnull()) continue;
    /* converting from projModule to symbol */
    if (sym->get_kind() != DD_MODULE) continue;
    if (sym->get_kind() == DD_MODULE && 
	dsym.relationalp()
	&& is_projModule((Relational *)dsym)) {
      projModule *mod = checked_cast(projModule, dsym);
      genString sym_name = sym.get_name();
      if (mod == 0) {
	           msg("WARN: File $1 does not exist.") << sym_name.str() << eom;
      } else {
	           projNode *project = mod->get_project();
	          sym = mod->get_def_file_from_proj(project);
      }
    }
    if (sym.isnull()) continue;
       file_get_modified_object(sym, tmp, (time_t) -1);
  }
  tmp.remove_dup_syms();
  for(int i = 0; i < tmp.size(); i++)
      if(tmp[i].get_kind() != DD_VERSION)
	  modified.insert_last(tmp[i]);
  return modified.size();
}

int add_one_level_syms(linkType lt, ddSelector sel_src, ddSelector sel_trg, symbolArr & syms)
{
  Initialize(add_one_level_syms);

  symbolArr members;
  symbolPtr sym1;
  symbolPtr sym2;
  symbolPtr sym3;
  for (int i = 0; i < syms.size(); ++i) {
    sym1 = syms[i];
    sym2 = sym1.get_xrefSymbol();
    if (sym2.isnull()) continue;
    if (sel_src.selects(sym2.get_kind())) {
      symbolArr mems;
      sym2->get_link(lt, mems);
      for (int j = 0; j < mems.size(); ++j) {
	sym3 = mems[j];
	if (sel_trg.selects(sym3.get_kind()))
	  members.insert_last(sym3);
      }
    }
  }
  syms.insert_last(members);
  syms.remove_dup_syms();
  return syms.size();
}




