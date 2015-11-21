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
//  file pdf_tree.h.C
//    implement the fast mapping for pdf definitions -- use flex

#include <cLibraryFunctions.h>
#include <msg.h>
#include <machdep.h>
#include <psetmem.h>
#include <path.h>
#include <pdf_tree.h>
#ifndef _glob_0_h
#include <glob_0.h>
#endif

#ifndef _genWild_h
#include <genWild.h>
#endif

#ifndef _genTmpfile_h
#include <genTmpfile.h>
#endif

#include <objArr_Int.h>

#ifndef _mssages_h
#include <messages.h>
#endif

#include <shell_calls.h>

#include <mpb.h>

void gtPushButton_next_help_context(const char*);

static objArr_Int* retvalues;

pn_list::pn_list(projNode *p, const char *path) : pn(p), next(0), top_path(path)
{
}

pn_list::~pn_list()
{
    if (next)
        delete next;
}

void pn_list::set_next(pn_list *n)
{
    next = n;
}

projNode *pn_list::get_pn()
{
    return pn;
}

pn_list *pn_list::get_next()
{
    return next;
}

int pn_list::get_list_number()
{
    int count = 0;
    pn_list *cur = this;
    while(cur)
    {
        count++;
        cur = cur->get_next();
    }
    return count;
}

const char *pn_list::ret_top_path()
{
    return top_path;
}


// =====================================================================

pm_list::pm_list(projMap *p, pm_list *n) : pm(p), next(n)
{
}

pm_list::~pm_list()
{
    if (next)
        delete next;
}

projMap *pm_list::get_pm()
{
    return pm;
}

pm_list *pm_list::get_next()
{
    return next;
}

// =====================================================================

pdf_tree_head::pdf_tree_head(pn_list *nl, pm_list *ml, char *full_path, 
	pdf_tree_head *p, LEAF_TYPE lp, int path_cnt_selector, PROJ_TYPE ty)
    : pnl(nl), pml(ml), path(full_path), my_idx(-1), resume_idx(-1), next(p), 
      lef_typ(lp), path_contained_selector(path_cnt_selector), type(ty)
{
}

pdf_tree_head::~pdf_tree_head()
{
    if (path.length() && (next == 0 || next->get_pnl() != pnl))
        delete pnl;
    delete pml;
    if (next)
        delete next;
}

pn_list *pdf_tree_head::get_pnl()
{
    return pnl;
}

void pdf_tree_head::set_pnl(pn_list *p)
{
    pnl = p;
}

pm_list *pdf_tree_head::get_pml()
{
    return pml;
}

void pdf_tree_head::set_next(pdf_tree_head *n)
{
    next = n;
}

pdf_tree_head *pdf_tree_head::get_next()
{
    return next;
}

const char *pdf_tree_head::get_path()
{
    return path.str();
}

LEAF_TYPE pdf_tree_head::obtain_leaf_type()
{
    return lef_typ;
}

int pdf_tree_head::is_path_contained_selector()
{
    return path_contained_selector;
}

void pdf_tree_head::set_my_idx(int v)
{
    my_idx = v;
}

void pdf_tree_head::set_resume_idx(int v)
{
    resume_idx = v;
}

int pdf_tree_head::get_my_idx()
{
    return my_idx;
}

int pdf_tree_head::get_resume_idx()
{
    return resume_idx;
}

int pdf_tree_head::obtain_res_idx()
{
    pm_list *pml = get_pml();
    while(pml->get_next())
        pml = pml->get_next();
    projMap *pm = pml->get_pm();

    if (!pm->is_exclude_proj())
        return 1;

    projMap *par = pm->get_parent_project();
    if (!par)
        return 1;
    int num = par->get_num_created_path();
    int count = 1;
    projMap *child = par->get_child_project();
    while(child != pm)
    {
        count++;
        child = child->get_next_project();
    }
    return num - count + 1;
}


// =====================================================================

pdf_state_m::pdf_state_m(pdf_tree_head *h, const char* path)
    : pth(h), top_path(path)
{
    pnl = h->get_pnl();
}

pdf_state_m::~pdf_state_m()
{
}

pdf_tree_head *pdf_state_m::obtain_pdf_tree_head()
{
    return pth;
}

pn_list *pdf_state_m::obtain_pn_list()
{
    return pnl;
}

const char *pdf_state_m::obtain_top_path()
{
    return top_path;
}

void pdf_state_m::set_child_leading_slash(int v)
{
    child_leading_slash = v;
}

int pdf_state_m::get_child_leading_slash()
{
    return child_leading_slash;
}

// =====================================================================

// create pml and path

static pm_list *obtain_pml_by_leaf(projMap *pm, pm_list *child)
{
    Initialize(obtain_pml_by_leaf);

    pm_list *p1 = db_new(pm_list,(pm,child));

    if (pm->get_parent_project())
        return obtain_pml_by_leaf(pm->get_parent_project(), p1);

    return p1;
}

static void combine_two_path(const char *full_path, const char *cur_path, genString &new_full_path)
{
    Initialize(combine_two_path);

    if (*cur_path == '/' || strlen(full_path) == 0)
        new_full_path = cur_path;
    else
        new_full_path.printf("%s/%s",full_path,cur_path);
}

// Case 1: extra tail "/**"
//
//         local = $BASE <-> /aset
//
//         $BASE/**
//
// Case 2: extra line
//
//         local = $BASE <-> /aset
//         {
//             ^**.x
//             "" : **.y => ""
//             "" = SCCS => ""
//         }
//
//         $BASE/**
//
// Case 3: extral tail "/**"
//
//         local : $BASE <-> /aset
//         {
//             "" = SCCS => ""
//             ...
//         }
//
//         $BASE/SCCS/**
//
// Case 4: extral "**" and tail "/**"
//
//         local = $BASE <-> /aset
//         {
//             ^**.x
//             "" : **.y => ""
//             "" = SCCS => ""
//         }
//
//         $BASE/**/SCCS/** # for SCCS

static pdf_tree_head *expand_one_pdf_tree(pn_list *pnl, projMap *pm, const char *full_path, pdf_tree_head *nt,
                                          int *num_of_craeted_paths, int path_cnt_selector)
{
    Initialize(expand_one_pdf_tree);

    pdf_tree_head *pth0 = nt;
    projMap *next  = pm->get_next_project();
    if (next && pm->get_parent_project())
        pth0 = expand_one_pdf_tree(pnl, next, full_path, nt, num_of_craeted_paths, path_cnt_selector);

    if (!path_cnt_selector && pm->need_verify_fn_to_ln())
        path_cnt_selector = 1;

    char const *cur_path = pm->get_dir_name();

    genString new_full_path;
    if (pm->get_parent_project() == 0)
        new_full_path = "";
    else
        combine_two_path(full_path, cur_path, new_full_path);

    projMap *child = pm->get_child_project();

    LEAF_TYPE lp = ACCEPT_LEAF;
    if (!child)
    {
        pm_list *pml = obtain_pml_by_leaf(pm, 0);
        if (pm->is_exclude_proj())
        {
            lp = EXCLUDE_LEAF;
            if (pm->is_implicit_sub_dir_proj()) // =
            {
                projMap *par = pm->get_parent_project();
                if (par->is_implicit_sub_dir_proj())
                {
                    genString temp;
                    temp.printf("**/%s/**",cur_path);
                    combine_two_path(full_path,temp,new_full_path); // Case 4
                }
                else
                    combine_two_path(new_full_path,"**",new_full_path); // Case 3
            }
            else
            {
                projMap *n = pm->get_next_project();
                if (n && !n->is_leaf())
                    combine_two_path(new_full_path,"**",new_full_path); // ^inc
            }
        }
        else if (pm->is_implicit_sub_dir_proj()) // Case 1
            combine_two_path(new_full_path,"**",new_full_path);
        pdf_tree_head *pth1 = db_new(pdf_tree_head, (pnl, pml, new_full_path, pth0, lp, path_cnt_selector,
                                     SEARCH_PROJ));
        *num_of_craeted_paths += 1;
        pm->set_num_created_path(1);
        return pth1;
    }

    int child_num_of_craeted_paths = 0;
    pdf_tree_head *pth2 = expand_one_pdf_tree(pnl, child, new_full_path, pth0, &child_num_of_craeted_paths,
                                              path_cnt_selector);

    if (pm->imp_without_real_filter()) // Case 2
    {
        pm_list *pml = obtain_pml_by_leaf(pm, 0);
        combine_two_path(new_full_path,"**",new_full_path);
        pdf_tree_head *pth3 = db_new(pdf_tree_head, (pnl, pml, new_full_path, 0, CREATED_LEAF, path_cnt_selector,
                                     SEARCH_PROJ));
        pdf_tree_head *last = pth2;
        while(last->get_next())
            last = last->get_next();
        last->set_next(pth3);
        child_num_of_craeted_paths++;
    }

    *num_of_craeted_paths += child_num_of_craeted_paths;
    pm->set_num_created_path(child_num_of_craeted_paths);

    return pth2;
}

extern pdf_tree_head *expand_pdf_tree(pdf_tree_head *pth)
{
    Initialize(expand_pdf_tree);

    pdf_tree_head *p0 = 0;
    if (pth->get_next())
        p0 = expand_pdf_tree(pth->get_next());

    int this_num_created_paths = 0;
    pdf_tree_head *p1 = expand_one_pdf_tree(pth->get_pnl(), pth->get_pml()->get_pm(), "", p0, &this_num_created_paths,
                                            0);
    pth->get_pml()->get_pm()->set_num_created_path(this_num_created_paths);
    pth->get_pml()->get_pm()->set_pdf_tree_head(p1);

    return p1;
}

void update_pdf_tree_line_count(pdf_tree_head *pth, int *last_line_count)
{
    Initialize(update_pdf_tree_line_count);

    while(pth)
    {
        pth->set_my_idx((*last_line_count)++);
        int res_idx = pth->obtain_res_idx();
        pth->set_resume_idx(res_idx);
        pth = pth->get_next();
    }
}

#ifndef ISO_CPP_HEADERS
#include <stdlib.h>
#else /* ISO_CPP_HEADERS */
#include <cstdlib>
using namespace std;
#endif /* ISO_CPP_HEADERS */

static int PDF_DebugLevel = customize::getIntPref("DIS_internal.PDF_DebugLevel");
inline int pdf_debug_print(const char* name)
{
  if (PDF_DebugLevel > 0)
    msg("---------------$1") << name << eom;
  return PDF_DebugLevel;
}


extern "C" void dbg_map_info(projMap *pm);

static void debug_pr_created_path_num(projMap *h)
{
    Initialize(debug_pr_created_path_num);

    if (pdf_debug_print(_my_name) > 0 ) {
        if (!h)
            return;
        dbg_map_info(h);
        msg("$1") << h->get_num_created_path() << eom;
        debug_pr_created_path_num(h->get_child_project());
        debug_pr_created_path_num(h->get_next_project());
    }
}

void debug_out_one_tree(pdf_tree_head *pth, projMap *h)
{
    Initialize(debug_out_one_tree);

    while(pth)
    {
        const char *lp;
        if (pth->obtain_leaf_type() == EXCLUDE_LEAF)
            lp = "EXCLUDE_LEAF";
        else if (pth->obtain_leaf_type() == ACCEPT_LEAF)
            lp = "ACCEPT_LEAF";
        else
            lp = "CREATED_LEAF";

        const char *sel;
        if (pth->is_path_contained_selector())
            sel = "path_contained_selector";
        else
            sel = "path_NO_selector";

        msg("$1 ($2) ($3) ($4) ($5)") << pth->get_path() << eoarg << lp << eoarg << sel << eoarg << pth->get_my_idx() << eoarg << pth->get_resume_idx() << eom;
        pth = pth->get_next();
    }
    debug_pr_created_path_num(h);
}

// ===========================================================================
// home project; hide projects

static pdf_state_m **cur_arr = 0;
static pdf_state_m **new_arr = 0;
static int cur_num = 0;
static int new_num = 0;
static int total_len = 0;

static pm_list *this_pm_l = 0;
static LEAF_TYPE this_lp = NOUSE_LEAF;

projMap *obtain_fast_child_project(projMap *pm)
{
    Initialize(obtain_fast_child_project);

    if (!this_pm_l)
        return 0;
    if (this_pm_l->get_pm() != pm)
    {
        this_pm_l = 0;
        return 0;
    }
    this_pm_l = this_pm_l->get_next();
    if (!this_pm_l)
        return 0;
    return this_pm_l->get_pm();
}

int pdf_tree_parse (const char *fn, int start)
{
    Initialize(pdf_tree_parse__const_char*_int);
    int off = 0;
    IF(start<0)
	return off;

    genString fn_temp = fn;
    for (char* cur = fn_temp; *cur; ++cur) {
	if (*cur == '/')
            *cur = '\n';
    }
    off = pdf_parse((char *)fn_temp, start, &(*retvalues)[0]);
    return off;
}

	
void pdf_tree_fn_to_ln_internal (int off, const char *fn, genString& ln, projNode **pn)
{
    Initialize(pdf_tree_fn_to_ln_internal);
    if (!off) return;
    Assert(off-1<cur_num);
    
    pn_list *pnl = cur_arr[off-1]->obtain_pn_list();
    projNode *proj = 0;
    if (cur_arr[off-1]->get_child_leading_slash() || pnl->get_next() == 0)
	proj = pnl->get_pn();
    else {
	while(pnl) {
	    const char *tp = pnl->ret_top_path();
	    int len = strlen(tp);
	    if (strncmp(tp,fn,len) == 0 && (*(fn+len) == '/' || *(fn+len) == '\0')) {
		proj = pnl->get_pn();
		break;
	    }
	    pnl = pnl->get_next();
	}
    }
    
    if (proj) {
	this_pm_l = cur_arr[off-1]->obtain_pdf_tree_head()->get_pml();
	this_lp   = cur_arr[off-1]->obtain_pdf_tree_head()->obtain_leaf_type();
	proj->fn_to_ln_internal(fn,ln,pn);
	this_pm_l = 0;
	this_lp = NOUSE_LEAF;
	if (ln.length()) {
	    if (pn && *pn == 0)
		*pn = proj;
	    return;
	}
    }
}

void pdf_tree_fn_to_ln(const char* fn, genString &ln, projNode ** pn)
// use flex to quickly match the path
//
// fn: is the path which will be matched
// ln: the returned logic name
// if *pn is zero, *pn will return the found project node
// if *pn is nonzero, ignore the found project node
{
    Initialize(pdf_tree_fn_to_ln);

    if (!fn || *fn == '\0')
        return;

    if (customize::getIntPref("DIS_internal.PDFCheck_Enabled") > 1) {
        msg("pdf_tree_fn_to_ln begin ($1)") << fn << eom;
        ;
    }

    ln = 0;
    int start = 0;

    while(start<cur_num)
    {
        if (pn) *pn = 0;

	int off = pdf_tree_parse((char *)fn, start);
	if (!off)
	    break;

	pdf_tree_fn_to_ln_internal (off, fn, ln, pn);

        if (ln.length())
	    break;

        start = off + cur_arr[off-1]->obtain_pdf_tree_head()->get_resume_idx() - 1;
    }
}

static projMap *local_proj_get_map(projNode *proj)
{
    Initialize(local_proj_get_map);

    projMap *pm = proj->get_map();
    while (!pm)
    {
        proj = proj->find_parent();
        if (!proj)
            return 0;
        pm = proj->get_map();
    }
    return pm;
}

static void init_pdf_tree_arrs()
// 1. count the total pdf using rules
//    using rule is the rule not hidden
// 2. create two arrays
// cur_arr contains all the currently using rules
// new_arr contains all the newly defined rules
// if both array are identical, it will not call flex and could return quickly
// which is the purpose to use two arrays.
{
    Initialize(init_pdf_tree_arrs);

    if (cur_arr){
      for (int i=0;i<cur_num;i++){
	if (cur_arr[i])
	  delete cur_arr[i];
      }    
      psetfree(cur_arr);
      cur_arr = 0;
      cur_num = 0;
    }
    if (new_arr){
      for (int i=0;i<new_num;i++){
	if (new_arr[i])
	  delete new_arr[i];
      }    
      psetfree(new_arr);
      new_arr = 0;
      new_num = 0;
    }
     
    projNode *proj;
    total_len = 0;
    // search_list at this very beginning moment is a full list
    int i;
    for (i=1; proj = projList::search_list->get_proj(i); i++)
    {
        projMap *pm = local_proj_get_map(proj);
        if (pm)
        {
            pdf_tree_head *pdh = pm->obtain_pdf_tree_head();
            pn_list *pnl = pdh->get_pnl();
            // if __rule project is a top project and all its children are rules,
            // it will not be in the pn_list.
            if (pnl)
            {
                int pnl_n = pnl->get_list_number();
                int pm_s = pm->get_num_created_path();
                total_len += pnl_n * pm_s;
            }
        }
        else // save one spot for the original control project /tmp
            total_len++;
    }

    if (pdf_debug_print(_my_name) > 0)
        msg("total_len = $1 new_num = $2") << total_len << eoarg << new_num << eom;

    cur_arr = (pdf_state_m **)psetmalloc(sizeof(pdf_state_m *) * total_len);
    new_arr = (pdf_state_m **)psetmalloc(sizeof(pdf_state_m *) * total_len);
    for (i=0;i<total_len;i++)
    {
        cur_arr[i] = 0;
        new_arr[i] = 0;
    }
}

static int two_array_identical()
{
    Initialize(two_array_identical);

    int same_flag = 1;
    if (new_num != cur_num)
        same_flag = 0;
    else
    {
        for (int i=0;i<cur_num;i++)
        {
            if (new_arr[i]->obtain_pn_list() != cur_arr[i]->obtain_pn_list())
            {
                same_flag = 0;
                break;
            }
        }
    }

    if (!same_flag)
    {
        pdf_state_m **tmp = cur_arr;
        cur_arr = new_arr;
        new_arr = tmp;
    }

    for (int i=0;i<cur_num;i++)
    {
        if (new_arr[i])
            delete new_arr[i];
        new_arr[i] = 0;
    }

    if (pdf_debug_print(_my_name) > 0)
        msg("total_len = $1 new_num = $2") << total_len << eoarg << new_num << eom;
    cur_num = new_num;
    new_num = 0;
    if (pdf_debug_print(_my_name) > 0)
        msg("total_len = $1 new_num = $2") << total_len << eoarg << new_num << eom;


    return same_flag;
}

static void save_gen_pdf(const char *file_name, int type)
// If $PSET_SAVE_GEN_PDF_DIR is set, the file will be saved
//
// file_name: is the generated file name
//
// type = 0: for flex
//      = 1: for config
{
    Initialize(save_gen_pdf);

    genString save_dir = OSapi_getenv("PSET_SAVE_GEN_PDF_DIR");

    if (save_dir.length() == 0)
        return;

    const char *p = strrchr(file_name,'/');
    if (!p)
        p = file_name;

    const char *tail;
    if (type == 0)
        tail = "for_flex";
    else
        tail = "for_config";

    genString to;
    to.printf ("%s/%s_%s",OSPATH(save_dir.str()), p,tail);
    int res = shell_cp (file_name, to);
}

static char* safe_special_chars( const char* str ) 
{ 
  // allocates memory 3*strlen(str) bytes 
  // copyes str into this memory  
  //    changing each special character c to \"c\" 
  // and returns pointer to new string 
 
  char* new_str = 0; 
  if ( str ) 
  { 
    new_str = new char [ 3*strlen(str) +1]; 
    if ( new_str ) 
    { 
      char *p = new_str; 
      while ( *str ) 
      { 
        if ( *str == ' ' ) 
        { *p++ = '\"'; *p++ = *str++; *p++ = '\"'; } 
        else 
          *p++ = *str++; 
      } 
      *p = 0; 
    } 
  } 
 
  return new_str; 
} 
 
static void re_generate_all_pdf_trees()
// re-genate the pdf tree after hide or un-hide project
//                        after select a new home project
//                        after refresh top level project
//                        after define control project the first time
{
    Initialize(re_generate_all_pdf_trees);

    static genTmpfile tnm("genpf");

    if (retvalues)
        delete retvalues;


    retvalues = new objArr_Int;
    retvalues->insert_last(0);
    int delete_tmp_file_flag = 1;
    if (pdf_debug_print(_my_name) > 0) {
        delete_tmp_file_flag = 0;
        msg("$1") << tnm.name() << eom;
        ;
        msg("========================") << eom;
        projNode *projj;
        for (int iz=0; projj = projList::search_list->get_proj(iz); iz++)
            msg("$1") << projj->get_ln() << eom;
        msg("========================") << eom;
        ;
    }
    FILE *out = tnm.open_buffered();
    OSapi_fprintf(out,"\n%s\n","%%");

    mpb_incr();

    for (int i= 0;i<cur_num;i++)
    {
        const char *t = cur_arr[i]->obtain_top_path();
        const char *c = cur_arr[i]->obtain_pdf_tree_head()->get_path();
        const char* special = strstr (c, "/%/");
        if (!special ||
            (strcmp(special+3,".pset") &&
             strcmp(special+3,".make") &&
             strcmp(special+3,".makemf") &&
             strcmp(special+3,".pmoddir")))
        {
            genString buf;
            if (*c == '/')
            {
                cur_arr[i]->set_child_leading_slash(1);
                buf.printf("%s",c);
            }
            else
            {
                cur_arr[i]->set_child_leading_slash(0);
                buf.printf("%s/%s",t,c);
            }

            char* new_buf = safe_special_chars( buf.str() ); 
            const char *p = glob_to_regexp(new_buf, COMPLICATE_LEVEL_3); 
            if ( new_buf ) 
              delete [] new_buf; 
 
            OSapi_fprintf(out,"%s {return %d; REJECT;}\n",p,i+1);
            retvalues->insert_last(i+1);
        }
    }

    tnm.close();

    save_gen_pdf((char *)tnm.name(), 0);

    pdf_load_pdf((char *)tnm.name());

    if (delete_tmp_file_flag)
        tnm.unlink();

    mpb_incr();
}

static void generate_one_tree_in_array(projNode *proj)
{
    Initialize(generate_one_tree_in_array);

    projMap *pm = local_proj_get_map(proj);
    if (!pm)
        return;

    pdf_tree_head *pth = pm->obtain_pdf_tree_head();
    int num = pm->get_num_created_path();

    const char *top_path = pm->get_dir_name();
    // if __rule project is a top project and all its children are rules,
    // it will not be in the pn_list.
    if (top_path) {

        if (pdf_debug_print(_my_name) > 0) {
            msg("before: num = $1 total_len = $2 new_num = $3") << num << eoarg << total_len << eoarg << new_num << eom;
        }

	int leaf_idx = 0;
	pm->set_pdf_tree_arr_indexes (new_num, leaf_idx);

        for (int i=0;i<num;i++) {
            new_arr[new_num++] = db_new(pdf_state_m,(pth,top_path));
            Assert(new_num <= total_len);
            pth = pth->get_next();
        }
    }
}

static void re_generate_arrays()
{
    Initialize(re_generate_arrays);

    init_pdf_tree_arrs();

    projNode* proj, *home = 0;
    int i;
    int start_idx = 0;
    if (projList::search_list->get_proj(0) == 0)
        start_idx = 1;

    for (i=start_idx; proj = projList::search_list->get_proj(i); i++)
        ;
    projMap **flag = (projMap **)psetmalloc(sizeof(projMap *)*i);
    int j;
    for (j=0;j<i;j++)
        flag[j] = 0;
    new_num = 0;

    if (pdf_debug_print(_my_name) > 0)
        msg("total_len = $1 new_num = $2") << total_len << eoarg << new_num << eom;

    for (i=start_idx; proj = projList::search_list->get_proj(i); i++)
    {
        if (i==0)
            home = proj;
        else if (home == proj)
            continue;

        if (!proj->get_xref())
            continue;

        if (! proj->get_xref()->is_writable() && proj->is_visible_proj())
            continue;

        for (j=0; j<i; j++)
        {
            if (flag[j] == local_proj_get_map(proj))
            {
                flag[i] = local_proj_get_map(proj);
                break;
            }
        }

        if (flag[i])
            continue;

        flag[i] = local_proj_get_map(proj);
        generate_one_tree_in_array(proj);
    }

    for (i=1; proj = projList::search_list->get_proj(i); i++)
    {
        if (home == proj)
            continue;

        if(proj->get_xref() && proj->get_xref()->is_writable())
            continue;

        if (flag[i])
            continue;

        for (j=0; j<i; j++)
        {
            if (flag[j] == local_proj_get_map(proj))
            {
                flag[i] = local_proj_get_map(proj);
                break;
            }
        }

        if (flag[i])
            continue;

        flag[i] = local_proj_get_map(proj);

        generate_one_tree_in_array(proj);
    }

    psetfree(flag);

    if (two_array_identical())
        return;
    re_generate_all_pdf_trees();
}

void re_generate_pdf_tree_for_refresh_top_proj()
{
    Initialize(re_generate_pdf_tree_for_refresh_top_proj);

    re_generate_arrays();
}

void re_generate_pdf_tree_for_hiding()
{
    Initialize(re_generate_pdf_tree_for_hiding);

    re_generate_arrays();
}

void re_generate_pdf_tree_for_home(projNode * /* home_p */ )
{
    Initialize(re_generate_pdf_tree_for_home);

    re_generate_arrays();
}

void re_generate_pdf_tree_for_ctl_proj(projNode *ctp, projMap *pm, char *phy_f)
{
    Initialize(re_generate_pdf_tree_for_ctl_proj);

    pn_list *pnl = db_new(pn_list,(ctp, phy_f));
    pm_list *pml = db_new(pm_list,(pm, 0));
    pdf_tree_head *pth = db_new(pdf_tree_head,(pnl,pml,"",0,ACCEPT_LEAF,0,SEARCH_PROJ));
    pdf_tree_head *pth0 = expand_pdf_tree(pth);
    delete pth;

    // link to the existing pdf tree
    pdf_tree_head *h = get_pdf_tree_head();
    while(h->get_next())
        h = h->get_next();
    h->set_next(pth0);
    re_generate_arrays();
}

// start for unexist const top project
typedef struct
{
    projMap *h;
} un_exist_proj_head_entry;

genArr(un_exist_proj_head_entry);

static genArrOf(un_exist_proj_head_entry) un_exist_proj_heads;


void log_un_exist_proj_heads(projMap *pm)
{
    Initialize(log_un_exist_proj_heads);

    un_exist_proj_head_entry *p = un_exist_proj_heads.grow();
    p->h = pm;
}


// end for unexist const top project
