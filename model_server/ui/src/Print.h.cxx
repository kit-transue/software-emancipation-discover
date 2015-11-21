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
//
//
// This file implements complete Print Interface
// 
#include <cLibraryFunctions.h>
#include <msg.h>
#include <messages.h>
#include <machdep.h>
#include <unistd.h>
#include <gtLabel.h>
#include <gtDlgTemplate.h>
#include <gtRadioBox.h>
#include <gtTogB.h>
#include <gtStringEd.h>
#include <gtHorzBox.h>
#include <gtPushButton.h>
#include <gtSepar.h>
#include <gtList.h>
#include <gtOptionMenu.h>
#include <Print.h>
#include <proj.h>
#include <path.h>
#include <genError.h>
#include <viewerShell.h>
#include <objOper.h>
#include <vpopen.h>
#include <viewGraHeader.h>
#include <top_widgets.h>
#include <ste_interface.h>
#ifndef ISO_CPP_HEADERS
#include <errno.h>
#else /* ISO_CPP_HEADERS */
#include <cerrno>
using namespace std;
#endif /* ISO_CPP_HEADERS */
#include <evalMetaChars.h>
#ifndef _WIN32
#include <poll.h>
#endif
#include <iostream.h>
#ifndef _WIN32
#include <fstream.h>
#include <sys/types.h>
#include <sys/wait.h>
#endif

#include <machdep.h>
#include <cmd.h>

#include <projectBrowser.h>
#ifndef _genTmpfile_h
#include <genTmpfile.h>
#endif
#if !defined(_WIN32) && !defined(hp10)
extern "C" {
    int poll(struct pollfd *, unsigned long, int);
};
#endif

printDialog *printDialog::cur_dialog = NULL;

// Function that viewerShell calls to startup print dialog
extern "C" void popup_Print(void *cv)
{
    Initialize (popup_Print);

#ifndef NEW_UI
    viewPtr current_view = (viewPtr)cv;

    printDialog *dlg = new printDialog (NULL, current_view);

    gtDialogTemplate *ui = dlg->ui();

    ui->popup(0);
    if (cv)
        dlg->pages();
#endif
}

// returns scale as set by user.
float printDialog::scale()
{
    Initialize (printDialog::scale);
#ifndef NEW_UI
    char *string = s_text->text();

    float const tolerance = 0.0001;

    float value = OSapi_atof(string);

    if (value <= tolerance) {
	msg("Scale will be set to $1.", warning_sev) <<  tolerance << eom;
	value = tolerance;
    }

    return value;
#else
    return 0;
#endif
}

// returns if we are printing in landscape mode.
int   printDialog::landscape()
{
    Initialize (printDialog::landscape);
#ifndef NEW_UI
    return ls_toggle->set();
#else
    return 0;
#endif
}

// returns specified printer if printing, NULL otherwise
char *printDialog::printer ()
{
    Initialize (printDialog::printer);
#ifndef NEW_UI
    if (printer_toggle->set()) {
	char *rval = printername->text();
	return rval;
    }
#endif
    return NULL;
}

// returns instance of print dialog.	    
printDialog *printDialog::instance() {
    Initialize (printDialog::instance);

    return cur_dialog;
}

// returns file if sending postscript to a file, NULL otherwise
char *printDialog::file()
{
    Initialize (printDialog::file);
#ifndef NEW_UI
    if (file_toggle->set()) {

	char *rval = filename->text();
	if (!rval || *rval == '\0')
	    rval = NULL;

	return rval;
    }
#endif
    return NULL;
}

// constructor for print dialog.
// must be called once during the session.
// dialog is cached for later invocation.
printDialog::printDialog(gtBase *parent, viewPtr cv)
{
    Initialize (printDialog::printDialog);

    printDialog *old_dialog = cur_dialog;

    cur_dialog = this;
    dlg        = NULL;

    build_interface(parent, cv);

    dots_per_unit = 72;
    if (old_dialog){
        if (help_mode != -1 && old_dialog->help_mode != -1)
            copy_old_values(old_dialog);
	delete old_dialog;
    }
    if (help_mode != -1)
        compute_paper_size();
}

void printDialog::copy_old_values(printDialog *old_dialog){
    Initialize(printDialog::copy_old_values);
   
#ifndef NEW_UI 
    if(!old_dialog) return;
    
    paper_width_in_pixels  = old_dialog->paper_width_in_pixels;
    paper_height_in_pixels = old_dialog->paper_height_in_pixels;
    dots_per_unit          = old_dialog->dots_per_unit;
    filename->text(old_dialog->filename->text());
    printername->text(old_dialog->printername->text());
    file_toggle->set(old_dialog->file_toggle->set(), 1);
    printer_toggle->set(old_dialog->printer_toggle->set(), 1);
    ls_toggle->set(old_dialog->ls_toggle->set(), 1);
    s_text->text(old_dialog->s_text->text());
    rows->text(old_dialog->rows->text());
    columns->text(old_dialog->columns->text());
    paper_width->text(old_dialog->paper_width->text());
    paper_height->text(old_dialog->paper_height->text());
    units_menu->menu_history(old_dialog->units_menu->menu_history());
    std_paper_size_menu->menu_history(old_dialog->std_paper_size_menu->menu_history());
#endif
}

static int compare_chapters(const void *p1, const void *p2)
{ 
    Initialize(compare_chapters);

    projNode * s1 = *(projNode**)p1;
    projNode * s2 = *(projNode**)p2;
    
    return strcmp(s1->get_ln(), s2->get_ln());
}

//boris: conversion from internal directory names to nice chapter titles
class item_for_cnvr {
  public:
    char *title;
    char *proj_name;
};

static objSet nice_names_by_title;
static objSet nice_names_by_pn;
static int first_flag = 0;

static int init_nice_names ()
{
    Initialize(init_nice_names);
    return 0;
}

static const char *get_proj_name_by_title (const char *title)
{
    Initialize(get_proj_name_by_title);
    
    if (!title || !title[0])
	return NULL;
    
    init_nice_names ();
    namedString *it = checked_cast(namedString,obj_search_by_name (title, nice_names_by_title));
    if (!it)
	return NULL;
    
    return it->get_value();
}

static const char *get_title_by_pn (const char *pn)
{
    Initialize(get_title_by_pn);
    
    if (!pn || !pn[0])
	return NULL;
    
    init_nice_names ();
    namedString *it = checked_cast(namedString,obj_search_by_name (pn, nice_names_by_pn));
    if (!it)
	return NULL;
    
    return it->get_value();
}


// build interface for the dialog
void printDialog::build_interface(gtBase *parent, viewPtr cv)
{
    Initialize (printDialog::build_interface);

#ifndef NEW_UI
    if (dlg)
        return;

        viewerShell *vs = viewerShell::viewerShell_of_view(cv);
            appPtr app = cv->get_appHeader();
            if(app->get_filename())
               dlg        = gtDialogTemplate::create (parent, "printDialog", app->get_filename());
            else 
               dlg        = gtDialogTemplate::create (parent, "printDialog", TXT("Print View"));
            help_mode  = 0;

    dlg->add_default_buttons
            (TXT("OK"), printDialog::ok_CB, this,
             TXT("Apply"), printDialog::apply_CB, this,
             TXT("Cancel"), printDialog::cancel_CB, this,
             TXT("Help"), NULL, NULL);
    dlg->default_button(NULL);

    file_toggle =
        gtToggleButton::create (dlg, "file_toggle", TXT("Save to File"),
                                printDialog::file_CB, this);
    file_toggle->attach_side_left();
    file_toggle->attach_side_top();
    file_toggle->manage();

    filename = gtStringEditor::create(dlg, "filename", NULL);
    filename->attach_side_left();
    filename->attach_top(file_toggle);
    filename->attach_side_right();
    filename->set_sensitive(0);
    filename->manage();
    file_toggle->set (0, 0);

    printer_toggle =
        gtToggleButton::create (dlg, "printer_toggle", TXT("Send to Printer"),
                                printDialog::printer_CB, this);
    printer_toggle->attach_side_left();
    printer_toggle->attach_top (filename);
    printer_toggle->manage();

    printername = gtStringEditor::create(dlg, "printername", NULL);
    printername->attach_top(printer_toggle);
    printername->attach_side_left();
    printername->attach_side_right();
    printername->manage();
    printer_toggle->set (1, 0);

    if (help_mode != -1)
    {
        gtSeparator *sep = gtSeparator::create (dlg, "sep1", gtHORZ);
        sep->attach_top (printername);
        sep->attach_offset_top(10);
        sep->attach_side_left();
        sep->attach_offset_left(0);
        sep->attach_side_right();
        sep->attach_offset_right(0);
        sep->manage();

	gtHorzBox *box1 = gtHorzBox::create (dlg, "box1");
	box1->attach_top (sep);
	box1->attach_offset_top(10);
	box1->attach_side_left ();
	box1->manage();

	gtPushButton *compute_pages =
	    gtPushButton::create(box1, "compute_pages", TXT("Compute Pages"),
                                 printDialog::compute_pages_CB, this);
	compute_pages->manage();

	gtPushButton *fit_to_page =
	    gtPushButton::create (box1, "fit_to_page", TXT("Fit to page"),
				    printDialog::fit_to_page_CB, this);
	fit_to_page->manage();

	ls_toggle =
	    gtToggleButton::create (box1, "ls_toggle", TXT("Landscape"),
				    printDialog::ls_toggle_CB, this);
	ls_toggle->manage();

	gtHorzBox *box2 = gtHorzBox::create (dlg, "box2");

	s_label = gtLabel::create(box2, "s_label", TXT("Scale:"));
	s_label->manage();

	s_text = gtStringEditor::create(box2, "s_text", NULL);
	s_text->columns(6);
	s_text->text("1.0");
	s_text->activate_callback (printDialog::scale_CB, this);
	s_text->manage();

	gtLabel *rows_label = gtLabel::create(box2, "rows_label", TXT("Rows:"));
	rows_label->set_sensitive(0);
	rows_label->manage();

	rows = gtStringEditor::create (box2, "rows", NULL);
	rows->columns(3);
	rows->set_sensitive(0);
	rows->manage();

	gtLabel *columns_label = gtLabel::create(box2, "columns_label", TXT("Columns"));
	columns_label->set_sensitive(0);
	columns_label->manage();

	columns = gtStringEditor::create (box2, "columns", NULL);
	columns->columns(3);
	columns->set_sensitive(0);
	columns->manage();

	box2->attach_top(box1);
	box2->attach_side_left();
	box2->manage();

	sep = gtSeparator::create (dlg, "sep2", gtHORZ);
	sep->attach_top (box2);
	sep->attach_offset_top(10);
	sep->attach_side_left();
	sep->attach_offset_left(0);
	sep->attach_side_right();
	sep->attach_offset_right(0);
	sep->manage();

        gtLabel *paper_label = gtLabel::create(dlg, "paper_label", TXT("Paper Attributes"));
        paper_label->attach_top (sep);
        paper_label->attach_offset_top(10);
        paper_label->attach_side_left();

        gtHorzBox *box3 = gtHorzBox::create (dlg, "box3");

        std_paper_size_menu =
             gtOptionMenu::create
             (box3, "std_paper_size_menu", "",

             gtMenuStandard, "user_defined", TXT("Custom"),
             this, printDialog::user_defined_CB,

             gtMenuStandard, "legal", TXT("Legal"),
             this, printDialog::legal_CB,

             gtMenuStandard, "letter", TXT("Letter"),
             this, printDialog::letter_CB,

             gtMenuStandard, "ledger", TXT("Ledger"),
             this, printDialog::ledger_CB,

	     gtMenuStandard, "A4", TXT("A4"),
             this, printDialog::A4_CB, 

             NULL);
        std_paper_size_menu->manage();
        std_paper_size_menu->menu_history (2);

        paper_width = gtStringEditor::create(box3, "paper_width", NULL);
        paper_width->columns(6);
        paper_width->set_callback (paper_size_CB, this);
        paper_width->manage();

        gtLabel *x_label = gtLabel::create (box3, "x_label", "X");
        x_label->manage();

        paper_height = gtStringEditor::create(box3, "paper_height", NULL);
        paper_height->columns(6);
        paper_height->manage();
        paper_height->set_callback (paper_size_CB, this);

        units_menu =
            gtOptionMenu::create
            (box3, "units_menu", "",

             gtMenuStandard, "Centimeters", TXT("Centimeters"),
             this, printDialog::centimeters_CB,

             gtMenuStandard, "Inches", TXT("Inches"),
             this, inches_CB,

             gtMenuStandard, "ps_pixels", TXT("Pixels"),
             this, ps_pixels_CB,

             NULL);
        units_menu->manage();

        box3->attach_top (paper_label);
        box3->attach_side_left();
        box3->manage();

        letter_CB (NULL, NULL, this, gtActivate);
    }
#endif
}

// destructor for the dialog.
printDialog::~printDialog()
{
    Initialize (printDialog::~printDialog);

#ifndef NEW_UI
    dlg->popdown();
    delete dlg;
#endif
    dlg = NULL;
}

// call back when scale is changed.
void printDialog::scale_CB (gtStringEditor*, gtEventPtr, void* cd, gtReason)
{
    Initialize (printDialog::scale_CB);
#ifndef NEW_UI
    printDialog *pd = (printDialog *)cd;

    pd->pages();
#endif
}

// retunrs page size in postscript pixel coordinates.
void printDialog::page_size (int &w, int &h, int &m)
{
    Initialize (printDialog::page_size);

    w = paper_width_in_pixels;
    h = paper_height_in_pixels;
    m = 18;
}

// comput page callback
void printDialog::compute_pages_CB (gtPushButton*, gtEventPtr, void* cd, gtReason)
{
    Initialize (printDialog::compute_pages_CB);
#ifndef NEW_UI
    printDialog *pd = (printDialog *)cd;

    pd->compute_paper_size();
    pd->pages();
#endif
}

// landscape toggle callback
void printDialog::ls_toggle_CB(gtToggleButton*, gtEventPtr, void* cd, gtReason)
{
    Initialize (printDialog::ls_toggle_CB);
#ifndef NEW_UI
    printDialog *pd = (printDialog *)cd;

    pd->pages();
#endif
}

// fit to page callback
void printDialog::fit_to_page_CB (gtPushButton*, gtEventPtr, void* cd, gtReason)
{
    Initialize (printDialog::fit_to_page_CB);
#ifndef NEW_UI
    printDialog *pd = (printDialog *)cd;

    // get the scale to fit the page
    float scale = pd->scale_for_fit_to_page ();

    // set scale to be the correct value
    char buf[64];
    OSapi_sprintf (buf, "%.4f", scale);
    pd->s_text->text(buf);
    pd->pages();
#endif
}

// file toggle callback
void printDialog::file_CB (gtToggleButton* t, gtEventPtr, void* cd, gtReason)
{
    Initialize (printDialog::file_CB);
#ifndef NEW_UI
    printDialog *pd = (printDialog *)cd;

    if (t->set()) {
	pd->filename->set_sensitive (1);
    } else {
	pd->filename->set_sensitive (0);
    }
#endif
}

// printer toggle callback
void printDialog::printer_CB (gtToggleButton* t, gtEventPtr, void* cd, gtReason)
{
    Initialize (printDialog::printer_CB);
#ifndef NEW_UI
    printDialog *pd = (printDialog *)cd;

    if (t->set()) {
	pd->printername->set_sensitive(1);
    } else {
	pd->printername->set_sensitive(0);
    }
#endif
}

// ok callback
void printDialog::ok_CB(gtPushButton* pb, gtEvent* e, void* cd, gtReason r)
{
    Initialize (printDialog::ok_CB);
#ifndef NEW_UI
    apply_CB (pb, e, cd, r);

    cancel_CB (pb, e, cd, r);
#endif
}

// apply callback
void printDialog::apply_CB(gtPushButton*, gtEvent*, void* cd, gtReason)
{
    Initialize (printDialog::apply_CB);
#ifndef NEW_UI
    printDialog *pd = (printDialog *)cd;

    if (pd->help_mode == 0) {
	viewerShell *vs = viewerShell::get_current_vs();
	viewPtr v = vs->get_current_view();
	if (v)
	    pd->postscript_print (v);
    }
#endif
}

// cancel callback
void printDialog::cancel_CB(gtPushButton*, gtEvent*, void* cd, gtReason)
{
    Initialize (printDialog::cancel_CB);
#ifndef NEW_UI
    printDialog *pd = (printDialog *)cd;

    if (pd)
	pd->dlg->popdown();
#endif
}

// user defined papersize callback
void printDialog::user_defined_CB (gtPushButton*, gtEventPtr, void* cd, gtReason)
{
    Initialize (printDialog::user_defined_CB);
#ifndef NEW_UI
    printDialog *pd = (printDialog *)cd;

    pd->paper_width->set_sensitive(1);
    pd->paper_height->set_sensitive (1);
    pd->pages();
#endif
}

// legal size paper callback
void printDialog::legal_CB (gtPushButton*, gtEventPtr, void* cd, gtReason)
{
    Initialize (printDialog::legal_CB);
#ifndef NEW_UI
    printDialog *pd = (printDialog *)cd;

    pd->paper_width->set_sensitive(0);
    pd->paper_height->set_sensitive(0);
    pd->paper_width->text("8.5");
    pd->paper_height->text("14.0");
    pd->units_menu->menu_history(1);
    pd->dots_per_unit = 72;
    pd->compute_paper_size();
    pd->pages();
#endif
}

// ledger size paper callback
void printDialog::ledger_CB (gtPushButton*, gtEventPtr, void* cd, gtReason)
{
    Initialize (printDialog::ledger_CB);
#ifndef NEW_UI
    printDialog *pd = (printDialog *)cd;

    pd->paper_width->set_sensitive(0);
    pd->paper_height->set_sensitive(0);
    pd->paper_width->text("11.0");
    pd->paper_height->text("17.0");
    pd->units_menu->menu_history(1);
    pd->dots_per_unit = 72;
    pd->compute_paper_size();
    pd->pages();
#endif
}

// letter size paper callback
void printDialog::letter_CB (gtPushButton*, gtEventPtr, void* cd, gtReason)
{
    Initialize (printDialog::letter_CB);
#ifndef NEW_UI
    printDialog *pd = (printDialog *)cd;

    pd->paper_width->set_sensitive(0);
    pd->paper_height->set_sensitive(0);
    pd->paper_width->text("8.5");
    pd->paper_height->text("11.0");
    pd->units_menu->menu_history(1);
    pd->dots_per_unit = 72;
    pd->compute_paper_size();
    if (pd->help_mode != -1)
        pd->pages();
#endif
}

// A4 size paper callback
void printDialog::A4_CB (gtPushButton*, gtEventPtr, void* cd, gtReason)
{
    Initialize (printDialog::A4_CB);
#ifndef NEW_UI
    printDialog *pd = (printDialog *)cd;

    pd->paper_width->set_sensitive(0);
    pd->paper_height->set_sensitive(0);
    pd->paper_width->text("8.27");
    pd->paper_height->text("11.69");
    pd->units_menu->menu_history(1);
    pd->dots_per_unit = 72;
    pd->compute_paper_size();
    if (pd->help_mode != -1)
        pd->pages();
#endif
}


// inches unit callback
void printDialog::inches_CB (gtPushButton* , gtEventPtr e, void* cd, gtReason r)
{
    Initialize (printDialog::inches_CB);
#ifndef NEW_UI
    printDialog *pd = (printDialog *) cd;

    pd->std_paper_size_menu->menu_history(0);
    user_defined_CB (NULL, e, cd, r);

    pd->dots_per_unit = 72;
    pd->compute_paper_size();
    pd->pages();
#endif
}

// centimeters unit callback
void printDialog::centimeters_CB (gtPushButton*, gtEventPtr e, void* cd, gtReason r)
{
    Initialize (printDialog::centimeters_CB);
#ifndef NEW_UI 
    printDialog *pd = (printDialog *) cd;
 
    pd->std_paper_size_menu->menu_history(0);
    user_defined_CB (NULL, e, cd, r);
 
    pd->dots_per_unit = (int) (72 / 2.5472);
    pd->compute_paper_size();
    pd->pages();
#endif
}

// pixel units callback
void printDialog::ps_pixels_CB(gtPushButton*, gtEventPtr e, void* cd, gtReason r)
{
    Initialize (printDialog::ps_pixels_CB);
#ifndef NEW_UI 
    printDialog *pd = (printDialog *) cd;
 
    pd->std_paper_size_menu->menu_history(0);
    user_defined_CB (NULL, e, cd, r);
 
    pd->dots_per_unit = 1;

    pd->compute_paper_size();
    pd->pages();
#endif
}

void printDialog::paper_size_CB (gtStringEditor *, gtEventPtr, void* cd, gtReason)
{
    Initialize (printDialog::paper_size_CB);
#ifndef NEW_UI
    printDialog *pd = (printDialog *) cd;

    pd->compute_paper_size();
    pd->pages();
#endif
}

// computes paper size based on units and new paper width and height
void printDialog::compute_paper_size()
{
    Initialize (printDialog::compute_paper_size);

    paper_width_in_pixels = (int) (dots_per_unit * OSapi_atof(paper_width->text()));
    paper_height_in_pixels = (int) (dots_per_unit * OSapi_atof(paper_height->text()));
}

// updates pages on the screen
void  printDialog::pages ()
{
    Initialize (printDialog::pages);
#ifndef NEW_UI
    int num_rows = 0;
    int num_columns = 0;

    viewerShell *vs = viewerShell::get_current_vs();
 
    viewPtr v = vs->get_current_view();
 
    if (v) {
        viewTreePtr vt = checked_cast (viewTree, v->get_root());
	v->compute_pages_to_print (vt, num_rows, num_columns);
	char buf[64];
	OSapi_sprintf (buf, "%d", num_rows);
	rows->text(buf);
	OSapi_sprintf (buf, "%d", num_columns);
	columns->text(buf);
    }
#endif
}

// returns scale for fot to page
float printDialog::scale_for_fit_to_page ()
{
    Initialize (printDialog::scale_for_fit_to_page);
#ifndef NEW_UI
    viewerShell *vs = viewerShell::get_current_vs();
 
    viewPtr v = vs->get_current_view();
 
    // if graphics view get the scale
    if (v && is_viewGraHeader(v)) {
	viewTreePtr vt = checked_cast (viewTree, v->get_root());
	return viewGraHeaderPtr(v)->scale_for_fit_to_page(vt);
    }
#endif
    return 1.0;
}

// prints obj in postscript format
int printDialog::postscript_print (ostream &f, commonTreePtr obj)
{
    Initialize (printDialog::postscript_print);
#ifndef NEW_UI
    viewPtr v = NULL;
    viewTreePtr vt = NULL;
 
    if (is_viewTree (obj)) {
 
        vt = checked_cast(viewTree,obj);
        v = vt->get_header();
 
    } else if (is_view (obj)) {
 
        v = checked_cast(view,obj);
        vt = checked_cast(viewTree, v->get_root());
    }

    if (v){
	v->send_postscript (f, vt);
	return 1;
    }else{
	return 0;
    }
#else
    return 0;
#endif
}


FILE    *printer_error_stream;
SIG_TYP old_child_signal;
SIG_TYP old_pipe_signal;
int     printer_child_died;
int     printer_child_status;
pid_t	printer_child_pid;

SIG_TYP printer_child_signal(int, int, struct sigcontext *, char *){
    if (printer_child_pid != (pid_t)0) {
	int pid;
#ifndef WNOHANG /* REAL PORTABILITY ??? */
#define WNOHANG 1
#endif
#ifndef _WIN32
	int options = WNOHANG;
	int status;

	pid                  = (int)OSapi_waitpid(printer_child_pid, &status, options);
	if (!printer_child_died && pid == printer_child_pid) {
	    printer_child_status = OSapi_WEXITSTATUS(status);
	    printer_child_died   = 1;
	}
    }
#else
printf("Not implemented on WinNT");
#endif /*_WIN32*/
    return 0;
}

SIG_TYP printer_pipe_signal(int, int, struct sigcontext *, char *){
#ifndef _WIN32
    OSapi_signal(SIGPIPE, (SIG_TYP)printer_pipe_signal);
#else
printf("Not implementeed on WinNT");
#endif/*_WIN32*/
    return 0;
}

static FILE* execute_printer_command(char **argv, FILE **stream)
{
#ifndef _WIN32
    Initialize(execute_printer_command);

    FILE **read_pipe_ptr = &printer_error_stream;
    FILE **write_pipe_ptr = stream;
		    
    printer_child_died = 0;
    printer_child_pid = (pid_t)0;
    old_child_signal   = (SIG_TYP)OSapi_signal(SIGCHLD, (SIG_TYP)printer_child_signal);
    old_pipe_signal    = (SIG_TYP)OSapi_signal(SIGPIPE, (SIG_TYP)printer_pipe_signal);

    int fdr[2];		// read pipe from child's stdout to parent
    int fdw[2];		// write pipe from parent to child's stdin
    fdr[0]=fdr[1]=fdw[0]=fdw[1]=-1;
    *read_pipe_ptr = *write_pipe_ptr = NULL;

    if(OSapi_pipe(fdr) || OSapi_pipe(fdw))
    {
	perror("execute_printer_command: cannot create pipes");
	if (fdr[0]>=0)  OSapi_close(fdr[0]);
	if (fdr[0]>=0)  OSapi_close(fdr[1]);
	if (fdw[0]>=0)  OSapi_close(fdw[0]);
	if (fdw[0]>=0)  OSapi_close(fdw[1]);
	return NULL;
    }

    int pid = (int)vfork();

    if(pid == 0)		// CHILD
    {
	// fdr is for reading stdout from child
	OSapi_close(fdr[0]);
	if(OSapi_dup2(fdr[1], 1) < 0 || OSapi_dup2(fdr[1],2) < 0)
	{
	    perror("execute_printer_command: cannot setup stdout for child");
	    OSapi__exit(-1);
	}

	// fdw is for writing stdin to child
	OSapi_close(fdw[1]);
	if(OSapi_dup2(fdw[0], 0) < 0)
	{
	    perror("execute_printer_command: cannot setup stdin for child");
	    OSapi__exit(-1);
	}

	// Note:  getdtablesize() defines number of handles possible,
	//    numbered from 0 to max-1
	int max=OS_dependent::getdtablesize();

	// close all the handles not needed by the child
	for(int i = 3; i < max; i++)
	    OSapi_close(i);

	OSapi_execvp("lp", argv);

	perror("execute_printer_command: exec failed\n");
	OSapi__exit(1);
    }
    else			// PARENT
    {
	if(pid < 0)
	{
	    perror("execute_printer_command: cannot vfork");
	    if (fdr[0]>=0)  OSapi_close(fdr[0]);
	    if (fdr[0]>=0)  OSapi_close(fdr[1]);
	    if (fdw[0]>=0)  OSapi_close(fdw[0]);
	    if (fdw[0]>=0)  OSapi_close(fdw[1]);
	}
	else
	{
	    printer_child_pid = pid;

	    // Check for child died between fork and setting printer_child_pid.
	    printer_child_signal(0, 0, NULL, 0);
	    OSapi_close(fdr[1]);

	    //boris&AlexZ: fixing crash in closed_bugs/bug8619/test.ccc
	    //             The crash happens only in optimized version
	    //             We think, that this is compiler bug. The value
            //             of read_pipe_ptr gets overriten. To pevent the
	    //             crash we restore the value here.
	    read_pipe_ptr = &printer_error_stream;

	    *read_pipe_ptr = OSapi_fdopen(fdr[0], "r");
	    OSapi_close(fdw[0]);
	    *write_pipe_ptr = OSapi_fdopen(fdw[1], "w");
	}
	return *read_pipe_ptr;
    }
#else
printf("Not implemented on WinNT");
#endif /*_WIN32*/
    return NULL;
}

static void close_printer(FILE *stream){
#ifndef _WIN32
    Initialize(close_printer);
    
    OSapi_fflush(stream);
    OSapi_fclose(stream);
    while(!printer_child_died) {
	OSapi_sleep(1);

	// Check for child died without signalling (dopey bug 19668).
	printer_child_signal(0, 0, NULL, 0);
    }
    struct OStype_stat buf;
    OSapi_fstat(OSapi_fileno(printer_error_stream), &buf);
    if(buf.st_size != 0 && printer_child_status != 0){
	char *message = new char[buf.st_size + 1];
	OSapi_fgets(message, (int)buf.st_size, printer_error_stream);
	msg("error printing: OS reports message '$1'; status: $2", error_sev) << message << eoarg << printer_child_status << eom;
	delete message;
    }
    OSapi_fclose(printer_error_stream);
    OSapi_signal(SIGCHLD, old_child_signal);
    OSapi_signal(SIGPIPE, old_pipe_signal);
#else
printf("Not implemented on WinNT");
#endif/*_WIN32*/
}

// prints a view in postscript
int printDialog::postscript_print (view *v)
{
#ifndef _WIN32
    Initialize (printDialog::postscript_print);

    if (!v) return 0;

    push_busy_cursor();
    char *device = printer();
    
    if (device) {
	char* argv[3];
        argv[0] = "lp";
	argv[1] = argv[2] = 0;    
	if (*device) {
	     argv[1] = new char[strlen(device)+3];
	     strcpy (argv[1],"-d");
	     strcat(argv[1],device); 
	 }

	FILE *stream;
	execute_printer_command(argv, &stream);

	if (argv[1])
	    delete argv[1];

	if (!stream) {
	    msg("ERROR: Pipe to lp could not be opened.") << eom;
	    OSapi_signal(SIGCHLD, old_child_signal);
	    OSapi_signal(SIGPIPE, old_pipe_signal);
	    pop_cursor();
	    return 0;
	}

	int fd = OSapi_fileno (stream);
	struct pollfd fds;
	fds.fd = fd;
	fds.events = POLLOUT;
	fds.revents = 0;
	while (1) {
	    int count = poll(&fds, 1, 10000); /* wait for 10 seconds */
	    if (count == -1 ) {
		if (errno == EAGAIN)
		    continue;	 /* should restart */
	     } else if (count == 1 && !printer_child_died && fds.revents &POLLOUT) {
		/* printer ready */
		ostream *f = new ofstream(fd);   
		postscript_print (*f, checked_cast(commonTree, v));
		delete f;
	     }
	     break;
	}

	close_printer(stream); // This call will also verify for any printer errors
    }

    // print to a file
    
    if (device = file()) {
	
	// evaluate metacharacters, if any
	genString translated_file_name;
	// Return error if the string could not be translated.
	if (!eval_shell_metachars((const char*)device, translated_file_name)) {
	    msg("ERROR: Cannot translate file '$1'.") << device << eom;
	    pop_cursor();
	    return 0;
	}
	ofstream f((char *)translated_file_name, ios::out);
	int errno_save = errno;
 
	if (f.fail()) {
	    // return exact reason for failure
#ifdef sun5
	    char* err_msg = OS_dependent::m_strerror(errno_save);
#else
	    char* err_msg = OS_dependent::strerror(errno_save);
#endif
	    msg("ERROR: '$1': $2") << (char*)translated_file_name << eoarg << err_msg << eom;
	    pop_cursor();
	    return 0;
	}

	postscript_print (f, checked_cast(commonTree, v));

	f.close();
    }
    pop_cursor();
#else
printf("Not implemented on WinNT");
#endif /*_WIN32*/
    return 1;
}

extern void ste_interface_create_postscript_file( appPtr, FILE *fl );

// prints an app in postscript
int printDialog::postscript_print (appPtr app, int append_flag)
{
#ifndef _WIN32
    Initialize (printDialog::postscript_print);

    if (!app) return 0;

    push_busy_cursor();
    char *device = printer();

    if (device) {
	char* argv[3];
        argv[0] = "lp";
	argv[1] = argv[2] = 0;    
	if (*device) {
	     argv[1] = new char[strlen(device)+3];
	     strcpy (argv[1],"-d");
	     strcat(argv[1],device); 
	 }

	FILE *stream;
	execute_printer_command(argv, &stream);

	if (argv[1])
	    delete argv[1];

	if (!stream) {
	    msg("ERROR: Pipe to lp could not be opened.") << eom;
	    OSapi_signal(SIGCHLD, old_child_signal);
	    OSapi_signal(SIGPIPE, old_pipe_signal);
	    pop_cursor();
	    return 0;
	}

	int fd = OSapi_fileno (stream);
	struct pollfd fds;
	fds.fd = fd;
	fds.events = POLLOUT;
	fds.revents = 0;
	while (1) {
	    int count = poll(&fds, 1, 10000); /* wait for 10 seconds */
	    if (count == -1 ) {
		if (errno == EAGAIN)
		    continue;	 /* should restart */
	     } else if (count == 1 && !printer_child_died && fds.revents &POLLOUT) {
		/* printer ready */
		ste_interface_create_postscript_file ( app, stream );
	     }
	     break;
	}

	close_printer(stream); // This call will also verify for any printer errors
    }

    // print to a file
    
    if (device = file()) {
	// evaluate metacharacters, if any
	genString translated_file_name;
	// Return error if the string could not be translated.
	if (!eval_shell_metachars((const char*)device, translated_file_name)) {
	    msg("ERROR: Cannot translate file '$1'.") << device << eom;
	    pop_cursor();
	    return 0;
	}

	FILE *fl;
	
	if(append_flag)
	    fl = OSapi_fopen((char *)translated_file_name, "a");
	else
	    fl = OSapi_fopen((char *)translated_file_name, "w");
	if (!fl) {
	    msg("ERROR: File '$1' cannot be opened.") << device << eom;
	    pop_cursor();
	    return 0;
	}
	ste_interface_create_postscript_file ( app, fl );
	OSapi_fclose(fl);
    }
    pop_cursor();
#else
printf("Not implemented on WinNT");
#endif /*_WIN32*/
    return 1;
}

