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
//  file path.h
//    define classes for pdf definitions

#ifndef _path_h
#define _path_h

#define FALSE 0
#define TRUE 1

#include <cLibraryFunctions.h>

#ifndef _genError_h
#include <genError.h>
#endif

#ifndef _genString_h
#include <genString.h>
#endif

#ifndef Wild__h
#include <genWild.h>
#endif

#ifndef _proj_h
#include <proj.h>
#endif

#ifndef _pathdef_h
#include <pathdef.h>
#endif

#ifndef _proj_hash_h
#include <proj_hash.h>
#endif

#ifndef _pdf_tree_h
#include <pdf_tree.h>
#endif

// The following enum is for db_new(projMap,(...))
// Most cases are SINGLE_CHILD
//
// The other three is for the created children of an implicit_sub_dirs project
// * / * / *.c
// The first  * is TOP_CHILD
// The second * is MIDDLE_CHILD
// *.C is the BOTTOM_CHILD
//
// [[ -> ]] will be defined in the TOP_CHILD. (SINGLE_CHILD is TOP_CHILD)
//
// [[ dDfF ]] will be go to the BOTTOM_CHILD. (SINGLE_CHILD is BOTTOM_CHILD)
// [[ dD ]] will be assigned for the created child except for BOTTOM_CHILD, because they are directories.
//
// In the mapping, if it is faild in [[ -> ]] mapping and the child is a created project
//    don't go to the child.
// Bascially, the created projects for [[ -> ]] is for the file browsing purpose.
enum
{
    TOP_CHILD,
    MIDDLE_CHILD,
    BOTTOM_CHILD,
    SINGLE_CHILD
};

#define DIR_SUB    1
#define MODULE_SUB 2

#define REJECT_BY_PROJ_NAME "Rejected by sub_proj_name"

class projFilter;
class projMap;
class WildPair;

#define RULE_SEPARATOR "!"

enum
{
    LNTOFN,
    FNTOLN
};

// The next enum is used by the rule_type in the data structure phy_rule_entry.
enum
{
    NORMAL_PHY_PATH,
    LINK_MAP_PHY_PATH,
    REALPATH_MAP_PHY_PATH,
    SEARCH_PHY_PATH,
    MAP_FILE_TYPE,
    MAP_SCRIPT_TYPE,
    MAP_FILTER_TYPE,
    TCL_BOOLEAN_EVAL,
    EQUAL_EVAL,
    NOT_EQUAL_EVAL,
    END_PHY_PATH
};

// MAKE_RULE           /%/.make
// MAKEMF_RULE         /%/.makemf
// PMODDIR_RULE        /%/.pmoddir
// PSET_RULE           /%/.pset
// OTHER_RULE          /%/.????

#define MAKE_RULE           1
#define MAKEMF_RULE         2
#define PMODDIR_RULE        4
#define PSET_RULE           8
#define OTHER_RULE         64

// The next will be used in the projFilter::file_name_match
// If it is check a file, search the last '/' and check the file name.
// If it is a directory, search the first '/' and check the first directory.
#define FILE_TYPE           1
#define DIR_TYPE            2
#define LINK_FILE           4
#define LINK_DIR            8
#define DEFAULT_TYPE       16
#define GARBAGE_FILE_TYPE  32
#define PARTIAL_DIR_TYPE   64
#define END_FILE_TYPE     128
// The next enum will be used for projMap's rules
enum
{
    PROJECT_NAME_RULE,
    LOGIC_NAME_RULE
};

//
//  type = IDX_CONST_STRING
//       The "str" in the union will store a constant path.
//  type = IDX_MATCH_INDEX
//       They are (0), (1), (2), ...
//       The "idx" in the union will store the index from which
//       the corresponding path will be obtained from the pattern portion.
//
//   EX: ~/as/(1)/src/(2)
//
//   The first node:
//        type = IDX_CONST_STRING
//        wk_str = "~/as/"
//
//   The second node:
//        type = IDX_MATCH_INDEX
//        wk_idx = 1
//
//   The third node:
//        type = IDX_CONST_STRING
//        wk_str = "/src/"
//
//   The fourth node:
//        type = IDX_MATCH_INDEX
//        wk_idx = 2
//

enum {
    IDX_CONST_STRING,
    IDX_MATCH_INDEX
};

class wkIdxDef
{
  public:
    wkIdxDef(wkIdxDef *s);
    wkIdxDef(char t, char const *s);
    wkIdxDef(char t, int index);

    ~wkIdxDef();

    char       get_type();
    char const *get_str();
    int        get_index();
    wkIdxDef   *get_next() const;

    void       set_index(int idx);

    void       set_next(wkIdxDef *n);

  private:
    char type;
    genString wk_str;
    int       wk_idx;

    wkIdxDef *next;
};

inline char wkIdxDef::get_type()
{
    //Initialize(wkIdxDef::get_type);
    return type;
}

inline char const *wkIdxDef::get_str()
{
    //Initialize(wkIdxDef::get_str);
    return (char const *)wk_str.str();
}

inline int wkIdxDef::get_index()
{
    //Initialize(wkIdxDef::get_index);
    return wk_idx;
}

inline wkIdxDef *wkIdxDef::get_next() const
{
    //Initialize(wkIdxDef::get_next);
    return next;
}

inline void wkIdxDef::set_index(int idx)
{
    //Initialize(wkIdxDef::set_index);
    wk_idx = idx;
}

inline void wkIdxDef::set_next(wkIdxDef *n)
{
    //Initialize(wkIdxDef::set_next);
    next = n;
}

//
//  type = PTN_CONST_STRING
//       The "str" will store a constant path.
//  type = PTN_MATCH_MAPPING
//       It is (*) OR
//       it is (*.C), (*.h), (*.[Cc]), ...
//       it is a constant string.
//
//  PTN_IDX is used for file selectors only
//  (*.h) [[ -> ../src/*/include/(1)

enum {
    PTN_CONST_STRING,
    PTN_MATCH_MAPPING,
    PTN_IDX
};

class wkPtnDef
{
  public:
    wkPtnDef(wkPtnDef *s);
    wkPtnDef(char t = PTN_CONST_STRING, char const *s = 0, int idx = -1);

    ~wkPtnDef();

    char       get_type();
    char const *get_str();
    char const *get_str_by_idx(int idx);
    projFilter *get_filter() const;
    wkPtnDef   *get_next()   const;
    int        get_idx();

    void set_next(wkPtnDef *n);

  private:
    char      type;
    genString wk_str;
    int idx;

    // PTN_MATCH_MAPPING
    // The next will replace str
    projFilter *ptn_pf;

    wkPtnDef *next;
};

inline char wkPtnDef::get_type()
{
    //Initialize(wkPtnDef::get_type);
    return type;
}

inline char const *wkPtnDef::get_str()
{
    //Initialize(wkPtnDef::get_str);
    return (char const *)wk_str.str();
}

inline projFilter *wkPtnDef::get_filter() const
{
    //Initialize(wkPtnDef::get_filter);
    return ptn_pf;
}

inline int wkPtnDef::get_idx()
{
    //Initialize(wkPtnDef::get_idx);
    return idx;
}

inline wkPtnDef *wkPtnDef::get_next() const
{
    //Initialize(wkPtnDef::get_next);
   return next;
}

inline void wkPtnDef::set_next(wkPtnDef *n)
{
    //Initialize(wkPtnDef::set_next);
    next = n;
}

class wkPtnRule
// definition for rule for a physical file path
// (internal storage for part of pdf information)
{
  public:
    wkPtnRule(wkPtnRule *s);
    wkPtnRule(int exclude_type, char const *ptn = 0, char direction_flag = TWO_WAY,
              char rule_type = NORMAL_PHY_PATH,
              char idx_defined_flag = 0, int link_phy_rule = 0);
    ~wkPtnRule();

    char const *get_org_ptn()   const;
    char const *get_ptn_def()   const;
    wkPtnDef  *get_ptn_strs()  const;

    char const *get_exclude_file_ptn_def()   const;
    char const *get_exclude_dir_ptn_def()   const;

    int       get_max_ptn_count();
    int       count_const_two_star();

    bool      ptn_is_defined();
    void      update_idx_ptn();

    void set_org_ptn(char const *s);
    void set_ptn_def(char const *s);
    void set_ptn_strs(wkPtnDef *l);

    int set_ptn(int exclude_type, char const *s, char direction_flag, char rule_type = NORMAL_PHY_PATH, char idx_defined_flag = 0,
                char origin_str_flag = 1, int link_phy_rule = 0);

    int failed();

    void set_multi_match_in_ptnrule(int exclude_type, int rule_type);
    WildPair *get_multi_match_in_ptnrule();
    void set_dir_multi_match_in_ptnrule(int par_imp_flag);
    WildPair *get_dir_multi_match_in_ptnrule();

    bool no_idx();
    void fill_idx(genString &s, char const *full_path);

  private:

    // ptn_def stores the defined string for pattern portion.
    genString org_ptn;
    genString ptn_def;
    // ptn_strs contains the analyzed result of pattern portion.
    wkPtnDef  *ptn_strs;
    int result;
    WildPair *multi_match_in_ptnrule;
    WildPair *dir_multi_match_in_ptnrule;
    genString exclude_file_ptn_def;
    genString exclude_dir_ptn_def;
};

inline char const *wkPtnRule::get_org_ptn() const
{
    //Initialize(wkPtnRule::get_org_ptn);
    return org_ptn;
}

inline char const *wkPtnRule::get_ptn_def() const
{
    //Initialize(wkPtnRule::get_ptn_def);
    return ptn_def;
}

inline char const *wkPtnRule::get_exclude_dir_ptn_def() const
{
    //Initialize(wkPtnRule::get_exclude_dir_ptn_def);
    return exclude_dir_ptn_def;
}

inline char const *wkPtnRule::get_exclude_file_ptn_def() const
{
    //Initialize(wkPtnRule::get_exclude_file_ptn_def);
    return exclude_file_ptn_def;
}

inline wkPtnDef *wkPtnRule::get_ptn_strs() const
{
    //Initialize(wkPtnRule::get_ptn_strs);
    return ptn_strs;
}

inline bool wkPtnRule::ptn_is_defined()
{
    //Initialize(wkPtnRule::ptn_is_defined);

    return(ptn_def.length() ? TRUE : FALSE);
}

inline void wkPtnRule::set_org_ptn(char const *s)
{
    //Initialize(wkPtnRule::set_org_ptn);
    org_ptn = s;
}

inline void wkPtnRule::set_ptn_def(char const *s)
{
    //Initialize(wkPtnRule::set_ptn_def);
    ptn_def = s;
}

inline void wkPtnRule::set_ptn_strs(wkPtnDef *l)
{
    //Initialize(wkPtnRule::set_ptn_strs);
    ptn_strs = l;
}

class wkIdxRule
{
  public:
    wkIdxRule(wkIdxRule *s);
    wkIdxRule(char const *idx, char direction_flag);

    ~wkIdxRule();

    char const *get_org_idx()   const;
    char const *get_idx_def();
    wkIdxDef  *get_idx_strs()  const;

    int       get_max_idx_count();

    bool      idx_is_defined();
    bool      is_constant_rule();

    char const *get_ptn();
    void set_org_idx(char const *s);
    void set_idx_def(char const *s);
    void set_idx_strs(wkIdxDef *l);

    void set_idx(char const *s, char direction_flag);

    void create_idx_ptn();
    void set_multi_match_in_idxrule();
    WildPair *get_multi_match_in_idxrule();

  private:

    char idx_def_flag;
    // idx_def stores the defined string for pattern portion.
    genString org_idx;
    genString idx_def;
    // idx_strs contains the analyzed result of pattern portion.
    wkIdxDef  *idx_strs;
    genString ptn;
    WildPair *multi_match_in_idxrule;
};

inline char const *wkIdxRule::get_idx_def()
{
    //Initialize(wkIdxRule::get_idx_def);
    return (char const *)idx_def.str();
}

inline wkIdxDef *wkIdxRule::get_idx_strs() const
{
    //Initialize(wkIdxRule::get_idx_strs);
    return idx_strs;
}

inline bool wkIdxRule::idx_is_defined()
{
    //Initialize(wkIdxRule::idx_is_defined);

    return(idx_def_flag ? TRUE : FALSE);
}

inline void wkIdxRule::set_org_idx(char const *s)
{
    //Initialize(wkIdxRule::set_org_idx);
    org_idx = s;
}

inline void wkIdxRule::set_idx_def(char const *s)
{
    //Initialize(wkIdxRule::set_idx_def);
    idx_def = s;
}

inline void wkIdxRule::set_idx_strs(wkIdxDef *l)
{
    //Initialize(wkIdxRule::set_idx_strs);
    idx_strs = l;
}

// Project rule definition.

enum
{
    MATCH_THIS_ONE,
    MATCH_ANY
};

class projFilter
{
  public:
    projFilter(char const *s = 0);

    ~projFilter();

    char const *get_filter() const;
    projFilter *get_next()   const;

    void set_next(projFilter *n);

    bool empty_filter();

    bool file_name_match(char const *name, char dir_flag, char match_only_one);

  private:
    genString  str;           // contains the filter definition such as *.C *.h.

    WildPair *filter;

    projFilter *next;
};

inline char const *projFilter::get_filter() const
{
    //Initialize(projFilter::get_filter);
    return str;
}

inline projFilter *projFilter::get_next() const
{
    //Initialize(projFilter::get_next);
    return next;
}

inline void projFilter::set_next(projFilter *n)
{
    //Initialize(projFilter::set_next);
    next = n;
}

struct retDirPtr
{
    char const *dir_name;
};

genArr(retDirPtr);

class pathParserErrEntry
{
  public:
    pathParserErrEntry(int e = 0, char const *msg = 0);

    int        err_code() const;
    char const *err_msg() const;

  private:
    int       _err_code;
    genString _err_msg;
};

typedef pathParserErrEntry* pathParserErrEntryPtr;

genArr(pathParserErrEntryPtr);

inline pathParserErrEntry::pathParserErrEntry(int e, char const *msg) :
    _err_code(e), _err_msg(msg)
{
    //Initialize(pathParserErrEntry::pathParserErrEntry);
}

inline int pathParserErrEntry::err_code() const
{
    //Initialize(pathParserErrEntry::err_code);

    return _err_code;
}

inline char const *pathParserErrEntry::err_msg() const
{
    //Initialize(pathParserErrEntry::err_msg);

    return _err_msg;
}

// 0: It is a member.
// 1: It is a project.
// 2: It is a duplicated projected.
//
// (1) = $ROOT <-> /aset
// { */*/*.C }
//
// will be transferred to
// (1) = $ROOT <-> /aset     <== PROJECT
// {
//     (*)/                  <== CREATED_PROJECT
//     {
//         (*)/              <== CREATED_PROJECT
//         {
//             (*.C)         <== CREATED_MEMBER
//         }
//     }
// }
//
// (1) = $ROOT <-> /aset     <== PROJECT
// {
//     "" : a*.c => ""       <== EXCLUDE_PROJECT (view_flag == 0)
// }
enum
{
    MEMBER,
    PROJECT,
    DUP_PROJECT,
    CREATED_MEMBER,
    CREATED_PROJECT,
    EXCLUDE_PROJECT
};

// rule_type: NORMAL_PHY_PATH, LINK_MAP_PHY_PATH, REALPATH_MAP_PHY_PATH, or SEARCH_PHY_PATH will define phy_rule.
//          : MAP_FILE_TYPE will define file_type.
//
// The first one is always : NORMAL_PHY_PATH for rule_type
//
// is_phy_rule_defined() will return TRUE for NORMAL_PHY_PATH, LINK_MAP_PHY_PATH,
//                                            REALPATH_MAP_PHY_PATH, or SEARCH_PHY_PATH
//                       will return FALSE for MAP_FILE_TYPE
typedef struct
{
    char      rule_type;
    int       file_type;
    wkPtnRule *phy_rule;
} phy_rule_entry;

genArr(phy_rule_entry);

#define NON_EXCLUDE      0
#define EXCLUDE_DIR      1
#define EXCLUDE_FILE     2
#define EXCLUDE_PAR_IMP  4

#define EXCLUDE_RULE    32

class projMap : public appTree
{
  public:
    projMap(projMap *s);

    projMap(char map_type = PROJECT, char sub_proj_flag = NO_IMPLICIT_DIR_PROJ, char dir_flag = TWO_WAY,
            char const *proj_name = 0, char const *phy_name = 0, char const *logic_name = 0, char child_type = SINGLE_CHILD,
            char new_lineno_flag = 1);

    ~projMap();

    define_relational(projMap,appTree);

    char const *get_org_project_name() const;
    char const *get_dir_name()         const;
    char const *get_org_dir_name()     const;
    char const *get_org_logic_name()   const;
    void                fill_ptn_strs();
    void                replace_phy_idx();

    projFilter          *get_member()         const;

    projMap             *get_prev_project()   const;
    projMap             *get_next_project()   const;
    projMap             *get_parent_project() const;
    projMap             *get_child_project()  const;

    int                 get_exclude_type();

    wkIdxRule           *get_project_rule();
    wkIdxRule           *get_logic_rule();
    wkPtnRule           *get_phy_rule(int idx);
    int                 get_phy_file_type(int idx);

    int                 get_view_flag();
    void                set_view_flag(int v);

    char                get_phy_rule_type(int idx);
    int                 get_phy_match_count();

    int                 get_line_num();
    void                set_line_num(int n);

    char const *get_map_phy_path();
    void                set_map_phy_path(char const *m);

    bool                is_child_real_right_way();
    void                set_child_real_right_way(char c);

    void                set_leading_slash_in_child_phy_path();

    void                search_modules_by_name(projNode *proj, 
				char const *proj_name, symbolArr &s);
    void                search_modules_by_name_recursive(projNode *proj, 
				char const *logic_name, 
				char const *full_proj_name,
				char const *proj_name, symbolArr &s1, 
				int deep_flag);

    void                get_physical_path(int idx, genString &s);

    genArrOf(retDirPtr) *get_ret_dir()        const;

    void update_successfull_rule(char const *rule, int len);

    void set_proj_flag(char f);

    void set_next_project(projMap *p);
    void set_child_project(projMap *p);

    int  is_project();
    int  is_member();
    int  is_leaf();
    int  is_imp_leaf();
    int  is_implicit_sub_dir_proj();
    int  is_exclude_proj();
    int  carot_proj();

    int  imp_without_real_filter(); // either no children or all children are excluding projects

    bool is_two_way();
    bool is_left_way();
    bool is_right_way();
    bool is_real_right_way();

    bool is_reg_file();
    bool is_reg_dir();
    bool is_link_file();
    bool is_link_dir();
    bool is_file();
    bool is_dir();
    bool is_link();
    bool is_leading_slash_in_child_phy_path();
    bool is_leading_slash_in_phy_path();

    bool contain_leaf();

    bool is_idx_contain_match_index();

    bool some_child_no_leading_slash();
    bool contain_sub_proj();

    bool exclude_path(char const *path, char file_type);
    bool exclude_dir_path(char const *path, char file_type);
    bool exclude_file_path(char const *path, char file_type);
    bool need_to_check_file_type();

    // a project name leaded by "__"
    bool is_invisible_project();

    // If this node does not require file type check, it returns TRUE.
    // If this node does require file type check,
    //    and the file pp matches the file type, it will return TRUE; Otherwise,
    //    it will return FALSE.
    bool match_file_type(char const *file, char file_type, int full_match_flag = 1, int *link_info = 0,
                         char realpath_flag = 0,
                         char ignore_this_node = 0, int xx_f = 0);

    void ln_to_fn_imp(char const *path, char const *sub_proj_name, genString &ret_str,int xx_f);

    void fn_to_ln_imp(char const *path, char const *sub_proj_name, char file_type, genString &ret_str);
    void fn_to_ln_imp(char const *path, char const *sub_proj_name, char file_type, genString &ret_str, int xx_f);
    void fn_to_ln_imp(char const *path, char const *sub_proj_name, char file_type, genString &ret_str, int xx_f,
                      int script_force_search);

    void ln_to_fn_imp(char const *path, char const *sub_proj_name, char file_type, genString &ret_str);
    void ln_to_fn_imp(char const *path, char const *sub_proj_name, char file_type, genString &ret_str, int xx_f);
    void ln_to_fn_imp(char const *path, char const *sub_proj_name, char file_type, genString &ret_str, int xx_f,
                      int script_force_search, projNode *pn = NULL);

    void fn_to_ln(char const *path, char const *sub_proj_name, genString &ret_str);
    void ln_to_fn(char const *path, char const *sub_proj_name, genString &ret_str,int xx_f);

    void fn_to_ln(char const *path, char const *sub_proj_name, char file_type, genString &ret_str);
    void fn_to_ln(char const *path, char const *sub_proj_name, char file_type, genString &ret_str, int xx_f,
                  int script_force_search);

    void fn_to_ln_internal(char const *path, char const *sub_proj_name, char file_type, genString &ret_str,
                           projNode **pr);
    void fn_to_ln_internal(char const *path, char const *sub_proj_name, char file_type, genString &ret_str,
                           int xx_f, projNode **pr);
    void fn_to_ln_internal(char const *path, char const *sub_proj_name, char file_type, genString &ret_str,
                           int xx_f, int script_force_search, projNode **pr);

    void ln_to_fn(char const *path, char const *sub_proj_name, char file_type, genString &ret_str, int xx_f);
    void ln_to_fn(char const *path, char const *sub_proj_name, char file_type, genString &ret_str, int xx_f,
                  int script_force_search, projNode *pn = NULL);

    void ln_to_proj_name(char const *ln, char const *prj_name, genString &proj_name, int file_type /*=1 -FILE, =0 -DIR*/ );

    // The next two are recursive
    // They are called by fn_to_ln and ln_to_fn correspondingly,
    //     when implicit rule is used.
    //
    // level is defined in go_thru_next_f2l and go_thru_next_l2f.
    // type will be one of LNTOFN and FNTOLN.
    //
    // complete_len is the length of path which is successfully translated yet.
    //
    // use_rule will be the concatenation of all the proj_names and member used so far.
    //
    // rule_type could be PROJECT_NAME_RULE for proj_rule or LOGIC_NAME_RULE for logic_rule.
    //
    // logic_rule_idx is the index of logic_rule in the array multi_logic_rule.
    //    [[ -> phy_path ]] might define more than one logic rule.
    //
    // If the symbol link fails for pnt_to_idx, realpath() will convert the physical path to the real path.
    // The first call to ptn_to_idx, realpath_flag is 0;
    // The first call to ptn_to_idx, realpath_flag is 1;
    void ptn_to_idx(char const *path, char const *org_path, char const *rem_path, genString &ret_path, char const *idx_name,
                    char file_type, int level, int complete_len, char const *use_rule, char rule_type, int *link_reject_flag,
                    char realpath_flag, int *no_pre_pend_flag, int *reject_by_exclude, projNode **pr);
    void idx_to_ptn( char const *path, char const *org_path, char const *rem_path, genString &ret_path, char const *idx_name,
                     char file_type, int level, int complete_len, char const *use_rule, char rule_type,
                     char const *phy_path, int *reject_by_exclude, int xx_f, projNode *pn);

    int create_one_phy_rule(char rule_type, char const *phy_name, char direction_flag, int file_type,
                            char idx_defined_flag, int exclude_type);
    void get_proj_idx_default(genString &s);
    void get_logic_idx_default(genString &s);

    void get_script_text(projNode *pn, genString &s);
    void get_filter_text(genString &s);
    bool is_script();
    bool is_filter();

    // type == 0: project
    // type == 1: logic
    // typ2 == 2: project and specially search for the exclude project
    void get_matched_name(char const *path, genString &ret_path, char type);

    void get_defined_name(genString &proj_name, genString &logic_name);

    void reset_ret_dir();

    static void reset_parser_err();
    static genArrOf(pathParserErrEntryPtr) *get_parse_err();
    static void add_one_parser_error(unsigned int err_code, char const *err_msg);

    static const pathParserErrEntry *entry(unsigned int);
    static unsigned int num_entries();

    static void init_tran_vars();
    static int  report_path_complete_full_len();
    static int  report_path_tran_error_code();
    static char const *report_path_tran_rule();

    static void set_path_complete_full_len(int cur_len);
    static void set_path_tran_error_code(char error_code);
    static void set_path_tran_rule(char const *rule);

    void set_file_type(char type);

    char get_flag();
    char get_sub_proj_flag();
    char get_direction();
    char get_file_type();

    void set_member(char const *m);

    bool selector_matched_pattern();
    bool selector_defined();
    bool need_verify_fn_to_ln();

    projMap *dup_map_tree();

    void verify_rtl(int &rtl_flag); // in verify_pdf.C

    void search_hash(char const *fn, genString &s, char file_type, char chosen_rule, char trans_type,
                     char const *sub_proj_name);
    void insert_hash(char const *fn, char const *proj_n,
                     char const *s, char file_type, char chosen_rule, char trans_type);
    void remove_hash(char const *fn, char file_type, char chosen_rule, char trans_type,
                     char const *sub_proj_name);
    path_hash_entry **create_hash_tbl(char file_type, char chosen_rule, char trans_type);
    path_hash_entry **get_hash_head(char file_type, char chosen_rule, char trans_type);

    void set_this_is_target(int v);
    bool check_this_is_target();
    int  get_this_is_target();

    void set_para_rule_type(int type);
    bool is_para_rule_type(int type);
    bool is_para_rule_type();

    bool is_read_only();
    bool is_writable();

    void set_num_created_path(int n);
    int  get_num_created_path();

    void set_pdf_tree_head(pdf_tree_head *p);
    pdf_tree_head *obtain_pdf_tree_head();

    path_hash_entry **copy_hash_tbl(projMap *cur, char file_type, char chosen_rule, char trans_type);
    void copy_phy_filters(projMap *s, int start);

    void update_projnode_filter(char const *proj_name, int deep_flag);
    WildPair *get_projNode_filter() const;

    void check_filter_and_script(int deep_flag);
    void update_filter(int deep_flag);
    bool accept_by_filter(char const *path, genString &ret_path);

    void set_root_node(int v);

    genArrOf(phy_rule_entry) *get_multi_phy_rule() const;

    // start index in the pdf_tree cur_arr of the root projMap (first leaf)
    // Also, that index is the line number (starting from 0) in the flex file, 
    // that corresponds to top level projMap (first meaningful leaf of the root projMap)
    int  get_root_pdf_tree_arr_index ();

    // each leaf has index in the pdf_tree cur_arr
    // First leaf has the same index as start index of the root projMap.
    // Also, leaf_index is the line number (starting from 0) in the flex file. 
    // leaf_index of the first_leaf = root_index for the projMap.
    int  get_leaf_pdf_tree_arr_index ();

    void set_pdf_tree_arr_indexes (int first_leaf, int &cur_leaf);
    int get_pmod_type  ();
    void set_pmod_type  (int);
    int get_included_type ();
    
    virtual void print (ostream& str = cout, int level = 0) const;

  private:

    path_hash_entry **fn_to_ln_file_h;
    path_hash_entry **fn_to_ln_dir_h;
    path_hash_entry **ln_to_fn_file_h;
    path_hash_entry **ln_to_fn_dir_h;
    path_hash_entry **fn_to_ln_file_imp_h;
    path_hash_entry **fn_to_ln_dir_imp_h;
    path_hash_entry **ln_to_fn_file_imp_h;
    path_hash_entry **ln_to_fn_dir_imp_h;

    int line_num; // The line in pdf file defines this project

    // a project with ->, <-, =>, or (**) will not be shown
    int view_flag;

    // which will be set in the node either
    // 1. its phy-path does not have leading slash and the phy-path of its child node
    //    has leading slash OR
    // 2. its phy-path has leading slash and the phy-path of its child node
    //    has leading slash and the child's path is a pre-path of the parent's path
    char leading_slash_in_child_phy_path;

    // WORKING BUFFER FOR ERROR GENERATOR
    // static int       path_complete_full_len;
    // static int       path_tran_error_code;
    // static genString path_tran_rule;

    // 0: It is a member.
    // 1: It is a project.
    // 2: duplicated projected.
    char flag;

    // : is NO_IMPLICIT_DIR_PROJ
    // = is IMPLICIT_DIR_PROJ
    char sub_proj_flag;

    // <-, ->, <->, or => between physical name and logic name
    // default is <->
    // LEFT_WAY, RIGHT_WAY, TWO_WAY, or REAL_RIGHT_WAY
    char direction;

    //                    project file
    // 1 : file       ==> f            ==> FILE_TYPE
    // 2 : directory  ==> d            ==> DIR_TYPE
    // 4 : link_file  ==> F            ==> LINK_FILE
    // 8 : link_dir   ==> D            ==> LINK_DIR
    // default: leaf is (fl) and non_leaf is (dl)
    // (*)/ could not have (f)
    int file_type;

    wkIdxRule *proj_rule;
    wkIdxRule *logic_rule;

    // Only its child has => and (0), (-1) ... negative index
    char child_real_right_way;
    // map_phy_path will stroe the mapped physical path which will be saved only when
    // it is converted from physical path to either the logical path or the proj name
    genString map_phy_path;

    genArrOf(phy_rule_entry) multi_phy_rule;

    // This is similar to the logic_rule and proj_rule for the member projects.
    // But these two rules is split by the parantheses.
    // The filter is split by the slash.
    projFilter *member_filter;

    // This is set only in the leaf node
    WildPair *projNode_filter;

    genArrOf(retDirPtr) ret_dir;

    // The next value is 0 always
    // It is set to 1 in the static function internal_fn_to_ln() of report.C
    // and reset to 0 there.
    int this_is_target;

    int para_rule_type;

    // initialize to be 0
    // [[ R ]] = 1
    // [[ W ]] = 2
    char read_only_flag;

    // in the pdf expanded tree
    // the number of path created by this pm is the same as the leaves in this
    // pm. One exception is for the implicit project which contains only the
    // exclude definitions.
    // local = $BASE <-> /aset { ^**.x "" : **.y => "" "" = SCCS => "" }
    // creates
    // $BASE/(**.x)
    // $BASE/(**.y)
    // $BASE/**(SCCS)/**
    // $BASE/**           <===  extra line
    // the pm for the project /local has three leaves. But the number of the created
    // path is 4.
    int num_created_path;
    pdf_tree_head *pth;
    FILE *writepipe;
    FILE *readpipe;
    int  root_node;
    int  root_pdf_tree_arr_index;
    int  leaf_pdf_tree_arr_index;

    int pmod_type;
    int included_type;
};

inline int projMap::get_pmod_type  ()        { return pmod_type; }
inline void projMap::set_pmod_type (int tp) { pmod_type = tp; }
inline int projMap::get_included_type ()     { return included_type; }

inline int projMap::get_root_pdf_tree_arr_index () 
{ 
    return root_pdf_tree_arr_index;
}

inline int projMap::get_leaf_pdf_tree_arr_index () 
{ 
    return leaf_pdf_tree_arr_index;
}

inline char projMap::get_flag()
{
    return flag;
}

inline char projMap::get_sub_proj_flag()
{
    return sub_proj_flag;
}

inline char projMap::get_direction()
{
    return direction;
}

inline char projMap::get_file_type()
{
    return file_type;
}


inline char const *projMap::get_org_project_name() const
{
    return proj_rule->get_org_idx();
}

inline char const *projMap::get_org_logic_name() const
{
    return logic_rule->get_org_idx();
}

inline char const *projMap::get_dir_name() const
{
    return multi_phy_rule[0]->phy_rule->get_ptn_def();
}

inline char const *projMap::get_org_dir_name() const
{
    return multi_phy_rule[0]->phy_rule->get_org_ptn();
}

inline projFilter *projMap::get_member() const
{
    return member_filter;
}

inline WildPair *projMap::get_projNode_filter() const
{
    return projNode_filter;
}

inline genArrOf(retDirPtr) *projMap::get_ret_dir() const
{
    return (retDirPtrArr*) &ret_dir;
}

inline genArrOf(phy_rule_entry) *projMap::get_multi_phy_rule() const
{
    return (phy_rule_entryArr*) &multi_phy_rule;
}

inline wkIdxRule *projMap::get_project_rule()
{
    return proj_rule;
}

inline wkIdxRule *projMap::get_logic_rule()
{
    return logic_rule;
}

inline int projMap::get_line_num()
{
    return line_num;
}

inline void projMap::set_line_num(int n)
{
    line_num = n;
}

inline int projMap::get_view_flag()
{
    return view_flag;
}

inline void projMap::set_view_flag(int v)
{
    view_flag = v;
}

inline void projMap::set_file_type(char f_type)
{
    file_type = f_type;
}

inline void projMap::set_proj_flag(char f)
{
    flag = f;
}

inline char const *projMap::get_map_phy_path()
{
    return (char const *)map_phy_path.str();
}

inline bool projMap::is_child_real_right_way()
{
    return (child_real_right_way ? TRUE : FALSE);
}

inline void projMap::set_child_real_right_way(char c)
{
    child_real_right_way = c;
}

inline int projMap::is_project()
{
    return((flag == PROJECT || flag == CREATED_PROJECT) ? 1 : 0 );
}

inline int projMap::is_exclude_proj()
{
    return(flag == EXCLUDE_PROJECT ? 1 : 0);
}

inline int projMap::carot_proj()
{
    return(*get_org_dir_name() == '^' ? 1 : 0);
}

inline int projMap::is_member()
{
    return((flag == MEMBER || flag == CREATED_MEMBER) ? 1 : 0 );
}

inline int projMap::is_leaf()
{
    return((get_child_project()) ? 0 : 1 );
}

inline int projMap::is_implicit_sub_dir_proj()
{
    return(sub_proj_flag == IMPLICIT_DIR_PROJ ? 1 : 0);
}

inline bool projMap::is_two_way()
{
    return(direction == TWO_WAY ? TRUE: FALSE);
}

inline bool projMap::is_left_way()
{
    return(direction == LEFT_WAY ? TRUE: FALSE);
}

inline bool projMap::is_right_way()
{
    return((direction == RIGHT_WAY || direction == REAL_RIGHT_WAY) ? TRUE: FALSE);
}

inline bool projMap::is_real_right_way()
{
    return(direction == REAL_RIGHT_WAY ? TRUE: FALSE);
}

inline bool projMap::is_reg_file()
{
    return((file_type & FILE_TYPE) ? TRUE: FALSE);
}

inline bool projMap::is_reg_dir()
{
    return((file_type & DIR_TYPE) ? TRUE: FALSE);
}

inline bool projMap::is_link_file()
{
    return((file_type & LINK_FILE) ? TRUE: FALSE);
}

inline bool projMap::is_link_dir()
{
    return((file_type & LINK_DIR) ? TRUE: FALSE);
}

inline bool projMap::is_file()
{
    return(is_leaf() && !is_implicit_sub_dir_proj());
}

inline bool projMap::is_dir()
{
    return((file_type & DIR_TYPE) || (file_type & LINK_DIR) ? TRUE: FALSE);
}

inline bool projMap::is_link()
{
    return((file_type & LINK_FILE) || (file_type & LINK_DIR) ? TRUE: FALSE);
}

inline bool projMap::need_to_check_file_type()
{
    return(file_type == DEFAULT_TYPE || file_type == (FILE_TYPE | DIR_TYPE | LINK_FILE | LINK_DIR)
           ? FALSE : TRUE);
}

generate_descriptor (projMap,appTree);

class or_path_data
{
  public:
    or_path_data(char const *projn, char const *phyn, char const *logn);
    ~or_path_data();

    void set_next(or_path_data *n);
    or_path_data *get_next();

    char const *get_projn() const;
    char const *get_phyn()  const;
    char const *get_logn()  const;

  private:
    char const *projn;
    char const *phyn;
    char const *logn;
    or_path_data *next;
};

inline void or_path_data::set_next(or_path_data *n)
{
    next = n;
}

inline or_path_data *or_path_data::get_next()
{
    return next;
}

inline char const *or_path_data::get_projn() const
{
    return projn;
}

inline char const *or_path_data::get_phyn() const
{
    return phyn;
}

inline char const *or_path_data::get_logn() const
{
    return logn;
}

//
//  The next one will help parser to handle the recursive definition of
//  project rule.
//
//  When "proj_name: wild_replacement" is reduced for a new project and
//  "project_spec: proj_name loc_and_contents" is not reduced for the previous project
//  the new project definition will go one level down, and is the first child
//  of the previous project.
//
//  When "proj_name: wild_replacement" is reduced for a new project and
//  "project_spec: proj_name loc_and_contents" is already reduced for the previous project
//  the new project definition is in the same level as the last project
//  and is the "next" of the previous project.
//
//  This class objects will construct a double link list.
//
//  The first node is the newest of all projects.
//
//  The second node is the last of the sub_projects of the project in
//      the first node.
//
//  Any node is the last of the sub_projects of the (sub_)project in
//      the previous node.
//
//  Because the member structure, it will go one level down when '{' is seen.
//  And it will go one level up when '}' is seen.
//
class projLevel
{
  public:
    projLevel(projMap *p = 0, int le = 0);

    ~projLevel();

    projMap    *get_map()    const;
    int        get_level();
    projLevel  *get_child()  const;
    projLevel  *get_parent() const;
    or_path_data *get_opd();

    void set_map(projMap *m);
    void set_child(projLevel *p);
    void set_opd(or_path_data *opd);

  private:
    projMap    *level_l; // the last of this level branch.
    int        l;        // level: debug use.
    projLevel  *child;
    projLevel  *parent;
    or_path_data *opd;
};

inline projLevel::projLevel(projMap *p, int le) :
       level_l(p), l(le), child(0), parent(0), opd(0)
{
}

inline projLevel::~projLevel()
{
}

inline projMap *projLevel::get_map() const
{
    return level_l;
}

inline int projLevel::get_level()
{
    return l;
}

inline projLevel *projLevel::get_child() const
{
    return child;
}

inline projLevel *projLevel::get_parent() const
{
    return parent;
}

inline or_path_data *projLevel::get_opd()
{
    return opd;
}

inline void projLevel::set_map(projMap *m)
{
    level_l = m;
}

inline void projLevel::set_opd(or_path_data *o)
{
    opd = o;
}

inline void projLevel::set_child(projLevel *p)
{
    child = p;
    if (p)
        p->parent = this;
}

extern bool    init_psetProjects(char const *fn);
extern bool    file_project(projMap *proj_head, char const *full_path, 
			char construct_new_path);
extern bool    module_in_project(char const *cur_module);
extern bool    path_in_project(char const *cur_path);
extern bool    need_expand_dir(char const *str);
extern bool    contain_target_char(char const *r, char tar, int ignore_quote);
extern bool    exclude_this_top_one(char const *path, projMap *ppm);
extern bool    check_proj_name_for_build(projNode *pn, char const *proj_name);

extern int     create_clean_ptn_str(wkPtnDef *ptn_strs, genString &dir_ptn);
extern int     report_complete_len();
extern int     report_tran_error_code();
extern int     count_level(char const *dir_ptn);
extern int     paraset_get_fullpath(char const *path, genString &new_path);
extern int     is_verify_pdf();
extern int     check_supersede_rule(projMap *proj_rule_head, char const *fn);
extern int     good_proj_name(char const *path, char const *sub_proj_name);

extern projMap *get_top_level_proj(char const *proj_name);
extern projMap *match_one_proj_name(projMap *proj_head, char const *s);
extern projMap *get_projmap_for_projnode(projNode *pn);

extern pdf_tree_head *report_all_maps(projMap *proj_head);

extern void    set_verify_pdf_mode();
extern void    log_proj_node(projNode *pn);
extern void    log_verify_names(projNode *pn, char const *full_path, char const *ln, int file_type);
extern void    verify_projnode();
extern void    verify_pdf(char const *fn);
extern void    pdf_debug(char const *proj_name, char const *phy_n, char const *log_n, char const *sub_proj_name);
extern void    combine_dir_and_sub_dir(char const *cur_dir, char const *sub_dir, genString &ret);
extern void    delete_all_maps(projMap *_head); // simulation
extern void    init_module_check_tree();
extern void    init_path_check_tree();
extern void    ptn_path_to_idx_path(char const *path, wkIdxRule *cur_idx_rule, 
			wkPtnRule *cur_ptn_rule, genString &ret_path, 
			projMap *pm);
extern void    tranpath(FILE *);
extern void    update_the_using_rule(genString &s, char const *rule, 
			projMap *cur);
extern void    project_convert_filename(char const *path, genString &ret_path);
extern void    remove_back_slash_and_parentheses(char *s);
extern void    proj_generate_subs_one_level(projNode*,char what, 
			int force_refresh_flag = 0);
extern void    proj_generate_subs_one_level_force_refresh(projNode*,char what);
extern void    remove_one_projmap(projNode *pn);
extern void    load_pdf_log_pm(projMap *pm, int first_flag);
extern void    load_pdf_remove_pm();
extern void    clean_pdf_file_name();
extern void    clear_aset_head();
extern void    construct_list_projects();
extern void    add_one_rule(projMap *pm);


extern void    remove_scope_name(char const *);
extern void    create_one_scope_name(int line_num, char const *scope_n, 
			int unit_name_count, char const **unit_name);
extern void    check_scope_name();
extern void    free_top_project_names_by_logic_name(char const **pn, int count);
extern char const *add_scope_name(char const **logic_name, int logic_name_count);
char const **create_name_buf_for_scope_name(int &idx);
char const **get_all_unit_names(char const *name, int &count);
extern char const **get_top_project_names_by_logic_name(char const *ln, int &count);


extern char const *get_PDF();

extern "C" char *path_sys_vars_translate(char const *path);
extern "C" char const *paraset_realpath(char const *path, char *ret_buf);

extern "C" int  get_path_parse_line_num();
extern "C" int  pathparse();
extern "C" int  gettime();

extern "C" void init_path_line_no();
extern "C" void create_one_path_parser_error(unsigned int, char const *, 
			char const *, int, int, int);
#endif

#if 0
/*
$Log: path.h  $
Revision 1.32 2001/12/21 10:45:03EST Sudha Kallem (sudha) 
use preference instead of subsystems rule in the pdf to handle groups, merging from /main/subsys_rule
// Revision 1.105  1994/07/11  17:05:38  so
// Bug track: n/a
// project for re-load pdf
//
// Revision 1.104  1994/06/23  19:10:42  so
// Bug track: n/a
// enhance -verify_pdf
//
// Revision 1.103  1994/06/20  14:17:53  so
// Bug track: n/a
// pdf-debugger : command-line options
//
// Revision 1.102  1994/06/05  16:38:47  so
// 1. if bring a raw view from the system, and do manage->get the raw view is not unloaded. The got file will be
//     file_name<2> in set buffer.
// 2. Do not check the directory fro the rule if all its children has a leading slash
// 3. Don't chop the last '/' if it is the make rule
// 4. fix bugs 6808 and 6367
//
// Revision 1.101  1994/06/01  15:51:15  so
// ^xx will exclude the directory xx and file xx
//
// Revision 1.100  1994/05/30  10:54:47  so
// clean up multi_match code
//
// Revision 1.99  1994/05/29  12:20:42  so
// fix
// ../(*)/include/(1)
//
// Revision 1.98  1994/05/27  18:48:12  builder
// another comment error
//
// Revision 1.97  1994/05/27  18:34:12  builder
// Fix comment error
//
//
// Revision 1.95  1994/05/26  12:46:19  so
// Bug track: n/a
// remove restriction for file name mapping
//
// Revision 1.94  1994/05/23  21:18:36  so
// Bug track: n/a
// fast projHeader::fn_to_ln
//
// Revision 1.92  1994/04/28  12:55:57  so
// Bug track: 6944
// fix bug 6944
//
// Revision 1.91  1994/04/26  12:39:27  so
// fix bug 6916 and 7098
//
// Revision 1.90  1994/04/12  11:42:50  so
// phase 2 of atria
//
// Revision 1.89  1994/04/05  11:20:06  so
// phase 1 of atria cm
//
// Revision 1.88  1994/03/31  00:18:55  so
// Bug track: 6659
// fix bug 6659
//
// Revision 1.87  1994/03/19  22:24:01  so
// Bug track: 6757
// fix bug 6757
//
// Revision 1.86  1994/03/17  14:12:23  so
// Bug track: 6703
// fix bug 6703
//
// Revision 1.85  1994/03/14  20:36:17  builder
// Port
//
// Revision 1.84  1994/03/12  11:30:33  so
// Bug track: 6575
// fix bug 6575
//
// Revision 1.83  1994/03/10  12:44:22  so
// Bug track: 6622 6671
// fix bug 6622 and 6671
//
// Revision 1.82  1994/02/17  23:27:35  so
// Bug track: 6405
// fix bug 6405
//
// Revision 1.81  1994/02/08  21:00:18  builder
// Port
//
// Revision 1.80  1994/01/11  19:15:50  so
// Bug track: 5857
// fix part of bug 5857
//
// Revision 1.79  1993/12/10  12:13:20  so
// Bug track: 5510
// fix bug 5510
//
// Revision 1.78  1993/11/16  22:09:48  so
// Bug track: 5155
// fix bug 5155
// A new feature '|' for the pdf file.
//
// Revision 1.77  1993/11/11  19:46:26  so
// Bug track: 5233
// fix bug 5233
//
// Revision 1.76  1993/11/10  16:08:04  so
// Bug track: 5219
// fix bug 5219
//
// Revision 1.75  1993/10/28  20:13:28  so
// Bug track: 4999, 5037, 5082
// fix bugs 4999, 5037, 5082
//
// Revision 1.74  1993/10/19  20:32:24  so
// Bug track: 5006
// fix bugs 5006, 5015, 4839
//
// Revision 1.73  1993/10/15  14:54:09  so
// Bug track: 4958
// fix bug 4958
//
// Revision 1.72  1993/10/12  21:29:21  so
// Bug track: 4793
// fix bug 4793
//
// Revision 1.71  1993/10/12  16:50:28  so
// Bug track: 4920
// fix bug 4920
// enhance for pdf to allow exclude projects
//
// Revision 1.70  1993/10/04  20:23:37  so
// Bug track: 4899
// fix bug 4899
//
// Revision 1.69  1993/09/24  21:47:47  so
// Bug track: 4243
// New feature
// PDF needs to handle copy of the file instead of link
//
// Revision 1.67  1993/09/20  14:37:22  so
// Bug track: 4542
// fix bug 4542
//
// Revision 1.66  1993/09/17  16:39:01  davea
// Bug track: 4783
// Don't use globdef.h, since it also #defines YES and NO, which
// conflict with the definition in Object.h
//
// Revision 1.65  1993/09/14  18:46:20  so
// use globdef.h for TRUE and FALSE
//
// Revision 1.64  1993/09/14  15:38:40  so
// don't use config.h
//
// Revision 1.63  1993/09/01  17:53:27  so
// performance fix.
//
// Revision 1.62  1993/08/30  23:38:12  so
// test
//
// Revision 1.61  1993/08/30  23:27:11  so
// test
//
// Revision 1.60  1993/07/29  17:47:26  so
// replace config.h with cpp_config.h
//
// Revision 1.59  1993/07/01  20:32:21  so
// declare
// extern projMap *get_projmap_for_projnode(projNode *pn);
//
// Revision 1.58  1993/06/25  14:01:56  bakshi
// explicit type casts for c++3.0.1 port
//
// Revision 1.57  1993/05/27  23:15:17  so
// declare extern "C" char const* paraset_realpath(char const* path, char const * ret_buf);
//
// Revision 1.56  1993/05/25  19:12:39  so
// make paraset_get_fullpath() global.
//
// Revision 1.55  1993/05/20  21:05:00  so
// Implement the pdf-generating tool.
//
// Revision 1.54  1993/05/19  15:46:45  so
// Implement filter for implicit project.
//
// Revision 1.53  1993/05/18  16:09:01  so
// fix bug 3137
// No crash for bad pdf file.
//
// Revision 1.52  1993/05/10  21:58:11  so
// Implement (-1) negative index in "=>" project.
//
// Revision 1.51  1993/05/09  15:18:02  so
// implement (**) for pdf.
//
// Revision 1.50  1993/05/07  15:06:44  so
// If a project is ->, <-, => or (**)
// it will not be displayed.
//
// Revision 1.49  1993/05/07  03:23:08  so
// make multi_match between two slashes work.
//
// Revision 1.48  1993/05/05  23:35:08  so
// => will not pre_pend the path for the project name and the logical name.
//
// Revision 1.47  1993/05/05  21:53:36  so
// Clean up the error msg.
// use line number instead of the project and physical names.
//
// Revision 1.46  1993/05/04  18:55:35  so
// If the whole physical path will be matched into the logical and project names, no parentheses are needed.
//
// Revision 1.45  1993/05/04  14:56:22  so
// fix bug 3342
// create a new level of modules if
// the physical path starts with a slash.
//
// Revision 1.44  1993/04/30  20:56:51  so
// remove one extern declaration.
//
// Revision 1.43  1993/04/28  02:12:46  so
// declare project_convert_filename
//
// Revision 1.42  1993/04/27  20:33:28  so
// fix bug 3526
//
// Revision 1.41  1993/04/26  23:39:31  so
// fix memory leaking
//
// Revision 1.40  1993/04/25  14:33:24  so
// fix bug 3433
// fn_to_ln and ln_to_fn use genString to pass the result.
//
// Revision 1.39  1993/04/20  17:20:58  so
// change the comment for leading_slash_in_child_phy_path.
//
// Revision 1.38  1993/04/19  16:40:39  so
// Set a flag in the root map, if all his child does not have leading slash in their physical path.
//
// Revision 1.37  1993/04/16  15:12:04  so
// test for 3272
//
// Revision 1.36  1993/04/11  14:52:25  so
// Add a new file type PARTIAL_DIR_TYPE
//
// Revision 1.35  1993/04/09  22:46:05  so
// Create the full_project_name in search_modules_by_name_recursive.
//
// Revision 1.34  1993/04/09  17:10:29  so
// ptn_to_idx() and idx_to_ptn() requires one more parameters "rem_path".
//
// Revision 1.33  1993/04/01  03:37:38  so
//  Fix bug 3047.
// A new function to search all modules by its project name which must be defined in pdf file.
//
// Revision 1.32  1993/03/10  00:45:19  so
// Implement the file type attribute.
//
// Revision 1.31  1993/03/05  23:18:11  so
// implement link map [[ -> ]] and [[ => ]]
//
// Revision 1.30  1993/02/18  17:35:39  so
// /(1)y(2)/ will be legal in the logical name for the right way mapping "->"
//
// Revision 1.29  1993/02/16  18:59:53  so
// extern "C" void init_path_line_no();
// Add a new external declaration.
//
// Revision 1.28  1993/02/16  18:12:08  so
// Because the physical path is parsed twice,
// the second time don't need to print the error or warning messages.
//
// Revision 1.27  1993/02/11  18:45:18  so
// Fix and rewrite the function "find_project()"
//
// Revision 1.26  1993/02/09  20:24:03  so
// Make directory mapping work.
//
// Revision 1.25  1993/02/08  22:53:19  so
// Implement simpler format.
//
// Revision 1.24  1993/01/20  17:55:01  so
// implement implicit rule mapping when explicit rule is defined.
//
// Revision 1.23  1993/01/20  15:14:11  smit
// change test_project_parser to init_psetprojects.
//
// Revision 1.22  1993/01/19  21:39:49  so
// declare three new extern functions.
//
// Revision 1.21  1993/01/19  03:00:56  so
// Try to get more exact retreat length.
//
// Revision 1.20  1993/01/18  19:36:59  so
// Instead of passing genString as a parameter, pass char const *.
//
// Revision 1.19  1993/01/16  22:40:09  so
// fix some bug to allow the directory in member.
//
// Revision 1.18  1993/01/15  23:03:21  so
// Implement to allow directory as a member.
//
// Revision 1.17  1993/01/15  16:32:32  so
// remove some Initialize for inline statements.
//
// Revision 1.16  1993/01/14  18:25:16  so
// Implement error code generator for parsing the project file.
//
// Revision 1.15  1993/01/12  03:01:46  so
// Implement to report only one level.
//
// Revision 1.14  1993/01/11  20:34:23  so
// Implement error code generator for the path mapping.
//
// Revision 1.12  1993/01/08  22:31:07  so
// error generator -- version 1
//
// Revision 1.11  1993/01/07  23:24:42  so
// change some for the error generator.
//
// Revision 1.10  1993/01/07  04:34:23  so
// member wildcard is implemented.
//
// Revision 1.8  1993/01/06  00:46:45  so
// implement explicit mapping for FULL SPEC.
//
// Revision 1.7  1992/12/31  16:54:48  so
// implement multiple matching
//
// Revision 1.6  1992/12/30  01:13:09  so
// Move the map to the root.
//
// Revision 1.5  1992/12/27  22:43:18  so
// Use the same method to transfer file name for full spec as for template spec.
//
// Revision 1.4  1992/12/22  04:22:58  so
// 3rd revision
//
// Revision 1.3  1992/12/21  20:53:25  mg
// *** empty log message ***
//
// Revision 1.2  1992/12/21  17:35:11  so
// 2nd revision
//
// Revision 1.1  1992/12/19  21:20:17  so
// Initial revision
//
*/
#endif
