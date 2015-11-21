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
#ifndef _ddL_boil_h
#define _ddL_boil_h

// dd_boil.h
//------------------------------------------
// synopsis:
// 
// Dialog box summoned from boilerplate option in the utility submenu of the browser main menu.
//------------------------------------------
//#include <gtGenArr.h>
#include <genString.h>
#include <symbolArr.h>

#ifndef _gt_h
#include <gt.h>
#endif

#define No_of_dd_boilerplates 35

RelClass(projModule);
class gtRTL;
class browserShell;


enum DD_boil{
    DD_func,
    DD_class,
    DD_struct,
    DD_member,
    DD_relation
};

class dd_boil
{
    static dd_boil* instance;
    int not_in_pdf;
    int desens_type;
    bool rel_impl_mode;
    gtDialogTemplate *shell;
    gtLabel* status_label;
   
    gtOptionMenu *boilerplate_menu;
    gtBase* gt_base;

    gtList *gt_list;
    gtScrolledWindow *gt_scrolled;
    gtList *domain_list;
    gtLabel *type_prompt;
    //    GenArr  main_array;
    //    GenArr  intermediate_array;
    //    GenArr type_array; 
    genString home_proj_name;
    genString user_name;
    genString boilerplate_type;
    symbolArr s1;
    int default_bit ;
    void popup();


    public:
     dd_boil(gtBase*,int type);
    ~dd_boil();
    static void refresh_after_delete();


    static void OK_CB(gtPushButton *,gtEvent *,void *cd,gtReason);
    static void Cancel_CB(gtPushButton *,gtEvent *,void *cd,gtReason);

    static void pop_dd_boilerplate(DD_boil type, void *cd);

    static void opt_set_local_def_CB(gtPushButton*, gtEvent*, void* cd, gtReason);
    static void opt_set_global_def_CB(gtPushButton*, gtEvent*, void* cd, gtReason);
    static void opt_show_def_CB(gtPushButton*, gtEvent*, void* cd, gtReason);
    static void opt_rem_def_CB(gtPushButton*, gtEvent*, void* cd, gtReason);
    static void func_init_CB(gtOptionMenu*, gtEvent*, void* cd, gtReason);
    static void func_final_CB(gtOptionMenu*, gtEvent*, void* cd, gtReason);
    static void before_func_decl_CB(gtOptionMenu*, gtEvent*, void* cd, gtReason);
    static void after_func_decl_CB(gtOptionMenu*, gtEvent*, void* cd, gtReason);
    static void before_func_def_CB(gtOptionMenu*, gtEvent*, void* cd, gtReason);
    static void after_func_def_CB(gtOptionMenu*, gtEvent*, void* cd, gtReason);
    static void pub_decl_CB(gtOptionMenu*, gtEvent*, void* cd, gtReason);
    static void prot_decl_CB(gtOptionMenu*, gtEvent*, void* cd, gtReason);
    static void priv_decl_CB(gtOptionMenu*, gtEvent*, void* cd, gtReason);
    static void before_class_decl_CB(gtOptionMenu*, gtEvent*, void* cd, gtReason);
    static void after_class_decl_CB(gtOptionMenu*, gtEvent*, void* cd, gtReason);
    static void before_class_def_CB(gtOptionMenu*, gtEvent*, void* cd, gtReason);
    static void after_class_def_CB(gtOptionMenu*, gtEvent*, void* cd, gtReason);
    static void before_struct_CB(gtOptionMenu*, gtEvent*, void* cd, gtReason);
    static void after_struct_CB(gtOptionMenu*, gtEvent*, void* cd, gtReason);
    static void member_def_init_CB(gtOptionMenu*, gtEvent*, void* cd, gtReason);
    static void member_def_final_CB(gtOptionMenu*, gtEvent*, void* cd, gtReason);
    static void before_member_def_CB(gtOptionMenu*, gtEvent*, void* cd, gtReason);
    static void after_member_def_CB(gtOptionMenu*, gtEvent*, void* cd, gtReason);
    static void before_member_decl_CB(gtOptionMenu*, gtEvent*, void* cd, gtReason);
    static void after_member_decl_CB(gtOptionMenu*, gtEvent*, void* cd, gtReason);
    static void relation_src_member_CB(gtOptionMenu*, gtEvent*, void* cd, gtReason);
    static void relation_src_header_CB(gtOptionMenu*, gtEvent*, void* cd, gtReason);
    static void relation_src_definition_CB(gtOptionMenu*, gtEvent*, void* cd, gtReason);
    static void relation_trg_member_CB(gtOptionMenu*, gtEvent*, void* cd, gtReason);
    static void relation_trg_header_CB(gtOptionMenu*, gtEvent*, void* cd, gtReason);
    static void relation_trg_definition_CB(gtOptionMenu*, gtEvent*, void* cd, gtReason);

    static void c_src_file_top_CB(gtOptionMenu*, gtEvent*, void* cd, gtReason);
    static void c_src_file_bottom_CB(gtOptionMenu*, gtEvent*, void* cd, gtReason);
    static void c_hdr_file_top_CB(gtOptionMenu*, gtEvent*, void* cd, gtReason);
    static void c_hdr_file_bottom_CB(gtOptionMenu*, gtEvent*, void* cd, gtReason);
    static void cpp_src_file_top_CB(gtOptionMenu*, gtEvent*, void* cd, gtReason);
    static void cpp_src_file_bottom_CB(gtOptionMenu*, gtEvent*, void* cd, gtReason);
    static void cpp_hdr_file_top_CB(gtOptionMenu*, gtEvent*, void* cd, gtReason);
    static void cpp_hdr_file_bottom_CB(gtOptionMenu*, gtEvent*, void* cd, gtReason);

    static void impl_CB(gtOptionMenu*, gtEvent*, void* cd, gtReason);
    static void do_same_thing(void* cd,int index);
    
    //    const char* add_type_array (projModule* module);
    //    void show_status (void);
    
    //    int set_boilerplate_default(const char* name);
    //    int get_boilerplate_default(genString& default_type);
    //    void init_sorted_array(GenArr &temp_array);
    //    void delete_item(char *el){ main_array.del(el);}
    //    void append_item(char *el){main_array.append(el);}
    
    void init_type_array();
    int is_default_bit_set(){ return default_bit;}
    void set_default_bit(){ default_bit = 1; }
    void reset_default_bit(){ default_bit = 0;}
    void set_def_common_code(int global_bit) ;

    int default_index ();

    static genString insert_func_init();
    static genString insert_func_final();
    static genString insert_before_func_decl();
    static genString insert_after_func_decl();
    static genString insert_before_func_def();
    static genString insert_after_func_def();
    static genString insert_pub_decl();
    static genString insert_priv_decl();
    static genString insert_prot_decl();
    static genString insert_before_class_decl();
    static genString insert_before_class_def();
    static genString insert_after_class_decl();
    static genString insert_after_class_def();
    static genString insert_before_struct();
    static genString insert_after_struct();
    static genString insert_member_def_init();
    static genString insert_member_def_final();
    static genString insert_before_member_def();
    static genString insert_after_member_def();
    static genString insert_before_member_decl();
    static genString insert_after_member_decl();
    static genString insert_relation_src_member();
    static genString insert_relation_src_header();
    static genString insert_relation_src_definition();
    static genString insert_relation_trg_member();
    static genString insert_relation_trg_header();
    static genString insert_relation_trg_definition();
    static genString insert_c_src_file_top();
    static genString insert_c_src_file_bottom();
    static genString insert_c_hdr_file_top();
    static genString insert_c_hdr_file_bottom();
    static genString insert_cpp_src_file_top();
    static genString insert_cpp_src_file_bottom();
    static genString insert_cpp_hdr_file_top();
    static genString insert_cpp_hdr_file_bottom();
    static genString insert_boilerplate(genString name);
    static genString filter_boilerplate(genString original_string);
 protected:

    gtHorzBox *create_boilerplate_menu(gtBase* ,int type);
    void create_option_menu(gtBase* );
};

void init_default_boilerplates(int from_cancel);

#endif //dd_boil.h




