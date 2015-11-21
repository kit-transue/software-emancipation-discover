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

#include <vport.h>

#ifndef vcharINCLUDED
#include vcharHEADER
#endif

#include <galaxy_undefs.h>
#include <cLibraryFunctions.h>
#include "../../DIS_ui/interface.h"

#include <genString.h>
#include <gtGenArr.h>
#include <symbolArr.h>
#include <proj.h>
#include <path.h>
#include <customize.h>
#include <vpopen.h>
#include <sys/stat.h>

#include "../include/boilerplate.h"

//
//  Global Variable Declarations
//

extern Application* DISui_app;
static genString phy_path_of_boiler;
Boilerplate* Boilerplate::instance = NULL;

static char *func_array[] = {"func-init",
                             "func-final",
                             "before-func-decl",
                             "after-func-decl",
                             "before-func-def",
                             "after-func-def",
                             "pub-decl",
                             "prot-decl",
                             "priv-decl",
                             "before-class-decl",
                             "after-class-decl",
                             "before-class-def",
                             "after-class-def",
                             "before-struct",
                             "after-struct",
                             "member-def-init",
                             "member-def-final",
                             "before-member-def",
                             "after-member-def",
                             "before-member-decl",
                             "after-member-decl",
                             "relation-src-member",
                             "relation-src-header",
                             "relation-src-definition",
                             "relation-trg-member",
                             "relation-trg-header",
                             "relation-trg-definition",
                             "c-src-file-top",
                             "c-src-file-bottom",
                             "c-hdr-file-top",
                             "c-hdr-file-bottom",
                             "cpp-src-file-top",
                             "cpp-src-file-bottom",
                             "cpp-hdr-file-top",
                             "cpp-hdr-file-bottom"
};

static int global_bit_set[No_of_boilerplates];

static genString func_init_boilerplate("");
static genString func_final_boilerplate("");
static genString before_func_decl_boilerplate("");
static genString after_func_decl_boilerplate("");
static genString before_func_def_boilerplate("");
static genString after_func_def_boilerplate("");
static genString pub_decl_boilerplate("");
static genString prot_decl_boilerplate("");
static genString priv_decl_boilerplate("");
static genString before_class_decl_boilerplate("");
static genString after_class_decl_boilerplate("");
static genString before_class_def_boilerplate("");
static genString after_class_def_boilerplate("");
static genString before_struct_boilerplate("");
static genString after_struct_boilerplate("");
static genString member_def_init_boilerplate("");
static genString member_def_final_boilerplate("");
static genString before_member_def_boilerplate("");
static genString after_member_def_boilerplate("");
static genString before_member_decl_boilerplate("");
static genString after_member_decl_boilerplate("");
static genString relation_src_member_boilerplate("");
static genString relation_src_header_boilerplate("");
static genString relation_src_definition_boilerplate("");
static genString relation_trg_member_boilerplate("");
static genString relation_trg_header_boilerplate("");
static genString relation_trg_definition_boilerplate("");
static genString c_src_file_top_boilerplate("");
static genString c_src_file_bottom_boilerplate("");
static genString c_hdr_file_top_boilerplate("");
static genString c_hdr_file_bottom_boilerplate("");
static genString cpp_src_file_top_boilerplate("");
static genString cpp_src_file_bottom_boilerplate("");
static genString cpp_hdr_file_top_boilerplate("");
static genString cpp_hdr_file_bottom_boilerplate("");

#define NUM_REL_BOILS 6

static genString* rel_boils[] = {
    &relation_src_member_boilerplate,
    &relation_src_header_boilerplate,
    &relation_src_definition_boilerplate,
    &relation_trg_member_boilerplate,
    &relation_trg_header_boilerplate,
    &relation_trg_definition_boilerplate
};

static const char* rel_boil_names[] = {
    "relation-src-member",
    "relation-src-header",
    "relation-src-definition",
    "relation-trg-member",
    "relation-trg-header",
    "relation-trg-definition",
};

static genString dd_class_name("");
static genString dd_function_name("");
static genString dd_struct_name("");
static genString dd_member_name("");
static genString dd_physical_file_name("");
static genString dd_logical_file_name("");
static genString dd_source_name("");
static genString dd_target_name("");
static genString dd_author_name("");
static genString dd_user_name("");
static genString dd_Date("");
static genString dd_project_name("");


//
//  Function Forward Declarations
//

extern void proj_search_modules_by_name(char *proj_name, symbolArr &s1);
extern int makeUNIXFile(genString& pathname);

//
//  Function Definitions
//


static int delete_path(const char* file_name) {

    // Return 1 only if deleted succesfully

    Initialize(delete_path);

    struct OStype_stat info;
    if (OSapi_stat((char*)file_name, &info) == 0) {

        projModule *mod;
        genString log_name;
        projHeader::fn_to_ln(file_name, log_name);

        if (log_name.length()) { // if not a project file, do nothing
            projNode* proj_node = projNode::get_home_proj();
            if (!proj_node) proj_node = projHeader::non_current_project(log_name);
            if (!proj_node) return -1;

            mod = proj_node->find_module(log_name);
            if (mod) {
                // confirm deletion if module is loaded

                appPtr app_head = mod->get_app();
                if (app_head) {
                    if (app_head->is_modified()) {
                        genString command;
                        command.printf ("dis_prompt {YesNoDialog} {Create} {Module modified; really want to delete?}");
                        vchar *answer = rcall_dis_DISui_eval (DISui_app, (vstr *)command.str());
                        if (strcmp ((char *)answer, "No") == 0) return 0;
                    }

                    mod->unload_module();
                }

                mod->delete_module();
            }
        }

        genString command;
        command.printf("rm -rf %s", file_name);
        v_system(command);

        // Check if deletion failed:
        // stat() succeeds  OR  failure was not due to missing file

        if(!OSapi_stat((char*)file_name, &info)  ||  errno != ENOENT) {
            genString command;
            command.printf ("dis_prompt {OKDialog} {Create} {Unable to delete file %s}", 
                            file_name);
            rcall_dis_DISui_eval_async (DISui_app, (vstr *)command.str());
            return -1;

        }

        return 1;
    }

    else {

        genString command;
        switch(errno) {
          case EACCES:
            command.printf ("dis_prompt {OKDialog} {Create} {No search permission for %s}", 
                            file_name);
            rcall_dis_DISui_eval_async (DISui_app, (vstr *)command.str());
            break;

          case ENOENT:
            command.printf ("dis_prompt {OKDialog} {Create} {File %s does not exist.}", 
                            file_name);
            rcall_dis_DISui_eval_async (DISui_app, (vstr *)command.str());
            break;

          default:
            command.printf ("dis_prompt {OKDialog} {Create} {Error #%d occured while deleting file %s}",  
                            errno, file_name);
            rcall_dis_DISui_eval_async (DISui_app, (vstr *)command.str());
            break;
        }

        return -1;
    }

}


/************************************************************************
*  boil_share_obtain_proj_info
*  Summary  : Initializes the shared project information for boilerplates.
*  Arguments: (none)
*  Returns  : s1 - the list of all boilerplates
*             main_array - 
*             phy_path_of_boiler - 
*
*/

static int boil_share_obtain_proj_info(symbolArr &s1, GenArr &main_array, genString &phy_path_of_boiler)
{
    Initialize(boil_share_obtain_proj_info);

    char *proj_name = ".boilerplate";

    proj_search_modules_by_name(proj_name,s1);
    projNode* home_proj = projNode::get_home_proj();

    projNode* pr = 0;
    int found = 0;
    char* ln = home_proj->get_ln();

    if (strcmp(ln+1,proj_name) == 0)
    {
        pr = home_proj;
        found = 1;
    }

    // For some reason I set the boilerplate project to be of the form
    // ~/temp - why ? -- becuse it helps in the translation of the
    // physical path of the boilerplate.

    if (!found)
    {
        if (home_proj->is_script())
        {
            genString proj_n;
            genString fn;
            proj_n.printf("%s/%s/aa", home_proj->get_ln(), proj_name);
            home_proj->ln_to_fn_imp(proj_n, fn, FILE_TYPE, 0, 1);
            if (fn.length())
            {
                found = 1;
                pr = home_proj;
                char *p = strrchr(fn.str(),'/');
                if (p)
                    *p = '\0';
                phy_path_of_boiler = fn.str();
            }
        }
        else
        {
            objSet os = parentProject_get_childProjects(home_proj);
            ObjPtr ob;
            ForEach (ob, os)
            {
                pr = checked_cast(projNode, ob);
                ln = pr->get_ln();
                char *p = strrchr(ln, '/');
                if (p)
                    p++;
                else
                    break;
                if (strcmp(p,proj_name) == 0)
                {
                    found = 1;
                    break;
                }
            }
        }
    }

    if (found)
    {
        if (!pr->is_script())
            pr->ln_to_fn_imp(ln,phy_path_of_boiler,DIR_TYPE);
    }

    for (int i = 0; i<s1.size(); i++)
    {
        projModule *m = checked_cast(projModule,s1[i]);
        if (m == NULL)
            continue;
        genString temp_name;
        m->get_phys_filename(temp_name);
        if (temp_name[temp_name.length() - 1] != '~')
            main_array.append((char *)temp_name.str());
    }
    return (!found);
}


/************************************************************************
*  init_default_boilerplates
*  Summary  : Initializes the default boilerplate static global variables.
*  Arguments: (none)
*  Returns  : (none)
*
*/

void init_default_boilerplates(int from_cancel) {

    static int no_of_initializations = 1;

    if (from_cancel) no_of_initializations = 1;

    if (no_of_initializations) {
        no_of_initializations = 0;
        func_init_boilerplate = customize::func_init();
        func_final_boilerplate = customize::func_final();
        before_func_decl_boilerplate = customize:: before_func_decl();
        after_func_decl_boilerplate = customize::after_func_decl();
        before_func_def_boilerplate = customize:: before_func_def();
        after_func_def_boilerplate = customize::after_func_def();
        pub_decl_boilerplate =  customize::pub_decl();
        prot_decl_boilerplate = customize::prot_decl();
        priv_decl_boilerplate = customize::priv_decl();
        before_class_decl_boilerplate = customize::before_class_decl();
        after_class_decl_boilerplate = customize::after_class_decl();
        before_class_def_boilerplate = customize::before_class_def();
        after_class_def_boilerplate = customize::after_class_def();
        before_struct_boilerplate = customize::before_struct();
        after_struct_boilerplate = customize::after_struct();
        member_def_init_boilerplate = customize::member_def_init() ;
        member_def_final_boilerplate = customize::member_def_final();
        before_member_def_boilerplate = customize::before_member_def();
        after_member_def_boilerplate = customize::after_member_def();
        before_member_decl_boilerplate = customize::before_member_decl();
        after_member_decl_boilerplate = customize::after_member_decl();
        relation_src_member_boilerplate = customize::relation_src_member();
        relation_src_header_boilerplate =  customize::relation_src_header();
        relation_src_definition_boilerplate =  customize::relation_src_definition();
        relation_trg_member_boilerplate =  customize::relation_trg_member();
        relation_trg_header_boilerplate =  customize::relation_trg_header();
        relation_trg_definition_boilerplate =  customize::relation_trg_definition();
        c_src_file_top_boilerplate = customize::c_src_file_top();
        c_src_file_bottom_boilerplate = customize::c_src_file_bottom();
        c_hdr_file_top_boilerplate = customize::c_hdr_file_top();
        c_hdr_file_bottom_boilerplate = customize::c_hdr_file_bottom();
        cpp_src_file_top_boilerplate = customize::cpp_src_file_top();
        cpp_src_file_bottom_boilerplate = customize::cpp_src_file_bottom();
        cpp_hdr_file_top_boilerplate = customize::cpp_hdr_file_top();
        cpp_hdr_file_bottom_boilerplate = customize::cpp_hdr_file_bottom();

        for (int i = 0; i < No_of_boilerplates; i++) global_bit_set[i] = 0;
    }
}


/************************************************************************
*  prop_all_changes_to_customize
*  Summary  : Propogate all changes to the customize class (saved defaults)
*  Arguments: (none)
*  Returns  : (none)
*
*/

static void prop_all_changes_to_customize()
{
  Initialize(prop_all_changes_to_customize);

  if(global_bit_set[0]) customize::func_init(func_init_boilerplate);
  if(global_bit_set[1]) customize::func_final(func_final_boilerplate);
  if(global_bit_set[2]) customize::before_func_decl(before_func_decl_boilerplate);
  if(global_bit_set[3]) customize::after_func_decl(after_func_decl_boilerplate);
  if(global_bit_set[4]) customize::before_func_def(before_func_def_boilerplate);
  if(global_bit_set[5]) customize::after_func_def(after_func_def_boilerplate);
  if(global_bit_set[6]) customize::pub_decl(pub_decl_boilerplate);
  if(global_bit_set[7]) customize::prot_decl(prot_decl_boilerplate);
  if(global_bit_set[8]) customize::priv_decl(priv_decl_boilerplate);
  if(global_bit_set[9]) customize::before_class_decl(before_class_decl_boilerplate);
  if(global_bit_set[10]) customize::after_class_decl(after_class_decl_boilerplate);
  if(global_bit_set[11]) customize::before_class_def(before_class_def_boilerplate);
  if(global_bit_set[12]) customize::after_class_def(after_class_def_boilerplate);
  if(global_bit_set[13]) customize::before_struct(before_struct_boilerplate);
  if(global_bit_set[14]) customize::after_struct(after_struct_boilerplate);
  if(global_bit_set[15])  customize::member_def_init(member_def_init_boilerplate);
  if(global_bit_set[16]) customize::member_def_final(member_def_final_boilerplate);
  if(global_bit_set[17]) customize::before_member_def(before_member_def_boilerplate);
  if(global_bit_set[18]) customize::after_member_def(after_member_def_boilerplate);
  if(global_bit_set[19]) customize::before_member_decl(before_member_decl_boilerplate);
  if(global_bit_set[20]) customize::after_member_decl(after_member_decl_boilerplate);
  if(global_bit_set[21]) customize::relation_src_member(relation_src_member_boilerplate);
  if(global_bit_set[22]) customize::relation_src_header(relation_src_header_boilerplate);
  if(global_bit_set[23]) customize::relation_src_definition(relation_src_definition_boilerplate);
  if(global_bit_set[24]) customize::relation_trg_member(relation_trg_member_boilerplate);
  if(global_bit_set[25]) customize::relation_trg_header(relation_trg_header_boilerplate);
  if(global_bit_set[26]) customize::relation_trg_definition(relation_trg_definition_boilerplate);
  if(global_bit_set[27]) customize::c_src_file_top(c_src_file_top_boilerplate);
  if(global_bit_set[28]) customize::c_src_file_bottom(c_src_file_bottom_boilerplate);
  if(global_bit_set[29]) customize::c_hdr_file_top(c_hdr_file_top_boilerplate);
  if(global_bit_set[30]) customize::c_hdr_file_bottom(c_hdr_file_bottom_boilerplate);
  if(global_bit_set[31]) customize::cpp_src_file_top(cpp_src_file_top_boilerplate);
  if(global_bit_set[32]) customize::cpp_src_file_bottom(cpp_src_file_bottom_boilerplate);
  if(global_bit_set[33]) customize::cpp_hdr_file_top(cpp_hdr_file_top_boilerplate);
  if(global_bit_set[34]) customize::cpp_hdr_file_bottom(cpp_hdr_file_bottom_boilerplate);

}  


/************************************************************************
*  prop_specific_change_to_customize
*  Summary  : Propogate a specific boilerplate change to customize.
*  Arguments: an integer that represents the boilerplate that has changed
*  Returns  : (none)
*
*/

static void prop_specific_change_to_customize(int i) {

        switch(i)
        {
        case 0: customize::func_init(func_init_boilerplate);
                 break;
        case 1: customize::func_final(func_final_boilerplate);
                 break;
        case 2: customize::before_func_decl(before_func_decl_boilerplate);
                 break;
        case 3: customize::after_func_decl(after_func_decl_boilerplate);
                 break;
        case 4: customize::before_func_def(before_func_def_boilerplate);
                 break;
        case 5: customize::after_func_def(after_func_def_boilerplate);
                 break;
        case 6: customize::pub_decl(pub_decl_boilerplate);
                 break;
        case 7: customize::prot_decl(prot_decl_boilerplate);
                 break;
        case 8: customize::priv_decl(priv_decl_boilerplate);
                 break;
        case 9: customize::before_class_decl(before_class_decl_boilerplate);
                 break;
        case 10: customize::after_class_decl(after_class_decl_boilerplate);
                 break;
        case 11: customize::before_class_def(before_class_def_boilerplate);
                 break;
        case 12: customize::after_class_def(after_class_def_boilerplate);
                 break;
        case 13: customize::before_struct(before_struct_boilerplate);
                 break;
        case 14: customize::after_struct(after_struct_boilerplate);
                 break;
        case 15: customize::member_def_init(member_def_init_boilerplate) ;
                 break;
        case 16: customize::member_def_final(member_def_final_boilerplate) ;
                 break;
        case 17: customize::before_member_def(before_member_def_boilerplate) ;
                 break;
        case 18: customize::after_member_def(after_member_def_boilerplate) ;
                 break;
        case 19: customize::before_member_decl(before_member_decl_boilerplate);
                 break;
        case 20: customize::after_member_decl(after_member_decl_boilerplate) ;
                 break;
        case 21: customize::relation_src_member(relation_src_member_boilerplate) ;
                 break;
        case 22: customize::relation_src_header(relation_src_header_boilerplate) ;
                 break;
        case 23: customize::relation_src_definition(relation_src_definition_boilerplate) ;
                 break;
        case 24: customize::relation_trg_member(relation_trg_member_boilerplate) ;
                 break;
        case 25: customize::relation_trg_header(relation_trg_header_boilerplate) ;
                 break;
        case 26: customize::relation_trg_definition(relation_trg_definition_boilerplate) ;
                 break;
        case 27: customize::c_src_file_top(c_src_file_top_boilerplate) ;
                 break;
        case 28: customize::c_src_file_bottom(c_src_file_bottom_boilerplate) ;
                 break;
        case 29: customize::c_hdr_file_top(c_hdr_file_top_boilerplate) ;
                 break;
        case 30: customize::c_hdr_file_bottom(c_hdr_file_bottom_boilerplate) ;
                 break;
        case 31: customize::cpp_src_file_top(cpp_src_file_top_boilerplate) ;
                 break;
        case 32: customize::cpp_src_file_bottom(cpp_src_file_bottom_boilerplate) ;
                 break;
        case 33: customize::cpp_hdr_file_top(cpp_hdr_file_top_boilerplate) ;
                 break;
        case 34: customize::cpp_hdr_file_bottom(cpp_hdr_file_bottom_boilerplate) ;
                 break;

        }
}


/************************************************************************
*  Boilerplate::Boilerplate
*  Summary  : Constructor for the Boilerplate class.
*  Arguments: (none)
*  Returns  : (none)
*
*/

Boilerplate::Boilerplate() {
    Initialize(Boilerplate);

    not_in_pdf = boil_share_obtain_proj_info(s1, main_array, phy_path_of_boiler);

    reset_default_bit();
    init_default_boilerplates(0);
    rel_impl_mode = false;
    boilerplate_type = "";
}


/************************************************************************
*  Boilerplate::~Boilerplate
*  Summary  : Destructor for the Boilerplate class.
*  Arguments: (none)
*  Returns  : (none)
*
*/

Boilerplate::~Boilerplate()
{
    instance = NULL;
}


/************************************************************************
*  Boilerplate::refresh_after_delete
*  Summary  : Refreshes the list of boilerplates after a delete happens.
*  Arguments: (none)
*  Returns  : (none)
*
*/

void Boilerplate::refresh_after_delete() {
   Initialize(boilerplate::refresh_after_delete);

   if (instance) {
      instance->s1.removeAll();
      instance->main_array.reset();
      boil_share_obtain_proj_info(instance->s1, 
                                  instance->main_array,
                                  phy_path_of_boiler);

      for (int i = 0; i < No_of_boilerplates; i++) {
         if (strcmp(instance->boilerplate_type, func_array[i]) == 0) {
            load_bplist (instance->boilerplate_type);

            genString command;
            command.printf ("dis_gdDialogIDEval {%s} {BPBoilerplateSet}", instance->dialog.str());
            rcall_dis_DISui_eval_async (DISui_app, (vstr *)command.str());
            break;
         }
      }
   }
}

/************************************************************************
*  Boilerplate::initialize
*  Summary  : Initializes the boilerplates by creating the instance member.
*  Arguments: (none)
*  Returns  : (none)
*
*/

void Boilerplate::initialize (const char *cdialog) {

   if (!instance) {
      instance = new Boilerplate;
      instance->dialog = cdialog;
   }
}


/************************************************************************
*  Boilerplate::close
*  Summary  : Closes the boilerplate by destroying the instance menber.
*  Arguments: (none)
*  Returns  : (none)
*
*/

void Boilerplate::close () {
    if (instance) {
        delete instance;
        instance = NULL;
    }
}


/************************************************************************
*  Boilerplate::load_bplist
*  Summary  : Loads the boilerplate list.  This is the list of boilerplates
*             associated with passed in boilerplate type name.
*  Arguments: BPTypename - The boilerplate type name.
*  Returns  : (none)
*
*/

void Boilerplate::load_bplist (char *BPTypename)
{

    if (instance) {
        if (strcmp (BPTypename, "Entire Implementation") != 0) {
          instance->rel_impl_mode = false;
          instance->boilerplate_type = BPTypename;
        }
        else {
          instance->rel_impl_mode = true;
          instance->boilerplate_type = "";
        }
   
        instance->init_type_array();
        instance->temp_array.reset();
        instance->init_sorted_array(instance->temp_array);
    }
}


/************************************************************************
*  Boilerplate::GetInstanceBpCount
*  Summary  : calculates the number of boilerplates associated with the
*             current boilerplate type.
*  Arguments: (none)
*  Returns  : The number of boilerplates.
*
*/

int Boilerplate::GetInstanceBpCount () {
    if (instance)
        return instance->type_array.size();
    else 
        return 0;
}


/************************************************************************
*  Boilerplate::GetInstanceBoilerplate
*  Summary  : returns the name of the boilerplate refered to by the passed
*             in index and the current boilerplate type.
*  Arguments: (none)
*  Returns  : The name of the boilerplate.
*
*/

const char *Boilerplate::GetInstanceBoilerplate (int index) {
    if (instance)
        return instance->temp_array[index];
    else 
        return NULL;
}


/************************************************************************
*  Boilerplate::DeleteInstanceBoilerplate
*  Summary  : Deletes the boilerplate specified by index
*  Arguments: The index in the user-interface of the boilerplate to delete.
*  Returns  : An error string or the empty string
*
*/

void Boilerplate::DeleteInstanceBoilerplate (int index) {

    if (!instance) {
        genString command;
        command.printf ("dis_prompt {OKDialog} {Create} {Boilerplate application must be activated}");
        rcall_dis_DISui_eval_async (DISui_app, (vstr *)command.str());
    }

    else {
        instance->delete_boilerplate (instance->temp_array[index]);
    }
}


/************************************************************************
*  Boilerplate::NewInstanceBoilerplate
*  Summary  : Creates a new boilerplate of the given name
*  Arguments: The name of the new boilerplate
*  Returns  : An error string or the empty string
*
*/
        
void Boilerplate::NewInstanceBoilerplate (const char *name) {

    const char *message;
    if (!instance) message = "Boilerplate application must be activated";
    else message = instance->new_boilerplate (name);

    if (message) {
        genString command;
        command.printf ("dis_prompt {OKDialog} {Create} {%s}", message);
        rcall_dis_DISui_eval_async (DISui_app, (vstr *)command.str());
    }

    else {
        genString command;
        command.printf ("dis_gdDialogIDEval {%s} {BPBoilerplateSet}", instance->dialog.str());
        rcall_dis_DISui_eval_async (DISui_app, (vstr *)command.str());
    }
}


/************************************************************************
*  Boilerplate::GetInstanceBoilerplateType
*  Summary  : returns the type name of the current boilerplate type.
*  Arguments: (none)
*  Returns  : The name of the boilerplate type.
*
*/

const char *Boilerplate::GetInstanceBoilerplateType () {
    if (instance)
        return instance->boilerplate_type.str();
    else 
        return NULL;
}


/************************************************************************
*  Boilerplate::GetInstanceDefault
*  Summary  : returns the name of the default boilerplate associated with
*             the current boilerplate type.
*  Arguments: (none)
*  Returns  : The name of the default boilerplate.
*
*/

const char *Boilerplate::GetInstanceDefault () {

    genString default_type;

    if (instance) {
        instance->get_boilerplate_default (default_type);

        const char* pc = strrchr (default_type.str(), '!');
        if (!pc)
    	    pc = default_type.str();
        else pc += 1;

        return pc;
    }

    else {
        return NULL;
    }

}


/************************************************************************
*  Boilerplate::SetInstanceDefault
*  Summary  : Sets the default boilerplate for the current boilerplate type.
*             This can be either a global or local default.
*  Arguments: the index of the new default boilerplate in the temp_array (reflects the display).
*             1 for global, 0 for local
*  Returns  : (none)
*
*/

void Boilerplate::SetInstanceDefault (int index, int global_bit) {

    if (instance) instance->set_boilerplate_default (instance->temp_array[index], global_bit);
}


/************************************************************************
*  Boilerplate::RemoveInstanceDefault
*  Summary  : Removes the default boilerplate for the current boilerplate type.
*  Arguments: (none)
*  Returns  : (none)
*
*/

void Boilerplate::RemoveInstanceDefault () {

    if (instance) instance->set_boilerplate_default ("");
}


/************************************************************************
*  Boilerplate::SaveInstanceDefaults
*  Summary  : Saves the default boilerplates in the customize class.
*  Arguments: (none)
*  Returns  : (none)
*
*/

void Boilerplate::SaveInstanceDefaults () {

    if (instance) instance->save_defaults();
}


/************************************************************************
*  Boilerplate::RestoreInstanceDefaults
*  Summary  : Restore the default boilerplates from those saved in the customize class.
*  Arguments: (none)
*  Returns  : (none)
*
*/

void Boilerplate::RestoreInstanceDefaults () {

    if (instance) instance->restore_defaults ();
}


/************************************************************************
*  Boilerplate::init_type_array
*  Summary  : initializes class members type_array and intermediate_array 
*  Arguments: (none)
*  Returns  : (none)
*
*/

void Boilerplate::init_type_array ()
{
    Initialize (Boilerplate::init_type_array);

    const char*	pc;
    projModule* module;
    genString	phys, temp;
    GenArr	search_specs;
    int		i;

    type_array.reset();
    intermediate_array.reset();

    if (rel_impl_mode) {
	for (i = 0; i < NUM_REL_BOILS; i++) {
	    temp = rel_boil_names[i];
	    temp += '!';
	    search_specs.append ((char*) temp.str ());
	}
    }
    else {
        temp = boilerplate_type;
        temp += '!';
	search_specs.append ((char*) temp.str ());
    }

    for (int j = 0; j < s1.size (); j++) {
	if ((module = checked_cast (projModule, s1[j])) == NULL)
	    continue;
	module->get_phys_filename (phys);
	if (phys[phys.length () - 1] == '~')
	    continue;
	for (i = 0; i < search_specs.size (); i++) {
	    if (strstr ((char*) phys.str (), search_specs[i])) {   
	        pc = add_type_array (module);
		if (rel_impl_mode)
		    intermediate_array.append ((char*) pc);
                else intermediate_array.append ((char*) phys.str ()); 
	        break;
	    }
	}
    }
}


/************************************************************************
*  Boilerplate::add_type_array
*  Summary  : adds a boilerplate to the type_array given a module.
*  Arguments: the module
*  Returns  : (none)
*
*/

const char* Boilerplate::add_type_array (projModule* module)
{
    Initialize (Boilerplate::add_type_array);

    if (!module)
	return NULL;
    
    const char* ln = module->get_name ();
    if (!ln)
	return NULL;

    const char* pc = strrchr (ln, '!');
    if (!pc)
	pc = ln;
    else pc += 1;

    if (type_array.search ((char*) pc) < 0)
        type_array.append ((char*) pc);

    return pc;
}


/************************************************************************
*  Boilerplate::init_sorted_array
*  Summary  : initializes a sorted array of boilerplates for the current boilerplate type.
*  Arguments: the sorted array of boilerplates.
*  Returns  : (none)
*
*/

void Boilerplate::init_sorted_array(GenArr &temp_array)
{
 
     int num_of_items = type_array.size();   
     for(int j=0;j<num_of_items;j++)
	temp_array.append(type_array[j]);
     temp_array.sort();
}


/************************************************************************
*  Boilerplate::get_boilerplate_default
*  Summary  :  Returns the default boilerplate for the given type.
*  Arguments: 
*  Returns  : (none)
*
*/

int Boilerplate::get_boilerplate_default (genString& default_type)
{
    int     i, j;
    char    *s1, *s2, *p1, *p2;

    if (rel_impl_mode) {
        default_type = "";
        p1 = NULL;
        for (i = j = 0; i < NUM_REL_BOILS; i++) {
            s1 = (char*) rel_boils[j]->str ();
            s2 = (char*) rel_boils[i]->str ();
            if ((s1 == NULL) || (s2 == NULL))
                continue;
            if ((p1 = strrchr (s1, '!')) == NULL)
                p1 = s1;
            else p1 += 1;
            if ((p2 = strrchr (s2, '!')) == NULL)
                p2 = s2;
            else p2 += 1;
            if (strcmp (p1, p2) != 0) {
                if (strlen (p1) < 1)
                    j = i;
                else if (strlen (p2) > 0)
                    return -1;
            }
        }
        default_type = p1;
        return -1;
    }

    for(i = 0; i < No_of_boilerplates; i++)
      if(boilerplate_type == func_array[i])
          break;

      switch (i) {

        case 0 :default_type = func_init_boilerplate;
                break;
        case 1 :default_type = func_final_boilerplate;
                break;
        case 2 :default_type = before_func_decl_boilerplate;
                break;
        case 3 :default_type = after_func_decl_boilerplate;
                break;
        case 4 :default_type = before_func_def_boilerplate;
                break;
        case 5 :default_type = after_func_def_boilerplate;
                break;
        case 6 :default_type = pub_decl_boilerplate;
                break;
        case 7 :default_type = prot_decl_boilerplate;
                break;
        case 8 :default_type = priv_decl_boilerplate;
                break;
        case 9 :default_type = before_class_decl_boilerplate;
                break;
        case 10 :default_type = after_class_decl_boilerplate;
                break;
        case 11 :default_type = before_class_def_boilerplate;
                break;
        case 12 :default_type = after_class_def_boilerplate;
                break;
        case 13 :default_type = before_struct_boilerplate;
                break;
        case 14 :default_type = after_struct_boilerplate;
                break;
        case 15 :default_type = member_def_init_boilerplate;
                break;
        case 16 :default_type = member_def_final_boilerplate;
                break;
        case 17 :default_type = before_member_def_boilerplate;
                break;
        case 18 :default_type = after_member_def_boilerplate;
                break;
        case 19 :default_type = before_member_decl_boilerplate;
                break;
        case 20 :default_type = after_member_decl_boilerplate;
                break;
        case 21 :default_type = relation_src_member_boilerplate;
                break;
        case 22 :default_type = relation_src_header_boilerplate;
                break;
        case 23 :default_type = relation_src_definition_boilerplate;
                break;
        case 24 :default_type = relation_trg_member_boilerplate;
                break;
        case 25 :default_type = relation_trg_header_boilerplate;
                break;
        case 26 :default_type = relation_trg_definition_boilerplate;
                break;
        case 27 :default_type = c_src_file_top_boilerplate;
                break;
        case 28 :default_type = c_src_file_bottom_boilerplate;
                break;
        case 29 :default_type = c_hdr_file_top_boilerplate;
                break;
        case 30 :default_type = c_hdr_file_bottom_boilerplate;
                break;
        case 31 :default_type = cpp_src_file_top_boilerplate;
                break;
        case 32 :default_type = cpp_src_file_bottom_boilerplate;
                break;
        case 33 :default_type = cpp_hdr_file_top_boilerplate;
                break;
        case 34 :default_type = cpp_hdr_file_bottom_boilerplate;
                break;

        }
    return i;
}


/************************************************************************
*  Boilerplate::set_boilerplate_default
*  Summary  :  Sets the default boilerplate for the current type.
*  Arguments:  The new default boilerplate.
*  Returns  : (none)
*
*/

int Boilerplate::set_boilerplate_default (const char* name) {

    int             i;
    genString       temp;

    if (rel_impl_mode) {
        for (i = 0; i < NUM_REL_BOILS; i++) {
            *rel_boils[i] = "";
            if ((name == NULL) || (*name == '\0'))
                continue;
            temp.printf ("%s/%s!%s", phy_path_of_boiler.str (), rel_boil_names[i], name);
            if (main_array.search ((char*) temp.str ()) >= 0)
                *rel_boils[i] = temp;
            }
            return -1;
    }

    i = default_index();
         
    switch (i) {

      case 0 :func_init_boilerplate = name;
              break;
      case 1 :func_final_boilerplate = name;
              break;
      case 2 :before_func_decl_boilerplate = name;
              break;
      case 3 :after_func_decl_boilerplate = name;
              break;
      case 4 :before_func_def_boilerplate = name;
              break;
      case 5 :after_func_def_boilerplate = name;
              break;
      case 6 :pub_decl_boilerplate = name;
              break;
      case 7 :prot_decl_boilerplate = name;
              break;
      case 8 :priv_decl_boilerplate = name;
              break;
      case 9 :before_class_decl_boilerplate = name;
              break;
      case 10 :after_class_decl_boilerplate = name;
              break;
      case 11 :before_class_def_boilerplate = name;
              break;
      case 12 :after_class_def_boilerplate = name;
              break;
      case 13 :before_struct_boilerplate = name;
              break;
      case 14 :after_struct_boilerplate = name;
              break;
      case 15 :member_def_init_boilerplate = name;
              break;
      case 16 :member_def_final_boilerplate = name;
              break;
      case 17 :before_member_def_boilerplate = name;
              break;
      case 18 :after_member_def_boilerplate = name;
              break;
      case 19 :before_member_decl_boilerplate = name;
              break;
      case 20 :after_member_decl_boilerplate = name;
              break;
      case 21 :relation_src_member_boilerplate = name;
              break;
      case 22 :relation_src_header_boilerplate = name;
              break;
      case 23 :relation_src_definition_boilerplate = name;
              break;
      case 24 :relation_trg_member_boilerplate = name;
              break;
      case 25 :relation_trg_header_boilerplate = name;
              break;
      case 26 :relation_trg_definition_boilerplate = name;
              break;
      case 27 :c_src_file_top_boilerplate = name;
              break;
      case 28 :c_src_file_bottom_boilerplate = name;
              break;
      case 29 :c_hdr_file_top_boilerplate = name;
              break;
      case 30 :c_hdr_file_bottom_boilerplate = name;
              break;
      case 31 :cpp_src_file_top_boilerplate = name;
              break;
      case 32 :cpp_src_file_bottom_boilerplate = name;
              break;
      case 33 :cpp_hdr_file_top_boilerplate = name;
              break;
      case 34 :cpp_hdr_file_bottom_boilerplate = name;
              break;
    }

    return i;
}


/************************************************************************
*  Boilerplate::set_boilerplate_default
*  Summary  :  Sets the default boilerplate (either global or local) 
*              for the current type.
*  Arguments:  The new default boilerplate.
*              a global/local switch
*  Returns  : (none)
*
*/

void Boilerplate::set_boilerplate_default (char *bpName, int global_bit) {

    char *string = OSapi_strdup(bpName);
    int index = type_array.search(string);

    char *name = (char *) intermediate_array[index];
    set_boilerplate_default(name);
    set_default_bit();
    global_bit_set[default_index()] = global_bit;
}


/************************************************************************
*  Boilerplate::default_index
*  Summary  :  Returns the index into the func_array for the current boilerplate type.
*  Arguments: (none)
*  Returns  : The index into the func_array.
*
*/

int Boilerplate::default_index()
{
    for (int i = 0; i < No_of_boilerplates; i++)
        if (boilerplate_type == func_array[i]) break;

    return i;
}


/************************************************************************
*  Boilerplate::save_defaults
*  Summary  :  Saves the default boilerplates.
*  Arguments: (none)
*  Returns  : (none)
*
*/

void Boilerplate::save_defaults () {

    if(is_default_bit_set()) {
        prop_all_changes_to_customize();
        customize::save_boilerplate();
    }
}


/************************************************************************
*  Boilerplate::restore_defaults
*  Summary  :  Restores the default boilerplates.
*  Arguments: (none)
*  Returns  : (none)
*
*/

void Boilerplate::restore_defaults () {

    init_default_boilerplates(1);
}


/************************************************************************
*  Boilerplate::delete_boilerplate
*  Summary  :  deletes the named boilerplate for in the current boilerplate type.
*  Arguments: The boilerplate to delete.
*  Returns  : (none)
*
*/

void Boilerplate::delete_boilerplate (char *bpName) {

    char *string = OSapi_strdup(bpName);
    int index = type_array.search(string);

    int k;
    char *name = (char *) OSapi_strdup(intermediate_array[index]);
    projNode* proj_node = projHeader::fn_to_project(name);
    projNode* home_node = projNode::get_home_proj();
    if(proj_node != home_node) {
        genString command;
        command.printf ("dis_prompt {OKDialog} {Create} {The selected file is not in the home project}");
        rcall_dis_DISui_eval_async (DISui_app, (vstr *)command.str());
    }

    else if ((k = delete_path (name)) != 1) {
        genString command;
        command.printf ("dis_prompt {OKDialog} {Create} {Error in removing boilerplate}");
        rcall_dis_DISui_eval_async (DISui_app, (vstr *)command.str());
    }

    else {
        intermediate_array.del(index);
        type_array.del(index);
        int sym_index = main_array.search(name);
        main_array.del(name);
        s1.remove_index(sym_index);
        genString default_type;
        int type_index = get_boilerplate_default(default_type);

        if (default_type != (const char *)"" && default_type == name ) {
            set_boilerplate_default("");
            prop_specific_change_to_customize(type_index);
            customize::save_specific_boilerplate(type_index, func_array[type_index]);
        }

        genString command;
        command.printf ("dis_gdDialogIDEval {%s} {BPBoilerplateSet}", dialog.str());
        rcall_dis_DISui_eval_async (DISui_app, (vstr *)command.str());
    }
}


/************************************************************************
*  Boilerplate::new_boilerplate
*  Summary  :  Creates a new boilerplate with the given name.
*  Arguments: The name of the new boilerplate.
*  Returns  : (none)
*
*/

const char *Boilerplate::new_boilerplate (const char *bpName) {

    if (not_in_pdf) {
       return "Must have .boilerplate subproject in home project to create";
    }

    if (boilerplate_type.length() == 0) {
       return "Must select a boilerplate type before creating";
    }

    genString phy_name_of_file;

    if (!construct_name (bpName, phy_name_of_file)) {
        return "Incorrect format for filename";
    }

    if (main_array.search (phy_name_of_file) >= 0) {
        return "File already exists";
    }

    if (makeUNIXFile (phy_name_of_file) != 0) {
        return "Error in creating boilerplate file";
    }

    projNode* home_proj = projNode::get_home_proj ();
    projModule* module;
    if (home_proj->is_script ()) {
        projNode::create_global_script_modules (phy_name_of_file, FILE_LANGUAGE_UNKNOWN, 0);
        genString ln;
        home_proj->fn_to_ln (phy_name_of_file, ln, FILE_TYPE, 0, 1);
        module = home_proj->find_module (ln);
    }

    else {
        module = home_proj->make_new_module (phy_name_of_file);
    }

    if (!module) {
        return "Internal error in Paraset, Save files";
    }

    else {
        module->update_module ();
        module->language (FILE_LANGUAGE_RAW);
        main_array.append (phy_name_of_file);
        s1.insert_last (module);
        intermediate_array.append ((char *)phy_name_of_file.str());
        return NULL;
    }
}


/************************************************************************
*  Boilerplate::construct_name
*  Summary  :  Constructs the filename for a boilerplate file.
*  Arguments: The name of the boilerplate.
*             The name of the boilerplate file.
*  Returns  : (none)
*
*/

bool Boilerplate::construct_name (const char* base, genString& name) {

    if (!base || !*base)
        return false;

    name = phy_path_of_boiler;
    name += '/';
    name += boilerplate_type;
    name += '!';

    int c;
    for (int i = 0; c = base[i]; i++) {
        if (!isprint (c) || isspace (c) || strchr ("/!", c))
            c = '_';
        name += c;
    }

    return true;
}


genString Boilerplate::filter_boilerplate (genString original_string) {
    Initialize(Boilerplate::parse_boilerplate);
   
    if (original_string != (char*) NULL) {
        genString outstring;
        outstring = "";
        int k = original_string.length();
        int i = 0;

        // keep less than the actual string itself 
        while (i < k-1)  {
            if (original_string[i] == '$') {
                switch (original_string[i+1]){
                  case 'C':
                    outstring += dd_class_name;
                    i += 2;    // Increment i by 2 because two charcters have been accounted for
                    break;
                 
                  case 'F':
                    outstring += dd_function_name;
                    i += 2;    // Increment i by 2 because two charcters have been accounted for
                    break;

                  case 'P':
                    outstring += dd_physical_file_name;
                    i += 2;    // Increment i by 2 because two charcters have been accounted for
                    break;

                  case 'L':
                    outstring += dd_logical_file_name;
                    i += 2;    // Increment i by 2 because two charcters have been accounted for
                    break;

                  case 'S':
                    outstring += dd_source_name;
                    i += 2;    // Increment i by 2 because two charcters have been accounted for
                    break;

                  case 'T':
                    outstring += dd_target_name;
                    i += 2;    // Increment i by 2 because two charcters have been accounted for
                    break;

                  case 'A':
                    outstring += dd_author_name;
                    i += 2;    // Increment i by 2 because two charcters have been accounted for
                    break;

                  case 'U':
                    outstring += dd_user_name;
                    i += 2;    // Increment i by 2 because two charcters have been accounted for
                    break;

                  case 'D':
                    outstring += dd_Date;
                    i += 2;    // Increment i by 2 because two charcters have been accounted for
                    break;

                  case 'R':
                    outstring += dd_project_name;
                    i += 2;    // Increment i by 2 because two characters have been accounted for
                    break;

                  case 's':
                    outstring += dd_struct_name;
                    i += 2;    // Increment i by 2 because two characters have been accounted for
                    break;

                  case 'm':
                    outstring += dd_member_name;
                    i += 2;    // Increment i by 2 because two characters have been accounted for
                    break;

                  case '$':
                    outstring += "$";
                    i += 2;
                    break;

                  default:
                    outstring += original_string[i];
                    i++;
                    break;
                }
            }

            else {
                outstring += original_string[i];
                i++;
            }
        }

        if (k) outstring += original_string[k-1];

        return outstring;

    }

    else return NULL;
}


void set_class_name (genString name) {
    Initialize (set_dd_class_name);
    dd_class_name = name;
}


void set_function_name (genString name) {
    Initialize (set_dd_function_name);
    dd_function_name = name;
}


void set_struct_name (genString name) {
    Initialize (set_dd_struct_name);
    dd_struct_name = name;
}


void set_member_name (genString name) {
    Initialize (set_dd_member_name);
    dd_member_name = name;
}


void set_physical_file_name (genString name) {
    Initialize (set_dd_physical_file_name);
    dd_physical_file_name = name;
}


void set_logical_file_name (genString name) {
    Initialize (set_dd_logical_file_name);
    dd_logical_file_name = name;
}


void set_source_name (genString name) {
    Initialize (set_dd_source_name);
    dd_source_name = name;
}


void set_target_name (genString name) {
    Initialize (set_dd_target_name);
    dd_target_name = name;
}


void set_author_name (genString name) {
    Initialize (set_dd_author_name);
    dd_author_name = name;
}


void set_user_name (genString name) {
    Initialize (set_dd_user_name);
    dd_user_name = name;
}


void set_Date (genString name) {
   Initialize (set_dd_Date);
   dd_Date = name;
   char* nlp = strchr(dd_Date, '\n');
   if (nlp) {
      *nlp = 0;         // don\'t include CR
   }
}


void set_project_name (genString name) {
    Initialize (set_dd_project_name);
    dd_project_name = name;
}


genString Boilerplate::insert_boilerplate (genString name) {
    Initialize(Boilerplate::insert_boilerplate);
    int i;

    genString original_string;

    original_string = "";
    char temp_string[513];
    if((name == (const char *)"") || (name == (char*)NULL)) return ("");
 
    FILE *fd = fopen((char *)name,"r");
    if(fd == NULL) return "";
    while ((i = fread (temp_string, sizeof(char), 512, fd)) > 0) {
        temp_string[i] = '\0';
        original_string += temp_string;
    }

    fclose (fd);
    return (char *)original_string.str();
}


genString Boilerplate::insert_func_init() {
    Initialize (Boilerplate::insert_func_init);
    init_default_boilerplates(0);

    return Boilerplate::insert_boilerplate(func_init_boilerplate);
}


genString Boilerplate::insert_func_final() {
    Initialize (Boilerplate::insert_func_init);

    init_default_boilerplates(0);
    return Boilerplate::insert_boilerplate(func_final_boilerplate);
}


genString Boilerplate::insert_before_func_decl() {
    Initialize (Boilerplate::insert_before_func_decl);

    init_default_boilerplates(0);
    return Boilerplate::insert_boilerplate(before_func_decl_boilerplate);
}


genString Boilerplate::insert_after_func_decl() {
    Initialize (Boilerplate::insert_after_func_decl);

    init_default_boilerplates(0);
    return Boilerplate::insert_boilerplate(after_func_decl_boilerplate);
}


genString Boilerplate::insert_before_func_def() {
    Initialize (Boilerplate::insert_before_func_def);

    init_default_boilerplates(0);
    return Boilerplate::insert_boilerplate(before_func_def_boilerplate);
}


genString Boilerplate::insert_after_func_def() {
    Initialize (Boilerplate::insert_after_func_def);

    init_default_boilerplates(0);
    return Boilerplate::insert_boilerplate(after_func_def_boilerplate);
}


genString Boilerplate::insert_pub_decl() {
    Initialize (Boilerplate::insert_pub_decl);

    init_default_boilerplates(0);
    return Boilerplate::insert_boilerplate(pub_decl_boilerplate);
}


genString Boilerplate::insert_priv_decl() {
    Initialize (Boilerplate::insert_priv_decl);

    init_default_boilerplates(0);
    return Boilerplate::insert_boilerplate(priv_decl_boilerplate);
}


genString Boilerplate::insert_prot_decl() {
    Initialize (Boilerplate::insert_prot_decl);

    init_default_boilerplates(0);
    return Boilerplate::insert_boilerplate(prot_decl_boilerplate);
}


genString Boilerplate::insert_before_class_decl() {
    Initialize (Boilerplate::insert_before_class_decl);
    init_default_boilerplates(0);
    return Boilerplate::insert_boilerplate(before_class_decl_boilerplate);
}


genString Boilerplate::insert_after_class_decl() {
    Initialize (Boilerplate::insert_after_class_decl);

    init_default_boilerplates(0);
    return Boilerplate::insert_boilerplate(after_class_decl_boilerplate);
}


genString Boilerplate::insert_before_class_def() {
    Initialize (Boilerplate::insert_before_class_def);

    init_default_boilerplates(0);
    return Boilerplate::insert_boilerplate(before_class_def_boilerplate);
}


genString Boilerplate::insert_after_class_def() {
    Initialize (Boilerplate::insert_after_class_def);

    init_default_boilerplates(0);
    return Boilerplate::insert_boilerplate(after_class_def_boilerplate);
}


genString Boilerplate::insert_before_struct() {
    Initialize (Boilerplate::insert_before_struct);

    init_default_boilerplates(0);
    return Boilerplate::insert_boilerplate(before_struct_boilerplate);
}


genString Boilerplate::insert_after_struct() {
    Initialize (Boilerplate::insert_after_struct);

    init_default_boilerplates(0);
    return Boilerplate::insert_boilerplate(after_struct_boilerplate);
}


genString Boilerplate::insert_member_def_init() {
    Initialize (Boilerplate::insert_member_def_init);

    init_default_boilerplates(0);
    return Boilerplate::insert_boilerplate(member_def_init_boilerplate);
}


genString Boilerplate::insert_member_def_final() {
    Initialize (Boilerplate::insert_member_def_final);

    init_default_boilerplates(0);
    return Boilerplate::insert_boilerplate(member_def_final_boilerplate);
}


genString Boilerplate::insert_before_member_def() {
    Initialize (Boilerplate::insert_before_member_def);

    init_default_boilerplates(0);
    return Boilerplate::insert_boilerplate(before_member_def_boilerplate);
}


genString Boilerplate::insert_after_member_def() {
    Initialize (Boilerplate::insert_after_member_def);

    init_default_boilerplates(0);
    return Boilerplate::insert_boilerplate(after_member_def_boilerplate);
}


genString Boilerplate::insert_before_member_decl() {
    Initialize (Boilerplate::insert_before_member_decl);

    init_default_boilerplates(0);
    return Boilerplate::insert_boilerplate(before_member_decl_boilerplate);
}


genString Boilerplate::insert_after_member_decl() {
    Initialize (Boilerplate::insert_after_member_decl);

    init_default_boilerplates(0);
    return Boilerplate::insert_boilerplate(after_member_decl_boilerplate);
}


genString Boilerplate::insert_relation_src_member() {
    Initialize (Boilerplate::insert_relation_src_member);

    init_default_boilerplates(0);
    return Boilerplate::insert_boilerplate(relation_src_member_boilerplate);
}


genString Boilerplate::insert_relation_src_header() {
    Initialize (Boilerplate::insert_relation_src_header);

    init_default_boilerplates(0);
    return Boilerplate::insert_boilerplate(relation_src_header_boilerplate);
}


genString Boilerplate::insert_relation_src_definition() {
    Initialize (Boilerplate::insert_relation_src_definition);

    init_default_boilerplates(0);
    return Boilerplate::insert_boilerplate(relation_src_definition_boilerplate);
}

genString Boilerplate::insert_relation_trg_member() {
    Initialize (Boilerplate::insert_relation_trg_member);

    init_default_boilerplates(0);
    return Boilerplate::insert_boilerplate(relation_trg_member_boilerplate);
}


genString Boilerplate::insert_relation_trg_header() {
    Initialize (Boilerplate::insert_relation_trg_header);

    init_default_boilerplates(0);
    return Boilerplate::insert_boilerplate(relation_trg_header_boilerplate);
}


genString Boilerplate::insert_relation_trg_definition() {
    Initialize (Boilerplate::insert_relation_trg_definition);

    init_default_boilerplates(0);
    return Boilerplate::insert_boilerplate(relation_trg_definition_boilerplate);
}


genString Boilerplate::insert_c_src_file_top() {
    Initialize (Boilerplate::insert_c_src_file_top);

    init_default_boilerplates(0);
    return Boilerplate::insert_boilerplate(c_src_file_top_boilerplate);
}


genString Boilerplate::insert_c_src_file_bottom() {
    Initialize (Boilerplate::insert_c_src_file_bottom);

    init_default_boilerplates(0);
    return Boilerplate::insert_boilerplate(c_src_file_bottom_boilerplate);
}


genString Boilerplate::insert_c_hdr_file_top() {
    Initialize (Boilerplate::insert_c_hdr_file_top);

    init_default_boilerplates(0);
    return Boilerplate::insert_boilerplate(c_hdr_file_top_boilerplate);
}


genString Boilerplate::insert_c_hdr_file_bottom() {
    Initialize (Boilerplate::insert_c_hdr_file_bottom);

    init_default_boilerplates(0);
    return Boilerplate::insert_boilerplate(c_hdr_file_bottom_boilerplate);
}


genString Boilerplate::insert_cpp_src_file_top() {
    Initialize (Boilerplate::insert_cpp_src_file_top);

    init_default_boilerplates(0);
    return Boilerplate::insert_boilerplate(cpp_src_file_top_boilerplate);
}


genString Boilerplate::insert_cpp_src_file_bottom() {
    Initialize (Boilerplate::insert_cpp_src_file_bottom);

    init_default_boilerplates(0);
    return Boilerplate::insert_boilerplate(cpp_src_file_bottom_boilerplate);
}


genString Boilerplate::insert_cpp_hdr_file_top() {
    Initialize (Boilerplate::insert_cpp_hdr_file_top);

    init_default_boilerplates(0);
    return Boilerplate::insert_boilerplate(cpp_hdr_file_top_boilerplate);
}


genString Boilerplate::insert_cpp_hdr_file_bottom() {
    Initialize (Boilerplate::insert_cpp_hdr_file_bottom);

    init_default_boilerplates(0);
    return Boilerplate::insert_boilerplate(cpp_hdr_file_bottom_boilerplate);
}
