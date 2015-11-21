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
// gtRTL_boil.h.C
//------------------------------------------
// synopsis:
// 
// Dialog box summoned from button at bottom of gtRTL
//------------------------------------------

// INCLUDE FILES
#define _cLibraryFunctions_h
#include <gtRTL_boil.h>
#include <msg.h>

#ifndef ISO_CPP_HEADERS
#include <ctype.h>
#include <strstream.h>
#include <string.h>
#else /* ISO_CPP_HEADERS */
#include <cctype>
using namespace std;
#include <strstream>
#include <cstring>
#endif /* ISO_CPP_HEADERS */

#include <genString.h>
#include <customize.h>

#include <systemMessages.h>
#include <genWild.h>
#include <Relational.h>
#include <dialog.h>
#include <steView.h>

#include <NewPrompt.h>
#include <top_widgets.h>
#include <symbolArr.h>
#include <path.h>
#include <xref.h>
#include <proj.h>

#include <gtRTL_cfg.h>
#include <gtRTL.h>
#include <gtLabel.h>
#include <gtOptionMenu.h>
#include <gtStringEd.h>
#include <gtHorzBox.h>
#include <gtVertBox.h>
#include <gtFrame.h>
#include <gtForm.h>
#include <gtPushButton.h>
#include <gtCascadeB.h>
#include <gtPDMenu.h>
#include <gtMenuBar.h>
#include <gtDlgTemplate.h>
#include <gtScrollWin.h>
#include <browserShell.h>
#include <viewerShell.h>
#include <psetmem.h>

// VARIABLE DEFINITIONS

static const char DEFAULT_HIDE_PATTERN[] = "*~";  // to hide altered copies of files

const char *func_array[] = {"func-init",
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

const size_t No_of_dd_boilerplates = sizeof(func_array) / sizeof(func_array[0]);

const char *boilerplate_options[] = {"File Types",
                               "Function Types",
                               "Class Types",
                               "Struct Types",
                               "Member Types",
                               "Relation Types"};

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

// FUNCTION DECLARATIONS

static int rename_file(char *old_name,char *new_name,void *cd);
extern void proj_search_modules_by_name(char *proj_name, symbolArr &s1);
static void get_boil_proj(void *cd);
static void init_default_boilerplates();
static void prop_all_changes_to_customize();
static void prop_specific_change_to_customize(int i);
extern int delete_path(gtBase* parent,const char* file_name);
extern bool translate_dir_path(const char*,const char *,const char*,const char*,bool,genString&);

//variable declaration
static genString phy_path_of_boiler;
gtRTL_boil* gtRTL_boil::instance = NULL;

// FUNCTION DEFINITIONS

int boil_share_obtain_proj_info(symbolArr &s1, GenArr &main_array, genString &phy_path_of_boiler)
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
                char *p = strrchr((char *)fn,'/');
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

gtRTL_boil::gtRTL_boil(browserShell* br) : br_rtl(br)
// Make it work if the boilerplate rule is the top project
// initialize physical path of the boilerplate directory
// store all the files in this directory into main_array.
// create the dialog box
{
    Initialize(gtRTL_boil);

    GenArr temp_array;
    not_in_pdf = boil_share_obtain_proj_info(s1, main_array, phy_path_of_boiler);

    reset_default_bit();
    init_default_boilerplates();
    rel_impl_mode = false;
    boilerplate_type = "";

    shell = gtDialogTemplate::create(br_rtl->container(), "list_utils", TXT("Boilerplate List"));

    shell->add_button ("ok", TXT("OK"), OK_CB, this);
    shell->add_button ("reset", TXT("Reset"), Reset_CB, this);
    shell->add_button ("cancel", TXT("Cancel"), Cancel_CB, this);

    shell->add_help_button();
    shell->resize_policy(gtResizeNone);
    shell->default_button(NULL);

    shell->help_context_name("Browser.Utility.Boilerplate.Help");

    gtVertBox *vbox;
    gtForm* top = gtForm::create(shell,"top");
    top->attach(gtTop);
    top->attach(gtLeft);
    top->manage();

    gtFrame* filter = gtFrame::create(top, "frame");
    filter->attach(gtTop);
    filter->attach(gtLeft,NULL,10);
    filter->attach(gtRight);
    filter->manage();

    create_boilerplate_menu(filter);

    gtFrame* status = gtFrame::create (shell, "status_frame");
    status->attach_tblr (status);
    status->manage ();

    status_label = gtLabel::create (status, "status_label", "");
    status_label->alignment (gtBeginning);
    status_label->manage ();

    gtForm* bottom = gtForm::create(shell,"bottom");
    bottom->attach(gtTop,top,10);
    bottom->attach(gtRight);
    bottom->attach(gtLeft);
    bottom->attach(gtBottom, status, 10);
    bottom->manage();

    gtForm* left = gtForm::create(bottom, "form");
    left->attach(gtTop);
    left->attach(gtLeft,NULL,20);
    left->manage();

    gtForm* right = gtForm::create(bottom, "form");
    right->attach(gtTop);
    right->attach(gtLeft,left,20);
    right->attach_pos_right(90);
    right->attach(gtBottom);
    right->manage();

    gtFrame* options = gtFrame::create(left,"frame");
    options->attach(gtTop);
    options->attach(gtLeft);
    options->attach(gtRight);
    options->manage();

    vbox = gtVertBox::create(options, "form");
    vbox->attach(gtTop);
    vbox->attach(gtLeft);
    vbox->attach(gtRight);
    vbox->manage();

    create_option_menu(vbox);

    gtFrame* prompt = gtFrame::create(right,"frame");
    prompt->attach(gtTop);
    prompt->attach(gtLeft);
    prompt->attach(gtRight);
    prompt->manage();

    type_prompt = (gtLabel *)gtLabel::create(
        prompt, "prompt", TXT("List of Boilerplates"));
    type_prompt->attach(gtLeft);
    type_prompt->attach(gtRight);
    type_prompt->attach(gtTop);
    type_prompt->attach(gtBottom);
    type_prompt->alignment(gtCenter);
    type_prompt->manage();

    gtFrame* sort = gtFrame::create(right, "frame");
    sort->attach(gtTop,prompt,5);
    sort->attach(gtLeft);
    sort->attach(gtRight);
    sort->attach(gtBottom);
    sort->manage();
    // changed from list to RTL to filter out *~
    domain_list = gtList::create((gtBase *)sort, (const char *)"domain_list", (const char *)"", gtBrowse,
				 (const char **)temp_array.return_ptr() , (int)temp_array.size());
    domain_list->attach(gtTop);
    domain_list->attach_pos_bottom(80);
    domain_list->attach(gtLeft);
    domain_list->attach(gtRight);
    domain_list->num_rows(10);
    domain_list->width(400);
    domain_list->action_callback(gtRTL_boil::action_CB,this);
    domain_list->manage();

    show_status ();
}

gtRTL_boil::~gtRTL_boil()
{
    delete shell;
    instance = NULL;
}


//------------------------------------------
// [static] gtRTL_boil::refresh_after_delete
//
// refresh display after deletion of a module (that might be one of the files
// shown in the domain list)
//------------------------------------------

void gtRTL_boil::refresh_after_delete() {
   Initialize(gtRTL_boil::refresh_after_delete);

   if (instance) {
      instance->s1.removeAll();
      instance->main_array.reset();
      boil_share_obtain_proj_info(instance->s1, instance->main_array,
				  phy_path_of_boiler);
      for (int i = 0; i < No_of_dd_boilerplates; i++) {
	 if (strcmp(instance->boilerplate_type, func_array[i]) == 0) {
	    do_same_thing(instance, i);  // refresh displayed list
	    break;
	 }
      }
   }
}

void gtRTL_boil::popup()
{
    shell->popup(0);
}

void gtRTL_boil::boilerplate_CB(gtOptionMenu*, gtEvent*, void* cd, gtReason)
{
   Initialize(gtRTL_boil::boilerplate_CB);

   if (!instance) {
      push_busy_cursor();
      instance = new gtRTL_boil((browserShell *)cd);
      pop_cursor();
   }
   instance->popup();
}

// function_boilerplate criteria option menu callbacks

void gtRTL_boil::func_init_CB(gtOptionMenu*, gtEvent*, void* cd, gtReason)
{
    Initialize(gtRTL_boil::func_init_CB);
    gtRTL_boil::do_same_thing(cd,0);
}

void gtRTL_boil::func_final_CB(gtOptionMenu*, gtEvent*, void* cd, gtReason)
{
    Initialize(gtRTL_boil::func_final_CB);
    gtRTL_boil::do_same_thing(cd,1);   
}

void gtRTL_boil::before_func_decl_CB(gtOptionMenu*, gtEvent*, void* cd, gtReason)
{
    Initialize(gtRTL_boil::before_func_decl_CB);	
    gtRTL_boil::do_same_thing(cd,2);
}

void gtRTL_boil::after_func_decl_CB(gtOptionMenu*, gtEvent*, void* cd, gtReason)
{
    Initialize(gtRTL_boil::after_func_decl_CB);
       gtRTL_boil::do_same_thing(cd,3); 
}

void gtRTL_boil::before_func_def_CB(gtOptionMenu*, gtEvent*, void* cd, gtReason)
{
    Initialize(gtRTL_boil::before_func_def_CB);	
    gtRTL_boil::do_same_thing(cd,4);
}

void gtRTL_boil::after_func_def_CB(gtOptionMenu*, gtEvent*, void* cd, gtReason)
{
    Initialize(gtRTL_boil::after_func_def_CB);
       gtRTL_boil::do_same_thing(cd,5); 
}

void gtRTL_boil::pub_decl_CB(gtOptionMenu*, gtEvent*, void* cd, gtReason)
{
    Initialize(gtRTL_boil::pub_decl_CB);
       gtRTL_boil::do_same_thing(cd,6); 
}

void gtRTL_boil::prot_decl_CB(gtOptionMenu*, gtEvent*, void* cd, gtReason)
{
    Initialize(gtRTL_boil::prot_decl_CB);
       gtRTL_boil::do_same_thing(cd,7);
}

void gtRTL_boil::priv_decl_CB(gtOptionMenu*, gtEvent*, void* cd, gtReason)
{
    Initialize(gtRTL_boil::priv_decl_CB);	
       gtRTL_boil::do_same_thing(cd,8); 
}

void gtRTL_boil::before_class_decl_CB(gtOptionMenu*, gtEvent*, void* cd, gtReason)
{
    Initialize(gtRTL_boil::before_class_decl_CB);
       gtRTL_boil::do_same_thing(cd,9);
}

void gtRTL_boil::after_class_decl_CB(gtOptionMenu*, gtEvent*, void* cd, gtReason)
{
    Initialize(gtRTL_boil::after_class_decl_CB);
       gtRTL_boil::do_same_thing(cd,10);
}

void gtRTL_boil::before_class_def_CB(gtOptionMenu*, gtEvent*, void* cd, gtReason)
{
    Initialize(gtRTL_boil::before_class_def_CB);
       gtRTL_boil::do_same_thing(cd,11);
}

void gtRTL_boil::after_class_def_CB(gtOptionMenu*, gtEvent*, void* cd, gtReason)
{
    Initialize(gtRTL_boil::after_class_def_CB);
       gtRTL_boil::do_same_thing(cd,12);
}

void gtRTL_boil::before_struct_CB(gtOptionMenu*, gtEvent*, void* cd, gtReason)
{
    Initialize(gtRTL_boil::before_struct_CB);
      gtRTL_boil::do_same_thing(cd,13); 
}
void gtRTL_boil::after_struct_CB(gtOptionMenu*, gtEvent*, void* cd, gtReason)
{
    Initialize(gtRTL_boil::after_struct_CB);
      gtRTL_boil::do_same_thing(cd,14); 
}
void gtRTL_boil::member_def_init_CB(gtOptionMenu*, gtEvent*, void* cd, gtReason)
{
    Initialize(gtRTL_boil::member_def_init_CB);
       gtRTL_boil::do_same_thing(cd,15);}

void gtRTL_boil::member_def_final_CB(gtOptionMenu*, gtEvent*, void* cd, gtReason)
{
    Initialize(gtRTL_boil::member_def_final_CB);
       gtRTL_boil::do_same_thing(cd,16);
}

void gtRTL_boil::before_member_def_CB(gtOptionMenu*, gtEvent*, void* cd, gtReason)
{
    Initialize(gtRTL_boil::before_member_def_CB);
       gtRTL_boil::do_same_thing(cd,17);
}

void gtRTL_boil::before_member_decl_CB(gtOptionMenu*, gtEvent*, void* cd, gtReason)
{
    Initialize(gtRTL_boil::before_member_decl_CB);
       gtRTL_boil::do_same_thing(cd,19);
}

void gtRTL_boil::after_member_def_CB(gtOptionMenu*, gtEvent*, void* cd, gtReason)
{
    Initialize(gtRTL_boil::after_member_def_CB);
      gtRTL_boil::do_same_thing(cd,18);
}

void gtRTL_boil::after_member_decl_CB(gtOptionMenu*, gtEvent*, void* cd, gtReason)
{
    Initialize(gtRTL_boil::after_member_decl_CB);
       gtRTL_boil::do_same_thing(cd,20);
}
void gtRTL_boil::relation_src_member_CB(gtOptionMenu*, gtEvent*, void* cd, gtReason)
{
    Initialize(gtRTL_boil::relation_src_member_CB);
       gtRTL_boil::do_same_thing(cd,21);
}

void gtRTL_boil::relation_src_header_CB(gtOptionMenu*, gtEvent*, void* cd, gtReason)
{
    Initialize(gtRTL_boil::relation_src_header_CB);
       gtRTL_boil::do_same_thing(cd,22);
}

void gtRTL_boil::relation_src_definition_CB(gtOptionMenu*, gtEvent*, void* cd, gtReason)
{
    Initialize(gtRTL_boil::relation_src_definition_CB);
       gtRTL_boil::do_same_thing(cd,23);
}

void gtRTL_boil::relation_trg_member_CB(gtOptionMenu*, gtEvent*, void* cd, gtReason)
{
    Initialize(gtRTL_boil::relation_trg_member_CB);
       gtRTL_boil::do_same_thing(cd,24);
}

void gtRTL_boil::relation_trg_header_CB(gtOptionMenu*, gtEvent*, void* cd, gtReason)
{
    Initialize(gtRTL_boil::relation_trg_header_CB);
       gtRTL_boil::do_same_thing(cd,25);
}

void gtRTL_boil::relation_trg_definition_CB(gtOptionMenu*, gtEvent*, void* cd, gtReason)
{
    Initialize(gtRTL_boil::relation_trg_definition_CB);
       gtRTL_boil::do_same_thing(cd,26);
}

void gtRTL_boil::c_src_file_top_CB(gtOptionMenu*, gtEvent*, void* cd, gtReason)
{
    Initialize(gtRTL_boil::c_src_file_top_CB);
       gtRTL_boil::do_same_thing(cd,27);
}

void gtRTL_boil::c_src_file_bottom_CB(gtOptionMenu*, gtEvent*, void* cd, gtReason)
{
    Initialize(gtRTL_boil::c_src_file_bottom_CB);
       gtRTL_boil::do_same_thing(cd,28);
}

void gtRTL_boil::c_hdr_file_top_CB(gtOptionMenu*, gtEvent*, void* cd, gtReason)
{
    Initialize(gtRTL_boil::c_hdr_file_top_CB);
       gtRTL_boil::do_same_thing(cd,29);
}

void gtRTL_boil::c_hdr_file_bottom_CB(gtOptionMenu*, gtEvent*, void* cd, gtReason)
{
    Initialize(gtRTL_boil::c_hdr_file_bottom_CB);
       gtRTL_boil::do_same_thing(cd,30);
}

void gtRTL_boil::cpp_src_file_top_CB(gtOptionMenu*, gtEvent*, void* cd, gtReason)
{
    Initialize(gtRTL_boil::cpp_src_file_top_CB);
       gtRTL_boil::do_same_thing(cd,31);
}

void gtRTL_boil::cpp_src_file_bottom_CB(gtOptionMenu*, gtEvent*, void* cd, gtReason)
{
    Initialize(gtRTL_boil::cpp_src_file_bottom_CB);
       gtRTL_boil::do_same_thing(cd,32);
}

void gtRTL_boil::cpp_hdr_file_top_CB(gtOptionMenu*, gtEvent*, void* cd, gtReason)
{
    Initialize(gtRTL_boil::cpp_hdr_file_top_CB);
       gtRTL_boil::do_same_thing(cd,33);
}

void gtRTL_boil::cpp_hdr_file_bottom_CB(gtOptionMenu*, gtEvent*, void* cd, gtReason)
{
    Initialize(gtRTL_boil::cpp_hdr_file_bottom_CB);
       gtRTL_boil::do_same_thing(cd,34);
}

void gtRTL_boil::impl_CB(gtOptionMenu*, gtEvent*, void* cd, gtReason)
{
    Initialize(gtRTL_boil::impl_CB);

    gtRTL_boil* menu = (gtRTL_boil*) cd;
    GenArr	temp_array;

    menu->opt_new_button->set_sensitive (false);
    menu->opt_edit_button->set_sensitive (false);
    menu->opt_del_button->set_sensitive (false);
    menu->type_prompt->text (TXT("List of Relation Implementations"));
    menu->rel_impl_mode = true;

    menu->init_type_array ();
    menu->init_sorted_array (temp_array);
    menu->domain_list->delete_all_items ();
    menu->domain_list->add_items ((const char**) temp_array.return_ptr (), temp_array.size (), 1);
}

void gtRTL_boil::do_same_thing(void* cd,int index)
{
    Initialize(gtRTL_boil::do_same_thing);
    gtRTL_boil* menu = (gtRTL_boil *)cd;
    GenArr temp_array;
    genString temp = "List of ";
    temp += func_array[index];
    temp += " Boilerplates";

    menu->opt_new_button->set_sensitive (true);
    menu->opt_edit_button->set_sensitive (true);
    menu->opt_del_button->set_sensitive (true);
    menu->rel_impl_mode = false;
   
    menu->boilerplate_type = func_array[index];
    menu->init_type_array();
    menu->init_sorted_array(temp_array);
    menu->domain_list->delete_all_items();
    menu->domain_list->add_items((const char**) temp_array.return_ptr() , 
       temp_array.size(),1);
    menu->type_prompt->text(TXT(temp.str()));
}

gtHorzBox* gtRTL_boil::create_boilerplate_menu(
    gtBase* shell)
{
    Initialize(gtRTL_boil::create_boilerplate_menu);
    gtHorzBox* hbox = gtHorzBox::create(shell, "hbox");

    gtMenuBar* menubar = gtMenuBar::create(hbox, "menubar");
    menubar->attach(gtBottom, NULL, 10);
    menubar->attach(gtLeft, NULL, 10);
    menubar->manage();


    gtCascadeButton* m1  = gtCascadeButton::create(
        menubar, "boilerplate_option", TXT("Boilerplate Type"),NULL,NULL);

    m1->pulldown_menu("boilerplate",

        gtMenuCascade, "file", TXT(boilerplate_options[0]),
	NULL,NULL,

        gtMenuCascade, "function", TXT(boilerplate_options[1]),
	NULL,NULL,

        gtMenuCascade, "class", TXT(boilerplate_options[2]),
        NULL,NULL,

        gtMenuCascade, "struct", TXT(boilerplate_options[3]),
        NULL,NULL,

        gtMenuCascade, "member", TXT(boilerplate_options[4]),
        NULL,NULL,
 
	gtMenuCascade, "relation", TXT(boilerplate_options[5]),
        NULL,NULL,

	gtMenuNull);

    gtCascadeButton* file_button =
	(gtCascadeButton*)m1->button("file");

    file_button->pulldown_menu(
	"file_menu",

	gtMenuStandard,func_array[27],TXT(func_array[27]),
	this,gtRTL_boil::c_src_file_top_CB,

	gtMenuStandard, func_array[28],TXT(func_array[28]),
	this,gtRTL_boil::c_src_file_bottom_CB,

	gtMenuStandard, func_array[29],TXT(func_array[29]),
	this,gtRTL_boil::c_hdr_file_top_CB,

	gtMenuStandard, func_array[30],TXT(func_array[30]),
	this,gtRTL_boil::c_hdr_file_bottom_CB,

	gtMenuStandard, func_array[31],TXT(func_array[31]),
	this,gtRTL_boil::cpp_src_file_top_CB,

	gtMenuStandard, func_array[32],TXT(func_array[32]),
	this,gtRTL_boil::cpp_src_file_bottom_CB,

	gtMenuStandard, func_array[33],TXT(func_array[33]),
	this,gtRTL_boil::cpp_hdr_file_top_CB,

	gtMenuStandard, func_array[34],TXT(func_array[34]),
	this,gtRTL_boil::cpp_hdr_file_bottom_CB,

	gtMenuNull);

    gtCascadeButton* function_button =
	(gtCascadeButton*)m1->button("function");

    function_button->pulldown_menu(
	"function_menu",

	gtMenuStandard, func_array[0],TXT(func_array[0]),
	this,gtRTL_boil::func_init_CB,	

	gtMenuStandard, func_array[1],TXT(func_array[1]),
	this,gtRTL_boil::func_final_CB,	

	gtMenuStandard, func_array[2],TXT(func_array[2]),
	this,gtRTL_boil::before_func_decl_CB,	

	gtMenuStandard, func_array[3],TXT(func_array[3]),
	this,gtRTL_boil::after_func_decl_CB,	

	gtMenuStandard, func_array[4],TXT(func_array[4]),
	this,gtRTL_boil::before_func_def_CB,	

	gtMenuStandard, func_array[5],TXT(func_array[5]),
	this,gtRTL_boil::after_func_def_CB,	

        gtMenuNull);

     gtCascadeButton* class_button =
	(gtCascadeButton*)m1->button("class");

     class_button->pulldown_menu(
	"class_menu",

/*	gtMenuStandard, func_array[6],TXT(func_array[6]),
	this,gtRTL_boil::pub_decl_CB,	

	gtMenuStandard, func_array[7],TXT(func_array[7]),
	this,gtRTL_boil::prot_decl_CB,	

	gtMenuStandard, func_array[8],TXT(func_array[8]),
	this,gtRTL_boil::priv_decl_CB,	*/  //Not supported currently. Must be enabled when fixed.

	gtMenuStandard, func_array[9],TXT(func_array[9]),
	this,gtRTL_boil::before_class_decl_CB,	

	gtMenuStandard, func_array[10],TXT(func_array[10]),
	this,gtRTL_boil::after_class_decl_CB,	

	gtMenuStandard, func_array[11],TXT(func_array[11]),
	this,gtRTL_boil::before_class_def_CB,	

/*	gtMenuStandard, func_array[12],TXT(func_array[12]),
	this,gtRTL_boil::after_class_def_CB,	*/ //Not supported currently. Must be enabled when fixed.

        gtMenuNull);

    gtCascadeButton* struct_button =
	(gtCascadeButton*)m1->button("struct");

    struct_button->pulldown_menu(
	"struct_menu",

	gtMenuStandard, func_array[13],TXT(func_array[13]),
	this,gtRTL_boil::before_struct_CB,	

	gtMenuStandard, func_array[14],TXT(func_array[14]),
	this,gtRTL_boil::after_struct_CB,	

        gtMenuNull);

    gtCascadeButton* member_button =
	(gtCascadeButton*)m1->button("member");

    member_button->pulldown_menu(
	"member_menu",

	gtMenuStandard, func_array[15],TXT(func_array[15]),
	this,gtRTL_boil::member_def_init_CB,	

	gtMenuStandard, func_array[16],TXT(func_array[16]),
	this,gtRTL_boil::member_def_final_CB,	

	gtMenuStandard, func_array[17],TXT(func_array[17]),
	this,gtRTL_boil::before_member_def_CB,	

	gtMenuStandard, func_array[18],TXT(func_array[18]),
	this,gtRTL_boil::after_member_def_CB,	

	gtMenuStandard, func_array[19],TXT(func_array[19]),
	this,gtRTL_boil::before_member_decl_CB,	

	gtMenuStandard, func_array[20],TXT(func_array[20]),
	this,gtRTL_boil::after_member_decl_CB,	

        gtMenuNull);

	gtCascadeButton* relation_button =
	(gtCascadeButton*)m1->button("relation");

    	relation_button->pulldown_menu(
	"relation_menu",

	gtMenuStandard, "impl", TXT("Entire Implementation"),
	this, gtRTL_boil::impl_CB,

	gtMenuSeparator, "sep",

	gtMenuStandard, func_array[21],TXT(func_array[21]),
	this,gtRTL_boil::relation_src_member_CB,	

	gtMenuStandard, func_array[22],TXT(func_array[22]),
	this,gtRTL_boil::relation_src_header_CB,	

	gtMenuStandard, func_array[23],TXT(func_array[23]),
	this,gtRTL_boil::relation_src_definition_CB,	

	gtMenuStandard, func_array[24],TXT(func_array[24]),
	this,gtRTL_boil::relation_trg_member_CB,	

	gtMenuStandard, func_array[25],TXT(func_array[25]),
	this,gtRTL_boil::relation_trg_header_CB,	

	gtMenuStandard, func_array[26],TXT(func_array[26]),
	this,gtRTL_boil::relation_trg_definition_CB,	

        gtMenuNull);

    function_button->manage();
    class_button->manage();	
    struct_button->manage();
    member_button->manage();
    m1->manage();
    hbox->manage();

    return hbox;
}


void gtRTL_boil::create_option_menu(gtBase* shell)
{
    opt_new_button=gtPushButton::create(shell,"new","New",
					gtRTL_boil::opt_new_CB,this);	    
    opt_new_button->manage();	

    opt_del_button=gtPushButton::create(shell,"del", "Delete",
					gtRTL_boil::opt_del_CB,this);	    
    opt_del_button->manage();	

    opt_edit_button=gtPushButton::create(shell,"edit","Edit",
					gtRTL_boil::opt_edit_CB,this);	    
    opt_edit_button->manage();	

    opt_set_def_button=gtPushButton::create(shell, "set default","Set default",
					gtRTL_boil::opt_set_def_CB,this);	    
    opt_set_def_button->manage();		

    opt_show_def_button=gtPushButton::create(shell, "show default","Show default",
					gtRTL_boil::opt_show_def_CB,this);    
    opt_show_def_button->manage();

    opt_rem_button=gtPushButton::create(shell, "remove","Remove default",
					gtRTL_boil::opt_rem_def_CB,this);    
    opt_rem_button->manage();
    		
}

void gtRTL_boil::opt_new_CB(gtPushButton*, gtEvent*, void* cd, gtReason)
{
    Initialize(gtRTL_boil::opt_new_CB);
    gtRTL_boil *menu = (gtRTL_boil *) cd;

    if (menu->not_in_pdf)
    {
       msg("ERROR: Must have .boilerplate subproject in home project to create") << eom;
       return;
    }

    if (menu->boilerplate_type.length() == 0)
    {
       msg("ERROR: Must select a boilerplate type before creating") << eom;
       return;
    }

    Prompt create_prompt(menu->br_rtl->container(),
			 /* menu->shell, */ TXT("Create"),TXT("Name of New Boilerplate"));
    genString answer;
    genString phy_name_of_file;

    if(create_prompt.ask(answer)>0)
    {
        char *new_string = strdup(answer.str());
        if (strlen(new_string)==0)
            msg("ERROR: Nothing Selected") << eom;
        else
        {
            push_busy_cursor();
            if (menu->construct_name(new_string,phy_name_of_file))
            {
                if (menu->main_array.search(phy_name_of_file)>=0)
                    msg("ERROR: File already exists") << eom;
                else
                {
                    if (projModule::create_new_file(phy_name_of_file))
                    {
                        projNode* home_proj = projNode::get_home_proj();
                        projModule* module;
                        if (home_proj->is_script())
                        {
                            projNode::create_global_script_modules(phy_name_of_file, FILE_LANGUAGE_UNKNOWN, 0);
                            genString ln;
                            home_proj->fn_to_ln(phy_name_of_file, ln, FILE_TYPE, 0, 1);
                            module = home_proj->find_module(ln);
                        }
                        else
                            module = home_proj->make_new_module(phy_name_of_file);

                        if (module)
                        {
                            module->update_module();
                            module->language(FILE_LANGUAGE_RAW);
                            menu->main_array.append(phy_name_of_file);
                            menu->s1.insert_last(module);
                            menu->intermediate_array.append((char *)phy_name_of_file.str());
                            char *added_string = strdup(menu->add_type_array(module));
                            GenArr temp_array;
                            menu->init_sorted_array(temp_array);
                            int location = temp_array.search(added_string);
                            menu->domain_list->add_item(added_string,location+1);
                        }
                        else
                            msg("ERROR: Internal error in Paraset, Save files") << eom;
                    }
                    else
                        msg("ERROR: Error in creating boilerplate file") << eom;
                }
            }
            else
                msg("ERROR: Incorrect format for filename") << eom;
            pop_cursor();
        }
    }
}

void gtRTL_boil::opt_del_CB(gtPushButton*, gtEvent*, void* cd, gtReason)
{	
   Initialize(gtRTL_boil::opt_del_CB);
   gtRTL_boil *menu = (gtRTL_boil *) cd;
   int k;
   int no_of_selections = menu->domain_list->num_selected();

   if(!no_of_selections)
	msg("ERROR: Nothing Selected") << eom;
   else
	{
           char **d_list = (char **) menu->domain_list->selected();
           char *string = strdup(d_list[0]); 
           int gtlist_pos= menu->domain_list->item_pos(string);
           int index = menu->type_array.search(string);
	   char *name = (char *) strdup(menu->intermediate_array[index]);
           projNode* proj_node = projHeader::fn_to_project(name);
           projNode* home_node = projNode::get_home_proj();          
           if(proj_node == home_node)
	    {
             if((k = delete_path(menu->shell,name))==1)
             {
		menu->domain_list->delete_pos(gtlist_pos);
		menu->intermediate_array.del(index);
                menu->type_array.del(index);
		int sym_index = menu->main_array.search(name);
		menu->main_array.del(name);
                menu->s1.remove_index(sym_index);
                genString default_type;
		int type_index = menu->get_boilerplate_default(default_type);
		if(default_type != "" && default_type == name )
		  {
		     menu->set_boilerplate_default("");
                     prop_specific_change_to_customize(type_index);
                     customize::save_specific_boilerplate(type_index,
			func_array[type_index]); 
    		  }
		}
             else 
                if(k == -1)
	          msg("ERROR: Error in removing boilerplate") << eom;
	    }
           else
		msg("ERROR: The selected file is not in the home project") << eom;
        }
	menu->show_status ();
}
void gtRTL_boil::opt_edit_CB(gtPushButton*, gtEvent*, void* cd, gtReason)
{
    Initialize(gtRTL_boil::opt_edit_CB);	
    edit_common_code(cd);
}

void gtRTL_boil::action_CB(gtList*, gtEvent*, void* cd, gtReason)
{
    Initialize(gtRTL_boil::action_CB);	
    edit_common_code(cd);
}
void gtRTL_boil::edit_common_code(void* cd)
{
    Initialize(gtRTL_boil::edit_common_code);	
    
    gtRTL_boil *menu = (gtRTL_boil *) cd;
    int index;
    int no_of_selections = menu->domain_list->num_selected();

   if(!no_of_selections) 
	msg("ERROR: Nothing Selected") << eom;
   else
	{
	char **d_list = (char **) (menu->domain_list->selected());
        char *string = strdup(d_list[0]);
        index = menu->type_array.search(string);

	   char *name = (char *) menu->intermediate_array[index];
           projNode* proj_node = projHeader::fn_to_project(name);
           projNode* home_node = projNode::get_home_proj();          

 	   viewPtr v = view_create(name);

           if(proj_node != home_node && v)
		{
   		  ldrPtr l = view_get_ldr(v);
                  appPtr a = (l) ? ldr_get_app(l) : NULL;
                  if (a) a->set_read_only (1);
                  if (is_steView(v))
	             (checked_cast(steView,v))->set_readonly(1);
		}
          view_create_flush();
          viewer* edit_v = viewerShell::find_target_viewer();
	  edit_v->enable_reparse(0);
	}
}

void gtRTL_boil::opt_set_def_CB(gtPushButton*, gtEvent*, void* cd, gtReason)
{
    Initialize(gtRTL_boil::opt_set_def_CB);	
    gtRTL_boil *menu = (gtRTL_boil *) cd;

    int no_of_selections = menu->domain_list->num_selected();
    if(!no_of_selections) 
	msg("ERROR: Nothing Selected") << eom;
    else
	{
	char **d_list = (char **) (menu->domain_list->selected());
        char *string = strdup(d_list[0]);
        int index = menu->type_array.search(string);

	char *name = (char *) menu->intermediate_array[index];
       	menu->set_boilerplate_default(name);  
        menu->set_default_bit();
        menu->show_status ();
      }
    	
}

void gtRTL_boil::opt_show_def_CB(gtPushButton*, gtEvent*, void* cd, gtReason)
{
    Initialize(gtRTL_boil::opt_show_def_CB);	
    gtRTL_boil *menu = (gtRTL_boil *) cd;
    genString default_type;     

    int no_of_selections = menu->domain_list->num_selected();
    
    if (!menu->rel_impl_mode && (menu->boilerplate_type == ""))
       msg("ERROR: Select a boilerplate type first") << eom;
    else
	{
         menu->get_boilerplate_default(default_type);
	  
	   if(default_type==(char*)NULL || default_type == "")
           	msg("ERROR: No default set.") << eom;
	   else
	        {
	// select that boilerplate type in the intermediate type array
		  int index = menu->intermediate_array.search((char *)default_type.str());
		genString logical_name= menu->type_array[index]; 
                  if(no_of_selections)
			menu->domain_list->deselect_all();
                  menu->domain_list->select_item((char*)logical_name,0);
                }
	}
}		

void gtRTL_boil::opt_rem_def_CB(gtPushButton*, gtEvent*, void* cd, gtReason)
{
  Initialize(gtRTL_boil::opt_rem_def_CB);
  gtRTL_boil* menu = (gtRTL_boil *) cd;
  
  genString default_type;     

  int no_of_selections = menu->domain_list->num_selected();
  
  if (!menu->rel_impl_mode && (menu->boilerplate_type == ""))
    msg("ERROR: Select a boilerplate type first") << eom;
  else
    {
      menu->get_boilerplate_default(default_type);
      
      if(default_type==(char*)NULL || default_type == "")
	msg("ERROR: No default set.") << eom;
      else
	{
	  menu->set_boilerplate_default("");
          menu->set_default_bit();
	  if(no_of_selections)
	    menu->domain_list->deselect_all();
	}
      menu->show_status ();
    }

}
int gtRTL_boil::get_boilerplate_default(genString& default_type)
{
	int	i, j;
	char	*s1, *s2, *p1, *p2;

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

int gtRTL_boil::set_boilerplate_default(const char* name)
{
	int		i;
	genString	temp;

	if (rel_impl_mode) {
	    for (i = 0; i < NUM_REL_BOILS; i++) {
		*rel_boils[i] = "";
		if ((name == NULL) || (*name == '\0'))
		    continue;
		temp.printf ("%s/%s!%s",
			phy_path_of_boiler.str (), rel_boil_names[i], name);
		if (main_array.search ((char*) temp.str ()) >= 0)
		    *rel_boils[i] = temp;
	    }
	    return -1;
	}
        
	for(i = 0; i < No_of_boilerplates; i++)
	  if(boilerplate_type == func_array[i])
             break;
          
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

bool gtRTL_boil::construct_name (const char* base, genString& name)
{
    //
    // --- JJS 1/12/95 ---
    // This function needs to be a nonstatic member so that it can embed
    // the correct boilerplate type into the boilerplate name.  I am also
    // rewriting it completely; the old version did not do proper error
    // checking, had memory leaks, and embedded useless information into
    // the generated name.
    //

    Initialize (gtRTL_boil::construct_name);

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

void gtRTL_boil::init_type_array ()
{
    Initialize (gtRTL_boil::init_type_array);

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

    show_status ();
}

void gtRTL_boil::show_status (void)
{
    Initialize (gtRTL_boil::show_status);

    const char* thing = rel_impl_mode ? TXT("implementation") : TXT("boilerplate");

    if (!rel_impl_mode && (boilerplate_type.length () < 1)) {
	status_label->text (TXT("Status: No boilerplate type selected."));
	return;
    }

    genString def, temp;
    get_boilerplate_default (def);

    if (def.length () < 1) {
	temp.printf (TXT("Status: No default %s."), thing);
	status_label->text (temp);
	return;
    }

    for (int j = 0; j < intermediate_array.size (); j++) {
        if (def == intermediate_array[j]) {
	    def = type_array[j];
	    break;
        }
    }

    temp.printf (TXT("Status: Default %s is %s."), thing, def.str ());
    status_label->text (temp);
}	

const char* gtRTL_boil::add_type_array (projModule* module)
{
    Initialize (gtRTL_boil::add_type_array);

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


void gtRTL_boil::init_sorted_array(GenArr &temp_array)
{
 
     int num_of_items = type_array.size();   
     for(int j=0;j<num_of_items;j++)
	temp_array.append(type_array[j]);
     temp_array.sort();
}

static void init_default_boilerplates()
{

	func_init_boilerplate = customize::func_init();
 	func_final_boilerplate = customize::func_final();
 	before_func_decl_boilerplate = customize:: before_func_decl();
 	after_func_decl_boilerplate = customize::after_func_decl();
 	before_func_def_boilerplate = customize:: before_func_def();
 	after_func_def_boilerplate = customize::after_func_def();
 	pub_decl_boilerplate = customize::pub_decl();
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
}

void gtRTL_boil::OK_CB(gtPushButton*, gtEvent*, void* cd, gtReason)
{	
    Initialize(gtRTL_boil::OK_CB);
    gtRTL_boil* menu = (gtRTL_boil *)cd;
    
    if(menu->is_default_bit_set()) {
	prop_all_changes_to_customize();
	customize::save_boilerplate();
    }

    menu->shell->popdown();
    delete menu; 
}


void gtRTL_boil::Reset_CB(gtPushButton*, gtEvent*, void* cd, gtReason)
{	
    Initialize(gtRTL_boil::Reset_CB);
    gtRTL_boil* menu = (gtRTL_boil *)cd;
    
    if(menu->is_default_bit_set()){
	
        //Restore local variables from the memory variables 
        
        init_default_boilerplates();
       
        // Also reflect this change in the current view!
        
        genString default_type;
        menu->get_boilerplate_default(default_type);
	  
	   if(default_type!=(char*)NULL && default_type != "")
           {
	     // select that boilerplate type in the intermediate type array
		  int index = menu->intermediate_array.search((char *)default_type.str());
		  genString logical_name= menu->type_array[index]; 
		  int no_of_selections = menu->domain_list->num_selected();
                  if(no_of_selections)
			menu->domain_list->deselect_all();
                  menu->domain_list->select_item((char*)logical_name,0);
            }
	         
         menu->reset_default_bit();
	 menu->show_status ();
    }
}

void gtRTL_boil::Cancel_CB(gtPushButton*, gtEvent*, void* cd, gtReason)
{	
    Initialize(gtRTL_boil::Reset_CB);
    gtRTL_boil* menu = (gtRTL_boil *)cd;

    //restore local variables from main memory variables

    init_default_boilerplates();
    menu->shell->popdown();
    delete menu; 
}

static void prop_all_changes_to_customize()
{
	customize::func_init(func_init_boilerplate);
 	customize::func_final(func_final_boilerplate);
 	customize::before_func_decl(before_func_decl_boilerplate);
 	customize::after_func_decl(after_func_decl_boilerplate);
 	customize::before_func_def(before_func_def_boilerplate);
 	customize::after_func_def(after_func_def_boilerplate);
 	customize::pub_decl(pub_decl_boilerplate);
 	customize::prot_decl(prot_decl_boilerplate);
 	customize::priv_decl(priv_decl_boilerplate);
 	customize::before_class_decl(before_class_decl_boilerplate);
 	customize::after_class_decl(after_class_decl_boilerplate);
 	customize::before_class_def(before_class_def_boilerplate);
 	customize::after_class_def(after_class_def_boilerplate);
 	customize::before_struct(before_struct_boilerplate);
 	customize::after_struct(after_struct_boilerplate);
	customize::member_def_init(member_def_init_boilerplate) ;
	customize::member_def_final(member_def_final_boilerplate) ;
	customize::before_member_def(before_member_def_boilerplate) ;
	customize::after_member_def(after_member_def_boilerplate) ;
	customize::before_member_decl(before_member_decl_boilerplate) ;
	customize::after_member_decl(after_member_decl_boilerplate) ;
        customize::relation_src_member(relation_src_member_boilerplate) ;
        customize::relation_src_header(relation_src_header_boilerplate) ;
        customize::relation_src_definition(relation_src_definition_boilerplate) ;
        customize::relation_trg_member(relation_trg_member_boilerplate) ;
        customize::relation_trg_header(relation_trg_header_boilerplate) ;
        customize::relation_trg_definition(relation_trg_definition_boilerplate) ;
        customize::c_src_file_top(c_src_file_top_boilerplate) ;
        customize::c_src_file_bottom(c_src_file_bottom_boilerplate) ;
        customize::c_hdr_file_top(c_hdr_file_top_boilerplate) ;
        customize::c_hdr_file_bottom(c_hdr_file_bottom_boilerplate) ;
        customize::cpp_src_file_top(cpp_src_file_top_boilerplate) ;
        customize::cpp_src_file_bottom(cpp_src_file_bottom_boilerplate) ;
        customize::cpp_hdr_file_top(cpp_hdr_file_top_boilerplate) ;
        customize::cpp_hdr_file_bottom(cpp_hdr_file_bottom_boilerplate) ;

}   

static void prop_specific_change_to_customize(int i)
{

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

/*
$Log: gtRTL_boil.h.C  $
Revision 1.22 2000/07/12 18:09:35EDT ktrans 
merge from stream_message branch
 * Revision 1.14  1994/06/20  14:08:09  so
 * Bug track: 7448
 * fix bug 7448
 *
*/




