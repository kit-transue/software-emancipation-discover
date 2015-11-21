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
#include <msg.h>
#include <machdep.h>
#include <customize.h>
#include <psetmem.h>

#ifndef _objOper_h
#include <objOper.h>
#endif

#ifndef _path1_h
#include <path1.h>
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

#ifndef __pdf_selector__h
#include <pdf_selector.h>
#endif

#ifndef _messages_h
#include <messages.h>
#endif

static char tran_flag;
extern "C" void save_tran_flag(char tran_type) { tran_flag = tran_type; }
int get_tran_flag () { return tran_flag; }
extern "C" int bget_path_parse_line_num();
extern projLevel *get_cur_parser_p ();

char RELATION_ROOT[] = "/ParaSET-Relations/";
char SUBSYSTEM_ROOT[] = "/ParaSET-Subsystems/";

static bool check_exclude_proj(char const *proj_name, char const *logic_name, char direction)
{
    Initialize(check_exclude_proj);

    if (!proj_name || strncmp(proj_name,"\"\"",2))
        return FALSE;

    if (!logic_name || strncmp(logic_name,"\"\"",2))
        return FALSE;

    if (direction != REAL_RIGHT_WAY)
        return FALSE;

    return TRUE;
}

int obtain_path_parse_line_num(char const *proj_name, char const *phy_name, char const *logic_name, char new_lineno_flag)
{
  Initialize(obtain_path_parse_line_num);
  
  static int lineno = 1;
  
  if (new_lineno_flag)
    if (proj_name && strlen(proj_name)
	|| phy_name && strlen(phy_name)
	|| logic_name && strlen(logic_name))
      lineno = get_path_parse_line_num();

  return lineno;
}

// if map_type == MEMBER, proj_name is the definition of member
projMap::projMap(char map_type, char sub_proj_type, char direction_flag, char const *proj_name, char const *phy_name,
                 char const *logic_name,
                 char child_type, char new_lineno_flag) :
                 flag(map_type), sub_proj_flag(sub_proj_type), direction(direction_flag), proj_rule(0),
                 member_filter(0), leading_slash_in_child_phy_path(0), line_num(0), view_flag(1),
                 child_real_right_way(0), map_phy_path(0),
                 fn_to_ln_file_h(0), fn_to_ln_dir_h(0), ln_to_fn_file_h(0), ln_to_fn_dir_h(0),
                 fn_to_ln_file_imp_h(0), fn_to_ln_dir_imp_h(0), ln_to_fn_file_imp_h(0), ln_to_fn_dir_imp_h(0),
                 this_is_target(0), para_rule_type(0), read_only_flag(NO_RW_FLAG), num_created_path(0),
                 pth(0), writepipe(0), readpipe(0), root_node(0), projNode_filter(0), 
                 root_pdf_tree_arr_index(-1), leaf_pdf_tree_arr_index(-1), 
                 pmod_type(PMOD_UNKNOWN), included_type(FILE_UNKNOWN)
// the constructor for projMap
//
// map_type: MEMBER (file), PROJECT (directory), EXCLUDE_PROJECT
// sub_proj_type: = or :    <== "=" is not supported any more
// direction_flag: <->, ->, <-, =>
// proj_name: project name
// phy_name: physical path
// logic_name: logic name
// child_type: for implicit project use which is not supported any more
// new_lineno_flag: if set, proj_name, phy_name, and logic_name are appended with the line number
{
    Initialize(projMap::projMap);

    line_num = obtain_path_parse_line_num(proj_name, phy_name, logic_name, new_lineno_flag);

    if (strlen(phy_name) > MAXPATHLEN-1)
    {
        create_one_path_parser_error(PATH_PARSE_PHY_PATH_TOO_LONG_ERR, 0, 0, line_num, 0);
        return;
    }

#ifdef _WIN32
    convert_back_slashes(logic_name);
#endif

    if ( direction_flag != REAL_RIGHT_WAY )
    {
      if (strstr(phy_name,"||"))
      {
        create_one_path_parser_error(PATH_PARSE_TWO_PIPES_ERR, 0, 0, line_num, 0);
        return;
      }

      if (proj_name  && contain_target_char(proj_name,'|', 1) ||
          logic_name && contain_target_char(logic_name,'|', 1))
          create_one_path_parser_error(PATH_PARSE_PIPE_IN_IDX_NAME_ERR, 0, 0, line_num, 0);
    }

    static int implicit_proj_wrn = 0;
    if (sub_proj_flag == IMPLICIT_DIR_PROJ && !implicit_proj_wrn)
    {
        implicit_proj_wrn = 1;
        create_one_path_parser_error(PATH_PARSE_NO_IMPLICIT_PROJ_SUPPORT_WRN, 0, 0, line_num, 0);
    }

    patherr_projmap_parse_line_num (line_num);

    char idx_const_string = 1;

    int exclude_phy_name = (*phy_name == '^');

    char *dis_phy_name = new char [ strlen(phy_name) +2];
    strcpy( dis_phy_name, phy_name );
    if ( exclude_phy_name )
        trn_path_2DIS ( dis_phy_name+1 );
    else
        trn_path_2DIS ( dis_phy_name );

    if (check_exclude_proj(proj_name,logic_name,direction) || exclude_phy_name)
    {
        map_type = EXCLUDE_PROJECT;
        flag = map_type;
        idx_const_string = 0;
        if (exclude_phy_name)
        {
            // dis_phy_name++; move down to ::wkPtnRule
            direction = REAL_RIGHT_WAY;
            direction_flag = REAL_RIGHT_WAY;
        }
    }

    int exclude_type = NON_EXCLUDE;
    if (*dis_phy_name == '^')
        exclude_type = EXCLUDE_DIR | EXCLUDE_FILE;
    else if (flag == EXCLUDE_PROJECT)
    {
        if (sub_proj_flag == IMPLICIT_DIR_PROJ)
            exclude_type = EXCLUDE_DIR;
        else
            exclude_type = EXCLUDE_FILE;
    }

    extern projLevel* get_cur_parser_p();
    projLevel *up_parser_p, *cur_p = get_cur_parser_p();
    
    if (cur_p && (up_parser_p = cur_p->get_parent()))
    {
        projMap *par = up_parser_p->get_map();
        if (par->get_sub_proj_flag() == IMPLICIT_DIR_PROJ)
            exclude_type |= EXCLUDE_PAR_IMP;
    }

    if (proj_name && strncmp(proj_name,"__",2) == 0)
        exclude_type |= EXCLUDE_RULE;

    if (proj_name == 0 || strlen(proj_name) && contain_target_char(proj_name,'(', 0))
        idx_const_string = 0;
    if (logic_name == 0 || strlen(logic_name) && contain_target_char(logic_name, '(', 0))
        idx_const_string = 0;

    int i;
    for (i=0; i<multi_selectors.size(); i++)
    {
        selector_entry *p = multi_selectors[i];
        char first_wrn = 0;
        if (i == 0 && p->warning_already)
            first_wrn = 1;
        if (p->selector_type == PURE_FILE_TYPE && p->read_only_flag != NO_RW_FLAG)
        {
            char once = 0;
            if (read_only_flag == R_FLAG && p->read_only_flag == W_FLAG ||
                read_only_flag == W_FLAG && p->read_only_flag == R_FLAG)
            {
                once = 1;
                if (!first_wrn)
                    create_one_path_parser_error(PATH_PARSE_READ_ONLY_SUPESEDE_WRITE_ONLY_WRN, 0, 0, 0, 0);
            }
            if (read_only_flag == NO_RW_FLAG|| read_only_flag == W_FLAG && p->read_only_flag == R_FLAG)
                read_only_flag = p->read_only_flag;
            if (once)
                break;
        }
    }

    char set_file_type_flag = 0;
    file_type = 0;
    for (i=0; i<multi_selectors.size(); i++)
    {
        selector_entry *p = multi_selectors[i];
	if ( p->selector_type == PMOD_TYPE) {
            pmod_type = pmod_type | PMOD_PDF;
            pmod_projs_insert (this);
        } else if ( p->selector_type == INCLUDED_TYPE)
            included_type = 1;
        else if (p->selector_type == PURE_FILE_TYPE && p->parser_file_type)
        {
            set_file_type_flag = 1;
            file_type |= p->parser_file_type;
        }
        else
            break; // only interested in the ones before LINK_MAP_FILE_TYPE and REALPATH_MAP_FILE_TYPE
    }

    if (!set_file_type_flag)
        file_type = DEFAULT_TYPE;
    else if (child_type != SINGLE_CHILD && child_type != BOTTOM_CHILD) // only for directories
    {
        int dir_file_type = 0;

        if ((file_type & FILE_TYPE) || (file_type & DIR_TYPE))
            dir_file_type |= DIR_TYPE;

        if ((file_type & LINK_FILE) || (file_type & LINK_DIR))
            dir_file_type |= LINK_DIR;

        file_type = dir_file_type;
    }

    multi_phy_rule.reset();

    char special_phy_rule = 0;
    char match_special_phy_rule = 0;
    if (child_type == SINGLE_CHILD || child_type == TOP_CHILD)
    {
        for (int i=0; i<multi_selectors.size(); i++)
        {
            selector_entry *p = multi_selectors[i];
            if (p->selector_type == LINK_MAP_FILE_TYPE     ||
                p->selector_type == REALPATH_MAP_FILE_TYPE ||
                p->selector_type == SEARCH_FILE_TYPE       ||
                p->selector_type == TCL_BOOLEAN_TYPE       ||
                p->selector_type == EQUAL_TYPE             ||
                p->selector_type == NOT_EQUAL_TYPE         ||
                p->selector_type == SCRIPT_TYPE            ||
                p->selector_type == FILTER_TYPE)
            {
                special_phy_rule = 1;
                if (p->selector_def && strchr(p->selector_def,'(') &&
                    p->selector_type != SCRIPT_TYPE &&
                    p->selector_type != FILTER_TYPE)
                {
                    match_special_phy_rule = 1;
                    break;
                }
            }
        }
    }

    // always the first one.
    // don't convert *.h to (*.h) here
    int bad_phy_rule = 0;
        bad_phy_rule = create_one_phy_rule(NORMAL_PHY_PATH, dis_phy_name, direction_flag, GARBAGE_FILE_TYPE,
                                           idx_const_string, exclude_type);
    if (special_phy_rule)
    {
        char found_special_phy_rule = 0;
        for (i=0; i<multi_selectors.size(); i++)
        {
            selector_entry *q = multi_selectors[i];
            if (q->selector_type == SCRIPT_TYPE)
            {
                found_special_phy_rule = 1;
                bad_phy_rule = create_one_phy_rule(MAP_SCRIPT_TYPE, q->selector_def, 0, GARBAGE_FILE_TYPE,
                                                   1, exclude_type);
            }
            else if (q->selector_type == FILTER_TYPE)
            {
                found_special_phy_rule = 1;
                bad_phy_rule = create_one_phy_rule(MAP_FILTER_TYPE, q->selector_def, 0, GARBAGE_FILE_TYPE,
                                                   1, exclude_type);
            }
            else if (q->selector_type == PURE_FILE_TYPE)
            {
                if (found_special_phy_rule && q->parser_file_type)
                    bad_phy_rule = create_one_phy_rule(MAP_FILE_TYPE, 0, 0, q->parser_file_type, 1,
                                                       exclude_type);
            }
            else
            {
                found_special_phy_rule = 1;
                if (q->selector_type == LINK_MAP_FILE_TYPE)
                    bad_phy_rule = create_one_phy_rule(LINK_MAP_PHY_PATH, q->selector_def, direction_flag,
                                                       GARBAGE_FILE_TYPE, 1, exclude_type);
                else if (q->selector_type == REALPATH_MAP_FILE_TYPE)
                    bad_phy_rule = create_one_phy_rule(REALPATH_MAP_PHY_PATH, q->selector_def, direction_flag,
                                                       GARBAGE_FILE_TYPE, 1, exclude_type);
                else if (q->selector_type == TCL_BOOLEAN_TYPE)
                    bad_phy_rule = create_one_phy_rule(TCL_BOOLEAN_EVAL, q->selector_def, direction_flag,
                                                       GARBAGE_FILE_TYPE, 1, exclude_type);
                else if (q->selector_type == EQUAL_TYPE)
                    bad_phy_rule = create_one_phy_rule(EQUAL_EVAL, q->selector_def, direction_flag,
                                                       GARBAGE_FILE_TYPE, 1, exclude_type);
                else if (q->selector_type == NOT_EQUAL_TYPE)
                    bad_phy_rule = create_one_phy_rule(NOT_EQUAL_EVAL, q->selector_def, direction_flag,
                                                       GARBAGE_FILE_TYPE, 1, exclude_type);
                else if (q->selector_type == SEARCH_FILE_TYPE)
                    bad_phy_rule = create_one_phy_rule(SEARCH_PHY_PATH, q->selector_def, direction_flag,
                                                       GARBAGE_FILE_TYPE, 1, exclude_type);
            }
        }
    }

    if (!bad_phy_rule)
    {
        // convert *.h to (*.h) here
        if (!selector_matched_pattern() && !idx_const_string && !get_phy_rule(0)->get_max_ptn_count())
            get_phy_rule(0)->set_ptn(exclude_type, dis_phy_name, direction_flag, NORMAL_PHY_PATH, 0, 1, 0);

        if (logic_name)
            logic_rule = db_new(wkIdxRule,(logic_name, direction_flag));
        else
        {
            logic_rule = db_new(wkIdxRule,(0, direction_flag));
            genString s = 0;
            get_logic_idx_default(s);
            if (s.length())
                logic_rule->set_idx(s, direction_flag);
        }

        if (proj_name)
            proj_rule = db_new(wkIdxRule,(proj_name, TWO_WAY));
        else
        {
            proj_rule = db_new(wkIdxRule,(0, TWO_WAY));
            genString s = 0;
            get_proj_idx_default(s);
            if (s.length())
                proj_rule->set_idx(s, TWO_WAY);
        }

        fill_ptn_strs();
        replace_phy_idx();
        proj_rule->create_idx_ptn();
        logic_rule->create_idx_ptn();

        proj_rule->set_multi_match_in_idxrule();
        logic_rule->set_multi_match_in_idxrule();
    }
    if (child_type == SINGLE_CHILD || child_type == BOTTOM_CHILD) // The last one, destroy the file attributes
        init_multi_selectors();

    genString r_path;
    project_convert_filename(dis_phy_name, r_path);

    save_tran_flag(TWO_WAY); // default
    patherr_projmap_parse_line_num (-1);

    delete [] dis_phy_name;
}

bool projMap::is_read_only()
{
    Initialize(projMap::is_read_only);

    if (read_only_flag == R_FLAG)
        return TRUE;
    return FALSE;
}

bool projMap::is_writable()
{
    Initialize(projMap::is_writable);

    if (read_only_flag == W_FLAG)
        return TRUE;
    return FALSE;
}

void projMap::reset_ret_dir()
{
    Initialize(projMap::reset_ret_dir);

    for (int i=0;i<ret_dir.size();i++)
        psetfree((void *)ret_dir[i]->dir_name);

    ret_dir.reset();
}

projMap::~projMap()
// delete one mapping tree
//
// root_node not zero if this is a root of mapping tree
{
    Initialize(projMap::~projMap);

    if (proj_rule)
        delete proj_rule;

    if (logic_rule)
        delete logic_rule;

    int i;

    for (i=0;i<multi_phy_rule.size();i++)
    {
        if(multi_phy_rule[i]->phy_rule)
            delete multi_phy_rule[i]->phy_rule;
    }

    if (member_filter)
        delete member_filter;

    if (projNode_filter)
        delete projNode_filter;

    for (i=0;i<ret_dir.size();i++)
        psetfree((void *)ret_dir[i]->dir_name);

    projMap *child = get_child_project();
    if (child)
        delete child;

    // When this one is called, the "next" of the root should be zero.
    // It is not zero now.
    // Use the "parent" to see if it is root.
    if (fn_to_ln_file_h)
        delete fn_to_ln_file_h;

    if (fn_to_ln_dir_h)
        delete fn_to_ln_dir_h;

    if (ln_to_fn_file_h)
        delete ln_to_fn_file_h;

    if (ln_to_fn_dir_h)
        delete ln_to_fn_dir_h;

    if (fn_to_ln_file_imp_h)
        delete fn_to_ln_file_imp_h;

    if (fn_to_ln_dir_imp_h)
        delete fn_to_ln_dir_imp_h;

    if (ln_to_fn_file_imp_h)
        delete ln_to_fn_file_imp_h;

    if (ln_to_fn_dir_imp_h)
        delete ln_to_fn_dir_imp_h;

    if (root_node)
        return;

    projMap *next = get_next_project();
    while (next)
    {
        projMap *cur = next;
        next = cur->get_next_project();
        cur->remove_from_tree();
        delete cur;
    }
}

void projMap::set_map_phy_path(char const *m)
{
    //Initialize(projMap::set_map_phy_path);
    if (!child_real_right_way)
        return;

    map_phy_path = m;
}

projMap *projMap::get_prev_project() const
{
    Initialize(projMap::get_prev_project);

    return checked_cast(projMap, get_prev());
}

projMap *projMap::get_next_project() const
{
    Initialize(projMap::get_next_project);

    return checked_cast(projMap, get_next());
}

projMap *projMap::get_parent_project() const
{
    Initialize(projMap::get_parent_project);

    return checked_cast(projMap, get_parent());
}

void projMap::set_next_project(projMap *p)
{
    Initialize(projMap::set_next_project);

    put_after(checked_cast(objTree,p));
}

void projMap::set_child_project(projMap *p)
{
    Initialize(projMap::set_child_project);

    put_first(checked_cast(objTree,p));
}

projMap* projMap::get_child_project() const
{
    Initialize(projMap::get_child_project);

    return(checked_cast(projMap, get_first()));
}

int projMap::get_this_is_target()
{
    Initialize(projMap::get_this_is_target);

    return this_is_target;
}

#define NIL_NAME "(nil)"
extern void gen_print_indent(ostream&, int);
extern void gen_print_cr(ostream&, int);

void projMap::print(ostream& st, int level) const 
{
    char const *map_type_name = NIL_NAME;
    switch (flag) {
    case MEMBER:          map_type_name = "MEMBER";          break;
    case PROJECT:         map_type_name = "PROJECT";         break;
    case DUP_PROJECT:     map_type_name = "DUP_PROJECT";     break;
    case CREATED_MEMBER:  map_type_name = "CREATED_MEMBER";  break;
    case CREATED_PROJECT: map_type_name = "CREATED_PROJECT"; break;
    case EXCLUDE_PROJECT: map_type_name = "EXCLUDE_PROJECT"; break;
    default:              map_type_name = NIL_NAME;          break;
    };

    char const *sub_proj_type_name = NIL_NAME;
    switch (sub_proj_flag) {
    case NO_IMPLICIT_DIR_PROJ: sub_proj_type_name = "NO_IMPLICIT_DIR_PROJ"; break;
    case IMPLICIT_DIR_PROJ:    sub_proj_type_name = "IMPLICIT_DIR_PROJ"   ; break;
    default:                   sub_proj_type_name = NIL_NAME              ; break;
    };

    char const *arrow_way_name = NIL_NAME;
    switch (direction) {
    case LEFT_WAY:        arrow_way_name = "LEFT_WAY";       break;
    case RIGHT_WAY:       arrow_way_name = "RIGHT_WAY";      break;
    case TWO_WAY:         arrow_way_name = "TWO_WAY";        break;
    case REAL_RIGHT_WAY:  arrow_way_name = "REAL_RIGHT_WAY"; break;
    default:              arrow_way_name = NIL_NAME;         break;
    };

    char const *rw_flag_name = NIL_NAME;
    switch (read_only_flag) {
    case NO_RW_FLAG: rw_flag_name = "NO_RW_FLAG"; break;
    case R_FLAG:     rw_flag_name = "R_FLAG"    ; break;
    case W_FLAG:     rw_flag_name = "W_FLAG"    ; break;
    default:         rw_flag_name = NIL_NAME    ; break;
    };
    
    char const *file_type_name = NIL_NAME;
    switch (file_type) {
    case FILE_TYPE        : file_type_name = "FILE_TYPE"        ; break;
    case DIR_TYPE         : file_type_name = "DIR_TYPE"         ; break;
    case LINK_FILE        : file_type_name = "LINK_FILE"        ; break;
    case LINK_DIR         : file_type_name = "LINK_DIR"         ; break;
    case DEFAULT_TYPE     : file_type_name = "DEFAULT_TYPE"     ; break;
    case GARBAGE_FILE_TYPE: file_type_name = "GARBAGE_FILE_TYPE"; break;
    case PARTIAL_DIR_TYPE : file_type_name = "PARTIAL_DIR_TYPE" ; break;
    case END_FILE_TYPE    : file_type_name = "END_FILE_TYPE"    ; break;
    default               : file_type_name = NIL_NAME           ; break;
    };

    char const *pmod_type_name = NIL_NAME;
    switch (pmod_type) {
    case PMOD_UNKNOWN   : pmod_type_name = "PMOD_UNKNOWN"   ; break;
    case PMOD_PDF       : pmod_type_name = "PMOD_PDF"       ; break;
    case PMOD_MODELBUILD: pmod_type_name = "PMOD_MODELBUILD"; break;
    case PMOD_HOME      : pmod_type_name = "PMOD_HOME"      ; break;
    case PMOD_NO        : pmod_type_name = "PMOD_NO"        ; break;
    default             : pmod_type_name = NIL_NAME         ; break;
    };

    char const *included_type_name = (included_type) ? "yes" : "no";

    gen_print_indent(st, level);
    st << map_type_name      << "; "
       << sub_proj_type_name << "; "
       << arrow_way_name     << "; "
       << rw_flag_name       << "; "
       << file_type_name     << "; "
       << pmod_type_name     << "; "
       << included_type_name << "; "
       << get_org_project_name() ;
}
