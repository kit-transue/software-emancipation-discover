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
class ddSelector;
class objArr;
class symbolArr;
class objRawApp;
class viewer;
class Process;
class genMask;

#include <cLibraryFunctions.h>
#include <systemMessages.h>


#include <groupHdr.h>
#include <Panner.h>
#include <instanceBrowser.h>
#include <feedback.h>
#include <RTL.h>
#include <viewNode.h>
#include <astTree.h>
#include <oodt_relation.h>
#include <group.h>
#include <dd_boil.h>
#include <miniBrowser.h>
#include <viewerShell.h>
#include <symbolScope.h>
#include <ui-rowcolumn.h>
#include <ui_INTERP_info.h>
#include <objRelmacros.h>
#include <prototypes.h>
#include <ldrHeader.h>
#include <ldrDGraph.h>
#include <ldrDGNode.h>
#include <appDGraph.h>

#include <gtDialog.h>
#include <gtBase.h>
#include <gtRTL_cfg.h>
#include <gtTogB.h>
#include <gtVertBox.h>
#include <gtStringEd.h>
#include <gtRadioBox.h>
#include <gtRTL_boil.h>
#include <gtPushButton.h>
#include <gtDisplay.h>
#include <StatusDialog.h>
#include <glayer.h>
#include <graWorld.h>
#include <browserShell.h>
#include "Save_Apps.h"
#include "tcl.h"
#ifdef _WIN32
#include "SymManager.h"
#include <memory.h>
#endif
#ifdef _WIN32
#include "nt_dirent.h"
#endif
#include "smtRegion.h"
#include "gtRTL.h"

symbolPtr symbolPtrReturn;

int start_from_connect = 0;

struct FeedBack* fb_instance = NULL;

char * cmd_current_journal_name = NULL;
char * cmd_execute_journal_name = NULL;
int    cmd_logging_on = 0;

init_relation(screen_of_viewer,1,relationMode::D,viewer_of_screen,1,NULL);
init_relational(viewer,Relational);
init_relational(InstanceRep,appTree);

//----------------------------------------------------
// Functions that have no output (i.e. print no message when called)
//

void logger_end_transaction(void) { }
void logger_flush_transactions(void) { }
void logger_start_transaction(void) { }
void SetClientHook (void (*)(_XEvent*)) { }
int put_is_running(void) {return 0;}
extern "C" void galaxyStartTimer (void) { }
extern "C" void galaxyStopTimer (void) { }


// return -1   if hd is not external header
//         0   on failure
//         1   on success


void add_to_pending_lists(smtHeader*) { }
void enable_reparse_buttons(smtHeader*, bool) { }


//----------------------------------------------------

extern "C" void pset_stub (char* msg, int silent=0) {
    static int squelch = -1;
    if (squelch == -1) {
	if (OSapi_getenv("DISCOVER_STUBS"))
	    squelch = 0;
	else
	    squelch = 1;
    }
    if(!silent && !squelch) OSapi_fprintf (stderr, "%s called, but is not defined\n", msg);
}      

extern "C" void galaxyInitialize (int, char*)
{
    pset_stub ("void galaxyInitialize (int argc, char* argv[])");
}

extern "C" void galaxyProcess (void)
{
    pset_stub ("void galaxyProcess (void)");
}

extern "C" void galaxyYield (void)
{
    pset_stub ("void galaxyYield (void)");
}

extern "C" void galaxyShutdown (int ) 
{
    pset_stub ("void galaxyShutdown (int )");
}

extern "C" void galaxyQuickShutdown (int )
{
    pset_stub ("void galaxyQuickShutdown (int status)");
}

extern "C" const char *do_define_function (const char *, const char *, const char *, const char *, const char *, const char *, int, const char *, const char *, const char *) {

    pset_stub ("do_define_function");
    return NULL;
}

extern "C" void automatic_test (char *)
{
    pset_stub ("automatic_test");
}

extern "C" int init_exec() 
{
    pset_stub ("init_exec()");
    return 0;
}

viewerShell::viewerShell(int create_minibrowser)
{
    pset_stub ("viewerShell::viewerShell(int create_minibrowser)");
}

viewerShell::~viewerShell() 
{
    pset_stub("viewerShell::~viewerShell()");
}

viewer* viewerShell::create_viewer()
{
    pset_stub("viewer* viewerShell::create_viewer()");
    return NULL;
}

viewerShell* viewerShell::get_vs_by_idx(int )
{
    pset_stub ("viewerShell::get_vs_by_ids (int)");
    return NULL;
}

void viewerShell_build_make_submenu(viewerShell *)
{
    pset_stub ("viewerShell_build_make_submenu(viewerShell *)");
}

extern "C" void do_define_struct () {
    pset_stub ("do_define_struct()");
}

extern "C" void popup_context_menu () {
    pset_stub ("popup_context_menu()");
}

void InstanceRep::handle_remove(void)
{
    pset_stub ("InstanceRep::handle_remove(void)");
}

int InstanceRep::use_own_name(void)
{
    pset_stub ("InstanceRep::use_own_name(void)");
    return 0;
}

int InstanceRep::use_own_icon(void)
{
    pset_stub ("InstanceRep::use_own_icon(void)");
    return 0;
}

InstanceRep::~InstanceRep(void)
{
    pset_stub ("InstanceRep::~InstanceRep(void)");
}

unsigned char InstanceRep::get_icon(void)
{
    pset_stub ("InstanceRep::get_icon(void)");
    return 0;
}

void InstanceRep::print(ostream&, int) const
{
    pset_stub ("InstanceRep::print(ostream&, int) const");
}

void InstanceRep::notify(int, Relational*, objPropagator*, RelType*) 
{
    pset_stub ("InstanceRep::notify(int, Relational*, objPropagator*, RelType*) ");
}

symbolPtr InstanceRep::get_associated_symbol(void) 
{
    pset_stub ("InstanceRep::get_associated_symbol(void) ");
    return symbolPtrReturn;
}

char *InstanceRep::get_name(void) const 
{
    pset_stub ("InstanceRep::get_name(void) const ");
    return NULL;
}


void FeedBack::message(StreamID , const char* ) {
    pset_stub ("FeedBack::message");
}

void FeedBack::put_fail_clean_up() {
    pset_stub ("FeedBack::put_fail_clean_up");
}

void FeedBack::report(StreamID , const char* , int , projNode* , projNode* , symbolPtr& ) {
    pset_stub ("FeedBack::report");
}

void FeedBackMonitor::update_merge_status(const char *, const char *, int ) {
    pset_stub ("FeedBackMonitor::update_merge_status");
}


void cleanup_miniBrowsers(void) {
    pset_stub ("cleanup_miniBrowser(void)");
}

InstanceController::InstanceController(symbolArr& , char *, char *, miniBrowser *) {
    pset_stub ("InstanceController::InstanceController");
}

void InstanceController::browse(void) {
    pset_stub ("InstanceController::browse");
}

symbolArr& InstanceController::get_representation(void) {
    pset_stub ("InstanceController::get_representation");
    return representation;
}

unsigned char InstanceController::get_sym_icon(int) {
    pset_stub ("InstanceController::get_sym_icon");
    return 0;
}

symbolPtr InstanceController::get_symbol(int ind) {
    pset_stub ("InstanceController::get_symbol");
    return instances[ind];
}

void InstanceController::handle_modification(int, InstanceRep *) {
    pset_stub ("InstanceController::handle_modification");
}

void InstanceController::handle_remove_rep(int ) { 
    pset_stub ("InstanceController::handle_remove_rep");
}

int InstanceController::representation_get_index(symbolPtr ) {
    pset_stub ("InstanceController::representation_get_index");
    return 0;
}

Relational* InstanceController::representation_get_real(symbolPtr ) {
    pset_stub ("InstanceController::representation_get_real");
    return NULL;
}

int InstanceController::rtl_dying(RTL *) {
    pset_stub ("InstanceController::rtl_dying");
    return 1;
}

InstanceController::~InstanceController() {
    pset_stub ("InstanceController::~InstanceController");
}

void InstanceController::selection_callback(RTL*, symbolArr&, miniBrowser *)
{
    pset_stub ("InstanceController::selection_callback(RTL*, symbolArr&, miniBrowser *)");
}

int InstanceController::use_own_names(void)
{
    pset_stub ("InstanceController::use_own_names(void)");
    return 0;
}

int InstanceController::use_own_icons(void)
{
    pset_stub ("InstanceController::use_own_icons(void)");
    return 0;
}

char *InstanceController::get_sym_name(int) 
{
    pset_stub ("InstanceController::get_sym_name(int)");
    return NULL;
}

objArrPtr RTL::search (commonTreePtr, void* , int, int) {
    pset_stub ("RTL::search");
    return NULL;
}

// oodt_class_member_sorter destructor.
// Needed only for HP link.
void oodt_class_member_sorter::sort(objArr&) {
    pset_stub ("oodt_class_member_sroter::sort(objArr&)");
};

extern "C" void Panner_perspective(Widget, int, int, int, int, int, int, int, int) {
    pset_stub ("Panner_perspective");
}

extern "C" void Panner_values(Widget,int *, int *, int *, int *, int *, int *, int *, int *) {
    pset_stub ("Panner_values");
}

extern "C" void apl_OODT_get_class_pointers (char *, char **, steDocumentPtr *, steDocumentPtr *) 
{
    pset_stub ("apl_OODT_get_class_pointers");
}

extern "C" ddElementPtr apl_OODT_create_modify_class_w_dir (const char* , const char* , const char* , const char* , const char* , steDocumentPtr , steDocumentPtr )
{
    pset_stub ("apl_OODT_create_modify_class_w_dir");
    return NULL;
}

extern "C" ddElementPtr apl_OODT_create_func_in_subsys (const char*, const char*, const char *, const char*, const char*, const char*, const char*, fileLanguage, bool) {
    pset_stub ("apl_OODT_create_func_in_subsys");
    return NULL;
}

extern "C" groupHdrPtr apl_OODT_get_subsys_from_rtl(RTLNodePtr , const char* ) {
    pset_stub ("apl_OODT_get_subsys_from_rtl");
    return NULL;
}

extern "C" char* apl_SPD_Get_Value (char*, char* ) {
    pset_stub ("apl_SPD_Get_Value");
    return NULL;
}

void astTree::printTree (ostream& ) const {
    pset_stub ("astTree::printTree");
}

extern "C" void ast_dbg_break (astTree* ) {
    pset_stub ("ast_dbg_break");
}

extern "C" bool aus_weights_read_subsys_setting(const char *) {
    pset_stub ("aus_weights_read_subsys_setting");
    return 0;
}


void bring_error_messages_on_top(void)
{
     pset_stub("bring_error_messages_on_top(void)");
}


void bring_modal_dialog_to_top(void)
{
     pset_stub("bring_modal_dialog_to_top(void)");
}


bool browser_delete_module(gtBase*, projModule*, bool, const char*)
{
     pset_stub("browser_delete_module(gtBase*, projModule*, bool, const char*)");
     return 0;
}


int cmd_available_p(int) // used in extracting outline view
{
     return 1;
}


int cmd_break(void)
{
     pset_stub("cmd_break(void)");
     return 0;
}

int cmd_old_symbol_display_format()
{
  pset_stub("cmd_old_symbol_display_format(void)");
  return 0;
}

extern "C" void cmd_do_flush()
{
    pset_stub("cmd_do_flush");
}

extern "C" int cmd_do_validation()
{
    pset_stub("cmd_do_validation");
    return 0;
}

int cmd_exec_continue(void)
{
     pset_stub("cmd_exec_continue(void)");
     return 0;
}


void cmd_gen_real_write(const char*, va_list )
{
     pset_stub("cmd_gen_real_write(const char*, va_list)");
 }


view *cmd_get_cur_view(void)
{
     pset_stub("cmd_get_cur_view(void)");
     return NULL;
}


extern "C" int cmd_get_do_popup_comp_name()
{
    pset_stub("cmd_get_do_popup_comp_name");
    return 1;
}

extern "C" int cmd_get_do_save()
{
    pset_stub("cmd_get_do_save");
    return 0;
}

int cmd_is_no_epoch_macro(void)
{
    pset_stub("cmd_is_no_epoch_macro(void)");
    return 0;
}

extern "C" void cmd_journal_execute()
{
    pset_stub("cmd_journal_execute");
}

extern "C" void cmd_journal_log()
{
    pset_stub("cmd_journal_log");
}

extern "C" void cmd_journal_save()
{
    pset_stub("cmd_journal_save");
}

extern "C" void cmd_journal_start()
{
    pset_stub("cmd_journal_start");
}


void cmd_move_view_arrow(view*, int, int)
{
    pset_stub("cmd_move_view_arrow(view*, int, int)");
}

void cmd_record_view(view*)
{
    pset_stub("cmd_record_view(view*)");
}

extern "C" void cmd_set_do_save()
{
    pset_stub("cmd_set_do_save");
}

int cmd_set_epoch_recording(int)
{
    pset_stub("cmd_set_epoch_recording(int)");
    return 0;
}

void cmd_synchronize(const char *)
{
    pset_stub("cmd_synchronize(const char *)");
}

void cmd_synchronize_parser(const char*)
{
    pset_stub("cmd_synchronize_parser(const char*)");
}

int cmd_validate_p(int)
{
    pset_stub("cmd_validate_p(int)");
    return 0;
}

int cmd_no_DD_VERSION()
{
    pset_stub("cmd_no_DD_VERSION(int)");
    return 0;
}

void cmd_validation_error(const char*)
{
    pset_stub("cmd_validation_error(const char*)");
}

void cmd_validation_warning(const char*)
{
    pset_stub("cmd_validation_warning(const char*)");
}

char * cmd_xterm_input_file;

objArrPtr commonTree::search(commonTree*, void*, int, int)
{
    pset_stub("commonTree::search(commonTree*, void*, int, int)");
    return NULL;
}

int create_view_of_def(symbolPtr)
{
    pset_stub("create_view_of_def(symbolPtr)");
    return 0;
}

void create_wildcard(projNode*, genString&, char)
{
    pset_stub("create_wildcard(projNode*, genString&, char)");
}

void do_extract_entities(symbolArr&, symbolArr&)
{
    pset_stub("do_extract_entities(symbolArr&, symbolArr&)");
}

void expand_groups(symbolArr&)
{
    pset_stub("expand_groups(symbolArr&)");
}

int fb_merge_one_file()
{
    pset_stub("int fb_merge_one_file()");
    return 0;
}

bool fb_files_being_put(const char*)
{
    pset_stub("fb_files_being_put(const char*)");
    return 0;
}

int fb_put_one_file(genString&, genString&, genString&, genString&, genString&, genString&, genString&, genString&, genString&, genString&)
{
    pset_stub("fb_put_one_file(genString&, genString&, genString&, genString&, genString&, genString&, genString&, genString&, genString&, genString&)");
    return 0;
}

void fb_status_one_file(const char*, int, projNode*, projModule*)
{
    pset_stub("fb_status_one_file(const char*, int, projNode*, projModule*)");
}

void free_wildcard_split_buffer(char**, int)
{
    pset_stub("free_wildcard_split_buffer(char**, int)");
}

void generate_wildcard_match_pattern(char*)
{
    pset_stub("generate_wildcard_match_pattern(char*)");
}

appPtr get_create_app_by_name(const char*)
{
    pset_stub("get_create_app_by_name(const char*)");
    return NULL;
}

void get_files_object_to_propagate(symbolArr&, symbolArr&, appTree*)
{
    pset_stub("get_files_object_to_propagate(symbolArr&, symbolArr&, appTree*)");
}

int get_impact_analysis(symbolArr&, symbolArr&)
{
    pset_stub("get_impact_analysis(symbolArr&, symbolArr&)");
    return 0;
}

void get_link_add_results(symbolPtr, linkType, symbolSet&)
{
    pset_stub("get_link_add_results(symbolPtr, linkType, symbolSet&)");
}

symbolPtr get_smt_decl_from_init(symbolPtr, genString&)
{
    pset_stub("get_smt_decl_from_init(symbolPtr, genString&)");
    return symbolPtrReturn;
}



void gtBase::manage(void)
{
    pset_stub("gtBase::manage(void)");
}

void gtDialog::popdown(void)
{
    pset_stub("gtDialog::popdown(void)");
}

void gtDialog::popup(int)
{
    pset_stub("gtDialog::popup(int)");
}
#ifdef XXX_gtDisplay_is_open
int gtDisplay_is_open(void)
{
    pset_stub("gtDisplay_is_open(void)");
    return 0;
}
#endif
void gtPrimitive::attach(gtCardinalDir, gtBase*, int, int)
{
    pset_stub("gtPrimitive::attach(gtCardinalDir, gtBase*, int, int)");
}

void gtPrimitive::install_accelerator(gtPrimitive*)
{
    pset_stub("gtPrimitive::install_accelerator(gtPrimitive*)");
}

void gtPrimitive::set_sensitive(int)
{
    pset_stub("gtPrimitive::set_sensitive(int)");
}

void gtPushButton_next_help_context(const char*)
{
    pset_stub("gtPushButton_next_help_context(const char*)");
}

gtRTL_spec::gtRTL_spec(char*, char*, char*, char*, char*) 
{
    pset_stub("gtRTL_spec::gtRTL_spec(char*, char*, char*, char*, char*)");
}

void gtRTL::update_status(void)
{
    pset_stub("gtRTL::update_status(void)");
}

gtRTL_cfg::gtRTL_cfg(void)
{
    pset_stub("gtRTL_cfg::gtRTL_cfg(void)");
}

int has_view(app*)
{
    pset_stub("has_view(app*)");
    return 0;
}

extern "C" void *import_any_file()
{
    pset_stub("import_any_file");
    return NULL;
}

objArrPtr ldr::search(commonTree*, void*, int, int)
{
    pset_stub("ldr::search(commonTree*, void*, int, int)");
    return NULL;
}

int makeUNIXFile(genString&)
{
    pset_stub("makeUNIXFile(genString&)");
    return 0;
}

int miniBrowserRTL_API::rtl_dying(RTL*)
{
    pset_stub("miniBrowserRTL_API::rtl_dying(RTL*)");
    return 0;
}

void miniBrowserRTL_API::add_button(gtPrimitive*)
{
    pset_stub("miniBrowserRTL_API::add_button(gtPrimitive*)");
}

gtBase *miniBrowserRTL_API::get_button_control(void)
{
    pset_stub("miniBrowserRTL_API::get_button_control(void)");
    return NULL;
}

char *miniBrowserRTL_API::get_sort_spec(RTL*)
{
    pset_stub("miniBrowserRTL_API::get_sort_spec(RTL*)");
    return NULL;
}

void miniBrowserRTL_API::make_own_buttons(gtBase *parent, gtRTL *gt_rtl, RTL *rtl)
{
    pset_stub("miniBrowserRTL_API::make_own_buttons(gtBase*, gtRTL *, RTL*)");
}

void miniBrowserRTL_API::rtl_out_of_scope(RTL*)
{
    pset_stub("miniBrowserRTL_API::rtl_out_of_scope(RTL*)");
}

void miniBrowserRTL_API::set_list(gtList*)
{
    pset_stub("miniBrowserRTL_API::set_list(gtList*)");
}

int  miniBrowserRTL_API::need_vcr_control(RTL *)
{
    pset_stub("miniBrowserRTL_API::need_vcr_control(RTL *)");
    return 0;
}

void miniBrowserRTL_API::selection_callback (RTL*, symbolArr&, miniBrowser *)
{
    pset_stub ("miniBrowserRTL_API::selection_callback(RTL*, symbolArr&, minibrowser*)");
}

int  miniBrowserRTL_API::action_callback(RTL*, symbolArr&, miniBrowser *)
{
    pset_stub("miniBrowserRTL_API::action_callback(RTL*, symbolArr&, miniBrowser *)");
    return 0;
}

miniBrowserRTL_API::miniBrowserRTL_API()
{
    pset_stub("miniBrowserRTL_API::miniBrowserRTL_API()");
}

#define VMESSAGE(type,fmt)		\
{					\
    va_list ap;				\
    va_start(ap, fmt);			\
    vmessage(type, fmt, ap);		\
    va_end(ap);			\
}

static void vmessage(systemMessageType type, const char* fmt, va_list ap)
{
    if (!fmt || !fmt[0])
        return;

    genString vmessage_buffer;
    vmessage_buffer.vsprintf(fmt, ap);

    {
	const char* msg_type;

        switch (type) {
	case MSG_STATUS: msg_type = "Status:"; break;
	case MSG_INFORM: msg_type = "Informational:"; break;
	case MSG_WARN: msg_type = "Warning::"; break;
	case MSG_WORKING: msg_type = "Working:"; break;
	case MSG_DIAG: msg_type = "Diagnostic:"; break;
	case MSG_ERROR: msg_type = "Error:"; break;
	  default: msg_type = "Unknown error:"; break;
        }

	fprintf (stderr, "%s:%s\n", msg_type, vmessage_buffer.str());
	fflush(stderr);
    }
}

extern "C" void msg_status(const char* fmt...)
{
    VMESSAGE(MSG_STATUS,fmt)
    pset_stub("msg_status");
}

extern "C" void msg_inform(const char* fmt...)
{
    VMESSAGE(MSG_INFORM,fmt)
    pset_stub("msg_inform");
}

extern "C" void msg_diag(const char* fmt...)
{
    VMESSAGE(MSG_DIAG,fmt)
    pset_stub("msg_diag");
}

extern "C" void msg_warn(const char* fmt...)
{
    VMESSAGE(MSG_WARN,fmt)
    pset_stub("msg_warn");
}

extern "C" void msg_error(const char* fmt...)
{
    VMESSAGE(MSG_ERROR,fmt)
    pset_stub("msg_error");
}

extern "C" int msg_question(const char * ...)
{
    pset_stub("msg_question");
    return 0;
}

int offer_to_save_wait(gtBase*, dlgFlavor, objArr*, void (*)(void*), void*, int)
{
    pset_stub("offer_to_save_wait(gtBase*, dlgFlavor, objArr*, void (*)(void*), void*, int)");
    return 0;
}

int offer_to_unload_list(objArr*, symbolArr*)
{
    pset_stub("offer_to_unload_list(objArr*, symbolArr*)");
    return 0;
}



int pdf_being_reloaded(void)
{
    pset_stub("pdf_being_reloaded(void)");
    return 0;
}

extern "C" void ping_paraset()
{
    pset_stub("ping_paraset");
}

extern "C" void popup_Print()
{
    pset_stub("popup_Print");
}

int popup_Question(const char*, const char*, const char*, const char*)
{
    pset_stub("popup_Question(const char*, const char*, const char*, const char*)");
    return 0;
}

int popup_Question(const char*, const char*, const char*, const char*, const char*)
{
    pset_stub("popup_Question(const char*, const char*, const char*, const char*, const char*)");
    return 0;
}

int print_symbolArr(symbolArr&, FILE*, char*)
{
    pset_stub("print_symbolArr(symbolArr&, FILE*, char*)");
    return 0;
}

app *proj_load_model(objRawApp*)
{
    pset_stub("proj_load_model(objRawApp*)");
    return NULL;
}

int proj_loading_in_progress(objRawApp*)
{
    pset_stub("proj_loading_in_progress(objRawApp*)");
    return 0;
}

void propManager_update_fetch(projModule*)
{
    pset_stub("propManager_update_fetch(projModule*)");
}

CPManager * prop_manager = 0;

void pset_dispatch_own_callback(_XEvent*)
{
    pset_stub("pset_dispatch_own_callback(_XEvent*)");
}

extern "C" void reparse_finish()
{
    pset_stub("reparse_finish");
}

void set_preferred_viewer(viewer*)
{
    pset_stub("set_preferred_viewer(viewer*)");
}

void set_put_is_running(void)
{
    pset_stub("set_put_is_running(void)");
}

extern "C" void set_top_no_epoch()
{
    pset_stub("set_top_no_epoch");
}

void share_re_load_pdf(char*)
{
    pset_stub("share_re_load_pdf(char*)");
}

char **split_wildcard(genString&, int&)
{
    pset_stub("split_wildcard(genString&, int&)");
    return NULL;
}

propManager *start_propagation(symbolArr&, void*, char*)
{
    pset_stub("start_propagation(symbolArr&, void*, char*)");
    return NULL;
}

void Edit_Wildcard_Group::static_sort_wildcard(genString&)
{
    pset_stub("Edit_Wildcard_Group::static_sort_wildcard(genString&)");
}

#ifndef _WIN32
void FeedBack::global_check_for_errors(projModule*, const char*)
{
    pset_stub("FeedBack::global_check_for_errors(projModule*, const char*)");
}
#endif

void browserShell::clear_all_browser_selections(browserShell*)
{
    pset_stub("browserShell::clear_all_browser_selections(browserShell*)");
}

void browserShell::quit(void)
{
    pset_stub("browserShell::quit(void)");
}

void dd_boil::pop_dd_boilerplate(DD_boil, void*)
{
    pset_stub("dd_boil::pop_dd_boilerplate(DD_boil, void*)");
}

void dd_boil::refresh_after_delete(void)
{
    pset_stub("dd_boil::refresh_after_delete(void)");
}

void gtBase::epoch_take_control(int (*)(void*), void*)
{
    pset_stub("gtBase::epoch_take_control(int (*)(void*), void*)");
}

void gtBase::flush_output(void)
{
    pset_stub("gtBase::flush_output(void)");
}

gtDialogTemplate*gtDialogTemplate::create(gtBase*, const char*, const char*)
{
    pset_stub("gtDialogTemplate::create(gtBase*, const char*, const char*)");
    return NULL;
}

int gtDisplay::is_open(void)
{
    pset_stub("gtDisplay::is_open(void)");
    return 0;
}

gtLabel* gtLabel::create(gtBase*, const char*, const char*)
{
    pset_stub("gtLabel::create(gtBase*, const char*, const char*)");
    return NULL;
}

void gtPushButton::next_help_context(const char*)
{
    pset_stub("gtPushButton::next_help_context(const char*)");
}

void gtRTL_boil::refresh_after_delete(void)
{
    pset_stub("gtRTL_boil::refresh_after_delete(void)");
}

gtRadioBox* gtRadioBox::create(gtBase*, const char*, const char*, ...)
{
    pset_stub("gtRadioBox::create(gtBase*, const char*, const char*, ...)");
    return NULL;
}

gtStringEditor* gtStringEditor::create(gtBase*, const char*, const char*)
{
    pset_stub("gtStringEditor::create(gtBase*, const char*, const char*)");
    return NULL;
}

gtToggleButton* gtToggleButton::create(gtBase*, const char*, const char*, void (*)(gtToggleButton*, _XEvent*, void*, gtReason), void*)
{
    pset_stub("gtToggleButton::create(gtBase*, const char*, const char*, void (*)(gtToggleButton*, _XEvent*, void*, gtReason), void*)");
    return NULL;
}

gtVertBox* gtVertBox::create(gtBase*, const char*)
{
    pset_stub("gtVertBox::create(gtBase*, const char*)");
    return NULL;
}

void miniBrowser::clear_selections(void)
{
    pset_stub("miniBrowser::clear_selections(void)");
}

void miniBrowser::fill_selected_nodes(symbolArr&)
{
    pset_stub("miniBrowser::fill_selected_nodes(symbolArr&)");
}

void projectBrowser::add_group_name(const char*, bool)
{
    pset_stub("projectBrowser::add_group_name(const char*, bool)");
}

void projectBrowser::batch_root_impact(const char*, const char*, FILE*, FILE*)
{
    pset_stub("projectBrowser::batch_root_impact(const char*, const char*, FILE*, FILE*)");
}

viewer::~viewer()
{
    pset_stub ("viewer::~viewer()");
}

RelationalPtr viewer::rel_copy () const 
{
    pset_stub ("RelationalPtr viewer::rel_copy () const");
    return NULL;
}

void viewer::print(ostream& , int ) const 
{
    pset_stub ("void viewer::print(ostream& st, int level) const");
}

viewPtr viewer::get_current_view_of_screen(steScreen*)
{
    pset_stub("viewer::get_current_view_of_screen(steScreen*)");
    return NULL;
}

steScreenPtr viewer::get_screen_of_view(view*)
{
    pset_stub("viewer::get_screen_of_view(view*)");
    return NULL;
}

viewer *viewer::preferred_viewer(void)
{
    pset_stub("viewer::preferred_viewer(void)");
    return NULL;
}

extern "C" void viewer_target_lock(viewer* vwr, int is_set)
{
    pset_stub ("void viewer_target_lock(viewer* vwr, int is_set)");
}

void viewerShell::set_current_vs(viewerShell* vs)
{
    pset_stub("void viewerShell::set_current_vs(viewerShell* vs)");
}

viewerShell* viewerShell::get_current_vs(int)
{
    pset_stub("viewerShell::get_current_vs(int)");
    return NULL;
}

viewerShell *viewerShell::get_current_vs(void)
{
    pset_stub("viewerShell::get_current_vs(void)");
    return NULL;
}

viewerShell *viewerShell::viewerShell_of_screen(steScreen*)
{
    pset_stub("viewerShell::viewerShell_of_screen(steScreen*)");
    return NULL;
}

viewerShell *viewerShell::viewerShell_of_view(view*)
{
    pset_stub("viewerShell::viewerShell_of_view(view*)");
    return NULL;
}

viewer *viewerShell::viewer_of_rtlview(viewListHeader*)
{
    pset_stub("viewerShell::viewer_of_rtlview(viewListHeader*)");
    return NULL;
}

void system_message_force_logger(int)
{
    pset_stub("system_message_force_logger(int)");
}


bool translate_dir_path(const char*, const char*, const char*, const char*, unsigned int, genString&)
{
    pset_stub("translate_dir_path(const char*, const char*, const char*, const char*, unsigned int, genString&)");
    return 0;
}

void ui_INTERP_info::remove_hook(void)
{
    pset_stub("ui_INTERP_info::remove_hook(void)");
}


void ui_INTERP_info::set_hook(void (*)(int, void*), unsigned long, void*)
{
    pset_stub("ui_INTERP_info::set_hook(void (*)(int, void*), unsigned long, void*)");
}


ui_INTERP_info::ui_INTERP_info(int)
{
    pset_stub("ui_INTERP_info::ui_INTERP_info(int)");
}

ui_INTERP_info::~ui_INTERP_info(void)
{
    pset_stub("ui_INTERP_info::~ui_INTERP_info(void)");
}

extern "C" char *ui_get_value()
{
    pset_stub("ui_get_value");
    return NULL;
}

extern "C" void ui_set_scroll_value()
{
    pset_stub("ui_set_scroll_value");
}

extern "C" void ui_set_scroll_range()
{
    pset_stub("ui_set_scroll_range");
}

extern "C" void ui_resize_window ()
{
    pset_stub("ui_resize_window");
}

extern "C" psetCONST char *ui_get_resource()
{
    pset_stub("ui_get_resource");
    return NULL;
}

extern "C" void ui_set_child_sensitive()
{
    pset_stub("ui_set_child_sensitive()");
}

extern "C" void ui_get_scroll_range()
{
    pset_stub("ui_get_scroll_range()");
}

extern "C" void ui_set_slider_size()
{
    pset_stub("ui_set_slider_size()");
}

void ui_rowcolumn::add(int, char*)
{
    pset_stub("ui_rowcolumn::add(int, char*)");
}

void ui_rowcolumn::clear(void)
{
    pset_stub("ui_rowcolumn::clear(void)");
}

void ui_rowcolumn::regenerate(void)
{
    pset_stub("ui_rowcolumn::regenerate(void)");
}

void update_rem_group_manager(const char*)
{
    pset_stub("update_rem_group_manager(const char*)");
}

void view_create_flush()
{
    pset_stub ("void view_create_flush()");
}

viewPtr view_create(appPtr app_head)
{
    pset_stub("viewPtr view_create(appPtr app_head)");
    return NULL;
}

boolean view_get_help_focus(void)
{
    pset_stub("view_get_help_focus(void)");
    return 0;
}

void view_select(view*)
{
    pset_stub("view_select(view*)");
}

viewer *view_set_target_viewer(viewer*)
{
    pset_stub("view_set_target_viewer(viewer*)");
    return NULL;
}

viewer *view_target_viewer(void)
{
    pset_stub("view_target_viewer(void)");
    return NULL;
}

void viewer::change_view(view*, int)
{
    pset_stub("viewer::change_view(view*, int)");
}

void viewer::enable_reparse(bool)
{
    pset_stub("viewer::enable_reparse(bool)");
}

steScreen *viewer::get_screen(void)
{
    pset_stub("viewer::get_screen(void)");
    return NULL;
}

void viewer::update_views_option_menu(void)
{
    pset_stub("viewer::update_views_option_menu(void)");
}

int viewerShell::configure(app*, viewer*&, view*&)
{
    pset_stub("viewerShell::configure(app*, viewer*&, view*&)");
    return 0;
}

viewPtr viewerShell::get_current_view(void)
{
    pset_stub("viewerShell::get_current_view(void)");
    return NULL;
}

void viewerShell::map(void)
{
    pset_stub("viewerShell::map(void)");
}

steScreenPtr viewerShell::next_screen(steScreen*, view*&) const 
{
    pset_stub("viewerShell::next_screen(steScreen*, view*&)");
    return NULL;
}

viewPtr viewerShell::open_view(app*, repType, appTree*)
{
    pset_stub("viewerShell::open_view(app*, repType, appTree*)");
    return NULL;
}

void viewerShell::split(view*)
{
    pset_stub("viewerShell::split(view*)");
}

viewer *viewerShell::viewer_of_view(view*)
{
    pset_stub("viewerShell::viewer_of_view(view*)");
    return NULL;
}

extern "C" void viewerShell_map()
{
    pset_stub("viewerShell_map");
}

extern "C" void viewerShell_open_view(app*, repType, appTree*)
{
    pset_stub("viewerShell_open_view");
}

char** GenArr::del(int)
{
    pset_stub("GenArr::del(int)");
    return NULL;
}

int GenArr::search(char *)
{
    pset_stub("GenArr::search(char *)");
    return 0;
}

char** GenArr::append(char*) 
{
    pset_stub("GenArr::append(char*)");
    return NULL;
}

void GenArr::reset()
{
    pset_stub("GenArr::reset");
}

void GenArr::sort()
{
    pset_stub("GenARr::sort");
}

char** GenArr::del(char *)
{
    pset_stub("GenArr::del(char *)");
    return NULL;
}

int wildcard_accept(const char*)
{
    pset_stub("wildcard_accept(const char*)");
    return 0;
}

int xrefSymbol::get_function_prototype(genString&)
{
    pset_stub("xrefSymbol::get_function_prototype(genString&)");
    return 0;
}

#ifdef _WIN32
DBM     *dbm_open(char *, int, int)
{
 // pset_stub ("DBM     *dbm_open(char *, int, int)");
 // dbm operations appear only to be used in association with symbolmanager
 // stuff (15.aug.96 kit transue)
 return (DBM *) NULL;
}
 
void    dbm_close(DBM *)
{
 pset_stub ("void    dbm_close(DBM *)");
}

datum   dbm_fetch(DBM *, datum)
{
 pset_stub("datum   dbm_fetch(DBM *, datum)");
 datum dTm;
 memset( &dTm, 0, sizeof(dTm) );
 return dTm; 
}

#endif /*_WIN32*/


void StatusDialog::reestablish_canceller()
{
  pset_stub( "void StatusDialog::reestablish_canceller()" );
}

void ddbuild_hook(smtHeader*)
{
  pset_stub( "ddbuild_hook(smtHeader*)" );
}

#ifndef _WIN32
void
purge_tmp_selections()
{
    pset_stub( "purge_tmp_selections()" );
}
#endif /*!_WIN32*/

void add_activity(const char *str)
{
  pset_stub("void add_activity(const char *str)");
}

void move_client_from_active_to_ext(int pos)
{
  pset_stub("void move_client_from_active_to_ext(int pos)");
}

void move_client_from_active_to_ext(char *)
{
  pset_stub("void move_client_from_active_to_ext(char *)");
}


void move_client_from_ext_to_active(char *)
{
  pset_stub("void move_client_from_ext_to_active(char *)");
}


extern "C" void assoc_create_hard (void)
{
  pset_stub("void assoc_create_hard (void)");
}

void gpiDialog_fix_ext_app_ui(char *)
{
  pset_stub("void gpiDialog_fix_ext_app_ui(char *)");
}


int offer_to_save_list_wait(gtBase *,dlgFlavor,objArr *,
                            void (*cb)(void *),void *,int)
{
  pset_stub("int offer_to_save_list_wait()");
  return 0;
}

int gpi_assoc_browser_in(struct data_cntx *)
{
  pset_stub("int gpi_assoc_browser_in(struct data_cntx *)");
  return 0;
}


int Layer::PrepareShutdown()
{return 1;}

#ifndef _WIN32

extern "C" int Tk_Init(Tcl_Interp *)
{
    return 1;
}

#endif

