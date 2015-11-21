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
//  file pdf_flat.C
//    to flat pdf project

#include <cLibraryFunctions.h>
#include <customize.h>

#ifndef ISO_CPP_HEADERS
#include <memory.h>
#else /* ISO_CPP_HEADERS */
#include <memory>
using namespace std;
#endif /* ISO_CPP_HEADERS */

#include <machdep.h>

#include <sys/types.h>

#ifndef _WIN32
#include <pwd.h>
#endif

#ifndef _objOper_h
#include <objOper.h>
#endif

#ifndef _path0_h
#include <path0.h>
#endif

#ifndef _patherr_h
#include <patherr.h>
#endif

#ifndef _pdf_flat_h
#include <pdf_flat.h>
#endif

static void update_idx_matching_idx(int type, genString &idx, int matched_count,
                                    int phy_max_count, int line_num)
// update the matching index in the project name and the logic name
// the fisrt child level, it does not need to update the matching index hence, matched_count is 0.
//
// type = 0: for project rule
//      = 1: for logic rule
// idx: is the rule itself
// matched_count: is the total count from the first child project to its parent project.
// phy_max_count: is the number of matched patterns in the first physical rule if this is not a leaf (module)
//                and all other rules are ignored
//              : is the number of matched patterns in all physical rules if it is a leaf (module).
// line_num: the line number in the pdf where it is defined.
{
    Initialize(update_idx_matching_idx);

    char *p = idx;
    int first = 1;
    genString s;
    while(*p)
    {
        if (*p == '(' && (first || (!first && *(p-1) != '\\')))
        {
            char *q = strchr(p,')');
            *q = '\0';
            int a = OSapi_atoi(p+1);
            *q = ')';
            p = q+1;

            if (a > phy_max_count)
            {
                const char *name;
                if (type == 0)
                    name = "project";
                else
                    name = "logic";
                create_one_path_parser_error(PATH_PARSE_FLAT_BIG_IDX_ERR, name, 0, line_num, a, phy_max_count);
            }

            char buf[20];
            OSapi_sprintf(buf,"(%d)",a+matched_count);
            s += buf;
        }
        else
            s += *p++;

        first = 0;
    }
    idx = s.str();
}

static void flat_child_proj(projMap *cur, char *proj_n, char *fn, char *ln, int matched_count,
                            int direction_flag, int deep_level)
// flatten a project tree; when it reach a leaf, a new projmap is created
//
// cur: the current projmap
// proj_n: rule for project name to its parent project
// fn: rule for physical name to its parent project
// ln: rule for logic name to its parent project
// matched_count: is the total count from the first child project to its parent project.
//                The fisrt child level, it does not need to update the matching index;
//                Hence, matched_count is 0.//
// type = 0: for project rule
//      = 1: for logic rule
// idx: is the rule itself


// direction_flag: TWO_WAY (<->), LEFT_WAY (<-), RIGHT_WAY (->)
// deep_level: 1: if this is called by a "next" project, then it will not be called recursively for its
//                "next" project.
//             0: it will recursively call its "next" projects.
{
    Initialize(flat_child_proj);

    genString local_proj_n = cur->get_project_rule()->get_idx_def();
    genString local_ln     = cur->get_logic_rule()->get_idx_def();
    genString local_fn     = cur->get_phy_rule(0)->get_ptn_def();

    int phy_max_count = cur->get_phy_rule(0)->get_max_ptn_count();
    if (!cur->get_child_project())
        phy_max_count = cur->get_phy_match_count();
    else // issue warning for ignored selectors
    {
        if (cur->get_multi_phy_rule()->size() > 1 ||
            cur->get_file_type() & DIR_TYPE       ||
            cur->get_file_type() & LINK_DIR)
            create_one_path_parser_error(PATH_PARSE_IGNORE_SELECTOR_WRN, 0, 0, cur->get_line_num(), 0, 0);
    }
    update_idx_matching_idx(0,local_proj_n, matched_count, phy_max_count, cur->get_line_num());
    update_idx_matching_idx(1,local_ln,     matched_count, phy_max_count, cur->get_line_num());

    genString new_proj_n;
    genString new_fn;
    genString new_ln;

    if (proj_n)
        new_proj_n.printf("%s/%s", proj_n, local_proj_n.str());
    else
        new_proj_n = local_proj_n.str();
    if (fn && !leading_char(local_fn,'/'))
    {
	if (cur->is_exclude_proj())
	    new_fn.printf("^%s/%s", fn, local_fn.str());
	else
	    new_fn.printf("%s/%s", fn, local_fn.str());
    }    
    else
    {
        if (cur->is_exclude_proj())
            new_fn.printf("^%s", local_fn.str());
        else
            new_fn = local_fn.str();
    }

    if (ln && !leading_char(local_ln,'/'))
        new_ln.printf("%s/%s", ln, local_ln.str());
    else
        new_ln = local_ln.str();

    int new_direction_flag = direction_flag;
    if (cur->get_direction() != TWO_WAY)
    {
        new_direction_flag = cur->get_direction();
        if (direction_flag == LEFT_WAY  && new_direction_flag == RIGHT_WAY ||
            direction_flag == RIGHT_WAY && new_direction_flag == LEFT_WAY)
            create_one_path_parser_error(PATH_PARSE_UNMATCH_DIRECTION_ERR, 0, 0, cur->get_line_num(), 0, 0);
    }

    projMap *child = cur->get_child_project();
    if (child)
        flat_child_proj(child, new_proj_n, new_fn, new_ln, matched_count+phy_max_count,
                        new_direction_flag, 0);
    else
    {
        projMap *n = db_new(projMap,(MEMBER, NO_IMPLICIT_DIR_PROJ, new_direction_flag,
                                     new_proj_n, new_fn, new_ln,
                                     SINGLE_CHILD, 0));
        n->copy_phy_filters(cur, 1); // only keep the filter of the leaf
        n->set_file_type(cur->get_file_type());
        pdf_flat_add_proj_head(n);
    }
    if (deep_level)
        return; // prevent going to the same "next" more than once

    cur = cur->get_next_project();
    while(cur)
    {
        flat_child_proj(cur, proj_n, fn, ln, matched_count, direction_flag, 1);
        cur = cur->get_next_project();
    }
}

static projMap *flat_one_proj(projMap *cur)
// flatten one root project if it is a script project
//
// cur is the root project
{
    Initialize(flat_one_proj);

    projMap *ret = db_new(projMap, (cur));
    projMap *child = cur->get_child_project();
    if (child)
    {
        pdf_flat_init_proj_head();
        flat_child_proj(child, 0, 0, 0, 0, TWO_WAY, 0);

        projMap *last = 0;
        // maintain the link
        for(int i=0;i<pdf_flat_size_of_proj_head();i++)
        {
            projMap *nc = pdf_flat_proj_head_by_idx(i);
            if (!last)
                ret->set_child_project(nc); // the first child
            else
                last->set_next_project(nc);
            last = nc;
        }
    }

    int tp = cur->get_pmod_type();
    ret->set_pmod_type(tp);
    if (tp != PMOD_NO && tp != PMOD_UNKNOWN) {
	pmod_projs_delete (cur);
	pmod_projs_insert (ret);
    }

    delete cur;
    return ret;
}

projMap *pdf_flat_proj(projMap *cur)
// the main entry to flatten projects for a pdf file
//
// cur is the projmap to be flattened
{
    Initialize(pdf_flat_proj);

    return flat_one_proj(cur);
}

/*
$Log: pdf_flat.cxx  $
Revision 1.11 2000/07/10 23:08:16EDT ktrans 
mainline merge from Visual C++ 6/ISO (extensionless) standard header files
*/
