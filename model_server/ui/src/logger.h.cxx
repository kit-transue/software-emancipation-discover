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
// logger.h.C
//------------------------------------------
// synopsis:
// Logger dialog in libGT
//
//------------------------------------------
// NOTE: This file used to be named "Logger.h.C", but the upper-case 'L'
// at the start of the name caused the C++ compiler to discard debugging
// information for this file!
//------------------------------------------

// INCLUDE FILES

#include <machdep.h>

#include <cLibraryFunctions.h>
#include <msg.h>
#include <messages.h>
#include <prompt.h>

#include <genError.h>
#include <genString.h>
#include <representation.h>
#include <psetmem.h>

#include <Logger.h>

#include <gtForm.h>
#include <gtLabel.h>
#include <gtList.h>
#include <gtTextEd.h>
#include <gtPanedWindow.h>
#include <gtCascadeB.h>
#include <gtPushButton.h>
#include <gtSepar.h>
#include <gtBitmap.h>
#include <gtRTL.h>
#include <options.h>

#define first_width 16
#define first_height 16
static char first_bits[] = {
   0x80, 0x00, 0xc0, 0x01, 0xe0, 0x03, 0xf0, 0x07, 0xf8, 0x0f, 0xfc, 0x1f,
   0xfe, 0x3f, 0xff, 0x7f, 0x80, 0x00, 0xc0, 0x01, 0xe0, 0x03, 0xf0, 0x07,
   0xf8, 0x0f, 0xfc, 0x1f, 0xfe, 0x3f, 0xff, 0x7f};
#define last_width 16
#define last_height 16
static char last_bits[] = {
   0xff, 0x7f, 0xfe, 0x3f, 0xfc, 0x1f, 0xf8, 0x0f, 0xf0, 0x07, 0xe0, 0x03,
   0xc0, 0x01, 0x80, 0x00, 0xff, 0x7f, 0xfe, 0x3f, 0xfc, 0x1f, 0xf8, 0x0f,
   0xf0, 0x07, 0xe0, 0x03, 0xc0, 0x01, 0x80, 0x00};
#define next_width 16
#define next_height 16
static char next_bits[] = {
   0xfe, 0x3f, 0xfe, 0x3f, 0xfc, 0x1f, 0xfc, 0x1f, 0xf8, 0x0f, 0xf8, 0x0f,
   0xf0, 0x07, 0xf0, 0x07, 0xe0, 0x03, 0xe0, 0x03, 0xc0, 0x01, 0xc0, 0x01,
   0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
#define prev_width 16
#define prev_height 16
static char prev_bits[] = {
   0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0xc0, 0x01, 0xc0, 0x01, 0xe0, 0x03,
   0xe0, 0x03, 0xf0, 0x07, 0xf0, 0x07, 0xf8, 0x0f, 0xf8, 0x0f, 0xfc, 0x1f,
   0xfc, 0x1f, 0xfe, 0x3f, 0xfe, 0x3f, 0x00, 0x00};
#define curr_width 16
#define curr_height 16
static char curr_bits[] = {
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf0, 0x0f, 0xf0, 0x0f,
   0xf0, 0x0f, 0xf0, 0x0f, 0xf0, 0x0f, 0xf0, 0x0f, 0xf0, 0x0f, 0xf0, 0x0f,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

// EXTERNAL FUNCTIONS

extern "C"
{
    void cmd_do_flush();
}

void HelpUI_LongHelp(gtBase*);

genArrOf(messageRecordPtr) Logger::records;
int Logger::transaction_level = 0;
static genString transaction_text;
static genString transaction_help_context;
static int disabled;
static int help_disabled;
static gtDialogTemplate* the_logger_parent = 0;

Logger* Logger::logger_instance = 0;

// FUNCTION DEFINITIONS

void logger_parent(gtDialogTemplate* parent) 
{
    if(the_logger_parent != parent){  // Check that parent is really changed before changing & removing log window.
	the_logger_parent = parent;
	if (parent == NULL && Logger::logger_instance) {
	    delete Logger::logger_instance;
	}
    }
}

void logger_clear() 
{
    Initialize(logger_clear);

    if (Logger::logger_instance)
        Logger::logger_instance->clear();
}

void logger_disable() 
{
    disabled = 1;
}

void logger_enable() 
{
    disabled = 0;
}

extern "C" void logger_help_disable()
{
    internal_logger_help_disable();
}

void internal_logger_help_disable()
{
    help_disabled = 1;
#ifndef NEW_UI
    if (Logger::logger_instance) {
	Logger::logger_instance->help_button->set_sensitive (0);
	Logger::logger_instance->logger_help_button->set_sensitive (0);
    }
#endif
}

extern "C" void logger_help_enable()
{
    internal_logger_help_enable();
}

void internal_logger_help_enable() 
{
    help_disabled = 0;
#ifndef NEW_UI
    if (Logger::logger_instance) {
	Logger::logger_instance->scroll (Logger::logger_instance->error_marker);
	Logger::logger_instance->logger_help_button->set_sensitive (1);
    }
#endif
}

void logger_start_transaction()
{
    Logger::transaction_level++;
}


void logger_flush_transactions()
{
    Logger::transaction_level = 1;
    logger_end_transaction();
}

void logger_end_transaction()
{
    Logger::transaction_level--;
  
    if (Logger::transaction_level <= 0) {
        Logger::transaction_level = 0;
        if (transaction_text.length() > 0) {
	    genString str = transaction_text;
	    transaction_text = NULL;
#ifndef NEW_UI
            gtPushButton::next_help_context (transaction_help_context);
#endif
//	    cmd_do_flush();
	    msg("STATUS: $1") << (char *)str << eom;
        }
    }
}

int logger_wm_close(void* /* data */ )
{
    if (Logger::logger_instance) {
	delete Logger::logger_instance;
    }

    return 0;
}

void *Logger::operator new(size_t sz) {
   Initialize(Logger::Logger);

   if (Logger::transaction_level > 0)  return NULL;
   return  psetcalloc(sz, 1);
}

Logger::Logger(gtBase* parent, const char* title,
	       void (*destroy_CB)(gtBase*, void*)):
      text_ed(NULL), text_ed0(NULL), top_level(NULL), window(NULL),
      error_marker(-1), save_done_help(0), suspended(0)
{
   Initialize(Logger::Logger);

    logger_instance = this;
#ifndef NEW_UI
    if(title == NULL)
	title = gettext(TXT("Log"));

    top_level = gtTopLevelShell::create(the_logger_parent ? (gtBase*)the_logger_parent : parent, "logger");
    top_level->title (TXT("Message Logger"));
 
     if(destroy_CB)
	top_level->user_destroy(destroy_CB, this);

     top_level->override_WM_destroy (logger_wm_close);

    window = gtMainWindow::create(top_level, "main_window", 0);
    window->manage();
    
    gtMenuBar* menu_bar = window->menu_bar();

    gtCascadeButton* cascade_button = gtCascadeButton::create(
        menu_bar, "loggert", TXT("File"), NULL, NULL);
 
    cascade_button->pulldown_menu(
        "loggerr_menu",
	gtMenuStandard, "clear", TXT("Clear"), this, clear_CB,
         gtMenuStandard, "save", TXT("Save..."), this, save_CB, 
	gtMenuSeparator, "sep",
        gtMenuStandard, "configure", TXT("Configure..."), this, config_CB,
	gtMenuSeparator, "sep",
        gtMenuStandard, "close", TXT("Close Window"), this, ok_CB,
        NULL);
    cascade_button->manage();
	 
    gtForm* form = gtForm::create(window, "form");
    form->attach_tblr();
    form->manage();

    gtPushButton* ok = gtPushButton::create (form, "ok", "OK", ok_CB, this);
    ok->attach (gtBottom);
    ok->attach (gtLeft);
    ok->attach_pos (gtRight, 33);
    ok->manage();

    gtPushButton* clear = gtPushButton::create (form, "clear", "Clear", clear_CB, this);
    clear->attach (gtBottom);
    clear->attach_pos (gtLeft, 33);
    clear->attach_pos (gtRight, 66);
    clear->manage();

    logger_help_button = gtPushButton::create (form, "logger_help", "Help", help_logger_CB, this); //verify
/*
    // we do not have a context sensitive help for now
    logger_help_button->attach (gtBottom);
    logger_help_button->attach_pos (gtLeft, 66);
    logger_help_button->attach (gtRight);
    if (help_disabled) logger_help_button->set_sensitive(0);
    logger_help_button->help_context_name ("Pset.Help.Errors.Logger");
    logger_help_button->manage();
 */
   
    gtSeparator* sep = gtSeparator::create (form, "separator", gtHORZ);
    sep->attach (gtLeft);
    sep->attach (gtRight);
    sep->attach (gtBottom, ok, 5);
    sep->manage();

    gtBitmap* icon_curr = gtBitmap::create(
        form, "icon", curr_bits, curr_width, curr_height);
    gtBitmap* icon_prev = gtBitmap::create(
        form, "icon", prev_bits, prev_width, prev_height);
    gtBitmap* icon_next = gtBitmap::create(
        form, "icon", next_bits, next_width, next_height);
    gtBitmap* icon_first = gtBitmap::create(
        form, "icon", first_bits, first_width, first_height);
    gtBitmap* icon_last = gtBitmap::create(
        form, "icon", last_bits, last_width, last_height);
 

    gtPushButton* bottom = gtPushButton::create (form, "bottom", icon_last, bottom_CB, this);
    bottom->attach (gtTop);
    bottom->attach (gtRight);
    bottom->width(last_width*2);
    bottom->height(last_height*2);
    bottom->manage();

    gtPushButton* next = gtPushButton::create (form, "next", icon_next, next_CB, this);
    next->attach (gtTop);
    next->attach (gtRight, bottom);
    next->width(last_width*2);
    next->height(last_height*2);
    next->manage();

    gtPushButton* prev = gtPushButton::create (form, "prev", icon_prev, prev_CB, this);
    prev->attach (gtTop);
    prev->attach (gtRight, next);
    prev->width(last_width*2);
    prev->height(last_height*2);
    prev->manage();

    gtPushButton* top = gtPushButton::create (form, "top", icon_first, top_CB, this);
    top->attach (gtTop);
    top->attach (gtRight, prev);
    top->width(last_width*2);
    top->height(last_height*2);
    top->manage();

    gtPushButton* curr = gtPushButton::create (form, "curr", icon_curr, curr_CB, this);
    curr->attach (gtTop);
    curr->attach (gtRight, top);
    curr->width(last_width*2);
    curr->height(last_height*2);
    curr->manage();

    help_button = gtPushButton::create (form, "help_selected", "?",  gtBase::help_button_callback, NULL);
    help_button->attach (gtTop);
    help_button->attach (gtRight, top);
    help_button->width(last_width*2);
    help_button->height(last_height*2);
    help_button->set_sensitive(0);
    help_button->manage();

    error_label = gtLabel::create (form, "help_selected", "Message type:                     ");
    error_label->attach_offset (gtTop, 5);
    error_label->attach (gtLeft);
    error_label->manage();
    help_button->attach (gtRight, curr);
    gtSeparator* septop = gtSeparator::create (form, "separatortop", gtHORZ);
    septop->attach (gtLeft);
    septop->attach (gtRight);
    septop->attach (gtTop, top, 2);
    septop->manage();


    gtPanedWindow* paned = gtPanedWindow::create(form, "paned_window");
    paned->attach(gtTop, top);
    paned->attach(gtLeft);
    paned->attach(gtRight);
    paned->attach(gtBottom, sep, 5);
    paned->manage();

    genString record_text;
    for (int i=0; i<records.size(); i++) {
	record_text += (*records[i])->text;
    }

    error_marker = records.size() - 1;

    gtForm* tx0 = gtForm::create(paned, "tx0");
    tx0->manage();

    gtLabel* label0 = gtLabel::create(tx0, "label0", TXT("Message History"));
    label0->attach_tblr(NULL, label0);
    label0->manage();

    text_ed = gtTextEditor::create(tx0, "text_editor", record_text);
    text_ed->attach_tblr(label0);
    text_ed->read_only_mode();
    text_ed->manage();

    gtForm* tx1 = gtForm::create (paned, "tx1");
    tx1->manage ();

    gtLabel* label1 = gtLabel::create (tx1, "label1", TXT("Latest Message"));
    label1->attach_tblr (NULL, label1);
    label1->manage ();

    text_ed0 = gtTextEditor::create(tx1, "text_editor0", NULL);
    text_ed0->attach_tblr (label1);
    text_ed0->read_only_mode();
    text_ed0->manage();

    scroll (error_marker);
    size(8, 50);
    text_ed0->num_rows(3);
    text_ed0->num_columns(50);

    popup();
#endif
}

Logger::~Logger() {
   Initialize(Logger::~Logger);
#ifndef NEW_UI
   if (logger_instance == this) {
      if (last_record.text.length()) {	// save in history
	 messageRecord* rec = new messageRecord;
	 rec->type = last_record.type;
	 rec->text = last_record.text;
	 rec->help = last_record.help;
	 rec->start = text_ed->get_last_position();
	 rec->end = rec->start + rec->text.length();
	 messageRecord** rec_ptr = records.grow(1);
	 *rec_ptr = rec;
      }
      if (top_level) {
	 top_level->user_destroy(NULL, NULL);
	 top_level->override_WM_destroy(NULL);
	 delete top_level;
      }
      logger_instance = NULL;
   }
#endif
}

void Logger::operator delete(void* storage)
{
    Initialize(Logger::operator delete);
#ifndef NEW_UI
    psetfree(storage);
#endif
}

void Logger::size(int rows, int columns)
{
    text_ed->num_rows(rows);
    text_ed->num_columns(columns);
}

void Logger::popup(int)
{
    Initialize(Logger::popup);
#ifndef NEW_UI
    top_level->popup();
    top_level->update_display();
#endif
}

static const char* get_label_text (systemMessageType type)
{
    switch (type) {
        case MSG_STATUS: return "Message type: Status";
        case MSG_INFORM: return "Message type: Informational";
        case MSG_WARN:   return "Message type: Warning";
        case MSG_WORKING:return "Message type: Working";
        case MSG_DIAG:   return "Message type: Diagnostic";
        case MSG_ERROR:  return "Message type: Error";
        default:         return "Message type: Unknown";
    }
}

void Logger::append(const char* string, const char* context_name, systemMessageType type)
{
    Initialize(Logger::append);
#ifndef NEW_UI

    genString str;
    genString temp = string;

    if (type != MSG_DIAG) {
	temp.trim();
	temp += "\n";
    }


    str += temp;

    if (!this || Logger::transaction_level) {
	transaction_text += str;
        transaction_help_context = context_name;
	return;
    }

    messageRecord temp_record;
    temp_record.type  = last_record.type;
    temp_record.text  = last_record.text.str();
    temp_record.help  = last_record.help.str();

    last_record.type  = type;
    last_record.text  = str.str();
    last_record.help  = context_name;
    last_record.start = 0;
    last_record.end   = 1;
    text_ed0->text(last_record.text);
    scroll0();

    top_level->bring_to_top();
    int org_color = text_ed0->get_background_color();
    text_ed0->set_background_color(gtRTL::get_filtered_color());
    text_ed0->flush();
    text_ed0->update_display();
    OSapi_usleep(500000);
    text_ed0->set_background_color(org_color);

    str = temp_record.text.str();

    if (str.length() && !suspended && !disabled)
    {
 	messageRecord* rec = new messageRecord;
	rec->type = temp_record.type;
	rec->text = temp_record.text.str();
	rec->help = temp_record.help.str();

	rec->start = text_ed->get_last_position();
        if (get_main_option("-stderr"))
	    OSapi_fprintf (stderr, str);
	text_ed->append(str);
	rec->end = text_ed->get_last_position();

        messageRecord** rec_ptr = records.grow(1);
	*rec_ptr = rec;
	
	error_marker = records.size() - 1;
	
        scroll (error_marker);
    }
#endif
}


void Logger::suspend_CB(gtPushButton*, gtEventPtr, void* , gtReason)
{
    Initialize(Logger::suspend_CB);
#ifndef NEW_UI
    if (!Logger::logger_instance) return;

    if(Logger::logger_instance->suspended)
    {
	Logger::logger_instance->suspended = 0;
	Logger::logger_instance->append(gettext(TXT("Logging resumed.\n\n")));
    }
    else
    {
	Logger::logger_instance->append(gettext(TXT("\n\nLogging suspended...")));
	Logger::logger_instance->suspended = 1;
    }
#endif
}

    
void Logger::ok_CB(gtPushButton*, gtEventPtr, void* , gtReason)
{
    Initialize(Logger::ok_CB);
    if (!Logger::logger_instance) return;
#ifndef NEW_UI
    delete logger_instance;
#endif
}

void Logger::clear()
{
    Initialize(Logger::clear);
#ifndef NEW_UI
    for (int i=0; i<records.size(); i++) {
        (*records[i])->text = NULL;
        (*records[i])->help = NULL;
    }
    records.reset();
    error_marker = -1;
    text_ed->text("");
    text_ed0->text("");

    last_record.text = "";
    last_record.help = "";

    error_label->text ("Message type:                     ");
    help_button->help_context_name(NULL);
#endif
}

void Logger::clear_CB(gtPushButton*, gtEventPtr, void* , gtReason)
{
    Initialize(Logger::clear_CB);
#ifndef NEW_UI
    if (!Logger::logger_instance) return;
    Logger::logger_instance->clear();
#endif
}

void Logger::save_CB(gtPushButton*, gtEventPtr, void* , gtReason)
{
    Initialize(Logger::save_CB);
    if (!Logger::logger_instance) return;
#ifndef NEW_UI
    genString pathname;
    pathname.printf(TXT("%s/pset.log"), OSapi_getenv("HOME"));
    if (dis_prompt (PT_SAVELOG, 
                    P_SAVELOGOUTPUT, 
                    PB_OK, 
                    NULL, 
                    PB_CANCEL, 
                    PC_MSGLOGGER, 
                    pathname) > 0) {

	FILE* file = OSapi_fopen(pathname, "w");

	if(file == NULL)
	{
	    msg("ERROR: Cannot save log file:\n'$1'\n$2") << pathname.str() << eoarg << OSapi_strerror(errno) << eom;
	}
	else
	{
	    genString string = Logger::logger_instance->text_ed->text();
            string += Logger::logger_instance->last_record.text.str();
	    if(string.length())
		OSapi_fwrite(string.str(), 1, string.length(), file);
	    OSapi_fclose(file);
	}
    }
#endif
}

static void set_message_options(systemMessageOptions* options)
{
    msg_options(*options, false);
}

void Logger::config_CB(gtPushButton*, gtEventPtr, void*, gtReason)
{
    Initialize(Logger::config_CB);
#ifndef NEW_UI
    popup_SystemMessages(&msg_options(), set_message_options);
#endif
}

void Logger::help_logger_CB(gtPushButton* button, gtEventPtr, void* , gtReason)
{
    Initialize(Logger::help_CB);
    if (!Logger::logger_instance) return;
#ifndef NEW_UI
    if (help_disabled)
	msg("WARN: Help is currently disabled") << eom;
    else
	HelpUI_LongHelp(button);
#endif
}

void Logger::scroll (int index)
{
#ifndef NEW_UI
    if (index >= 0 && index < records.size()) {
	int start = (int)(*records[index])->start;
	int end = (int)(*records[index])->end;

        const char* help_name =  (*records[index])->help;
        systemMessageType type = (*records[index])->type;

 	text_ed->set_selection (start, end);
	text_ed->show_position(start);

	if (help_name  && *help_name && !help_disabled) {
	    help_button->help_context_name(help_name);
	    help_button->set_sensitive (help_disabled ? 0 : 1);
	} else
	    help_button->set_sensitive (0);
	error_label->text (get_label_text(type));
    } else {
	error_label->text ("Message type:");
	help_button->set_sensitive (0);
    }
#endif
}

void Logger::scroll0()
{
#ifndef NEW_UI
    const char* help_name =  last_record.help;
    systemMessageType type = last_record.type;

    text_ed0->set_selection (0,last_record.text.length()-1);
    text_ed0->show_position(0);

    if (help_name  && *help_name && !help_disabled) {
        help_button->help_context_name(help_name);
        help_button->set_sensitive (help_disabled ? 0 : 1);
    }
    else
        help_button->set_sensitive (0);
    error_label->text (get_label_text(type));
    error_marker = records.size();
#endif
}

void Logger::top_CB(gtPushButton*, gtEventPtr, void* , gtReason)
{
    Initialize(Logger::top_CB);
    if (!Logger::logger_instance) return;
   #ifndef NEW_UI 
    if (records.size())
	Logger::logger_instance->error_marker = 0;
    else
	Logger::logger_instance->error_marker = -1;
    
    Logger::logger_instance->scroll (Logger::logger_instance->error_marker);
#endif
}

void Logger::bottom_CB(gtPushButton*, gtEventPtr, void* , gtReason)
{
    Initialize(Logger::bottom_CB);
    if (!Logger::logger_instance) return;
   
#ifndef NEW_UI 
    Logger::logger_instance->error_marker = records.size() -1;

    Logger::logger_instance->scroll (Logger::logger_instance->error_marker);
#endif
}

void Logger::next_CB(gtPushButton*, gtEventPtr, void* , gtReason)
{
    Initialize(Logger::next_CB);
    if (!Logger::logger_instance) return;
   
#ifndef NEW_UI 
    Logger::logger_instance->error_marker++;
    if (Logger::logger_instance->error_marker >= records.size()) {
       Logger::logger_instance->error_marker = records.size();
       Logger::logger_instance->scroll0();
    }
    else Logger::logger_instance->scroll (Logger::logger_instance->error_marker);
#endif
}

void Logger::prev_CB(gtPushButton*, gtEventPtr, void* , gtReason)
{
    Initialize(Logger::prev_CB);
    if (!Logger::logger_instance) return;
#ifndef NEW_UI
    if (Logger::logger_instance->error_marker > 0) Logger::logger_instance->error_marker--;
    
    Logger::logger_instance->scroll (Logger::logger_instance->error_marker);
#endif
}


void Logger::curr_CB(gtPushButton*, gtEventPtr, void* , gtReason)
{
    Initialize(Logger::curr_CB);
    if (!Logger::logger_instance) return;
#ifndef NEW_UI
    Logger::logger_instance->scroll0();
#endif
}

