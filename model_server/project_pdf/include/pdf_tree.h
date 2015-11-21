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
//  file pdf_tree.h
//    define the fast mapping for pdf definitions -- use flex

#ifndef _pdf_tree_h
#define _pdf_tree_h

#ifndef _glob_0_h
#include <glob_0.h>
#endif

enum PROJ_TYPE
{
    HOME_PROJ   = 1,
    HIDE_PROJ   = 2,
    SEARCH_PROJ = 4
};

class pn_list
{
  public:
    pn_list(projNode *pn, const char *top_path);
    ~pn_list();
    void set_next(pn_list *n);
    projNode *get_pn();
    pn_list *get_next();
    int get_list_number();
    const char *get_top_path(projNode *p);
    const char *ret_top_path();

  private:
    projNode  *pn;
    genString top_path;
    pn_list   *next;
};

class pm_list
{
  public:
    pm_list(projMap *p, pm_list *n);
    ~pm_list();
    projMap *get_pm();
    pm_list *get_next();

  private:
    projMap *pm;
    pm_list *next;
};

// EXCLUDE_LEAF: "" = dir_name  => ""
//               "" : file_name => ""
//               ^file_name
// CREATED_LEAF: the last line for the implicit project which contains
//               only exclude-projects.
// ACCEPT_LEAF : All others
enum LEAF_TYPE
{
    NOUSE_LEAF = 0,
    EXCLUDE_LEAF,
    ACCEPT_LEAF,
    CREATED_LEAF
};

class pdf_tree_head
{
  public:
    pdf_tree_head(pn_list *nl, pm_list *ml, char *full_path, pdf_tree_head *n,
                  LEAF_TYPE lp, int path_cnt_selector, PROJ_TYPE type);
    ~pdf_tree_head();
    pn_list *get_pnl();
    void set_pnl(pn_list *p);
    pm_list *get_pml();
    void set_next(pdf_tree_head *n);
    pdf_tree_head *get_next();
    const char *get_path();
    LEAF_TYPE obtain_leaf_type();
    int is_path_contained_selector();
    void set_my_idx(int v);
    void set_resume_idx(int v);
    int  get_my_idx();
    int  get_resume_idx();

    void set_type(int p);
    int get_type();

    int obtain_res_idx();

    void add_new_pnl(projNode *pn, char *path);

  private:
    int           type;
    pn_list       *pnl;
    pm_list       *pml;
    genString     path;
    int           my_idx;
    int           resume_idx;
    LEAF_TYPE     lef_typ;
    int           path_contained_selector; // = 1: if the path contains selector
    pdf_tree_head *next;
};

class pdf_state_m
{
  public:
    pdf_state_m(pdf_tree_head *h, const char *path);
    ~pdf_state_m();
    pdf_tree_head *obtain_pdf_tree_head();
    pn_list *obtain_pn_list();
    const char *obtain_top_path();
    void set_child_leading_slash(int v);
    int  get_child_leading_slash();

  private:
    pdf_tree_head *pth;
    pn_list       *pnl;
    genString     top_path;
    int           child_leading_slash;
};

extern int collect_file_names(glob_t &ggg, char const *full_path, projMap *pm, int /* targ_type */ );
extern int go_thru_un_exist_proj_heads();
extern int pdf_parse(char *str, int off, int*);
extern int refresh_top_project(projMap *pm);
extern int get_proj_count(int type);
extern int get_total_time(int type);

extern pdf_tree_head *expand_pdf_tree(pdf_tree_head *pth0);
extern pdf_tree_head *get_pdf_tree_head();

extern void unset_construct_list_for_first_time();

extern projMap *obtain_fast_child_project(projMap *pm);
extern projNode *obtain_next_projnode(projMap *pm, projNode *pn);

extern LEAF_TYPE obtain_this_lp();

extern void debug_out_one_tree(pdf_tree_head *pth, projMap *h);
extern void pdf_tree_fn_to_ln(const char* fn, genString &ln, projNode ** pn);
extern void re_generate_pdf_tree_for_ctl_proj(projNode *ctp, projMap *pm, char *file);
extern void re_generate_pdf_tree_for_hiding();
extern void re_generate_pdf_tree_for_home(projNode *pn);
extern void re_generate_pdf_tree_for_refresh_top_proj();
extern void update_pdf_tree_line_count(pdf_tree_head *pth0, int *last_line_count);
extern void log_un_exist_proj_heads(projMap *pm);
extern void add_new_pn(projMap *pm, char *pm_file_name);
extern void report_globfree(glob_t &ggg);
extern void update_pnl_in_pm(projMap *pm);
extern void add_total_time(int t, int n, int type);

extern void pdf_load_pdf(char *file);
extern void load_pdf_free_pth();
extern void un_exist_proj_heads_reset();
extern void pdf_config_file(char *config_file);
extern char *get_pdf_config_file();

#endif

/*
$Log: pdf_tree.h  $
Revision 1.15 2000/07/07 08:12:59EDT sschmidt 
Port to SUNpro 5 compiler
 * Revision 1.6  1994/07/13  15:50:50  so
 * Bug track: 7750
 * fix bug 7750
 *
 * Revision 1.5  1994/07/11  17:06:19  so
 * Bug track: n/a
 * project for re-load pdf
 *
 * Revision 1.4  1994/06/12  12:12:16  so
 * Bug track: n/a
 * add cancel button for refresh projects
 *
 * Revision 1.3  1994/06/09  12:45:32  so
 * Bug track: n/a
 * remove the restriction to refresh root projects
 *
 * Revision 1.2  1994/06/01  17:15:35  so
 * ^xx will exclude the directory xx and file xx
 *
 * Revision 1.1  1994/05/23  20:16:40  so
 * Initial revision
 *
*/
