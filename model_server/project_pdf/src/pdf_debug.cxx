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
// pdf_debugger.C
//------------------------------------------
// synopsis:
//
// This is for pdf debugger.
//    1. -pdf_prt proj_name
//         will list the tree-like output that includes projnode and projmodule
//    2. -pdf_fn fn (-pdf_sub_proj_name proj_name)
//         verify this physical path fn
//         if -pdf_sub_proj_name is specified, only the projnode matched this project
//           name will be checked.
//    3. -pdf_ln ln (-pdf_sub_proj_name proj_name)
//         verify the logic name ln
//         if -pdf_sub_proj_name is specified, only the projnode matched this project
//           name will be checked.

// INCLUDE FILES

#include <cLibraryFunctions.h>
#include <msg.h>
#include "machdep.h"

#ifndef _proj_h
#include <proj.h>
#endif

#ifndef _path_h
#include <path.h>
#endif

#ifndef _genArr_h
#include <genArr.h>
#endif

#ifndef __psetmem_h
#include <psetmem.h>
#endif

static const char *lds = "           ";
static const char *erh = "PDF ERROR: ";
static const char *wah = "PDF WARNING: ";

static int proj_idx = 0;

static void check_dir(char *phy, projNode *pn, char *sub_proj_name, projNode *cur, int sub_def);

static projNode *get_v_proj(char *proj_name, int &found)
// search the search_list which matches proj_name if proj_name is defined
// if proj_name is NULL, then any projnode except the control project and the invisible
// (rule) projects will be accepted.
//
// found is set to 1 if it finds control project or invisible projects
{
    Initialize(get_v_proj);

    projNode *pn;
    while(1)
    {
        pn = projList::search_list->get_proj(proj_idx++);
        if (pn == projNode::get_control_project())
        {
            if (proj_name && good_proj_name(proj_name,pn->get_ln()))
            {
                found = 1;
                msg("$1Project \"$2\" is a reserved internal project for ParaSET") << wah << eoarg << proj_name << eom;
            }
            continue;
        }

        if (pn && !pn->is_visible_proj())
        {
            if (proj_name && good_proj_name(proj_name,pn->get_ln()))
            {
                found = 1;
                msg("$1Project \"$2\" is an invisible project") << wah << eoarg << proj_name << eom;
            }
            continue;
        }
        return pn;
    }
}

static int check_proj_exist(projNode *pn, char *proj_name, int file_type)
// check the physical name for this proj_name under this project node pn
//
// file_type could be FILE_TYPE or DIR_TYPE
//
// If at least, the project name could match to one physical name, return 1; otherwise
// return 0;
{
    Initialize(check_proj_exist);

    genString phy_n;
    pn->get_map()->ln_to_fn_imp(proj_name,pn->get_ln(),file_type,phy_n, 0, 1);
    int status = 0;
    if (phy_n.length())
    {
        status = 1;
        struct OStype_stat stat_buf;
        if (OSapi_stat(phy_n, &stat_buf) != 0)
            msg("$1The physical name \"$2\" for the proj_name \"$3\" does not exist.") << wah << eoarg << phy_n.str() << eoarg << proj_name << eom;
    }
    return status;
}

static void pr_proj_error(char *proj_name, int found)
// print the error about this proj_name
{
    Initialize(pr_proj_error);

    if (!found)
    {
        msg("$1proj_name \"$2\" is not specified in the pdf file.") << erh << eoarg << proj_name << eom;
        return;
    }
}

static int check_proj_name(char *proj_name)
// check if this proj_name could be converted into a physical name
// return 1 if it could; otherwise it returns 0.
{
    Initialize(check_proj_name);

    if (strcmp(proj_name, "/") == 0)
        return 1;

    int found = 0;
    projNode *pn;
    int status = 0;
    while(pn = get_v_proj(proj_name, found))
    {
        if (good_proj_name(proj_name, pn->get_ln()))
        {
            if (check_proj_exist(pn, proj_name, FILE_TYPE))
                status = 1;
            else if (check_proj_exist(pn, proj_name, DIR_TYPE))
                status = 1;
        }
    }
    return status;
}

static void pdf_list_proj_name(char *proj_name)
// this is the main routine for the switch -pdf_prt proj_name
{
    Initialize(pdf_list_proj_name);

    msg("\n====== list the project tree for \"$1\" ======\n") << proj_name << eom;
    int found = 0;
    projNode *pn = projNode::find_project (proj_name);;
    if (pn) 
	if (pn->pdf_prt(proj_name,0))
	    found = 1;

    pr_proj_error(proj_name, found);
}

static int remove_right_way_mark(genString &rule)
// if the rule is RIGHT WAY, then rule will contain ":R(line "
//   the last space is very important because no file name mapping should contain a space
//   unless they are pset or pmod .... rule
//
// if rule contain R which will be removed and it will return 1
// otherwise it will return 0
{
    Initialize(remove_right_way_mark);

    genString s = rule;
    char *p = strstr((char *)s, ":R(line ");
    if (!p)
        return 0;
    *(p+1) = '\0';
    rule.printf("%s%s",s.str(),p+2);
    return 1;
}

static void pr_error(int sub_def, int &one_warning, int type)
// sub_def = 1 if -pdf_sub_proj_name is defined; otherwise it is 0
//
// if one_warning is 1 then print some warning and one_warning will be set to 0
//
// type = 0 for logic name
//        1 for project name
{
    Initialize(pr_error);

    const char *tar;
    if (type == 0)
        tar = "logic";
    else
        tar = "project";
    if (sub_def)
    {
        genString rule = projMap::report_path_tran_rule();
        if (rule.length() == 0)
            msg("$1When converting from the physical name to $2 name,\n$3") << erh << eoarg << tar << eoarg << lds << "no rule in this project accepts this file." << eom;
        else if (strcmp(rule,REJECT_BY_PROJ_NAME) == 0)
            msg("$1When converting from the physical name to $2 name,\n$3") << erh << eoarg << tar << eoarg << lds << "it is rejected by the project name." << eom;
        else
            msg("$1When converting from the physical name to $2 name,\n$3\n$4") << erh << eoarg << tar << eoarg << lds << "it is rejected by the exclude rule:" << eoarg << lds << rule.str() << eom;
    }
    else if (one_warning)
    {
        one_warning = 0;
        msg("$1Please specify a sub_project name to show why it fails to convert to $2 name.") << wah << eoarg << tar << eom;
    }
}

static void check_name_mappings(projNode *pn, char *phy_n, char *spn, int file_type,
                                int sub_def)
// check mapping between physical name and logic   name
// check mapping between physical name and project name
//
// pn is the top project which contains phy_n
//
// phy_n is the physical name to be checked
//
// spn is the sub_project_name
//
// file_type: FILE_TYPE, phy_n is a file      name
// file_type: DIR_TYPE,  phy_n is a directory name
//
// sub_def = 1 if -pdf_sub_proj_name is defined; otherwise sub_def = 0
{
    Initialize(check_name_mappings);

    if (!sub_def && good_proj_name(pn->get_ln(), "/ParaDOCS"))
        return;

    if (file_type & FILE_TYPE)
        msg("  === check file name $1 under project name $2 ===\n") << phy_n << eoarg << spn << eom;
    else
        msg("  === check directory name $1 under project name $2 ===\n") << phy_n << eoarg << spn << eom;

    static int one_warning = 1;
    genString ln;
    // fn to ln
    pn->get_map()->fn_to_ln(phy_n, spn, file_type, ln, 0, 1);
    if (ln.length() == 0) // fail
    {
        pr_error(sub_def, one_warning, 0);
        return;
    }

    // fn to proj_name
    genString proj_n;
    pn->get_map()->fn_to_ln_imp(phy_n, spn, file_type, proj_n, 0, 1);
    genString rule0 = projMap::report_path_tran_rule();
    int right_way = remove_right_way_mark(rule0);
    if (proj_n.length() == 0) // fail
    {
        pr_error(sub_def, one_warning, 1);
        return;
    }

    // pset file name
    genString pset_n;
    genString src;
    src.printf("%s/%%/.pset",phy_n);
    projHeader::fn_to_ln_special_rule(src,pset_n);

    genString physical_n;
    if (proj_n.length() == 0)
        proj_n = "NULL";
    if(phy_n == NULL)
	physical_n = "NULL";
    else
	physical_n = phy_n;
    if (ln.length() == 0)
        ln = "NULL";
    if (pset_n.length() == 0)
        pset_n = "NULL";
     
    msg("($1)($2)($3)($4)") << proj_n.str() << eoarg << physical_n.str() << eoarg << ln.str() << eoarg << pset_n.str() << eom;

    struct OStype_stat stat_buf;
    if (OSapi_stat(phy_n, &stat_buf) != 0)
      msg(" -- NOT EXIST") << eom;
    else
      msg("") << eom;

    // check reverse mapping -- logical name
    int logic_error = 0;
    genString phy0;
    pn->get_map()->ln_to_fn(ln, spn, file_type, phy0, 0, 1);
    genString rule1 = projMap::report_path_tran_rule();
    int RW = remove_right_way_mark(rule1);
    Assert(!RW); // when logic name is obtained the rule should not be RIGHT WAY
    if (phy0.length() == 0)
        msg("$1the logic name could not map back to a physical name in the same project") << wah << eom;
    else if (!right_way && (strcmp(phy0, phy_n) || strcmp(rule0, rule1)))
    {
        // it is OK if rule0 is RIGHT WAY and rule1 is TWO WAY
        // rule1 must be TWO WAY because LEFT WAY is not used
        // hence it needs to check if rule0 is RIGHT WAY

        logic_error = 1;
        msg("$1") << erh << eom;
        if ((file_type & FILE_TYPE) && strcmp(phy0, phy_n))
            msg("$1the logic name will map back to a different physical name\n$2\"$3\"") << lds << eoarg << lds << eoarg << phy0.str() << eom;
        if (file_type & FILE_TYPE)
            msg("$1It is not supported that the following two rules could\n$2\n$3\"$4\"\n$5\"$6\"") << lds << eoarg << lds << "map two different physical names into an identical logic name:" << eoarg << lds << eoarg << rule0.str() << eoarg << lds << eoarg << rule1.str() << eom;
        else
            msg("$1The following two rules need to be swapped, because they\n$2\n$3\"$4\"\n$5\"$6\"") << lds << eoarg << lds << "map two different directory names into an identical logic name:" << eoarg << lds << eoarg << rule0.str() << eoarg << lds << eoarg << rule1.str() << eom;
    }

    // check reverse mapping -- project name
    if (!logic_error)
    {
        int proj_error = 0;
        genString phy1;
        pn->get_map()->ln_to_fn_imp(proj_n, spn, file_type, phy1, 0, 1);
        genString rule1 = projMap::report_path_tran_rule();
        remove_right_way_mark(rule1);
        if (phy1.length() == 0)
        {
            proj_error = 1;
            msg("$1the project name could not map back to a physical name") << erh << eom;
        }
        else if (strcmp(phy1, phy_n) || strcmp(rule0, rule1))
        {
            proj_error = 1;
            msg("$1") << erh << eom;
            if ((file_type & FILE_TYPE) && strcmp(phy1, phy_n))
                msg("$1the project name will map back to a different physical name\n$2\"$3\"") << lds << eoarg << lds << eoarg << phy1.str() << eom;
            if (file_type & FILE_TYPE)
                msg("$1It is not supported that the following two rules could\n$2\n$3\"$4\"\n$5\"$6\"") << lds << eoarg << lds << "map two different physical names into an identical project name:" << eoarg << lds << eoarg << rule0.str() << eoarg << lds << eoarg << rule1.str() << eom;
            else
                msg("$1The following two rules need to be swapped, because they\n$2\n$3\"$4\"\n$5\"$6\"") << lds << eoarg << lds << "map two different directory names into an identical project name:" << eoarg << lds << eoarg << rule0.str() << eoarg << lds << eoarg << rule1.str() << eom;
        }

        if (!proj_error)
        {
            projNode *par = pn->find_sub_project(proj_n);
            if (par && par->get_map() == 0)
                check_dir(phy_n, pn, spn, par, sub_def);
        }
    }
}

static void check_dir(char *phy, projNode *pn, char *spn, projNode *cur, int sub_def)
// check the mapping of a directory
//
// phy is the physical name of its child
//
// pn is the top project
//
// spn is the sub_project which covers the interested area
//
// cur is the bottom project which is checked now
//
// sub_def = 1 if -pdf_sub_proj_name is defined; otherwise sub_def = 0
{
    Initialize(check_dir);

    genString phy_n = phy;
    while(1)
    {
        char *p = strrchr((char *)phy_n,'/');
        if (p)
            *p = '\0';
        if (strcmp(phy_n,"/") == 0)
            return;
        genString ln;
        pn->get_map()->fn_to_ln(phy_n, spn, DIR_TYPE, ln, 0, 1);
        if (ln.length())
            break;
    }

    check_name_mappings(pn, phy_n, spn, DIR_TYPE, sub_def);

    projNode *par = cur->find_parent();
    if (par && par->get_map() == 0)
        check_dir(phy_n, pn, spn, par, sub_def);
}

static void pdf_list_physical_name(char *phy_n, char *sub_proj_name)
// this is the main routine for -pdf_fn phy_n -pdf_sub_proj_name proj_name
//
// phy_n is the physical name to be checked
//
// sub_proj_name is defined by -pdf_sub_proj_name. If it is defined, only the sub_tree
//    belongs to this sub_proj_name is interestde
{
    Initialize(pdf_list_physical_name);

    msg("\n====== list for physical name \"$1\" ") << phy_n << eom;
    if (sub_proj_name)
        msg("under the sub_project_name \"$1\" ") << sub_proj_name << eom;
    msg(" ======\n") << eom;

    int found = 0;
    proj_idx = 1;
    projNode *pn;
    while(pn = get_v_proj(sub_proj_name, found))
    {
        if (!sub_proj_name || sub_proj_name && good_proj_name(sub_proj_name, pn->get_ln()))
        {
            found = 1;
            genString spn;
            int sub_def = 0;
            if (sub_proj_name)
            {
                spn = sub_proj_name;
                sub_def = 1;
            }
            else
                spn = pn->get_ln();
            check_name_mappings(pn, phy_n, spn, FILE_TYPE, sub_def);
        }
    }
    pr_proj_error(sub_proj_name, found);
}

static void pdf_list_logic_name(char *log_n, char *sub_proj_name)
// this is the main routine for -pdf_ln log_n -pdf_sub_proj_name proj_name
//
// log_n is the logic name to be checked
//
// sub_proj_name is defined by -pdf_sub_proj_name. If it is defined, only the sub_tree
//    belongs to this sub_proj_name is interestde
{
    Initialize(pdf_list_logic_name);

    msg("\n====== list for logic name \"$1\" ") << log_n << eom;
    if (sub_proj_name)
        msg("under the sub_project_name \"$1\" ") << sub_proj_name << eom;
    msg(" ======\n") << eom;

    int found = 0;
    proj_idx = 1;
    projNode *pn;
    while(pn = get_v_proj(sub_proj_name, found))
    {
        if (!sub_proj_name || sub_proj_name && good_proj_name(sub_proj_name, pn->get_ln()))
        {
            found = 1;
            genString spn;
            int sub_def = 0;
            if (sub_proj_name)
            {
                spn = sub_proj_name;
                sub_def = 1;
            }
            else
                spn = pn->get_ln();

            genString phy_n;
            pn->get_map()->ln_to_fn(log_n, spn, FILE_TYPE, phy_n, 0, 1);
            if (phy_n.length() == 0)
                msg("$1the logic name could not map to a physical name.") << wah << eom;
            check_name_mappings(pn, phy_n, spn, FILE_TYPE, sub_def);
        }
    }
    pr_proj_error(sub_proj_name, found);
}

void pdf_debug(char *proj_name, char *phy_n, char *log_n, char *sub_proj_name)
// this is the main entry for pdf debugger
// it will call pdf_list_proj_name(char *proj_name)
//     for the switch -pdf_prt proj_name
// it will call pdf_list_physical_name(phy_n, sub_proj_name)
//     for the switch -pdf_fn phy_n -pdf_sub_proj_name proj_name
// it will call pdf_list_logic_name(log_n, sub_proj_name)
//     for the switch -pdf_ln log_n -pdf_sub_proj_name proj_name
{
    Initialize(pdf_debug);

    if (!phy_n && !log_n && sub_proj_name)
        msg("$1-pdf_sub_proj_name should only be used with either -pdf_fn or -pdf_ln") << wah << eom;
    if (proj_name)
    {
        if (!check_proj_name(proj_name))
        {
            pr_proj_error(proj_name, 0);
            return;
        }
        pdf_list_proj_name(proj_name);
    }

    if (sub_proj_name && !check_proj_name(sub_proj_name))
    {
        pr_proj_error(sub_proj_name, 0);
        return;
    }

    if (phy_n)
        pdf_list_physical_name(phy_n, sub_proj_name);
    if (log_n)
        pdf_list_logic_name(log_n, sub_proj_name);
    ;
}

/*
$Log: pdf_debug.cxx  $
Revision 1.11 2000/07/12 18:12:06EDT ktrans 
merge from stream_message branch
 * Revision 1.2  1994/06/23  19:14:45  so
 * Bug track: n/a
 * enhance -verify_pdf
 *
 * Revision 1.1  1994/06/20  14:21:47  so
 * Initial revision
 *
*/
