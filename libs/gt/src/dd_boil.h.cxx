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
// dd_boil.h.C
//------------------------------------------
// synopsis:
// 
// Dialog box summoned from button at bottom of gtRTL
//------------------------------------------

// INCLUDE FILES
#include <cLibraryFunctions.h>
#include <msg.h>
#include <dd_boil.h>
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
#include <dd_set_opt.h>

// VARIABLE DEFINITIONS
extern char *func_array[];
extern char *boilerplate_options[];

static genString dd_func_init_boilerplate("");
static genString dd_func_final_boilerplate("");
static genString dd_before_func_decl_boilerplate("");
static genString dd_after_func_decl_boilerplate("");
static genString dd_before_func_def_boilerplate("");
static genString dd_after_func_def_boilerplate("");
static genString dd_pub_decl_boilerplate("");
static genString dd_prot_decl_boilerplate("");
static genString dd_priv_decl_boilerplate("");
static genString dd_before_class_decl_boilerplate("");
static genString dd_after_class_decl_boilerplate("");
static genString dd_before_class_def_boilerplate("");
static genString dd_after_class_def_boilerplate("");
static genString dd_before_struct_boilerplate("");
static genString dd_after_struct_boilerplate("");
static genString dd_member_def_init_boilerplate("");
static genString dd_member_def_final_boilerplate("");
static genString dd_before_member_def_boilerplate("");
static genString dd_after_member_def_boilerplate("");
static genString dd_before_member_decl_boilerplate("");
static genString dd_after_member_decl_boilerplate("");
static genString dd_relation_src_member_boilerplate("");
static genString dd_relation_src_header_boilerplate("");
static genString dd_relation_src_definition_boilerplate("");
static genString dd_relation_trg_member_boilerplate("");
static genString dd_relation_trg_header_boilerplate("");
static genString dd_relation_trg_definition_boilerplate("");
static genString dd_c_src_file_top_boilerplate("");
static genString dd_c_src_file_bottom_boilerplate("");
static genString dd_c_hdr_file_top_boilerplate("");
static genString dd_c_hdr_file_bottom_boilerplate("");
static genString dd_cpp_src_file_top_boilerplate("");
static genString dd_cpp_src_file_bottom_boilerplate("");
static genString dd_cpp_hdr_file_top_boilerplate("");
static genString dd_cpp_hdr_file_bottom_boilerplate("");

#define NUM_REL_BOILS 6

static genString* rel_boils[] = {
    &dd_relation_src_member_boilerplate,
    &dd_relation_src_header_boilerplate,
    &dd_relation_src_definition_boilerplate,
    &dd_relation_trg_member_boilerplate,
    &dd_relation_trg_header_boilerplate,
    &dd_relation_trg_definition_boilerplate
};

static const char* rel_boil_names[] = {
    "relation-src-member",
    "relation-src-header",
    "relation-src-definition",
    "relation-trg-member",
    "relation-trg-header",
    "relation-trg-definition",
};

static int global_bit_set[No_of_dd_boilerplates];			  

// genString Variables

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


// FUNCTION DECLARATIONS

extern int boil_share_obtain_proj_info(symbolArr &s1, GenArr &main_array, genString &phy_path_of_boiler);
void init_default_boilerplates(int from_cancel);
static void prop_all_changes_to_customize();

//variable declaration
static genString phy_path_of_boiler;
dd_boil* dd_boil::instance = NULL;

// FUNCTION DEFINITIONS

dd_boil::dd_boil(gtBase* gtb,int type)
: gt_base(gtb),desens_type(type)
{
    Initialize(dd_boil);

    GenArr temp_array;
    not_in_pdf = boil_share_obtain_proj_info(s1, main_array, phy_path_of_boiler);

    reset_default_bit();  
    rel_impl_mode = false;
    boilerplate_type = "";

    shell = gtDialogTemplate::create(
	gtb, "list_utils", TXT("Boilerplate List"));

        shell->add_button ("ok", TXT("OK"), OK_CB, this);
        shell->add_button ("cancel", TXT("Cancel"), Cancel_CB, this);


        shell->add_help_button();
	shell->resize_policy(gtResizeNone);
	shell->default_button(NULL);
        


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

    create_boilerplate_menu(filter,desens_type);

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


    domain_list = gtList::create(
        (gtBase *)sort, (const char *)"domain_list", (const char *)TXT(""), (gtListStyle)gtBrowse, (const char **)temp_array.return_ptr() , (int)temp_array.size());
    domain_list->attach(gtTop);
    domain_list->attach_pos_bottom(80);
    domain_list->attach(gtLeft);
    domain_list->attach(gtRight);
    domain_list->num_rows(10);
    domain_list->width(400);
    domain_list->manage();

    show_status ();
}


dd_boil::~dd_boil()
{
    delete shell;
    instance = NULL;
}

//------------------------------------------
// [static] dd_boil::refresh_after_delete
//
// refresh display after deletion of a module (that might be one of the files
// shown in the domain list)
//------------------------------------------

void dd_boil::refresh_after_delete() {
   Initialize(dd_boil::refresh_after_delete);

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

void dd_boil::popup()
{
    shell->popup(0);
}

void dd_boil::pop_dd_boilerplate(DD_boil type, void* cd)
{
    Initialize(dd_boil::pop_dd_boilerplate);
    
    if (!instance) {
       push_busy_cursor();
       instance = new dd_boil((gtBase*)cd,type);
       pop_cursor();
    }
    instance->popup();
    init_default_boilerplates(0);
}

// function_boilerplate criteria option menu callbacks


void dd_boil::func_init_CB(gtOptionMenu*, gtEvent*, void* cd, gtReason)
{
    Initialize(dd_boil::func_init_CB);
    dd_boil::do_same_thing(cd,0);
}

void dd_boil::func_final_CB(gtOptionMenu*, gtEvent*, void* cd, gtReason)
{
    Initialize(dd_boil::func_final_CB);
    dd_boil::do_same_thing(cd,1);   
}

void dd_boil::before_func_decl_CB(gtOptionMenu*, gtEvent*, void* cd, gtReason)
{
    Initialize(dd_boil::before_func_decl_CB);	
    dd_boil::do_same_thing(cd,2);
}

void dd_boil::after_func_decl_CB(gtOptionMenu*, gtEvent*, void* cd, gtReason)
{
    Initialize(dd_boil::after_func_decl_CB);
       dd_boil::do_same_thing(cd,3); 
}

void dd_boil::before_func_def_CB(gtOptionMenu*, gtEvent*, void* cd, gtReason)
{
    Initialize(dd_boil::before_func_def_CB);	
    dd_boil::do_same_thing(cd,4);
}

void dd_boil::after_func_def_CB(gtOptionMenu*, gtEvent*, void* cd, gtReason)
{
    Initialize(dd_boil::after_func_def_CB);
       dd_boil::do_same_thing(cd,5); 
}

void dd_boil::pub_decl_CB(gtOptionMenu*, gtEvent*, void* cd, gtReason)
{
    Initialize(dd_boil::pub_decl_CB);
       dd_boil::do_same_thing(cd,6); 
}

void dd_boil::prot_decl_CB(gtOptionMenu*, gtEvent*, void* cd, gtReason)
{
    Initialize(dd_boil::prot_decl_CB);
       dd_boil::do_same_thing(cd,7);
}

void dd_boil::priv_decl_CB(gtOptionMenu*, gtEvent*, void* cd, gtReason)
{
    Initialize(dd_boil::priv_decl_CB);	
       dd_boil::do_same_thing(cd,8); 
}

void dd_boil::before_class_decl_CB(gtOptionMenu*, gtEvent*, void* cd, gtReason)
{
    Initialize(dd_boil::before_class_decl_CB);
       dd_boil::do_same_thing(cd,9);
}

void dd_boil::after_class_decl_CB(gtOptionMenu*, gtEvent*, void* cd, gtReason)
{
    Initialize(dd_boil::after_class_decl_CB);
       dd_boil::do_same_thing(cd,10);
}

void dd_boil::before_class_def_CB(gtOptionMenu*, gtEvent*, void* cd, gtReason)
{
    Initialize(dd_boil::before_class_def_CB);
       dd_boil::do_same_thing(cd,11);
}

void dd_boil::after_class_def_CB(gtOptionMenu*, gtEvent*, void* cd, gtReason)
{
    Initialize(dd_boil::after_class_def_CB);
       dd_boil::do_same_thing(cd,12);
}

void dd_boil::before_struct_CB(gtOptionMenu*, gtEvent*, void* cd, gtReason)
{
    Initialize(dd_boil::before_struct_CB);
      dd_boil::do_same_thing(cd,13); 
}
void dd_boil::after_struct_CB(gtOptionMenu*, gtEvent*, void* cd, gtReason)
{
    Initialize(dd_boil::after_struct_CB);
      dd_boil::do_same_thing(cd,14); 
}
void dd_boil::member_def_init_CB(gtOptionMenu*, gtEvent*, void* cd, gtReason)
{
    Initialize(dd_boil::member_def_init_CB);
       dd_boil::do_same_thing(cd,15);
}

void dd_boil::member_def_final_CB(gtOptionMenu*, gtEvent*, void* cd, gtReason)
{
    Initialize(dd_boil::member_def_final_CB);
       dd_boil::do_same_thing(cd,16);
}

void dd_boil::before_member_def_CB(gtOptionMenu*, gtEvent*, void* cd, gtReason)
{
    Initialize(dd_boil::before_member_def_CB);
       dd_boil::do_same_thing(cd,17);
}

void dd_boil::before_member_decl_CB(gtOptionMenu*, gtEvent*, void* cd, gtReason)
{
    Initialize(dd_boil::before_member_decl_CB);
       dd_boil::do_same_thing(cd,18);
}

void dd_boil::after_member_def_CB(gtOptionMenu*, gtEvent*, void* cd, gtReason)
{
    Initialize(dd_boil::after_member_def_CB);
      dd_boil::do_same_thing(cd,19);
}

void dd_boil::after_member_decl_CB(gtOptionMenu*, gtEvent*, void* cd, gtReason)
{
    Initialize(dd_boil::after_member_decl_CB);
       dd_boil::do_same_thing(cd,20);
}

void dd_boil::relation_src_member_CB(gtOptionMenu*, gtEvent*, void* cd, gtReason)
{
    Initialize(dd_boil::relation_src_member_CB);
       dd_boil::do_same_thing(cd,21);
}

void dd_boil::relation_src_header_CB(gtOptionMenu*, gtEvent*, void* cd, gtReason)
{
    Initialize(dd_boil::relation_src_header_CB);
       dd_boil::do_same_thing(cd,22);
}

void dd_boil::relation_src_definition_CB(gtOptionMenu*, gtEvent*, void* cd, gtReason)
{
    Initialize(dd_boil::relation_src_definition_CB);
       dd_boil::do_same_thing(cd,23);
}

void dd_boil::relation_trg_member_CB(gtOptionMenu*, gtEvent*, void* cd, gtReason)
{
    Initialize(dd_boil::relation_trg_member_CB);
       dd_boil::do_same_thing(cd,24);
}

void dd_boil::relation_trg_header_CB(gtOptionMenu*, gtEvent*, void* cd, gtReason)
{
    Initialize(dd_boil::relation_trg_header_CB);
       dd_boil::do_same_thing(cd,25);
}

void dd_boil::relation_trg_definition_CB(gtOptionMenu*, gtEvent*, void* cd, gtReason)
{
    Initialize(dd_boil::relation_trg_definition_CB);
       dd_boil::do_same_thing(cd,26);
}

void dd_boil::c_src_file_top_CB(gtOptionMenu*, gtEvent*, void* cd, gtReason)
{
    Initialize(dd_boil::c_src_file_top_CB);
       dd_boil::do_same_thing(cd,27);
}

void dd_boil::c_src_file_bottom_CB(gtOptionMenu*, gtEvent*, void* cd, gtReason)
{
    Initialize(dd_boil::c_src_file_bottom_CB);
       dd_boil::do_same_thing(cd,28);
}

void dd_boil::c_hdr_file_top_CB(gtOptionMenu*, gtEvent*, void* cd, gtReason)
{
    Initialize(dd_boil::c_hdr_file_top_CB);
       dd_boil::do_same_thing(cd,29);
}

void dd_boil::c_hdr_file_bottom_CB(gtOptionMenu*, gtEvent*, void* cd, gtReason)
{
    Initialize(dd_boil::c_hdr_file_bottom_CB);
       dd_boil::do_same_thing(cd,30);
}

void dd_boil::cpp_src_file_top_CB(gtOptionMenu*, gtEvent*, void* cd, gtReason)
{
    Initialize(dd_boil::cpp_src_file_top_CB);
       dd_boil::do_same_thing(cd,31);
}

void dd_boil::cpp_src_file_bottom_CB(gtOptionMenu*, gtEvent*, void* cd, gtReason)
{
    Initialize(dd_boil::cpp_src_file_bottom_CB);
       dd_boil::do_same_thing(cd,32);
}

void dd_boil::cpp_hdr_file_top_CB(gtOptionMenu*, gtEvent*, void* cd, gtReason)
{
    Initialize(dd_boil::cpp_hdr_file_top_CB);
       dd_boil::do_same_thing(cd,33);
}

void dd_boil::cpp_hdr_file_bottom_CB(gtOptionMenu*, gtEvent*, void* cd, gtReason)
{
    Initialize(dd_boil::cpp_hdr_file_bottom_CB);
       dd_boil::do_same_thing(cd,34);
}

void dd_boil::impl_CB(gtOptionMenu*, gtEvent*, void* cd, gtReason)
{
    Initialize(dd_boil::impl_CB);

    dd_boil* menu = (dd_boil*) cd;
    GenArr    temp_array;

    menu->type_prompt->text (TXT("List of Relation Implementations"));
    menu->rel_impl_mode = true;

    menu->init_type_array ();
    menu->init_sorted_array (temp_array);
    menu->domain_list->delete_all_items ();
    menu->domain_list->add_items ((const char**) temp_array.return_ptr (), temp_array.size (), 1);
}

void dd_boil::do_same_thing(void* cd,int index)
{
    Initialize(dd_boil::do_same_thing);
    dd_boil* menu = (dd_boil *)cd;
    GenArr temp_array;
    genString temp = "List of ";
    temp += func_array[index];
    temp += " Boilerplates";

    menu->rel_impl_mode = false;
   
    menu->boilerplate_type = func_array[index];
    menu->init_type_array();
    menu->init_sorted_array(temp_array);
    menu->domain_list->delete_all_items();
    menu->domain_list->add_items((const char**) temp_array.return_ptr() , 
       temp_array.size(),1);
    menu->type_prompt->text(TXT(temp.str()));
}


gtHorzBox* dd_boil::create_boilerplate_menu(
    gtBase* shell,int type)
{
    Initialize(dd_boil::create_boilerplate_menu);
    gtHorzBox* hbox = gtHorzBox::create(shell, "hbox");

    gtMenuBar* menubar = gtMenuBar::create(hbox, "menubar");
    menubar->attach(gtBottom, NULL, 10);
    menubar->attach(gtLeft, NULL, 10);
    menubar->manage();


    gtCascadeButton* m1  = gtCascadeButton::create(
        menubar, "boilerplate_option", TXT("Boilerplate Type"),NULL,NULL);

    m1->pulldown_menu("boilerplate",

	gtMenuCascade, "file", TXT(boilerplate_options[0]),
	NULL, NULL,

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

	gtMenuStandard, func_array[27],TXT(func_array[27]),
	this,dd_boil::c_src_file_top_CB,

	gtMenuStandard, func_array[28],TXT(func_array[28]),
	this,dd_boil::c_src_file_bottom_CB,

	gtMenuStandard, func_array[29],TXT(func_array[29]),
	this,dd_boil::c_hdr_file_top_CB,

	gtMenuStandard, func_array[30],TXT(func_array[30]),
	this,dd_boil::c_hdr_file_bottom_CB,

	gtMenuStandard, func_array[31],TXT(func_array[31]),
	this,dd_boil::cpp_src_file_top_CB,

	gtMenuStandard, func_array[32],TXT(func_array[32]),
	this,dd_boil::cpp_src_file_bottom_CB,

	gtMenuStandard, func_array[33],TXT(func_array[33]),
	this,dd_boil::cpp_hdr_file_top_CB,

	gtMenuStandard, func_array[34],TXT(func_array[34]),
	this,dd_boil::cpp_hdr_file_bottom_CB,

	gtMenuNull);

    gtCascadeButton* function_button =
	(gtCascadeButton*)m1->button("function");

    function_button->pulldown_menu(
	"function_menu",

	gtMenuStandard, func_array[0],TXT(func_array[0]),
	this,dd_boil::func_init_CB,	

	gtMenuStandard, func_array[1],TXT(func_array[1]),
	this,dd_boil::func_final_CB,	

	gtMenuStandard, func_array[2],TXT(func_array[2]),
	this,dd_boil::before_func_decl_CB,	

	gtMenuStandard, func_array[3],TXT(func_array[3]),
	this,dd_boil::after_func_decl_CB,	

	gtMenuStandard, func_array[4],TXT(func_array[4]),
	this,dd_boil::before_func_def_CB,	

	gtMenuStandard, func_array[5],TXT(func_array[5]),
	this,dd_boil::after_func_def_CB,	

        gtMenuNull);

     gtCascadeButton* class_button =
	(gtCascadeButton*)m1->button("class");

     class_button->pulldown_menu(
	"class_menu",

/*	gtMenuStandard, func_array[6],TXT(func_array[6]),
	this,dd_boil::pub_decl_CB,	

	gtMenuStandard, func_array[7],TXT(func_array[7]),
	this,dd_boil::prot_decl_CB,	

	gtMenuStandard, func_array[8],TXT(func_array[8]),
	this,dd_boil::priv_decl_CB,	*/     // These are currently not supported. Must be reenabled later.

	gtMenuStandard, func_array[9],TXT(func_array[9]),
	this,dd_boil::before_class_decl_CB,	

	gtMenuStandard, func_array[10],TXT(func_array[10]),
	this,dd_boil::after_class_decl_CB,	

	gtMenuStandard, func_array[11],TXT(func_array[11]),
	this,dd_boil::before_class_def_CB,	

/*	gtMenuStandard, func_array[12],TXT(func_array[12]),
	this,dd_boil::after_class_def_CB,   */   //Not supported currently. Must be enabled when fixed.

        gtMenuNull);

    gtCascadeButton* struct_button =
	(gtCascadeButton*)m1->button("struct");

    struct_button->pulldown_menu(
	"struct_menu",

	gtMenuStandard, func_array[13],TXT(func_array[13]),
	this,dd_boil::before_struct_CB,	

	gtMenuStandard, func_array[14],TXT(func_array[14]),
	this,dd_boil::after_struct_CB,	

        gtMenuNull);

    gtCascadeButton* member_button =
	(gtCascadeButton*)m1->button("member");

    member_button->pulldown_menu(
	"member_menu",

	gtMenuStandard, func_array[15],TXT(func_array[15]),
	this,dd_boil::member_def_init_CB,	

	gtMenuStandard, func_array[16],TXT(func_array[16]),
	this,dd_boil::member_def_final_CB,	

	gtMenuStandard, func_array[17],TXT(func_array[17]),
	this,dd_boil::before_member_def_CB,	

	gtMenuStandard, func_array[18],TXT(func_array[18]),
	this,dd_boil::after_member_def_CB,	

	gtMenuStandard, func_array[19],TXT(func_array[19]),
	this,dd_boil::before_member_decl_CB,	

	gtMenuStandard, func_array[20],TXT(func_array[20]),
	this,dd_boil::after_member_decl_CB,	

        gtMenuNull);

    gtCascadeButton* relation_button =
	(gtCascadeButton*)m1->button("relation");

    relation_button->pulldown_menu(
	"relation_menu",

        gtMenuStandard, "impl", TXT("Entire Implementation"),
        this, dd_boil::impl_CB,

        gtMenuSeparator, "sep",

	gtMenuStandard, func_array[21],TXT(func_array[21]),
	this,dd_boil::relation_src_member_CB,	

	gtMenuStandard, func_array[22],TXT(func_array[22]),
	this,dd_boil::relation_src_header_CB,	

	gtMenuStandard, func_array[23],TXT(func_array[23]),
	this,dd_boil::relation_src_definition_CB,	

	gtMenuStandard, func_array[24],TXT(func_array[24]),
	this,dd_boil::relation_trg_member_CB,	

	gtMenuStandard, func_array[25],TXT(func_array[25]),
	this,dd_boil::relation_trg_header_CB,	

	gtMenuStandard, func_array[26],TXT(func_array[26]),
	this,dd_boil::relation_trg_definition_CB,	

        gtMenuNull);

        file_button->set_sensitive(0);
 	class_button->set_sensitive(0);
	struct_button->set_sensitive(0); 
        function_button->set_sensitive(0);	
        member_button->set_sensitive(0);
        relation_button->set_sensitive(0);
	switch (type) {
// DD_file not yet available/used:
//	case 	DD_file : file_button->set_sensitive(1);
//		break;
	case	DD_func : function_button->set_sensitive(1);
	 	break;
	case	DD_class: class_button->set_sensitive(1);
		break;
	case	DD_struct: struct_button->set_sensitive(1);
		break;
        case    DD_member: member_button->set_sensitive(1);
                break;
        case    DD_relation: relation_button->set_sensitive(1);
     	}
    file_button->manage();
    function_button->manage();
    class_button->manage();	
    struct_button->manage();
    member_button->manage();
    relation_button->manage();
    m1->manage();
    hbox->manage();

    return hbox;
}


void dd_boil::create_option_menu(gtBase* shell)
{
    
    Initialize(dd_boil::create_option_menu);
   
    gtPushButton* opt_set_local_def_button=gtPushButton::create(shell,
	 "set local default","Set local default",
		dd_boil::opt_set_local_def_CB,this);
	    
    opt_set_local_def_button->manage();		

    gtPushButton* opt_set_global_def_button=gtPushButton::create(shell,
	"set global default","Set Global default",
		dd_boil::opt_set_global_def_CB,this);    
    opt_set_global_def_button->manage();		

    gtPushButton* opt_show_def_button=gtPushButton::create(shell,
	"show default","Show default",
		dd_boil::opt_show_def_CB,this);    
    opt_show_def_button->manage();		

    gtPushButton* opt_rem_button=gtPushButton::create(shell,
	"remove","Remove default",dd_boil::opt_rem_def_CB,this);    
    opt_rem_button->manage();


}


void dd_boil::opt_set_local_def_CB(gtPushButton*, gtEvent*, void* cd, gtReason)
{
    Initialize(dd_boil::opt_set_local_def_CB);	
    dd_boil *menu = (dd_boil *) cd;

    menu->set_def_common_code(0);
}

void dd_boil::opt_set_global_def_CB(gtPushButton*, gtEvent*, void* cd, gtReason)
{
    Initialize(dd_boil::opt_set_global_def_CB);	
    dd_boil *menu = (dd_boil *) cd;

    menu->set_def_common_code(1);
}		

void dd_boil::set_def_common_code(int global_bit)
{

    Initialize(dd_boil::set_def_common_code);

    int no_of_selections = domain_list->num_selected();
    if(!no_of_selections) 
	msg("ERROR: Nothing Selected") << eom;
    else
	{
	char **d_list = (char **) (domain_list->selected());
        char *string = strdup(d_list[0]);
        int index = type_array.search(string);

	char *name = (char *) intermediate_array[index];
       	set_boilerplate_default(name);  
        set_default_bit();
        show_status ();
        global_bit_set[default_index()] = global_bit;	
      }
}
void dd_boil::opt_show_def_CB(gtPushButton*, gtEvent*, void* cd, gtReason)
{
    Initialize(dd_boil::opt_show_def_CB);	
    dd_boil *menu = (dd_boil *) cd;
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

void dd_boil::opt_rem_def_CB(gtPushButton*, gtEvent*, void* cd, gtReason)
{
  Initialize(dd_boil::opt_rem_def_CB);
  dd_boil* menu = (dd_boil *) cd;
  
  genString default_type;     

  int no_of_selections = menu->domain_list->num_selected();
  
  if(menu->boilerplate_type == "")
    msg("ERROR: Select a boilerplate type first") << eom;
  else
    {
      menu->get_boilerplate_default(default_type);
      
      if(default_type==(char*)NULL || default_type == "")
	msg("ERROR: No default set.") << eom;
      else
	{
          //This can be set for local variables only in this case
	  menu->set_boilerplate_default("");
          if(no_of_selections)
	    menu->domain_list->deselect_all();
	}
      menu->show_status ();
    }

}
int dd_boil::get_boilerplate_default(genString& default_type)
{
    Initialize(dd_boil::get_boilerplate_default);

    int		i, j;
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
    
	i = default_index();
                  
	  switch (i) {
        
		case 0 :default_type = dd_func_init_boilerplate;
                        break;
		case 1 :default_type = dd_func_final_boilerplate;
                        break;
		case 2 :default_type = dd_before_func_decl_boilerplate;
                        break;
		case 3 :default_type = dd_after_func_decl_boilerplate;
                        break;
		case 4 :default_type = dd_before_func_def_boilerplate;
                        break;
		case 5 :default_type = dd_after_func_def_boilerplate;
                        break;
		case 6 :default_type = dd_pub_decl_boilerplate;
                        break;
		case 7 :default_type = dd_prot_decl_boilerplate;
                        break;
		case 8 :default_type = dd_priv_decl_boilerplate;
                        break;
		case 9 :default_type = dd_before_class_decl_boilerplate;
                        break;
		case 10 :default_type = dd_after_class_decl_boilerplate;
                        break;
		case 11 :default_type = dd_before_class_def_boilerplate;
                        break;
		case 12 :default_type = dd_after_class_def_boilerplate;
                        break;
		case 13 :default_type = dd_before_struct_boilerplate;
                        break;
		case 14 :default_type = dd_after_struct_boilerplate;
                        break;
		case 15 :default_type = dd_member_def_init_boilerplate;
                        break;
		case 16 :default_type = dd_member_def_final_boilerplate;
                        break;
		case 17 :default_type = dd_before_member_def_boilerplate;
                        break;
		case 18 :default_type = dd_after_member_def_boilerplate;
                        break;
		case 19 :default_type = dd_before_member_decl_boilerplate;
                        break;
		case 20 :default_type = dd_after_member_decl_boilerplate;
                        break;
		case 21 :default_type = dd_relation_src_member_boilerplate;
                        break;
	        case 22 :default_type = dd_relation_src_header_boilerplate;
                        break;
	        case 23 :default_type = dd_relation_src_definition_boilerplate;
                        break;
		case 24 :default_type = dd_relation_trg_member_boilerplate;
                        break;
	        case 25 :default_type = dd_relation_trg_header_boilerplate;
                        break;
	        case 26 :default_type = dd_relation_trg_definition_boilerplate;
                        break;
		case 27 :default_type = dd_c_src_file_top_boilerplate;
			break;
		case 28 :default_type = dd_c_src_file_bottom_boilerplate;
			break;
		case 29 :default_type = dd_c_hdr_file_top_boilerplate;
			break;
		case 30 :default_type = dd_c_hdr_file_bottom_boilerplate;
			break;
		case 31 :default_type = dd_cpp_src_file_top_boilerplate;
			break;
		case 32 :default_type = dd_cpp_src_file_bottom_boilerplate;
			break;
		case 33 :default_type = dd_cpp_hdr_file_top_boilerplate;
			break;
		case 34 :default_type = dd_cpp_hdr_file_bottom_boilerplate;
			break;
		}
	return i;
}

int dd_boil::set_boilerplate_default(const char* name)
{

    Initialize(dd_boil::set_boilerplate_default);

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
    
        i = default_index();
          
	switch (i) {
        
		case 0 :dd_func_init_boilerplate = name;
                        break;
		case 1 :dd_func_final_boilerplate = name;
                        break;
		case 2 :dd_before_func_decl_boilerplate = name;
                        break;
		case 3 :dd_after_func_decl_boilerplate = name;
                        break;
		case 4 :dd_before_func_def_boilerplate = name;
                        break;
		case 5 :dd_after_func_def_boilerplate = name;
                        break;
		case 6 :dd_pub_decl_boilerplate = name;
                        break;
		case 7 :dd_prot_decl_boilerplate = name;
                        break;
		case 8 :dd_priv_decl_boilerplate = name;
                        break;
		case 9 :dd_before_class_decl_boilerplate = name;
                        break;
		case 10 :dd_after_class_decl_boilerplate = name;
                        break;
		case 11 :dd_before_class_def_boilerplate = name;
                        break;
		case 12 :dd_after_class_def_boilerplate = name;
                        break;
		case 13 :dd_before_struct_boilerplate = name;
                        break;
		case 14 :dd_after_struct_boilerplate = name;
                        break;
		case 15 :dd_member_def_init_boilerplate = name;
                        break;
		case 16 :dd_member_def_final_boilerplate = name;
                        break;
		case 17 :dd_before_member_def_boilerplate = name;
                        break;
		case 18 :dd_after_member_def_boilerplate = name;
                        break;
		case 19 :dd_before_member_decl_boilerplate = name;
                        break;
		case 20 :dd_after_member_decl_boilerplate = name;
                        break;
		case 21 :dd_relation_src_member_boilerplate = name;
                        break;
	        case 22 :dd_relation_src_header_boilerplate = name;
                        break;
	        case 23 :dd_relation_src_definition_boilerplate = name;
                        break;
		case 24 :dd_relation_trg_member_boilerplate = name;
                        break;
	        case 25 :dd_relation_trg_header_boilerplate = name;
                        break;
	        case 26 :dd_relation_trg_definition_boilerplate = name;
                        break;
	        case 27 :dd_c_src_file_top_boilerplate = name;
                        break;
	        case 28 :dd_c_src_file_bottom_boilerplate = name;
                        break;
	        case 29 :dd_c_hdr_file_top_boilerplate = name;
                        break;
	        case 30 :dd_c_hdr_file_bottom_boilerplate = name;
                        break;
	        case 31 :dd_cpp_src_file_top_boilerplate = name;
                        break;
	        case 32 :dd_cpp_src_file_bottom_boilerplate = name;
                        break;
	        case 33 :dd_cpp_hdr_file_top_boilerplate = name;
                        break;
	        case 34 :dd_cpp_hdr_file_bottom_boilerplate = name;
                        break;

		}
	return i;
}       


void dd_boil::init_type_array()
{
    Initialize (dd_boil::init_type_array);

    const char*	pc;
    projModule*	module;
    genString	phys, temp;
    GenArr	search_specs;
    int   	i;

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

void dd_boil::show_status (void)
{
    Initialize (dd_boil::show_status);

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

const char* dd_boil::add_type_array (projModule* module)
{
    Initialize (dd_boil::add_type_array);

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

void dd_boil::init_sorted_array(GenArr &temp_array)
{
    Initialize(dd_boil::init_sorted_array); 
     int num_of_items =type_array.size();   
     for(int j=0;j<num_of_items;j++)
	temp_array.append(type_array[j]);
     temp_array.sort();
}

void init_default_boilerplates(int from_cancel)
{
    Initialize(init_default_boilerplates);
    static int no_of_initialisations =1;

//If init_default_boilerplates is being called from cancel you would
// like to forcibly cause initialisation of all boilerplate types

    if(from_cancel)
      no_of_initialisations = 1;
    if(no_of_initialisations)
     {
	no_of_initialisations = 0;
	dd_func_init_boilerplate = customize::func_init();
 	dd_func_final_boilerplate = customize::func_final();
 	dd_before_func_decl_boilerplate = customize::before_func_decl();
 	dd_after_func_decl_boilerplate = customize::after_func_decl();
 	dd_before_func_def_boilerplate = customize::before_func_def();
 	dd_after_func_def_boilerplate = customize::after_func_def();
 	dd_pub_decl_boilerplate = customize::pub_decl();
 	dd_prot_decl_boilerplate = customize::prot_decl();
 	dd_priv_decl_boilerplate = customize::priv_decl();
 	dd_before_class_decl_boilerplate = customize::before_class_decl();
 	dd_after_class_decl_boilerplate = customize::after_class_decl();
 	dd_before_class_def_boilerplate = customize::before_class_def();
 	dd_after_class_def_boilerplate = customize::after_class_def();
 	dd_before_struct_boilerplate = customize::before_struct();
 	dd_after_struct_boilerplate = customize::after_struct();
	dd_member_def_init_boilerplate = customize::member_def_init() ;
        dd_member_def_final_boilerplate = customize::member_def_final();
	dd_before_member_def_boilerplate = customize::before_member_def();
        dd_after_member_def_boilerplate = customize::after_member_def();
        dd_before_member_decl_boilerplate = customize::before_member_decl();
        dd_after_member_decl_boilerplate = customize::after_member_decl();
	dd_relation_src_member_boilerplate = customize::relation_src_member();
	dd_relation_src_header_boilerplate =  customize::relation_src_header();
	dd_relation_src_definition_boilerplate =  customize::relation_src_definition();
	dd_relation_trg_member_boilerplate =  customize::relation_trg_member();
	dd_relation_trg_header_boilerplate =  customize::relation_trg_header();
	dd_relation_trg_definition_boilerplate =  customize::relation_trg_definition();

	dd_c_src_file_top_boilerplate = customize::c_src_file_top();
	dd_c_src_file_bottom_boilerplate = customize::c_src_file_bottom();
	dd_c_hdr_file_top_boilerplate = customize::c_hdr_file_top();
	dd_c_hdr_file_bottom_boilerplate = customize::c_hdr_file_bottom();
	dd_cpp_src_file_top_boilerplate = customize::cpp_src_file_top();
	dd_cpp_src_file_bottom_boilerplate = customize::cpp_src_file_bottom();
	dd_cpp_hdr_file_top_boilerplate = customize::cpp_hdr_file_top();
	dd_cpp_hdr_file_bottom_boilerplate = customize::cpp_hdr_file_bottom();

	for(int i = 0; i < No_of_dd_boilerplates; i++)
		global_bit_set[i] = 0;
    }
}

void dd_boil::OK_CB(gtPushButton*, gtEvent*, void* cd, gtReason)
{	
    Initialize(dd_boil::OK_CB);
    dd_boil* menu = (dd_boil *)cd;
    
    if(menu->is_default_bit_set()) {
	prop_all_changes_to_customize();
	customize::save_boilerplate();
    }

    menu->shell->popdown();
    delete menu; 
}


void dd_boil::Cancel_CB(gtPushButton*, gtEvent*, void* cd, gtReason)
{	
    Initialize(dd_boil::Cancel_CB);
    dd_boil* menu = (dd_boil *)cd;

     //Restore local variables from the memory variables 
        
      init_default_boilerplates(1);
       
    menu->shell->popdown();
    delete menu; 
}

static void prop_all_changes_to_customize()
{
    Initialize(prop_all_changes_to_customize);
    if(global_bit_set[0]) customize::func_init(dd_func_init_boilerplate);
  if(global_bit_set[1]) customize::func_final(dd_func_final_boilerplate);
  if(global_bit_set[2]) customize::before_func_decl(dd_before_func_decl_boilerplate);
  if(global_bit_set[3]) customize::after_func_decl(dd_after_func_decl_boilerplate);
  if(global_bit_set[4]) customize::before_func_def(dd_before_func_def_boilerplate);
  if(global_bit_set[5]) customize::after_func_def(dd_after_func_def_boilerplate);
  if(global_bit_set[6]) customize::pub_decl(dd_pub_decl_boilerplate);
  if(global_bit_set[7]) customize::prot_decl(dd_prot_decl_boilerplate);
  if(global_bit_set[8]) customize::priv_decl(dd_priv_decl_boilerplate);
  if(global_bit_set[9]) customize::before_class_decl(dd_before_class_decl_boilerplate);
  if(global_bit_set[10]) customize::after_class_decl(dd_after_class_decl_boilerplate);
  if(global_bit_set[11]) customize::before_class_def(dd_before_class_def_boilerplate);
  if(global_bit_set[12]) customize::after_class_def(dd_after_class_def_boilerplate);
  if(global_bit_set[13]) customize::before_struct(dd_before_struct_boilerplate);
  if(global_bit_set[14]) customize::after_struct(dd_after_struct_boilerplate);
  if(global_bit_set[15])  customize::member_def_init(dd_member_def_init_boilerplate) ;
  if(global_bit_set[16]) customize::member_def_final(dd_member_def_final_boilerplate) ;
  if(global_bit_set[17]) customize::before_member_def(dd_before_member_def_boilerplate) ;
  if(global_bit_set[18]) customize::after_member_def(dd_after_member_def_boilerplate) ;
  if(global_bit_set[19]) customize::before_member_decl(dd_before_member_decl_boilerplate) ;
  if(global_bit_set[20]) customize::after_member_decl(dd_after_member_decl_boilerplate) ;
  if(global_bit_set[21]) customize::relation_src_member(dd_relation_src_member_boilerplate) ;
  if(global_bit_set[22]) customize::relation_src_header(dd_relation_src_header_boilerplate) ;
  if(global_bit_set[23]) customize::relation_src_definition(dd_relation_src_definition_boilerplate) ;
  if(global_bit_set[24]) customize::relation_trg_member(dd_relation_trg_member_boilerplate) ;
  if(global_bit_set[25]) customize::relation_trg_header(dd_relation_trg_header_boilerplate) ;
  if(global_bit_set[26]) customize::relation_trg_definition(dd_relation_trg_definition_boilerplate) ;
  if(global_bit_set[27]) customize::c_src_file_top(dd_c_src_file_top_boilerplate);
  if(global_bit_set[28]) customize::c_src_file_bottom(dd_c_src_file_bottom_boilerplate);
  if(global_bit_set[29]) customize::c_hdr_file_top(dd_c_hdr_file_top_boilerplate);
  if(global_bit_set[30]) customize::c_hdr_file_bottom(dd_c_hdr_file_bottom_boilerplate);
  if(global_bit_set[31]) customize::cpp_src_file_top(dd_cpp_src_file_top_boilerplate);
  if(global_bit_set[32]) customize::cpp_src_file_bottom(dd_cpp_src_file_bottom_boilerplate);
  if(global_bit_set[33]) customize::cpp_hdr_file_top(dd_cpp_hdr_file_top_boilerplate);
  if(global_bit_set[34]) customize::cpp_hdr_file_bottom(dd_cpp_hdr_file_bottom_boilerplate);

}   

int dd_boil::default_index()
{
    Initialize(dd_boil::default_index);
    int i;
    for(i = 0; i < No_of_dd_boilerplates; i++)
	  if(boilerplate_type == func_array[i])
             break;
	return i;
}

genString dd_boil::insert_func_init()
{
    Initialize(dd_boil::insert_func_init);
    init_default_boilerplates(0);
    
    return dd_boil::insert_boilerplate(dd_func_init_boilerplate);
}

genString dd_boil::insert_func_final()
{
    Initialize(dd_boil::insert_func_init);

    init_default_boilerplates(0);
    return dd_boil::insert_boilerplate(dd_func_final_boilerplate);
}

genString dd_boil::insert_before_func_decl()
{
    Initialize(dd_boil::insert_before_func_decl);

    init_default_boilerplates(0);
    return dd_boil::insert_boilerplate(dd_before_func_decl_boilerplate);
}

genString dd_boil::insert_after_func_decl()
{
    Initialize(dd_boil::insert_after_func_decl);

    init_default_boilerplates(0);
    return dd_boil::insert_boilerplate(dd_after_func_decl_boilerplate);
}

genString dd_boil::insert_before_func_def()
{
    Initialize(dd_boil::insert_before_func_def);

    init_default_boilerplates(0);
    return dd_boil::insert_boilerplate(dd_before_func_def_boilerplate);
}

genString dd_boil::insert_after_func_def()
{
    Initialize(dd_boil::insert_after_func_def);

    init_default_boilerplates(0);
    return dd_boil::insert_boilerplate(dd_after_func_def_boilerplate);
}

genString dd_boil::insert_pub_decl()
{
    Initialize(dd_boil::insert_pub_decl);

    init_default_boilerplates(0);
    return dd_boil::insert_boilerplate(dd_pub_decl_boilerplate);
}

genString dd_boil::insert_priv_decl()
{
    Initialize(dd_boil::insert_priv_decl);

    init_default_boilerplates(0);
    return dd_boil::insert_boilerplate(dd_priv_decl_boilerplate);
}

genString dd_boil::insert_prot_decl()
{
    Initialize(dd_boil::insert_prot_decl);

    init_default_boilerplates(0);
    return dd_boil::insert_boilerplate(dd_prot_decl_boilerplate);
}

genString dd_boil::insert_before_class_decl()
{
    Initialize(dd_boil::insert_before_class_decl);
    init_default_boilerplates(0);
    return dd_boil::insert_boilerplate(dd_before_class_decl_boilerplate);
}

genString dd_boil::insert_after_class_decl()
{
    Initialize(dd_boil::insert_after_class_decl);

    init_default_boilerplates(0);
    return dd_boil::insert_boilerplate(dd_after_class_decl_boilerplate);
}


genString dd_boil::insert_before_class_def()
{
    Initialize(dd_boil::insert_before_class_def);

    init_default_boilerplates(0);
    return dd_boil::insert_boilerplate(dd_before_class_def_boilerplate);
}

genString dd_boil::insert_after_class_def()
{
    Initialize(dd_boil::insert_after_class_def);

    init_default_boilerplates(0);
    return dd_boil::insert_boilerplate(dd_after_class_def_boilerplate);
}

genString dd_boil::insert_before_struct()
{
    Initialize(dd_boil::insert_before_struct);

    init_default_boilerplates(0);
    return dd_boil::insert_boilerplate(dd_before_struct_boilerplate);
}

genString dd_boil::insert_after_struct()
{
    Initialize(dd_boil::insert_after_struct);

    init_default_boilerplates(0);
    return dd_boil::insert_boilerplate(dd_after_struct_boilerplate);
}

genString dd_boil::insert_member_def_init()
{
    Initialize(dd_boil::insert_member_def_init);

    init_default_boilerplates(0);
    return dd_boil::insert_boilerplate(dd_member_def_init_boilerplate);
}

genString dd_boil::insert_member_def_final()
{
    Initialize(dd_boil::insert_member_def_final);

    init_default_boilerplates(0);
    return dd_boil::insert_boilerplate(dd_member_def_final_boilerplate);
}

genString dd_boil::insert_before_member_def()
{
    Initialize(dd_boil::insert_before_member_def);

    init_default_boilerplates(0);
    return dd_boil::insert_boilerplate(dd_before_member_def_boilerplate);
}

genString dd_boil::insert_after_member_def()
{
    Initialize(dd_boil::insert_after_member_def);

    init_default_boilerplates(0);
    return dd_boil::insert_boilerplate(dd_after_member_def_boilerplate);
}


genString dd_boil::insert_before_member_decl()
{
    Initialize(dd_boil::insert_before_member_decl);

    init_default_boilerplates(0);
    return dd_boil::insert_boilerplate(dd_before_member_decl_boilerplate);
}

genString dd_boil::insert_after_member_decl()
{
    Initialize(dd_boil::insert_after_member_decl);

    init_default_boilerplates(0);
    return dd_boil::insert_boilerplate(dd_after_member_decl_boilerplate);
}

genString dd_boil::insert_relation_src_member()
{
    Initialize(dd_boil::insert_relation_src_member);

    init_default_boilerplates(0);
    return dd_boil::insert_boilerplate(dd_relation_src_member_boilerplate);
}


genString dd_boil::insert_relation_src_header()
{
    Initialize(dd_boil::insert_relation_src_header);

    init_default_boilerplates(0);
    return dd_boil::insert_boilerplate(dd_relation_src_header_boilerplate);
}


genString dd_boil::insert_relation_src_definition()
{
    Initialize(dd_boil::insert_relation_src_definition);

    init_default_boilerplates(0);
    return dd_boil::insert_boilerplate(dd_relation_src_definition_boilerplate);
}


genString dd_boil::insert_relation_trg_member()
{
    Initialize(dd_boil::insert_relation_trg_member);

    init_default_boilerplates(0);
    return dd_boil::insert_boilerplate(dd_relation_trg_member_boilerplate);
}


genString dd_boil::insert_relation_trg_header()
{
    Initialize(dd_boil::insert_relation_trg_header);

    init_default_boilerplates(0);
    return dd_boil::insert_boilerplate(dd_relation_trg_header_boilerplate);
}


genString dd_boil::insert_relation_trg_definition()
{
    Initialize(dd_boil::insert_relation_trg_definition);

    init_default_boilerplates(0);
    return dd_boil::insert_boilerplate(dd_relation_trg_definition_boilerplate);
}

genString dd_boil::insert_c_src_file_top()
{
    Initialize(dd_boil::insert_c_src_file_top);

    init_default_boilerplates(0);
    return dd_boil::insert_boilerplate(dd_c_src_file_top_boilerplate);
}

genString dd_boil::insert_c_src_file_bottom()
{
    Initialize(dd_boil::insert_c_src_file_bottom);

    init_default_boilerplates(0);
    return dd_boil::insert_boilerplate(dd_c_src_file_bottom_boilerplate);
}

genString dd_boil::insert_c_hdr_file_top()
{
    Initialize(dd_boil::insert_c_hdr_file_top);

    init_default_boilerplates(0);
    return dd_boil::insert_boilerplate(dd_c_hdr_file_top_boilerplate);
}

genString dd_boil::insert_c_hdr_file_bottom()
{
    Initialize(dd_boil::insert_c_hdr_file_bottom);

    init_default_boilerplates(0);
    return dd_boil::insert_boilerplate(dd_c_hdr_file_bottom_boilerplate);
}

genString dd_boil::insert_cpp_src_file_top()
{
    Initialize(dd_boil::insert_cpp_src_file_top);

    init_default_boilerplates(0);
    return dd_boil::insert_boilerplate(dd_cpp_src_file_top_boilerplate);
}

genString dd_boil::insert_cpp_src_file_bottom()
{
    Initialize(dd_boil::insert_cpp_src_file_bottom);

    init_default_boilerplates(0);
    return dd_boil::insert_boilerplate(dd_cpp_src_file_bottom_boilerplate);
}

genString dd_boil::insert_cpp_hdr_file_top()
{
    Initialize(dd_boil::insert_cpp_hdr_file_top);

    init_default_boilerplates(0);
    return dd_boil::insert_boilerplate(dd_cpp_hdr_file_top_boilerplate);
}

genString dd_boil::insert_cpp_hdr_file_bottom()
{
    Initialize(dd_boil::insert_cpp_hdr_file_bottom);

    init_default_boilerplates(0);
    return dd_boil::insert_boilerplate(dd_cpp_hdr_file_bottom_boilerplate);
}

genString dd_boil::insert_boilerplate(genString name)
{
    Initialize(dd_boil::insert_boilerplate);
    int i;

    genString original_string;

    original_string = "";
    char temp_string[513];	
    if((name == "") || (name == (char*)NULL))
	return ("");
  
    FILE *fd = OSapi_fopen((char *)name,"r");
    if(fd == NULL)
	return "";
    while((i = fread(temp_string,sizeof(char),512,fd)) >0)
	{
	  temp_string[i] = '\0';
	  original_string += temp_string;	
	}
    OSapi_fclose(fd);
    return (char *)original_string.str();
}

genString dd_boil::filter_boilerplate(genString original_string)
{
    Initialize(dd_boil::parse_boilerplate);
    
    if(original_string != (char*)NULL)
      {
	genString outstring;
        outstring = "";
	int k = original_string.length();
	int i = 0;
	
	while(i<k-1) // keep less than the actual string itself
	  {
	    if(original_string[i] == '$')
	      {
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
	    else
	      {
		outstring += original_string[i];
		i++;
	      }
	  }

        if (k)
	    outstring += original_string[k-1];

	return outstring;
      }
     else
       return NULL;
  }
void set_dd_class_name(genString name)
{
    Initialize(set_dd_class_name);
    dd_class_name = name;
}
void set_dd_function_name(genString name)
{
    Initialize(set_dd_function_name);
    dd_function_name = name;
}
void set_dd_struct_name(genString name)
{
    Initialize(set_dd_struct_name);
    dd_struct_name = name;
}
void set_dd_member_name(genString name)
{
    Initialize(set_dd_member_name);
    dd_member_name = name;
}
void set_dd_physical_file_name(genString name)
{
    Initialize(set_dd_physical_file_name);
    dd_physical_file_name = name;
}
void set_dd_logical_file_name(genString name)
{
       Initialize(set_dd_logical_file_name);
    dd_logical_file_name = name;
}
void set_dd_source_name(genString name)
{
 Initialize(set_dd_source_name);
    dd_source_name = name;
}
void set_dd_target_name(genString name)
{
    Initialize(set_dd_target_name);
    dd_target_name = name;
}
void set_dd_author_name(genString name)
{
 Initialize(set_dd_author_name);
    dd_author_name = name;
}
void set_dd_user_name(genString name)
{
 Initialize(set_dd_user_name);
    dd_user_name = name;
}
void set_dd_Date(genString name)
{
   Initialize(set_dd_Date);
   dd_Date = name;
   char* nlp = strchr(dd_Date, '\n');
   if (nlp) {
      *nlp = 0;		// don\'t include CR
   }
}
void set_dd_project_name(genString name)
{
 Initialize(set_dd_project_name);
    dd_project_name = name;
}




