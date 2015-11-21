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
// SystemMessages.h.C
//------------------------------------------
// synopsis:
// Dialog box for controlling display of messages.
//------------------------------------------

// INCLUDE FILES

#include "machdep.h"
#include <gtDlgTemplate.h>
#include <gtForm.h>
#include <gtHorzBox.h>
#include <gtPushButton.h>
#include <gtToggleBox.h>

#include <customize.h>

#include <systemMessages.h>
#include <_SystemMessages.h>


// VARIABLE DEFINITIONS

class SystemMessages
{
  public:
    gtDialogTemplate*	shell;

    SystemMessages(void (*)(systemMessageOptions*));

    void update(systemMessageOptions*);

  private:
    static void OK_CB(gtPushButton*, gtEventPtr, void*, gtReason);
    static void Apply_CB(gtPushButton*, gtEventPtr, void*, gtReason);
    static void Cancel_CB(gtPushButton*, gtEventPtr, void*, gtReason);

    void	 	(*callback)(systemMessageOptions*);
    gtToggleBox*	main_options;
    gtToggleBox*	filter_options;
    gtToggleBox*	job_options;

    void do_it();
};


static SystemMessages* sm;


// FUNCTION DEFINITIONS

void popup_SystemMessages(
    systemMessageOptions* options,
    void (*callback)(systemMessageOptions*))
{
#ifndef NEW_UI
    if(sm == NULL)
	sm = new SystemMessages(callback);
    sm->update(options);
    sm->shell->popup(0);
#endif
}


void SystemMessages_update(systemMessageOptions* options)
{
#ifndef NEW_UI
    if(sm)
	sm->update(options);
#endif
}


SystemMessages::SystemMessages(void (*cb)(systemMessageOptions*))
: callback(cb)
{
#ifndef NEW_UI
    shell = gtDialogTemplate::create(
	NULL, "System_Messages", "System Messages");
    shell->add_default_buttons(OK_CB, this, Apply_CB, this, Cancel_CB, this);

    gtHorzBox* hbox = gtHorzBox::create(shell, "hbox");
    gtForm::vertStack(hbox);
    hbox->attach(gtBottom);

    main_options = gtToggleBox::create(
	hbox, "main_options", TXT("Main Options"),
	TXT("Use Message Logger"), "create_log_files", NULL, NULL,
	NULL);
    main_options->manage();

    filter_options = gtToggleBox::create(
	hbox, "filter_options", TXT("Message Filters"),
	TXT("Error"), "show_errors", NULL, NULL,
	TXT("Warning"), "show_warnings", NULL, NULL,
	TXT("Informative"), "show_informative", NULL, NULL,
	TXT("Diagnostic"), "show_diagnostics", NULL, NULL,
	NULL);
    filter_options->manage();

    job_options = gtToggleBox::create(
	hbox, "job_options", TXT("Job Categories"),
	TXT("Importing Files"), "show_import_files", NULL, NULL,
	TXT("Compilation"), "show_compilation", NULL, NULL,
	TXT("Propagation"), "show_propagation", NULL, NULL,
	NULL);
    job_options->manage();

    hbox->manage();
#endif
}


void SystemMessages::update(struct systemMessageOptions *ptr)
{
#ifndef NEW_UI
    if (ptr)
    {
	main_options->item_set(0, 	ptr->logFile, 0);

	filter_options->item_set(0,	ptr->error,0 );
	filter_options->item_set(1,	ptr->warning,0 );
	filter_options->item_set(2,	ptr->informative,0 );
	filter_options->item_set(3,	ptr->diagnostic,0 );

	job_options->item_set(0, 	ptr->import,0 );
	job_options->item_set(1, 	ptr->compile,0 );
	job_options->item_set(2, 	ptr->propagate,0 );
    }
#endif
}

void SystemMessages::OK_CB(gtPushButton *, gtEvent *, void* data, gtReason)
{
#ifndef NEW_UI
    ((SystemMessages*)data)->do_it();
    ((SystemMessages*)data)->shell->popdown();
#endif
}

void SystemMessages::Apply_CB(gtPushButton *, gtEvent *, void* data, gtReason)
{
#ifndef NEW_UI
    ((SystemMessages*)data)->do_it();
#endif
}

void SystemMessages::Cancel_CB(gtPushButton *, gtEvent *, void* data, gtReason)
{
#ifndef NEW_UI
    ((SystemMessages*)data)->shell->popdown();
#endif
}

void SystemMessages::do_it()
{
#ifndef NEW_UI
    struct systemMessageOptions options;

    options.logFile =		main_options->item_set(0);
    options.error =		filter_options->item_set(0);
    options.warning =		filter_options->item_set(1);
    options.informative =	filter_options->item_set(2);
    options.diagnostic =	filter_options->item_set(3);
    options.import =		job_options->item_set(0);
    options.compile =		job_options->item_set(1);
    options.propagate = 	job_options->item_set(2);

    // Update the MsgLogger preference.
    customize::putPref("DIS_msg.UseMsgLogger", options.logFile);
    customize::putPref("DIS_msg.LogErrorMessages", options.error);
    customize::putPref("DIS_msg.LogWarningMessages", options.warning);
    customize::putPref("DIS_msg.LogInformativeMessages", options.informative);
    customize::putPref("DIS_msg.LogDiagnosticMessages", options.diagnostic);
    customize::putPref("DIS_msg.LogDuringImport", options.import);
    customize::putPref("DIS_msg.LogDuringCompile", options.compile);
    customize::putPref("DIS_msg.LogDuringPropagate", options.propagate);

    if (callback)
	(*callback)(&options);
#endif
}

