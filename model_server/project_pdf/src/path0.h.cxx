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
// path0.h.C
// implementation to verify pdf when paraset tries to load it.

#include <cLibraryFunctions.h>
#include <msg.h>
#include <machdep.h>
#include <customize.h>

#ifndef _objOper_h
#include <objOper.h>
#endif

#ifndef _path0_h
#include <path0.h>
#endif

#ifndef _messages_h
#include <messages.h>
#endif

#include <psetmem.h>

static int fatal_error = 0;
static int quit_flag = 0;

static char const *file_name = 0;

static genString might_err;
static int       might_count = 0;

static void local_msg(char const *context, char const *err)
{
    Initialize(local_msg);

    if (file_name)
    {
	msg("ERROR: In file $1\n[$2]") << file_name << eoarg << err << eom;
        file_name = 0;
    }
    else
	msg("ERROR: [$1]") << err << eom;
}

// Class Dir_Name_Info begins
Dir_Name_Info::Dir_Name_Info(char const *n)
{
    Initialize(Dir_Name_Info::Dir_Name_Info);

    name = n;
    pf   = 0;
    next = 0;
    if (!need_expand_dir(name))
        type = CONST_STRING;
    else if (strcmp(name,"*") == 0)
        type = MATCH_ALL;
    else
    {
        type = MATCH_PATERN;
        pf   = db_new(projFilter, (name));
    }
}

Dir_Name_Info::~Dir_Name_Info()
{
    Initialize(Dir_Name_Info::~Dir_Name_Info);

    if (next)
        delete next;
    if (pf)
        delete pf;
}

void Dir_Name_Info::set_next(Dir_Name_Info *n)
{
    Initialize(Dir_Name_Info::set_next);

    next = n;
}

Dir_Name_Info *Dir_Name_Info::get_next()
{
    Initialize(Dir_Name_Info::get_next);

    return next;
}

char *Dir_Name_Info::get_name()
{
    Initialize(Dir_Name_Info::get_name);

    return (char *)name.str();  // XXX: constness
}

DIR_NAME_TYPE Dir_Name_Info::get_type()
{
    Initialize(Dir_Name_Info::get_type);

    return type;
}

projFilter *Dir_Name_Info::get_pf()
{
    Initialize(Dir_Name_Info::get_pf);

    return pf;
}

void Dir_Name_Info::get_full_path_name(genString &fn)
// return the full path of a physical path
// must be called from the head node
//
// fn contain the returned path
{
    Initialize(Dir_Name_Info::get_full_path_name);

    fn = 0;
    Dir_Name_Info *cur = this;
    while(cur)
    {
        fn += "/";
        fn += cur->get_name();
        cur = cur->get_next();
    }
}


// Class Dir_Name_Info ends

// Class Verify_Rule begins
Verify_Rule::Verify_Rule(char *projn, char *phyn, char *logn, int line_no, int ftp, char const *fr, projMap *projm)
{
    Initialize(Verify_Rule::Verify_Rule);

    line_num      = line_no;
    file_type     = ftp;

    proj_name_num = count_level(projn);
    proj_name     = set_name(projn);

    phy_name_num  = count_level(phyn);
    phy_name      = set_name(phyn);

    log_name_num = count_level(logn);
    log_name     = set_name(logn);

    family_relation = fr;

    pm = projm;

    const_flag = 1;
    const_level = 0;
    Dir_Name_Info *cur = phy_name;
    while(cur)
    {
        if (cur->get_type() != CONST_STRING)
        {
            const_flag = 0;
            break;
        }
        const_level++;
        cur = cur->get_next();
    }
}

Verify_Rule::~Verify_Rule()
{
    Initialize(Verify_Rule::~Verify_Rule);

    delete proj_name;
    delete phy_name;
    delete log_name;
}

Dir_Name_Info *Verify_Rule::set_name(char *n)
{
    Initialize(Verify_Rule::set_name);

    Dir_Name_Info *h    = 0;
    Dir_Name_Info *last = 0;
    Dir_Name_Info *cur  = 0;

    char *p;
    char *q;

    if (leading_char(n,'/'))
        p = n+1;
    else
        p = n;

    while(*p)
    {
        q = strchr(p,'/');
        if (q)
            *q = '\0';
        cur = db_new(Dir_Name_Info, (p));

        if (h)
            last->set_next(cur);
        else
            h = cur;

        last = cur;

        if (q)
        {
            *q = '/';
            p = q+1;
        }
        else
            break;
    }

    if (!h)
        return db_new(Dir_Name_Info, (""));
    return h;
}

Dir_Name_Info *Verify_Rule::get_proj_name()
{
    Initialize(Verify_Rule::get_proj_name);

    return proj_name;
}

Dir_Name_Info *Verify_Rule::get_phy_name()
{
    Initialize(Verify_Rule::get_phy_name);

    return phy_name;
}

Dir_Name_Info *Verify_Rule::get_log_name()
{
    Initialize(Verify_Rule::get_log_name);

    return log_name;
}

int Verify_Rule::get_line_num()
{
    Initialize(Verify_Rule::get_line_num);

    return line_num;
}

int Verify_Rule::get_file_type()
{
    Initialize(Verify_Rule::get_file_type);

    return file_type;
}

int Verify_Rule::get_proj_name_num()
{
    Initialize(Verify_Rule::get_proj_name_num);

    return proj_name_num;
}

int Verify_Rule::get_phy_name_num()
{
    Initialize(Verify_Rule::get_phy_name_num);

    return phy_name_num;
}

int Verify_Rule::get_log_name_num()
{
    Initialize(Verify_Rule::get_log_name_num);

    return log_name_num;
}

char const *Verify_Rule::get_family_relation()
{
    Initialize(Verify_Rule::get_family_relation);

    return family_relation.str();
}

projMap *Verify_Rule::get_pm()
{
    Initialize(Verify_Rule::get_pm);

    return pm;
}

char Verify_Rule::get_const_flag()
{
    Initialize(Verify_Rule::get_const_flag);

    return const_flag;
}

int Verify_Rule::get_const_level()
{
    Initialize(Verify_Rule::get_const_level);

    return const_level;
}

// Class Verify_Rule ends

// VERIFY SUPERSEDE RULE BEGINNING
typedef struct
{
    Verify_Rule *vr;
} VR;
genArr(VR);

static genArrOf(VR) verify_rule_array;

static void log_one_rule_def(char *proj_name, char *phy_name, char *log_name, int line_num, int file_type, char const *fr, projMap *pm)
{
    Initialize(log_one_rule_def);

    VR *p = verify_rule_array.grow();
    p->vr = db_new(Verify_Rule, (proj_name, phy_name, log_name, line_num, file_type, fr, pm));
}

static void collect_proj_log_rules(projMap *pm, char const *proj_name, char const *phy_name, char const *log_name, char const *par_fr, int cur_fr_idx)
{
    Initialize(collect_proj_log_rules);

    if (pm->get_direction() == TWO_WAY)
    {
        genString phyn0  = pm->get_dir_name();
        remove_back_slash_and_parentheses((char *)phyn0.str());
        genString projn0;
        genString logn0;
        pm->get_defined_name(projn0, logn0);

        if (projn0.length() == 0)
        {
            projn0 = pm->get_org_project_name();
            if (projn0.length() == 0)
                projn0 = phyn0.str();
        }

        if (logn0.length() == 0)
        {
            logn0 = pm->get_org_logic_name();
            if (logn0.length() == 0)
                logn0 = phyn0.str();
        }

        genString cur_proj_name;
        combine_dir_and_sub_dir(proj_name, projn0, cur_proj_name);
        genString cur_phy_name;
        combine_dir_and_sub_dir(phy_name, phyn0, cur_phy_name);
        genString cur_log_name = "";
        if (logn0.length())
            combine_dir_and_sub_dir(log_name, logn0, cur_log_name);

        projMap *child = pm->get_child_project();

        int file_type;
        if (child)
            file_type = DIR_TYPE;
        else
            file_type = FILE_TYPE;

        genString cur_family_relation;
        cur_family_relation.printf("%s%d.", par_fr, cur_fr_idx);

        log_one_rule_def(cur_proj_name, cur_phy_name, cur_log_name, pm->get_line_num(), file_type, cur_family_relation, pm);

        // go to child
        if (child)
            collect_proj_log_rules(child, cur_proj_name, cur_phy_name, cur_log_name, cur_family_relation, 0);
    }
    // go to next
    projMap *par  = pm->get_parent_project();
    projMap *next = pm->get_next_project();
    if (par && next)
        collect_proj_log_rules(next, proj_name, phy_name, log_name,  par_fr, cur_fr_idx+1);
}

static void collect_rules(projMap *pm, char *proj_name, char *phy_name, char *log_name, char const *par_fr, int cur_fr_idx)
{
    Initialize(collect_rules);

    if (pm->get_direction() == TWO_WAY)
    {
        genString phyn0  = pm->get_dir_name();
        remove_back_slash_and_parentheses((char *)phyn0.str());

        genString cur_phy_name;
        combine_dir_and_sub_dir(phy_name, phyn0, cur_phy_name);

        projMap *child = pm->get_child_project();

        int file_type;
        if (child)
            file_type = DIR_TYPE;
        else
            file_type = FILE_TYPE;

        genString cur_family_relation;
        cur_family_relation.printf("%s%d.", par_fr, cur_fr_idx);

        log_one_rule_def(proj_name, cur_phy_name, log_name, pm->get_line_num(), file_type, cur_family_relation, pm);

        // go to child
        if (child)
            collect_rules(child, proj_name, cur_phy_name, log_name, cur_family_relation, 0);
    }

    // go to next
    projMap *next = pm->get_next_project();
    if (next)
        collect_rules(next, proj_name, phy_name, log_name, par_fr, cur_fr_idx+1);
}

static void free_rules_array()
{
    Initialize(free_rules_array);

    for (int i=0; i<verify_rule_array.size(); i++)
    {
        VR *p = verify_rule_array[i];
        delete p->vr;
    }
}

static int name_cmp(int i, int j, int type)
{
    Initialize(name_cmp);

    VR *p = verify_rule_array[i];
    VR *q = verify_rule_array[j];

    int status;
    if (type == 0)
        status = p->vr->get_proj_name_num() - q->vr->get_proj_name_num();
    else if (type == 1)
        status = p->vr->get_phy_name_num() - q->vr->get_phy_name_num();
    else
        status = p->vr->get_log_name_num() - q->vr->get_log_name_num();

    if (status)
        return status;

    status = p->vr->get_file_type() - q->vr->get_file_type();
    if (status)
        return status;

    if (type == 1)
    {
        status = p->vr->get_const_flag() - q->vr->get_const_flag();
        if (status > 0) // p is const q is match pattern
            return -1;
        if (status < 0) // q is const p is match pattern
            return 1;
    }

    Dir_Name_Info *d0;
    Dir_Name_Info *d1;
    if (type == 0)
    {
        d0 = p->vr->get_proj_name();
        d1 = q->vr->get_proj_name();
    }
    else if (type == 1)
    {
        d0 = p->vr->get_phy_name();
        d1 = q->vr->get_phy_name();
    }
    else
    {
        d0 = p->vr->get_log_name();
        d1 = q->vr->get_log_name();
    }

    while(d0)
    {
        status = strcmp(d0->get_name(),d1->get_name());
        if (status)
            return status;
        d0 = d0->get_next();
        d1 = d1->get_next();
    }
    return 0;
}

static int proj_name_cmp(const void *i, const void *j)
{
    Initialize(proj_name_cmp);

    return name_cmp(*(int *)i,*(int *)j,0);
}

static int phy_name_cmp(const void *i, const void *j)
{
    Initialize(phy_name_cmp);

    return name_cmp(*(int *)i,*(int *)j,1);
}

static int log_name_cmp(const void *i, const void *j)
{
    Initialize(log_name_cmp);

    return name_cmp(*(int *)i,*(int *)j,2);
}

// type == 0 for project  name
// type == 1 for physical name
// type == 2 for logic name
static void output_duprule_warning(int line_num0, int line_num1, int type)
{
    Initialize(output_duprule_warning);

    fatal_error = 1;

    genString name;
    if (type == 0)
        name = "project";
    else if (type == 1)
        name = "physical";
    else
        name = "logical";

    genString err;
    err.printf("The %s rule in line %d will be superseded by the definition in line %d\n",
               name.str(),line_num1,line_num0);
    local_msg(C_PDFCONFLICT, err);
}

enum CPM_RULE_TYPE
{
    PATTERN_INCLUDED,
    PATTERN_INCLUDING,
    PATTERN_EQUAL,
    PATTERN_NOT_EQUAL,
    PATTERN_COMPLEX
};

static CPM_RULE_TYPE brute_force_cmp_one_piece(char const *s0, char const *s1)
{
    Initialize(brute_force_cmp_one_piece);
    while(*s0 == *s1)
    {
        if (*s0 == '[')
            break;
        s0++;
        s1++;
    }

    if (strchr("*?[",*s0) == 0 || strchr("*?[",*s1) == 0)
        return PATTERN_NOT_EQUAL; // must be not equal

    int type0 = need_expand_dir(s0);
    int type1 = need_expand_dir(s1);

    if (type0 == 0 || type1 == 0) // One is constant
        return PATTERN_NOT_EQUAL; // must be not equal

    return PATTERN_COMPLEX; // too complicate
}

static CPM_RULE_TYPE brute_force_cmp(char *s0, char *s1)
{
    Initialize(brute_force_cmp);

    while(1)
    {
        char *p0 = strchr(s0,'.');
        char *p1 = strchr(s1,'.');

        if (p0 && !p1 || !p0 && p1)
            return PATTERN_NOT_EQUAL;

        if (p0)
            *p0 = '\0';
        if (p1)
            *p1 = '\0';

        int status = brute_force_cmp_one_piece(s0, s1);
        if (p0)
            *p0 = '.';
        if (p1)
            *p1 = '.';

        if (status == PATTERN_NOT_EQUAL)
            return PATTERN_NOT_EQUAL;

        if (!p0 && !p1)
            return PATTERN_COMPLEX;

        s0 = p0+1;
        s1 = p1+1;

        if (*s0 == '\0' && *s1 || *s0 && *s1 == '\0')
            return PATTERN_NOT_EQUAL;

        if (*s0 == '\0' && *s1 == '\0')
             return PATTERN_COMPLEX;
   }
}

static bool contain_constant(char const *p)
// check if the string p contains any character not in "*?[]"
{
    Initialize(contain_constant);

    int nest = 0;
    while(*p)
    {
        if (*p == '\\')
        {
            p++;
            if (*p)
                p++;
            else
                return FALSE;
        }
        else
        {
            if (*p != '[')
                nest++;
            else if (*p != ']')
                nest--;
            else if (!nest && *p != '*' && *p != '?')
                return TRUE;

            p++;
        }
    }

    return FALSE;
}

static CPM_RULE_TYPE cmp_two_patterns(Dir_Name_Info *s0, Dir_Name_Info *s1)
// compare definition between two slashes from two rules.
// The rules could be project name, physical name, or logical name.
{
    Initialize(cmp_two_patterns);

    if (s0->get_type() == CONST_STRING || s1->get_type() == CONST_STRING)
    {
        if (s0->get_type() == CONST_STRING && s1->get_type() == CONST_STRING)
        {
            if (strcmp(s0->get_name(), s1->get_name()) == 0)
                return PATTERN_EQUAL;
            else
                return PATTERN_NOT_EQUAL;
        }
        else
        {
            char       *name;
            projFilter *pf;
            if (s0->get_type() == CONST_STRING)
            {
                name = s0->get_name();
                pf   = s1->get_pf();
                if (pf->file_name_match(name,FILE_TYPE,MATCH_THIS_ONE))
                    return PATTERN_INCLUDED;
                else
                    return PATTERN_NOT_EQUAL;
            }
            else
            {
                name = s1->get_name();
                pf   = s0->get_pf();
                if (pf->file_name_match(name,FILE_TYPE,MATCH_THIS_ONE))
                    return PATTERN_INCLUDING;
                else
                    return PATTERN_NOT_EQUAL;
            }
        }
    }

    if (strcmp(s0->get_name(), s1->get_name()) == 0)
        return PATTERN_EQUAL;

    char const *ssp;
    if (ssp = strstr(s0->get_name(), s1->get_name()))
    {
        if (contain_constant(ssp+strlen(s1->get_name())))
            return PATTERN_NOT_EQUAL;
        return PATTERN_INCLUDING;
    }
    else if (ssp = strstr(s1->get_name(), s0->get_name()))
    {
        if (contain_constant(ssp+strlen(s0->get_name())))
            return PATTERN_NOT_EQUAL;
        return PATTERN_INCLUDED;
    }

    if (s0->get_type() == MATCH_ALL)
        return PATTERN_INCLUDING;
    if (s1->get_type() == MATCH_ALL)
        return PATTERN_INCLUDED;

    return brute_force_cmp(s0->get_name(), s1->get_name());
}

static int same_file_selector(projMap *p, projMap *q)
{
    Initialize(same_file_selector);

    int idx = 1;
    while(1)
    {
        wkPtnRule *p0 = p->get_phy_rule(idx);
        wkPtnRule *q0 = q->get_phy_rule(idx);

        if (!p0 && !q0)
            return 1; // ? IDENTICAL

        if (!p0 && q0 || p0 && !q0)
            return 0;

        char pt = p->get_phy_rule_type(idx);
        char qt = q->get_phy_rule_type(idx);
        if (pt != qt)
            return 0;

        int pf = p->get_phy_file_type(idx);
        int qf = q->get_phy_file_type(idx);
        if (pf != qf)
            return 0;

        char const *pp = p0->get_org_ptn();
        char const *qp = q0->get_org_ptn();

        if (!pp && qp || pp && !qp || pp && qp && strcmp(pp,qp))
            return 0;

        idx++;
    }
}

static void remove_bottom_level(genString &p)
{
    Initialize(remove_bottom_level);

    if (!p.length())
        return;

    char *q = (char *)p.str() + p.length() - 1;
    q--;
    for (int i=0;i<p.length()-1;i++)
    {
        if (*q == '.')
        {
            *q = '\0';
            genString s = p.str();
            p = s.str();
            return;
        }
        q--;
    }

    p = "";
}

static int not_same_level(char const *frs0, char const *frs1)
{
    Initialize(not_same_level);

    if (!frs0 || !frs1)
        return 1;

    genString p0 = frs0;
    genString p1 = frs1;
    remove_bottom_level(p0);
    remove_bottom_level(p1);

    if (strcmp(p0.str(),p1.str()))
        return 1;

    return 0;
}

static int compare_two_rule_const_part(Verify_Rule *p, Verify_Rule *q, int type, int const_level)
{
    Initialize(compare_two_rule_const_part);

    Dir_Name_Info *s0;
    Dir_Name_Info *s1;
    if (type == 1)
    {
        s0 = p->get_phy_name();
        s1 = q->get_phy_name();
    }

    for (int i=0;i<const_level;i++)
    {
        int status = strcmp(s0->get_name(),s1->get_name());
        if (status)
            return status;

        s0 = s0->get_next();
        s1 = s1->get_next();
    }
    return 0; // equal
}

static int rules_array_identical(genArrOf(phy_rule_entry) &p, genArrOf(phy_rule_entry) &q)
{
    Initialize(rules_array_identical);

    int result = 0;

    if(p.size() != q.size())
        return result;

    int size = p.size();
    if(size < 2)
        return result;

    result = 1;
    for (int i = 1; i < size && result != 0; i++)
    {
        phy_rule_entry *p_rule = p[i];
        phy_rule_entry *q_rule = q[i];

        if (p_rule->rule_type != q_rule->rule_type)
            result = 0;
        else
        {
            int type = p_rule->rule_type;
            if (type == LINK_MAP_PHY_PATH || type == REALPATH_MAP_PHY_PATH 
		|| type == SEARCH_PHY_PATH || TCL_BOOLEAN_EVAL || EQUAL_EVAL || NOT_EQUAL_EVAL )
            {
                char const *p_pattern = p_rule->phy_rule->get_ptn_def();
                char const *q_pattern = q_rule->phy_rule->get_ptn_def();
                if (p_pattern != NULL && q_pattern != NULL)
                {
                    genString p_str = p_pattern;
                    genString q_str = q_pattern;
                    remove_back_slash_and_parentheses((char *)p_str.str());
                    remove_back_slash_and_parentheses((char *)q_str.str());
                    if(strcmp(p_str, q_str))
                        result = 0;
                }
            }
        }
    }
    return result;
}

static int equal_rule(Verify_Rule *p, Verify_Rule *q)
{
    Initialize(equal_rule);

    if (p->get_file_type() != FILE_TYPE || q->get_file_type() != FILE_TYPE)
        return 0;

    Dir_Name_Info *s0 = p->get_phy_name();
    Dir_Name_Info *s1 = q->get_phy_name();
    while(s0)
    {
        if (strcmp(s0->get_name(),s1->get_name()))
            return 0;

        s0 = s0->get_next();
        s1 = s1->get_next();
        if (s0 && !s1 || !s0 && s1)
            return 0;
    }
    genArrOf(phy_rule_entry) *p_rules = p->get_pm()->get_multi_phy_rule();
    genArrOf(phy_rule_entry) *q_rules = q->get_pm()->get_multi_phy_rule();
    return rules_array_identical(*p_rules, *q_rules);

//    return 1;
}

static int script_proj_and_read_only_proj(VR *p, VR *q)
// If p belongs to a script proj and q belongs to a read-only proj, return 1;
// If q belongs to a script proj and p belongs to a read-only proj, return 1;
// If p or q is a filter project (branch), return 1;
// Otherwise; it return 0;
{
    Initialize(script_proj_and_read_only_proj);

    projMap *pm0 = p->vr->get_pm();
    projMap *pm1 = q->vr->get_pm();
    while(pm0->get_parent_project())
        pm0 = pm0->get_parent_project();
    while(pm1->get_parent_project())
        pm1 = pm1->get_parent_project();
    if (pm0->is_script() && pm1->is_read_only() ||
        pm1->is_script() && pm0->is_read_only())
        return 1;
    if (pm0->is_filter() || pm1->is_filter())
        return 1;
    return 0;
}

static int compare_two_rule(Verify_Rule *p, Verify_Rule *q, int type, char const */* fn */ )
// compare two rules
//
// p: is for one rule
// q: is for another rule
//    the rule saved in p is defined earlier than the rule saved in q.
//
// type == 0 for project  name
// type == 1 for physical name
// type == 2 for logic  name
{
    Initialize(compare_two_rule);

    if (type == 2 && not_same_level(p->get_family_relation(),q->get_family_relation()))
        return 1;

    if (p->get_file_type() != q->get_file_type())
        return 1;
    Dir_Name_Info *s0;
    Dir_Name_Info *s1;
    if (type == 0)
    {
        if (p->get_proj_name_num() != q->get_proj_name_num())
            return 1;
        s0 = p->get_proj_name();
        s1 = q->get_proj_name();
    }
    else if (type == 1)
    {
        if (p->get_phy_name_num() != q->get_phy_name_num())
            return 1;
        s0 = p->get_phy_name();
        s1 = q->get_phy_name();
    }
    else
    {
        if (p->get_log_name_num() != q->get_log_name_num())
            return 1;
        s0 = p->get_log_name();
        s1 = q->get_log_name();
    }

    char const *n0 = 0;
    char const *n1 = 0;
    char including_flag = 0;
    CPM_RULE_TYPE status;
    while(s0)
    {
        if (strlen(s0->get_name()) == 0 || strlen(s1->get_name()) == 0)
            status = PATTERN_NOT_EQUAL;
        else
            status = cmp_two_patterns(s0,s1);

        if (status == PATTERN_NOT_EQUAL)
            return 0;
        else if  (status == PATTERN_INCLUDED)
        {
            if (including_flag)
                break;
            return 0;
        }
        else if (status == PATTERN_COMPLEX)
        {
            n0 = s0->get_name();
            n1 = s1->get_name();
        }
        else if (status == PATTERN_INCLUDING)
            including_flag = 1;

        s0 = s0->get_next();
        s1 = s1->get_next();
    }

    if (n0)
    {
        might_err.printf(
     "The pattern \"%s\" in line %d and the pattern \"%s\" in line %d may conflict.\n",
                  n0,p->get_line_num(),n1,q->get_line_num());
        might_count++;
    }
    else if (including_flag)
    {
        genString phy_path_name;
        q->get_phy_name()->get_full_path_name(phy_path_name);
        int status0 = need_expand_dir(phy_path_name);

        if (!status0)
        {
            projMap *par = p->get_pm();
            while(par->get_parent_project())
                par = par->get_parent_project();
            genString phy_name = par->get_dir_name();
            remove_back_slash_and_parentheses((char *)phy_name.str());
            genString ss;
            par->get_matched_name(phy_name, ss, 0);
            genString proj_name;
            proj_name.printf("/%s",ss.str());
            genString ln;
            if (p->get_file_type() != FILE_TYPE)
                par->fn_to_ln(phy_path_name, proj_name, DIR_TYPE,  ln);
            else
                par->fn_to_ln(phy_path_name, proj_name, FILE_TYPE, ln);
            if (ln.length())
                status0 = 1;
        }
        if (status0)
        {
            if (type != 1 && p->get_file_type() != FILE_TYPE ||
                type == 1 && p->get_file_type() == FILE_TYPE)
                output_duprule_warning(p->get_line_num(), q->get_line_num(), type);
            else if (type != 0 && p->get_file_type() == FILE_TYPE) // for logical name in file level
            {
                // if it is parent <-> children relation, then it is ok
                char const *pfr = p->get_family_relation();
                char const *qfr = q->get_family_relation();
                if (strncmp(pfr,qfr,strlen(pfr)) && same_file_selector(p->get_pm(),q->get_pm()))
                {
                    genArrOf(phy_rule_entry) *p_rules = p->get_pm()->get_multi_phy_rule();
                    genArrOf(phy_rule_entry) *q_rules = q->get_pm()->get_multi_phy_rule();
                    if(rules_array_identical(*p_rules, *q_rules))
                        output_duprule_warning(p->get_line_num(), q->get_line_num(), type); // two cross-overed rules
                }
            }
        }

        return 1;
    }
    else if ((status == PATTERN_EQUAL) && (type != 1 || p->get_file_type() != DIR_TYPE))
    {
        genString err;
        char const *type_name;

        if (type == 0)
            type_name = "PROJECT";
        else if (type == 1)
            type_name = "PHYSICAL";
        else // type == 2 for logical name
            type_name = "LOGICAL";

        err.printf("The rules in the following lines are identical for %s name: %d %d\n",
                   type_name, p->get_line_num(), q->get_line_num());
        local_msg(NULL, err);

    }

    return 0;
}

static void obtain_size_of_same_file_type_and_same_level(int start, int *idx, int &const_size, int &match_size)
{
    Initialize(obtain_size_of_same_file_type_and_same_level);

    const_size = 0;
    match_size = 0;
    VR *p = verify_rule_array[idx[start]];
    int nam_num   = p->vr->get_phy_name_num();
    int file_type = p->vr->get_file_type();
    for (int i=start; i<verify_rule_array.size(); i++)
    {
        VR *q = verify_rule_array[idx[i]];

        if (nam_num != q->vr->get_phy_name_num() || file_type != q->vr->get_file_type())
            return;

        if (q->vr->get_const_flag())
            const_size++;
        else
            match_size++;
    }
}

static void check_const_phy_path(int start, int *idx, int const_size, int type, char const *fn)
// compare constant physical paths
//
// start: starting index into the sorted index array idx
// idx: an integer array that contains the sorted index into the verify_rule_array.
//      The sorted sequence depends on the "type".
// const_size: the number of entries started from "start" are constant physical paths
// type == 0 for project  name
//      == 1 for physical name
//      == 2 for logic  name
// fn: the pdf file name
{
    Initialize(check_const_phy_path);

    if (const_size <= 1)
        return;

    for (int i=start; i < start + const_size - 1; i++)
    {
        VR *p = verify_rule_array[idx[i]];
        VR *q = verify_rule_array[idx[i+1]];
        if (script_proj_and_read_only_proj(p, q))
            continue;
        if (p->vr->get_line_num() < q->vr->get_line_num())
            compare_two_rule(p->vr,q->vr,type,fn);
        else
            compare_two_rule(q->vr,p->vr,type,fn);
    }
}

static void check_const_to_match_phy_path(int start, int *idx, int *repeate_idx, int const_size, int match_size,
                                          int type, char const *fn)
// compare the constant physical path with the non-constant physical path
//
// start: starting index into the sorted index array idx
// idx: an integer array that contains the sorted index into the verify_rule_array.
//      The sorted sequence depends on the "type".
// repeate_idx: if two contiguous physical paths are files and are identical, then both entries in this array
//              repeate_idx will be set to 1
// const_size: the number of entries started from "start" are constant physical paths
// match_size: the number of entries started from "start+const_size" are non-constant physical paths
// type == 0 for project  name
//      == 1 for physical name
//      == 2 for logic  name
// fn: the pdf file name
{
    Initialize(check_const_to_match_phy_path);

    if (const_size == 0 || match_size == 0)
        return;

    int match_start = start+const_size;
    int match_end   = start+const_size+match_size;
    for (int i=start; i < start + const_size; i++)
    {
        VR *p = verify_rule_array[idx[i]];
        int new_match_start = match_start;

        int j;
        for (j=match_start;j<match_end;j++)
        {
            if (repeate_idx[j] && repeate_idx[j+1])
                new_match_start++;
            else
                break;
        }
        match_start = new_match_start;
        for (j=match_start;j<match_end;j++)
        {
//          OSapi_fprintf(stderr,"**check_const_to_match_phy_path i = %d j = %d\n",i,j);
//          OSapi_fflush(stderr);
            VR *q = verify_rule_array[idx[j]];
            if (script_proj_and_read_only_proj(p, q))
                continue;

            int status = compare_two_rule_const_part(p->vr,q->vr,type,q->vr->get_const_level());
            if (status > 0)
                new_match_start++;
            else if (status < 0)
                break;
            else
            {
                if (p->vr->get_line_num() < q->vr->get_line_num())
                    compare_two_rule(p->vr,q->vr,type,fn);
                else
                    compare_two_rule(q->vr,p->vr,type,fn);
            }
        }
        match_start = new_match_start;
    }
}

static int min_const_level(Verify_Rule *p, Verify_Rule *q)
{
    Initialize(min_const_level);

    int const_level = p->get_const_level();
    if (const_level > q->get_const_level())
        const_level = q->get_const_level();
    return const_level;
}

static void check_match_phy_path(int start, int *idx, int *repeate_idx, int match_size, int type, char const *fn)
// check the identical physical path.
//
// start: starting index into the sorted index array idx
// idx: an integer array that contains the sorted index into the verify_rule_array.
//      The sorted sequence depends on the "type".
// repeate_idx: if two contiguous physical paths are files and are identical, then both entries in this array
//              repeate_idx will be set to 1
// match_size: the number of entries started from "start+const_size" are non-constant physical paths
// type == 0 for project  name
//      == 1 for physical name
//      == 2 for logic  name
// fn: the pdf file name
{
    Initialize(check_match_phy_path);

    if (match_size <= 1)
        return;

    for (int i=start; i < start + match_size - 1; i++)
    {
        if (repeate_idx[i] && repeate_idx[i+1])
            continue;
        VR *p = verify_rule_array[idx[i]];
        for (int j=i+1;j<start+match_size;j++)
        {
//          OSapi_fprintf(stderr,"==check_match_phy_path i = %d j = %d\n",i,j);
//          OSapi_fflush(stderr);

            VR *q = verify_rule_array[idx[j]];
            if (script_proj_and_read_only_proj(p, q))
                continue;

            int const_level = min_const_level(p->vr,q->vr);

            int status = compare_two_rule_const_part(p->vr,q->vr,type,const_level);
            if (status < 0)
                break;
            else
            {
               if (p->vr->get_line_num() < q->vr->get_line_num())
                    compare_two_rule(p->vr,q->vr,type,fn);
                else
                    compare_two_rule(q->vr,p->vr,type,fn);
            }
        }
    }
}

static void go_thru_phy_rules(int *idx, int type, char const *fn)
// compare all physical rules
//
// idx: an integer array that contains the sorted index into the verify_rule_array.
//      The sorted sequence depends on the "type".
// type == 0 for project  name
//      == 1 for physical name
//      == 2 for logic  name
// fn: the pdf file name
{
    Initialize(go_thru_phy_rules);

    int *repeate_idx = (int *)calloc(sizeof(int), verify_rule_array.size());

    int i = 0;
    VR *p = verify_rule_array[idx[0]];
    genString err;
    char err_flag = 0;
    char num[10];
    for(i=0;i<verify_rule_array.size()-1;i++)
    {
        VR *q = verify_rule_array[idx[i+1]];
        if (script_proj_and_read_only_proj(p, q))
            continue;
        if (equal_rule(p->vr,q->vr))
        {
            repeate_idx[i] = 1;
            repeate_idx[i+1] = 1;
            if (!err_flag)
            {
                err_flag = 1;
                err.printf("The rules in the following lines are identical: %d",p->vr->get_line_num());
            }
            OSapi_sprintf(num," %d", q->vr->get_line_num());
            err += num;
        }
        else if (err_flag)
        {
            err += "\n";
            local_msg(NULL, err);
            err_flag = 0;
        }
        p = q;
    }

    if (err_flag)
    {
        err += "\n";
        local_msg(NULL, err);
    }

    i = 0;
    while(i<verify_rule_array.size())
    {
        int const_size;
        int match_size;

        obtain_size_of_same_file_type_and_same_level(i, idx, const_size, match_size);
        check_const_phy_path(i, idx, const_size, type, fn);
        check_const_to_match_phy_path(i, idx, repeate_idx, const_size, match_size, type, fn);
        check_match_phy_path(i+const_size, idx, repeate_idx, match_size, type, fn);
        i += const_size + match_size;
    }
    psetfree(repeate_idx);
}

static void go_thru_rules(int *idx, int type, char const *fn)
// compare the rules in the same type.
//
// idx: an integer array that contains the sorted index into the verify_rule_array.
//      The sorted sequence depends on the "type".
// type == 0 for project  name
//      == 1 for physical name
//      == 2 for logic  name
// fn: the pdf file name
{
    Initialize(go_thru_rules);

    if (type == 1)
    {
        go_thru_phy_rules(idx, type, fn);
        return;
    }

    for (int i=0; i<verify_rule_array.size(); i++)
    {
        VR *p = verify_rule_array[idx[i]];
        for (int j=i+1; j<verify_rule_array.size(); j++)
        {
            VR *q = verify_rule_array[idx[j]];
            if (script_proj_and_read_only_proj(p, q))
                continue;
            int status;
            if (p->vr->get_line_num() < q->vr->get_line_num())
                status = compare_two_rule(p->vr,q->vr,type,fn);
            else
                status = compare_two_rule(q->vr,p->vr,type,fn);
            if (status)
                break;
        }
    }
}

// type == 0 for project  name
// type == 1 for physical name
// type == 2 for logic    name
static void check_dup_rule(int type, char const *fn)
{
    Initialize(check_dup_rule);

    if (!verify_rule_array.size())
        return;
    int *idx = (int *)calloc(sizeof(int), verify_rule_array.size());
    for(int i=0;i<verify_rule_array.size();i++)
        idx[i] = i;

    if (type == 0)
        OSapi_qsort(idx,verify_rule_array.size(),sizeof(int),proj_name_cmp);
    else if (type == 1)
        OSapi_qsort(idx,verify_rule_array.size(),sizeof(int),phy_name_cmp);
    else
        OSapi_qsort(idx,verify_rule_array.size(),sizeof(int),log_name_cmp);

    go_thru_rules(idx, type, fn);
    psetfree(idx);
}

static void check_proj_log_names(projMap *cur, char const *fn)
{
    Initialize(check_proj_log_names);

    while(cur)
    {
        verify_rule_array.reset();
        collect_proj_log_rules(cur, "" , "", "", "", 0);
        check_dup_rule(0, fn);
        check_dup_rule(2, fn);

        free_rules_array();
        cur = cur->get_next_project();
    }
}

int check_supersede_rule(projMap *proj_rule_head, char const *fn)
{
    Initialize(check_supersede_rule);

    if (customize::getIntPref("DIS_internal.PDFVerify_Enabled") < 1)
        return 0;

    might_count = 0;

    fatal_error = 0;
    file_name = fn;

    time_t time_a;
    OSapi_time(&time_a);

    check_proj_log_names(proj_rule_head, fn);

    verify_rule_array.reset();

    collect_rules(proj_rule_head, "" , "", "", "", 0);

    check_dup_rule(1, fn);

    free_rules_array();

    DBG
    {
        time_t time_b;
        OSapi_time(&time_b);
	msg("check supersede rule = $1", normal_sev) << (time_b - time_a) << eom;
    }

    if (might_count)
    {
        msg("\nPDF status:\n$1\nSimilar conflicts exist $2 times\n\n") << might_err.str() << eoarg << might_count << eom;
        ;
    }

    if (!quit_flag)
        return 0;
    else
        return fatal_error;
}
// VERIFY SUPERSEDE RULE END

/*
$Log: path0.h.cxx  $
Revision 1.33 2003/03/14 16:35:03EST John Verne (jverne) 
- Do not assume time_t == long; use time_t
- Change some string literals to char* to keep aCC happy
Revision 1.32 2001/11/07 14:08:25EST ktrans 
Remove dormant project code, -verify_pdf option, and slow put w/ feedback.  See no_slow_put branch for details.
// Revision 1.16  1994/06/05  16:41:01  so
// 1. if bring a raw view from the system, and do manage->get the raw view is not unloaded. The got file will be
//     file_name<2> in set buffer.
// 2. Do not check the directory fro the rule if all its children has a leading slash
// 3. Don't chop the last '/' if it is the make rule
// 4. fix bugs 6808 and 6367
//
// Revision 1.15  1994/05/23  21:21:34  so
// Bug track: n/a
// fast projHeader::fn_to_ln
//
// Revision 1.13  1994/03/17  14:16:53  so
// Bug track: 6703
// fix bug 6703
//
// Revision 1.12  1994/03/15  12:19:00  so
// Bug track: 6176
// fix bug 6176
//
// Revision 1.11  1994/02/07  12:38:52  so
// Bug track: n/a
// change "folowing" to "following"
//
// Revision 1.10  1994/01/24  19:25:09  so
// Bug track: n/a
// Call msg_error right away when an error occurs. Don't append it to a genString.
//
// Revision 1.9  1994/01/21  22:24:32  so
// Bug track: 6064
// fix bug 6064
//
// Revision 1.8  1994/01/21  16:51:04  so
// Bug track: 6010, 6064
// fix bugs 6010 and 6064
//
// Revision 1.7  1994/01/12  22:52:50  kws
// Use psetmalloc
//
// Revision 1.6  1993/10/30  14:36:38  so
// Bug track: 5134
// 5134
//
// Revision 1.5  1993/10/30  13:22:55  so
// Bug track: 5134
// fix bug 5134
// pass smoke 8 tests
//
// Revision 1.4  1993/10/28  20:17:47  so
// Bug track: 4999, 5037, 5082
// fix bugs 4999, 5037, 5082
//
// Revision 1.3  1993/10/27  14:13:25  so
// Bug track: 5049 5050 5051
// fix bugs 5049, 5050, 5051
//
// Revision 1.2  1993/10/04  20:25:37  so
// Bug track: 4899
// fix bug 4899
//
// Revision 1.1  1993/09/20  14:39:24  so
// Initial revision
//
*/
