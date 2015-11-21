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
#include <pdf_scope.h>
#include <genString.h>
#include <genError.h>
#include <messages.h>
#include <systemMessages.h>
#include <cLibraryFunctions.h>
#include <msg.h>
#include <machdep.h>
#include <customize.h>
#include <psetmem.h>

#ifndef _NewPrompt_h
#include <NewPrompt.h>
#endif

#include <patherr.h>
#include <path1.h>
#include <projHeader.h>
#include <projList.h>
#include <path.h>

#ifdef TXT
#undef TXT
#endif

#define TXT(a) a

extern void gtPushButton_next_help_context(const char*);
extern int name_cmp(const void *a, const void *b );

static genArrOf(unit_name_entry) unit_name_heads;

extern "C" void save_unit(char *unit_name)
{
    Initialize(save_unit);
 
    unit_name_entry *p = unit_name_heads.grow();
    p->name = strdup(unit_name);
}

static genString scn;

static genArrOf(scope_name_entry) scope_name_heads;

void create_one_scope_name(int line_num, const char *scope_n,
                           int unit_name_count, const char **unit_name)
{
    Initialize(create_one_scope_name);

    if (scope_n == 0)
	scope_n = scn;

    scope_name_entry *q = scope_name_heads.grow();
    q->line_num      = line_num;
    q->scope_name    = strdup(scope_n);
    if (!unit_name_count)
        q->num_of_units  = unit_name_heads.size();
    else
        q->num_of_units  = unit_name_count;
    q->unit_log_name = (char **)psetmalloc(sizeof(char *) * q->num_of_units);

    for (int i=0;i<q->num_of_units;i++)
    {
        if (!unit_name_count)
        {
            unit_name_entry *p  = unit_name_heads[i];
            q->unit_log_name[i] = p->name;
        }
        else
            q->unit_log_name[i] = strdup(unit_name[i]);
    }
}

static void unit_name_cleanup(char **unit_log_name, int num_of_units)
{
    Initialize(unit_name_cleanup);

    for (int i=0;i<num_of_units;i++)
    {
        if (unit_log_name[i])
            free(unit_log_name[i]);
    }
    psetfree(unit_log_name);
}

static void scope_name_cleanup(scope_name_entry *p)
{
    Initialize(scope_name_cleanup);

    unit_name_cleanup(p->unit_log_name, p->num_of_units);
    free(p->scope_name);
    p->scope_name    = 0;
    p->unit_log_name = 0;
    p->num_of_units  = 0;
    p->line_num      = 0;
}

static void check_dup_scope_name()
{
    Initialize(check_dup_scope_name);

    if (scope_name_heads.size() <= 1)
        return;
    for (int i=0;i<scope_name_heads.size()-1;i++)
    {
        scope_name_entry *p = scope_name_heads[i];
        if (p->scope_name == 0)
            continue;
        for (int j=i+1;j<scope_name_heads.size();j++)
        {
            scope_name_entry *q = scope_name_heads[j];
            if (q->scope_name == 0)
                continue;
            if (strcmp(p->scope_name, q->scope_name) == 0)
            {
                create_one_path_parser_error(PATH_PARSE_DUP_SCOPE_NAME_WRN, q->scope_name, 0,
                                             q->line_num, p->line_num, 0);
                // cleanup
                scope_name_cleanup(q);
            }
            else if (p->num_of_units == q->num_of_units)
            {
                char diff_flag = 0;
                for (int k=0;k<p->num_of_units;k++)
                {
                    if (strcmp(p->unit_log_name[k],q->unit_log_name[k]))
                    {
                        diff_flag = 1;
                        break;
                    }
                }
                if (!diff_flag)
                {
                    create_one_path_parser_error(PATH_PARSE_DUP_SCOPE_NAME_WRN, q->scope_name, 0,
                                                 q->line_num, p->line_num, 1);
                    // cleanup
                    scope_name_cleanup(q);
                }
            }
        }
    }
}

static void fill_unit_name_wildcard(scope_name_entry *p, char **log_name, int top_proj_num)
{
    Initialize(fill_unit_name_wildcard);

    char **new_name = (char **)psetmalloc(sizeof(char *) * top_proj_num);
    char *occupy = (char *)calloc(top_proj_num, sizeof(char));
    int idx = 0;

    for (int i=0;i<p->num_of_units;i++)
    {
        if (p->unit_log_name[i])
        {
            WildPair *m = (WildPair *)db_new(WildPair, (p->unit_log_name[i]));
            int found = 0;
            for (int j=0;j<top_proj_num;j++)
            {
                int len = m->match_to_end(log_name[j]);
                if (len > 0)
                {
                    if (!occupy[j])
                        new_name[idx++] = strdup(log_name[j]);
                    found = 1;
                    occupy[j] = 1;  // can't break here; wildcard needs to match all
                }
            }
            if (!found)
                create_one_path_parser_error(PATH_PARSE_ILLEGAL_UNIT_NAME_WRN, p->scope_name,
                                             p->unit_log_name[i], p->line_num, 0, 0);
        }
    }

    unit_name_cleanup(p->unit_log_name, p->num_of_units);
    p->unit_log_name = new_name;
    p->num_of_units  = idx;
    OSapi_qsort(p->unit_log_name, p->num_of_units, sizeof(char *), name_cmp);

    free(occupy);
}

// pairs for logic names and project names -- BEGIN
// reload-pdf will be used by system administrater
// unit_scope will not work after reload_pdf if the logic names
//            and project names are changed.

typedef struct
{
    char *ln;
    char *pn;
} PAIR_LOGIC_PROJECT_NAMES;

genArr(PAIR_LOGIC_PROJECT_NAMES);

static genArrOf(PAIR_LOGIC_PROJECT_NAMES) pair_ln_pn_heads;

// pairs for logic names and project names -- END

static void check_dup_illegal_unit_name()
{
    Initialize(check_dup_illegal_unit_name);

    char **log_name;
    int  top_proj_num = 0;
    projNode *proj;
    int i;

    for (i=1; proj = projList::search_list->get_proj(i); i++)
        top_proj_num++;
    if (scope_name_heads.size())
        log_name = (char **)psetmalloc(sizeof(char *) *  top_proj_num);
    int idx = 0;
    for (i=1; proj = projList::search_list->get_proj(i); i++)
    {
        if (proj->get_map() == 0) // for thr control project
            continue;
        genString phy_name;
        genString ln;
        proj->ln_to_fn_imp(proj->get_ln(), phy_name, DIR_TYPE, 0, 1);
        proj->fn_to_ln(phy_name, ln, DIR_TYPE, 0, 1);
        if (scope_name_heads.size())
            log_name[idx++] = strdup(ln.str());

        // save project name and logic name
        PAIR_LOGIC_PROJECT_NAMES *p = pair_ln_pn_heads.grow();
        p->pn = OSapi_strdup(proj->get_ln());
        p->ln = OSapi_strdup(ln.str());
    }
    top_proj_num = idx;
    if (!scope_name_heads.size())
        return;

    OSapi_qsort(log_name, top_proj_num, sizeof(char *), name_cmp);
    idx = 0;
    for (i=1; i<top_proj_num; i++)
    {
        if (strcmp(log_name[i], log_name[idx]))
            log_name[++idx] = log_name[i];
        else
            free(log_name[i]);
    }
    top_proj_num = idx+1;

    for (i=0;i<scope_name_heads.size();i++)
    {
        scope_name_entry *p = scope_name_heads[i];
        fill_unit_name_wildcard(p, log_name, top_proj_num);
        if (p->scope_name == 0)
            continue;

        int last_err  = 0;
        int last_warn = 0;
        genString last_name = " ";
        int j;
        for (j=0;j<p->num_of_units;j++)
        {
            if (p->unit_log_name[j])
            {
                if (strcmp(last_name.str(), p->unit_log_name[j]) == 0)
                {
                    // same name
                    if (!last_err && !last_warn)
                    {
                        create_one_path_parser_error(PATH_PARSE_DUP_UNIT_NAME_WRN, p->scope_name,
                                                     p->unit_log_name[j], p->line_num, 0, 0);
                        last_warn = 1;
                    }
                    free(p->unit_log_name[j]);
                    p->unit_log_name[j] = 0;
                    continue;
                }
                last_name = p->unit_log_name[j];
            }
            else
            {
                last_err  = 0;
                last_warn = 0;
                last_name = "";
            }
        }

        int found = 0;
        for (j=0;j<p->num_of_units;j++)
        {
            if (p->unit_log_name[j])
            {
                found = 1;
                break;
            }
        }
        if (!found)
        {
            create_one_path_parser_error(PATH_PARSE_DUP_SCOPE_NAME_WRN, p->scope_name, 0,
                                         p->line_num, 0, 2);
            // cleanup
            scope_name_cleanup(p);
        }
    }

    for(i=0;i<top_proj_num;i++)
        free(log_name[i]);
    psetfree(log_name);
}


void check_scope_name()
{
    Initialize(check_scope_name);

    check_dup_scope_name();

    check_dup_illegal_unit_name();
}
