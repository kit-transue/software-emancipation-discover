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
// gtRTL_util.h.C
//------------------------------------------
// synopsis:
// 
// Dialog box summoned from button at bottom of gtRTL
//------------------------------------------

// INCLUDE FILES

#include <gtRTL_util.h>
#ifndef ISO_CPP_HEADERS
#include <ctype.h>
#include <strstream.h>
#else /* ISO_CPP_HEADERS */
#include <cctype>
using namespace std;
#include <strstream>
#endif /* ISO_CPP_HEADERS */

#include <msg.h>
#include <messages.h>
#include <genWild.h>
#include <Relational.h>
#include <dialog.h>
#include <viewList.h>
#include <NewPrompt.h>
#include <Xm/Protocols.h>

#include <gtBaseXm.h>
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
#include <top_widgets.h>
#ifndef ISO_CPP_HEADERS
#include <stdio.h>
#else /* ISO_CPP_HEADERS */
#include <cstdio>
#endif /* ISO_CPP_HEADERS */
#include <attribute.h>
#include <scopeMgr.h>
#include <filterButtonTable.h>

#ifndef _metric_h
#include <metric.h>
#endif

// VARIABLE DEFINITIONS

static const char * END_OF_LIST = " * ";
 
static const char * BUT_INIT_TEXT = " ";

extern const char NEW_SORT_SPEC_SIG ;

// FUNCTION DECLARATIONS

static void refresh_format_menu(gtRTL_util *, format_info* info);
static void refresh_sort_menu(gtRTL_util *, sort_info* info);

genString renew_sort_spec(const char * spec);
void get_show_and_cli_filts(const char * str, genString& show, genString& cli);

// FUNCTION DEFINITIONS

gtRTL_util::gtRTL_util(gtRTL* rtl)
: gt_rtl(rtl)
{
    int i;
    for(i = 0; i < N_SORT_BOXES; ++i)
    {
	s_info[i].sort_field = 0;
	s_info[i].sort_direction = 0;
	s_info[i].me = this;
	s_info[i].sort_box = NULL;
    }
    for(i = 0; i < N_FORMAT_BOXES; ++i)
    {
	info[i].format_field = 0;
	info[i].width_edit = 0;
 	info[i].me =this;
	info[i].format_box = NULL;
    }

    viewListHeaderPtr vh = (viewListHeaderPtr)gt_rtl->dial()->get_view_ptr(0);

    genString show_filt, cli_filt, fexp;
    parse_specs(vh->format_spec(), vh->sort_spec(), vh->show_filter(), show_filt, cli_filt, fexp);

    shell = gtDialogTemplate::create (NULL, "list_utils", TXT("Filter List"));
    shell->add_button ("ok", TXT("OK"), OK_CB, this);
    shell->add_button ("cancel", TXT("Cancel"), Cancel_CB, this);
    shell->add_help_button();
    shell->help_context_name("Dialogs.FilterList.Help");

    // Add handler for deleting the window by pressing "X" button in title bar.
    Widget w = shell->rep()->widget();
    Atom atomDelete = XmInternAtom(XtDisplay(w), "WM_DELETE_WINDOW", False);
    XmAddWMProtocolCallback(w, atomDelete, Delete_CB, this);

    shell->resize_policy(gtResizeAny);

    gtForm* left = gtForm::create(shell, "form");
    left->attach(gtTop);
    left->attach(gtLeft);
    left->manage();

    gtForm* right = gtForm::create(shell, "form");
    right->attach(gtTop);
    right->attach(gtLeft, left, 10);
    right->manage();

    gtFrame* filter = gtFrame::create(left, "frame");
    filter->attach(gtTop);
    filter->attach(gtLeft);
    filter->attach(gtRight);
    filter->manage();

    gtVertBox* vbox;
    gtLabel* prompt;

    vbox = gtVertBox::create(filter, "form");
    vbox->manage();

    prompt = gtLabel::create(
	vbox, "prompt", TXT("Show Entries Matching"));
    prompt->manage();

    regexp = gtStringEditor::create(
	vbox, "regexp", (char*)show_filt);
    regexp->manage();

    prompt = gtLabel::create(
	vbox, "prompt", TXT("Hide Entries Matching"));
    prompt->manage();

    hide_regexp = gtStringEditor::create(
	vbox, "regexp", vh->hide_filter());
    hide_regexp->manage();

    gtFrame* format = gtFrame::create(left, "frame");
    format->attach(gtTop, filter, 10);
    format->attach(gtLeft);
    format->manage();

    vbox  = gtVertBox::create(format, "vbox");
    format_box = vbox;
    vbox->manage();

    prompt = gtLabel::create(
	vbox, "prompt", TXT("Format List"));
    prompt->manage();

    gtFrame* config = gtFrame::create(right, "frame");
    config->attach(gtTop);
    config->attach(gtLeft);
    config->attach(gtRight);
    config->manage();

    gtFrame* cli_frame = gtFrame::create(right, "frame");
    cli_frame->attach(gtTop, config, 10);
    cli_frame->attach(gtLeft);
    cli_frame->attach(gtRight);
    cli_frame->manage();

    vbox = gtVertBox::create(cli_frame, "form");
    vbox->manage();

    prompt = gtLabel::create(
	vbox, "prompt", TXT("Match Expression"));
    prompt->manage();

    cli_exp = gtStringEditor::create(
	vbox, "cli_exp", (char*)cli_filt);
    cli_exp->manage();

    gtFrame* form_frame = gtFrame::create(right, "frame");
    form_frame->attach(gtTop, cli_frame, 10);
    form_frame->attach(gtLeft);
    form_frame->attach(gtRight);
    form_frame->manage();

    vbox = gtVertBox::create(form_frame, "form");
    vbox->manage();

    prompt = gtLabel::create(
	vbox, "prompt", TXT("Use Format Expression"));
    prompt->manage();

    form_exp = gtStringEditor::create(
        vbox, "form_exp", (char*)fexp);
    form_exp->manage();

    gtFrame* sort = gtFrame::create(right, "frame");
    sort->attach(gtTop, form_frame, 10);
    sort->attach(gtLeft);
    sort->attach(gtRight);
    sort->manage();

    vbox = gtVertBox::create(sort, "vbox");
    sort_box = vbox;
    vbox->manage();

    prompt = gtLabel::create(
	vbox, "prompt", TXT("Sort By"));
    prompt->manage();

    gtForm* form = gtForm::create(config, "vbox");
    form->manage();

    c_data = new cfg_callback;
    c_data->thisp = this;
    c_data->spec = (gtRTL_spec*)def_table->spec[0];

    gtMenuBar* menubar = gtMenuBar::create(form, "menubar");
    menubar->attach(gtBottom, NULL, 10);
    menubar->attach(gtLeft, NULL, 10);
    menubar->manage();

    prompt = gtLabel::create(
	form, "prompt", TXT("Predefined Filters"));
    prompt->attach(gtBottom, menubar);
    prompt->attach(gtLeft);
    prompt->manage();

    gtCascadeButton* cbutt = gtCascadeButton::create(
	menubar, "cascade_button", TXT("Configure..."), NULL, NULL);
    
    
    gtPulldownMenu* menu = gtPulldownMenu::create(
	form, "menucascade",

	gtMenuStandard, "create_filter", TXT("Create New Filter..."),
	c_data, gtRTL_util::create_filter,

	gtMenuStandard, "rename_filter", TXT("Rename Filter..."),
	c_data, gtRTL_util::rename_filter,

	gtMenuStandard, "save_filter", TXT("Save Filter"),
	c_data, gtRTL_util::save_filter,

	gtMenuStandard, "create_filter", TXT("Delete Filter"),
	c_data, gtRTL_util::delete_filter,

	gtMenuNull);

    cbutt->sub_menu(menu);
    cbutt->manage();
    form->manage();

    filter_menu = gtOptionMenu::create(
	form, "menu", "", gtMenuNull);
    filter_menu->attach(gtTop, prompt);
    filter_menu->attach(gtLeft, menubar);
    filter_menu->manage();

    for(i = 0; i < def_table->spec.size(); ++i)
    {
	c_data->spec = (gtRTL_spec*)def_table->spec[i];
	filter_menu->insert_entries(-1, gtMenuStandard,
				    (char*)c_data->spec->name,
				    (char*)c_data->spec->name,
				    c_data, Config_CB, gtMenuNull);
    }
    c_data->spec = (gtRTL_spec*)def_table->spec[0];


    rehistory_menus();
    
    //Guy: set the predefined_filter_name from the viewListHeader
    predefined_filter_name = vh->get_predefined_filter_name();
    //set the menu to show the predefined filter
    if (predefined_filter_name.length() != 0)
	filter_menu->menu_history(predefined_filter_name);
    //End guy\'s change
    
}


gtRTL_util::~gtRTL_util() {     delete shell;     delete c_data; }


void gtRTL_util::popup()
{
    shell->popup(1);
}


void gtRTL::util_CB(gtPushButton*, gtEvent*, void* cd, gtReason)
{
    if(((gtRTL*)cd)->rtl) {
	gtRTL_util* menu = new gtRTL_util((gtRTL*)cd);
	menu->popup();
    }
}

void gtRTL::reset_CB(gtPushButton*, gtEvent*, void* cd, gtReason)
{
    RTLPtr rtlptr;
    if((rtlptr = ((gtRTL*)cd)->rtl)) {
	((gtRTL*)cd)->set_rtl_reset_filters(rtlptr);
    }
}


void gtRTL_util::Config_CB(gtPushButton* pb, gtEvent*, void* cd, gtReason)
//
// Called when the user picks an entry from the configuration option menu.
//
{
    cfg_callback* c = (cfg_callback*)cd;
    gtRTL_spec* sp = c->spec = def_table->get_entry(pb);
    c->thisp->reset_specs(sp->format, sp->sort, sp->filter, sp->hide);
    //Guy: set predefined_filter_nameP to be the name of the newly selected predefined filter
    //(first get the instance of the gtRTL_util)
    gtRTL_util* p = ((cfg_callback*)cd)->thisp;
    p->predefined_filter_name = (char*)sp->name;
    
}


//
// The next 4 functions are callbacks for the configure pulldown menu.
//

void gtRTL_util::create_filter(gtPushButton*, gtEvent*, void* cd, gtReason)
{
    gtRTL_util* p = ((cfg_callback*)cd)->thisp;
    gtRTL_spec* spec = ((cfg_callback*)cd)->spec;

    Prompt prompt(p->shell, TXT("Create Filter"), TXT("Enter name of filter:"));
    genString new_name;
    if(prompt.ask(new_name) <= 0)
	return;

    char* name = (char*)new_name;
    genString sort, format, filter;
    p->compute_specs(format, sort, filter);
    char* hide = p->hide_regexp->text();

    gtRTL_spec* new_spec = new gtRTL_spec(name, (char*)sort, (char*)format, (char*)filter, hide);
    cfg_callback* cb_struct = (cfg_callback*)cd;
    cb_struct->spec = new_spec;
    cb_struct->thisp = p;
    p->filter_menu->insert_entries(
	-1, gtMenuStandard, name, name, cb_struct, Config_CB, gtMenuNull);
    p->filter_menu->menu_history(name);
    def_table->add_entry(new_spec);

    gtFree(hide);
}


void gtRTL_util::rename_filter(gtPushButton*, gtEvent*, void* cd, gtReason)
{
    cfg_callback* cb_struct = (cfg_callback*)cd;

    gtRTL_util* p = NULL;
    if (cb_struct) p = cb_struct->thisp;

    gtRTL_spec* spec = NULL;
    if (p) spec = cb_struct->spec;

    // Get new name from the user.
    Prompt prompt(p->shell, TXT("Rename Filter"), TXT("Enter new name:"));

    genString newName;
    if (spec) newName = spec->name;

    gtPushButton* oldButton = NULL;
    if (p) oldButton = (gtPushButton*)(p->filter_menu->button((char*)newName));
    if (oldButton) {
        if(prompt.ask(newName) > 0) {
            // Remove the old button from the table.
            for(int i = 0; i < p->filter_menu->total_entries(); ++i) {
        	if (oldButton == p->filter_menu->entry(i))
        	    p->filter_menu->remove_entry(i);
            }

            // Rename the filter.
            def_table->rename_entry(spec, newName);

            // Create a new button, add it to the list, and display it.
            cb_struct->spec = spec;
            cb_struct->thisp = p;
            p->filter_menu->insert_entries( -1, gtMenuStandard,
                            (char*)newName, (char*)newName, cb_struct,
                            Config_CB, gtMenuNull);
            p->filter_menu->menu_history((char*)newName);
        }
    } else {
        msg("ERROR: Unable to rename filter \'$1\'.") << (char*) newName << eom;
    }
}


void gtRTL_util::save_filter(gtPushButton*, gtEvent*, void* cd, gtReason)
{
    gtRTL_util* p = ((cfg_callback*)cd)->thisp;
    gtRTL_spec* spec = ((cfg_callback*)cd)->spec;

    char* name = (char*)spec->name;
    genString format, sort, show;
    p->compute_specs(format, sort, show);
    char* hide = p->hide_regexp->text();

    gtRTL_spec* new_spec = new gtRTL_spec(name, (char*)sort, (char*)format, (char*)show, hide);
    cfg_callback* cb_struct = (cfg_callback*)cd;
    cb_struct->spec = new_spec;
    cb_struct->thisp = p;
    def_table->change_entry(spec, new_spec);

    gtFree(hide);
}

void gtRTL_util::delete_filter(gtPushButton*, gtEvent*, void* cd, gtReason)
{
    gtRTL_util* p = ((cfg_callback*)cd)->thisp;
    gtRTL_spec* spec = ((cfg_callback*)cd)->spec;
    if(def_table->spec.size() < 2) {
	msg("ERROR: Sorry, cannot delete last entry.") << eom;
	return;
    }

    def_table->remove_entry(spec);
    gtPushButton* button =
	(gtPushButton*)p->filter_menu->button( (char*)spec->name);
    for(int i = 0; i < p->filter_menu->total_entries(); ++i) {
	if (button == p->filter_menu->entry(i))
	    p->filter_menu->remove_entry(i);
    }
    p->filter_menu->menu_history(
	(char*) ((gtRTL_spec*)def_table->spec[0])->name);
}

#undef index

void RTL_parse_specs(format_info * info, sort_info * s_info, const char * format_spec, const char * sort_spec, const char * show_spec,
			     genString & show_filt, 
			     genString & cli_filt, genString & fexp)
{
    int i;
    for(i = 0; i < N_SORT_BOXES; ++i) {
	s_info[i].letters = "";
	s_info[i].forward = 1;
	s_info[i].index = i;
    }
    for (i = 0; i < N_FORMAT_BOXES; ++i) {
	info[i].letters = "";
	info[i].field_width = 0;
	info[i].index = i;
    }

    i = 0;
    fexp = "";
    if( !format_spec || !* format_spec) {
	info[0].letters = "on";
	fexp = "%";
    }
    else {
	while(*format_spec && i < N_FORMAT_BOXES) {
	    if(*format_spec++ == '%') {
		if(!*format_spec) break;
		fexp += '%';
		while(isdigit(*format_spec))
		{
		    info[i].field_width *= 10;
		    info[i].field_width += int(*format_spec) - int('0');
		    format_spec++;
		}
		while (*format_spec && *format_spec != ',' && !isspace(*format_spec) && *format_spec != '%') 
		  info[i].letters += *format_spec++;
		if (*format_spec == ',') format_spec++;
		++i;
	    }
	    else
	      fexp += *(format_spec - 1);
	}
    }

    i = 0;
    if( !sort_spec || !* sort_spec)
      s_info[0].letters = "on";
    else {
	genString new_spec = renew_sort_spec(sort_spec);
	sort_spec = (char*)new_spec;
	while(*sort_spec && i < N_SORT_BOXES) {
	    if(*sort_spec++ == '%') {
		if (*sort_spec++ == 'r') s_info[i].forward = 0; 
		else s_info[i].forward = 1;
		while(*sort_spec && *sort_spec != '%')
		  s_info[i].letters += *sort_spec++;
		++i;
	    }
	}
    }
    get_show_and_cli_filts(show_spec, show_filt, cli_filt);

}


void gtRTL_util::parse_specs(const char * format_spec, const char * sort_spec, const char * show_spec,
			     genString & show_filt, 
			     genString & cli_filt, genString & fexp)
{
    RTL_parse_specs(info, s_info, format_spec, sort_spec, show_spec, show_filt, cli_filt, fexp);
}


void gtRTL_util::reset_specs(
    char* format_spec, char* sort_spec, char* show_filter, char* hide_filter)
{
    genString show_filt, cli_filt, fexp;

    parse_specs(format_spec, sort_spec, show_filter, show_filt, cli_filt, fexp);

    regexp->text((char*)show_filt);
    hide_regexp->text(hide_filter);
    cli_exp->text((char*)cli_filt);
    form_exp->text((char*)fexp);

    rehistory_menus();
}


void gtRTL_util::rehistory_menus()
//
// Make the option menus correspond to the internal
// (info and s_info) representation of the sort/format specs
//
{
    gtStringEditor* se;

    int i;
    for(i = 0; i < N_FORMAT_BOXES; ++i)
    {
	if (info[i].letters[0]) {
	    if (!info[i].format_box) {
	 	create_format_menu (&info[i]);
	    }
	    info[i].format_field->label(abb_to_title(info[i].letters));
	}

	char buffer[5];
	sprintf(buffer, "%d", info[i].field_width);
	if (info[i].width_edit) {
	    se = info[i].width_edit;
	    se->text(buffer);
	}
    }

    for(i = 0; i < N_SORT_BOXES; ++i) {
	if (s_info[i].letters[0]) {
	    if (!s_info[i].sort_box) {
		create_sort_menu (&s_info[i]);
	    }
	    s_info[i].sort_field->label(abb_to_title(s_info[i].letters));
      	}
    }
	
    refresh_sort_menu(this, s_info);
    refresh_format_menu(this, info);
}

static void refresh_format_menu(gtRTL_util *dlg, format_info* info)
//
// Make sure that menus below the last "*" are set insensitive and cleared out.
//
{
    int grayed_out = 0;
    for(int i = info->index; i < N_FORMAT_BOXES; ++i, ++info) {

	if(grayed_out) {
	    info->letters = "";
	    if (info->format_box) info->format_box->unmanage();
	}

	else {
	    if (!info->format_box) {
	 	dlg->create_format_menu (info);
	    }

	    char* temp = info->width_edit->text();
	    if(!* temp)
	      info->width_edit->text("0");
	    gtFree (temp);
	    
	    info->format_box->manage();
	    info->width_edit->manage();
	}

	if(!grayed_out && strcmp(info->letters, "") == 0) {
	    grayed_out = 1;
	    if (!info->format_box) {
	 	dlg->create_format_menu (info);
	    }

	    info->format_field->label(END_OF_LIST);
	    info->width_edit->unmanage();
	    info->width_edit->text("");
	}
    }
}


static void refresh_sort_menu(gtRTL_util *dlg, sort_info* info)
{
    int grayed_out = 0;
    for(int i = info->index; i < N_SORT_BOXES; ++i, ++info) {

	if(grayed_out) {
	    info->letters = "";
	    if (info->sort_box) info->sort_box->unmanage();
	}

	else {
	    if (!info->sort_box) {
	 	dlg->create_sort_menu(info);
	    }

	    if (info->forward)
	      info->sort_direction->menu_history("sort_up");
	    else
	      info->sort_direction->menu_history("sort_down");

	    info->sort_box->manage();
	    info->sort_direction->manage();
	}

	if(!grayed_out && strcmp(info->letters, "") == 0)
	{
	    grayed_out = 1;
	    if (!info->sort_box) {
	 	dlg->create_sort_menu(info);
	    }

	    info->sort_field->label(END_OF_LIST);
	    info->forward = 1;
	    info->sort_direction->unmanage();
	}
    }
}


// Sort criteria option menu callbacks
void gtRTL_util::sort_CB(gtPushButton* but, gtEvent*, void* cd, gtReason)
{
    sort_info* info = (sort_info*)cd;
    char * temp = but->title();
    info->letters = title_to_abb(temp);
    info->sort_field->label(temp);
    gtFree(temp);
    refresh_sort_menu(info->me, info);
}

void gtRTL_util::no_sort_CB(gtPushButton*, gtEvent*, void* cd, gtReason)
{
    sort_info* info = (sort_info *)cd;
    info->letters = "";
    info->sort_field->label(END_OF_LIST);
    refresh_sort_menu(info->me, info);
}

void gtRTL_util::sort_ascending_CB(gtOptionMenu*, gtEvent*, void* cd, gtReason)
{
    sort_info* info = (sort_info *)cd;
    info->forward = 1;
    refresh_sort_menu(info->me, info);
}

void gtRTL_util::sort_descending_CB(gtOptionMenu*, gtEvent*, void* cd, gtReason)
{
    sort_info* info = (sort_info *)cd;
    info->forward = 0;
    refresh_sort_menu(info->me, info);
}

#define CB_BUTTON(abb) \
gtMenuStandard, abb, abb_to_title(abb), \
info, cb

static void make_attrs_menu(gtCascadeButton * but, void (*cb)(gtPushButton*, gtEvent*, void* cd, gtReason), void * info) {

    Initialize(make_attrs_menu);

    but->pulldown_menu(
	"attrs_menu",
        CB_BUTTON("type"),
	CB_BUTTON("args"),
	CB_BUTTON("numargs"),
	CB_BUTTON("kind"),
        CB_BUTTON("astcategory"),
	CB_BUTTON("fname"),
	CB_BUTTON("language"),
	CB_BUTTON("psetfile"),
        gtMenuNull
    );
}

objNameSet& get_all_extern_attributes(void);

static void make_user_attrs_menu(gtCascadeButton * but, void (*cb)(gtPushButton*, gtEvent*, void* cd, gtReason), void * info) {
    Initialize(make_user_attrs_menu);

    but->pulldown_menu("user_attrs_menu", gtMenuNull);
    objNameSet attrs = get_all_extern_attributes();
    Iterator it(attrs);
    Object *o;
    while(o = it.operator++()){
	Relational *el  = (Relational *) o;
	genString label = el->get_name();
	but->insert_entries(-1, gtMenuStandard, (char *)label, (char *)label, info, cb, gtMenuNull);
    }
}

static void make_metrics_menu(gtCascadeButton * but, void (*cb)(gtPushButton*, gtEvent*, void* cd, gtReason), void * info) {
    Initialize(make_attrs_menu);

    but->pulldown_menu("attrs_menu", gtMenuNull);

    for (int type = 0; type < NUM_OF_METRICS; type ++) {
	genString name = "met_";
	name += Metric::def_array[type]->abbrev;
	but->insert_entries(-1, CB_BUTTON((char*)name), gtMenuNull);
    }
}

#define SORT_BUTTON(abb) \
gtMenuStandard, abb, abb_to_title(abb), \
newInfo, gtRTL_util::sort_CB

gtForm* gtRTL_util::create_sort_menu(sort_info* newInfo) {

    gtForm* hbox = newInfo->sort_box = gtForm::create(sort_box, "hbox");
    hbox->attach(gtLeft);
    hbox->attach(gtRight);
    gtMenuBar * menubar = gtMenuBar::create(hbox, "menubar");
    menubar->attach(gtLeft);
    menubar->attach(gtTop);
    menubar->manage();
    gtCascadeButton* c_but = newInfo->sort_field = 
      gtCascadeButton::create(menubar, "sort_but", BUT_INIT_TEXT, NULL, NULL);
    
    c_but->pulldown_menu(
	"sort_menu",					
	gtMenuStandard, "no_sort", END_OF_LIST,
	newInfo, gtRTL_util::no_sort_CB,

	SORT_BUTTON("ff"),
	SORT_BUTTON("on"),
	SORT_BUTTON("of"),
	SORT_BUTTON("ol"),
	SORT_BUTTON("ox"),
	gtMenuCascade, "attrs", "Attribute", NULL, NULL,
	gtMenuCascade, "uattrs", "User Attribute", NULL, NULL,
	gtMenuCascade, "metrics", "Metric", NULL, NULL,

	gtMenuNull);
    gtCascadeButton * attrs_but = (gtCascadeButton *)c_but->button("attrs");
    make_attrs_menu(attrs_but, sort_CB, newInfo);

    gtCascadeButton * uattrs_but = (gtCascadeButton *)c_but->button("uattrs");
    make_user_attrs_menu(uattrs_but, sort_CB, newInfo);

    gtCascadeButton * metrics_but = (gtCascadeButton *)c_but->button("metrics");
    make_metrics_menu(metrics_but, sort_CB, newInfo);
    
    gtOptionMenu* rb = newInfo->sort_direction = gtOptionMenu::create(
	hbox, "sort_order", "",

	gtMenuStandard, "sort_up", TXT("Ascending"),
	newInfo, gtRTL_util::sort_ascending_CB,

	gtMenuStandard, "sort_down", TXT("Descending"),
	newInfo, gtRTL_util::sort_descending_CB,

	gtMenuNull);

    c_but->manage();

    rb->attach(gtRight);
    rb->attach(gtLeft, menubar, 10);
    rb->attach(gtTop);

    rb->manage();

    return hbox;
}

// format option menu callbacks
void gtRTL_util::format_CB(gtPushButton* but, gtEvent*, void* cd, gtReason)
{
    format_info* info = (format_info*)cd;
    char* but_title = but->title();
    info->letters = title_to_abb(but_title);
    info->format_field->label(but_title);
    gtFree(but_title);
    refresh_format_menu(info->me, info);
}

void gtRTL_util::no_format_CB(gtPushButton*, gtEvent*, void* cd, gtReason)
{
    format_info* info = (format_info*)cd;
    info->letters = "";
    info->format_field->label(END_OF_LIST);
    refresh_format_menu(info->me, info);
}

#define FORMAT_BUTTON(abb) \
gtMenuStandard, abb, abb_to_title(abb), \
newInfo, gtRTL_util::format_CB


gtHorzBox* gtRTL_util::create_format_menu (format_info* newInfo) {

    gtHorzBox* hbox = newInfo->format_box = gtHorzBox::create(format_box, "hbox");
    hbox->attach(gtLeft);
    hbox->attach(gtRight);
    gtMenuBar * menubar = gtMenuBar::create(hbox, "menubar");
    menubar->manage();
    gtCascadeButton* c_but = newInfo->format_field = 
      gtCascadeButton::create(menubar, "format_but", BUT_INIT_TEXT, NULL, NULL);
    c_but->pulldown_menu(
        "format_option",

        gtMenuStandard, "no_format", END_OF_LIST,
	newInfo, gtRTL_util::no_format_CB,

	FORMAT_BUTTON("on"),
	FORMAT_BUTTON("of"),
	FORMAT_BUTTON("ol"),
	gtMenuCascade, "attrs", "Attribute", NULL, NULL,
	gtMenuCascade, "uattrs", "User Attribute", NULL, NULL,
	gtMenuCascade, "metrics", "Metric", NULL, NULL,
				       
	gtMenuNull);

    gtCascadeButton * attrs_but = (gtCascadeButton *)c_but->button("attrs");
    make_attrs_menu(attrs_but, format_CB, newInfo);

    gtCascadeButton * uattrs_but = (gtCascadeButton *)c_but->button("uattrs");
    make_user_attrs_menu(uattrs_but, format_CB, newInfo);

    gtCascadeButton * metrics_but = (gtCascadeButton *)c_but->button("metrics");
    make_metrics_menu(metrics_but, format_CB, newInfo);
    
    c_but->manage();

    char buffer[16];
    sprintf(buffer, "%d", newInfo->field_width);

    gtStringEditor* width_ed = newInfo->width_edit =
	gtStringEditor::create(hbox, "format_width", "");
    width_ed->columns(4);

    width_ed->manage();

    return hbox;
}

static void RTL_compute_all_specs(format_info * info, 
				  sort_info * s_info,
				  char * show_text,
				  char * cli_text,
				  char * ftext,
				  genString & format_spec,
				  genString & sort_spec,
				  genString & show_spec) {
    Initialize(RTL_compute_all_specs);

    ostrstream stream;

    char * s = ftext;
    int i;
    for(i = 0; i < N_FORMAT_BOXES && info[i].letters[0]; ++i) {
	if (!*s && i>0) stream << " ";

	while(*s && *s != '%') stream << *s++;
	if (*s == '%') s++;

	stream << "%";
	if(info[i].field_width > 0)
	  stream << info[i].field_width;
	stream << (char*)info[i].letters << ",";
    }
    while(*s && !(*s == '%' || (*s == ' ' && *(s + 1) == '%') ) ) {
	stream << *s++;
    }
    
    stream << ends;

    char * str = stream.str();
    format_spec = str;
    delete str;

    show_spec = show_text;
    if (*cli_text) {
	show_spec += '\t';
	show_spec += cli_text;
    }
    
    ostrstream s2;
    s2 << NEW_SORT_SPEC_SIG;
    for(i = 0; i < N_SORT_BOXES && s_info[i].letters[0]; ++i) {
	s2 << "%" ;
	if(s_info[i].forward)
	  s2 << 'f';
	else
	  s2 << 'r';
	s2 << (char*)s_info[i].letters;
    }
    
    s2 << ends;
    str = s2.str();
    sort_spec = str;
    delete str;
}


// store the current criteria in strings in the universal system format
void gtRTL_util::compute_specs(genString& format_spec, genString & sort_spec,
				genString& show_spec)
{
    
    for(int i = 0; i < N_FORMAT_BOXES && info[i].letters[0]; ++i) {
	char* string = info[i].width_edit->text();
	sscanf(string, "%d", &info[i].field_width);
	gtFree(string);
    }	

    char * ftext = form_exp->text();
    char * cli_text = cli_exp->text();
    char * filter = regexp->text();


    RTL_compute_all_specs(info, s_info, filter, cli_text, ftext, 
			  format_spec, sort_spec, show_spec);
    gtFree( ftext);
    gtFree( cli_text);
    gtFree( filter);
}

void gtRTL_util::apply()
{
    push_busy_cursor();
    gtRTL* p = gt_rtl;
    viewListHeaderPtr vh = (viewListHeaderPtr)p->dial()->get_view_ptr(0);

    genString f_spec, s_spec, show_spec;
    compute_specs(f_spec, s_spec, show_spec);

    vh->format_spec((char*)f_spec);
    vh->sort_spec((char*)s_spec);
    vh->show_filter((char*)show_spec);

    char* hide_string = hide_regexp->text();
    vh->hide_filter(hide_string);
    gtFree(hide_string);

    //Guy: set the name of the predefined filter in the viewListHeader
    vh->set_predefined_filter_name((char*)predefined_filter_name);
    
    vh->regenerate_view();
    pop_cursor();
}

extern void cmd_filter_OK_CB_report ();

void gtRTL_util::OK_CB(gtPushButton*, gtEvent*, void* cd, gtReason)
{
    cmd_filter_OK_CB_report();

    gtRTL_util* menu = (gtRTL_util*)cd;
    menu->apply();
    delete menu;
}


void gtRTL_util::Apply_CB(gtPushButton*, gtEvent*, void* cd, gtReason)
{
    gtRTL_util* menu = (gtRTL_util*)cd;
    menu->apply();
}


void gtRTL_util::Cancel_CB(gtPushButton*, gtEvent*, void* cd, gtReason)
{
    gtRTL_util* menu = (gtRTL_util*)cd;
    delete menu;
}

void gtRTL_util::Delete_CB(Widget w, XtPointer client_data, XtPointer call_data)
{
    gtRTL_util* menu = (gtRTL_util*)client_data;
    delete menu;
}

/*
   START-LOG-------------------------------------------

   $Log: gtRTL_util.h.C  $
   Revision 1.27 2002/04/29 11:41:19EDT ktrans 
   Bug track: 20510.  When cleaning up format specs with extra format specifiers, check for an additional space, too.
// Revision 1.15  1994/08/05  14:25:26  farber
// Bug track: 0000
// change to work on HP
//
// Revision 1.14  1994/08/04  14:11:37  farber
// Bug track: 7717
// insert appropriate information into the viewlist header
//
// Revision 1.13  1994/04/13  15:01:16  bhowmik
// Replaced Container name with Filename in sort options in
// Filter Dialog box.
//
// Revision 1.12  1994/04/13  13:35:15  bhowmik
// Container name is no longer displayed in Filter Dialog .
//
// Revision 1.11  1994/02/26  22:09:29  builder
// Reparent filter dialog box so that the normal font would be used instead of
// the icon font. (bug 6529) -kws
//
// Revision 1.10  1994/01/21  00:23:14  kws
// New rtl display
//
// Revision 1.9  1993/12/31  19:38:36  boris
// Bug track: Test Validation
// Test Validation
//
// Revision 1.8  1993/11/12  20:16:00  andrea
// Bug track: n/a
// I added functionality to display line numbers for objects
//
// Revision 1.7  1993/06/25  22:10:53  bakshi
// #undef index for c++3.0.1 port
//
// Revision 1.6  1993/06/22  22:41:58  kws
// Remove apply button
//
// Revision 1.5  1993/06/18  21:43:28  andrea
// added hour glass when waiting for filter to complete
//
// Revision 1.4  1993/06/02  20:32:25  glenn
// Remove checked_cast macros.
//
// Revision 1.3  1993/06/02  20:25:59  glenn
// Remove Initialize macros.  Remove genError.h.
//
// Revision 1.2  1993/05/01  17:51:34  jon
// removed 'regexp' from string labels
//
// Revision 1.1  1993/04/30  18:50:40  glenn
// Initial revision
//
   END-LOG---------------------------------------------
*/

