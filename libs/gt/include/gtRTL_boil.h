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
#ifndef _gtRTL_boil_h
#define _gtRTL_boil_h

// gtRTL_boil.h
//------------------------------------------
// synopsis:
// 
// Dialog box summoned from boilerplate option in the utility submenu of the browser main menu.
//------------------------------------------
//#include <gtGenArr.h>
#include <genString.h>
#include <symbolArr.h>
#include <view_creation.h>

#ifndef _gt_h
#include <gt.h>
#endif

#define No_of_boilerplates 35

RelClass(projModule);

extern view* view_create(const char* name);
class gtRTL;
class browserShell;

class gtRTL_boil
{
    static gtRTL_boil* instance;
    
    gtDialogTemplate *shell;
    gtLabel* status_label;

    gtPushButton* opt_new_button;
    gtPushButton* opt_del_button;
    gtPushButton* opt_edit_button;
    gtPushButton* opt_set_def_button;
    gtPushButton* opt_show_def_button;
    gtPushButton* opt_rem_button;

    gtStringEditor *regexp;
   
    gtOptionMenu *boilerplate_menu;

    int not_in_pdf;  
    browserShell *br_rtl;
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
    int default_bit;
    bool rel_impl_mode;

    public:
     gtRTL_boil(browserShell*);
    ~gtRTL_boil();
    static void refresh_after_delete();

    void popup();

    static void OK_CB(gtPushButton *,gtEvent *,void *cd,gtReason);
    static void Reset_CB(gtPushButton *,gtEvent *,void *cd,gtReason);
    static void Cancel_CB(gtPushButton *,gtEvent *,void *cd,gtReason);

    static void action_CB(gtList*, gtEvent*, void* cd, gtReason);
    static void edit_common_code(void* cd);

    static void boilerplate_CB(gtOptionMenu *, gtEvent *, void *cd, gtReason);

    static void opt_ren_CB(gtPushButton*, gtEvent*, void* cd, gtReason);
    static void opt_new_CB(gtPushButton*, gtEvent*, void* cd, gtReason);
    static void opt_edit_CB(gtPushButton*, gtEvent*, void* cd, gtReason); 
    static void opt_del_CB(gtPushButton*, gtEvent*, void* cd, gtReason);
    static void opt_set_def_CB(gtPushButton*, gtEvent*, void* cd, gtReason);
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
    
    const char *add_type_array (projModule* module);
    void show_status (void);
    
    int set_boilerplate_default(const char* name);
    int get_boilerplate_default(genString& default_type);

    bool construct_name (const char* base, genString& name);
    //    void init_sorted_array(GenArr &temp_array);
    //    void delete_item(char *el){ main_array.del(el);}
    //    void append_item(char *el){main_array.append(el);}
    
    void init_type_array();
    int is_default_bit_set(){ return default_bit;}
    void set_default_bit(){ default_bit = 1; }
    void reset_default_bit(){ default_bit = 0;}
 

 protected:

    gtHorzBox *create_boilerplate_menu(gtBase* );
    void create_option_menu(gtBase* );
};


/*
   START-LOG-------------------------------------------

   $Log: gtRTL_boil.h  $
   Revision 1.9 1996/06/19 09:26:16EDT abover 
   Moving <GenArr.h> to <gtGenArr.h>
 * Revision 1.5  1994/01/06  18:30:48  wmm
 * Bug track: 5814
 * Fix bug 5814 (rename decl-{before | after} boilerplates to the more correct and descriptive
 * {before | after}-func-def (and fix a couple of other minor unreported bugs along the way).)
 *
 * Revision 1.4  1993/08/05  13:45:09  plumba
 * added realtional boilerplates
 *
 * Revision 1.3  1993/07/27  14:04:31  plumba
 * boilerplate
 *
 * Revision 1.1  1993/07/15  14:12:31  plumba
 * Initial revision
 *
// Revision 1.1  1993/04/30  18:51:49  glenn
// Initial revision
//
   END-LOG---------------------------------------------
*/

#endif // _gtRTL_boil_h


