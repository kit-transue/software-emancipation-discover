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
#include <customize.h>
#ifndef ISO_CPP_HEADERS
#include <memory.h>
#else /* ISO_CPP_HEADERS */
#include <memory>
using namespace std;
#endif /* ISO_CPP_HEADERS */
#include <machdep.h>
#include <psetmem.h>

#ifndef _objOper_h
#include <objOper.h>
#endif

#ifndef _path_h
#include <path.h>
#endif

#ifndef _patherr_h
#include <patherr.h>
#endif

#ifndef _path0_h
#include <path0.h>
#endif

#ifndef _pathcheck_h
#include <pathcheck.h>
#endif

#ifndef _fileCache_h
#include <fileCache.h>
#endif

#ifndef _vpopen_h
#include <vpopen.h>
#endif


wkIdxDef::wkIdxDef(wkIdxDef *s)
{
    Initialize(wkIdxDef::wkIdxDef);

    type          = s->get_type();
    wk_str        = s->get_str();
    wk_idx        = s->get_index();

    if (s->get_next())
        next = db_new(wkIdxDef, (s->get_next()));
    else
        next = 0;
}

wkIdxRule::wkIdxRule(wkIdxRule *s)
{
    Initialize(wkIdxRule::wkIdxRule);

    idx_def_flag = s->idx_is_defined();
    org_idx      = s->get_org_idx();
    idx_def      = s->get_idx_def();
    idx_strs     = db_new(wkIdxDef, (s->get_idx_strs()));
    ptn          = s->get_ptn();
    multi_match_in_idxrule = 0;
    set_multi_match_in_idxrule();
}

wkPtnDef::wkPtnDef(wkPtnDef *s)
// copy constructor for wkPtnDef
//
// s is the source wkPtnDef.
{
    Initialize(wkPtnDef::wkPtnDef);

    type          = s->get_type();
    wk_str        = s->get_str();
    idx           = s->get_idx();
    if (s->get_filter())
        ptn_pf    = db_new(projFilter, (s->get_filter()->get_filter()));
    else
        ptn_pf    = 0;

    if (s->get_next())
        next = db_new(wkPtnDef, (s->get_next()));
    else
        next = 0;
}

wkPtnRule::wkPtnRule(wkPtnRule *s)
// copy constructor for wkPtnRule
//
// s is the source wkPtnRule.
{
    Initialize(wkPtnRule::wkPtnRule);

    org_ptn  = s->get_org_ptn();
    ptn_def  = s->get_ptn_def();
    if (s->get_ptn_strs())
        ptn_strs = db_new(wkPtnDef, (s->get_ptn_strs()));
    else
        ptn_strs = 0;
    result = 0;
    exclude_dir_ptn_def   = s->get_exclude_dir_ptn_def();
    exclude_file_ptn_def  = s->get_exclude_file_ptn_def();
    multi_match_in_ptnrule = 0;

    set_multi_match_in_ptnrule(NON_EXCLUDE, NORMAL_PHY_PATH);
    if (exclude_file_ptn_def.length())
        set_dir_multi_match_in_ptnrule(NON_EXCLUDE);
}

path_hash_entry **projMap::copy_hash_tbl(projMap *cur, char file_type, char chosen_rule, char trans_type)
// copy one hash table
//
// cur: is the source projmap from which the hash table is copied from
// file_type: FILE_TYPE or DIR_TYPE
// choosen_rule: LOGIC_NAME_RULE or PROJECT_NAME_RULE
// trans_type: FNTOLN or LNTOFN
{
    Initialize(projMap::copy_hash_tbl);

    path_hash_entry **hh = cur->get_hash_head(file_type, chosen_rule, trans_type);
    if (hh == 0)
        return 0;
    path_hash_entry **nn = create_hash_tbl(file_type, chosen_rule, trans_type);
    for (int i=0;i<MAX_PATH_HASH;i++)
    {
        if (hh[i])
        {
            path_hash_entry *h = hh[i];
            path_hash_entry *last = 0;
            while(h)
            {
                path_hash_entry *n = db_new(path_hash_entry, (h->get_fn(), h->get_ln(), h->get_proj_n()));
                if (!last)
                    nn[i] = n;
                else
                    last->set_next(n);
                last = n;

                h = h->get_next();
            }
        }
    }
    return nn;
}

void projMap::copy_phy_filters(projMap *s, int start)
// copy the physical rules
//
// s is the source projmap from which the rules will be copied from
// start= 0 or 1
//      = 0 copy from the first rule; for or-projects
//      = 1 copy from the second rule; for flattening project
{
    Initialize(projMap::copy_phy_filters);

    for(int i=start;i<s->get_multi_phy_rule()->size();i++)
    {
        phy_rule_entry *p = multi_phy_rule.grow();
        phy_rule_entry *q = (*s->get_multi_phy_rule())[i];
        p->rule_type = q->rule_type;
        p->file_type = q->file_type;
        p->phy_rule = db_new(wkPtnRule, (q->phy_rule));
    }
}

projMap::projMap(projMap *s)
// the copy constructor for projMap
//
// s: is the source projmap
// return the newly created (copied) projmap
{
    Initialize(projMap::projMap);

    fn_to_ln_file_h     = copy_hash_tbl(s, FILE_TYPE, LOGIC_NAME_RULE,   FNTOLN);
    fn_to_ln_dir_h      = copy_hash_tbl(s, DIR_TYPE,  LOGIC_NAME_RULE,   FNTOLN);
    ln_to_fn_file_h     = copy_hash_tbl(s, FILE_TYPE, LOGIC_NAME_RULE,   LNTOFN);
    ln_to_fn_dir_h      = copy_hash_tbl(s, DIR_TYPE,  LOGIC_NAME_RULE,   LNTOFN);
    fn_to_ln_file_imp_h = copy_hash_tbl(s, FILE_TYPE, PROJECT_NAME_RULE, FNTOLN);
    fn_to_ln_dir_imp_h  = copy_hash_tbl(s, DIR_TYPE,  PROJECT_NAME_RULE, FNTOLN);
    ln_to_fn_file_imp_h = copy_hash_tbl(s, FILE_TYPE, PROJECT_NAME_RULE, LNTOFN);
    ln_to_fn_dir_imp_h  = copy_hash_tbl(s, DIR_TYPE,  PROJECT_NAME_RULE, LNTOFN);

    line_num       = s->get_line_num();
    view_flag      = s->get_view_flag();

    leading_slash_in_child_phy_path = 0;

    flag           = s->get_flag();
    sub_proj_flag  = s->get_sub_proj_flag();
    direction      = s->get_direction();
    file_type      = s->get_file_type();

    proj_rule      = db_new(wkIdxRule, (s->get_project_rule()));
    logic_rule     = db_new(wkIdxRule, (s->get_logic_rule()));

    child_real_right_way = s->is_child_real_right_way();
    map_phy_path         = s->get_map_phy_path();

    copy_phy_filters(s, 0);

    if (s->get_member())
        member_filter = db_new(projFilter, (s->get_member()->get_filter()));
    else
        member_filter = 0;

    // projNode_filter which is done in proj_rule_head->update_projnode_filter("", 0);

    // ret_dir is working buffer which is used in report.C

    this_is_target = 0;
    para_rule_type = 0;
    if (s->is_read_only())
        read_only_flag = R_FLAG;
    else if (s->is_writable())
        read_only_flag = W_FLAG;
    else
        read_only_flag = NO_RW_FLAG;

    num_created_path = 0;
    pth = 0;

    writepipe       = 0;
    readpipe        = 0;
    root_node       = 0;
    projNode_filter = 0;

    genString r_path;
    project_convert_filename(s->get_org_dir_name(), r_path);
    root_pdf_tree_arr_index = s->root_pdf_tree_arr_index;
    leaf_pdf_tree_arr_index = s->leaf_pdf_tree_arr_index;
}
