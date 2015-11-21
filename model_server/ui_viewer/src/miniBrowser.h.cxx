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
#include <XrefQuery.h>

#include <cLibraryFunctions.h>
#include <msg.h>
#include <genError.h>
#include <systemMessages.h>

#include <gtShell.h>
#include <gtForm.h> 
#include <objOper.h>
#include <RTL.h>
#include <symbolPtr.h>
#include <proj.h>
#include <ddict.h>
#include <path.h>
#include <closure.h>
#include <Interpreter.h>
#include <SetsUI.h>

#include <gtBaseXm.h>
#include <gtArrowButton.h>
#include <gtPanedWindow.h>
#include <gtNoParent.h>
#include <gtTogB.h>
#include <gtRadioBox.h>
#include <gtPushButton.h>
#include <gtStringEd.h>
#include <gtRTL.h>
#include <gtCascadeB.h>
#include <gtPDMenu.h>
#include <gtMenuBar.h>
#include <gtSepar.h>
#include <gtOptionMenu.h>
#include <gtBitmap.h>

#include <Xm/RowColumn.h>
#include <Xm/Separator.h>
#include <Xm/LabelG.h>
#include <Xm/Text.h>
#include <Xm/Form.h>
#include <Xm/PushB.h>

#include <Question.h>
#include <viewerShell.h>
#include <ste_interface.h>
#include <browserShell.h>
#include <top_widgets.h>
#include <view_creation.h>
#include <driver.h>
#include <X11/StringDefs.h>
#include <xref_queries.h>
#include <miniBrowser.h>
#include <instanceBrowser.h>
#include <dd_utils.h>

#ifndef _groupHdr_h
#include <groupHdr.h>
#endif

#include <LanguageController.h>
#include <BrowserController.h>
#include <BrowserData.h>
#include <cliUI.h>
#include <Interpreter.h>

#define mb_on_width   16
#define mb_on_height 16
static char mb_on_bits[] = {
   0xf8, 0x1f, 0xf8, 0x1f, 0x08, 0x10, 0x08, 0x10, 0x08, 0x10, 0x08, 0x10,
   0x08, 0x10, 0x08, 0x10, 0x58, 0x15, 0x08, 0x10, 0x08, 0x10, 0x08, 0x10,
   0x08, 0x10, 0x08, 0x10, 0x08, 0x10, 0xf8, 0x1f};

#define mb_off_width  16
#define mb_off_height 16
static char mb_off_bits[] = {
   0xf8, 0x9f, 0xf8, 0x5f, 0x08, 0x30, 0x08, 0x10, 0x08, 0x18, 0x08, 0x14,
   0x08, 0x12, 0x08, 0x11, 0xd8, 0x15, 0x48, 0x10, 0x28, 0x10, 0x18, 0x10,
   0x08, 0x10, 0x0c, 0x10, 0x0a, 0x10, 0xf9, 0x1f};


extern     projModulePtr projModule_of_symbol(symbolPtr& sym);
extern "C" void vs_display_mini_browser(Widget, int);
extern "C" void fill_selected_objects (symbolArr *selection);    // ProjectQuery.C

// ProjectQuery.C
extern     int save_array_into_file(symbolArr &array, FILE *svfp);

class FileNameDlg {
  public:
    FileNameDlg(RTL *r);
    ~FileNameDlg();
  private:
    static void ok_CB(gtPushButton*, gtEventPtr, void*, gtReason);
    static void cancel_CB(gtPushButton*, gtEventPtr, void*, gtReason);
    gtDialogTemplate *dlg;
    gtStringEditor   *file_name;
    gtToggleButton   *toggle;
    RTL              *rtl_to_write;
};

genArr(miniBrowserPtr);
static genArrOf(miniBrowserPtr) miniBrowser_instances;

class ProjectController : public miniBrowserRTL_API  {
  public:
    ProjectController(projNode *node, miniBrowser *browser);
    ~ProjectController();
    
    virtual void make_own_buttons(gtBase *parent, gtRTL *, RTL *rtl);
    virtual void rtl_out_of_scope(RTL *rtl);
  private:
    static void UpButton(gtPushButton*, gtEvent*, void* pc, gtReason);
    
    projNode    *node;
    miniBrowser *browser;
};

class ListHistoryEntry : public Obj {
  public:
    ~ListHistoryEntry();

    RTL *rtl;
    int top_position;
    int list_number;   // Used to put it in the original list during remove operation
};

class RTLGarbager : public Relational {
  public:
    RTLGarbager() {} ;
};

defrel_many_to_many(RTLGarbager,trash_can,RTL,rtl);
init_rel_or_ptr(RTLGarbager,trash_can, 0,RTL,rtl, 0);

init_rel_or_ptr(miniBrowserRTL_API,APIhandler, relationMode::D,RTL,rtl, 0);

RTLGarbager garbage;
RTLGarbager controlled_rtls;

static GC        miniBrowser_GC;
static int       miniBrowser_gc_initialized = 0;
static Dimension miniBrowser_height;
static int       miniBrowser_button_x;
static int       miniBrowser_xdiff;
static int       do_dragging = 0;

typedef struct {
    Widget    button;
    Widget    separator;
    Dimension width;
    int       sep_pos;
    int       button_pos;
} miniBrowser_event_data;

//---------------------------------------------------------------

static RTL *copy_create_rtl(RTL *src_rtl)
{
    Initialize(copy_rtl);
    
    if(!src_rtl)
	return NULL;
    RTLNodePtr old_node = checked_cast(RTLNode, src_rtl->get_root());
    // Create & copy rtl to new one to allow later deletion.
    RTL *new_rtl = db_new(RTL, (NULL));
    new_rtl->set_name(src_rtl->get_name());
    new_rtl->set_phys_name(src_rtl->get_phys_name());
    RTLNodePtr rtl_node    = checked_cast(RTLNode, new_rtl->get_root());
    symbolArr old_contents = old_node->rtl_contents();
    for(int i = 0; i < old_contents.size(); i++){
	rtl_node->rtl_insert(old_contents[i]);
    }
    return new_rtl;
}

void handle_rtl_deletion(RTL *rtl)
{
    Initialize(handle_rtl_deletion);

    objSet cans = rtl_get_trash_cans(rtl);
    Obj    *ob;
    ForEach(ob, cans){
	RTLGarbager *can = (RTLGarbager *)ob;
	// Delete rtl if it was created by ourselfs
	if(can == &garbage){
	    miniBrowserRTL_API *api = rtl_get_APIhandler(rtl);
	    if(api){
		if(api->rtl_dying(rtl)){  // Ok to delete rtl
		    obj_unload(rtl);
		    break;
		}
	    }else{
		obj_unload(rtl);
		break;
	    }
	}else
	    if(can == &controlled_rtls){
		trash_can_rem_rtl(&controlled_rtls, rtl);
	    }
    }
}

//---------------------------------------------------------------

ProjectController::ProjectController(projNode *p_node, miniBrowser *p_browser)
{
    this->node    = p_node;
    this->browser = p_browser;
}

ProjectController::~ProjectController()
{
//    printf("ProjectController dying.\n");
//    OSapi_fflush(stdout);
}

void ProjectController::rtl_out_of_scope(RTL *)
{
    Initialize(ProjectController::rtl_out_of_scope);
    
    // Since nothing to control anymore delete itself
    delete this;
}

void ProjectController::make_own_buttons(gtBase *parent, gtRTL *, RTL *rtl)
{
    Initialize(ProjectController::make_own_buttons);
    
    if(rtl != projNode::get_project_rtl()){
	gtPushButton *up_button = gtPushButton::create(parent, "up_button", "Up", 
						       ProjectController::UpButton, this);
	add_button(up_button);
    }
}

void ProjectController::UpButton(gtPushButton*, gtEvent*, void* data, gtReason)
{
    Initialize(ProjectController::UpButton);
    
    ProjectController *pc = (ProjectController *)data;
    if(!pc->node)
	return;
    projNode *parent_node = (projNode *)get_relation(parentProject_of_childProject, pc->node);
    if(!parent_node){
	RTL *rtl = projNode::get_project_rtl();
	pc->browser->browse(rtl);
	return;
    }
    projHeader *header = checked_cast(projHeader, parent_node->get_header());
    if(!header)
	return;
    ProjectController *new_pc = new ProjectController(parent_node, pc->browser);
    APIhandler_put_rtl(new_pc, header);
    pc->browser->browse(header);
}

//---------------------------------------------------------------

ListHistoryEntry::~ListHistoryEntry()
{
    handle_rtl_deletion(rtl);
}

//---------------------------------------------------------------

FileNameDlg::FileNameDlg(RTL *r)
{
    Initialize(FileNameDlg::FileNameDlg);

    rtl_to_write = r;
    // Create the dialog.
    dlg = gtDialogTemplate::create(NULL, "new_file", TXT("Write List"));
    dlg->add_button("ok", TXT("OK"), ok_CB, this);
    dlg->add_button("cancel", TXT("Cancel"), cancel_CB, this);
//    dlg->add_help_button();
//    dlg->help_context_name("Browser.File.NewFile.Help");

    // Create the directory label.
    gtLabel* label = gtLabel::create(dlg, "label", TXT("Enter file name:"));
    label->alignment(gtBeginning);
    label->attach(gtLeft);
    label->attach(gtRight);
    label->attach(gtTop);
    label->manage();

    // Create the directory answer text box.
    file_name = gtStringEditor::create(dlg, "answer", NULL);
    const int file_columns = 35;  // max(30, strlen(current_name) + 15);
    file_name->columns(file_columns);
    file_name->attach(gtTop, label, 10);
    file_name->attach(gtLeft);
    file_name->attach(gtRight);
    file_name->manage();
    
    toggle = gtToggleButton::create(dlg, "toggle", "Logical/Physical name", NULL, NULL);
    toggle->attach(gtTop, file_name, 10);
    toggle->attach(gtLeft);
    toggle->set(1, 0);
    toggle->manage();

    dlg->popup(1);
    dlg->set_default_parent(NULL);
}

void FileNameDlg::ok_CB(gtPushButton*, gtEventPtr, void* data, gtReason)
{
    Initialize(FileNameDlg::ok_CB);

    FileNameDlg* fd = (FileNameDlg*)data;

    char* file_name = fd->file_name->text();
    genString full_name, mod_ln;
    projNode* home = projNode::get_home_proj();
    if(!fd->toggle->set()){
	project_convert_filename(file_name, full_name);
	home->fn_to_ln(full_name, mod_ln, FILE_TYPE, 0, 1);
    }else{
	home->ln_to_fn_imp(file_name, full_name, FILE_TYPE, 0, 1);
	if(!full_name.length()){
	    msg("Unable to convert logical name into physical path.", error_sev) << eom;
	    return;
	}
	home->fn_to_ln(full_name, mod_ln, FILE_TYPE, 0, 1);
    }
    if (!mod_ln.length()){
	genString msg;
	msg.printf("No mapping for %s in pdf file.", (char *)full_name);
	gtPushButton::next_help_context("Pset.Help.Groups.NoMapping");
	int answer = popup_Question("Write", (const char *)msg.str(), "Save anyway", "Cancel");
	if(answer != 1){
	    gtFree(file_name);
	    return;
	}
    }else{
	projModule *module;
	if (home->is_script())
	{
	    projNode::create_global_script_modules(full_name, FILE_LANGUAGE_UNKNOWN, 0);
	    module = home->find_module(mod_ln);
	}else
	    module = home->make_module(mod_ln);
	if(module)
	    module->update_module();
    }
    FILE *svfp = OSapi_fopen((char *)full_name, "w");
    if (!svfp) {
	msg("Unable to create file: $1", error_sev) << file_name << eom;
	gtFree(file_name);
	return;
    }
    RTLNodePtr rtl_node = checked_cast(RTLNode, fd->rtl_to_write->get_root());
    symbolArr& contents = rtl_node->rtl_contents();
    save_array_into_file(contents, svfp);
    OSapi_fclose(svfp);
    gtFree(file_name);
    delete fd;
}

FileNameDlg::~FileNameDlg()
{
    delete dlg;
}

void FileNameDlg::cancel_CB(gtPushButton*, gtEventPtr, void* data, gtReason)
{
    Initialize(FileNameDlg::cancel_CB);

    delete (FileNameDlg*)data;
}

//---------------------------------------------------------------

miniBrowserRTL_API::miniBrowserRTL_API()
{
    list = NULL;
}

void miniBrowserRTL_API::selection_callback(RTL *, symbolArr&, miniBrowser *)
{
}

int miniBrowserRTL_API::action_callback(RTL *, symbolArr&, miniBrowser *)
{
    return 1;
}

int miniBrowserRTL_API::rtl_dying(RTL *)
{
    return 1;
}

void miniBrowserRTL_API::rtl_out_of_scope(RTL *)
{
}

char *miniBrowserRTL_API::get_sort_spec(RTL *)
{
    return NULL;
}

int miniBrowserRTL_API::need_vcr_control(RTL *)
{
    return 0;
}

void miniBrowserRTL_API::make_own_buttons(gtBase *, gtRTL *, RTL *)
{
}

void miniBrowserRTL_API::set_list(gtList *l)
{
    list = l;
}

void miniBrowserRTL_API::add_button(gtPrimitive *prim)
{
    if(list){
	list->add_button(prim);
    }
}

gtBase *miniBrowserRTL_API::get_button_control(void)
{
    if(list)
	return list->get_button_form();
    else
	return NULL;
}

// --------------------------------------------------------------

miniBrowser::miniBrowser(void *parent_menu, void *parent_icon_place, void *parent)
{
    Initialize(miniBrowser);

    displayed          = 1;
    parent_viewershell = parent;
    
    gtNoParent *parent_icon = gtNoParent::create(parent_icon_place);
    gtBitmap   *icon_on     = gtBitmap::create(parent_icon, "browser_on", mb_on_bits, mb_on_width, mb_on_height);
    gtBitmap   *icon_off    = gtBitmap::create(parent_icon, "browser_off", mb_off_bits, mb_off_width, mb_off_height);
    icon_on->manage();
    icon_off->manage();
   
    on_button = gtPushButton::create(parent_icon, "icon_on", icon_on,
				     ToggleBrowser, this);
    on_button->width(29);
    on_button->height(29);
    on_button->attach(gtLeft);
    on_button->attach(gtTop);
    on_button->attach(gtBottom);
    on_button->attach(gtRight);
    on_button->manage();
    off_button = gtPushButton::create(parent_icon, "icon_off", icon_off,
				      ToggleBrowser, this);
    off_button->width(29);
    off_button->height(29);
    off_button->attach(gtLeft);
    off_button->attach(gtTop);
    off_button->attach(gtBottom);
    off_button->attach(gtRight);
    
    mainForm = gtForm::create((Widget)parent, "mini_main");
    mainForm->attach(gtTop);
    mainForm->attach(gtBottom);
    mainForm->attach(gtLeft);
    mainForm->attach(gtRight);
    mainForm->manage();

    gtMenuBar *menubar = gtMenuBar::create(mainForm, "query_menu");
    menubar->attach(gtTop);
    menubar->attach(gtLeft);
    menubar->attach(gtRight);
    menubar->manage();

    gtNoParent *parent_menubar = gtNoParent::create(parent_menu);

    /*
    gtCascadeButton* popup_browser = gtCascadeButton::create(
	parent_menubar, "popup_browser", TXT("Minibrowser(ON) "), PopupBrowser, this);
    popup_browser->manage();
    */

    query_menu = gtCascadeButton::create(menubar, "query", TXT("Query"), NULL, NULL);
    num_queries = 0;

    query_menu->pulldown_menu("query_pulldown", gtMenuNull);

    query_menu->event_handler(EnterWindowMask, FALSE, 
			      (gtEventHandler) update_query_menu_CB, this);

    query_menu->insert_entries(-1, gtMenuCheck, "closure", TXT("Closure"), this,
			       closure_CB, gtMenuNull);

    pBController = LanguageController::GetBrowserController();

    closure_button = (gtToggleButton*) query_menu->entry(0);
    closure_button->set(get_closure_state(), 0);

    query_menu->manage();
    
    history_button = gtCascadeButton::create(menubar, "history", TXT("History"), NULL, NULL);
    history_button->pulldown_menu("history_pulldown",
				  gtMenuNull);
    history_button->manage();

    group_button = gtCascadeButton::create(menubar, "group", TXT("Group"), 
					   NULL, NULL);
//					   (gtCascadeCB)GroupButton, this);
    group_button->pulldown_menu("group_pulldown",
				gtMenuStandard, "group_create", TXT("Create Group"), this, GroupButton,
				gtMenuStandard, "group_create", TXT("Group Manager"), this, GroupManagerButton,
//				gtMenuStandard, "list_save_top", TXT("Write top list..."), this, SaveList1_CB,
//				gtMenuStandard, "list_save_bottom", TXT("Write bottom list..."), this, SaveList2_CB,
				gtMenuNull);
    
    group_button->manage();

    gtSeparator* separ = gtSeparator::create(mainForm, "sep", gtHORZ);
    separ->attach(gtLeft);
    separ->attach(gtRight);
    separ->attach(gtTop, menubar, 3);
    separ->manage();
    
    paned = gtPanedWindow::create(mainForm, "paned_window");
    paned->attach(gtTop, separ, 3);
    paned->attach(gtLeft);
    paned->attach(gtRight);
    paned->attach(gtBottom);
    paned->width(100);
    paned->manage();
    
    list1Form = gtForm::create(paned, "list_1_form");
    list1Form->attach(gtTop);
    list1Form->attach(gtLeft);
    list1Form->attach(gtRight);
    list1Form->manage();

//    list1 = gtRTL::create(paned, "list_1", " ", gtExtended, NULL, 0);
    list1 = gtRTL::create(list1Form, "list_1", " ", gtExtended, NULL, 0);
    list1->handle_special_buttons(1);
    list1->action_callback(list1_action, this);
    list1->select_callback(list1_select, this);
    list1->attach(gtLeft);
    list1->attach(gtRight);
    list1->attach(gtTop);
    list1->attach(gtBottom);
    list1->num_rows(8);
    list1->manage();
    RTLPtr rtl     = projNode::get_project_rtl();
//    RTLPtr new_rtl = copy_create_rtl(rtl);
    RTLPtr new_rtl = rtl;
    new_rtl->set_name("Top Level Projects");
    list1->set_rtl(new_rtl);
    add_rtl_to_history(new_rtl, 1);
    title1("Top Level Projects");
    selected_list = list1;

    list2Form = gtForm::create(paned, "list_1_form");
    list2Form->attach(gtTop);
    list2Form->attach(gtLeft);
    list2Form->attach(gtRight);

//    list2 = gtRTL::create(paned, "list_2", " ", gtExtended, NULL, 0);
    list2 = gtRTL::create(list2Form, "list_2", " ", gtExtended, NULL, 0);
    list2->handle_special_buttons(1);
    list2->action_callback(list2_action, this);
    list2->select_callback(list2_select, this);
    list2->attach(gtTop);
    list2->attach(gtLeft);
    list2->attach(gtRight);
    list2->attach(gtBottom);
    list2->manage();
    Dimension height;
    XtVaGetValues(paned->container()->rep()->widget(), XmNheight, &height, NULL);
    XtVaSetValues(list2Form->rep()->widget(), XmNpaneMinimum, height / 2, NULL);
    list2Form->manage();
    XtVaSetValues(list2Form->rep()->widget(), XmNpaneMinimum, 100, NULL);

//    XtVaSetValues(list2->container()->rep()->widget(), XmNpaneMinimum, height / 2, NULL);
//    list2->manage();
//    XtVaSetValues(list2->container()->rep()->widget(), XmNpaneMinimum, 100, NULL);

    list1_prev_rtl        = NULL;
    list2_prev_rtl        = NULL;
    unmanaged_list        = NULL;
    new_form              = NULL;
    form_cleanup_callback = NULL;
    cleanup_callback_data = NULL;
    miniBrowserPtr p      = this;
    miniBrowser_instances.append(&p);
}

miniBrowser::~miniBrowser()
{
    if(unmanaged_list == NULL){
	if(form_cleanup_callback)
	    form_cleanup_callback(this, cleanup_callback_data);
	restore_list();
    }
    int i;
    for(i = 0; i < miniBrowser_instances.size(); i++){
	miniBrowser *mb = *miniBrowser_instances[i];
	if(mb == this){
	    miniBrowser_instances.remove(i);
	    break;
	}
    }
    /* This code will not work... but it\'s good explanation that is going on below 
    RTL *rtl = list1->RTL();
    if(rtl)
	handle_rtl_deletion(rtl);
    rtl = list2->RTL();
    if(rtl)
	handle_rtl_deletion(rtl);
    */
    list1 = NULL;
    list2 = NULL;
    for(i = 0; i < history_list.size(); i++){
	ListHistoryEntry *entry = (ListHistoryEntry *)history_list[i];
	report_out_of_scope(entry->rtl);
	delete entry;
    }
    objSet rtls = trash_can_get_rtls(&garbage);
    ObjPtr ob;
    ForEach(ob, rtls){
	RTL *rtl = (RTL *)ob;
	handle_rtl_deletion(rtl);
    }
    // Get the rest of rtls wich was controlled but slip throught other method\'s of notification.
    rtls = trash_can_get_rtls(&controlled_rtls);
    ObjPtr ob1;
    ForEach(ob1, rtls){
	RTL *rtl = (RTL *)ob1;
	report_out_of_scope(rtl);
    }

}

void miniBrowser::refresh(void)
{
    symbolArr list1sel, list2sel;

    if(unmanaged_list != list1){
	list1->app_nodes (&list1sel);
	list1->regenerate_rtl(list1->RTL(), 1, 2);
	list1->post_selection (list1sel, false);
    }
    if(unmanaged_list != list2){
	list2->app_nodes (&list2sel);
	list2->regenerate_rtl(list2->RTL(), 1, 2);
	list2->post_selection (list2sel, false);
    }
}

void miniBrowser::title1(char *txt)
{
    if(customize::miniBrowser_show_titles())
	list1->gt_list()->get_label()->text(txt);
    else
	list1->gt_list()->get_label()->text("");
}

void miniBrowser::title2(char *txt)
{
    if(customize::miniBrowser_show_titles())
	list2->gt_list()->get_label()->text(txt);
    else
	list2->gt_list()->get_label()->text("");
}

void miniBrowser::visible(int flag)
{
    Initialize(miniBrowser::visible);

    displayed = flag;
    if(!displayed)
	clear_selection();
}

int miniBrowser::visible(void)
{
    Initialize(miniBrowser::visible);
    
    return displayed;
}

gtBase *miniBrowser::get_paned_window()
{
    Initialize(miniBrowser::get_paned_window);
    
    return paned;
}

gtBase *miniBrowser::get_main_form()
{
    Initialize(miniBrowser::get_main_form);
    
    return mainForm;
}

// Inserts form into minibrowser paned window
void miniBrowser::manage_into_paned_form(gtForm *form, int preffered_height)
{
    Initialize(miniBrowser::manage_into_paned_form);
    
    if(!form)
	return;
    if(preffered_height){
	XtVaSetValues(form->container()->rep()->widget(), XmNpaneMinimum, preffered_height, NULL);
	form->manage();
	XtVaSetValues(form->container()->rep()->widget(), XmNpaneMinimum, 100, NULL);
    }else
	form->manage();
}

void miniBrowser::manage_into_main_form(gtForm *form)
{
    Initialize(miniBrowser::manage_into_main_form);
    
    if(!form)
	return;
    form->attach(gtLeft);
    form->attach(gtRight);
    form->attach(gtBottom);
    form->manage();
    paned->attach(gtBottom, form);
}

void miniBrowser::write_list(gtRTL *list)
{
    Initialize(miniBrowser::write_list);
    
    if(list->RTL()){
	RTLNodePtr rtl_node = checked_cast(RTLNode, list->RTL()->get_root());
	if(rtl_node && rtl_node->rtl_contents().size() != 0){
	    FileNameDlg *dlg = new FileNameDlg(list->RTL());
	    return;
	}
    }
    msg("The list is empty.", warning_sev) << eom;
}

void *miniBrowser::get_cleanup_form_data(void)
{
    return cleanup_callback_data;
}

void miniBrowser::set_list_cleanup_form(FormCleanupCallbackProc proc, void *data)
{
    form_cleanup_callback = proc;
    cleanup_callback_data = data;
}

// Unmanages list from form.  list == 0 - top list list != 0 bottom list
// Create new form for elememt placing and return it
gtForm *miniBrowser::unmanage_list(int list)
{
    gtForm *list_form;
    if(list == 0){
	list1->unmanage();
	list1->detach(gtTop);
	list1->detach(gtBottom);
	list1->detach(gtLeft);
	list1->detach(gtRight);
	unmanaged_list = list1;
	list_form      = list1Form;
    } else {
	list2->unmanage();
	list2->detach(gtTop);
	list2->detach(gtBottom);
	list2->detach(gtLeft);
	list2->detach(gtRight);
	unmanaged_list = list2;
	list_form      = list2Form;
    }
    new_form = gtForm::create(list_form, "temp_form");
    new_form->attach(gtTop);
    new_form->attach(gtLeft);
    new_form->attach(gtRight);
    new_form->attach(gtBottom);
    new_form->manage();
    return new_form;
}

int miniBrowser::restore_list(void)
{
    int ret_val = -1;

    if(new_form){
	new_form->unmanage();
	new_form->detach(gtTop);
	new_form->detach(gtLeft);
	new_form->detach(gtRight);
	new_form->detach(gtBottom);
	delete new_form;
	new_form = NULL;
    }
    if(unmanaged_list == list1){
	list1->attach(gtTop);
	list1->attach(gtBottom);
	list1->attach(gtLeft);
	list1->attach(gtRight);
	list1->manage();
	ret_val = 0; 
    } else if (unmanaged_list == list2) {
	list2->attach(gtTop);
	list2->attach(gtBottom);
	list2->attach(gtLeft);
	list2->attach(gtRight);
	list2->manage();
	ret_val = 1;
    }
    unmanaged_list = NULL;
    return ret_val;
}

void miniBrowser::SaveList1_CB(gtPushButton*, gtEvent*, void* mb, gtReason)
{
    Initialize(miniBrowser::SaveList1_CB);
    
    miniBrowser *browser = (miniBrowser *)mb;
    browser->write_list(browser->list1);
}

void miniBrowser::SaveList2_CB(gtPushButton*, gtEvent*, void* mb, gtReason)
{
    Initialize(miniBrowser::SaveList2_CB);

    miniBrowser *browser = (miniBrowser *)mb;
    browser->write_list(browser->list2);
}

void miniBrowser::history_CB(gtPushButton* b, gtEvent*, void* mb, gtReason)
{
    Initialize(miniBrowser::history_CB);
    
    miniBrowser *browser    = (miniBrowser *)mb;
    ListHistoryEntry *entry = (ListHistoryEntry *)b->userData();
    if(!entry)
	return;
    // Special case for top level projects ! Always get them from project node.
    if(entry == browser->history_list[0]){
	RTLPtr rtl     = projNode::get_project_rtl();
	RTLPtr new_rtl = rtl;
	new_rtl->set_name("Top Level Projects");
	browser->browse(new_rtl, 1);
    }else{
	RTLPtr rtl     = entry->rtl;
	RTLPtr new_rtl = rtl;
	objSet cans    = rtl_get_trash_cans(rtl);
	if(cans.includes(&garbage)){
	    // This rtl suppose to be deleted later, so create a copy and forget about this object
	    new_rtl = copy_create_rtl(rtl);
	    trash_can_put_rtl(&garbage, new_rtl);
	}
	miniBrowserRTL_API *api = rtl_get_APIhandler(rtl);
	if(api){
	    if(new_rtl != rtl)
		APIhandler_put_rtl(api, new_rtl);
	}
	browser->browse(new_rtl, entry->top_position);
    }
/*
    gtForm *f       = gtForm::create(browser->get_main_form(), "test");
    gtPushButton *b = gtPushButton::create(f,  "ttt", TXT("OPTIONS"), NULL, NULL);
    b->attach(gtLeft);
    b->attach(gtRight);
    b->attach(gtTop);
//    b->attach(gtBottom);
    b->manage();
    browser->manage_into_main_form(f);
    */    
}

void miniBrowser::report_out_of_scope(RTLPtr rtl)
// Report to API that rtl goes out of scope if it not displayed anywhere in minibrowser
{
    Initialize(miniBrowser::report_out_of_scope);

    miniBrowserRTL_API *api = rtl_get_APIhandler(rtl);
    if(!api)
	return;
    if(!list1 || !list2)
	api->rtl_out_of_scope(rtl);
    else
	if(list1->RTL() != rtl && list2->RTL() != rtl)
	    api->rtl_out_of_scope(rtl);
}

void miniBrowser::remove_from_history(RTLPtr rtl)
{
    Initialize(miniBrowser::remove_from_history);
    
    for(int i = 1; i < history_list.size(); i++){
	ListHistoryEntry *entry = (ListHistoryEntry *)history_list[i];
	if(entry->rtl == rtl){
	    // Shift list & remove buttons
	    int j;
	    for(j = i; j < history_list.size() - 1; j++){
		history_button->remove_entry(i);
		history_list[j] = history_list[j + 1];
	    }
	    history_button->remove_entry(i);
	    // Put buttons back
	    for(j = i; j < history_list.size() - 1; j++){
		ListHistoryEntry *entry = (ListHistoryEntry *)history_list[j];
		RTLPtr el               = entry->rtl;
		char   *name            = el->get_phys_name();
		if(!name || name[0] == 0)
		    name = el->get_name();
		history_button->insert_entries(-1, gtMenuStandard, "his_entry", name, this, history_CB,
					       gtMenuNull);
		gtPrimitive *button = history_button->entry(j);
		button->userData(entry);
	    }
	    // Finally make new history list (to update size)
	    objArr tmp = history_list;
	    history_list.removeAll();
	    for(j = 0; j < tmp.size() - 1; j++)
		history_list.insert_last(tmp[j]);
	    report_out_of_scope(entry->rtl);
	    delete entry;
	}
    }
}

void miniBrowser::add_rtl_to_history(RTLPtr rtl, int position, int list_num)
{
    Initialize(miniBrowser::add_rtl_to_history);

    for(int i = 0; i < history_list.size(); i++){
	ListHistoryEntry *entry = (ListHistoryEntry *)history_list[i];
	if(entry->rtl == rtl)
	    return;
    }
    int last_button;
    ListHistoryEntry *new_entry = new ListHistoryEntry;
    new_entry->rtl              = rtl;
    new_entry->top_position     = position;
    new_entry->list_number      = list_num;
    if(history_list.size() >= customize::miniBrowser_history_length()){
	if(history_list[1] != NULL){
	    ListHistoryEntry *entry = (ListHistoryEntry *)history_list[1];
	    report_out_of_scope(entry->rtl);
	    delete entry;
	}
	int i;
	for(i = 1; i < history_list.size() - 1; i++){
	    history_button->remove_entry(1);
	    history_list[i] = history_list[i + 1];
	}
	history_button->remove_entry(1);
	for(i = 1; i < history_list.size() - 1; i++){
	    ListHistoryEntry *entry = (ListHistoryEntry *)history_list[i];
	    RTLPtr el               = entry->rtl;
	    char   *name            = el->get_phys_name();
	    if(!name || name[0] == 0)
		name = el->get_name();
	    history_button->insert_entries(-1, gtMenuStandard, "his_entry", name, this, history_CB,
					   gtMenuNull);
	    gtPrimitive *button = history_button->entry(i);
	    button->userData(entry);
	}
	history_list[(last_button = history_list.size() - 1)] = new_entry;
    }else{
	history_list.insert_last(new_entry);
	last_button = history_list.size() - 1;
    }
    char *name = rtl->get_phys_name();
    if(!name || name[0] == 0)
	name = rtl->get_name();
    history_button->insert_entries(-1, gtMenuStandard, "his_entry", name, this, history_CB,
				   gtMenuNull);
    gtPrimitive *button = history_button->entry(last_button);
    button->userData(new_entry);
}

inline int shift_or_control(gtEventPtr e)
//
// Return true if the SHIFT or CONTROL keys were pressed in the event.
//
{
    return e->xbutton.state & (ShiftMask | ControlMask);
}

void miniBrowser::list1_select(gtList*, gtEventPtr e, void* mb, gtReason)
//
// Selection callback for top list.
//
{
    miniBrowser *m   = (miniBrowser *)mb;
    m->selected_list = m->list1;
    SetNextRtl(NULL);

    if (e){
	if(!shift_or_control(e)){
	    m->list2->deselect_all();
	    m->list2_selection.removeAll();
	}
	m->list1_selection.removeAll();
	m->list1->app_nodes(&m->list1_selection);
        m->rtl_select(shift_or_control(e));
    }else{
	m->list2->deselect_all();
	m->list2_selection.removeAll();
	m->list1_selection.removeAll();
	m->list1->app_nodes(&m->list1_selection);
        m->rtl_select(0);
    }
    RTL *rtl = m->list1->RTL();
    miniBrowserRTL_API *api;
    if(rtl && (api = rtl_get_APIhandler(rtl))){
	api->selection_callback(rtl, m->list1_selection, m);
    }
}

void miniBrowser::list2_select(gtList*, gtEventPtr e, void* mb, gtReason)
//
// Selection callback for bottom list.
//
{
    miniBrowser *m   = (miniBrowser *)mb;
    m->selected_list = m->list2;
    SetNextRtl(NULL);

    if (e){
	if(!shift_or_control(e)){
	    m->list1->deselect_all();
	    m->list1_selection.removeAll();
	}
	m->list2_selection.removeAll();
	m->list2->app_nodes(&m->list2_selection);
        m->rtl_select(shift_or_control(e));
    }else{
	m->list1->deselect_all();
	m->list1_selection.removeAll();
	m->list2_selection.removeAll();
	m->list2->app_nodes(&m->list2_selection);
        m->rtl_select(0);
    }
    RTL *rtl = m->list2->RTL();
    miniBrowserRTL_API *api;
    if(rtl && (api = rtl_get_APIhandler(rtl))){
	api->selection_callback(rtl, m->list2_selection, m);
    }
}

void miniBrowser::rtl_select(int extend)
//
// Store selection in list.
//
{
    Initialize(miniBrowser::rtl_select);
    
    if (!extend) {
	driver_instance->clear_selection(false);
    }
    
    selected_symbols.removeAll();
    symbolArr selection;
    selection.insert_last(list1_selection);
    selection.insert_last(list2_selection);
    if(selection.size())
    {
	for(int i = 0; i < selection.size(); ++i)
	{
            fsymbolPtr  sym  = selection[i];
	    if (sym.without_xrefp() || sym.is_ast() || sym.is_dataCell()) {
		if(!selected_symbols.includes(sym))
		    selected_symbols.insert_last(sym);
	    } else {
		fsymbolPtr  xsym = sym.get_xrefSymbol();
		if (xsym.xrisnull()) {
		    if ( selected_symbols.includes (sym) )
			continue;

		    Relational *rel_ptr = (Relational *)sym;
		    if (is_RTL (rel_ptr)) {
			RTL *rtl = (RTL *)rel_ptr;
			RTLNode *instance_nodes =
			    checked_cast(RTLNode,((RTL *)rel_ptr)->get_root());
			::obj_insert(rtl, REPLACE, instance_nodes, instance_nodes, NULL);
			selected_symbols.insert_last (sym);
		    }
		} else {
		    if(!selected_symbols.includes(sym))
			selected_symbols.insert_last(sym);
		}
	    }
	}

	symbolPtr sym   = selection[selection.size() - 1];
        genString info_msg;
	if(sym.without_xrefp())
	    (void) get_display_string_from_symbol(sym, info_msg);
	else {
	    fsymbolPtr xsym = sym.get_xrefSymbol();
	    if (xsym.xrisnotnull())
		(void) get_display_string_from_symbol(xsym, info_msg);
	}

	if (info_msg.length())
	    msg("symbol_info:miniBrowser.h.C", normal_sev) << (char *)info_msg << eom;
    }
}

void miniBrowser::remove(RTL *rtl)
{
    Initialize(miniBrowser::remove);
    
    remove_from_history(rtl);
    gtRTL *old_selected     = selected_list;
    ListHistoryEntry *entry = NULL;
    if(rtl == list1->RTL()){
	// Scan throght history list & try to find most sutable replacement
	for(int i = history_list.size() - 1; i > 0; i--){
	    ListHistoryEntry *e = (ListHistoryEntry *)history_list[i];
	    if(e->list_number == 1){
		entry = e;
		break;
	    }
	}
	selected_list = list2;
	list1->set_rtl(NULL);
	title1(" ");
	handle_rtl_deletion(rtl);
    }
    if(rtl == list2->RTL()){
	// Scan throght history list & try to find most sutable replacement
	for(int i = history_list.size() - 1; i > 0; i--){
	    ListHistoryEntry *e = (ListHistoryEntry *)history_list[i];
	    if(e->list_number == 2){
		entry = e;
		break;
	    }
	}
	selected_list = list1;
	list2->set_rtl(NULL);
	title2(" ");
	handle_rtl_deletion(rtl);
    }
    /*
    if(entry){
	RTL *new_rtl = entry->rtl;
	objSet cans  = rtl_get_trash_cans(entry->rtl);
	if(cans.includes(&garbage)){
	    // This rtl suppose to be deleted later, so create a copy and forget about this object
	    new_rtl = copy_create_rtl(entry->rtl);
	    trash_can_put_rtl(&garbage, new_rtl);
	}
	miniBrowserRTL_API *api = rtl_get_APIhandler(entry->rtl);
	if(api){
	    if(new_rtl != rtl)
		APIhandler_put_rtl(api, new_rtl);
	}
	browse(new_rtl, entry->top_position);
//	remove_from_history(entry->rtl);
    }
    */
    selected_list = old_selected;
}

void miniBrowser::browse(appTree *app_node, int position)
{
    Initialize(miniBrowser::browse_appTree);
    
    RTL *rtl = checked_cast(RTL, get_relation(header_of_tree, app_node));
    if(rtl){
	trash_can_put_rtl(&garbage, rtl);
	browse(rtl, position);
    }
}


static gtList *list_to_set = NULL; // Used only for list_position_set!!!
static int     list_pos;           // Used only for list_position_set!!!

static void list_position_set(void *pos)
{
    int position = *(int *)pos;
    if(list_to_set){
	if(list_to_set->num_items() >= position)
	    list_to_set->set_pos(position);
    }
}

void miniBrowser::set_output_window(int window_index)  /* index - 0 - top window, 1 - bottom window */
{
    Initialize(miniBrowser::set_output_window);

    if(window_index){
	selected_list = list1;
    } else {
	selected_list = list2;
    }
}

int miniBrowser::get_next_window(void)  /* returns 0 - top window, 1 - bottom window */
{
    if(selected_list == list1)
	return 1;
    else
	return 0;
}

void miniBrowser::browse(RTLPtr rtl_head, int position)
{
    Initialize(miniBrowser::browse);
    
    if(!visible())
	vs_display_mini_browser((Widget)parent_viewershell, 1);
    trash_can_put_rtl(&controlled_rtls, rtl_head);
    RTLPtr old_rtl = NULL;
    int    old_position;
    int    old_list_num;
    if(selected_list == list1){
	if(unmanaged_list == list2){
	    if(form_cleanup_callback)
		form_cleanup_callback(this, cleanup_callback_data);
	    restore_list();
	}
	old_rtl      = list2->RTL();
	old_position = list2->gt_list()->top_item_position();
	old_list_num = 2;
	list2->set_rtl_reset_filters(rtl_head);
	title2(rtl_head->get_name());
	list_to_set = list2->gt_list();
    }else{
	if(unmanaged_list == list1){
	    if(form_cleanup_callback)
		form_cleanup_callback(this, cleanup_callback_data);
	    restore_list();
	}
	old_rtl      = list1->RTL();
	old_position = list1->gt_list()->top_item_position();
	old_list_num = 1;
	list1->set_rtl_reset_filters(rtl_head);
	title1(rtl_head->get_name());
	list_to_set = list1->gt_list();
    }
    list_pos = position;
    pset_send_own_callback(list_position_set, &list_pos);
    if(old_rtl){
	add_rtl_to_history(old_rtl, old_position, old_list_num);
    }
}

void miniBrowser::switch_window(void)
{
    if(selected_list == list1)
	selected_list = list2;
    else
	selected_list = list1;
}

void miniBrowser::open_module(symbolPtr sym)
{
    Initialize(open_module);

    projModule *mod = NULL;
    if(sym.relationalp()){
	view_create(sym, Rep_UNKNOWN);
	view_create_flush();
	return;
    }else{
	if(sym.is_xrefSymbol()){
	    if(sym.get_kind() == DD_MODULE)
		mod = projModule_of_symbol(sym);
	    if(mod){
		view_create(sym, Rep_UNKNOWN);
		view_create_flush();
		return;
	    }
	}
	if(!mod){
	    appPtr app_head = checked_cast(app, sym.get_def_app());
	    if(app_head){
		appTreePtr app_root = checked_cast(appTree, app_head->get_root());
		view_create(app_root);
		view_create_flush();
		return;
	    }
	    genString fn = sym.get_name();
	    if(fn.length()){
		view_create(fn);
		view_create_flush();
	    }
	    return;
	}
    }
    if(!mod)
	return;
    genString fn;
    mod->get_phys_filename(fn);
    projNode *pn  = mod->get_project();
    projNode *lpn = pn;
    genString ln;
    if(!fn.length())
	return;
    projHeader::fn_to_ln(fn, ln, &lpn, 0, 1);
    projModule *lmod = mod;
    if (pn && lpn)
    {
	if (pn != lpn)
	    lmod = lpn->find_module(ln);
    }
    appPtr app = lmod->restore_module();
    if(app)
	view_create(app);
    else
	view_create(fn);
    view_create_flush();
}

void miniBrowser::list1_action(gtList*, gtEventPtr, void* mb, gtReason)
{
    Initialize(miniBrowser::list1_action);
    
    miniBrowser *browser = (miniBrowser *)mb;
    RTL         *rtl     = browser->list1->RTL();
    miniBrowserRTL_API *api;
    int ret_val = 1;
    if(rtl && (api = rtl_get_APIhandler(rtl))){
	ret_val = api->action_callback(rtl, browser->list1_selection, browser);
    }
    if(ret_val)
	browser->list_action();
    browser->clear_selections();
}

void miniBrowser::list2_action(gtList*, gtEventPtr, void* mb, gtReason)
{
    Initialize(miniBrowser::list2_action);

    miniBrowser *browser = (miniBrowser *)mb;
    RTL         *rtl     = browser->list2->RTL();
    miniBrowserRTL_API *api;
    int ret_val = 1;
    if(rtl && (api = rtl_get_APIhandler(rtl))){
	ret_val = api->action_callback(rtl, browser->list2_selection, browser);
    }
    if(ret_val)
	browser->list_action();
    browser->clear_selections();
}

void miniBrowser::list_action(void)
{
    Initialize(miniBrowser::list_action);

    push_busy_cursor();
    
    BrowserData* bd = LanguageController::GetBrowserData();

    for (int i = 0; i < selected_symbols.size(); i++)
      {
	const char* cmd = bd->get_action_cmd(selected_symbols[i].get_kind());
	if (cmd != NULL)
	  {
	    Interpreter* intr = GetActiveInterpreter();
	    if (strlen(cmd) > 0 && intr)
	      { 
		symbolArr sel, dummy;
		sel.insert_last(selected_symbols[i]);
		intr->EvalQuery((char*)cmd, sel, dummy);
	      } 
	  }
	else
	  {
	    symbolPtr sym = selected_symbols[i];
	    symbolPtr xsym;
	    if(sym.get_kind() == DD_SMT || sym.get_kind() == DD_REGION || sym.is_instance() || sym.is_ast() || sym.is_dataCell()) 
	      xsym = sym; // do not get xrefSymbol for these types (will try to open view on smt/position)
	    else
	      xsym = sym.get_xrefSymbol();
	    if (!xsym.is_ast() && !xsym.is_dataCell() && xsym.xrisnull() && !sym.without_xrefp()) {
	      Relational *rel_ptr = (Relational *)sym;
	      if (is_RTL (rel_ptr)) {
		RTL *rtl = (RTL *)rel_ptr;
		RTLNode *instance_nodes =
		  checked_cast(RTLNode,((RTL *)rel_ptr)->get_root());
		::obj_insert(rtl, REPLACE, instance_nodes, instance_nodes, NULL);
		RTLNode *childs = checked_cast(RTLNode, get_relation(tree_of_header, rel_ptr));
		if(childs){
		  // Create & copy rtl to new one to allow later deletion.
		  RTL *rtl     = checked_cast(RTL, get_relation(header_of_tree, childs));
		  RTL *new_rtl = rtl;
		  browse(new_rtl);
		}
	      }
	    } else {
	      // boris: Allow new Relational DD_MODULE and DD_PROJECT to go here with xrefSymbols
	      ddKind knd = (xsym.xrisnull()) ? sym.get_kind() : xsym.get_kind();
	      switch (knd)
		{
		case DD_PROJECT:
		  { projHeader *rtl_head = checked_cast(projHeader, sym.get_def_app());
		    if(rtl_head){
		      projNodePtr proj_root = projNodePtr(rtl_head->get_root());
		      proj_root->refresh();
		      RTL *new_rtl          = rtl_head;
		      ProjectController *pc = new ProjectController(proj_root, this);
		      APIhandler_put_rtl(pc, new_rtl);
		      browse(new_rtl);
		    } }
		  break;
		case DD_MODULE:
		  ste_finalize();
		  open_module(sym);
		  break;
		case DD_SUBSYSTEM:
		  ste_finalize();
		  view_create(xsym, Rep_OODT_Inheritance);
		    view_create_flush();
		  break;
		case DD_IFL_SRC:
		case DD_RELATION:
		  msg("Unable to open view for this symbol.", error_sev) << eom;
		  break;
		case DD_SMT:
		case DD_REGION:
		  ste_finalize();
		  view_create(xsym, Rep_UNKNOWN, 1);
		  view_create_flush();
		  break;
		default:
		  if (xsym.is_ast() || sym.is_dataCell() || xsym.xrisnotnull()) {
		  ste_finalize();
		  view_create(xsym, Rep_UNKNOWN, 1);
		  view_create_flush();
		}
		  break;
		}
	    }
	  }
      }
    pop_cursor();
}

void miniBrowser::make_list2_visible(void)
{
    Initialize(miniBrowser::make_list2_visible);

    Dimension height;
    Dimension list_height;
//    XtVaSetValues(paned->container()->rep()->widget(), XmNunitType, XmPIXELS, NULL);
    XtVaGetValues(paned->container()->rep()->widget(), XmNheight, &height, NULL);
    XtVaGetValues(list2->container()->rep()->widget(), XmNheight, &list_height, NULL);
    if(list_height >= height / 2)
	return;
    list2->unmanage();
    XtVaSetValues(list2->container()->rep()->widget(), XmNpaneMinimum, height / 2, NULL);
    list2->manage();
    XtVaSetValues(list2->container()->rep()->widget(), XmNpaneMinimum, 100, NULL);
    selected_list = list1;
}

void miniBrowser::GroupManagerButton(gtPushButton*, gtEvent*, void*, gtReason)
{
    Initialize(miniBrowser::GroupManagerButton);

    SetsUI::Invoke(NULL);
}

void miniBrowser::GroupButton(gtPushButton*, gtEvent*, void* mb, gtReason)
{
    Initialize(miniBrowser::GroupButton);

    symbolArr selection;
    fill_selected_objects(&selection);
    if(selection.size() == 0){
	msg("Nothing_selected:miniBrowser.h.C", error_sev) << eom;
	return;
    }
    call_cli_callback(GetActiveInterpreter(), "mini_capture_list", "", &selection, NULL);
}

void miniBrowser::PopupBrowser(gtCascadeButton *b, gtEvent*, void* mb, gtReason)
{
    Initialize(miniBrowser::PopupBrowser);
    
    miniBrowser *browser = (miniBrowser *)mb;
    if(browser->visible()){
	vs_display_mini_browser((Widget)browser->parent_viewershell, 0);
	b->label("Minibrowser(OFF)");
    }
    else{
	vs_display_mini_browser((Widget)browser->parent_viewershell, 1);
	b->label("Minibrowser(ON) ");
    }
}

void miniBrowser::ToggleBrowser(gtPushButton *, gtEvent*, void* mb, gtReason)
{
    Initialize(miniBrowser::ToggleBrowser);
    
    miniBrowser *browser = (miniBrowser *)mb;
    if(browser->visible()){
	vs_display_mini_browser((Widget)browser->parent_viewershell, 0);
	browser->on_button->unmanage();
	browser->off_button->manage();
    }
    else{
	vs_display_mini_browser((Widget)browser->parent_viewershell, 1);
	browser->off_button->unmanage();
	browser->on_button->manage();
    }
}

int cli_eval_query(const char* cmd, symbolArr& scope, symbolArr&);

void miniBrowser::QueryButton(gtPushButton* b, gtEvent*, void* mb, gtReason)
{
    Initialize(miniBrowser::QueryButton);

    miniBrowser *browser = (miniBrowser *)mb;

    char *title = b->title();

    symbolArr selection;
    fill_selected_objects(&selection);
    if(selection.size() == 0){
	// If nothing is selected get current viewed module.
	viewerShell* vsh = view_target_viewer()->get_shell();
	viewPtr     view = vsh->get_current_view();
	if(view){
	    switch(repType(view->get_type()))
	    {
	      case Rep_VertHierarchy:		// Operate on app of view.
	      case Rep_FlowChart:
	      case Rep_TextDiagram:
	      case Rep_TextText:
	      case Rep_TextFullDiagram:
	      case Rep_SmtText:
	      {
		  ldrPtr ldr_head = view_get_ldr(view);
		  if(ldr_head){
		      appPtr app_head = ldr_get_app(ldr_head);
		      if(app_head)
			  selection.insert_last(app_head->get_module());
		  }
	      }
	        break;
	      case Rep_RawText:
                 msg("Unable to query Raw Text.", error_sev) << eom;
                 break;
	      default:
                 msg("Unable to query unknown Text type.", error_sev) << eom;
	        break;
	    }
	}
	if(selection.size() == 0)
	{
            browser->remove_queries();
	    return;
	}
    }
    
    push_busy_cursor();
    RTL *rtl                = db_new(RTL, (NULL));
    RTLNodePtr rtl_node     = checked_cast(RTLNode, rtl->get_root());
    genString selected_name = selection[0].get_name();  
    char *nm                = (char *)selected_name;
    genString name;
    if(selected_name.length() > 25)
 	nm[25 - 1] = '\0';
    name.printf("%s: %s", (char *)selected_name, title);
    rtl->set_name((char *)name);
 
    symbolArr& results = rtl_node->rtl_contents();
    results.removeAll();

    const char* cmd = browser->pBController->get_query_command(title);
    if (cmd && strlen(cmd) > 0)
      {
	genString command;
	browser->pBController->handle_closure(cmd, command);
	cli_eval_query(command, selection, results);
      }

    gtFree(title);

    if(results.size() == 0){
	msg("Query_found_no_matches:miniBrowser.h.C", normal_sev) << (char *)name << eom;
	if(rtl)
	    obj_delete(rtl);
    }else{
	trash_can_put_rtl(&garbage, rtl);  // Store this object to delete when we don\'t need it any more

	if(strcmp(cmd, "where used") == 0 || strcmp(cmd, "where referenced") == 0)
	{
	    // Convert selection to xrefSymbols !!! to make sure that any change will keep array consistent
	    symbolArr xselection;
	    for(int i = 0; i < selection.size(); i++){
		symbolPtr xsym = selection[i].get_xrefSymbol();
		if(!xsym.isnull())
		    xselection.insert_last(xsym);
	    }
	    InstanceAPI *instance = new InstanceAPI(xselection);
	    APIhandler_put_rtl(instance, rtl);
	}
	browser->browse(rtl);
    }

    //browser->remove_queries();

    pop_cursor();
}

void miniBrowser::SlideButton(gtArrowButton*, gtEvent*, void* mb, gtReason)
{
    Initialize(miniBrowser::SlideButton);
    
    miniBrowser *browser = (miniBrowser *)mb;
    vs_display_mini_browser((Widget)browser->parent_viewershell, 0);
}

void miniBrowser::init_search_path(symbolArr& selection)
{
    Initialize(miniBrowser::init_search_path);
    
    objArr proj_arr;
    
    // First scan for projects
    int i;
    for(i = 0; i < selection.size(); i++){
	symbolPtr sym = selection[i];
	if(sym.get_kind() == DD_PROJECT){
	    projHeader *rtl_head = checked_cast(projHeader, sym.get_def_app());
	    if(rtl_head){
		projNodePtr proj_root = projNodePtr(rtl_head->get_root());
		proj_arr.insert_last(proj_root);
	    }
	}
    }
    scope.set_domain(proj_arr);
    // Add selected modules
    for(i = 0; i < selection.size(); i++){
	symbolPtr sym = selection[i];
	if(sym.get_kind() == DD_MODULE) {
	    symbolPtr xsym = sym.get_xrefSymbol();
	    if (xsym.xrisnotnull())
		scope.add_module(sym);
	    else if (sym.without_xrefp())
		scope.add_module(sym);
	}
    }
}

void miniBrowser::clear_selection(void)
{
    Initialize(miniBrowser::clear_selection);
    
    list1->deselect_all();
    list2->deselect_all();
    list1_selection.removeAll();
    list2_selection.removeAll();
    selected_symbols.removeAll();
}

// Static function
void miniBrowser::clear_selections(void)
{
    Initialize(miniBrowser::clear_selections);

    for(int i = 0; i < miniBrowser_instances.size(); i++){
	miniBrowser *mb = *miniBrowser_instances[i];
	mb->clear_selection();
    }
}

// Static function
void miniBrowser::fill_selected_nodes(symbolArr& array)
{
    Initialize(miniBrowser::fill_selected_nodes);

    for(int i = 0; i < miniBrowser_instances.size(); i++){
	miniBrowser *mb = *miniBrowser_instances[i];
	array.insert_last(mb->selected_symbols);
    }
}

static void miniBrowser_initialize_GC(Widget w)
{
    XGCValues values;
    
    if(miniBrowser_gc_initialized)
	return;
    values.foreground          = 0xEEEEEE;
    values.function            = GXxor;
    values.subwindow_mode      = IncludeInferiors;
    miniBrowser_GC             = XtGetGC(w, GCForeground | GCFunction | GCSubwindowMode, &values);
    miniBrowser_gc_initialized = 1;
}

extern "C" void miniBrowser_start_dragging(Widget w, void *data, XEvent *ev, Boolean *cont)
{
    if(do_dragging){
	// Do not dispatch this event further
	*cont = False;
	return;
    }
    if(ev->xbutton.button != Button1)
	return;
    do_dragging   = 1;
    Widget parent = XtParent(w);
    XtVaSetValues(parent, XmNunitType, XmPIXELS, NULL);
    XtVaSetValues(w, XmNunitType, XmPIXELS, NULL);
    XtVaGetValues(parent, XmNheight, &miniBrowser_height, NULL);
    XtVaGetValues(w, XmNleftOffset, &miniBrowser_button_x, NULL);
    Widget separator = (Widget)data;
    int separator_x;
    XtVaSetValues(separator, XmNunitType, XmPIXELS, NULL);
    XtVaGetValues(separator, XmNleftOffset, &separator_x, NULL);
    miniBrowser_xdiff = separator_x - miniBrowser_button_x;
    XDrawLine(XtDisplay(parent), XtWindow(parent), miniBrowser_GC, separator_x, 15,
	      separator_x, miniBrowser_height);
}

extern "C" void miniBrowser_do_dragging(Widget w, void *, XEvent *ev, Boolean *)
{
    if(!do_dragging)
	return;
    Widget parent = XtParent(w);
    XDrawLine(XtDisplay(parent), XtWindow(parent), miniBrowser_GC, miniBrowser_button_x + miniBrowser_xdiff,
	      15, miniBrowser_button_x + miniBrowser_xdiff, miniBrowser_height);
    Dimension width;
    XtVaGetValues(parent, XmNwidth, &width, NULL);
    int x;
    XtVaGetValues(w, XmNleftOffset, &x, NULL);
    miniBrowser_button_x = x + ev->xbutton.x;
    if(miniBrowser_button_x < 40)
	miniBrowser_button_x = 40;
    if(miniBrowser_button_x > width - 40)
	miniBrowser_button_x = width - 40;
    XDrawLine(XtDisplay(parent), XtWindow(parent), miniBrowser_GC, miniBrowser_button_x + miniBrowser_xdiff,
	      15, miniBrowser_button_x + miniBrowser_xdiff, miniBrowser_height);
}

extern "C" void miniBrowser_end_dragging(Widget w, void *data, XEvent *ev, Boolean *)
{
    if(ev->xbutton.button != Button1)
	return;
    miniBrowser_event_data *form_data = (miniBrowser_event_data *)data;
    do_dragging   = 0;
    Widget parent = XtParent(w);
    XDrawLine(XtDisplay(parent), XtWindow(parent), miniBrowser_GC, miniBrowser_button_x + miniBrowser_xdiff,
	      15, miniBrowser_button_x + miniBrowser_xdiff, miniBrowser_height);
    XtVaSetValues(w, XmNleftOffset, miniBrowser_button_x,
		  NULL);
    Widget separator = form_data->separator;
    XtVaSetValues(separator, XmNleftOffset, miniBrowser_button_x + miniBrowser_xdiff,
		  NULL);
    form_data->button_pos = miniBrowser_button_x;
    form_data->sep_pos    = miniBrowser_button_x + miniBrowser_xdiff;
}

extern "C" void miniBrowser_form_resize(Widget, void *data, XEvent *ev)
{
    miniBrowser_event_data *form_data = (miniBrowser_event_data *)data;
    if(ev->type != ConfigureNotify)
	return;
    if(form_data->width == 0){
	Dimension total_form_width;

	XtVaSetValues(form_data->separator, XmNunitType, XmPIXELS, NULL);
	XtVaSetValues(form_data->button, XmNunitType, XmPIXELS, NULL);
	XtVaSetValues(XtParent(form_data->button), XmNunitType, XmPIXELS, NULL);
	XtVaSetValues(XtParent(XtParent(form_data->button)), XmNunitType, XmPIXELS, NULL);
	XtVaGetValues(XtParent(XtParent(form_data->button)), XmNwidth, &total_form_width, NULL);
	if(total_form_width < 840){
	    XtVaSetValues(form_data->separator, XmNleftOffset, (form_data->sep_pos = total_form_width - 150), NULL);
	    XtVaSetValues(form_data->button, XmNleftOffset, (form_data->button_pos = total_form_width - 154), NULL);
	}else{
	    XtVaGetValues(form_data->separator, XmNleftOffset, &form_data->sep_pos, NULL);
	    XtVaGetValues(form_data->button, XmNleftOffset, &form_data->button_pos, NULL);
	}
	XtVaGetValues(XtParent(form_data->button), XmNwidth, &form_data->width, NULL);
//	form_data->width = ev->xconfigure.width;
	return;
    }
    if(form_data->width == ev->xconfigure.width)
	return;
    float rel     = (float)form_data->sep_pos / form_data->width;
    float new_pos = rel * ev->xconfigure.width;
    form_data->button_pos = (int)new_pos - (form_data->sep_pos - form_data->button_pos);
    form_data->sep_pos    = (int)new_pos;
    form_data->width      = ev->xconfigure.width;
    XtVaSetValues(form_data->separator, XmNleftOffset, form_data->sep_pos,
		  NULL);
    XtVaSetValues(form_data->button, XmNleftOffset, form_data->button_pos,
		  NULL);
}

extern "C" void miniBrowser_destroy_callback(Widget, void *data, void *)
{
    miniBrowser_event_data *form_data = (miniBrowser_event_data *)data;
    delete form_data;
}

extern "C" void miniBrowser_initialize_slide_button(Widget button, Widget separator)
{
    miniBrowser_initialize_GC(XtParent(button));
    // Following structure would be freed in the event handler
    miniBrowser_event_data *data = new miniBrowser_event_data;
    data->button    = button;
    data->separator = separator;
    data->width     = 0;

    XtAddEventHandler(button, ButtonPressMask, FALSE, (XtEventHandler)miniBrowser_start_dragging, 
		      (XtPointer)separator);
    XtAddEventHandler(button, PointerMotionMask, FALSE, (XtEventHandler)miniBrowser_do_dragging, 
		      (XtPointer)separator);
    XtAddEventHandler(button, ButtonReleaseMask, FALSE, (XtEventHandler)miniBrowser_end_dragging,
		      (XtPointer)data);
    XtAddEventHandler(XtParent(button), StructureNotifyMask, FALSE,
         	      (XtEventHandler)miniBrowser_form_resize, (XtPointer)data);
    XtAddCallback(XtParent(button), XtNdestroyCallback, (XtCallbackProc)miniBrowser_destroy_callback, 
		  (XtPointer)data);
}

extern "C" void miniBrowser_initialize_slide_separator(Widget)
{
}

//------------------------------------------
// closure button handling
//------------------------------------------

void miniBrowser::closure(bool enabled) {
   Initialize(miniBrowser::closure);

   closure_button->set(enabled, 0);
}

void miniBrowser::closure_CB(gtToggleButton* but, gtEvent*, void*, gtReason) {
   Initialize(miniBrowser::closure_CB);

   if (but) {
      set_closure_state(but->set());
   }
}


//------------------------------------------

void miniBrowser::cleanup_lists()
{
   RTL *rtl1 = list1->RTL();
   list1->set_rtl(NULL);
   title1(" ");
   handle_rtl_deletion(rtl1);
 
   RTL *rtl2 = list2->RTL();
   list2->set_rtl(NULL);
   title2(" ");
   handle_rtl_deletion(rtl2);

   while(history_list.size() > 1){
       history_button->remove_entry(1);
       ListHistoryEntry *entry = (ListHistoryEntry *)history_list[1];
       report_out_of_scope(entry->rtl);
       history_list.remove(entry);
       delete entry;
   }
   objSet rtls = trash_can_get_rtls(&garbage);
   ObjPtr ob;
   ForEach(ob, rtls){
       RTL *rtl = (RTL *)ob;
       handle_rtl_deletion(rtl);
   }

// Get the rest of rtls wich was controlled but slip throught other method\'s of notification.
   rtls = trash_can_get_rtls(&controlled_rtls);
   ObjPtr ob1;
   ForEach(ob1, rtls){
      RTL *rtl = (RTL *)ob1;
      report_out_of_scope(rtl);
  }
/*
    RTLPtr new_rtl     = projNode::get_project_rtl();
    new_rtl->set_name("Top Level Projects");
    list1->set_rtl(new_rtl);
    add_rtl_to_history(new_rtl, 1);
    title1("Top Level Projects");
    selected_list = list1;
*/
}

void cleanup_miniBrowsers()
{
   for(int i = 0; i < miniBrowser_instances.size(); i++){
	miniBrowser *mb = *miniBrowser_instances[i];
	if ( mb )
	   mb->cleanup_lists();
    }
}

void miniBrowser::update_query_menu_CB(void* data, caddr_t browser, gtEventPtr)
{
    miniBrowser* mb = (miniBrowser*)browser;
    mb->update_query_menu();
}

void get_ddKinds(genMask& mask, const symbolArr& syms);
void miniBrowser::update_query_menu()
{
    Initialize(miniBrowser::update_query_menu);

    symbolArr selection;
    fill_selected_objects(&selection);
    if(selection.size() == 0){
	// If nothing is selected get current viewed module.
	viewerShell* vsh = view_target_viewer()->get_shell();
	viewPtr     view = vsh->get_current_view();
	if(view){
	    switch(repType(view->get_type()))
	    {
	      case Rep_VertHierarchy:		// Operate on app of view.
	      case Rep_FlowChart:
	      case Rep_TextDiagram:
	      case Rep_TextText:
	      case Rep_TextFullDiagram:
	      case Rep_SmtText:
	      {
		  ldrPtr ldr_head = view_get_ldr(view);
		  if(ldr_head){
		      appPtr app_head = ldr_get_app(ldr_head);
		      if(app_head)
			  selection.insert_last(app_head->get_module());
		  }
	      }
	        break;
	      case Rep_RawText:
                 msg("Unable to query Raw Text.", error_sev) << eom;
                 break;
	      default:
                 msg("Unable to query unknown Text type.", error_sev) << eom;
	        break;
	    }
	}
    }

    genMask sel;
    get_ddKinds(sel, selection);
    if (sel == old_selection)
	return; //no need to change queries
	
    remove_queries();
    old_selection = sel;

    int j;
    genArrCharPtr queries;
    genArrCharPtr subqueries;

    pBController->get_queries_with_submenus(sel, queries);

    for (int i = 0; i < queries.size(); i++)
    {
	num_queries++;
	if (!pBController->is_submenu(*queries[i]))
	{
	    query_menu->insert_entries(num_queries, gtMenuStandard, "menu_item", 
				       *queries[i], this, QueryButton, gtMenuNull);
	}
	else // for sub menues
        {
	    char label[10];
	    // need this label not to break regressions
	    OSapi_sprintf(label, "submenu%d", 
			  pBController->get_submenu_index(*queries[i]) + 1);
	    query_menu->insert_entries(num_queries, gtMenuCascade, label, 
				       *queries[i], this, QueryButton, gtMenuNull);
	    pBController->get_submenu_items(selection, *queries[i], subqueries);
	    gtCascadeButton* button = (gtCascadeButton*)query_menu->entry(num_queries);
	    button->pulldown_menu(label, gtMenuNull);
	    for (j = 0; j < subqueries.size(); j++)
	    {
		button->insert_entries(-1, gtMenuStandard, "submenu_item",
				       *subqueries[j], this, QueryButton, gtMenuNull);
	    }
	}
    }
    
}

void miniBrowser::remove_queries()
{
    /* remove all entries from query_menu except for "Closure" */
    for (int i = 1; i <= num_queries; i++)
    {
	gtPrimitive* entry = query_menu->entry(1);
	const char* name = entry->name();
	if (strcmp(name, "menu_item") != 0) //if this is a submenu
	{
	    gtCascadeButton* button = (gtCascadeButton*) entry;
	    gtPrimitive *p, *old_p;
	    do
	    {
		old_p = button->entry(0);
		button->remove_entry(0);
		p = button->entry(0);
	    }
	    while (p && p != old_p);
	}
	
	query_menu->remove_entry(1);
    }
    num_queries = 0;
    old_selection.clear();
}

