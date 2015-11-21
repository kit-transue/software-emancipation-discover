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
// path.h./C/ implementation for class  projMap - for pdf definitions
//   and class wkPtnRule  for physical name
//   and class wkIdxRule  for logical and project name

#include <cLibraryFunctions.h>
#include <msg.h>
#include <machdep.h>
#include <customize.h>
#include <psetmem.h>

#include <objOper.h>
#include <path1.h>
#include <path.h>
#include <patherr.h>
#include <path0.h>
#include <pathor.h>
#include <pathcheck.h>
#include <path_l.h>
#include <messages.h>
#include <fileCache.h>
#include <vpopen.h>
#include <genTmpfile.h>
#include <feedback.h>
#include <pdf_flat.h>
#include <pdf_scope.h>
#include <pdf_selector.h>
#include <driver_mode.h>
#include <tcl.h>
#include <shell_calls.h>
#include <genArrCharPtr.h>
#include <mpb.h>
#include <proj_save.h>

static int PDF_DebugLevel = customize::getIntPref("DIS_internal.PDF_DebugLevel");


extern "C" FILE *yy_path_in;
//extern "C" void  aset_driver_exit(int);
//extern "C" void dbg_map_info(projMap *pm);
//extern int get_option_genStrings(char const *, genString *, int );
//extern char const *obtain_global_psetcfg_dir();
//extern void verify_load_pdf(char const *fn);
extern int obtain_path_parse_line_num(char const *proj_name, char const *phy_name, char const *logic_name, char new_lineno_flag);
extern int get_tran_flag ();

static void log_root_path(char const *path);
static bool phy_path_with_leading_slash(wkPtnRule *rule);

static int obtain_ptn_match_cnt(wkPtnDef *cur);
static void add_top_exc_proj_heads(projMap *ppm, char const *name);

init_relational (projMap,appTree);

//boris for file conversion hache table
int proj_path_report_conversion (char const *, genString&);
int proj_path_convert (char const *, genString&);

//char const *customize_configurator_system();
char const *customize_default_project_definition();

static char parse_is_being_executed = 0;

//Variables to track "/DISCOVER-Subsystems" project and ignore it.
static char const *subsystem_lname = "/DISCOVER-Subsystems";
static bool inside_subsystem_project = FALSE;
static int subsystem_proj_level = -1;

void set_parse_pdf_is_being_executed(int val)
{
    parse_is_being_executed = val;
}
 

static char chosen_rule = LOGIC_NAME_RULE;
static int  retreat_len = 0;

// ORKING BUFFER FOR ERROR GENERATOR
// int       projMap::path_complete_full_len;
// int       projMap::path_tran_error_code;
// genString projMap::path_tran_rule;
static int       path_complete_full_len;
static int       path_tran_error_code;
static genString path_tran_rule;

// When fn_to_ln is OK for the logic name,
// it will call fn_to_ln for the proj_name
// Don't log the error info.
// If it fails, change to error code 2: rejected by the proj name.
static char dont_log_err_info = 0;

void projMap::init_tran_vars()
{
    Initialize(projMap::init_tran_vars);

    if (dont_log_err_info)
        return;

    path_complete_full_len = 0;
    path_tran_error_code   = PATH_TRAN_NO_ERROR;
    path_tran_rule         = 0;
}

int projMap::report_path_complete_full_len()
{
    return path_complete_full_len;
}

char const *projMap::report_path_tran_rule()
{
    return path_tran_rule;
}

int projMap::report_path_tran_error_code()
{
    return path_tran_error_code;
}

void projMap::set_path_complete_full_len(int cur_len)
{
    if (! dont_log_err_info)
        path_complete_full_len = cur_len;
}

void projMap::set_path_tran_error_code(char err_code)
{
    if (! dont_log_err_info)
        path_tran_error_code = err_code;
}

void projMap::set_path_tran_rule(char const *rule)
{
    if (! dont_log_err_info)
        path_tran_rule = rule;
}

void update_the_using_rule(genString &s, char const *rule, projMap *cur)
// create the used rule
{
    genString proj_n = cur->get_org_project_name();
    if (proj_n.length() == 0)
        proj_n = "";
    if (cur->get_direction() == RIGHT_WAY)
        s.printf("%s%s%s:%s:R(line %d)", rule, RULE_SEPARATOR, 
                 proj_n.str(), cur->get_org_dir_name(), cur->get_line_num());
    else
        s.printf("%s%s%s:%s:(line %d)", rule, RULE_SEPARATOR, proj_n.str(), 
                 cur->get_org_dir_name(), cur->get_line_num());
}

static void update_the_using_rule_from_bottom(genString &s, 
                                              char const *rule, projMap *cur)
// create the used rule reversely
{
    genString proj_n = cur->get_org_project_name();
    if (proj_n.length() == 0)
        proj_n = "";
    if (cur->get_direction() == RIGHT_WAY)
        s.printf("%s%s:%s:R(line %d)%s", RULE_SEPARATOR, proj_n.str(), 
                 cur->get_org_dir_name(), cur->get_line_num(), rule);
    else
        s.printf("%s%s:%s:(line %d)%s", RULE_SEPARATOR, proj_n.str(), 
                 cur->get_org_dir_name(), cur->get_line_num(), rule);
}

// from bottom up
// static int pdf_bottom_line_num = 0;
void projMap::update_successfull_rule(char const *rule, int len)
{
    Initialize(projMap::update_successfull_rule);

    genString s;
    update_the_using_rule_from_bottom(s,rule,this);

    projMap *par = get_parent_project();
    if (par == 0) {
        projMap::set_path_complete_full_len(len);
        projMap::set_path_tran_error_code(PATH_TRAN_NO_ERROR);
        projMap::set_path_tran_rule((char const *)s.str());
    } else
        par->update_successfull_rule((char const *)s.str(), len);
}

static void update_complete_rule(int new_complete_length, char const *rule, projMap *cur)
{
    Initialize(update_complete_rule);

    if (projMap::report_path_complete_full_len() < new_complete_length) {
        projMap::set_path_complete_full_len(new_complete_length);
        genString s;
        update_the_using_rule(s, rule, cur);
        projMap::set_path_tran_rule(s);
    }
}

// save and compare for the project name
static int       save_path_complete_full_len;
static int       save_path_tran_error_code;
static genString save_path_tran_rule;
static int       save_path_tran_rule_flag = 0;



static bool use_same_rule(char const *rule)
{
    Initialize(use_same_rule);

    if (save_path_tran_rule_flag == 0)
        return true;

    if (strncmp(save_path_tran_rule,rule,strlen(rule)))
        return false;

    return true;
}

void project_path_sys_vars_translate(char const *m, genString &ret_path)
{
    Initialize(project_path_sys_vars_translate);

    char *p = path_sys_vars_translate(m);
    ret_path = p;
    psetfree(p);
}

//
// WORKING BUFFER FOR MAPPING
//
static int str_num = 0;
static char **str_buf = 0;
static genString tail_portion = 0;

static bool tail_portion_is_defined()
{
    if (tail_portion.length() == 0)
        return FALSE;

    return TRUE;
}

static void init_working_buffer(int match_count)
{
    Initialize(init_working_buffer);

    static int allocated_str_num = 0;

    // Free the old one.
    if (allocated_str_num) {
        for (int i=0;i<allocated_str_num+1;i++)
            if (str_buf[i])
                psetfree(str_buf[i]);

        str_num = 0;
        allocated_str_num = str_num;
    }

    str_num = match_count;
    match_count++;
    allocated_str_num = str_num;

    if (str_buf) {
        psetfree(str_buf);
        str_buf = 0;
    }

    // Create the new one.
    if (match_count)
        // let's get one extra space used as the working space
        // for the last PTN_MATCH_NO_MAPPINGs. <== no longer needed
        str_buf = (char **)calloc(sizeof(char *), (str_num + 1));
}

enum {
    NO_MORE_PTN,
    PTN_NO_SLASH,
    PTN_WITH_SLASH
};

//
// Only support one (*) between two '/'s
// "/x(*)y/", "/x(*)/", "/(*)/", "/(*)y/" are legal
// "/x(*)y(*)z/" is ambiguous.
//
// idx is the starting index for the buffer str_buf.
//
// split the physical path into the matched patterns
//
// path: the physical path
//
// cur: contains the rule for this phisical path
//
// cur_len: is the length of the matched path
//
// full_match_flag: will be set to 1 if the full path could be matched; otherwise it is set to 0
//
// idx: is used for selectors because selectors are checked one by one
static bool new_split_ptn_path(char const *path1, wkPtnRule *cur, int *cur_len, int *full_match_flag, int &idx)
{
    Initialize(new_split_ptn_path);

    char *path = strdup(path1);
    {
        WildPair *m = cur->get_multi_match_in_ptnrule();
        *cur_len = 0;
        *full_match_flag = 0;
        int len = m->match(path,MATCH_WITHOUT_END);
        if (len < 0)
            goto FAIL;
        *cur_len = len;
        char const *p = path + *cur_len;
        if (leading_char(path,'/') && *cur_len == 1)
            p = path;
        if (*p && *p != '/') {
            char *q = strchr(p,'/');
            if (q)
                *q = '\0';
            len = m->match(path,MATCH_WITH_END);
            if (q)
                *q = '/';

            if (len <= 0) {
                *cur_len = 0;
                goto FAIL;
            }
            *cur_len = len;
            p = path + *cur_len;
        } else if (!len)
            goto FAIL;
        if (*p && *p == '/')
            tail_portion = p+1;
        else
            tail_portion = 0;

        wkPtnDef *cur_def = cur->get_ptn_strs();
        int s_idx = 1;
        // int prev_start = strlen(path) + 2; //  1 is enough
        int prev_end = -2;                    // -1 is enough
        while(1) {
            while(cur_def) {
                if (cur_def->get_type() == PTN_MATCH_MAPPING || cur_def->get_type() == PTN_IDX ||
                    (cur_def->get_type() == PTN_CONST_STRING && strstr(cur_def->get_str(),"**")))
                    break;
                cur_def = cur_def->get_next();
            }
            if (!cur_def)
                break;
            int start;
            int end;
            while(1) {
                m->range(s_idx, start, end);
                s_idx++;
                if (start >= prev_end)
                    {
                        // prev_start = start;
                        prev_end = end;
                        break;
                    }
            }

            if (str_buf[idx])
                psetfree(str_buf[idx]); // str_buf is already used as working buffer
            char temp = path[end];
            path[end] = '\0';
            if (cur_def->get_type() == PTN_IDX) {
                if (strcmp(path+start,str_buf[cur_def->get_idx()])) {
                    *cur_len = 0;
                    goto FAIL; // fail
                }
            }
            else
                str_buf[idx] = strdup(path+start);
            path[end] = temp;
            cur_def = cur_def->get_next();
            idx++;
        }
        *full_match_flag = 1;
        free(path);
        return true;
    }
 FAIL:
    free(path);
    return false;
}

static void combine_idx_path(wkIdxDef *idx_head, wkPtnDef *ptn_head, int *complete_len, genString &ret_path,
                             projMap *pm)
{
    Initialize(combine_idx_path);

    int len = 1;

    // for error retreat
    char *flag = new char [sizeof(char) * str_num];
    memset(flag,0,sizeof(char) * str_num);

    wkIdxDef *cur = idx_head;

    while(cur) {
        if (cur->get_type() == IDX_CONST_STRING)
            len += strlen(cur->get_str());
        else {
            if (cur->get_index() < 0) {
                genString s;
                pm->get_physical_path(cur->get_index(), s);
                len += strlen(s.str());
            } else if (cur->get_index() >= str_num) {
                // Try to catch one problem here.
                // case 0: (0)/(2)/(1) <-> (*)/(*)/(*)
                // case 1: (0)/(1)/(2) <-> (*)/(*)/(*)
                //
                // If a ptn path a/b is sent here, case 0 should reject it.
                // But case 1 should accept it.
                cur = cur->get_next();
                while(cur) {
                    if (cur->get_type() == IDX_MATCH_INDEX && cur->get_index() < str_num) {
                        if (ptn_head) {
                            int i;

                            for (i=0;i<str_num;i++) {
                                if (flag[i] == 0)
                                    break;
                            }

                            if (i != str_num) {
                                int  len   = 0;
                                char start = 0;
                                int  cnt   = 0;

                                wkPtnDef *cur_ptn = ptn_head;
                                while(cur_ptn) {
                                    if (!start && cur_ptn->get_type() != PTN_CONST_STRING) {
                                        cnt++;
                                        if (cnt == i)
                                            start = 1;
                                    }

                                    if (start) {
                                        if (cur_ptn->get_type() == PTN_CONST_STRING)
                                            len += strlen(cur_ptn->get_str());
                                        else if (cnt == str_num)
                                            break;
                                        else
                                            len += strlen(str_buf[cnt++]);
                                    }

                                    cur_ptn = cur_ptn->get_next();
                                }
                                *complete_len -= len;
                            }
                        }

                        ret_path = 0;
                        delete [] flag;
                        return; // Reject it for case 0.
                    }
                    cur = cur->get_next();
                }

                // Accept it for case 1.
                break; // Assume the bad big (N) is fixed. This is for directory.
            } else {
                flag[cur->get_index()] = 1; // record it is visited.
                len += strlen(str_buf[cur->get_index()]);
            }
        }
        cur = cur->get_next();
    }

    genString ret_buf = 0;

    cur = idx_head;

    while(cur) {
        if (cur->get_type() == IDX_CONST_STRING)
            ret_buf += cur->get_str();
        else if (cur->get_index() < 0) {
            genString s;
            pm->get_physical_path(cur->get_index(), s);
            ret_buf += s.str();
        } else if (cur->get_index() < str_num)
            ret_buf += str_buf[cur->get_index()];
        else
            break; // Assume the bad big (N) is fixed. This is for directory.

        cur = cur->get_next();
    }

    if (tail_portion_is_defined())
        combine_dir_and_sub_dir((char const *)ret_buf.str(), tail_portion, ret_path);
    else
        ret_path = (char const *)ret_buf.str();

    delete [] flag;
}

static bool new_split_idx_path(char const *path1, wkIdxRule *cur_rule, int *cur_len, char /* rule_type */ ,
                               int *full_match_flag)
// try to match a logical or a project name
//
// path: the logical or project name
// cur_rule: contsins the definition for this logic or project name
// cur_len: is the length of the matched path
// full_match_flag: will be set to 1 if the full path could be matched; otherwise it is set to 0
{
    Initialize(new_split_idx_path);

    char *path = strdup(path1);
    {
        *cur_len = 0;
        *full_match_flag = 0;
        WildPair *m = cur_rule->get_multi_match_in_idxrule();
        if (!m)
            goto FAIL;
        int len = m->match(path);
        if (len < 0) // < 0 no match; = 0 no match also
            goto FAIL;
        *cur_len = len;
        char *p = path + *cur_len;
        if (leading_char(path,'/') && *cur_len == 1)
            p = path;
        if (*p && *p != '/') {
            char *q = strchr(p,'/');
            if (q)
                *q = '\0';
            len = m->match(path,MATCH_WITH_END);
            if (q)
                *q = '/';

            if (len <= 0) {
                *cur_len = 0;
                goto FAIL; // fail
            }
            *cur_len = len;
            p = path + *cur_len;
        }
        else if (!len)
            goto FAIL;
        if (*p && *p == '/')
            tail_portion = p+1;
        else
            tail_portion = 0;
        int s_idx = 1;
        wkIdxDef *cur = cur_rule->get_idx_strs();
        int prev_end = -2; // -1 is enough
        while(1) {
            int idx = -1;
            while(cur) {
                int break_flag = 0;
                if (cur->get_type() == IDX_MATCH_INDEX) {
                    idx = cur->get_index();
                    break_flag = 1;
                }

                cur = cur->get_next();

                if (break_flag)
                    break;
            }
            if (idx < 0)
                break;
            int start;
            int end;
            while(1) {
                m->range(s_idx, start, end);
                s_idx++;
                if (start >= prev_end) {
                    // prev_start = start;
                    prev_end = end;
                    break;
                }
            }

            char temp = path[end];
            path[end] = '\0';
            if (str_buf[idx]) {
                if (strcmp(str_buf[idx],path+start)) { // fail
                    path[end] = temp;
                    *cur_len = 0;
                    goto FAIL; // fail
                }
            }
            str_buf[idx] = strdup(path+start);
            path[end] = temp;
        }
        *full_match_flag = 1;
        free(path);
        return true;
    }
 FAIL:
    free(path);
    return false;
}

static void combine_constant_ptn_path(wkPtnDef *cur, genString &ret_path)
{
    Initialize(combine_constant_ptn_path);

    ret_path = 0;

    genString ret_buf;
    while(cur) {
        char const *p = cur->get_str();
        if (need_expand_dir(p))
            return;
        ret_buf += p;
        cur = cur->get_next();
    }

    if (tail_portion_is_defined())
        combine_dir_and_sub_dir((char const *)ret_buf.str(), tail_portion, ret_path);
    else
        ret_path = (char const *)ret_buf.str();
}

static void combine_ptn_path(wkPtnDef *ptn_head, wkIdxDef *idx_head, int *complete_len, genString &ret_path)
// After analyzing the logical name or the project name, it constructs the physical name.
//
// ptn_head: is the head of a link list which contains the definition for the physical path.
//
// idx_head: is the head of a link list which contains the definition for the logical name of the project name
//
// complete_len: is the length of the longest matched string
//
// ret_path: is the output parameter which contains the constructed physical path.
{
    Initialize(combine_ptn_path);

    int len = 1;
    int idx;

    wkPtnDef *cur = ptn_head;

    if (str_num == 0) { // a verify special case
        combine_constant_ptn_path(ptn_head, ret_path);
        return;
    }

    int local_str_num = obtain_ptn_match_cnt(ptn_head);
    for(int i=0;i<local_str_num;i++) {
        while(cur) {
            if (cur->get_type() == PTN_MATCH_MAPPING)
                break;
            cur = cur->get_next();
        }
        char const *p = cur->get_str();
        char exp_flag = need_expand_dir(p);
        if (str_buf[i] == 0) {
            if (exp_flag) {
                ret_path = 0;
                return;
            } else // constant string
                str_buf[i] = strdup(p);
        } else if (!exp_flag) {
            // constant string
            if (strcmp(str_buf[i],p)) {
                ret_path = 0;
                return;
            }
        }
        cur = cur->get_next();
    }

    cur = ptn_head;
    idx = 0;
    while(cur) {
        if (cur->get_type() == PTN_CONST_STRING)
            len += strlen(cur->get_str());
        else if (idx < local_str_num)
            len += strlen(str_buf[idx++]);
        else
            break; // for directory

        cur = cur->get_next();
    }

    genString ret_buf = 0;

    cur = ptn_head;

    idx = 0;
    while(cur) {
        char const *p;
        if (cur->get_type() == PTN_CONST_STRING)
            p = cur->get_str();
        else if (idx < local_str_num)
            p = str_buf[idx++];
        else
            break; // for directory

        ret_buf += p;
        // if it is a filter
        if (cur->get_type() == PTN_MATCH_MAPPING) {
            projFilter *pf = cur->get_filter();

            if (!pf->file_name_match(p, (FILE_TYPE|DIR_TYPE), MATCH_ANY)) {
                // It needs to remove the length of the tail_portion from path_complete_full_len

                int  len   = 0;
                char start = 0;

                wkIdxDef *cur_idx = idx_head;

                while(cur_idx) {
                    if (!start && cur_idx->get_type() == IDX_MATCH_INDEX && cur_idx->get_index() >= idx-1)
                        start = 1;

                    if (start) {
                        if (cur_idx->get_type() == IDX_CONST_STRING)
                            len += strlen(cur_idx->get_str());
                        else if (cur_idx->get_index() == local_str_num)
                            break;
                        else if (cur_idx->get_index() >= 0)
                            len += strlen(str_buf[cur_idx->get_index()]);
                    }

                    cur_idx = cur_idx->get_next();
                }

                *complete_len -= len;

                ret_path = 0;
                return;
            }
        }

        cur = cur->get_next();
    }

    if (tail_portion_is_defined())
        combine_dir_and_sub_dir((char const *)ret_buf.str(), tail_portion, ret_path);
    else
        ret_path = (char const *)ret_buf.str();
}

void convert_home_dir(char const *path1, genString &ret_path)
{
    Initialize(convert_home_dir);

    char *path = strdup(path1);
    ret_path = path;

    if (*path == '~') {
        char *p;
        char *t = 0;
        char *tmp_home = 0;

        p = path + 1;
        if (strlen(p) && *p != '/') {
            p = strchr(path,'/');
            if (p)
                *p = '\0';
            dis_passwd pd(path+1);

            if (pd == 0) {
                // No conversion will be done.
                // If it is in the project file, log an error message.
                if (parse_is_being_executed)
                    create_one_path_parser_error(PATH_PARSE_HOME_DIR_WRN, path, 0, 0, 0);
                free(path);
                return;
            } else {
                t = pd.pw_dir;
                if (p)
                    *p = '/'; // recover
            }
        } else {
            char const *home = OSapi_getenv("HOME");
            create_path_2DIS( home, &tmp_home );
            t = tmp_home;
        }

        if (p) {
            if (*p == '/')
                p++; // skip the '/'

            combine_dir_and_sub_dir(t,p,ret_path);
        } else
            ret_path = t;

        if ( tmp_home )
            free( tmp_home );
    }
    free(path);
}

static bool all_digits(char const *p)
// Check if the string "p" only contains digits.
{
    Initialize(all_digits);

    if (strlen(p) == 0)
        return 0;

    if (*p == '-' || *p == '+')
        p++;

    while(*p) {
        if (isdigit(*p))
            p++;
        else
            return(0);
    }
    return 1;
}

static void create_new_idx_def(char type, char const *str, int match_index, wkIdxDef **idx_strs, wkIdxDef **cur)
{
    Initialize(create_new_idx_def);

    wkIdxDef *new_def;

    if (type == IDX_CONST_STRING)
        new_def = db_new(wkIdxDef, (IDX_CONST_STRING, str));
    else
        new_def = db_new(wkIdxDef, (IDX_MATCH_INDEX, match_index));

    if (*idx_strs == 0)
        *idx_strs = new_def;
    else
        (*cur)->set_next(new_def);

    if (cur)
        *cur = new_def;
}

static void found_idx_left_parenthesis(char **q, char const **s, char *w)
{
    Initialize(found_idx_left_parenthesis);

    (*s)++;
    if (parse_is_being_executed && *q)
        create_one_path_parser_error(PATH_PARSE_MISS_PARENTHESES_ERR, 0, 0, 0, 0);

    *q = w;
}

static int found_idx_right_parenthesis(char **qq, char const **s, char *w, int *match_index,
                                       char *found_match_index_flag, char *found_const_string_flag)
// found a ")"
//
// qq: is the address for "(". If *qq is 0, means no "(" in front of ")" which is bad
// s: is the address into the logic (project) path. s++ will skip ")"
// w: the next position in the working buffer
// match_index: return the found index.
// found_match_index_flag: if "(" is found before ")", and only digits between "(" and ")" are found,
//                         this flag is set to 1 which means good.
//                         Otherwise, the next flag is set to 1.
// found_const_string_flag: see definition in the previous flag
{
    Initialize(found_idx_right_parenthesis);

    (*s)++;
    char *q = *qq;

    if (q == 0)
        {
            if (parse_is_being_executed)
                create_one_path_parser_error(PATH_PARSE_MISS_PARENTHESES_ERR, 0, 0, 0, 0);
        }
    else
        {
            *w = '\0';

            if (*q == 0)
                {
                    if (parse_is_being_executed)
                        create_one_path_parser_error(PATH_PARSE_EMPTY_PARENTHESES_ERR, 0, 0, 0, 0);
                }
            else if (all_digits(q))
                {
                    *found_match_index_flag = 1;

                    OSapi_sscanf(q,"%d",match_index);
                    *q = '\0';
                    *qq = 0;
                    return 1;
                }
            create_one_path_parser_error(PATH_PARSE_IDX_PARENS_NOT_DIGIT_ERR, 0, 0, 0, 0);

            // unput '(' and ')' back
            genString tt = q;
            tt += ")";
            *q++ = '(';
            strcpy(q,tt);
            *qq = 0;
            *found_const_string_flag = 1;
        }

    return 0;
}

static void remove_double_slash(char const *str, genString &ppp)
{
    Initialize(remove_double_slash);

    ppp = 0;
    if (str == 0)
        return;

    if (strlen(str) == 0)
        {
            ppp = "";
            return;
        }

    ppp = str;
    char *p = (char *)ppp.str();
    char last_char_is_slash = 0;
    char *cp_char = p;
    while(*p)
        {
            if (*p == '/')
                {
                    if (last_char_is_slash)
                        p++;
                    else
                        {
                            *cp_char++ = *p++;
                            last_char_is_slash = 1;
                        }
                }
            else
                {
                    *cp_char++ = *p++;
                    last_char_is_slash = 0;
                }
        }
    *cp_char = '\0';
}

//
// The routine will convert the definition of index portion into detailed
// structure. Please refer to the example for wkIdxDef in the include file.
//
static int analyze_idx_def(char const *idx_def, wkIdxDef **idx_strs,
                           char /* direction_flag */ )
{
    Initialize(analyze_idx_def);

    if (strlen(idx_def) == 0)
        {
            create_new_idx_def(IDX_CONST_STRING, "", 0, idx_strs, 0);
            return 0;
        }

    char const *p         = idx_def;
    genString working_buf = p;
    wkIdxDef *cur         = 0;

    *idx_strs = 0;
    while(1)
        {
            int  match_index;
            char *w = (char *)working_buf.str();
            char *q = 0;
            char const *s = p;
            char found_const_string_flag = 0;
            char found_match_index_flag  = 0;
            char first = 1;
            while(*s)
                {
                    if (*s == '(' && (first || (!first && *(s-1) != '\\')))
                        found_idx_left_parenthesis(&q, &s, w);
                    else if (*s == ')' && (first || (!first && *(s-1) != '\\')))
                        {
                            if (found_idx_right_parenthesis(&q, &s, w, &match_index, &found_match_index_flag,
                                                            &found_const_string_flag))
                                break;
                        }
                    else
                        {
                            *w++ = *s++;
                            if (q == 0)
                                found_const_string_flag = 1;
                        }
                    first = 0;
                }

            if (q && !found_match_index_flag)
                {
                    if (parse_is_being_executed)
                        create_one_path_parser_error(PATH_PARSE_MISS_PARENTHESES_ERR, 0, 0, 0, 0);
                }

            *w = '\0';

            // Find a constant string.
            if (found_const_string_flag)
                create_new_idx_def(IDX_CONST_STRING, (char *)working_buf.str(), 0, idx_strs, &cur);

            if (found_match_index_flag)
                create_new_idx_def(IDX_MATCH_INDEX, 0, match_index, idx_strs, &cur);

            p = s;
            if (*p == '\0')
                break;
        }

    return(0); // successful
}

static int check_nest_of(char const *str, char left_c, char right_c)
{
    Initialize(check_nest_of);

    int count = 0;
    while(*str)
        {
            if (*str == '\\')
                {
                    str++;
                    if (*str == '\0')
                        break;
                }
            else if (*str == left_c)
                count++;
            else if (*str == right_c)
                count--;

            str++;
        }
    return count;
}

// bad string return 0, ex : *.[ch which misses "]"
// successful return 1
static int create_new_ptn_def(char type, char *str, int idx, wkPtnDef **ptn_strs, wkPtnDef **cur)
{
    Initialize(create_new_ptn_def);

    wkPtnDef *new_def;

    if (check_nest_of(str,'[',']'))
        {
            create_one_path_parser_error(PATH_PARSE_SQUARE_BR_NOT_MATCH_ERR, 0, 0, 0, 0);
            return 0;
        }
    new_def = db_new(wkPtnDef, (type, str, idx));

    if (*ptn_strs == 0)
        *ptn_strs = new_def;
    else
        (*cur)->set_next(new_def);

    *cur = new_def;
    return 1;
}

static void found_ptn_left_parenthesis(char **q, char **s, char *w, int &nested_parentheses_count)
// found a "("
//
// q is the address for "(". If q is 0, means no "(" in front of ")" which is bad
// s is the address into the physical path. s++ will skip ")"
// w is the address of the working buffer where stands the character which will be checked next
// nested_parentheses_count: count the nested level of the parentheses
{
    Initialize(found_ptn_left_parenthesis);

    (*s)++;
    nested_parentheses_count++;
    if (nested_parentheses_count == 1) // the outest one
        *q = w;
}

static int found_ptn_right_parenthesis(char *q, char **s, char *found_match_pattern_flag, int &nested_parentheses_count)
// found a ")"
//
// q is the address for "(". If q is 0, means no "(" in front of ")" which is bad
// s is the address into the physical path. s++ will skip ")"
// found_match_pattern_flag: if "(" is found before ")", this flag is set to 1 which means good.
//                           Otherwise, it is not set which means bad.
// nested_parentheses_count: count the nested level of the parentheses
{
    Initialize(found_ptn_right_parenthesis);

    (*s)++;
    if (q == 0) // nested_parentheses_count == 0
        {
            if (parse_is_being_executed)
                create_one_path_parser_error(PATH_PARSE_MISS_PARENTHESES_ERR, 0, 0, 0, 0);
            return 1;
        }

    nested_parentheses_count--;

    if (!nested_parentheses_count)
        *found_match_pattern_flag = 1;

    return 0;
}

static int analyze_ptn_def(char *ptn_def, wkPtnDef **ptn_strs,
                           char direction_flag, int add_parentheses, int rule_type)
// The routine will convert the definition of pattern portion into detailed
// structure. Please refer to the example for wkPtnDef in the include file.
//
// ptn_def: is the defined physical path
// ptn_strs: is the returned result
// direction_flag: please refer to create_one_phy_rule()
// add_parentheses = 1: no () in physical path and idx strings are constant; hence, add () to each portion of physical
//                      path that is between two constiguous slashes.
//                   0: do not add ()
// rule_type = NORMAL_PHY_PATH: normal physical path definition
//           = LINK_MAP_PHY_PATH: if it is a link mapping selector [[ -> ... ]]
//           = REALPATH_MAP_PHY_PATH: if it is a realpath mapping selector [[ => ... ]]
//           = SEARCH_PHY_PATH: if it is a search selector [[ ls ... ]]
//           = EQUAL_EVAL     : if it is a search selector [[ == (1) (2) ]]
//           = NOT_EQUAL_EVAL : if it is a search selector [[ != (1) (2) ]]
//           = MAP_FILE_TYPE: if it is a file type selector [[ dDfF ]] selector
//           = MAP_SCRIPT_TYPE: if this is a script project
//           = MAP_FILTER_TYPE: if this is a filter project
//           = END_PHY_PATH: get_phy_rule_type(int idx) will return END_PHY_PATH if idx is out of range
{
    Initialize(analyze_ptn_def);

    char      *p          = ptn_def;
    genString working_buf = p;
    wkPtnDef  *cur        = 0;

    *ptn_strs = 0;
    int nested_parentheses_warning = 0;
    while(1)
        {
            char *w = (char *)working_buf.str();
            char *q = 0;
            char *s = p;
            char found_const_string_flag  = 0;
            char found_match_pattern_flag = 0;
            char first = 1;
            int nested_parentheses_count = 0;
            while(*s)
                {
                    if (*s == '(' && (first || (!first && *(s-1) != '\\')))
                        {
                            found_ptn_left_parenthesis(&q, &s, w, nested_parentheses_count);
                            if (nested_parentheses_count > 1)
                                {
                                    if (!nested_parentheses_warning)
                                        {
                                            nested_parentheses_warning = 1;
                                            create_one_path_parser_error(PATH_PARSE_NESTED_PARENTHESES_WRN, 0, 0, 0, 0);
                                        }
                                    *w++ = '(';
                                }
                        }
                    else if (*s == ')' && (first || (!first && *(s-1) != '\\')))
                        {
                            if (found_ptn_right_parenthesis(q, &s, &found_match_pattern_flag, nested_parentheses_count))
                                return 1; // fail
                            if (!nested_parentheses_count)
                                break;
                            *w++ = ')';
                        }
                    else
                        {
                            *w++ = *s++;
                            if (q == 0)
                                found_const_string_flag = 1;
                        }
                    first = 0;
                }

            if (q && !found_match_pattern_flag)
                {
                    if (parse_is_being_executed)
                        {
                            create_one_path_parser_error(PATH_PARSE_MISS_PARENTHESES_ERR, 0, 0, 0, 0);
                            return 1; // fail
                        }
                }

            *w = '\0';

            // Find a constant string.
            if (found_const_string_flag)
                {
                    char temp;
                    if (q)
                        {
                            temp = *q;
                            *q = '\0';
                        }
                    if (!create_new_ptn_def(PTN_CONST_STRING, working_buf, -1, ptn_strs, &cur))
                        return 1; // fail
                    if (q)
                        *q = temp;
                }

            if (found_match_pattern_flag)
                {
                    if (!q && parse_is_being_executed)
                        {
                            create_one_path_parser_error(PATH_PARSE_MISS_PARENTHESES_ERR, 0, 0, 0, 0);
                            return 1; //fail
                        }

                    if (*q == 0)
                        {
                            if (parse_is_being_executed)
                                create_one_path_parser_error(PATH_PARSE_EMPTY_PARENTHESES_ERR, 0, 0, 0, 0);
                            return 1;
                        }

                    if (all_digits(q) && !add_parentheses && rule_type != NORMAL_PHY_PATH)
                        {
                            int idx = -1;
                            OSapi_sscanf(q,"%d",&idx);
                            if (!create_new_ptn_def(PTN_IDX, q, idx-1, ptn_strs, &cur))
                                return 1; // should not fail here
                        }
                    else
                        {
                            if (leading_char(q,'/') || trailing_char(q,'/'))
                                create_one_path_parser_error(PATH_PARSE_LEADING_TRAILING_SLASH_ERR, 0, 0, 0, 0);
                            if (!create_new_ptn_def(PTN_MATCH_MAPPING, q, -1, ptn_strs, &cur))
                                return 1; // should not fail here
                        }
                }

            p = s;
            if (*p == '\0')
                break;
        }

    if (direction_flag == TWO_WAY)
        {
            cur = *ptn_strs;
            while(cur)
                {
                    if (cur->get_type() == PTN_CONST_STRING && need_expand_dir(cur->get_str()))
                        {
                            create_one_path_parser_error(PATH_PARSE_INCOMPLETE_LOG_TO_PHY_ERR, 0, 0, 0, 0, 0);
                            return 1; // fail
                        }
                    cur = cur->get_next();
                }
        }

    return(0); // successful
}


// count the number of matching patterns in the pattern portion.
static int obtain_ptn_match_cnt(wkPtnDef *cur)
{
    Initialize(obtain_ptn_match_cnt);

    int count = 0;

    while(cur)
        {
            if (cur->get_type() == PTN_MATCH_MAPPING)
                count++;
            cur = cur->get_next();
        }

    return(count);
}

wkIdxDef::wkIdxDef(char t, char const *s) : type(t), next(0)
{
    wk_str = s;
}

wkIdxDef::wkIdxDef(char t, int index) : type(t), next(0)
{
    wk_idx = index;
}

wkIdxDef::~wkIdxDef()
{
    Initialize(wkIdxDef::~wkIdxDef);

    if (next)
        delete next;
}

void wkIdxRule::create_idx_ptn()
{
    Initialize(wkIdxRule::createA_idx_ptn);

    wkIdxDef *cur = get_idx_strs();
    ptn = 0;
    while(cur)
        {
            if (cur->get_type() == IDX_CONST_STRING)
                ptn += cur->get_str();
            else // if (cur->get_type() == IDX_MATCH_INDEX)
                {
                    ptn += '(';
                    int index = cur->get_index();
                    if (0 < index && index <= str_num)
                        ptn += str_buf[index-1];
                    ptn += ')';
                }
            cur = cur->get_next();
        }
    if (PDF_DebugLevel > 0)
        {
            msg("\"$1\"\"$2\"") << get_idx_def() << eoarg << ptn.str() << eom;
            ;
        }
}

wkPtnDef::wkPtnDef(char t, char const *s, int index)
    : type(t), wk_str(s), ptn_pf(0), idx(index), next(0)
{
    Initialize(wkPtnDef::wkPtnDef);

    wk_str = s;

    if (t == PTN_MATCH_MAPPING)
        ptn_pf = db_new(projFilter,(s));
}


wkPtnDef::~wkPtnDef()
{
    Initialize(wkPtnDef::~wkPtnDef);

    if (ptn_pf)
        delete ptn_pf;

    if (next)
        delete next;
}

wkPtnRule::wkPtnRule(int exclude_type, char const *ptn, char direction_flag, char rule_type,
                     char idx_const_string, int link_phy_rule) :
    org_ptn(ptn), ptn_def(0), ptn_strs(0), result(0), multi_match_in_ptnrule(0),
    dir_multi_match_in_ptnrule(0), exclude_file_ptn_def(0), exclude_dir_ptn_def(0)
    // create one ptn rule
    //
    // ptn: the defined physical path
    // link_phy_rule = 0: no selector is defined
    //               = 1: some selectors are defined
    // exclude_type, direction_flag, rule_type, idx_const_string: please refer to create_one_phy_rule()
{
    if (*ptn == '^')
        ptn++;

    if (rule_type == MAP_SCRIPT_TYPE || rule_type == MAP_FILTER_TYPE)
        idx_const_string = 1; // don"t want to get ()

    result = set_ptn(exclude_type, ptn, direction_flag, rule_type, idx_const_string, 1, link_phy_rule);
}

wkPtnRule::~wkPtnRule()
{
    Initialize(wkPtnRule::~wkPtnRule);

    if (ptn_strs)
        delete ptn_strs;

    if (multi_match_in_ptnrule)
        delete multi_match_in_ptnrule;
}

void wkPtnRule::set_multi_match_in_ptnrule(int exclude_type, int rule_type)
// set matched rule for the file path
//
// exclude_type=1: this is an exclude project ==> ^path
//             =0: this is a regular project
//
// rule_type: if it is not NORMAL_PHY_PATH, don"t pre_pend **/
{
    Initialize(wkPtnRule::set_multi_match_in_ptnrule);

    char *p = ptn_def;
    genString new_ptn_def;
    if (exclude_type & EXCLUDE_FILE && rule_type == NORMAL_PHY_PATH)
        exclude_file_ptn_def.printf("**/%s",ptn_def.str());

    if (exclude_file_ptn_def.length())
        p = exclude_file_ptn_def;

    if (multi_match_in_ptnrule)
        delete multi_match_in_ptnrule;
    multi_match_in_ptnrule = (WildPair *)db_new(WildPair, (p));
}

WildPair *wkPtnRule::get_multi_match_in_ptnrule()
// get the WildPair object for the definition of the physical path
{
    Initialize(wkPtnRule::get_multi_match_in_ptnrule);

    return multi_match_in_ptnrule;
}

void wkPtnRule::set_dir_multi_match_in_ptnrule(int par_imp_flag)
// set matched rule for the directory path
//
// par_imp_flag = 1: parent project is an implicit project "="
//                0: parent project is a regular project ":"
{
    Initialize(wkPtnRule::set_dir_multi_match_in_ptnrule);

    if (!exclude_dir_ptn_def.length())
        {
            if (par_imp_flag)
                exclude_dir_ptn_def.printf("**/%s/**",ptn_def.str());
            else
                exclude_dir_ptn_def.printf("%s/**",ptn_def.str());
        }

    if (dir_multi_match_in_ptnrule)
        delete dir_multi_match_in_ptnrule;
    dir_multi_match_in_ptnrule = (WildPair *)db_new(WildPair, (exclude_dir_ptn_def.str()));
}

WildPair *wkPtnRule::get_dir_multi_match_in_ptnrule()
// get the WildPair object for the definition of the directory portion of the physical path
{
    Initialize(wkPtnRule::get_dir_multi_match_in_ptnrule);

    return dir_multi_match_in_ptnrule;
}

int wkPtnRule::failed()
{
    Initialize(wkPtnRule::failed);

    return result;
}

void wkPtnRule::update_idx_ptn()
// It needs to replace the idx field in the physical path to some pre-defined matched pattern
{
    Initialize(wkPtnRule::update_idx_ptn);

    genString s = 0;

    wkPtnDef *cur = get_ptn_strs();
    while(cur)
        {
            if (cur->get_type() == PTN_CONST_STRING)
                s += cur->get_str();
            else if (cur->get_type() == PTN_MATCH_MAPPING)
                {
                    s += '(';
                    s += cur->get_str();
                    s += ')';
                }
            else if (cur->get_type() == PTN_IDX)
                {
                    if (cur->get_idx() < str_num && cur->get_idx() >= 0)
                        {
                            s += '(';
                            s += str_buf[cur->get_idx()];
                            s += ')';
                        }
                    // else forget the error now which will be caught in verify_selector_idx();
                }
            cur = cur->get_next();
        }

    ptn_def = s.str();

    set_multi_match_in_ptnrule(NON_EXCLUDE, NORMAL_PHY_PATH);
}

wkIdxRule::wkIdxRule(char const *idx, char direction_flag) : idx_def_flag(0),
                                                       org_idx(idx), idx_def(0), idx_strs(0), ptn(0), multi_match_in_idxrule(0)
{
    Initialize(wkIdxRule::wkIdxRule);

    if (idx)
        {
            idx_def_flag = 1;
            set_idx(idx, direction_flag);
        }
    else
        idx_def_flag = 0;
}

wkIdxRule::~wkIdxRule()
{
    Initialize(wkIdxRule::~wkIdxRule);

    if (idx_strs)
        delete idx_strs;

    if (multi_match_in_idxrule)
        delete multi_match_in_idxrule;
}

void wkIdxRule::set_multi_match_in_idxrule()
// set the matched rule for project name or logical name
{
    Initialize(wkIdxRule::set_multi_match_in_idxrule);

    if (ptn.length() == 0)
        return;
    multi_match_in_idxrule = (WildPair *)db_new(WildPair, (ptn));
}

WildPair *wkIdxRule::get_multi_match_in_idxrule()
// return the WildPair object for the whole ptn definition of this idx (logic or project) name
{
    Initialize(wkIdxRule::get_multi_match_in_idxrule);

    return multi_match_in_idxrule;
}

char const *wkIdxRule::get_org_idx() const
{
    Initialize(wkIdxRule::get_org_idx);

    static char const *n = "";

    if (org_idx.length())
        return org_idx;
    else
        return n;
}

void wkIdxRule::set_idx(char const *s, char direction_flag)
// construct the internal data structure for the logic name or project name
//
// s: is the logic or project name
// direction_flag: please refer to create_one_phy_rule()
{
    Initialize(wkIdxRule::set_idx);

    if (idx_strs)
        {
            delete idx_strs;
            idx_strs = 0;
        }

    genString ss;
    int leading_quote = 0;
    if (*s == '\"')
        leading_quote = 1;
    else
        {
            convert_home_dir(s, ss);
            s = ss;
        }

    // convert environment variables
    genString p;
    if ( is_str_quoted(s) )
        p = dequote_str(s);
    else
        project_path_sys_vars_translate(s, p);

    genString ppp;

    if (!leading_quote && direction_flag != REAL_RIGHT_WAY )
        {
            remove_double_slash(p,ppp);
            p = (char *)ppp.str();
            if (trailing_char (p.str(),'/') && p.length() > 1)
                chop_trail((char *)p.str());
        }

    idx_def = (char *)p.str();

    analyze_idx_def(idx_def, &idx_strs, direction_flag);
}

static void convert_link(char const *path1, char const *org_path, genString &ret_path, char origin_str_flag)
// convert a physical path if it is a link
//
// path: the physical path (some thing was already translated: ex: system environment variable "$HOME")
// org_path: the original path
// ret_path: the output path
// origin_str_flag: if set, it is the original physical path; Otherwise, it is the second time to set this physical path
//                  and s is realpath
{
    Initialize(convert_link);

    char *path = strdup(path1);

    // search the last '/' before the first '(' or '*'
    char *p     = path;
    char save_c = 0;

    ret_path = path;

    while(*p) {
        if (*p == '(' || *p == '*') {
            save_c = *p;
            break;
        }
        p++;
    }

    char *q = 0;
    if (save_c) {
        *p = '\0';
        q = strrchr(path,'/');
        if (q)
            *q = '\0';
        else if (origin_str_flag)
            ; //  create_one_path_parser_error(PATH_PARSE_ROOT_DIR_NOT_EXIST_WRN, org_path, org_path,
        //                           projmap_get_path_parse_line_num(), 0);
        *p = save_c; // recover
    }

    char r_path[MAXPATHLEN];
#ifndef _WIN32
    if (strchr(org_path,'*') == 0 && origin_str_flag && strstr(org_path,"/%/") == 0)
#else
        if (strchr(org_path,'*') == 0 && origin_str_flag && strstr(org_path,"/\\%/") == 0)
#endif
            {
                if (!fileCache_realpath(path,r_path))
                    ; //  create_one_path_parser_error(PATH_PARSE_ROOT_DIR_NOT_EXIST_WRN, org_path, org_path,
                //                          projmap_get_path_parse_line_num(), 0);
            }
    if (q)
        *q = '/'; // recover

    free(path);
}

bool contain_target_char(char const *r, char tar, int ignore_quote)
// check if the string s containing tar
// ignore_quote: if set, forget the text between quotes; otherwise, any text must be checked.
{
    Initialize(contain_target_char);

    int found_quote = 0;
    while(*r)
        {
            if (*r == '\\')
                {
                    r++;
                    if (*r)
                        r++;
                }
            else if (*r == '"')
                {
                    if (found_quote)
                        found_quote = 0;
                    else
                        found_quote = 1;
                }
            else if (*r == tar)
                {
                    if (!ignore_quote || !found_quote)
                        return TRUE; // found matching pattern
                }
            r++;
        }
    return FALSE;
}

static int convert_constant_string(char *s, genString &p)
// put "()" around the constant path
//
// s is the input constant path
// p is the returned path
{
    Initialize(convert_constant_string);

    p = s;
    if (contain_target_char(s, '(', 0))
        return 0;

    // create artificial parentheses
    if (leading_char(s,'/'))
        {
            s++;
            p = "/";
        }
    else
        p = 0;
    while(1)
        {
            char *r = strchr(s,'/');
            if (r)
                *r = '\0';

            p += "(";
            p += s;
            p += ")";

            if (r)
                {
                    *r  = '/'; // recover
                    p  += "/";
                    s   = r+1;
                }
            else
                break;
        }
    return 1;
}

static void simple_realpath(char *s, genString &new_s)
{
    Initialize(simple_realpath);

    new_s = 0;
    while(*s)
        {
            new_s += *s;
            if (*s == '/')
                {
                    while(1)
                        {
                            while(*s == '/')
                                s++;
                            if (strncmp(s, "./", 2) == 0)
                                s += 2;
                            else
                                break;
                        }
                }
            else
                s++;
        }
    if (new_s.length() > 2 && strncmp(new_s , "./", 2) == 0)
        {
            genString ss = (char *)new_s.str() + 2;
            new_s = (char *)ss.str();
        }
}

static void remove_3_stars(char const *str , genString &new_s)
{
    Initialize(remove_3_stars);

    if (!strstr(str,"***")) {
        new_s = str;
        return;
    }

    char *s = strdup(str);
    char *p;
    char *start_p;
    genString bad_stars = 0;

    while(1) {
        p = strstr(s,"**");
        start_p = p;
        if (!p) {
            new_s += s;
            break;
        }
        p += 2;
        *p = '\0';
        new_s += s;
        *p = '*';
        while(*p == '*')
            p++;
        char temp = *p;
        *p = '\0';
        if (bad_stars.length() == 0)
            bad_stars = start_p;
        *p = temp;
        if (*p == 0)
            break;
        s = p;
    }
    create_one_path_parser_error(PATH_PARSE_3_STARS_WRN, bad_stars, 0, 0, 0);
    free(s);
}

static void check_not_supported_features(char *s, genString& ret_path)
{
    Initialize(check_not_supported_features);

    ret_path = s;

    if (strlen(s) < 2)
        return;

    int  err_flag = 0;
    char *ss      = s;
    char *last_p  = 0;
    while(1)
        {
            char *p = strstr(ss,"..");
            if (!p)
                break;
            last_p = p;
            if (*(p-1) != '/' || *(p+2) != '/' && *(p+2) != '\0')
                {
                    err_flag = 1;
                    break;
                }
            ss = p+2;
        }

    if (!last_p)
        return;

    if (!err_flag)
        {
            err_flag = 1;
            if (leading_char(s,'/'))
                {
                    char temp = *(last_p+2);
                    *(last_p+2) = '\0';
                    if (!need_expand_dir(s))
                        {
                            paraset_get_fullpath(s, ret_path);
                            *(last_p+2) = temp;
                            ret_path += last_p+2;
                            err_flag = 0;
                        }
                    *(last_p+2) = temp;
                }
        }

    if (err_flag)
        create_one_path_parser_error(PATH_PARSE_DOUBLE_DOTS_ERR, 0, 0, 0, 0);
}

static void no_two_two_star(char const *s)
{
    Initialize(no_two_two_star);

    char const *p = strstr(s,"**");
    if (!p)
        return;
    p = strstr(p+2,"**");
    if (!p)
        return;
    create_one_path_parser_error(PATH_PARSE_TWO_TWO_STARS_ERR, 0, 0, 0, 0);
}

int wkPtnRule::set_ptn(int exclude_type, char const *s, char direction_flag, char rule_type,
                       char idx_const_string,
                       char origin_str_flag, int link_phy_rule)
// construct the internal data structur for the physical path
//
// s: the physical path
// origin_str_flag: if set, it is the original physical path; Otherwise, it is the second time to set this physical path
//                  and s is realpath
// link_phy_rule = 0: no selector is defined
//               = 1: some selectors are defined
// exclude_type, direction_flag, rule_type, idx_const_string: please refer to create_one_phy_rule()
{
    Initialize(wkPtnRule::set_ptn);

    if (strcmp(s,"\"\"") == 0)
        {
            create_one_path_parser_error(PATH_PARSE_EMPTY_PHY_PATH_ERR, 0, 0, 0, 0);
            return 1; // fail
        }

    if (strcmp(s,".") == 0 || strcmp(s,"(.)") == 0)
        {
            create_one_path_parser_error(PATH_PARSE_DOT_ONLY_ERR, 0, 0, 0, 0);
            return 1; // fail
        }

    genString new_s;
    genString new_ss;
    remove_3_stars(s,new_s);
    s = new_s;
    no_two_two_star(s);

    if (ptn_strs)
        delete ptn_strs;

    // convert environment variables
    genString p;
    project_path_sys_vars_translate(s, p);

    genString ppp;
    remove_double_slash(p,ppp);
    p = (char *)ppp.str();

    // convert ~
    if (leading_char (p,'~'))
        {
            ppp = (char *)p.str();
            convert_home_dir(ppp, p);
        }

    if (link_phy_rule == 0)
        {
            check_not_supported_features(p, ppp);
            p = (char *)ppp.str();

            simple_realpath(p, new_ss);
            p = (char *)new_ss.str();
        }

    int add_parentheses = 0;
    if (!idx_const_string)
        {
            genString pp;
            add_parentheses = convert_constant_string((char *)p.str(),pp);
            ptn_def = (char *)pp.str();
        }
    else
        ptn_def = (char *)p.str();

    if (trailing_char (ptn_def.str(),'/') && ptn_def.length() > 1)
        chop_trail((char *)ptn_def.str());

    // convert link
    // If it is for link map, don't convert the link because only format is cared.
    if (leading_char((char *)ptn_def.str(), '/') &&
        (rule_type == NORMAL_PHY_PATH || rule_type == REALPATH_MAP_PHY_PATH) &&
        (exclude_type & EXCLUDE_RULE) == 0)

        {
            genString t = (char *)ptn_def.str();
            convert_link((char *)t.str(), s, ptn_def, origin_str_flag);
        }

    set_multi_match_in_ptnrule(exclude_type, rule_type);
    if (exclude_type & EXCLUDE_DIR)
        set_dir_multi_match_in_ptnrule(exclude_type & EXCLUDE_PAR_IMP);

    if (ptn_is_defined())
        {
            // If direction_flag == TWO_WAY, it might issue an error PATH_PARSE_INCOMPLETE_LOG_TO_PHY_ERR later.
            // This error is not suitable for the following selectors; hence to go-around by changing the direction_flag
            if (rule_type == LINK_MAP_PHY_PATH     ||
                rule_type == REALPATH_MAP_PHY_PATH ||
                rule_type == TCL_BOOLEAN_EVAL      ||
                rule_type == EQUAL_EVAL            ||
                rule_type == NOT_EQUAL_EVAL        ||
                rule_type == SEARCH_PHY_PATH)
                direction_flag = RIGHT_WAY;

            // ignore the returned value. error has been reported.
            return analyze_ptn_def((char *)ptn_def.str(),&ptn_strs, direction_flag, add_parentheses, rule_type);
        }

    return 0; // for success
}


bool wkIdxRule::is_constant_rule()
{
    wkIdxDef *cur = get_idx_strs();
    if (cur->get_next() || cur->get_type() != IDX_CONST_STRING)
        return FALSE;

    return TRUE;
}

char const *wkIdxRule::get_ptn()
{
    return ptn;
}

int wkPtnRule::get_max_ptn_count()
{
    if (this == 0)
        return 0;
    return obtain_ptn_match_cnt(get_ptn_strs());
}

int wkPtnRule::count_const_two_star()
// count the number of ** in the constant string of a physical path
// constant string means this string is not sorrounded by parentheses
{
    if (this == 0)
        return 0;
    wkPtnDef *cur = get_ptn_strs();
    int count = 0;
    while(cur)
        {
            if (cur->get_type() == PTN_CONST_STRING && strstr(cur->get_str(),"**"))
                count++;
            cur = cur->get_next();
        }
    return count;
}

void wkPtnRule::fill_idx(genString &s, char const *full_path)
// If the physical path contains idx, it will be replaced with text.
// You must fill str_buf[], before you come here.
//
// s: contain the returned path
// full_path: is the directory name to be used when (0) is defined
{
    Initialize(wkPtnRule::fill_idx);

    s = 0;
    wkPtnDef *cur = get_ptn_strs();
    while(cur)
        {
            if (cur->get_type() == PTN_IDX)
                {
                    if (cur->get_idx() < 0)
                        s += full_path;
                    else
                        s += str_buf[cur->get_idx()];
                }
            else
                s += cur->get_str();
            cur = cur->get_next();
        }
}

bool wkPtnRule::no_idx()
// check if the physical rule contains any index
// If it is, return FALSE otherwise return TRUE.
{
    Initialize(wkPtnRule::no_idx);

    wkPtnDef *cur = get_ptn_strs();
    while(cur)
        {
            if (cur->get_type() == PTN_IDX)
                return FALSE;
            cur = cur->get_next();
        }
    return TRUE;
}

void projMap::get_defined_name(genString &proj_name, genString &logic_name)
// convert physical definition to logic and project definition for one level only
//
// proj_name:  return project definition
// logic_name: return logic   definition
{
    Initialize(projMap::get_defined_name);

    wkPtnRule *cur_ptn_rule = get_phy_rule(0);
    int match_count = cur_ptn_rule->get_max_ptn_count();
    if (!match_count)
        return;

    init_working_buffer(match_count);

    char *tmp = strdup(get_dir_name());
    char *p = tmp;
    int idx = 0;

    while(*p) {
        char *st = 0;
        char *en;
        while(*p) {
            if (*p == '(')
                st = p+1;
            else if (*p == ')') {
                en = p;
                break;
            } else if (*p == '\\')
                p++;
            p++;
        }
        if (st) {
            *en = '\0';
            str_buf[idx++] = strdup(st);
            *en = ')';
            p = en + 1;
        }
        else
            break;
    }
    free(tmp);
    Assert(idx == match_count);

    int cur_len = 0;
    combine_idx_path(get_project_rule()->get_idx_strs(), cur_ptn_rule->get_ptn_strs(), &cur_len, proj_name,  this);
    combine_idx_path(get_logic_rule()->get_idx_strs(),   cur_ptn_rule->get_ptn_strs(), &cur_len, logic_name, this);
}

void projMap::get_matched_name(char const *path, genString &ret_path, char tp)
// convert physical name to logic or project name for one level only
//
// path: the physical name
// ret_path: contain the converted path
// type == 0: project
// type == 1: logic
// type == 2: project and specially search for the exclude project
{
    Initialize(projMap::get_matched_name);

    if (is_exclude_proj() && tp != 2) {
        ret_path = 0;
        return;
    }

    genString new_path = path;
    path = new_path;

    wkIdxRule *cur_idx_rule;
    if (tp == 0)
        cur_idx_rule = get_project_rule();
    else
        cur_idx_rule = get_logic_rule();
    wkPtnRule *cur_ptn_rule = get_phy_rule(0);

    int match_count = get_phy_match_count();

    init_working_buffer(match_count);

    int cur_len;
    int status;

    int full_match_flag = 0;
    int sidx = 0;
    status = new_split_ptn_path(path, cur_ptn_rule, &cur_len, &full_match_flag, sidx);

    if (!status) {
        ret_path = 0;
        return;
    }

    // The next routine will fail only when (N) is too big
    // and there is no match in the ptn path
    combine_idx_path(cur_idx_rule->get_idx_strs(), cur_ptn_rule->get_ptn_strs(), &cur_len, ret_path, this);
}

//  START FOR PROJ DEF
//
//  cur_parser_p: It is the current projLevel node.
//  proj_rule_cur: The currently defined project node.
//  proj_rule_head: It is the first defined project and this node becomes
//         the head of all projects and child_projects.
//

// If a project contains "{" and "}", then dec_level will reset proj_rule_cur to zero.
// For the special case such as local = /usr/kk/s { (*.C) }
// proj_rule_cur still needs to be reset to zero.
// Hence touch_level will be reset to zero in crt_proj, if the cur_parser_p->get_level() is zero.
// But it will be set in inc_level and dec_level.
// In close_proj, if touch_level == 0, then proj_rule_cur will be set to zero.
static char      touch_level     = 0;
static projLevel *cur_parser_p   = 0;
projLevel *get_cur_parser_p () { return cur_parser_p; }

static projMap   *proj_rule_cur  = 0;
static projMap   *proj_rule_head = 0;

// BEGIN WORKING AREA FOR THE MUTIPLE PDF FILES
typedef struct
{
    projMap *head;
    char    *filename;
} multi_pdf_entry;

genArr(multi_pdf_entry);

static genArrOf(multi_pdf_entry) multi_pdf_buf;

static int log_pdf_file_name(char const *fn)
{
    Initialize(log_pdf_file_name);

    for(int i=0;i<multi_pdf_buf.size();i++)
        {
            multi_pdf_entry *p = multi_pdf_buf[i];
            if (strcmp(p->filename,fn) == 0)
                return 0;
        }
    multi_pdf_entry *p = multi_pdf_buf.grow();
    p->filename = strdup(fn);
    p->head = 0;
    return 1;
}

void clean_pdf_file_name()
// When loading a pdf file, its name is logged into a buffer. Any new pdf
// file name is checked against this buffer to avoid loading the same file
// a second time.
//
// When the customer wants to replace pdf files, this routine cleans this
// logged buffer.
{
    Initialize(clean_pdf_file_name);

    for(int i=0;i<multi_pdf_buf.size();i++)
        {
            multi_pdf_entry *p = multi_pdf_buf[i];

            psetfree(p->filename);
            p->filename = 0;
            p->head = 0;
        }
    multi_pdf_buf.reset();
}

static void log_projMap_head()
{
    Initialize(log_projMap_head);

    if (multi_pdf_buf.size() == 0)
        return;
    multi_pdf_entry *p = multi_pdf_buf[multi_pdf_buf.size()-1];
    p->head = proj_rule_head;
}

static void check_one_other_file_project_names(projMap *tar, char const *tar_proj_name, projMap *cur, char const *filename)
{
    Initialize(check_one_other_file_project_names);

    while(cur) {
        char const *proj_name = cur->get_org_project_name();
        wkIdxRule *p0 = cur->get_project_rule();
        if (p0->is_constant_rule()) {
            if (strcmp(proj_name, tar_proj_name) == 0) {
                // BAD!
                create_one_path_parser_error(PATH_PARSE_SAME_PROJECT_NAME_ERR, proj_name, filename,
                                             tar->get_line_num(),
                                             cur->get_line_num(),
                                             0);
                return;
            }
        }
        cur = cur->get_next_project();
    }
}

static void check_other_file_project_names(projMap *cur, char const *proj_name)
{
    Initialize(check_other_file_project_names);

    for (int i=0;i<multi_pdf_buf.size();i++)
        {
            multi_pdf_entry *p = multi_pdf_buf[i];
            check_one_other_file_project_names(cur, proj_name, p->head, p->filename);
        }
}

// END WORKING AREA FOR THE MUTIPLE PDF FILES
// BEGIN WORKING AREA FOR THE ROOT PATH
typedef struct
{
    char *org_path;
    char *real_path;
    int  org_len;
    int  real_len;
} root_path_entry;

genArr(root_path_entry);

static genArrOf(root_path_entry) root_path_data;

extern "C" void init_root_path_data()
{
    Initialize(init_root_path_data);
    root_path_data.reset();
}

static void log_root_path(char const *path)
// log the root physical path for the link problem
// Any physical path with leading slash will be checked with the logged path to
// solve the link problem.
//
// path: is the root path
{
    Initialize(log_root_path);

    genString new_path = path;
    remove_back_slash_and_parentheses((char *)new_path.str());
    path = (char *)new_path.str();

    if (strcmp(path,"/") == 0)
        return;

    if (strstr(path,"/%/.pset"))
        return;

    if (strstr(path,"/%/.make"))
        return;

    if (strstr(path,"/%/.makemf"))
        return;

    if (strstr(path,"/%/.pmoddir"))
        return;

    for (int i=0; i<root_path_data.size(); i++)
        {
            root_path_entry *p = root_path_data[i];
            if (strncmp(p->org_path,path,p->org_len) == 0) // has been done before
                return;
        }

    char pathname[MAXPATHLEN];
    fileCache_realpath (path, pathname);

    root_path_entry *p = root_path_data.grow();
    p->org_path  = strdup(path);
    p->real_path = strdup(pathname);
    p->org_len   = strlen(path);
    p->real_len  = strlen(pathname);
}

char *projMap_convert_realpath(char const *path, char *pathname)
{
    Initialize(projMap_convert_realpath);

    if (path == 0)
        return 0;

    genString new_path;

    convert_home_dir(path, new_path);
    path = new_path.str();

    strcpy(pathname,path);
    int i;
    for (i=0; i<root_path_data.size(); i++) {
        root_path_entry *p = root_path_data[i];
        if (strncmp(p->org_path,path,p->org_len) == 0 ||
            strncmp(p->org_path,pathname,strlen(pathname)) == 0)
            return pathname;
    }

    for (i=0; i<root_path_data.size(); i++) {
        root_path_entry *p = root_path_data[i];
        if (strncmp(p->org_path,path,p->org_len) == 0)
            return pathname;

        if (strncmp(p->real_path,pathname,p->real_len) == 0) {
            genString s;
            s.printf("%s%s",p->org_path,pathname+p->real_len);
            strcpy(pathname,(char *)s.str());
            return pathname;
        }

        if (strncmp(p->real_path,pathname,strlen(pathname)) == 0) {
            int len = p->real_len - strlen(pathname);
            if (len <= 0 || p->org_len - len <= 0)
                continue;
            strncpy(pathname, p->org_path, p->org_len - len);
            pathname[p->org_len - len] = '\0';
            return pathname;
        }
    }

    char *pathDup = strdup(path);
    char *p = pathDup;
    while(1) { // lets do directory by directory
        p = strchr(p+1,'/');
        if (!p) // end of path
            break;

        *p = '\0';

        char *status = 0;
#ifdef _WIN32
        //Fixing code so that absolute path names on NT do not get converted to
        //some wrong directory path.
        if (pathDup) {
            int len = 2;
            if (pathDup[0] == '/')
                len++;
            if (strlen(pathDup) == len && isalpha(pathDup[len-2]) && pathDup[len-1] == ':') {
                genString p(pathDup); 
                p += "/";  
                status = fileCache_realpath (p, pathname);
            } else { 
                status = fileCache_realpath (pathDup, pathname);
            }
        }
#else
        status = fileCache_realpath (pathDup, pathname);
#endif
        *p = '/';

        if (!status) {
            // if (errno != ENOENT)
            strcpy(pathname,pathDup);
            free(pathDup);
            return pathname;
        }

        if (strncmp(pathDup, pathname, strlen(pathname)) == 0)
            continue;

        genString newpath;

        combine_dir_and_sub_dir(pathname, p+1, newpath);

        for (i=0; i<root_path_data.size(); i++) {
            root_path_entry *p = root_path_data[i];
            if (strncmp(p->real_path,newpath.str(),p->real_len) == 0) {
                genString s;
                s.printf("%s%s",p->org_path,newpath.str()+p->real_len);
                strcpy(pathname,(char *)s.str());
                free(pathDup);
                return pathname;
            }

            if (strncmp(p->real_path,newpath.str(),newpath.length()) == 0) {
                int len = p->real_len - newpath.length();
                if (len <= 0 || p->org_len - len <= 0)
                    continue;
                strncpy(pathname, p->org_path, p->org_len - len);
                pathname[p->org_len - len] = '\0';
                free(pathDup);
                return pathname;
            }
        }
    }

    strcpy(pathname,pathDup);
    free(pathDup);
    return pathname;
}

void project_convert_filename(char const *path, genString &ret_path)
{
    Initialize(project_convert_filename);

    if (path == 0 || strlen(path) == 0)
        {
            ret_path = path;
            return;
        }

    //boris: check the hash table first
    if (proj_path_convert (path, ret_path))
        return;

    genString input_path = path;

    char pathname[MAXPATHLEN];

    genString s;
    project_path_sys_vars_translate(path, s);
    path = (char *)s.str();

    if (projMap_convert_realpath(path, pathname))
        ret_path = pathname;
    else
        ret_path = path;

    //boris:
    proj_path_report_conversion ((char *)input_path, ret_path);
    if (strcmp(path,(char *)input_path) != 0)
        proj_path_report_conversion (path, ret_path);
}

// END WORKING AREA FOR THE ROOT PATH

// If it is called from the initialized routine,
//     it is forced to create the top one.
// If it is called from the parser, this one will be executed
//     only when a full spec id defined which means proj_rule_cur
//     is not zero.
extern "C" void inc_level(char forced, int level)
{
    Initialize(inc_level);

    //mark the beginning of the subsystems rule
    if (inside_subsystem_project && subsystem_proj_level == -1 && level > -1) 
        {
            subsystem_proj_level = level;
            return;
        }

    touch_level = 1;

    if (proj_rule_cur == 0 && forced == 0)
        return;

    projLevel *new_parser_p;

    if (cur_parser_p)
        {
            new_parser_p = db_new(projLevel, (0, cur_parser_p->get_level()+1));
            cur_parser_p->set_child(new_parser_p);
        }
    else
        new_parser_p = db_new(projLevel, ());

    cur_parser_p = new_parser_p;
}

static void set_current_proj(projLevel *cur_parser_p)
{
    Initialize(set_current_proj);

    projLevel *up_parser_p = cur_parser_p->get_parent();
    if (up_parser_p)
        proj_rule_cur = up_parser_p->get_map(); // Project is currently being defined.
    else
        proj_rule_cur = 0; // No project is currently being defined.
}

extern "C" void dec_level(char forced, int level)
// forced = 0: If it is called from the initialized routine,
//             it is forced to remove the top one.
// forced = 1: If it is called from the parser, this one will be executed
//             only when a full spec is defined which means proj_rule_cur
//             is not zero.
{
    Initialize(dec_level);
    
    //mark the end of the subsystems rule
    if (inside_subsystem_project && level > -1 && 
        level == subsystem_proj_level) {
        inside_subsystem_project = FALSE;
        subsystem_proj_level = -1;
        return;
    }

    touch_level = 1;

    if (proj_rule_cur == 0 && forced == 0)
        return;

    projLevel *up_parser_p;

    // This level completes. Move one level up in the "projLevel" double link list.
    up_parser_p = cur_parser_p->get_parent();

    delete cur_parser_p;

    cur_parser_p = up_parser_p;

    // The toppest level does not have parent, up_parser_p is NULL.
    if (cur_parser_p)
        {
            cur_parser_p->set_child(0);

            set_current_proj(cur_parser_p);
        }
}

// Parser tells us it completes the currently defined project.
extern "C" void close_proj()
{
    if (touch_level == 0 && cur_parser_p->get_level() == 0)
        proj_rule_cur = 0; // No project is currently being defined.
}

static void link_current_project(projMap *cur)
// link the whole projMap tree
//
// cur: is the latest created projMap.
{
    Initialize(link_current_project);

    if (cur_parser_p == 0 || cur_parser_p->get_parent() == 0)
        add_top_exc_proj_heads(cur, 0);

    if (proj_rule_head == 0)
        proj_rule_head = cur; // Save it because it is the very first one.
    proj_rule_cur = cur; // Save the currently defined project node.

    // Get the last of the same level list.
    projMap *last = cur_parser_p->get_map();

    if (last == 0)
        {
            // This is the first project in the same level.
            projLevel *up_parser_p = cur_parser_p->get_parent();

            // If this is the very first project, up_parser_p will be NULL.
            if (up_parser_p)
                {
                    last = up_parser_p->get_map();
                    // The new one is the first child of the "last" node of the above level.
                    last->set_child_project(cur);
                }
        }
    else
        {
            projLevel *up_parser_p = cur_parser_p->get_parent();

            if (up_parser_p == 0 && !parser_locate_error()) // root projmap
                {
                    if (last->is_script())
                        {
                            projMap *prev = last->get_prev_project();
                            last = pdf_flat_proj(last);
                            if (prev == 0)
                                proj_rule_head = last;
                            else
                                prev->set_next_project(last);
                        }
                }
            last->set_next_project(cur);
        }

    // save the last project of this level.
    cur_parser_p->set_map(cur);
}

static char get_par_proj_type()
{
    Initialize(get_par_proj_type);

    projLevel *up_parser_p = cur_parser_p->get_parent();
    if (up_parser_p == 0)
        return NO_IMPLICIT_DIR_PROJ;

    projMap *proj_head = up_parser_p->get_map();

    return proj_head->get_sub_proj_flag();
}

static char *cur_phy_name;
static char *cur_logic_name;

extern "C" void save_phy_logic_names(char *phy_name, char *logic_name)
{
    cur_phy_name   = phy_name;   // allocate in lexer and will be freed in crt_whole_proj();
    cur_logic_name = logic_name; // allocate in lexer and will be freed in crt_ hole_proj();
}


static bool contain_double_dots(char */* path */ )
{
    Initialize(contain_double_dots);
    return FALSE;
}

// The parser locates a new project definition.
extern "C" void crt_whole_proj(char *proj_name, char *phy_name, char *logic_name, char sub_proj_flag)
{
    Initialize(crt_whole_proj);

    genString phy_name_str = dequote_str(phy_name);
    psetfree(phy_name); // allocate in lexer
    phy_name = (char *)phy_name_str.str();

    if (contain_double_dots(proj_name) || contain_double_dots(phy_name) || contain_double_dots(logic_name))
        ; // return;

    if (cur_parser_p->get_level() == 0)
        touch_level = 0;

    if (logic_name && OSapi_strcmp(logic_name, subsystem_lname) == 0) {
        inside_subsystem_project = TRUE;
        msg("Subsystems rule in the pdf is obsolete. Please add directory where the groups are to be saved to \"groupsPath\" preference.", warning_sev) << eom;
    }
    else if (!inside_subsystem_project) {
        // PROJECT means project. 0: member is null.
        projMap *cur = db_new(projMap, (PROJECT, sub_proj_flag, get_tran_flag(), proj_name, phy_name, logic_name, SINGLE_CHILD, 1));
        link_current_project(cur);
    }

    if (proj_name)
        psetfree(proj_name);  // allocate in lexer
    if (logic_name)
        psetfree(logic_name); // allocate in lexer
}

// for top exclude projects -- BEGIN

typedef struct
{
    projMap *pm;
    WildPair *m;
} top_exc_proj_entry;

genArr(top_exc_proj_entry);

static genArrOf(top_exc_proj_entry) top_exc_proj_heads;
static int top_exc_proj_define_flag = 0;

static void init_top_exc_proj_heads()
{
    Initialize(init_top_exc_proj_heads);

    top_exc_proj_define_flag = 0;
    for (int i=0;i<top_exc_proj_heads.size();i++)
        {
            top_exc_proj_entry *p = top_exc_proj_heads[i];
            if (p->m)
                delete p->m;
        }
    top_exc_proj_heads.reset();
}

static void add_top_exc_proj_heads(projMap *ppm, char const *name)
{
    Initialize(add_top_exc_proj_heads);

    top_exc_proj_entry *p = top_exc_proj_heads.grow();
    if (ppm)
        {
            p->pm = ppm;
            p->m  = 0;
        }
    else
        {
            top_exc_proj_define_flag = 1;
            p->pm = 0;
            p->m = (WildPair *)db_new(WildPair, (name+1));
        }
}

extern "C" void save_top_exclude_proj(char *top_ex_proj)
{
    Initialize(save_top_exclude_proj);
     
    add_top_exc_proj_heads(0, top_ex_proj);
}


bool exclude_this_top_one(char const *path, projMap *ppm)
{
    Initialize(exclude_this_top_one);

    if (!top_exc_proj_define_flag)
        return false;

    for (int i=0;i<top_exc_proj_heads.size();i++)
        {
            top_exc_proj_entry *p = top_exc_proj_heads[i];
            if (p->m)
                {
                    int len = p->m->match(path,MATCH_WITHOUT_END);
                    if (len == strlen(path))
                        return true;
                }
            if (p->pm == ppm)
                return false;
        }
    return false;
}

// for top exclude projects -- END

// The parser locates a new project definition.
extern "C" void crt_proj(char *proj_name, char sub_proj_flag)
{
    crt_whole_proj(proj_name, cur_phy_name, cur_logic_name, sub_proj_flag);
}

static projMap *create_one_child_project(char proj_type, char *p, char child_type)
{
    Initialize(create_one_child_project);

    return db_new(projMap, (proj_type, NO_IMPLICIT_DIR_PROJ, TWO_WAY, 0, p, 0, child_type, 0));
}

static void create_children(char *p)
{
    Initialize(create_children);

    inc_level(0, -1);
    char *q = strchr(p,'/');
    if (q)
        {
            *q = '\0';
            projMap *cur = create_one_child_project(CREATED_PROJECT, p, MIDDLE_CHILD);
            link_current_project(cur);
            *q = '/';
            create_children(q+1);
        }
    else
        {
            projMap *cur = create_one_child_project(CREATED_MEMBER, p, BOTTOM_CHILD);
            link_current_project(cur);
            cur->set_member(p);
        }

    close_proj();
    dec_level(0, -1);
}

extern "C" void add_member(char *member, char new_lineno_flag)
// Parser locates a new member for the currently defined project "proj_rule_cur".
//
// member: is the physical name
// new_lineno_flag: if set, it is called from the yacc.
//     The line number is appended at the end of the physical name.
{
    Initialize(add_member);

    if (inside_subsystem_project)
        return;

    genString mem_str = dequote_str(member);
    psetfree(member); // allocate in lexer
    member = (char *)mem_str.str();

    if (contain_double_dots(member))
        ; // return;

    if (leading_char(member,'~'))
        {
            genString rr;
            convert_home_dir(member, rr);
            psetfree(member);
            member = strdup(rr.str());
        }

    if (new_lineno_flag) // update the line number
        obtain_path_parse_line_num(0, member, 0, 1);

    if (get_par_proj_type() == NO_IMPLICIT_DIR_PROJ ||
        get_par_proj_type() == IMPLICIT_DIR_PROJ && strstr(member,"**"))
        {
            char proj_type;
            proj_type = MEMBER;
            if (*member == '^')
                proj_type = EXCLUDE_PROJECT;

            projMap *cur = db_new(projMap, (proj_type, NO_IMPLICIT_DIR_PROJ, TWO_WAY, 0, member, 0, SINGLE_CHILD, 0));
            link_current_project(cur);
            if (*member == '^')
                cur->set_member(member+1);
            else
                cur->set_member(member);
        }
    else
        {
            // IMPLICIT_DIR_PROJ '=' is changed to
            // NO_IMPLICIT_DIR_PROJ ':' ==> CREATED_PROJECT or
            // NO_IMPLICIT_DIR_PROJ ':' ==> CREATED_MEMBER
            char *p = strchr(member,'/');
            projMap *cur;
            if (p)
                {
                    *p = '\0';
                    cur = create_one_child_project(CREATED_PROJECT, member, TOP_CHILD);
                }
            else
                {
                    cur = create_one_child_project(CREATED_MEMBER, member, SINGLE_CHILD);
                    cur->set_member(member);
                }

            link_current_project(cur);

            // take the child
            if (p)
                {
                    *p = '/'; // recover
                    create_children(p+1);
                }
        }
}

static void local_set_proj_file_type(char *file_selector, char *first_wrn)
{
    Initialize(local_set_proj_file_type);

    genString cnt;
    char selector_type = selector_analyze(file_selector, cnt);
    if (selector_type == EMPTY_TYPE)
        return;

    if (cnt.length())
        file_selector = strdup(cnt.str());
    else
        file_selector = strdup("");
    static selector_entry *last;
    if (multi_selectors.size() == 0)
        last = 0;

    selector_entry *p;
    if (!last || selector_type != PURE_FILE_TYPE)
        {
            p = multi_selectors.grow();
            p->selector_type  = selector_type;
            p->selector_def   = file_selector;
            p->read_only_flag = NO_RW_FLAG;
            p->warning_already= 0;
            if (selector_type != PURE_FILE_TYPE)
                {
                    last = 0;
                    return;
                }
        }
    else
        p = last;

    char file_type = 0;
    char *save_file_selector = file_selector;
    char *free_save_file_selector = file_selector;

    while(*file_selector)
        {
            if (*file_selector == 'f')
                file_type |= FILE_TYPE;
            else if (*file_selector == 'd')
                file_type |= DIR_TYPE;
            else if (*file_selector == 'F')
                file_type |= LINK_FILE;
            else if (*file_selector == 'D')
                file_type |= LINK_DIR;
            else if (*file_selector == 'R')
                {
                    if (p->read_only_flag == W_FLAG) // [[ W ]]
                        {
                            if (*first_wrn)
                                create_one_path_parser_error(PATH_PARSE_READ_ONLY_SUPESEDE_WRITE_ONLY_WRN, 0, 0, 0, 0);
                            *first_wrn = 0;
                        }
                    p->read_only_flag = R_FLAG;
                }
            else if (*file_selector == 'W')
                {
                    if (p->read_only_flag == R_FLAG) // [[ R ]]
                        {
                            if (*first_wrn)
                                create_one_path_parser_error(PATH_PARSE_READ_ONLY_SUPESEDE_WRITE_ONLY_WRN, 0, 0, 0, 0);
                            *first_wrn = 0;
                        }
                    else
                        p->read_only_flag = W_FLAG;
                }
            else
                {
                    if (save_file_selector) // only print error one time
                        create_one_path_parser_error(PATH_PARSE_UNKNOWN_SELECTOR_ERR, save_file_selector, 0, 0, 0);
                    save_file_selector = 0;
                }
            file_selector++;
        }

    if (!last)
        {
            p->parser_file_type = file_type;
            last = p;
        }
    else
        {
            p->parser_file_type &= file_type;
            psetfree(free_save_file_selector);
        }
}

extern "C" void set_proj_file_type(char *file_selector)
{
    Initialize(set_proj_file_type);

    genString buf = file_selector;
    psetfree(file_selector);

    char *p = (char *)buf.str();
    char first_wrn = 1;
    while(1)
        {
            while(isspace(*p))
                p++;
            if (strncmp(p,"[[",2))
                {
                    create_one_path_parser_error(PATH_PARSE_SYNTAX_ERR, "[[", 0, 0, 0);
                    return;
                }
            char *q;
            char *s = p;
            while(1)
                {
                    q = strstr(s,"]]");
                    if (q == 0 || *(q-1) != '\\')
                        break;
                    s = q+2;
                }
            if (!q)
                {
                    create_one_path_parser_error(PATH_PARSE_SYNTAX_ERR, "]]", 0, 0, 0);
                    return;
                }
            q += 2;
            char temp = *q;
            *q = '\0';
            local_set_proj_file_type(p, &first_wrn);
            if (temp == '\0')
                break;
            *q = temp;
            p = q;
        }
    if (!first_wrn)
        {
            selector_entry *p = multi_selectors[0];
            p->warning_already = 1;
        }
}

// PROJMAP
void projMap::search_modules_by_name_recursive(projNode *proj, 
                                               char const *logic_name, char const *full_proj_name, 
                                               char const *proj_name, symbolArr &s1, int deep_flag)
// search all modules that define a sub-project name "proj_name" and belong to the projNode "proj"
// all modules returned by "s"
// logic_name: the logic name so far
// full_proj_name: the full project name so far
// deep_flag = 0: go through the link for next projects
//           = 1: do not go through the link for next projects
{
    Initialize(projMap::search_modules_by_name_recursive);

    wkIdxRule *pr_rule  = get_project_rule();
    wkIdxRule *ln_rule = get_logic_rule();

    genString cur_proj_name;
    genString cur_logic_name;
    
    // sioffe:: this piece is changed because get_phy_rule(0)->get_org_ptn() 
    // gives a physical name, not a subproject name. Instead, we always use
    // get_ptn() and then remove parentheses, meaning: replace things like
    // $EnvVar and (1) in the subproject name, then remove parentheses.

    cur_proj_name = pr_rule->get_ptn();
    cur_logic_name = ln_rule->get_ptn();
    remove_back_slash_and_parentheses((char *)cur_proj_name.str());
    remove_back_slash_and_parentheses((char *)cur_logic_name.str());

    projMap *child = get_child_project();
    genString new_logic_name;
    combine_dir_and_sub_dir(logic_name, cur_logic_name, new_logic_name);
    if (child == 0)
        {
            if (strcmp(cur_proj_name,proj_name) == 0)
                {
                    proj->force_refresh(1);
                    projModule *mf = proj->find_module(new_logic_name);
                    if (mf)
                        s1.insert_last(mf);
                }
        }
    else
        {
            genString new_full_proj_name;
            combine_dir_and_sub_dir(full_proj_name, cur_proj_name, new_full_proj_name);
            if (strcmp(cur_proj_name,proj_name) == 0)
                {
                    projNode *child_node = proj->find_project(new_full_proj_name, new_logic_name);
                    // if child_node == 0, this rule might be a subset of some pre-rule.
                    if (child_node)
                        child_node->search_all_modules(s1); // All modules are OK
                }
            else
                child->search_modules_by_name_recursive(proj, new_logic_name, new_full_proj_name, proj_name, s1, 0);
        }

    if (deep_flag)
        return;

    if (get_parent_project() != 0)
        {
            projMap *next = get_next_project();
            while (next)
                {
                    next->search_modules_by_name_recursive(proj, logic_name, full_proj_name, proj_name, s1, 1);
                    next = next->get_next_project();
                }
        }
}

void projMap::search_modules_by_name(projNode *proj, char const *proj_name, 
                                     symbolArr &s)
// search all modules that define a sub-project name "proj_name" and 
// belong to the projNode "proj"
// all modules returned by "s"
{
    Initialize(projMap::search_modules_by_name);

    if (!proj->is_script())
        search_modules_by_name_recursive(proj, "", "", proj_name, s, 0);
    else
        {
            path_hash_entry **hh = get_hash_head(FILE_TYPE, LOGIC_NAME_RULE, FNTOLN);
            if (hh == 0)
                return;

            for (int idx=0; idx<MAX_PATH_HASH; idx++)
                {
                    path_hash_entry *cur = hh[idx];
                    if (cur == 0)
                        continue;

                    while(cur)
                        {
                            if (strstr(cur->get_proj_n(),proj_name))
                                {
                                    projModule *mf = proj->search_module_hash(cur->get_ln());
                                    if (mf)
                                        s.insert_last(mf);
                                }
                            cur = cur->get_next();
                        }
                }
        }
}

bool projMap::need_verify_fn_to_ln()
{
    Initialize(projMap::need_verify_fn_to_ln);

    if (get_child_project() || is_implicit_sub_dir_proj()) // directory
        {
            if (get_file_type() != (DIR_TYPE | LINK_DIR))
                return TRUE;
        }
    else // file
        {
            if (get_file_type() != (FILE_TYPE | LINK_FILE))
                return TRUE;
        }
    if (get_multi_phy_rule()->size() > 1)
        return TRUE;
    return FALSE;
}

bool projMap::selector_defined()
// if a selector is defined but it is not [[ dDfF ]], a script, or a filter
{
    Initialize(projMap::selector_defined);

    if (get_multi_phy_rule()->size() > 1)
        {
            for (int i=1; i<get_multi_phy_rule()->size(); i++)
                {
                    char tp = get_phy_rule_type(i);
                    if (tp != MAP_SCRIPT_TYPE && tp != MAP_FILTER_TYPE && tp != END_PHY_PATH)
                        return TRUE;
                }
        }

    return FALSE;
}

bool projMap::selector_matched_pattern()
{
    Initialize(projMap::selector_matched_pattern);

    for (int i=1; i<get_multi_phy_rule()->size(); i++)
        {
            if (get_phy_rule(i)->get_max_ptn_count())
                return TRUE;
        }

    return FALSE;
}

bool projMap::contain_sub_proj()
{
    Initialize(projMap::contain_sub_proj);

    if (is_leaf())
        {
            if (get_sub_proj_flag() == IMPLICIT_DIR_PROJ)
                return TRUE;
            else
                return FALSE;
        }

    projMap *child = get_child_project();
    while(child)
        {
            if (!child->is_leaf() ||
                get_sub_proj_flag() == IMPLICIT_DIR_PROJ ||
                child->get_sub_proj_flag() == IMPLICIT_DIR_PROJ)
                return TRUE;

            child = child->get_next_project();
        }
    return FALSE;
}

void projMap::get_physical_path(int idx, genString &s)
{
    Initialize(projMap::get_physical_path);

    s = 0;
    projMap *par = get_parent_project();

    if (idx == -1) // (0)
        {
            while(par)
                {
                    genString ss;

                    if (s.length() == 0)
                        s = par->get_map_phy_path();
                    else
                        {
                            ss.printf("%s/%s",par->get_map_phy_path(),s.str());
                            s = ss.str();
                        }

                    if (leading_char((char *)s.str(), '/'))
                        return;
                    par = par->get_parent_project();
                }
        }
    else
        {
            while(par)
                {
                    idx++;
                    if (idx == -1)
                        s = par->get_map_phy_path();

                    if (leading_char(par->get_map_phy_path(), '/'))
                        return;
                    par = par->get_parent_project();
                }
        }
}

wkPtnRule *projMap::get_phy_rule(int idx)
{
    Initialize(projMap::get_phy_rule);

    genArrOf(phy_rule_entry) *mr = get_multi_phy_rule();

    phy_rule_entry *p = (*mr)[idx];

    if (p)
        return (p->phy_rule); // For MAP_FILE_TYPE will also return 0
                              // get_phy_rule_type() will return MAP_FILE_TYPE

    return 0; // out of range
              // get_phy_rule_type() will return END_PHY_PATH
}

int projMap::get_phy_file_type(int idx)
{
    Initialize(projMap::get_phy_file_type);

    genArrOf(phy_rule_entry) *mr = get_multi_phy_rule();

    phy_rule_entry *p = (*mr)[idx];

    if (p)
        return (p->file_type);

    return END_FILE_TYPE; // out of range
}

char projMap::get_phy_rule_type(int idx)
{
    Initialize(projMap::get_phy_rule_type);

    genArrOf(phy_rule_entry) *mr = get_multi_phy_rule();

    phy_rule_entry *p = (*mr)[idx];

    if (p)
        return (p->rule_type);

    return END_PHY_PATH; // out of range
}

void remove_back_slash_and_parentheses(char *s)
{
    Initialize(remove_back_slash_and_parentheses);

    char *p = s;
    while(*s)
        {
            if (*s == '\\')
                {
                    s++;
                    *p++ = *s++;
                }
            else if (*s == '(' || *s == ')')
                s++;
            else
                *p++ = *s++;
        }
    *p = '\0';
}

void projMap::set_leading_slash_in_child_phy_path()
{
    Initialize(projMap::set_leading_slash_in_child_phy_path);

    leading_slash_in_child_phy_path = 1;
}

bool projMap::is_leading_slash_in_child_phy_path()
{
    Initialize(projMap::is_leading_slash_in_child_phy_path);

    return(leading_slash_in_child_phy_path);
}

bool projMap::is_leading_slash_in_phy_path()
{
    Initialize(projMap::is_leading_slash_in_phy_path);

    wkPtnRule *rule = get_phy_rule(0);
    return phy_path_with_leading_slash(rule);
}


bool projMap::is_idx_contain_match_index()
// if the logic name or the project name needs mapping (not constant string)
{
    Initialize(projMap::is_idx_contain_match_index);

    if (get_project_rule()->idx_is_defined() == 0 || get_logic_rule()->idx_is_defined() == 0)
        return TRUE;

    wkIdxRule *rule = get_project_rule();
    if (rule->idx_is_defined())
        {
            if (contain_target_char(rule->get_org_idx(),'(', 0))
                return TRUE;
        }
    rule = get_logic_rule();
    if (rule->idx_is_defined())
        {
            if (contain_target_char(rule->get_org_idx(),'(', 0))
                return TRUE;
        }

    return FALSE;
}

void projMap::set_member(char const *member)
{
    Initialize(projMap::set_member);

    if (member && strncmp(member,"..",2) == 0) {
        //      if (parse_is_being_executed)
        //          create_one_path_parser_error(PATH_PARSE_DOUBLE_DOTS_WRN, 0, 0, 0, 0);
        return;
    }

    if (member == 0) {
        member_filter = 0;
        return;
    }

    genString mm;
    char *tmp = strdup(member);
    char *m = tmp;

    if (strlen(m) > 2 && strncmp(m,"./",2) == 0)
        *m = '*';
    else if (strcmp(m,".") == 0 || strcmp(m,"./") == 0) {
        add_member(strdup("*/"), 0);
        strcpy(m,"*");
    } else {
        project_path_sys_vars_translate(m, mm);
        m = (char *)mm.str();
    }

    if (leading_char(m,'/'))
        m++;

    projFilter *last = 0;
    projFilter *pf;

    while(1) {
        char *p = strchr(m,'/');
        if (p)
            *p = '\0';

        pf = db_new(projFilter,(m));

        if (last)
            last->set_next(pf);
        else
            member_filter = pf;

        last = pf;

        if (p) {
            *p = '/'; // recover
            m = p+1;
        } else
            break;
    }
    free(tmp);
}

static void create_idx_str(int count, genString &s)
{
    Initialize(create_idx_str);

    for (int i=1;i<count+1;i++)
        {
            if (i==1)
                s = "(";
            else
                s += "/(";

            char buf[4];
            OSapi_sprintf(buf,"%d)",i);

            s += buf;
        }
}

// UTILITY

static void log_link_file_name(char const *mesg, char const *name)
{
    Initialize(log_link_file_name);

    if (customize::getIntPref("DIS_internal.PDFCheck_Enabled") > 0) {
        if (!name)
            name = "(null)";
        msg("$1 ($2)") << mesg << eoarg << name << eom;
        ;
    }
}

extern "C" int remove_all_dots_and_double_slashes (char const *path, char *resolved_path);

extern "C" char const *paraset_realpath(char const *path, char *ret_buf)
{
    Initialize(paraset_realpath);

    if (path == 0 || ret_buf == 0 || *path == 0)
        return 0;

    char pathname[MAXPATHLEN];
    if (*path != '/') {
	genString ret_path;
	convert_home_dir(path, ret_path);
	path = ret_path;
	project_path_sys_vars_translate(path, ret_path);
	path = ret_path;

	if (*path == '/')
	    strcpy(pathname,path);
	else {
	    char *p = (char *)OS_dependent::getwd(pathname);
	    if (p == 0)
		return 0;
	    p += strlen(p);
	    *p++ = '/';
	    strcpy(p,path);
        }
    } else
	strcpy(pathname,path);

    int res = remove_all_dots_and_double_slashes (pathname, ret_buf);
    if (res != 0)
        ret_buf = 0;

    return ret_buf;
}

int paraset_get_fullpath(char const *path, genString &new_path)
{
    Initialize(paraset_get_fullpath);

    char pathname[MAXPATHLEN];
    if (paraset_realpath(path, pathname) == 0)
        {
            new_path = path;
            return 0;
        }

    new_path = pathname;
    return 1;
}

//
// rule_type: NORMAL_PHY_PATH   for the normal one
//            LINK_MAP_PHY_PATH for [[ -> ]]
//            REALPATH_MAP_PHY_PATH for [[ => ]]
//            SEARCH_PHY_PATH for [[ ls ]]
//            EQUAL_EVAL     [[ == (1) (2) ]]
//            NOT_EQUAL_EVAL [[ != (1) (2) ]]
//
//            MAP_FILE_TYPE for [[ fFdD ]]
//
static bool is_phy_rule_defined(int rule_type)
{
    Initialize(projMap::is_phy_rule_defined);

    if (rule_type == NORMAL_PHY_PATH       ||
        rule_type == LINK_MAP_PHY_PATH     ||
        rule_type == REALPATH_MAP_PHY_PATH ||
        rule_type == TCL_BOOLEAN_EVAL      ||
        rule_type == EQUAL_EVAL            ||
        rule_type == NOT_EQUAL_EVAL        ||
        rule_type == SEARCH_PHY_PATH)
        return TRUE;
    else
        return FALSE;
}

int projMap::create_one_phy_rule(char rule_type, char const *phy_name, char direction_flag, int file_tp,
                                 char idx_const_string, int exclude_type)
// create one physical rule
//
// rule_type = NORMAL_PHY_PATH: normal physical path definition
//           = LINK_MAP_PHY_PATH: if it is a link mapping selector [[ -> ... ]]
//           = REALPATH_MAP_PHY_PATH: if it is a realpath mapping selector [[ => ... ]]
//           = SEARCH_PHY_PATH: if it is a search selector [[ ls ... ]]
//           = MAP_FILE_TYPE: if it is a file type selector [[ dDfF ]] selector
//           = MAP_SCRIPT_TYPE: if this is a script project
//           = MAP_FILTER_TYPE: if this is a filter project
//           = END_PHY_PATH: get_phy_rule_type(int idx) will return END_PHY_PATH if idx is out of range
//           = EQUAL_EVAL     [[ == (1) (2) ]]
//           = NOT_EQUAL_EVAL [[ != (1) (2) ]]
//
// phy_name: the defined physical path
//
// direction_flag: TWO_WAY (<->), RIGHT_WAY (->), LEFT_WAY (<-), or REAL_RIGHT_WAY (=>)
//
// file_type: FILE_TYPE: file
//            DIR_TYPE: directory
//            LINK_FILE: link file
//            LINK_DIR: link directory
//            DEFAULT_TYPE: no [[ dDfF ]] defined
//            GARBAGE_FILE_TYPE: should be DEFAULT_TYPE
//            PARTIAL_DIR_TYPE: only check partial directory name
//            END_FILE_TYPE: get_phy_file_type(int idx) will return END_FILE_TYPE if idx is out of range
//
// idx_const_string = 0: if the physical path does not contain any () then each portion of path between two constiguous
//                       slashes will be added and surrounded by ()
//                  = 1: do nothing
// exclude_type = NON_EXCLUDE: not a exclude project
//              = EXCLUDE_DIR: exclude directories
//              = EXCLUDE_FILE: exclude files
//              = EXCLUDE_PAR_IMP: special to exclude a partial directioy for the implicit project "="
{
    Initialize(projMap::create_one_phy_rule);

    phy_rule_entry *p = multi_phy_rule.grow();
    int link_phy_rule = 0;
    if (multi_phy_rule.size() > 1)
        link_phy_rule = 1;
    p->rule_type = rule_type;
    p->file_type = file_tp;

    if (rule_type == MAP_SCRIPT_TYPE || rule_type == MAP_FILTER_TYPE)
        p->phy_rule = db_new(wkPtnRule,(exclude_type, phy_name,0,rule_type,0,link_phy_rule));
    else if (is_phy_rule_defined(rule_type) && phy_name)
        p->phy_rule = db_new(wkPtnRule,(exclude_type, phy_name, direction_flag, rule_type,
                                        idx_const_string || selector_matched_pattern(),link_phy_rule));
    else
        p->phy_rule = 0;

    if (p->phy_rule && p->phy_rule->failed())
        return 1;

    return 0;
}

void projMap::get_proj_idx_default(genString &s)
{
    Initialize(projMap::get_proj_idx_default);

    // use logic name as default
    if (logic_rule->idx_is_defined())
        s = logic_rule->get_idx_def();
    else
        get_logic_idx_default(s);
}

int projMap::get_phy_match_count()
// count the number of the defined matched matterns (include **)
{
    Initialize(projMap::get_phy_match_count);

    int count = 0;
    for (int i=0; i<multi_phy_rule.size(); i++)
        {
            phy_rule_entry *p = multi_phy_rule[i];
            int rule_type = p->rule_type;
            if (is_phy_rule_defined(rule_type))
                count += p->phy_rule->get_max_ptn_count() + p->phy_rule->count_const_two_star();
        }
    return count;
}

void projMap::get_logic_idx_default(genString &s)
{
    Initialize(projMap::get_logic_idx_default);

    int count = get_phy_match_count();
    if (count)
        create_idx_str(count,s);
}

void projMap::fill_ptn_strs()
{
    Initialize(projMap::fill_ptn_strs);

    int match_count = get_phy_match_count();
    init_working_buffer(match_count);
    int idx = 0;
    for (int i=0; i<get_multi_phy_rule()->size(); i++)
        {
            wkPtnDef *cur = get_phy_rule(i)->get_ptn_strs();
            while(cur)
                {
                    if (cur->get_type() == PTN_MATCH_MAPPING)
                        str_buf[idx++] = strdup(cur->get_str());
                    cur = cur->get_next();
                }
        }
}

void projMap::replace_phy_idx()
// It needs to replace the idx field in the physical path to some pre-defined matched pattern
{
    Initialize(projMap::replace_phy_idx);

    genArrOf(phy_rule_entry) *mr = get_multi_phy_rule();

    for (int i=0; i<get_multi_phy_rule()->size(); i++)
        {
            phy_rule_entry *p = (*mr)[i];

            wkPtnDef *cur = p->phy_rule->get_ptn_strs();
            int rule_type = p->rule_type;
            if (rule_type == MAP_SCRIPT_TYPE || rule_type == MAP_FILTER_TYPE)
                continue;
            while(cur)
                {
                    if (cur->get_type() == PTN_IDX)
                        p->phy_rule->update_idx_ptn();
                    cur = cur->get_next();
                }
        }
}

// Any routine calls this, should free the returned space.
void combine_dir_and_sub_dir(char const *cur_dir, char const *sub_dir, 
                             genString &ret)
{
    Initialize(combine_dir_and_sub_dir);

    if (leading_char (sub_dir, '/'))
        {
            ret = sub_dir;
            return;
        }

    ret = cur_dir;

    // append subdirectory with slash if necessary
    if (sub_dir && *sub_dir)
        {
            if (!trailing_char(ret, '/'))
                ret += "/";

            ret += sub_dir;
        }
}

bool projMap::match_file_type(char const *file, char file_tp, int full_match_flag, int *link_info,
                              char realpath_flag, char ignore_this_node, int xx_f)
// If this node does not require file type check, it returns TRUE.
// If this node does require file type check,
//    and the file pp matches the file type, it will return TRUE; Otherwise,
//    it will return FALSE.
//
// For link map FILE ATTRIBUTE, the file type of this node should not be consider.
//
// file: file name which is checked
// file_type: the file type for this file
// full_match_flag: if not set, ignore the data in the projmap
// link_info: This file is a link but the file type is [[ fd ]]. No link is allowed.
// realpath_flag: if set, ignore the data in the projmap
// ignore_this_node: if set, don"t consider the data in the projmap
// xx_f: if set, it will take link file.
//       if not set, does the normal check.
{
    Initialize(projMap::match_file_type);

    //  if (!need_to_check_file_type())
    //      return TRUE; // non-existing file

    if (get_file_type() == DEFAULT_TYPE)
        return TRUE;

    int status;
    struct OStype_stat buf;
    status = fileCache_stat(file,&buf);
    struct OStype_stat bufl;
    if (status == 0)
        status = fileCache_lstat(file,&bufl);

    if (status == -1) // non-existing file
        {
            if (is_file() && (file_tp & FILE_TYPE)
                || (is_dir() || !full_match_flag) && (file_tp & DIR_TYPE))
                return TRUE;
            else
                return FALSE;
        }

    if (ignore_this_node)
        {
#ifndef _WIN32
            if (!S_ISLNK(bufl.st_mode) && ((file_tp & FILE_TYPE) || (file_tp & DIR_TYPE)) ||
                S_ISLNK(bufl.st_mode) && ((file_tp & LINK_FILE) || (file_tp & LINK_DIR)))
#else
                if (((file_tp & FILE_TYPE) || (file_tp & DIR_TYPE)))
#endif
                    return TRUE;
                else
                    return FALSE;
        }
    else
        {
#ifndef _WIN32
            if (!S_ISLNK(bufl.st_mode))
                {
#endif
#ifdef _WIN32
                    if (((buf.st_mode & _S_IFREG) != 0) && (is_reg_file() || realpath_flag) && file_tp == FILE_TYPE
                        || ((buf.st_mode & _S_IFDIR) != 0) && (is_reg_dir() || !full_match_flag) && file_tp == DIR_TYPE)
                        return TRUE;
#else
                    if (S_ISREG(buf.st_mode) && (is_reg_file() || realpath_flag) && file_tp == FILE_TYPE
                        || S_ISDIR(buf.st_mode) && (is_reg_dir() || !full_match_flag) && file_tp == DIR_TYPE)
                        return TRUE;
#endif
#ifndef _WIN32
                }
#endif
#ifndef _WIN32
            if (S_ISLNK(bufl.st_mode))
                {
                    if (xx_f)
                        return TRUE;

                    if (S_ISREG(buf.st_mode) && is_link_file() && file_tp == FILE_TYPE 
                        || S_ISDIR(buf.st_mode) && (is_link_dir() || !full_match_flag) && file_tp == DIR_TYPE) {
                        if (link_info)
                            *link_info = FALSE;

                        return TRUE;
                    } else if (link_info)
                        *link_info = TRUE; // This file is a link but the file type is [[ fd ]]. No link is allowed.
                }
#endif
        }
    return FALSE;
}

void projMap::set_this_is_target(int v)
{
    Initialize(projMap::set_this_is_target);

    this_is_target = v;

    projMap *par = get_parent_project();
    if (!par)
        return;

    while(par->get_parent_project())
        par = par->get_parent_project();
    par->set_this_is_target(v);
}

bool projMap::check_this_is_target()
// if 1. root is target
//    2. this is a leaf
//    3. this is not a target
//    return FALSE;
// otherwise return TRUE.
{
    Initialize(projMap::check_this_is_target);

    if (get_child_project() || get_this_is_target())
        return TRUE;

    projMap *par = this;
    while(par->get_parent_project())
        par = par->get_parent_project();
    if (!par->get_this_is_target())
        return TRUE;
    return FALSE;
}

// either no children or all children are excluding projects
int projMap::imp_without_real_filter()
{
    Initialize(projMap::imp_without_real_filter);

    if (get_sub_proj_flag() == IMPLICIT_DIR_PROJ)
        {
            projMap *child = get_child_project();
            if (!child)
                return TRUE;
            while(child)
                {
                    if (child->get_view_flag())
                        return FALSE;
                    child = child->get_next_project();
                }
            return TRUE;
        }

    return FALSE;
}

// It will return TRUE if it is an implicit project and its sub-projects are MEMBER
// Otherwise, it will return false;
int projMap::is_imp_leaf()
{
    Initialize(projMap::is_imp_leaf);

    if (get_sub_proj_flag() == IMPLICIT_DIR_PROJ &&
        get_child_project() &&
        get_child_project()->get_flag() == EXCLUDE_PROJECT)
        return TRUE;
    return FALSE;
}

bool projMap::is_invisible_project()
{
    Initialize(projMap::is_invisible_project);

    char const *p = get_org_project_name();
    if (!p || strncmp(p,"__",2))
        return FALSE;
    return TRUE;
}


static bool child_with_slash(projMap *child, char rule_type, char tran_type)
{
    Initialize(child_with_slash);

    if (child) // && child->is_project())
        {
            // This child starts with a '/'
            if (tran_type == FNTOLN)
                {
                    wkPtnRule *child_ptn_rule = child->get_phy_rule(0);

                    wkPtnDef *cur = child_ptn_rule->get_ptn_strs();
                    if (cur->get_type() == PTN_CONST_STRING && *cur->get_str() == '/')
                        return TRUE;
                }
            else
                {
                    wkIdxRule *child_idx_rule;

                    if (rule_type == PROJECT_NAME_RULE)
                        child_idx_rule = child->get_project_rule();
                    else
                        child_idx_rule = child->get_logic_rule();

                    wkIdxDef *cur = child_idx_rule->get_idx_strs();
                    if (cur->get_type() == IDX_CONST_STRING && *cur->get_str() == '/')
                        return TRUE;
                }
        }
    return FALSE;
}

static void go_thru_next_p2i(projMap *cur, char const *path, char const *org_path, char const *rem_path, genString &ret_path,
                             char const *idx_name, char file_tp, int level, int complete_len, char const *rule,
                             char rule_type, int *link_reject_flag, char realpath_flag,
                             int *no_pre_pend_flag, int *reject_by_exclude, projNode **pr)
// try to map the next projects
//
// cur: the current projMap
// path: the physical path for this sub_proj
// org_path: the whole_path
// rem_path: the unmatched tail
// ret_path: the matched logic (project) path
// idx_name: the sub_project_name needs to be satisfied
// file_type: either FILE_TYPE for file or DIR_TYPE for directory
//
// The first call to ptn_to_idx in "projMap::fn_to_ln" passes level 0.
//    When it comes to go_thru_next_p2i, the level is 1, reject it.
//    The next of the "top_level_project" should be NULL any way.
//
// The child->ptn_to_idx in ptn_to_idx itself, passes level 1.
//    When it comes to go_thru_next_p2i, the level is 2, it is the only
//    accepted case.
//
// The next->ptn_to_idx in go_thru_next_p2i, passes level 2.
//    When it comes to go_thru_next_p2i, the level is 3, reject it to
//    avoid the unnecessary recursive.
//
// complete_len: The length of the matched path
// rule: is the rules used so far
// rule_type: PROJECT_NAME_RULE => convert physical to project
//            LOGIC_NAME_RULE => convert physical to logic
// link_reject_flag: will be set by ptn_to_idx if file type does not match.
//                   This file is a link but the file type is [[ fd ]]. No link is allowed.
// realpath_flag: if set, the file could be accepted in project node level
//                which is set to 0 now
// no_pre_pend_flag = 1 for rule; forget the path of parents;; Otherwise it is 0
// reject_by_exclude = 1; this file is rejected by some excluded rule
// pr: if *pr is 0, which is called from projHeader and need pr returned.
//     use projHeader do the second search
//   : if *pr is not 0, no pr will be returned and use pr for second search
{
    Initialize(go_thru_next_p2i);

    if (level != 2)
        {
            ret_path = 0;
            return;
        }

    while (1)
        {
            if (cur->get_this_is_target())
                {
                    ret_path = 0;
                    return;
                }

            cur = cur->get_next_project();
            if (cur == 0)
                {
                    ret_path = 0;
                    return;
                }

            int start_new = child_with_slash(cur, chosen_rule, FNTOLN);

            if (start_new)
                cur->ptn_to_idx(org_path, org_path, rem_path, ret_path, idx_name, file_tp, 2, complete_len,
                                rule, rule_type, link_reject_flag, realpath_flag, no_pre_pend_flag,
                                reject_by_exclude, pr);
            else
                cur->ptn_to_idx(path, org_path, rem_path, ret_path, idx_name, file_tp, 2, complete_len, rule,
                                rule_type, link_reject_flag, realpath_flag, no_pre_pend_flag,
                                reject_by_exclude, pr);

            if ((*reject_by_exclude /* && cur->is_exclude_proj() */ ) || *link_reject_flag || ret_path.str())
                break;

            *reject_by_exclude = 0;
        }
}

static void go_thru_child_then_next_p2i(
                                        projMap *cur,
                                        char const */* path */ ,
                                        char const *org_path,
                                        char const *rem_path,
                                        genString &ret_path,
                                        char const *idx_name,
                                        char const *sub_idx_name,
                                        char file_tp,
                                        int level,
                                        int complete_len,
                                        char const *rule,
                                        char rule_type,
                                        int *link_reject_flag,
                                        wkIdxRule *cur_idx_rule,
                                        char realpath_flag,
                                        int *no_pre_pend_flag,
                                        int *reject_by_exclude,
                                        projNode **pr)
// try to map the child projects then try the next projects
//
// cur: the current projMap
// org_path: the whole_path
// rem_path: the unmatched tail
// ret_path: the matched logic (project) path
// idx_name: the sub_project_name needs to be satisfied
// sub_idx_name: is the tail portion of idx_name. idx_name = index name of this node(cur) + sub_idx_name
// file_type: either FILE_TYPE for file or DIR_TYPE for directory
// level: for recursive count
// complete_len: The length of the matched path
// rule: is the rules used so far
// rule_type: PROJECT_NAME_RULE => convert physical to project
//            LOGIC_NAME_RULE => convert physical to logic
// link_reject_flag: will be set by ptn_to_idx if file type does not match.
//                   This file is a link but the file type is [[ fd ]]. No link is allowed.
// cur_idx_rule: the using idx rule
// realpath_flag: if set, the file could be accepted in project node level
//                which is set to 0 now
// no_pre_pend_flag = 1 for rule; forget the path of parents;; Otherwise it is 0
// reject_by_exclude = 1; this file is rejected by some excluded rule
// pr: if *pr is 0, which is called from projHeader and need pr returned.
//     use projHeader do the second search
//   : if *pr is not 0, no pr will be returned and use pr for second search
{
    Initialize(go_thru_child_then_next_p2i);

    // When come here, it fails in the cur node.
    // Hence go thru the child by the original path "org_path" to see if the child starts with
    //    a slash.
    // Go thru the next node with "path" to see if the next node accepts it.

    // If the path starts from '/', the child might start from '/' too.
    projMap *child = cur->get_child_project();

    if (cur->is_leading_slash_in_child_phy_path() && child)
        {
            genString s;
            update_the_using_rule(s, rule, cur);

            while(child)
                {
                    genString save_tail_portion = tail_portion.str();
                    int start_new = child_with_slash(child, chosen_rule, FNTOLN);

                    // It starts from the root, nothing is mapped yet. Hence the complete length should be 0.
                    if (start_new)
                        child->ptn_to_idx(org_path, org_path, org_path, ret_path, sub_idx_name, file_tp, 1, 0, s,
                                          rule_type, link_reject_flag, realpath_flag, no_pre_pend_flag,
                                          reject_by_exclude, pr);
                    else
                        go_thru_child_then_next_p2i(child, org_path, org_path, org_path, ret_path, idx_name,
                                                    sub_idx_name, file_tp, level, complete_len, s, rule_type,
                                                    link_reject_flag, cur_idx_rule, realpath_flag,
                                                    no_pre_pend_flag, reject_by_exclude, pr);

                    if ((*reject_by_exclude && child->is_exclude_proj()) || ret_path.length())
                        break;
                    else
                        *reject_by_exclude = 0;

                    tail_portion = save_tail_portion.str();
                    child = child->get_next_project();
                }
        }

    if (ret_path.length() == 0)
        {
            go_thru_next_p2i(cur, rem_path, org_path, rem_path, ret_path, idx_name, file_tp, level,
                             complete_len, rule, rule_type, link_reject_flag, realpath_flag, no_pre_pend_flag,
                             reject_by_exclude, pr);
            return;
        }

    wkIdxDef *cur_idx = cur_idx_rule->get_idx_strs();
    genString full_path;
    genString projn0;
    genString logn0;
    if (*no_pre_pend_flag == 0)
        {
            if (rule_type == PROJECT_NAME_RULE && cur_idx->get_str() == 0)
                {
                    if (idx_name == 0)
                        {
                            if ( *org_path != '/' )
                                {
                                    ret_path = 0;
                                    return;
                                }

                            cur->get_defined_name(projn0, logn0);
                            idx_name = projn0.str();
                        }
                }
            else
                {
                    idx_name = cur_idx->get_str();
                    if ( idx_name == 0 && *org_path == '/' )
                        {
                            cur->get_defined_name(projn0, logn0);
                            idx_name = (char *)projn0.str();
                        }
                }

            combine_dir_and_sub_dir(idx_name, (char *)ret_path.str(), full_path);
            ret_path = full_path.str();
        }
}

static int search_path(char *path, wkPtnRule *cur_ptn_rule, char *link_path)
// search the path for ls ...
//
// path: current path. The directory will be obtained from it. cd there.
//       Then do ls.
// cur_ptn_rule: the current physical path definition
// link_path: it will contain the obtained path
{
    Initialize(search_path);

    link_path[0] = '\0';
    if (trailing_char(path,'/'))
        return 0;

    struct OStype_stat buf;
    char *q = strrchr(path,'/');
    if (q)
        *q = '\0';

    // if it is not a directory
#ifdef _WIN32
    if (fileCache_stat(path, &buf) || buf.st_mode & _S_IFDIR == 0)
        return 0;
#else
    if (fileCache_stat(path, &buf) || !S_ISDIR(buf.st_mode))
        return 0;
#endif

    genArrCharPtr charptr_arr;
    
    wkPtnDef *cur = cur_ptn_rule->get_ptn_strs();
    while(cur)
        {
            if (cur->get_type() == PTN_IDX) {
                char *tmp = str_buf[cur->get_idx()];
                charptr_arr.append ( &tmp );
            } else {
                char const *tmp = cur->get_str();
                charptr_arr.append ( &tmp );
            }
            cur = cur->get_next();
        }

    genTmpfile s("pathC");
    char const **ls_args = (char const **)charptr_arr[0];
    int tmp_stat = shell_cd_ls ( s.name(), path, charptr_arr.size(), ls_args );

    if (q)
        *q = '/';

    FILE *in = s.open_for_read();
    if (in == 0)
        return 0;

    OSapi_fgets(link_path,MAXPATHLEN,in);
    s.close();
    s.unlink();
    link_path[MAXPATHLEN-1] = '\0';
    trn_path_2DIS( link_path );

    if (strlen(link_path) == 0 || strlen(link_path) == MAXPATHLEN-1)
        return 0;

    // remove the line feed
    link_path[strlen(link_path)-1] = '\0';
    return 1;
}

// Returns 1 on success
static int selector_substitute (genString& cmd, wkPtnRule *rul)
{
    Initialize(selector_substitute__genString);
    int res = 0;
    wkPtnDef *cur = (rul) ? rul->get_ptn_strs() : 0;
    cmd = NULL;
    while(cur)
        {
            if (cur->get_type() == PTN_IDX)
                cmd += str_buf[cur->get_idx()];
            else
                cmd += cur->get_str();
            cur = cur->get_next();
        }
    res = (cmd.length()) ? 1 : 0;
    return res;
}

static int selector_tcl_eval(char *path, wkPtnRule *cur_ptn_rule)
// evaluates the TCL expression specified in selector
//
// path: current path.
// cur_ptn_rule: the current physical path definition
// Returns the boolean evaluation result (0 *or* 1)
{
    Initialize(selector_tcl_eval);
    static Tcl_Interp *tcl = Tcl_CreateInterp();
    genString err_msg;
    int res = 0;
    if (tcl) {
	genString txt;
	if (selector_substitute (txt, cur_ptn_rule)) {
	    int tcl_stat = Tcl_ExprBoolean(tcl, (char *)txt, &res);
	    if (PDF_DebugLevel > 0) {
		msg("$1: tcl [[ $2 ]].Status: $3 Result: $4") << (char const *)_my_name << eoarg << (char *)txt << eoarg << tcl_stat << eoarg << res << eom;
		;
            }
	    IF (tcl_stat != TCL_OK) {
                res = 0;
                err_msg.printf(TXT("Failed to interpret: [[ %s ]]"), (char *)txt);
            }
        } else
            err_msg = TXT("Failed to substitute the selector expression");
    } else
        err_msg = TXT("Failed to start Tcl Interpreter.");

    if (err_msg.length()) {
        msg("$1: $2 for file $3") << (char const *)_my_name << eoarg << (char *)err_msg << eoarg << path << eom;
        ;
    }
    return res;
}

static int selector_is_equal(char *path, wkPtnRule *cur_ptn_rule)
// compares two strings regardless white spaces
//
// path: current path.
// cur_ptn_rule: the current physical path definition
// Returns the boolean evaluation result (0 *or* 1)
{
    Initialize(selector_is_equal__char *_wkPtnRule*);
    genString err_msg;
    int res = 0;
    genString txt;
    if (selector_substitute (txt, cur_ptn_rule)) {
	char *buf = (char *)txt;
	char c, *start1 = 0, *end1 = 0, *start2 = 0, *end2 = 0;
        while ( (c = *buf) ) {
	    if (isspace(c)) {
                if (start1 && end1 == 0)
                    end1 = buf;
                else if (start2 && end2 == 0) {
                    end2 = buf;
                    break;
                }
            } else {
                if (start1 == 0)
                    start1 = buf;
                else if (end1 && start2 == 0)
                    start2 = buf;
            }
            buf++ ;
        }

        if (start1, end1, start2) {
            *end1 = '\0';
            if (end2) *end2 = '\0';
            res = (OSapi_strcmp(start1, start2) == 0);
            if (PDF_DebugLevel > 0) {
                msg("$1: compare \"$2\" \"$3\". Result: $4") << (char const *)_my_name << eoarg << start1 << eoarg << start2 << eoarg << res << eom;
                ;
            }
            *end1 = ' ';
            if (end2) *end2 = ' ';
        } else {
	    res = 0;
	    err_msg.printf(TXT("Failed to interpret: [[ %s ]]"), (char *)txt);
        }
    } else
        err_msg = TXT("Failed to substitute the selector expression");

    if (err_msg.length()) {
        msg("$1: $2 for file $3") << (char const *)_my_name << eoarg << (char *)err_msg << eoarg << path << eom;
        ;
    }
    return res;
}

static int check_all_selectors(projMap *cur, char *first_path, int index, genString &last_file, int xx_f, int str_idx)
// check the selectors
//
// cur: is the current project definition
//
// first_path: input physical path
//
// index: is the number of the matched patterns not include **
//
// last_file: is the last path after the path goes through all selectors
//
// xx_f = 1: for special environmeny like EDS; check the link for [[f]]
//
// return 1: If first_path passes all selectors; otherwise it returns 0.
{
    Initialize(check_all_selectors);

    if (xx_f)
        return 1;

    genString ss = first_path;

    index += cur->get_phy_rule(0)->count_const_two_star();

    for (int i=1; i<cur->get_multi_phy_rule()->size(); i++)
        {
            char link_path[MAXPATHLEN];

            int rule_type = cur->get_phy_rule_type(i);
            if (rule_type == MAP_SCRIPT_TYPE || rule_type == MAP_FILTER_TYPE)
                continue;
            if (rule_type == MAP_FILE_TYPE)
                {
                    int file_tp = cur->get_phy_file_type(i);
                    // projMap *child = cur->get_child_project();
                    if (// file_tp == FILE_TYPE && child || file_tp == DIR_TYPE && !child ||
                        !cur->match_file_type(ss,file_tp,1,0,0,1, xx_f))
                        return 0; // reject by file type
                }
            else
                {
                    wkPtnRule *cur_ptn_rule = cur->get_phy_rule(i);

                    if (rule_type == REALPATH_MAP_PHY_PATH)
                        {
                            if (fileCache_realpath(ss, link_path) == 0)
                                return 0; // fail
                        }
                    else if (rule_type == LINK_MAP_PHY_PATH)
                        {
                            log_link_file_name("check_all_selectors before readlink", (char *)ss.str());

#ifndef _WIN32
                            int no_char = OSapi_readlink(ss.str(), link_path, MAXPATHLEN);

                            if (no_char < 0)
                                return 0; // failed
#else
                            int no_char = 0;
                            return 0;
#endif

                            link_path[no_char] = '\0';
                            log_link_file_name("check_all_selectors after readlink", link_path);

                            genString s;
                            remove_double_slash(link_path, s);
                            strcpy(link_path, (char *)s.str());

                        }
                    else if (rule_type == SEARCH_PHY_PATH)
                        {
                            if (!search_path(ss,cur_ptn_rule,link_path))
                                return 0; // fail
                        } 
                    else if (rule_type == TCL_BOOLEAN_EVAL)
                        {
                            return selector_tcl_eval (ss, cur_ptn_rule);
                        }
                    else if (rule_type == EQUAL_EVAL)
                        {
                            return selector_is_equal (ss, cur_ptn_rule);
                        }
                    else if (rule_type == NOT_EQUAL_EVAL)
                        {
                            return !selector_is_equal (ss, cur_ptn_rule);
                        }

                    if (cur_ptn_rule && cur_ptn_rule->get_org_ptn() && strlen(cur_ptn_rule->get_org_ptn()))
                        {
                            if (rule_type != SEARCH_PHY_PATH)
                                {
                                    int cur_len;
                                    int full_match_flag = 0;
                                    int status = new_split_ptn_path(link_path, cur_ptn_rule, &cur_len, &full_match_flag, index);

                                    if (status == 0 || !full_match_flag || strlen(link_path) != cur_len)
                                        return 0; // fail
                                }

                            index += cur_ptn_rule->get_max_ptn_count() + cur_ptn_rule->count_const_two_star();
                        }
                    if (rule_type != LINK_MAP_PHY_PATH || cur_ptn_rule->get_max_ptn_count())
                        {
                            int cur_len;
                            int full_match_flag = 0;
                            int status = new_split_ptn_path(link_path, cur_ptn_rule, &cur_len, &full_match_flag, str_idx);
                            if (!status)
                                return 0; // fail
                        }
                }

            if (!leading_char(link_path, '/'))
                {
                    genString s;
                    struct OStype_stat buf;
#ifdef _WIN32
                    if (fileCache_stat(ss.str(), &buf) == 0 && buf.st_mode & _S_IFREG != 0)
#else
                        if (fileCache_stat(ss.str(), &buf) == 0 && S_ISREG(buf.st_mode))
#endif
                            {
                                char *q = strrchr((char *)ss,'/');
                                *q = '\0';
                            }
                    if (OSapi_strchr(link_path,'/') || (OSapi_strncmp(link_path,"..",2) == 0) || (OSapi_strncmp(link_path,"./",2) == 0))
                        {
                            s.printf("%s/%s",ss.str(),link_path);
                            paraset_get_fullpath((char *)s.str(), ss);
                        }
                    else
                        {
                            char *p = strrchr((char *)ss,'/');
                            if (p)
                                {
                                    *(p+1) = '\0';
                                    ss += link_path;
                                }
                        }
                }
            else
                ss = link_path;
        }
    last_file = ss.str();
    return 1;
}

static void go_thru_selectors_p2i(
                                  projMap *cur,
                                  char const *path,
                                  char const *org_path,
                                  char const */* rem_path */,
                                  genString &ret_path,
                                  char const */* idx_name */ ,
                                  char const */* sub_idx_name */ ,
                                  char file_tp,
                                  int /* level */ ,
                                  int complete_len,
                                  char const *rule,
                                  char rule_type,
                                  int *link_reject_flag,
                                  wkIdxRule *cur_idx_rule,
                                  wkPtnRule *cur_ptn_rule,
                                  char /* realpath_flag */ ,
                                  int * /* no_pre_pend_flag */ ,
                                  int *reject_by_exclude, projNode **pr)
// verify the selectors
//
// cur: the current projMap
// path: the physical path for this sub_proj
// org_path: the whole_path
// rem_path: the unmatched tail
// ret_path: the matched logic (project) path
// idx_name: the sub_project_name needs to be satisfied
// sub_idx_name: is the tail portion of idx_name. idx_name = index name of this node(cur) + sub_idx_name
// file_type: either FILE_TYPE for file or DIR_TYPE for directory
// level: for recursive count
// complete_len: The length of the matched path
// rule: is the rules used so far
// rule_type: PROJECT_NAME_RULE => convert physical to project
//            LOGIC_NAME_RULE => convert physical to logic
// link_reject_flag: will be set by ptn_to_idx if file type does not match.
//                   This file is a link but the file type is [[ fd ]]. No link is allowed.
// cur_idx_rule: the using idx rule
// cur_ptn_rule: the physical rule
// realpath_flag: if set, the file could be accepted in project node level
//                which is set to 0 now
// no_pre_pend_flag = 1 for rule; forget the path of parents;; Otherwise it is 0
// reject_by_exclude = 1; this file is rejected by some excluded rule
// pr: if *pr is 0, which is called from projHeader and need pr returned.
//     use projHeader do the second search
//   : if *pr is not 0, no pr will be returned and use pr for second search
{
    Initialize(go_thru_selectors_p2i);

    ////////////////////////////////////////////////////////////////////
    //
    // VERIFY if the path is accepted by the rule of physical path
    //
    ////////////////////////////////////////////////////////////////////
    int match_count = cur->get_phy_match_count();
    init_working_buffer(match_count);
    int cur_len;
    int full_match_flag = 0;
    int sidx = 0;
    int status = new_split_ptn_path(path, cur_ptn_rule, &cur_len, &full_match_flag, sidx);

    ////////////////////////////////////////////////////////////////////
    //
    // REJECT by the rule of physical path
    //
    ////////////////////////////////////////////////////////////////////

    if (!status || !full_match_flag || (tail_portion_is_defined() && !cur->get_child_project()))
        return;

    ////////////////////////////////////////////////////////////////////
    //
    // Try each selector to accept this path
    //
    ////////////////////////////////////////////////////////////////////
    long len = 0;
    if (tail_portion_is_defined())
        len = tail_portion.length();
    genString pre_fix = org_path;
    char *pp = pre_fix;
    pp[strlen(pp) - len] = '\0';

    int index = cur_ptn_rule->get_max_ptn_count();

    genString save_tail_portion = tail_portion.str();
    genString last_file;

    if (check_all_selectors(cur, pre_fix, index, last_file, 0, sidx) == 0)
        {
            ret_path = 0;
            return;
        }
    else if (cur->is_exclude_proj())
        {
            *reject_by_exclude = 1;
            ret_path = 0;
            return;
        }

    if (cur_idx_rule->idx_is_defined() || rule_type == PROJECT_NAME_RULE ||
        file_tp != FILE_TYPE || save_tail_portion.length())
        {
            tail_portion = save_tail_portion.str();
            combine_idx_path(cur_idx_rule->get_idx_strs(), 0, &cur_len, ret_path, cur);

            if (ret_path.length())
                {
                    complete_len += strlen(path);
                    update_complete_rule(complete_len, rule, cur);
                }
        }
    else if (strcmp(last_file,org_path))
        {
            if (*pr)
                (*pr)->fn_to_ln(last_file, ret_path);
            else
                projHeader::fn_to_ln(last_file, ret_path, pr); // do the second search

            if (ret_path.length() == 0)
                *link_reject_flag = 1;
            else
                *link_reject_flag = 0;
        }
}

static void go_thru_selectors_i2p(
                                  projMap *cur, char *path, char *org_path, genString &ret_path, char file_tp,
                                  char rule_type, wkIdxRule *cur_idx_rule, wkPtnRule *cur_ptn_rule, int xx_f)
// verify the selectors when converting a logic name or project name to the physical name
//
// cur: the current projMap
//
// path: the physical path for this sub_proj
//
// org_path: the whole_path
//
// ret_path: the matched logic (project) path
//
// file_type: either FILE_TYPE for file or DIR_TYPE for directory
//
// rule_type: PROJECT_NAME_RULE => convert physical to project
//            LOGIC_NAME_RULE => convert physical to logic
//
// cur_idx_rule: the using idx rule
//
// cur_ptn_rule: the physical rule
//
// xx_f: if set, match_file_type will return TRUE for a link
{
    Initialize(go_thru_selectors_i2p);

    ////////////////////////////////////////////////////////////////////
    //
    // split physical path
    //
    ////////////////////////////////////////////////////////////////////
    int match_count = cur->get_phy_match_count();
    init_working_buffer(match_count);
    int cur_len;
    int full_match_flag = 0;
    int sidx = 0;
    int status = new_split_ptn_path(path, cur_ptn_rule, &cur_len, &full_match_flag, sidx);

    ////////////////////////////////////////////////////////////////////
    //
    // Try each selector to accept this path
    //
    ////////////////////////////////////////////////////////////////////
    int index = cur_ptn_rule->get_max_ptn_count();

    genString save_tail_portion = tail_portion.str();
    genString last_file;

    if (check_all_selectors(cur, org_path, index, last_file, xx_f, sidx) == 0)
        {
            ret_path = 0;
            return;
        }

    if (cur_idx_rule->idx_is_defined() || rule_type == PROJECT_NAME_RULE ||
        file_tp != FILE_TYPE || save_tail_portion.length())
        {
            tail_portion = save_tail_portion.str();
            combine_idx_path(cur_idx_rule->get_idx_strs(), 0, &cur_len, ret_path, cur);
        }
    else if (strcmp(last_file,org_path))
        projHeader::fn_to_ln(last_file, ret_path); // do the second search
}

bool projMap::exclude_dir_path(char const *path, char file_tp)
// check if this directory "path" is excluded
// file_type could be DIR_TYPE or FILE_TYPE
{
    Initialize(projMap::exclude_dir_path);

    genString s = path;
    char *p;
    if (file_tp == FILE_TYPE)
        {
            p = strrchr(s.str(),'/');
            if (!p)
                return FALSE;
            *p = '\0';
        }
    p = (char *)s.str();
    WildPair *m = get_phy_rule(0)->get_dir_multi_match_in_ptnrule();
    int len = m->match_to_end(p);
    if (len >= 0)
        return TRUE; // match

    return FALSE;
}

bool projMap::exclude_file_path(char const *path, char file_tp)
// check if this file "path" is excluded
// file_type could be DIR_TYPE or FILE_TYPE
{
    Initialize(projMap::exclude_file_path);

    if (file_tp == DIR_TYPE)
        return FALSE;
    WildPair *m = get_phy_rule(0)->get_multi_match_in_ptnrule();
    int len = m->match_to_end(path);
    if (len >= 0)
        return TRUE; // match

    return FALSE;
}

int projMap::get_exclude_type()
{
    Initialize(projMap::get_exclude_type);

    int exclude_type = NON_EXCLUDE;
    if (carot_proj())
        exclude_type = EXCLUDE_DIR | EXCLUDE_FILE;

    if (is_exclude_proj())
        {
            if (get_sub_proj_flag() == IMPLICIT_DIR_PROJ)
                exclude_type = EXCLUDE_DIR;
            exclude_type = EXCLUDE_FILE;
        }

    if (get_parent_project())
        {
            if (get_parent_project()->get_sub_proj_flag() == IMPLICIT_DIR_PROJ)
                exclude_type |= EXCLUDE_PAR_IMP;
        }

    if (get_org_project_name() && strncmp(get_org_project_name(),"__",2) == 0)
        exclude_type |= EXCLUDE_RULE;
    return exclude_type;
}

bool projMap::exclude_path(char const *path, char file_tp)
{
    Initialize(projMap::exclude_path);

    if(carot_proj()) // ^ is also for dir
        {
            if (exclude_dir_path(path, file_tp) || exclude_file_path(path, file_tp))
                return TRUE;
            return FALSE;
        }
    else if (get_sub_proj_flag() == IMPLICIT_DIR_PROJ)
        {
            if (exclude_dir_path(path, file_tp))
                return TRUE;
            return FALSE;
        }
    else // if (get_sub_proj_flag() != IMPLICIT_DIR_PROJ)
        {
            if (exclude_file_path(path, file_tp))
                return TRUE;
            return FALSE;
        }
}

void projMap::ptn_to_idx(char const *path, char const *org_path, char const *rem_path, genString &ret_path, char const *idx_name,
                         char file_tp, int level, int complete_len, char const *rule, char rule_type, int *link_reject_flag,
                         char realpath_flag, int *no_pre_pend_flag, int *reject_by_exclude, projNode **pr)
// convert the physical name to logic (project) name by project definition
//
// path: the physical path for this sub_proj
//
// org_path: the whole_path
//
// rem_path: the unmatched tail
//
// ret_path: the matched logic (project) path
//
// idx_name: the sub_project_name needs to be satisfied
//
// file_type: either FILE_TYPE for file or DIR_TYPE for directory
//
// level: for recursive count
//
// complete_len: The length of the matched path
//
// rule: is the rules used so far
//
// rule_type: PROJECT_NAME_RULE => convert physical to project
//            LOGIC_NAME_RULE => convert physical to logic
//
// link_reject_flag: will be set by ptn_to_idx if file type does not match.
//                   This file is a link but the file type is [[ fd ]]. No link is allowed.
//
// realpath_flag: if set, the file could be accepted in project node level
//                which is set to 0 now
//
// no_pre_pend_flag = 1 for rule; forget the path of parents;; Otherwise it is 0
//
// reject_by_exclude = 1; this file is rejected by some excluded rule
//
// pr: if *pr is 0, which is called from projHeader and need pr returned.
//     use projHeader do the second search
//   : if *pr is not 0, no pr will be returned and use pr for second search
{
    Initialize(projMap::ptn_to_idx);

    wkIdxRule *cur_idx_rule;
    wkIdxRule *chk_idx_rule;
    wkPtnRule *cur_ptn_rule;
    if (rule_type == PROJECT_NAME_RULE)
        {
            cur_idx_rule = proj_rule;
            chk_idx_rule = proj_rule;
            cur_ptn_rule = get_phy_rule(0);
        }
    else
        {
            if (is_left_way())
                {
                    go_thru_next_p2i(this, rem_path, org_path, rem_path, ret_path, idx_name, file_tp, level+1,
                                     complete_len, rule, rule_type, link_reject_flag, realpath_flag,
                                     no_pre_pend_flag, reject_by_exclude, pr);
                    return;
                }

            cur_idx_rule = logic_rule;
            chk_idx_rule = proj_rule;
            cur_ptn_rule = get_phy_rule(0);
        }

    // try to get the sub_idx_name
    char const *sub_idx_name = 0;
    if (idx_name)
        {
            genString proj_name = get_org_project_name();
            remove_back_slash_and_parentheses((char *)proj_name.str());
            int len = proj_name.length();
            if (strncmp(idx_name,proj_name.str(),len) == 0)
                {
                    if (strlen(idx_name) > len + 1 && *(idx_name+len) == '/')
                        sub_idx_name = idx_name + len + 1;
                }
        }

    if (selector_defined())
        {
            go_thru_selectors_p2i(this, path, org_path, rem_path, ret_path, idx_name, sub_idx_name, file_tp,
                                  level, complete_len, rule, rule_type, link_reject_flag,
                                  cur_idx_rule, cur_ptn_rule, realpath_flag, no_pre_pend_flag,
                                  reject_by_exclude, pr);

            if (*reject_by_exclude || *link_reject_flag)
                return;

            if (rule_type == PROJECT_NAME_RULE && idx_name && ret_path.length())
                {
                    int len = ret_path.length();
                    if (strncmp(idx_name,ret_path,len) == 0)
                        {
                            if (strlen(idx_name) > len + 1 && *(idx_name+len) == '/')
                                sub_idx_name = idx_name + len + 1;
                            else
                                ret_path = 0;
                        }
                    else
                        ret_path = 0;
                }

            if (ret_path.length() == 0 || !check_this_is_target())
                go_thru_next_p2i(this, rem_path, org_path, rem_path, ret_path, idx_name, file_tp, level+1,
                                 complete_len, rule, rule_type, link_reject_flag, realpath_flag,
                                 no_pre_pend_flag, reject_by_exclude, pr);

            return;
        }

    if (is_exclude_proj() &&
        get_parent_project()) // &&  get_parent_project()->get_sub_proj_flag() == IMPLICIT_DIR_PROJ)
        {
            if (exclude_path(path, file_tp))
                {
                    ret_path = 0;
                    *reject_by_exclude = 1;
                    update_successfull_rule("",strlen(org_path));
                }
            else
                {
                    go_thru_next_p2i(this, rem_path, org_path, rem_path, ret_path, idx_name, file_tp, level+1,
                                     complete_len, rule, rule_type, link_reject_flag, realpath_flag,
                                     no_pre_pend_flag, reject_by_exclude, pr);
                    if ((file_type == DIR_TYPE ||
                         get_parent_project()->imp_without_real_filter()) &&
                        *reject_by_exclude == 0 && level != 2)
                        ret_path = rem_path;
                }
            return;
        }

    int match_count = get_phy_match_count();
    init_working_buffer(match_count);
    int cur_len = 0;

    // If this path match the pattern, it will return the idx path
    // Otherwise, it will return 0.
    int full_match_flag = 0;
    int sidx = 0;
    int status =new_split_ptn_path(path, cur_ptn_rule, &cur_len, &full_match_flag, sidx);

    if (status && !check_this_is_target())
        {
            ret_path = 0;
            *reject_by_exclude = 1;
            update_successfull_rule("",strlen(org_path));
        }

    if (!status)
        update_complete_rule(complete_len + cur_len, rule, this);
    else if (file_tp != PARTIAL_DIR_TYPE && !full_match_flag)
        status = 0;

    if (!status)
        {
            go_thru_child_then_next_p2i(this, path, org_path, rem_path, ret_path, idx_name, sub_idx_name,
                                        file_tp, level+1, complete_len, rule, rule_type, link_reject_flag,
                                        cur_idx_rule, realpath_flag, no_pre_pend_flag, reject_by_exclude, pr);
            return;
        }

    int comp_len;
    if (leading_char(path,'/'))
        comp_len = cur_len;
    else
        comp_len = complete_len + cur_len;

    genString ret_str;
    combine_idx_path(cur_idx_rule->get_idx_strs(), cur_ptn_rule->get_ptn_strs(), &comp_len, ret_str, this);

    update_complete_rule(comp_len, rule, this);

    if (ret_str.str() == 0)
        {
            go_thru_child_then_next_p2i(this, path, org_path, rem_path, ret_path, idx_name, sub_idx_name,
                                        file_tp, level+1, complete_len, rule, rule_type, link_reject_flag,
                                        cur_idx_rule, realpath_flag, no_pre_pend_flag, reject_by_exclude, pr);
            return;
        }

    long len = tail_portion.length();

    sub_idx_name = 0;
    if (idx_name)
        {
            genString chk_str;
            if (chk_idx_rule != cur_idx_rule)
                combine_idx_path(chk_idx_rule->get_idx_strs(), cur_ptn_rule->get_ptn_strs(), &comp_len, chk_str, this);
            else
                chk_str = ret_str.str();

            char *si = (char *)chk_str.str();
            if (len)
                si[chk_str.length() - len - 1] = '\0'; // remove "/" + tail_portion
            int len0 = strlen(si);
            int len1 = strlen(idx_name);
            if (strncmp(idx_name,si,len0) == 0)
                {
                    if (len1 > len0)
                        {
                            if (*(idx_name+len0) == '/')
                                sub_idx_name = idx_name + len0 + 1;
                            else
                                {
                                    go_thru_next_p2i(this, rem_path, org_path, rem_path, ret_path, idx_name, file_tp,
                                                     level+1, complete_len, rule, rule_type, link_reject_flag,
                                                     realpath_flag, no_pre_pend_flag, reject_by_exclude, pr);
                                    if (ret_path.length() == 0)
                                        projMap::set_path_tran_rule(REJECT_BY_PROJ_NAME);
                                    return;
                                }
                        }
                }
            else if (strncmp(idx_name,si,len1) || si[len1] != '/')
                {   // proj_name does not match
                    go_thru_next_p2i(this, rem_path, org_path, rem_path, ret_path, idx_name, file_tp, level+1,
                                     complete_len, rule, rule_type, link_reject_flag, realpath_flag,
                                     no_pre_pend_flag, reject_by_exclude, pr);
                    if (ret_path.length() == 0)
                        projMap::set_path_tran_rule(REJECT_BY_PROJ_NAME);
                    return;
                }
        }

    genString pre_fix = org_path;
    char *pp = pre_fix;
    pp[strlen(pp) - len] = '\0';

    // fixes lstat prb for hp
    int file_length = strlen(pp);
    if( pp[file_length - 1] == '/') {
        pp[file_length - 1] = '\0';
    }

    char cur_file_type = file_tp;

#ifndef _WIN32
    if ((tail_portion_is_defined() || file_tp == PARTIAL_DIR_TYPE) && // so: it is a directory
	!strstr(tail_portion.str(), "%/."))                           // lkoro: 05/05/1998 (bug #15050)
#else
        if ((tail_portion_is_defined() || file_tp == PARTIAL_DIR_TYPE) && // so: it is a directory
            !strstr(tail_portion.str(), "\\%/."))                           // lkoro: 05/05/1998 (bug #15050)
#endif
            cur_file_type = DIR_TYPE;

    if (is_exclude_proj())
        {
            if ( is_implicit_sub_dir_proj() && cur_file_type == DIR_TYPE ||
                 !is_implicit_sub_dir_proj() && cur_file_type == FILE_TYPE)
                {
                    ret_path = 0; // sorry filter
                    *reject_by_exclude = 1;
                    update_successfull_rule("",strlen(org_path));
                    return;
                }
        }

    projMap *child = obtain_fast_child_project(this);
    if (!child)
        child = get_child_project();

    if (// cur_file_type == FILE_TYPE && child || cur_file_type == DIR_TYPE && !child || // obtain_this_lp() != ACCEPT_LEAF &&
        !match_file_type(pp, cur_file_type, full_match_flag, link_reject_flag, realpath_flag, 0))
        {
            go_thru_child_then_next_p2i(this, path, org_path, rem_path, ret_path, idx_name, sub_idx_name,
                                        file_type, level+1, complete_len, rule, rule_type, link_reject_flag,
                                        cur_idx_rule, realpath_flag, no_pre_pend_flag, reject_by_exclude, pr);
            return;
        }

    if (!tail_portion_is_defined())
        {
            ret_path = ret_str.str();
            update_successfull_rule("",strlen(org_path));
            if (is_real_right_way())
                {
                    *no_pre_pend_flag = 1;
                    add_one_rule(this);
                }
            return;
        }

    if (child == 0)
        {
            if (is_implicit_sub_dir_proj())
                {
                    if (ret_str.length()) // successful, but the length might be the same
                        {
                            genString s;
                            update_the_using_rule(s, rule, this);
                            projMap::set_path_tran_rule(s);
                            projMap::set_path_complete_full_len(comp_len);
                        }
                    ret_path = ret_str.str();
                    update_successfull_rule("",strlen(org_path));
                }
            else
                go_thru_next_p2i(this, rem_path, org_path, rem_path, ret_path, idx_name, file_tp, level+1,
                                 complete_len, rule, rule_type, link_reject_flag, realpath_flag,
                                 no_pre_pend_flag, reject_by_exclude, pr);
            return;
        }

    // tail_portion is a static variable and will be changed by child->ptn_to_idx()
    // The next statement must be executed before child->ptn_to_idx()
    ret_str.put_value(ret_str.str(), int(ret_str.length() - len));

    pre_fix = path;
    pp = pre_fix;
    pp[strlen(pp) - len - 1] = '\0'; // -1 will remove the last slash
    set_map_phy_path(pp);

    genString s;
    update_the_using_rule(s, rule, this);

    genString child_portion;
    genString new_path;
    while(1)
        {
            child_portion = 0;
            new_path = tail_portion;
            genString save_tail_portion = tail_portion.str();
            int start_new = child_with_slash(child, chosen_rule, FNTOLN);

            if (start_new)
                // It starts from the root, nothing is mapped yet. Hence the complete length should be 0.
                child->ptn_to_idx(org_path, org_path, new_path, child_portion, sub_idx_name, file_tp, 1, 0, s,
                                  rule_type, link_reject_flag, realpath_flag, no_pre_pend_flag, reject_by_exclude,
                                  pr);
            else
                child->ptn_to_idx(new_path, org_path, new_path, child_portion, sub_idx_name, file_tp, 1,
                                  complete_len + cur_len, s, rule_type, link_reject_flag, realpath_flag,
                                  no_pre_pend_flag, reject_by_exclude, pr);

            tail_portion = save_tail_portion.str();

            if ((*reject_by_exclude && child->is_exclude_proj()) || *link_reject_flag)
                {
                    ret_path = 0;
                    return;
                }
            else if (*reject_by_exclude)
                {
                    child = child->get_next_project();
                    if (child)
                        *reject_by_exclude = 0;
                    else
                        {
                            ret_path = 0;
                            return;
                        }
                }
            else
                break;
        }

    if (child_portion.str() == 0)
        {
            projMap *parent = this->get_parent_project();
            if (parent) // && parent->is_implicit_sub_dir_proj())
                go_thru_next_p2i(this, rem_path, org_path, rem_path, ret_path, idx_name, file_tp, level+1,
                                 complete_len, rule, rule_type, link_reject_flag, realpath_flag,
                                 no_pre_pend_flag, reject_by_exclude, pr);
            else
                ret_path = 0;
            return;
        }

    // If this time is to try to get the project name, it needs to use the same rule.
    if (!use_same_rule(s))
        {
            go_thru_child_then_next_p2i(this, path, org_path, rem_path, ret_path, idx_name, sub_idx_name,
                                        file_tp, level+1, complete_len, rule, rule_type, link_reject_flag,
                                        cur_idx_rule, realpath_flag, no_pre_pend_flag, reject_by_exclude, pr);
            return;
        }

    if (*no_pre_pend_flag == 0)
        combine_dir_and_sub_dir((char *)ret_str.str(), (char *)child_portion.str(), ret_path);
    else
        ret_path = (char *)child_portion.str();
}


void projMap::fn_to_ln_imp(char const *path, char const *sub_proj_name, char file_tp, genString &ret_str)
{
    fn_to_ln_imp(path, sub_proj_name, file_tp, ret_str, 0);
}

void projMap::fn_to_ln_imp(char const *path, char const *sub_proj_name, char file_tp, genString &ret_str, int xx_f)
{
    fn_to_ln_imp(path, sub_proj_name, file_tp, ret_str, xx_f, 0);
}

void projMap::fn_to_ln_imp(char const *path, char const *sub_proj_name, char file_tp, genString &ret_str, int xx_f,
                           int script_force_search)
{
    Initialize(projMap::fn_to_ln_imp(char *,char *,genString&,char,int));

    chosen_rule = PROJECT_NAME_RULE;
    fn_to_ln(path, sub_proj_name, file_tp, ret_str, xx_f, script_force_search);
    chosen_rule = LOGIC_NAME_RULE;
}

void projMap::fn_to_ln(char const *path, char const *sub_proj_name, genString &ret_path)
{
    projNode *pr = 0;
    fn_to_ln_internal(path, sub_proj_name, FILE_TYPE, ret_path, &pr);
}

void projMap::fn_to_ln(char const *org_path, char const *sub_proj_name, char file_tp, genString &ret_path)
{
    projNode *pr = 0;
    fn_to_ln_internal(org_path, sub_proj_name, file_tp, ret_path, 0, &pr);
}


void projMap::fn_to_ln(char const *org_path, char const *sub_proj_name, char file_tp, genString &ret_path,
                       int xx_f, int script_force_search)
{
    projNode *pr = 0;
    fn_to_ln_internal(org_path, sub_proj_name, file_tp, ret_path, xx_f, script_force_search, &pr);
}


void projMap::fn_to_ln_internal(char const *path, char const *sub_proj_name, char file_tp,
                                genString &ret_path, projNode **pr)
{
    fn_to_ln_internal(path, sub_proj_name, file_tp, ret_path, 0, pr);
}

void projMap::fn_to_ln_internal(char const *path, char const *sub_proj_name, char file_tp,
                                genString &ret_path, int xx_f, projNode **pr)
{
    fn_to_ln_internal(path, sub_proj_name, file_tp, ret_path, xx_f, 0, pr);
}

extern "C" void gen_stack_prt(int level);

void projMap::fn_to_ln_internal(char const *org_path, char const *sub_proj_name, char file_tp,
                                genString &ret_path, int xx_f, int script_force_search, projNode **pr)
// map a file name from its physical name to the logic name (project or logic)
//
// org_path: is the input path
//
// sub_proj_name: the project name where this file belongs to
//
// file_type: DIR_TYPE for a directory
//          : FILE_TYPE for a file
//
// ret_path: contains the mapped path if succeeds
// ret_path: 0 if fails
//
// xx_f = 1: do the mapping by forgetting the selector definitions
//      = 0: checks all selectors
//
// script_force_search = 1: will do the mapping
//                     = 0: only search the harsh table which contains all file names that are generated when the
//                          projnode is created.
// pr: if *pr is 0, which is called from projHeader and need pr returned.
//     use projHeader do the second search
//   : if *pr is not 0, no pr will be returned and use pr for second search
{
    Initialize(projMap::fn_to_ln_internal(char *,char *,char));

#if defined(_WIN32) && defined(_DEBUG)
    char *pdbg_org_path = (char *)GetDiscoverRegistryDebug("fn_to_ln");
    if ( pdbg_org_path && stricmp(pdbg_org_path,(char *)org_path) == 0 )
        _asm int 3;
#endif

    log_link_file_name("fn_to_ln_internal begin", (char *)org_path);

    int no_save_hash = 1;
    if (pr && *pr)
        no_save_hash = 0;

    if (!script_force_search)
        {
            search_hash(org_path, ret_path, file_tp, chosen_rule, FNTOLN, sub_proj_name);
            if (ret_path.length())
                return;

            if (!parse_is_being_executed && is_script())
                return;
        }

    genString ppp;
    remove_double_slash((char *)org_path, ppp);
    org_path = ppp;
    genString path = org_path;

    // For error generator.
    projMap::init_tran_vars();

    retreat_len = 0;

    if (leading_char((char *)path.str(),'~'))
        {
            int len = path.length();
            genString s = path;
            // convert_home_dir will free path, but new_path is genString.
            convert_home_dir(s.str(), path);
            retreat_len = path.length() - len;
        }

    if (!leading_char((char *)path.str(), '/'))
        {
            genString new_path;

            if (!paraset_get_fullpath((char *)path.str(),new_path))
                ret_path = 0;
            else
                fn_to_ln(new_path, sub_proj_name, file_tp, ret_path);

            //      if (PDF_DebugLevel > 0)  cout << "=>" << ret_path.str() << endl;
            return;
        }

    // The last third parameter 0 is level which is explained in go_thru_next_p2i.
    // The last second 0 is the length of mapping portion. It is 0 now.
    // The last 0 means path might be a link.

    int link_reject_flag = 0; // will be set by ptn_to_idx if file type does not match
    int no_pre_pend_flag = 0;
    int reject_by_exclude = 0;

    ptn_to_idx((char *)path.str(), (char *)path.str(), (char *)path.str(), ret_path,
               (char *)sub_proj_name + 1, file_tp, 0, 0, "",  chosen_rule,
               &link_reject_flag, 0, &no_pre_pend_flag, &reject_by_exclude, pr);

    if (reject_by_exclude || link_reject_flag) // reject by exclude rule
        ret_path = 0;
    else if (projMap::report_path_tran_rule()
             && strcmp(projMap::report_path_tran_rule(), REJECT_BY_PROJ_NAME) // reject by project name
             && ret_path.length() == 0) // no rule for it
        projMap::set_path_tran_rule(0);

    if (ret_path.str() == 0 && projMap::report_path_tran_error_code() == PATH_TRAN_NO_ERROR)
        projMap::set_path_tran_error_code(PATH_TRAN_NO_MAPPING);

    if (retreat_len)
        projMap::set_path_complete_full_len(projMap::report_path_complete_full_len() - retreat_len);

    // successful. add the length of the tail.
    if (ret_path.str() && tail_portion_is_defined())
        projMap::set_path_complete_full_len(projMap::report_path_complete_full_len() + tail_portion.length());

    if (projMap::report_path_tran_error_code() == PATH_TRAN_REJECT_BY_PROJ_NAME)
        {
            projMap::set_path_complete_full_len(0);
            projMap::set_path_tran_rule(0);
        }

    if (ret_path.str() && no_pre_pend_flag == 0)
        {
            if (!leading_char((char *)ret_path.str(),'/'))
                {
                    genString ss;
                    combine_dir_and_sub_dir("/",(char *)ret_path.str(),ss);
                    ret_path = ss.str();
                }
        }

    genString filter_path;
    if (file_tp == FILE_TYPE && !accept_by_filter(org_path, filter_path))
        ret_path = 0;

    //  do_second_search_fn_to_ln = 0;
    //  if (PDF_DebugLevel > 0)  cout << "=>" << ret_path.str() << endl;
    if (!no_save_hash && !xx_f && !script_force_search && ret_path.length())
        insert_hash(org_path, sub_proj_name, ret_path, file_tp, chosen_rule, FNTOLN);

    log_link_file_name("fn_to_ln_internal end", (char *)ret_path.str());

    return;
}

static void go_thru_next_i2p( projMap *cur, char const *path, char const *org_path, char const *rem_path, genString &ret_path,
                              char const *idx_name, char file_tp, int level, int complete_len, char const *rule,
                              char rule_type, char const *phy_path, int *reject_by_exclude, int xx_f, projNode *pn)
// convert the logical or project name to the physical name through the next projMaps
//
// cur, path, org_path, rem_path, ret_path, idx_name, file_tp referred to go_thru_child_then_next_i2p
//
// The first call to idx_to_ptn in "projMap::ln_to_fn" passes level 0.
//    When it comes to go_thru_next_i2p, the level is 1, reject it.
//    The next of the "top_level_project" should be NULL any way.
//
// The child->idx_to_ptn in idx_to_ptn itself, passes level 1.
//    When it comes to go_thru_next_i2p, the level is 2, it is the only
//    accepted case.
//
// The next->idx_to_ptn in go_thru_next_i2p, passes level 2.
//    When it comes to go_thru_next_i2p, the level is 3, reject it to
//    avoid the unnecessary recursive.
//
// complete_len, rule, rule_type, phy_path, reject_by_exclude, xx_f referred to go_thru_child_then_next_i2p
{
    Initialize(go_thru_next_i2p);

    if (level != 2)
        {
            ret_path = 0;
            return;
        }

    while (1)
        {
            cur = cur->get_next_project();
            if (cur == 0)
                {
                    ret_path = 0;
                    return;
                }

            int start_new = child_with_slash(cur, chosen_rule, LNTOFN);

            if (start_new)
                cur->idx_to_ptn( org_path, org_path, rem_path, ret_path, idx_name, file_tp, 2, complete_len,
                                 rule, rule_type, phy_path, reject_by_exclude, xx_f, pn);
            else
                cur->idx_to_ptn( path, org_path, rem_path, ret_path, idx_name, file_tp, 2, complete_len, rule,
                                 rule_type, phy_path, reject_by_exclude, xx_f, pn);

            if (ret_path.str())
                break;
        }
}

// GET_PTN_CONSTANT_STRING BEGINNING
typedef struct
{
    projMap *pm;
    char    *path;
} pm_entry;

genArr(pm_entry);

static genArrOf(pm_entry) pm_buf;

static void init_pm_buf()
{
    Initialize(init_pm_buf);

    for(int i=0;i<pm_buf.size();i++)
        {
            pm_entry *p = pm_buf[i];
            if (p->path)
                psetfree(p->path);
        }
    pm_buf.reset();
}

static void get_one_real_const_ptn_str(projMap *cur, genString &s)
{
    Initialize(get_one_real_const_ptn_str);

    wkPtnRule *cur_ptn_rule = cur->get_phy_rule(0);
    s = 0;
    wkPtnDef *cur_ptn = cur_ptn_rule->get_ptn_strs();
    while(cur_ptn)
        {
            char const *p = cur_ptn->get_str();
            if (need_expand_dir(p))
                {
                    s = 0;
                    break;
                }
            s += p;
            cur_ptn = cur_ptn->get_next();
        }
}

// construct the physical path form this node up to the root node
// If failed, it returned FALSE; otherwise, it will return TRUE.
//
// pme: is an node in the route which contains both the object projMap and the path for this projMap
//      the path will be set here if succedds
// fn: the path of home project
static bool construct_one_ptn_const_path(pm_entry *pme, genString &fn)
{
    Initialize(construct_one_ptn_const_path);

    if (!fn.length()) {
        genString s;
        get_one_real_const_ptn_str(pme->pm,s);
        if (s.length()) {
            pme->path = strdup(s.str());
            return TRUE;
        }

        return FALSE;
    }

    wkPtnRule *cur_ptn_rule = pme->pm->get_phy_rule(0);
    char const *ptn_def = cur_ptn_rule->get_ptn_def();
    int level = count_level(ptn_def);
    char *p = (char *)fn.str();
    if (leading_char(p,'/'))
        p++;

    for(int i=0;i<level;i++) {
        char *q = strchr(p,'/');
        if (!q) {
            if (i != level - 1)
                return FALSE; // not the last one
            p += strlen(p);
        }
        else
            p = q + 1;
    }

    genString cur_path;
    if (*p) {
        *(p-1) = '\0';
        cur_path = fn.str();
        fn = p;
    } else {
        cur_path = fn.str();
        fn = 0;
    }

    int match_count = pme->pm->get_phy_match_count();
    init_working_buffer(match_count);

    int cur_len;
    int status;
    int full_match_flag = 0;
    int sidx = 0;
    status = new_split_ptn_path(cur_path, cur_ptn_rule, &cur_len, &full_match_flag, sidx);

    if (!status)
        return FALSE; // match failed

    pme->path = strdup(cur_path);
    return TRUE;
}

static void get_ptn_const_str(projMap *cur, genString &s, projNode *pn)
{
    Initialize(get_ptn_const_str);

    s = 0;
    init_pm_buf();

    projMap *par = cur;
    while(cur)
        {
            pm_entry *p = pm_buf.grow();
            p->pm = cur;
            p->path = 0;
            par = cur;
            cur = cur->get_parent_project();
        }

    genString fn = par->get_dir_name();
    remove_back_slash_and_parentheses((char *)fn.str());
    if (need_expand_dir(fn))
        {
            projNode *h = projNode::get_home_proj();
            if(h == NULL)
                {
                    if(pn != NULL)
                        h = pn;
                    else
                        {
                            msg("ERROR: No way to convert logical name to physical") << eom;
                            return; // This is strange return
                        }
                }
            char const *name = h->get_name();
            h->ln_to_fn_imp(name, fn, DIR_TYPE);
            IF (!fn.length())
                return; // impossible to return from here
        }

    int i;
    for(i=0;i<pm_buf.size();i++)
        {
            pm_entry *p = pm_buf[pm_buf.size()-i-1];
            if (!construct_one_ptn_const_path(p,fn))
                return; // failed
        }

    for(i=0;i<pm_buf.size();i++)
        {
            pm_entry *p = pm_buf[i];
            if (leading_char(p->path,'/'))
                {
                    for(int j=0;j<=i;j++)
                        {
                            pm_entry *p = pm_buf[i-j];
                            combine_dir_and_sub_dir((char *)s.str(),p->path,s);
                        }
                }
        }
}
// GET_PTN_CONSTANT_STRING ENDING


// pdf_load ends to save pm then delete it

static void go_thru_child_then_next_i2p(projMap *cur, char const */* path */ , char const *org_path, char const *rem_path,
                                        genString &ret_path,
                                        char const *idx_name, char const *sub_idx_name, char file_tp, int level, int complete_len,
                                        char const *rule, char rule_type, char const *phy_path, wkPtnRule *cur_ptn_rule,
                                        int *reject_by_exclude, int xx_f, projNode *pn)
// convert the logical or project name to the physical name from the child projMaps first then the next projMaps
//
// cur: The current projMap
// path: the physical path for this sub_proj
// org_path: the whole_path
// rem_path: the unmatched tail
// ret_path: the matched logic (project) path
// idx_name: the sub_project_name needs to be satisfied
// sub_idx_name: the sub_project_name needs to be satisfied for the child projmaps
// file_type: either FILE_TYPE for file or DIR_TYPE for directory
// level: for recursive count
// complete_len: The length of the matched path
// rule: is the rules used so far
// rule_type: PROJECT_NAME_RULE => convert physical to project
//            LOGIC_NAME_RULE => convert physical to logic
// phy_path: The physical path which has been converted
// cur_ptn_rule: the current physical path definition
// reject_by_exclude: if set, this file is rejected by some excluded rule
// xx_f: if set, match_file_type will return TRUE for a link
{
    Initialize(go_thru_child_then_next_i2p);

    // When come here, it fails in the cur node.
    // Hence go thru the child by the original path "org_path" to see if the child starts with
    //    a slash.
    // Go thru the next node with "path" to see if the next node accepts it.

    // If the path starts from '/', the child might start from '/' too.
    projMap *child = cur->get_child_project();
    if (child && chosen_rule == LOGIC_NAME_RULE)
        {
            genString s;
            update_the_using_rule(s, rule, cur);

            while(child)
                {
                    genString save_tail_portion = tail_portion.str();
                    int start_new = child_with_slash(child, chosen_rule, LNTOFN);

                    // It starts from the root, nothing is mapped yet. Hence the complete length should be 0.
                    // Since start new, so the phy_path is an empty string.
                    if (start_new)
                        child->idx_to_ptn( org_path, org_path, org_path, ret_path, sub_idx_name, file_tp, 1, 0, s,
                                           rule_type, "", reject_by_exclude, xx_f, pn);
                    else
                        go_thru_child_then_next_i2p(child, org_path, org_path, org_path, ret_path, idx_name, sub_idx_name,
                                                    file_tp, level, complete_len, s, rule_type, phy_path,
                                                    cur_ptn_rule, reject_by_exclude, xx_f, pn);

                    if (ret_path.length())
                        break;

                    tail_portion = save_tail_portion.str();
                    child = child->get_next_project();
                }
        }

    if (ret_path.length() == 0)
        {
            go_thru_next_i2p( cur, rem_path, org_path, rem_path, ret_path, idx_name, file_tp, level,
                              complete_len, rule, rule_type, phy_path, reject_by_exclude, xx_f, pn);
            return;
        }

    genString full_path;
    genString phy_pathstr;
    genString save_tail_portion = tail_portion.str();
    get_ptn_const_str(cur, phy_pathstr, pn);
    tail_portion = save_tail_portion.str();
    combine_dir_and_sub_dir(phy_pathstr, (char *)ret_path.str(), full_path);
    ret_path = full_path.str();
}

void projMap::idx_to_ptn( char const *path, char const *org_path, char const *rem_path, 
                          genString &ret_path, char const *idx_name, char file_tp, int level, 
                          int complete_len, char const *rule, char rule_type, char const *phy_path, 
                          int *reject_by_exclude, int xx_f, projNode *pn)
// convert the logic (project) name to physical name by project definition
//
// path: the physical path for this sub_proj
//
// org_path: the whole_path
//
// rem_path: the unmatched tail
//
// ret_path: the matched logic (project) path
//
// idx_name: the sub_project_name needs to be satisfied
//
// file_type: either FILE_TYPE for file or DIR_TYPE for directory
//
// level: for recursive count
//
// complete_len: The length of the matched path
//
// rule: is the rules used so far
//
// rule_type: PROJECT_NAME_RULE => convert physical to project
//            LOGIC_NAME_RULE => convert physical to logic
//
// phy_path: The physical path which has been converted
//
// realpath_flag: if set, the file could be accepted in project node level
//                which is set to 0 now
//
// xx_f: if set, match_file_type will return TRUE for a link
{
    Initialize(projMap::idx_to_ptn);

    wkIdxRule *cur_idx_rule;
    wkIdxRule *chk_idx_rule;
    wkPtnRule *cur_ptn_rule;
    if (rule_type == PROJECT_NAME_RULE)
        {
            cur_idx_rule = proj_rule;
            chk_idx_rule = proj_rule;
            cur_ptn_rule = get_phy_rule(0);
        }
    else
        {
            if (is_right_way())
                {
                    go_thru_next_i2p( this, rem_path, org_path, rem_path, ret_path, idx_name, file_tp, level+1,
                                      complete_len, rule, rule_type, phy_path, reject_by_exclude, xx_f, pn);
                    return;
                }

            cur_idx_rule = logic_rule;
            chk_idx_rule = proj_rule;
            cur_ptn_rule = get_phy_rule(0);
        }

    // try to get the sub_idx_name
    char const *sub_idx_name = 0;
    if (idx_name)
        {
            genString proj_name = get_org_project_name();
            remove_back_slash_and_parentheses((char *)proj_name.str());
            int len = proj_name.length();
            if (strncmp(idx_name,proj_name.str(),len) == 0)
                {
                    if (strlen(idx_name) > len + 1 && *(idx_name+len) == '/')
                        sub_idx_name = idx_name + len + 1;
                }
        }

    int match_count = get_phy_match_count();
    init_working_buffer(match_count);
    int cur_len = 0;

    // If this path match the pattern, it will return the ptn path
    // Otherwise, it will return 0.
    int full_match_flag = 0;
    int status = 0;
    if ( !(file_tp == DIR_TYPE && flag == MEMBER) )
	status = new_split_idx_path(path, cur_idx_rule, &cur_len, rule_type, &full_match_flag);

    if (!status)
        update_complete_rule(complete_len + cur_len, rule, this);
    else if (file_tp != PARTIAL_DIR_TYPE && !full_match_flag)
        status = 0;

    if (!status)
        {
            go_thru_child_then_next_i2p(this, path, org_path, rem_path, ret_path, idx_name, sub_idx_name, file_tp,
                                        level+1, complete_len, rule, rule_type, phy_path, cur_ptn_rule,
                                        reject_by_exclude, xx_f, pn);
            return;
        }

    int comp_len;
    if (strcmp(path,org_path) == 0)
        comp_len = cur_len;
    else
        comp_len = complete_len + cur_len;

    genString ret_str;
    combine_ptn_path(cur_ptn_rule->get_ptn_strs(), cur_idx_rule->get_idx_strs(), &comp_len, ret_str);
    update_complete_rule(comp_len, rule, this);

    if (ret_str.length() == 0)
        {
            go_thru_child_then_next_i2p(this, path, org_path, rem_path, ret_path, idx_name, sub_idx_name, file_tp,
                                        level+1, complete_len, rule, rule_type, phy_path, cur_ptn_rule,
                                        reject_by_exclude, xx_f, pn);
            return;
        }

    long len = tail_portion.length();
    sub_idx_name = 0;
    if (idx_name)
        {
            genString chk_str;
            if (chk_idx_rule != cur_idx_rule)
                combine_idx_path(chk_idx_rule->get_idx_strs(), cur_ptn_rule->get_ptn_strs(), &comp_len, chk_str, this);
            else
                chk_str = path;

            char *si = (char *)chk_str.str();
            if (len)
                si[chk_str.length() - len - 1] = '\0'; // remove "/" + tail_portion
            int len0 = strlen(si);
            int len1 = strlen(idx_name);
            if (strncmp(idx_name,si,len0) == 0)
                {
                    if (len1 > len0)
                        {
                            if (*(idx_name+len0) == '/')
                                sub_idx_name = idx_name + len0 + 1;
                            else
                                {
                                    go_thru_next_i2p(this, rem_path, org_path, rem_path, ret_path, idx_name, file_tp, level+1,
                                                     complete_len, rule, rule_type, phy_path, reject_by_exclude, xx_f, pn);
                                    if (ret_path.length() == 0)
                                        projMap::set_path_tran_rule(REJECT_BY_PROJ_NAME);
                                    return;
                                }
                        }
                }
            else if (strncmp(idx_name,si,len1) || si[len1] != '/')
                {   // proj_name does not match
                    go_thru_next_i2p(this, rem_path, org_path, rem_path, ret_path, idx_name, file_tp, level+1,
                                     complete_len, rule, rule_type, phy_path, reject_by_exclude, xx_f, pn);
                    if (ret_path.length() == 0)
                        projMap::set_path_tran_rule(REJECT_BY_PROJ_NAME);
                    return;
                }
        }

    genString pre_fix = ret_str.str();
    char *pp = pre_fix;
    pp[strlen(pp) - len] = '\0';

    genString sd;
    if (strlen(phy_path) == 0 && get_parent_project())
        {
            genString save_tail_portion = tail_portion.str();
            get_ptn_const_str(get_parent_project(), sd, pn);
            tail_portion = save_tail_portion.str();
            phy_path = sd;
        }

    genString new_phy_path;
    combine_dir_and_sub_dir(phy_path, pp, new_phy_path);

    if (selector_defined())
        {
            genString verify_ret_path;
            go_thru_selectors_i2p(this, ret_str, new_phy_path, verify_ret_path, file_tp,
                                  rule_type, cur_idx_rule, cur_ptn_rule, xx_f);
            if (verify_ret_path.length() == 0)
                {
                    go_thru_next_i2p( this, rem_path, org_path, rem_path, ret_path, idx_name, file_tp, level+1,
                                      complete_len, rule, rule_type, phy_path, reject_by_exclude, xx_f, pn);
                    return;
                }
            else if (is_exclude_proj())
                {
                    *reject_by_exclude = 1;
                    ret_path = 0;
                    return;
                }
        }

    char cur_file_type = file_tp;
    if (tail_portion_is_defined() || file_tp == PARTIAL_DIR_TYPE) // it is a directory
        cur_file_type = DIR_TYPE;

    if (is_exclude_proj())
        {
            if ( is_implicit_sub_dir_proj() && cur_file_type == DIR_TYPE ||
                 !is_implicit_sub_dir_proj() && cur_file_type == FILE_TYPE)
                {
                    ret_path = 0; // sorry filter
                    *reject_by_exclude = 1;
                    update_successfull_rule("",strlen(org_path));
                    return;
                }
        }

    projMap *child = get_child_project();
    if (// cur_file_type == FILE_TYPE && child || cur_file_type == DIR_TYPE && !child ||
        !match_file_type(new_phy_path, cur_file_type, full_match_flag, 0, 0, 0, xx_f))
        {
            go_thru_child_then_next_i2p(this, path, org_path, rem_path, ret_path, idx_name, sub_idx_name, file_tp,
                                        level+1, complete_len, rule, rule_type, phy_path, cur_ptn_rule,
                                        reject_by_exclude, xx_f, pn);
            return;
        }

    if (!tail_portion_is_defined())
        {
            ret_path = ret_str.str();
            update_successfull_rule("",strlen(org_path));
            return;
        }

    if (child == 0)
        {
            if (is_implicit_sub_dir_proj())
                {
                    if (ret_str.length()) // successful, but the length might be the same
                        {
                            genString s;
                            update_the_using_rule(s, rule, this);
                            projMap::set_path_tran_rule(s);
                            projMap::set_path_complete_full_len(comp_len);
                        }
                    ret_path = ret_str.str();
                    update_successfull_rule("",strlen(org_path));
                }
            else
                go_thru_next_i2p(this, rem_path, org_path, rem_path, ret_path, idx_name, file_tp, level+1,
                                 complete_len, rule, rule_type, phy_path, reject_by_exclude, xx_f, pn);
            return;
        }

    // tail_portion is a static variable and will be changed by child->idx_to_ptn()
    // The next statement must be executed before child->idx_to_ptn()
    ret_str.put_value(ret_str.str(), int(ret_str.length() - len));

    int start_new = child_with_slash(child, chosen_rule, LNTOFN);

    genString s;
    update_the_using_rule(s, rule, this);

    genString child_portion;
    genString new_path = tail_portion;
    if (start_new)
        // It starts from the root, nothing is mapped yet. Hence the complete length should be 0.
        child->idx_to_ptn( org_path, org_path, new_path, child_portion, sub_idx_name, file_tp, 1, 0, s,
                           rule_type, "", reject_by_exclude, xx_f, pn);
    else
        {
            genString new_phy_path;
            combine_dir_and_sub_dir(phy_path, (char *)ret_str.str(), new_phy_path);
            child->idx_to_ptn( new_path, org_path, new_path, child_portion, sub_idx_name, file_tp, 1,
                               complete_len + cur_len, s, rule_type, new_phy_path, reject_by_exclude, xx_f, pn);
        }

    if (child_portion.length() == 0)
        {
            projMap *parent = this->get_parent_project();
            if (parent) // && parent->is_implicit_sub_dir_proj())
                go_thru_next_i2p( this, rem_path, org_path, rem_path, ret_path, idx_name, file_tp, level+1,
                                  complete_len, rule, rule_type, phy_path, reject_by_exclude, xx_f, pn);
            else
                ret_path = 0;

            return;
        }

    combine_dir_and_sub_dir((char *)ret_str.str(), (char *)child_portion.str(), ret_path);
}


void projMap::ln_to_fn_imp(char const *path, char const *sub_proj_name, char file_tp, genString &ret_str)
{
    ln_to_fn_imp(path, sub_proj_name, file_tp, ret_str, 0);
}

void projMap::ln_to_fn_imp(char const *path, char const *sub_proj_name, char file_tp, genString &ret_str, int xx_f)
{
    ln_to_fn_imp(path, sub_proj_name, file_tp, ret_str, xx_f, 0);
}

void projMap::ln_to_fn_imp(char const *path, char const *sub_proj_name, char file_tp, genString &ret_str, int xx_f,
                           int script_force_search, projNode *pn)
{
    chosen_rule = PROJECT_NAME_RULE;
    ln_to_fn( path, sub_proj_name, file_tp, ret_str, xx_f, script_force_search, pn);
    chosen_rule = LOGIC_NAME_RULE;
}

int good_proj_name(char const *path, char const *sub_proj_name)
{
    Initialize(good_proj_name);

    int len = strlen(sub_proj_name);
    if (strncmp(path,sub_proj_name,len))
        return 0;
    if (*(path+len) != '/' && *(path+len) != '\0')
        return 0;
    return 1;
}


void projMap::ln_to_fn(char const *path, char const *sub_proj_name, char file_tp, genString &ret_path, int xx_f,
                       int script_force_search, projNode *pn)
// map a file name from the logic name (project or logic) to its physical name
//
// path: is the input path
//
// sub_proj_name: the project name where this file belongs to
//
// file_type: DIR_TYPE for a directory
//          : FILE_TYPE for a file
//
// ret_path: contains the mapped path if succeeds
// ret_path: 0 if fails
//
// xx_f = 1: do the mapping by forgetting the selector definitions
//      = 0: checks all selectors
//
// pn : Corresponding project Node may be NULL.
//
// script_force_search = 1: will do the mapping
//                     = 0: only search the harsh table which contains all file names that are generated when the
//                          projnode is created.
{
    Initialize(projMap::ln_to_fn);

#if defined(_WIN32) && defined(_DEBUG)
    char *pdbg_path = (char *)GetDiscoverRegistryDebug("ln_to_fn");
    if ( pdbg_path && stricmp(pdbg_path,(char *)path) == 0 )
        _asm int 3;
#endif

    log_link_file_name("ln_to_fn begin", (char *)path);

    if (!script_force_search)
        {
            search_hash(path, ret_path, file_tp, chosen_rule, LNTOFN, sub_proj_name);
            if (ret_path.length())
                return;

            if (!parse_is_being_executed && is_script())
                return;
        }

    genString new_path = path;
    path = new_path;

    // For error generator.
    projMap::init_tran_vars();

    char reject_by_sub_proj_name = 0;

    // quick check. If path does not contain the sub_proj_name, reject it.
    if (chosen_rule == LOGIC_NAME_RULE || good_proj_name(path,sub_proj_name))
        {
            int complete_len = 0;
            // remove the first '/' for the project name
            if (chosen_rule == PROJECT_NAME_RULE)
                {
                    path++;
                    complete_len = 1; // for the removed '/'
                }

            int reject_by_exclude = 0;
            // The last parameter 0 is level which is explained in go_thru_next_i2p.
            // Since it already removes the first slash, the complete path should be 1
            // The last 0 means to start from the first logic rule which is only for LOGIC_NAME_RULE
            idx_to_ptn( (char *)path, (char *)path, (char *)path, ret_path, (char *)sub_proj_name+1, file_tp,
                        0, complete_len, "", chosen_rule, "", &reject_by_exclude, xx_f, pn);

            if (ret_path.length() && !leading_char((char *)ret_path.str(),'/'))
                combine_dir_and_sub_dir("/",(char *)ret_path.str(), ret_path);

            if (ret_path.length() == 0 && projMap::report_path_tran_error_code() == PATH_TRAN_NO_ERROR)
                projMap::set_path_tran_error_code(PATH_TRAN_NO_MAPPING);

            // successful. add the length of the tail.
            if (ret_path.length() && tail_portion_is_defined())
                projMap::set_path_complete_full_len(projMap::report_path_complete_full_len() + tail_portion.length());

            if (projMap::report_path_tran_error_code() == PATH_TRAN_REJECT_BY_PROJ_NAME)
                {
                    projMap::set_path_complete_full_len(0);
                    projMap::set_path_tran_rule(0);
                }
        }
    else
        reject_by_sub_proj_name = 1;

    if (reject_by_sub_proj_name)
        {
            projMap::set_path_tran_error_code(PATH_TRAN_REJECT_BY_PROJ_NAME);
            projMap::set_path_complete_full_len(0);
            projMap::set_path_tran_rule(0);
        }

    if (ret_path.length())
        {
            genString filter_path;
            if (file_tp == FILE_TYPE && !accept_by_filter(ret_path, filter_path))
                ret_path = 0;
        }

    if (!xx_f && !script_force_search && ret_path.length())
        {
            if (chosen_rule == PROJECT_NAME_RULE)
                path--;
            insert_hash(path, sub_proj_name, ret_path, file_tp, chosen_rule, LNTOFN);
        }
}

void projMap::ln_to_proj_name(char const *ln, char const *proj_name, genString &longest_proj_name, int ln_tp )
// try to get the longest project name from the project name of a module
//
// ln is project name already.
//
// proj_name: is the project name of one of its parent projects
//
// longest_proj_name might be ln
//
// ln_tp =0 do not cut of last portion of proj_name if the rule contains "**"
//
{
    Initialize(projMap::ln_to_proj_name);

    longest_proj_name = 0;

    // Get the physical name
    genString phy_path;
    int file_flag = 1;
    ln_to_fn_imp(ln, proj_name, FILE_TYPE, phy_path, 0, 1);

    if (phy_path.length() == 0)
        {
            ln_to_fn_imp(ln, proj_name, DIR_TYPE, phy_path, 0, 1);
            file_flag = 0;
        }

    if (phy_path.length() == 0)
        return;

    projMap *proj_head = this;
    if (!is_script())
        {
            genString s = projMap::report_path_tran_rule();
            proj_head = match_one_proj_name(this, s);
        }

    // boris 05/25/99: it was the capital 'IF' before, but it caused
    // error message with PDFs like the following:
    //   proj {
    //      *
    //      * { * }
    //   }
    // trying to do %defines -fil /proj/sub/a.h
    //
    if (proj_head == 0) // should not be
        return;

    if (proj_head->is_file() || proj_head->is_implicit_sub_dir_proj())
        {
            genString qq = ln;
            if (proj_head->is_implicit_sub_dir_proj() ||
                (proj_head->get_parent_project() && proj_head->get_parent_project()->is_implicit_sub_dir_proj()))
                {
                    int level = 0;
                    struct OStype_stat buf;
#ifdef _WIN32
                    if (fileCache_stat(phy_path, &buf) == 0 && buf.st_mode & _S_IFREG != 0)
#else
                        if (fileCache_stat(phy_path, &buf) == 0 && S_ISREG(buf.st_mode))
#endif
                            level = 1;

                    char const *q = qq.str();
                    for (int i=0;i<level;i++)
                        {
                            char *p = strrchr(q,'/');
                            if (p == 0) // unlikely
                                break;
                            *p = '\0';
                        }
                    longest_proj_name = q;
                }
            else
                {
                    int len = proj_head->get_projNode_filter()->match(ln);
                    if (len >= 0)
                        {
                            char *q = (char *)qq.str();
                            if (q[len] == '/')
                                q[len] = '\0';
                            else
                                {
                                    char *p = strchr(&q[len], '/');
                                    if (p)
                                        *p = '\0';
                                    len = proj_head->get_projNode_filter()->match_to_end(q);
                                    Assert(len == strlen(q));
                                }
                            longest_proj_name = q;
                        }
                }
        }
    else
        {
            char *p = 0;
            char const *phy_path = proj_head->get_dir_name();
            if (ln_tp && file_flag && strstr(phy_path,"**"))
                p = strrchr(ln,'/');
            if (p)
                {
                    *p = '\0';
                    longest_proj_name = ln;
                    *p= '/';
                }
            else
                longest_proj_name = ln;
        }
}

void projMap::set_para_rule_type(int tp)
{
    Initialize(projMap::set_para_rule_type);

    para_rule_type |= tp;
}

bool projMap::is_para_rule_type(int tp)
{
    Initialize(projMap::is_para_rule_type);

    return (para_rule_type & tp);
}


bool projMap::is_script()
// check if this projmap belongs to a script project
// if its root is a script project, it is and returns TRUE;
// Otherwise, it returns FALSE.
{
    Initialize(projMap::is_script);

    if (get_parent_project())
        return get_parent_project()->is_script();

    for (int i=1; i<get_multi_phy_rule()->size(); i++)
        {
            int rule_type = get_phy_rule_type(i);
            if (rule_type == MAP_SCRIPT_TYPE)
                return TRUE;
        }
    return FALSE;
}

bool projMap::is_filter()
// check if this projmap belongs to a filter project
// if its root is a filter project, it is and returns TRUE;
// Otherwise, it returns FALSE.
{
    Initialize(projMap::is_filter);

    if (get_parent_project())
        return get_parent_project()->is_filter();

    for (int i=1; i<get_multi_phy_rule()->size(); i++)
        {
            int rule_type = get_phy_rule_type(i);
            if (rule_type == MAP_FILTER_TYPE)
                return TRUE;
        }
    return FALSE;
}

void projMap::get_script_text(projNode *pn, genString &s)
// use the physical name in the projnode to replace the index in the script definition
//
// pn: the projnode
// s: return the script
{
    Initialize(projMap::get_script_text);

    s = 0;
    wkPtnRule *rule;
    for (int i=1; i<get_multi_phy_rule()->size(); i++)
        {
            int rule_type = get_phy_rule_type(i);
            if (rule_type == MAP_SCRIPT_TYPE)
                {
                    rule = get_phy_rule(i);
                    break;
                }
        }

    if (!rule)
        return;

    if (rule->no_idx())
        {
            s = rule->get_ptn_def();
            return;
        }

    // get the top directory
    genString path;
    pn->ln_to_fn_imp(pn->get_ln(), path, DIR_TYPE);

    int match_count = get_phy_match_count();
    init_working_buffer(match_count);
    int cur_len;
    int full_match_flag = 0;
    int sidx = 0;
    int status = new_split_ptn_path(path, get_phy_rule(0), &cur_len, &full_match_flag, sidx);
    rule->fill_idx(s, path);
}

void projMap::get_filter_text(genString &s)
// get the defined filter that is returned by s
{
    Initialize(projMap::get_filter_text);

    s = 0;
    for (int i=1; i<get_multi_phy_rule()->size(); i++)
        {
            int rule_type = get_phy_rule_type(i);
            if (rule_type == MAP_FILTER_TYPE)
                {
                    wkPtnRule *rule = get_phy_rule(i);
                    s = rule->get_ptn_def();
                    return;
                }
        }
}

bool projMap::some_child_no_leading_slash()
{
    Initialize(projMap::some_child_no_leading_slash);

    projMap *cur = get_child_project();
    while(cur)
        {
            if (!leading_char(cur->get_dir_name(),'/'))
                return TRUE;
            cur = cur->get_next_project();
        }
    return FALSE;
}

extern "C" void dbg_map_info(projMap *pm)
{
    Initialize(dbg_map_info);

    if (pm->get_org_project_name())
        msg("proj  = $1") << pm->get_org_project_name() << eom;
    else
        msg("proj is NULL") << eom;

    msg("phy   = $1") << pm->get_org_dir_name() << eom;

    if (pm->get_org_logic_name())
        msg("logic = $1") << pm->get_org_logic_name() << eom;
    else
        msg("logic is NULL") << eom;

    msg("line = $1") << pm->get_line_num() << eom;
    ;
}

// FILTER

projFilter::projFilter(char const *s) : str(s), filter(0), next(0)
                                  // the constructor for projFilter
                                  //
                                  // s: is the pattern
{
    if (s)
        filter = (WildPair *)db_new(WildPair, (s));
}

projFilter::~projFilter()
// the destructor for projFilter
{
    if (filter)
        delete filter;

    if (next)
        delete next;
}


bool projFilter::file_name_match(char const *path, char dir_flag, char match_only_one)
// check if the path matches to the defined pattern.
//
// path: is the checking path
// dir_flag: DIR_TYPE  => need to remove the filename
//           FILE_TYPE => need to remove the directory
//           DIR_TYPE|FILE_TYPE => check the whole path
// match_only_one: MATCH_ANY: If this one does not match, try the next pattern.
//                 MATCH_THIS_ONE: If this one does not match, it fails.
{
    Initialize(projFilter::file_name_match);

    // no filter: means to accept all files
    if (this == 0)
        return TRUE;

    genString  q = path;
    char      *p = q;
    char      *s;

    // calling from full spec, it is a directory
    if (dir_flag == DIR_TYPE)
        {
            // get the directory portion
            s = (char *)strchr(p,'/');
            if (s)
                *s = '\0';
        }
    else if (dir_flag == FILE_TYPE)
        {
            s = (char *)strrchr(p,'/');
            if (s)
                p = s+1;
        }
    // else if (dir_flag == (FILE_TYPE|DIR_TYPE)) ; // compare the whole string

    int len0 = filter->match_to_end(p);
    if (len0 < 0)
        {
            if (next && match_only_one == MATCH_ANY)
                return next->file_name_match(p, dir_flag, match_only_one);
            return FALSE;
        }

    return TRUE;
}

// APPLICATION

static int is_dir_phy_path(projMap *cur)
{
    Initialize(is_dir_phy_path);

    char const *p = cur->get_phy_rule(0)->get_org_ptn();
    if (trailing_char (p, '/'))
        return 1;

    return 0;
}

static void adjust_idx_offset(wkIdxRule *rule, int *offset)
// the index in the rule is either for project or for logic
// offset contains the offset for these index
{
    Initialize(adjust_idx_offset);

    wkIdxDef *cur = rule->get_idx_strs();

    while(cur)
        {
            if (cur->get_type() == IDX_MATCH_INDEX)
                {
                    int idx = cur->get_index() - 1;
                    cur->set_index(cur->get_index() + offset[idx]);
                }

            cur = cur->get_next();
        }
}

static int num_of_two_stars(char const *str)
// count the number of **
{
    Initialize(num_of_two_stars);

    int count = 0;
    while(1)
        {
            char const *p = strstr(str,"**");
            if (!p)
                break;
            count++;
            str = p+2;
        }
    return count;
}

static void adjust_idx(projMap *cur)
// adjust idx in logic rule and project rule
// 1. reduce by 1 because it start from 0 (pdf starts from 1)
// 2. if ** in physical rule, then increase by 1 because it embeds one extra ()
{
    Initialize(adjust_idx);

    int max_idx_cnt = cur->get_phy_match_count();

    if (!max_idx_cnt)
        return;

    int *offset = (int *)psetmalloc(sizeof(int) * max_idx_cnt);
    int i;
    for (i=0; i<max_idx_cnt; i++)
        offset[i] = -1;

    int idx = 0;
    for (i=0; i<cur->get_multi_phy_rule()->size(); i++)
        {
            wkPtnRule *phy_rule = cur->get_phy_rule(i);
            wkPtnDef *cur_ptn = phy_rule->get_ptn_strs();
            while(cur_ptn)
                {
                    if (cur_ptn->get_type() == PTN_CONST_STRING)
                        {
                            int num = num_of_two_stars(cur_ptn->get_str());
                            if (num)
                                {
                                    for (int j=idx; j<max_idx_cnt; j++)
                                        offset[j] += num;
                                }
                        }
                    else if (cur_ptn->get_type() == PTN_MATCH_MAPPING)
                        {
                            idx++;
                        }
                    cur_ptn = cur_ptn->get_next();
                }
        }

    adjust_idx_offset(cur->get_project_rule(),offset);
    adjust_idx_offset(cur->get_logic_rule(),offset);

    psetfree(offset);
    wkIdxRule *logic_rule = cur->get_logic_rule();
}

static void must_be_file_node(projMap *cur, char *proj_name)
// this projmap is a file
//
// cur: is the tested projMap
// proj_name: is the rule used so far
{
    Initialize(must_be_file_node);

    // If it is not a leaf, it must be [[ d ]], [[ D ]], or [[ dD ]].
    if (cur->get_file_type() == DEFAULT_TYPE)
        ; // cur->set_file_type(FILE_TYPE | LINK_FILE);
    else
        {
            if (cur->get_file_type() & (~(FILE_TYPE | LINK_FILE)))
                create_one_path_parser_error(PATH_PARSE_BAD_SELECTOR_FOR_FILE_NODE_ERR, proj_name, 0, cur->get_line_num(),
                                             0);
            cur->set_file_type(cur->get_file_type() & (FILE_TYPE | LINK_FILE));
        }

    // check if the member is set
    if (cur->get_member() == 0)
        cur->set_member(cur->get_phy_rule(0)->get_org_ptn());

    if (cur->get_direction() == TWO_WAY)
        {
            char const *path = cur->get_org_dir_name();
            if (strncmp(path,"/**",3) == 0 || strncmp(path,"/(**)",5) == 0)
                {
                    char const *h = "/**";
                    if (strncmp(path,"/(**)",5) == 0)
                        h = "/(**)";
                    create_one_path_parser_error(PATH_PARSE_LONG_EXPAND_WRN, h, 0, cur->get_line_num(),
                                                 0);
                }
        }
}

static void must_be_dir_node(projMap *cur, char *proj_name)
// this projmap is a directory
//
// cur: is the tested projMap
// proj_name: is the rule used so far
{
    Initialize(must_be_dir_node);

    // If it is not a leaf, it must be [[ d ]], [[ D ]], or [[ dD ]].
    if (cur->get_file_type() == DEFAULT_TYPE)
        ; // cur->set_file_type(DIR_TYPE | LINK_DIR);
    else
        {
            if (cur->get_file_type() & (~(DIR_TYPE | LINK_DIR)))
                create_one_path_parser_error(PATH_PARSE_BAD_SELECTOR_FOR_DIR_NODE_ERR, proj_name, 0, cur->get_line_num(),
                                             0);
            cur->set_file_type(cur->get_file_type() & (DIR_TYPE | LINK_DIR));
        }
}

static void check_child_for_imp_proj(projMap *cur, char *proj_name)
{
    Initialize(check_child_for_imp_proj);

    projMap *child = cur->get_child_project();

    while(child)
        {
            // a = $A <-> /a { include { *.h } } is bad
            if (child->get_child_project())
                {
                    create_one_path_parser_error(PATH_PARSE_IMP_CHILD_SUB_PROJ_ERR, proj_name, 0, child->get_line_num(), 0);
                    break;
                }

            child = child->get_next_project();
        }
}

static bool log_path_with_leading_slash(wkIdxRule *rule)
{
    Initialize(log_path_with_leading_slash);

    // must use the original definition
    char const *org_idx = rule->get_org_idx();

    if (leading_char (org_idx, '/'))
        return TRUE;

    return FALSE;
}

static bool phy_path_with_leading_slash(wkPtnRule *rule)
{
    Initialize(phy_path_with_leading_slash);

    // must use the converted path
    // For example ~ or $ROOT, will be converted to a path started with a slash
    char const *ptn_def = rule->get_ptn_def();

    if (leading_char (ptn_def, '/'))
        return TRUE;

    return FALSE;
}

static void check_slash_for_proj_name(projMap *cur, char *proj_name)
{
    Initialize(check_slash_for_proj_name);

    if (log_path_with_leading_slash(cur->get_project_rule()) && cur->get_direction() != REAL_RIGHT_WAY)
        {
            create_one_path_parser_error(PATH_PARSE_SUB_PROJ_NAME_START_WITH_SLASH_WRN, proj_name, 0, cur->get_line_num(),
                                         0);
            char const *s = cur->get_project_rule()->get_idx_def() + 1; // skip the '/'
            while(*s == '/')
                s++;
            if (*s == '\0')
                create_one_path_parser_error(PATH_PARSE_NO_PROJ_NAME_ERR, proj_name, 0, cur->get_line_num(), 0);
            cur->get_project_rule()->set_idx(s, TWO_WAY);
        }
}

static void check_idx_rule_leading_slash(wkIdxRule *rule, projMap *cur, char *proj_name)
{
    Initialize(check_idx_rule_leading_slash);

    if (!log_path_with_leading_slash(rule))
        {
            create_one_path_parser_error(PATH_PARSE_TOPPROJ_LOGSLASH_ERR, proj_name, 0, cur->get_line_num(), 0);
            genString s = "/";
            s += rule->get_idx_def();
            rule->set_idx(s, cur->get_direction()); // preappend '/' for the logic name
        }
}

static void check_ptn_rule_leading_slash(wkPtnRule *rule, projMap *cur, char *proj_name, char rule_type,
                                         char idx_const_string)
{
    Initialize(check_ptn_rule_leading_slash);

    if (!phy_path_with_leading_slash(rule))
        {
            create_one_path_parser_error(PATH_PARSE_TOPPROJ_PHYSLASH_WRN, proj_name, 0, cur->get_line_num(), 0);
            genString s = "/";
            s += rule->get_org_ptn();
            // preappend '/' for the physical name in the logic rule
            rule->set_ptn(cur->get_exclude_type(), s, cur->get_direction(), rule_type, idx_const_string || cur->selector_matched_pattern(), 1, 0);
        }
}

static void check_leading_slash(projMap *cur, char *proj_name)
{
    Initialize(check_leading_slash);

    // project rule should not start with a slash
    // check_idx_rule_leading_slash(cur->get_project_rule(), cur, proj_name);

    // logic rule
    check_idx_rule_leading_slash(cur->get_logic_rule(), cur, proj_name);

    int idx = 0;
    while(1)
        {
            int rule_type = cur->get_phy_rule_type(idx);
            if (rule_type == END_PHY_PATH)
                break;

            if (rule_type == NORMAL_PHY_PATH)
                {
                    wkPtnRule *rule = cur->get_phy_rule(idx);
                    check_ptn_rule_leading_slash(rule, cur, proj_name, rule_type, !cur->is_idx_contain_match_index());
                }
            idx++;
        }
}

#define MATCH_PTN_CONTANST 1
#define MATCH_LOGIC_IDX    2
#define MATCH_PROJECT_IDX  4

static int empty_spot(char *bit_map, int len, int bit_pattern)
// check if any matching field in the physical path could not be mapped back from the logic or project name
// bit_map contains the definition for the idx fields of the logic name or project name and the const field in the
//                  physical path
// bit_pattern = MATCH_LOGIC_IDX: check the logic name
//               MATCH_PROJECT_IDX: check the project name
{
    Initialize(empty_spot);

    for(int i=0;i<len;i++)
        {
            if ((bit_map[i] & (bit_pattern | MATCH_PTN_CONTANST)) == 0)
                return TRUE;
        }
    return FALSE;
}

static void check_idxrule_idx(wkIdxRule *rule, int max_idx_cnt, char *proj_name, int line_num,
                              char direction, char *bit_map, int bit_pattern)
// check if the index defined in project (logic) name is OK
//
// rule: is the definition for project or logic name
//
// max_idx_cnt is the number of the matched patterns in the physical path (not include **)
//
// proj_name: the project name for this projnode
//
// line_num: The line number in the pdf file where this definition resides
//
// direction: for ->, <->, or => between physical and logic
//
// bit_map: contain 3 bits
//          MATCH_PTN_CONTANST: If this bit is set, this field is constant which does not require logic or project name
//                              to match back
//          MATCH_LOGIC_IDX:    If this bit is set, his filed is defined in logic name.
//          MATCH_PROJECT_IDX:  If this bit is set, his filed is defined in project name.
//
// bit_pattern = MATCH_LOGIC_IDX: for logic name
//               MATCH_PROJECT_IDX: for project name
{
    Initialize(check_idxrule_idx);

    char const *name;
    if (bit_pattern == MATCH_PROJECT_IDX)
        name = "project";
    else
        name = "logical";

    wkIdxDef *cur = rule->get_idx_strs();
    while(cur)
        {
            if (cur->get_type() == IDX_MATCH_INDEX)
                {
                    if (cur->get_index() > max_idx_cnt || cur->get_index() < 1)
                        {
                            create_one_path_parser_error(PATH_PARSE_BIG_IDX_ERR, 
                                                         proj_name, name, line_num, cur->get_index(), 
                                                         max_idx_cnt);
                            break;
                        }
                    else
                        bit_map[cur->get_index()-1] |= bit_pattern;
                }
            cur = cur->get_next();
        }

    if (direction == LEFT_WAY || direction == TWO_WAY)
        {
            if (empty_spot(bit_map,max_idx_cnt,bit_pattern))
                create_one_path_parser_error(PATH_PARSE_INCOMPLETE_LOG_TO_PHY_ERR, name, 0,
                                             line_num, 0, 0);
        }
}

static void verify_selector_idx(wkPtnRule *rule, int rule_type, int max_idx_cnt,
                                projMap *pm, char *proj_name)
// verify the idx fields in the selectors
//
// rule: the secector definition
// rule_type: the type for selector; please refer to projMap::create_one_phy_rule()
// max_idx_cnt: The max idx allowed in this selector
// pm: the checked projmap
// proj_name: the relation from the root projmap to this pm
{
    Initialize(verify_selector_idx);

    if (!rule)
        return;

    wkPtnDef *cur = rule->get_ptn_strs();
    while(cur)
        {
            if (cur->get_type() == PTN_IDX      &&
                (cur->get_idx() > max_idx_cnt-1 ||
                 (cur->get_idx() < 0  && rule_type != MAP_SCRIPT_TYPE ||
                  cur->get_idx() < -1 && rule_type == MAP_SCRIPT_TYPE )))
                {
                    create_one_path_parser_error(PATH_PARSE_BIG_IDX_ERR, proj_name, "physical name",
                                                 pm->get_line_num(), cur->get_idx()+1, max_idx_cnt);
                    break;
                }
            cur = cur->get_next();
        }
}

static void fill_constant_field(projMap *pm, char *bit_map)
// Before to check if the index in the logic or project name could map back into physical pattern
// let"s find the constant pattern in the physical path which does not require the match form the
// index string
//
// pm: is the checking projmap
//
// bit_map: has the length of the number of pattern in the physical pattern
//          3 bit will be used from right
//          bit 0 is for MATCH_PTN_CONTANST
//          bit 1 is for MATCH_LOGIC_IDX
//          bit 2 is for  MATCH_PROJECT_IDX

{
    Initialize(fill_constant_field);

    int idx = 0;
    for (int i=0; i<pm->get_multi_phy_rule()->size(); i++)
        {
            wkPtnDef *cur = pm->get_phy_rule(i)->get_ptn_strs();

            while(cur)
                {
                    if (cur->get_type() == PTN_MATCH_MAPPING)
                        {
                            if (!need_expand_dir(cur->get_str()))
                                bit_map[idx] |= MATCH_PTN_CONTANST;
                            idx++;
                        }
                    else if (i == 0 && cur->get_type() == PTN_CONST_STRING)
                        {
                            if (need_expand_dir(cur->get_str()) &&
                                (pm->get_direction() == LEFT_WAY || pm->get_direction() == TWO_WAY))
                                create_one_path_parser_error(PATH_PARSE_INCOMPLETE_LOG_TO_PHY_ERR, 0, 0,
                                                             pm->get_line_num(), 0, 0);
                        }
                    cur = cur->get_next();
                }
        }
}


static void check_failed_rule(projMap *cur, char *proj_name)
// check if the defined physical rules is good
//
// cur: is the being checked projmap.
// proj_name: is the rule defined so far from which it could find out the path from
//            this map to its root.
{
    Initialize(check_failed_rule);

    if ( proj_name )
        {
            int   lvl = 0;
            for (char *phy_text= proj_name; *phy_text; phy_text++ )
                {
                    if ( *phy_text == '(' )
                        lvl++;
                    else if ( *phy_text == ')' )
                        lvl--;
                    else if ( lvl == 0 && *phy_text == '|' )
                        {
                            create_one_path_parser_error(PATH_PARSE_PIPE_IN_PATH_ERR, proj_name, 0, cur->get_line_num(), 0);
                            break;
                        }
                }
        }

    char last_type_is_not_for_link = 0;
    if (cur->get_file_type() == 0)
        create_one_path_parser_error(PATH_PARSE_RULE_FAILED_ERR, proj_name, 0, cur->get_line_num(), 0);
    else if ((cur->get_file_type() != DEFAULT_TYPE) &&
             (cur->get_file_type() & (LINK_DIR | LINK_FILE)) == 0)
        last_type_is_not_for_link = 1;

    int max_idx_cnt = cur->get_phy_rule(0)->get_max_ptn_count();

    int first = 1;
    for (int i=1; i<cur->get_multi_phy_rule()->size(); i++)
        {
            int rule_type = cur->get_phy_rule_type(i);

            phy_rule_entry *p = (*cur->get_multi_phy_rule())[i];

            if (rule_type == MAP_SCRIPT_TYPE || rule_type == MAP_FILTER_TYPE)
                {
                    if (cur->get_parent_project())
                        {
                            if (first)
                                {
                                    first = 0;
                                    create_one_path_parser_error(PATH_PARSE_SCRIPT_NOT_TOP_PROJ_WRN, 0, 0, cur->get_line_num(), 0);
                                }
                        }
                    else
                        {
                            if (cur->get_multi_phy_rule()->size() > 2)
                                {
                                    create_one_path_parser_error(PATH_PARSE_SCRIPT_MORE_TOP_PROJ_WRN, 0, 0, cur->get_line_num(), 0);
                                    break;
                                }
                        }
                }
            if (rule_type == MAP_FILE_TYPE)
                {
                    if ((p->file_type & (DIR_TYPE | FILE_TYPE)) == 0 && last_type_is_not_for_link)
                        create_one_path_parser_error(PATH_PARSE_RULE_FAILED_ERR, proj_name, 0, cur->get_line_num(), 0);

                    last_type_is_not_for_link = 0;
                    if (p->file_type == 0)
                        create_one_path_parser_error(PATH_PARSE_RULE_FAILED_ERR, proj_name, 0, cur->get_line_num(), 0);
                    else if ((p->file_type & (LINK_DIR | LINK_FILE)) == 0)
                        last_type_is_not_for_link = 1;
                }
            else
                {
                    if (rule_type == LINK_MAP_PHY_PATH && last_type_is_not_for_link)
                        create_one_path_parser_error(PATH_PARSE_RULE_FAILED_ERR, proj_name, 0, cur->get_line_num(), 0);

                    if (rule_type == REALPATH_MAP_PHY_PATH)
                        last_type_is_not_for_link = 1;
                    else
                        last_type_is_not_for_link = 0;

                    verify_selector_idx(cur->get_phy_rule(i), rule_type, max_idx_cnt, cur, proj_name);
                    max_idx_cnt += cur->get_phy_rule(i)->get_max_ptn_count();
                }
        }

    //  if (!cur->is_script())
    {
        char *bit_map = (char *)calloc(sizeof(char), max_idx_cnt);

        fill_constant_field(cur, bit_map);

        check_idxrule_idx(cur->get_project_rule(), max_idx_cnt, proj_name, cur->get_line_num(),
                          cur->get_direction(), bit_map, MATCH_PROJECT_IDX);
        check_idxrule_idx(cur->get_logic_rule(),   max_idx_cnt, proj_name, cur->get_line_num(),
                          cur->get_direction(), bit_map, MATCH_LOGIC_IDX);

        psetfree(bit_map);
    }
}

static bool different_phy_path(wkPtnRule *rule, wkPtnRule *root_rule)
{
    Initialize(different_phy_path);

    char const *ptn_def = rule->get_ptn_def();
    char const *root_ptn_def = root_rule->get_ptn_def();

    if (!leading_char(root_ptn_def,'/'))
        return true;

    int len = strlen(root_ptn_def);

    if (strncmp(root_ptn_def, ptn_def, len))
        return true;

    if (strlen(ptn_def+len) && !leading_char(ptn_def+len,'/'))
        return true;

    return false;
}

static void check_leading_slash_in_child_phy_path(projMap *cur, int deep_flag)
// check if the child project contains leading slash in its physical path;
//     if it is, set a flag in this projMap for quick reference later.
//
// cur: is the checked projMap
//
// deep_flag = 0: go through the link for next projects
//           = 1: do not go through the link for next projects
{
    Initialize(check_leading_slash_in_child_phy_path);

    wkPtnRule *rule = cur->get_phy_rule(0);
    if (phy_path_with_leading_slash(rule)) // && !leading_char(rule->get_org_ptn(),'~'))
        log_root_path(rule->get_ptn_def());

    if (cur->get_parent())
        {
            wkPtnRule *rule = cur->get_phy_rule(0);
            if (phy_path_with_leading_slash(rule))
                {
                    projMap *par = cur->get_parent_project();
                    while(par)
                        {
                            wkPtnRule *par_rule = par->get_phy_rule(0);
                            if (different_phy_path(rule, par_rule))
                                par->set_leading_slash_in_child_phy_path();
                            par = par->get_parent_project();
                        }
                }

            if (!deep_flag)
                {
                    projMap *next = cur->get_next_project();
                    while (next)
                        {
                            check_leading_slash_in_child_phy_path(next, 1);
                            next = next->get_next_project();
                        }
                }
        }

    projMap *child = cur->get_child_project();
    if (child)
        check_leading_slash_in_child_phy_path(child, 0);
}

static void fix_project_file_type(projMap *cur, char const *rule, int deep_flag)
// check if the selector [[fF]] or [[dD]] is good
// adjust the idx in project and logic rules
//
// cur: is the checked projMap
//
// rule: is the connection from the root project to this projMap
//
// deep_flag = 0: go through the link for next projects
//           = 1: do not go through the link for next projects
{
    Initialize(fix_project_file_type);

    genString new_rule;
    update_the_using_rule(new_rule, rule, cur);

    if (!cur->is_leaf())
        must_be_dir_node(cur, new_rule);
    else // leaf node
        {
            if (is_dir_phy_path(cur))
                must_be_dir_node(cur, new_rule);
            else
                {
                    if (cur->is_implicit_sub_dir_proj()) // '='
                        must_be_dir_node(cur, new_rule);
                    else // ':'
                        must_be_file_node(cur, new_rule);
                }
        }

    if (cur->get_parent() == 0)
        check_leading_slash_in_child_phy_path(cur, 0);

    // CHECKING
    if (cur->is_implicit_sub_dir_proj()) // '='
        check_child_for_imp_proj(cur, new_rule);

    if (cur->get_parent_project() == 0)
        {
            if (file_project(cur, "", 1))
                create_one_path_parser_error(PATH_PARSE_ROOT_PROJ_FILE_WRN, new_rule, 0, cur->get_line_num(), 0);
            check_leading_slash(cur, new_rule);
        }

    check_slash_for_proj_name(cur, new_rule);

    check_failed_rule(cur, new_rule);

    // let's fix the index here
    adjust_idx(cur);

    if (cur->get_child_project())
        fix_project_file_type(cur->get_child_project(), new_rule, 0);

    if (deep_flag)
        return;

    cur = cur->get_next_project();
    while(cur)
        {
            fix_project_file_type(cur, rule, 1);
            cur = cur->get_next_project();
        }
}

static void fix_real_path(projMap *cur, int deep_flag)
// fix the link in the physical path
//
// cur: is the checked projMap
// deep_flag = 0: go through the link for next projects
//           = 1: do not go through the link for next projects
{
    Initialize(fix_real_path);

    wkPtnRule *rule = cur->get_phy_rule(0);
    if (phy_path_with_leading_slash(rule) && leading_char(rule->get_org_ptn(),'~'))
        {
            char const *path = rule->get_ptn_def();
            char pathname[MAXPATHLEN];
            if (projMap_convert_realpath(path,pathname))
                {
                    if (strcmp(path,pathname))
                        rule->set_ptn(cur->get_exclude_type(), pathname, cur->get_direction(), cur->get_phy_rule_type(0),
                                      !cur->is_idx_contain_match_index() || cur->selector_matched_pattern(), 0, 0);
                }
        }

    if (cur->get_child_project())
        fix_real_path(cur->get_child_project(), 0);

    if (deep_flag)
        return;

    cur = cur->get_next_project();
    while(cur)
        {
            fix_real_path(cur, 1);
            cur = cur->get_next_project();
        }
}

// INITIALIZE
static void init_working_variables()
{
    Initialize(init_working_variables);

    proj_rule_cur   = 0;
    cur_parser_p    = 0;
    proj_rule_head  = 0;
}

static void check_root_dir(projMap *cur)
// cur is the projmap.
// It checks if the dir exists.
// It does not check the root directory for the rule project
//    if every child has leading slash.
{
    Initialize(check_root_dir);

    genString path = cur->get_dir_name();
    if (need_expand_dir(path))
        return;
    remove_back_slash_and_parentheses((char *)path.str());

    int verify_flag = 0;
    if (cur->get_org_project_name() && strncmp(cur->get_org_project_name(),"__",2) == 0)
        {
            if (cur->some_child_no_leading_slash())
                verify_flag = 1;
        }
    else if (leading_char(path,'/') && cur->get_child_project())
        verify_flag = 1;

    if (verify_flag)
        {
            struct OStype_stat buf;

#ifdef _WIN32
            if (fileCache_stat(path.str(), &buf) || _S_IFDIR & buf.st_mode == 0)
#else
                if (fileCache_stat(path.str(), &buf) || !S_ISDIR(buf.st_mode))
#endif
                    create_one_path_parser_error(PATH_PARSE_ROOT_DIR_NOT_EXIST_WRN, cur->get_org_dir_name(), (char *)path.str(),
                                                 cur->get_line_num(), 1);
        }
}

static void fix_project_view_flag(projMap *cur, int deep_flag)
// cur is the projmap that is being checked
//
// fix the view flag for the projmap and check if the dir exists.
//
// deep_flag = 0: go through the link for next projects
//           = 1: do not go through the link for next projects
{
    Initialize(fix_project_view_flag);

    if (cur->get_child_project())
        fix_project_view_flag(cur->get_child_project(), 0);

    if (!deep_flag && cur->get_next_project())
        {
            projMap *next = cur->get_next_project();
            while(next)
                {
                    fix_project_view_flag(next, 1);
                    next = next->get_next_project();
                }
        }

    check_root_dir(cur);

    if (cur->get_direction() == LEFT_WAY  ||
        cur->get_direction() == REAL_RIGHT_WAY ||
        cur->is_invisible_project())
        {
            cur->set_view_flag(0);
            return;
        }

    if (cur->get_direction() == RIGHT_WAY)
        {
            cur->set_view_flag(0);
            return;
        }

    // if all children will not be shown, so does this one

    if (cur->is_implicit_sub_dir_proj())
        return;

    projMap *child = cur->get_child_project();
    if (child)
        {
            while(child)
                {
                    if (child->get_view_flag())
                        return;

                    child = child->get_next_project();
                }

            cur->set_view_flag(0);
        }
}

static void report_para_rule_type(projMap *cur, int deep_flag)
// cur is the interested projmap
// If it is a rule, returns the rule type.
//
// deep_flag = 0: go through the link for next projects
//           = 1: do not go through the link for next projects
{
    Initialize(report_para_rule_type);

    genString phy = cur->get_dir_name();
    remove_back_slash_and_parentheses((char *)phy.str());
    char *p = 0;
    if(phy.length())
        {
            int type  = OTHER_RULE;
            char *str = (char *)phy.str();
            p = strstr(str, ".makemf");
            if (p)
                type = MAKEMF_RULE;
            if (type == OTHER_RULE)
                {
                    p = strstr(str, ".make");
                    if (p)
                        type = MAKE_RULE;
                }
            if (type == OTHER_RULE)
                {
                    p = strstr(str, ".pmoddir");
                    if (p)
                        type = PMODDIR_RULE;
                }
            if (type == OTHER_RULE)
                {
                    p = strstr(str, ".pset");
                    if (p)
                        type = PSET_RULE;
                }
            char *percent_ptr = strstr(str, "/%/");
            if (!percent_ptr)
                {
                    if (type != OTHER_RULE)
                        {
                            create_one_path_parser_error(PATH_PARSE_MISS_PERCENT_WRN, str);
                            percent_ptr = NULL;
                        }
                }
            else
                {
                    if (type != OTHER_RULE)
                        if (percent_ptr + 3 != p)
                            type = OTHER_RULE;
                }
            if (percent_ptr != NULL)
                {
                    projMap *par = cur;
                    while(par)
                        {
                            par->set_para_rule_type(type);
                            par = par->get_parent_project();
                        }
                }
        }

    projMap *child = cur->get_child_project();
    if (child)
        report_para_rule_type(child, 0);

    if (deep_flag)
        return;

    cur = cur->get_next_project();
    while(cur)
        {
            report_para_rule_type(cur, 1);
            cur = cur->get_next_project();
        }
}

static void check_same_project_name(projMap *cur)
// check if the project name defined by cur has been defined before
{
    Initialize(check_same_project_name);

    while(cur) {
        wkIdxRule *p0 = cur->get_project_rule();
        if (!cur->is_implicit_sub_dir_proj() && cur->get_child_project() == 0) {
            create_one_path_parser_error(PATH_PARSE_TOP_PROJECT_NO_CHILD_ERR, 0, 0,
                                         cur->get_line_num(), 0, 0);
            return;
        }
        if (cur->get_direction() == REAL_RIGHT_WAY) {
            create_one_path_parser_error(PATH_PARSE_TOP_PROJECT_REAL_RIGHT_ERR, 0, 0,
                                         cur->get_line_num(), 0, 0);
            return;
        }
        if (p0->is_constant_rule()) {
            char const *proj_name = cur->get_org_project_name();
            projMap *cur1 = cur->get_next_project();
            while(cur1) {
                wkIdxRule *p1 = cur1->get_project_rule();
                if (p1->is_constant_rule()) {
                    char const  *proj_name1 = cur1->get_org_project_name();
                    if (strcmp(proj_name, proj_name1) == 0) {
                        // BAD!
                        create_one_path_parser_error(PATH_PARSE_SAME_PROJECT_NAME_ERR, proj_name, 0,
                                                     cur->get_line_num(),
                                                     cur1->get_line_num(),
                                                     0);
                        return;
                    }
                }
                cur1 = cur1->get_next_project();
            }
            check_other_file_project_names(cur, proj_name);
        }
        cur = cur->get_next_project();
    }
}

static int not_paradoc_pdf(char const *fn)
{
    Initialize(not_paradoc_pdf);

    char const *q = strrchr(fn,'/');
    if (q)
        q++;
    else
        q = fn;
    return strcmp(q,"ParaDOCS.pdf");
}

static int just_find_left_brace = 0;
static void check_empty_project(char *p, int line_count)
{
    Initialize(check_empty_project);

    int in_quote = 0;
    while(*p && *p != '\n')
        {
            if (*p == '\\')
                p++;
            else if (*p == '"')
                {
                    if (in_quote)
                        in_quote = 0;
                    else
                        in_quote = 1;
                }
            else if (!in_quote)
                {
                    if (*p == '{')
                        just_find_left_brace = 1;
                    else
                        {
                            if (*p == '}' && just_find_left_brace)
                                create_one_path_parser_error(PATH_PARSE_EMPTY_PROJ_ERR, 0, 0,
                                                             line_count, 0, 0);
                            just_find_left_brace = 0;
                        }
                }
            p++;
        }
}

static bool pdf_contain_long_string(char const *fn)
{
    Initialize(pdf_contain_long_string);

    struct OStype_stat buf;
    if (fileCache_stat(fn,&buf))
        return TRUE;

    int size = (int)buf.st_size;

    char *line = (char *)psetmalloc(size+1);
    FILE *in = OSapi_fopen(fn,"r");
    just_find_left_brace = 0;

    int status = FALSE;
    int line_count = 1;
    while(OSapi_fgets(line,size,in))
        {
            check_empty_project(line,line_count++);
            if (strlen(line) < YYLMAX)
                continue;
            char *p = line;
            char first_char = 0;
            if (*p == '\"')
                first_char = 1;
            while(1)
                {
                    char *q = strchr(p, '\"');
                    if (!q)
                        break;
                    if (!first_char && *(q-1) == '\\')
                        {
                            p = q+1;
                            first_char = 0;
                            continue;
                        }
                    first_char = 0;

                    int count = 0;
                    q++;
                    while(*q)
                        {
                            if (*q == '\"')
                                break;
                            count++;
                            if (count >= YYLMAX)
                                {
                                    status = TRUE;
                                    break;
                                }
                            if (*q == '\\')
                                {
                                    q++;
                                    if (*q == '\0')
                                        break;
                                }
                            q++;
                        }
                    if (status || *q == '\0')
                        break;
                    p = q+1;
                }
            if (status)
                break;
        }

    psetfree(line);
    return status;
}

// begin for pdf tree generation

static pdf_tree_head *aset_head = 0;
static pdf_tree_head *pth_last;
static int last_line_count = 0;


pdf_tree_head *get_pdf_tree_head()
{
    Initialize(get_pdf_tree_head);

    return aset_head;
}

void projMap::set_num_created_path(int n)
{
    Initialize(projMap::set_num_created_path);

    num_created_path = n;
}

int projMap::get_num_created_path()
{
    Initialize(projMap::get_num_created_path);

    return num_created_path;
}

void projMap::set_pdf_tree_head(pdf_tree_head *p)
{
    Initialize(projMap::set_pdf_tree_head);

    pth = p;
}

pdf_tree_head *projMap::obtain_pdf_tree_head()
{
    Initialize(projMap::obtain_pdf_tree_head);

    return pth;
}

// end of pdf tree generation

void projMap::update_projnode_filter(char const *proj_name, int deep_flag)
// update the projnode_filter of the leaf projMap
//
// proj_name: the projectname so far
// deep_flag: 1: if this is called by a "next" project, then it will not be called recursively for its
//               "next" project.
//            0: it will recursively call its "next" projects.
{
    Initialize(projMap::update_projnode_filter);

    genString new_proj_name;
    new_proj_name.printf("%s/%s",proj_name,get_project_rule()->get_ptn());

    projMap *child = get_child_project();
    if (child)
        child->update_projnode_filter(new_proj_name, 0);
    else
        projNode_filter = (WildPair *)db_new(WildPair, (proj_name));

    if (deep_flag)
        return; // prevent going to the same "next" more than once

    projMap *cur = get_next_project();
    while(cur)
        {
            cur->update_projnode_filter(proj_name, 1);
            cur = cur->get_next_project();
        }
}

// sudha:09/02/97, accept model file .pset etc. by filter
int is_model_file (char *path)
{
    if (strstr(path, ".pset") || strstr(path, ".pmoddir") || strstr(path, ".make") || strstr(path, ".makemf"))
        return 1;
    else
        return 0;
}

bool projMap::accept_by_filter(char const *path, genString &ret_path)
// check if the input path is accepted by the filter
// ret_path contains the path generated by the filter
{
    Initialize(projMap::accept_by_filter);

    ret_path = OSPATH(path);
    if (!is_filter())
        return TRUE;

    if (is_model_file((char *) path))
        return TRUE;

    char result[MAXPATHLEN];

    result[0]='\0';
    if (readpipe && writepipe) {
        char const *real_path = realOSPATH(path);
        OSapi_fprintf(writepipe,"%s\n",real_path);
        OSapi_fflush(writepipe);
        OSapi_fgets(result,MAXPATHLEN,readpipe);
        result[MAXPATHLEN-1] = '\0';
    } else 
        return FALSE;

#ifndef _WIN32
    if ( result[0] )
        result[strlen(result)-1] = '\0'; // remove linefeed
#else
    if ( result[0] && result[1] )
        result[strlen(result)-2] = '\0'; // remove linefeed
#endif
    // return if file returned by filter is not same as path
    ret_path = trn_path_2DIS(result);
    if (ret_path.length() == 0 || ret_path == "''" )
        return FALSE;
    return TRUE;
}

#ifdef _WIN32
#define CHECKLIST_EXE "checklist.exe"
#else
#define CHECKLIST_EXE "checklist"
#endif

void projMap::update_filter(int deep_flag)
// create two way pipe for the project which contains filter
//
// deep_flag = 0: go through the link for next projects
//           = 1: do not go through the link for next projects
{
    Initialize(projMap::update_filter);

    if (is_filter())
        {
            genString f_txt;
            get_filter_text(f_txt);

            char *p = f_txt;

            genTmpfile s0("filter");

            if(strncmp(p,"flist ", 6) == 0){
                p += 6;
                while(*p && isspace(*p))
                    p++;

                f_txt.printf("%s/bin/%s -true 1 -false '' %s", OSapi_getenv("PSETHOME"), CHECKLIST_EXE, p);
            }

            else if(strncmp(p,"elist ", 6) == 0){
                p += 6;
                while(*p && isspace(*p))
                    p++;

                f_txt.printf("%s/bin/%s -true '' -false 1 %s", OSapi_getenv("PSETHOME"), CHECKLIST_EXE, p);
            }

            genString s;
            project_path_sys_vars_translate(f_txt, s);
 
            vpopen2way(s.str(), &readpipe, &writepipe);
            if (!readpipe || !writepipe)
                create_one_path_parser_error(PATH_PARSE_CANT_GEN_FILTER_PIPE_WRN, 0, 0, get_line_num(), 0, 0);

        }

    if (deep_flag)
        return;

    projMap *cur = get_next_project();
    while(cur)
        {
            cur->update_filter(1);
            cur = cur->get_next_project();
        }
}


void projMap::check_filter_and_script(int deep_flag)
// check the root project if its script and filter conatin matching patterns
// or if its filter contains matching index.
//
// deep_flag = 0: go through the link for next projects
//           = 1: do not go through the link for next projects
{
    Initialize(projMap::check_filter_and_script);

    for (int i=1; i<get_multi_phy_rule()->size(); i++)
        {
            int rule_type = get_phy_rule_type(i);
            if (rule_type == MAP_FILTER_TYPE || rule_type == MAP_SCRIPT_TYPE)
                {
                    wkPtnRule *rule = get_phy_rule(i);
                    wkPtnDef  *cur_ptn_def  = rule->get_ptn_strs();
                    while(cur_ptn_def)
                        {
                            if (cur_ptn_def->get_type() == PTN_MATCH_MAPPING)
                                {
                                    if (rule_type == MAP_FILTER_TYPE)
                                        create_one_path_parser_error(PATH_PARSE_MATCH_IN_SCRIPT_ERR, "Filter", 0,
                                                                     get_line_num(), 0, 0);
                                    else // if (rule_type == MAP_SCRIPT_TYPE)
                                        create_one_path_parser_error(PATH_PARSE_MATCH_IN_SCRIPT_ERR, "Script", 0,
                                                                     get_line_num(), 0, 0);
                                }
                            else if (cur_ptn_def->get_type() == PTN_IDX && rule_type == MAP_FILTER_TYPE)
                                create_one_path_parser_error(PATH_PARSE_IDX_IN_FILTER_ERR, 0, 0,
                                                             get_line_num(), 0, 0);
                            cur_ptn_def = cur_ptn_def->get_next();
                        }
                }
        }

    if (deep_flag)
        return;

    projMap *cur = get_next_project();
    while(cur)
        {
            cur->check_filter_and_script(1);
            cur = cur->get_next_project();
        }
}

void projMap::set_root_node(int v)
// set the flag root_node to be v
{
    Initialize(projMap::set_root_node);

    root_node = v;
}


static void pdf_mark_root(projMap *cur)
// mark the root projects
//
// cur is the first root project
{
    Initialize(pdf_mark_root);

    while(cur)
        {
            cur->set_root_node(1);
            cur = cur->get_next_project();
        }
}


static void flat_last_one(projMap *cur)
// flat the last projmap
//
// cur is the head of the link list
{
    Initialize(flat_last_one);

    while(1)
        {
            projMap *next = cur->get_next_project();
            if (next)
                cur = next;
            else
                break;
        }
    if (cur->is_script())
        {
            projMap *prev = cur->get_prev_project();
            cur = pdf_flat_proj(cur);
            if (prev)
                prev->set_next_project(cur);
            else
                proj_rule_head = cur;
        }
}

extern "C" void yy_path_init_input(FILE*);
static bool init_psetProjects_internal (char const *fn)
// the main routine to parse a pdf file
//
// fn: is the pdf file name.
{
    Initialize(init_psetProjects_internal);

    projNode::proj_init();

    init_top_exc_proj_heads();
    projMap::reset_parser_err();
    init_working_variables();
    pmod_projs_init();

    FILE *in;

    if (fn == 0)
        fn = (char *)customize_default_project_definition();

    genString project_file_name;
    project_convert_filename(fn, project_file_name);
    patherr_report_project_file_name (project_file_name);
    fn = project_file_name;

    mpb_incr();

    if (not_paradoc_pdf(fn) && !log_pdf_file_name(fn))
        {
            create_one_path_parser_error(PATH_PARSE_SAME_PROJ_FILE_WRN, 0, 0, 0, 0);
            return TRUE;
        }

    if (is_gui())
        {
            FILE *in = OSapi_fopen(fn,"r");
            if (!in)
                {
                    OSapi_perror(fn);
                    create_one_path_parser_error(PATH_PARSE_PROJ_FILE_NON_EXIST_ERR, 0, 0, 0, 0);
                    return FALSE;
                }

            char line[256];
            OSapi_fgets(line,255,in);
            OSapi_fclose(in);
            line[255] = '\0';
            if (strlen(line))
                line[strlen(line)-1] = '\0';
        }

    genTmpfile s0("filter");
    genString gen_fn = s0.name();

    in = OSapi_fopen(fn,"r");

    if (!in)
        {
            OSapi_perror(fn);
            create_one_path_parser_error(PATH_PARSE_PROJ_FILE_NON_EXIST_ERR, 0, 0, 0, 0);
            return FALSE;
        }

    char line[256];
    if (OSapi_fgets(line,255,in) == 0)
        {
            OSapi_fclose(in);
            return TRUE; // empty file
        }
    OSapi_fclose(in);

    if (pdf_contain_long_string(fn))
        {
            msg("PDF FILE \"$1\" contains a long text string which is longer than \"$2\" bytes") << fn << eoarg << YYLMAX << eom;
            return FALSE;
        }

    mpb_incr();

    if (parser_locate_error())
        return FALSE;

    FILE * pdf_file = OSapi_fopen(fn,"r");

    parse_is_being_executed = 1;
    yy_path_init_input(pdf_file);
    inc_level(1, -1);
    init_path_line_no();
    init_multi_selectors(); // initialize the very first time here.
    mpb_incr();
    // After this, it will be reset after this project is defined.
    int status = pathparse();
    mpb_incr();
    dec_level(1, -1);

    OSapi_fclose(pdf_file);

    if (!proj_rule_head)
        {
            create_one_path_parser_error(PATH_PARSE_NO_PROJ_ERR, 0, 0, 0, 0, 0);
            return FALSE;
        }

    if (!status && !parser_locate_error()) // parse succeeded
        flat_last_one(proj_rule_head);

    int status1 = 0;

    if (!status && !parser_locate_error()) // parse succeeded
        {
            pdf_mark_root(proj_rule_head); // root_node is set here
            if ( !(status = patherr_check_pmod_projs (proj_rule_head)) ) {
                proj_rule_head->update_projnode_filter("", 0);
                // next checking might find some errors
                proj_rule_head->check_filter_and_script(0);
            }
        }

    if (!status && !parser_locate_error()) // parse succeeded
        {
            proj_rule_head->update_filter(0); // readpipe and writepipe is set here

            fix_project_file_type(proj_rule_head, "", 0);

            check_same_project_name(proj_rule_head);

            fix_real_path(proj_rule_head, 0);

            fix_project_view_flag(proj_rule_head, 0);
            report_para_rule_type(proj_rule_head, 0);

            status1 = check_supersede_rule(proj_rule_head, fn);
        }

    mpb_incr();

    if (!status && !parser_locate_error()) // parse succeeded
        {
            pdf_tree_head *pth0 = report_all_maps(proj_rule_head);
            if (!pth0)
                return FALSE;
            //      if (not_paradoc_pdf(fn))
            {
                pdf_tree_head *pth = expand_pdf_tree(pth0);
                update_pdf_tree_line_count(pth, &last_line_count);
                if (PDF_DebugLevel > 0) 
                    {
                        debug_out_one_tree(pth,proj_rule_head);
                    }
                if (!aset_head)
                    aset_head = pth;
                else
                    pth_last->set_next(pth);
                pth_last = pth;
                while(pth_last->get_next())
                    pth_last = pth_last->get_next();
            }
            delete pth0;

            // bool status0 = check_one_phy_one_logic();

            if (not_paradoc_pdf(fn))
                log_projMap_head();
        }

    if (parser_locate_error() || status1) //  || !status0)
        {
            parse_is_being_executed = 0;
            return FALSE;
        }

    mpb_incr();

    void prepare_construct_list_projects();
    prepare_construct_list_projects();
    parse_is_being_executed = 0;

    return TRUE;
}

bool init_psetProjects (char const *fn)
{
    Initialize(init_psetProjects);

    bool retval = init_psetProjects_internal(fn);
    parser_report_errors();

    return retval;
}

static int proj_name_match(char const *targ_s, char const *s)
{
    int len0 = strlen(targ_s);
    int len1 = strlen(s);

    if (len0 < len1)
        return 0;

    if (strncmp(targ_s,s,len1))
        return 0;

    if (targ_s[len1] && targ_s[len1] != '/')
        return 0;

    return 1;
}

void projMap::search_hash(char const *fn, genString &s, char file_tp, char chosen_rule, char trans_type,
                          char const *sub_proj_name)
{
    Initialize(projMap::search_hash);

    s = 0;
    if (fn == 0)
        return;

    path_hash_entry **hh = get_hash_head(file_tp, chosen_rule, trans_type);
    if (hh == 0)
        return;

    int idx = path_hash(fn, MAX_PATH_HASH);

    path_hash_entry *cur = hh[idx];
    if (cur == 0)
        return;

    while(cur)
        {
            if (strcmp(cur->get_fn(),fn) == 0 && proj_name_match(cur->get_proj_n(), sub_proj_name))
                {
                    s = cur->get_ln();

                    return;
                }
            cur = cur->get_next();
        }
}

void projMap::remove_hash(char const *fn, char file_tp, char chosen_rule, char trans_type,
                          char const *sub_proj_name)
{
    Initialize(projMap::remove_hash);

    if (fn == 0)
        return;

    path_hash_entry **hh = get_hash_head(file_tp, chosen_rule, trans_type);
    if (hh == 0)
        return;

    int idx = path_hash(fn, MAX_PATH_HASH);

    path_hash_entry *cur = hh[idx];
    if (cur == 0)
        return;

    path_hash_entry *last = 0;
    while(cur)
        {
            if (strcmp(cur->get_fn(),fn) == 0 && proj_name_match(cur->get_proj_n(), sub_proj_name))
                {
                    if (last == 0)
                        hh[idx] = cur->get_next();
                    else
                        last->set_next(cur->get_next());
                    return;
                }
            last = cur;
            cur = cur->get_next();
        }
}

// file_type:   DIR_TYPE | FILE_TYPE
// chosen_rule: LOGIC_NAME_RULE | PROJECT_NAME_RULE
// trans_type:  FNTOLN | LNTOFN
path_hash_entry **projMap::create_hash_tbl(char file_tp, char chosen_rule, char trans_type)
{
    Initialize(projMap::create_hash_tbl);

    if (file_tp == FILE_TYPE)
        {
            if (chosen_rule == LOGIC_NAME_RULE)
                {
                    if (trans_type == FNTOLN)
                        {
                            if (fn_to_ln_file_h == 0)
                                fn_to_ln_file_h = (path_hash_entry **)calloc(sizeof(path_hash_entry *), MAX_PATH_HASH);
                            return fn_to_ln_file_h;
                        }
                    else // if (trans_type == LNTOFN)
                        {
                            if (ln_to_fn_file_h == 0)
                                ln_to_fn_file_h = (path_hash_entry **)calloc(sizeof(path_hash_entry *), MAX_PATH_HASH);
                            return ln_to_fn_file_h;
                        }
                }
            else // if (chosen_rule == PROJECT_NAME_RULE)
                {
                    if (trans_type == FNTOLN)
                        {
                            if (fn_to_ln_file_imp_h == 0)
                                fn_to_ln_file_imp_h = (path_hash_entry **)calloc(sizeof(path_hash_entry *), MAX_PATH_HASH);
                            return fn_to_ln_file_imp_h;
                        }
                    else // if (trans_type == LNTOFN)
                        {
                            if (ln_to_fn_file_imp_h == 0)
                                ln_to_fn_file_imp_h = (path_hash_entry **)calloc(sizeof(path_hash_entry *), MAX_PATH_HASH);
                            return ln_to_fn_file_imp_h;
                        }
                }
        }
    else // if (file_tp == DIR_TYPE)
        {
            if (chosen_rule == LOGIC_NAME_RULE)
                {
                    if (trans_type == FNTOLN)
                        {
                            if (fn_to_ln_dir_h == 0)
                                fn_to_ln_dir_h = (path_hash_entry **)calloc(sizeof(path_hash_entry *), MAX_PATH_HASH);
                            return fn_to_ln_dir_h;
                        }
                    else // if (trans_type == LNTOFN)
                        {
                            if (ln_to_fn_dir_h == 0)
                                ln_to_fn_dir_h = (path_hash_entry **)calloc(sizeof(path_hash_entry *), MAX_PATH_HASH);
                            return ln_to_fn_dir_h;
                        }
                }
            else // if (chosen_rule == PROJECT_NAME_RULE)
                {
                    if (trans_type == FNTOLN)
                        {
                            if (fn_to_ln_dir_imp_h == 0)
                                fn_to_ln_dir_imp_h = (path_hash_entry **)calloc(sizeof(path_hash_entry *), MAX_PATH_HASH);
                            return fn_to_ln_dir_imp_h;
                        }
                    else // if (trans_type == LNTOFN)
                        {
                            if (ln_to_fn_dir_imp_h == 0)
                                ln_to_fn_dir_imp_h = (path_hash_entry **)calloc(sizeof(path_hash_entry *), MAX_PATH_HASH);
                            return ln_to_fn_dir_imp_h;
                        }
                }
        }
}

path_hash_entry **projMap::get_hash_head(char file_tp, char chosen_rule, char trans_type)
{
    Initialize(projMap::get_hash_head);

    if (file_tp == FILE_TYPE)
        {
            if (chosen_rule == LOGIC_NAME_RULE)
                {
                    if (trans_type == FNTOLN)
                        return fn_to_ln_file_h;
                    else // if (trans_type == LNTOFN)
                        return ln_to_fn_file_h;
                }
            else // if (chosen_rule == PROJECT_NAME_RULE)
                {
                    if (trans_type == FNTOLN)
                        return fn_to_ln_file_imp_h;
                    else // if (trans_type == LNTOFN)
                        return ln_to_fn_file_imp_h;
                }
        }
    else // if (file_tp == DIR_TYPE)
        {
            if (chosen_rule == LOGIC_NAME_RULE)
                {
                    if (trans_type == FNTOLN)
                        return fn_to_ln_dir_h;
                    else // if (trans_type == LNTOFN)
                        return ln_to_fn_dir_h;
                }
            else // if (chosen_rule == PROJECT_NAME_RULE)
                {
                    if (trans_type == FNTOLN)
                        return fn_to_ln_dir_imp_h;
                    else // if (trans_type == LNTOFN)
                        return ln_to_fn_dir_imp_h;
                }
        }
}

// file_type:   DIR_TYPE | FILE_TYPE
// chosen_rule: LOGIC_NAME_RULE | PROJECT_NAME_RULE
// trans_type:  FNTOLN | LNTOFN
void projMap::insert_hash(char const *fn, char const *proj_n,
                          char const *s, char file_tp, char chosen_rule,
                          char trans_type)
{
    Initialize(projMap::insert_hash);

    if (fn == 0)
	return;

    path_hash_entry **hh = create_hash_tbl(file_tp, chosen_rule, trans_type);
    int idx = path_hash(fn, MAX_PATH_HASH);
    path_hash_entry *cur = hh[idx];

    path_hash_entry *last = cur;
    while(cur)
        {
            if (strcmp(cur->get_fn(),fn) == 0)
                {
                    if (proj_name_match(cur->get_proj_n(), proj_n))
                        return;
                    if (proj_name_match(proj_n,cur->get_proj_n()))
                        {
                            cur->set_proj_n(proj_n);
                            return;
                        }
                }

            last = cur;
            cur = cur->get_next();
        }
    path_hash_entry *nn = db_new(path_hash_entry, (fn, s, proj_n));
    if (last)
        last->set_next(nn);
    else
        hh[idx] = nn;
}

bool projMap::contain_leaf()
{
    Initialize(projMap::contain_leaf);

    projMap *p = this;
    while(1)
        {
            p = p->get_prev_project();
            if (!p)
                break;
            if (p->is_leaf() && !p->is_exclude_proj() && p->para_rule_type==0)
                return true;
        }
    p = this;
    while(1)
        {
            p = p->get_next_project();
            if (!p)
                break;
            if (p->is_leaf() && !p->is_exclude_proj() && p->para_rule_type==0)
                return true;
        }
    return false;
}

// If any module exists on the same level or above this project node of "name"
bool check_proj_name_for_build(projNode *pn, char const *proj_name)
{
    Initialize(check_proj_name_for_build);

    while(pn)
        {
            projNode *par = pn->find_parent();
            if (!par)
                break;
            pn = par;
        }

    genString phy;
    pn->ln_to_fn_imp(proj_name, phy, DIR_TYPE, 0, 1);
    if (!phy.length())
        return false;

    pn = projNode::find_project(proj_name);
    Assert(pn);

    projMap *pm =  get_projmap_for_projnode(pn);
    Assert(pm);

    while (pm)
        {
            if (pm->contain_leaf())
                return false;
            pm = pm->get_parent_project();
        }
    return true;
}

void projMap::set_pdf_tree_arr_indexes (int root_idx, int & leaf_idx) 
{ 
    Initialize(projMap::set_pdf_tree_arr_indexes__int_int&);
    root_pdf_tree_arr_index = root_idx;
    leaf_pdf_tree_arr_index = root_idx + leaf_idx;

    projMap *cur = checked_cast(projMap,get_first()) ;
    if (cur == 0) {
        leaf_idx ++ ;
        return;
    }

    for ( ; cur ; cur = checked_cast(projMap,cur->get_next())) {
        cur->set_pdf_tree_arr_indexes(root_idx, leaf_idx);
    }
}

//==========================================
char const *get_env_var_chars();
static void locate_var_end(char *start,char ** end)
//char *start;
//char **end;
{
    char *s = start;

    /* only alphabets, digits, and '_' are in the variables */
    while(*s) {
        if (isdigit(*s) || isalpha(*s) || *s == '_')
            s++;
        else {
            *end = s;
            return;
        }
    }
    *end = s;
    return;
}

static char *combine_path(char ** ret, char const *path)
//char **ret;
//char *path;
{
    char *new_ret;
    int  len;

    len = strlen(*ret) + strlen(path) + 1;
    new_ret = (char *)calloc(sizeof(char), len);
    strcpy(new_ret,*ret);
    strcat(new_ret,path);
    free(*ret);

    return new_ret;
}

extern void prj_report_env_var (char const *var, char const *val);

extern "C" char *path_sys_vars_translate(char const *path)
{
    char *p;
    char *ret;
    char const *env_var_chars = get_env_var_chars();

    ret = strdup("");

    if (path == 0)
        return ret;

    char *pathDup = strdup(path);
    char *ppath = pathDup;

    while(1) {
        char *start, start_chr;
        char *end;

        char *p = ppath;
        start = 0;
        while(*p) {
            if (*p == '\\') {
                p++;
                if (*p)
                    p++;
                else
                    break;
            }
            else if ( *(p+1) && *(p+1) != '\\' && *(p+1) != '/' && strchr(env_var_chars, *p) ) {
                start = p;
                break;
            } else
                p++;
        }

        if (start) {
            locate_var_end(start+1,&end);
            start_chr = *start;
            *start = '\0';
        }

        /* for constant string */
        if (strlen(ppath))
            ret = combine_path(&ret,ppath);

        /* for variables. */
        if (start) {
            char saved_c;
            char *p;
            char const *env;

            *start = start_chr;
            saved_c = *end;
            *end = '\0';

            env = OSapi_getenv(start+1);
            if ( env && *env ) {
                create_path_2DIS(env, &p);
                prj_report_env_var (start+1, p);
            } else
                p=0;

            if (p) {
                ret = combine_path(&ret,p);
                free( p );
            } else {
                msg("The environment variable $1 is undefined.") << start+1 << eom;
                ret = combine_path(&ret,start+1);
            }

            *end = saved_c;
            ppath = end;
            if (*ppath == '\0')
                break;
#ifdef _WIN32
            if (start_chr == '%' && *ppath == '%' && *(++ppath) == '\0' )
                break;
#endif
        } else
            break;
    }

    free(pathDup);
    return(ret);
}
