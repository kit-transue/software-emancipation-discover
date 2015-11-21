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
// NewPrompt.h.C
//------------------------------------------
// synopsis:
// Prompt in gtLib.
//
// description:
// Popup a dialog box with one line of input and OK/CANCEL/HELP buttons.
//------------------------------------------

#include <msg.h>
#include "machdep.h"
#ifndef ISO_CPP_HEADERS
#include <stdlib.h>
#include <string.h>
#else /* ISO_CPP_HEADERS */
#include <cstdlib>
using namespace std;
#include <cstring>
#endif /* ISO_CPP_HEADERS */
#include <messages.h>
#include <gtNoParent.h>
#include <gtDlgTemplate.h>
#include <gtLabel.h>
#include <gtStringEd.h>

#include <NewPrompt.h>
#include <genString.h>

//#define TXT(a) a

Prompt::Prompt(gtBase* parent, const char* title, const char* prompt)
{
    init(parent, title, prompt, NULL, NULL);
}

Prompt::Prompt(gtBase* parent, const char* title, const char* prompt,
	       const char* err_message)
{
    init(parent, title, prompt, err_message, NULL);
}

Prompt::Prompt(gtBase* parent, const char* title, const char* prompt,
	       const char* err_message, const char* OK2_label)
{
    init(parent, title, prompt, err_message, OK2_label);
}


void Prompt::init(gtBase* parent, const char* title, const char* prompt,
                  const char* err_msg, const char* OK2_label)
{
#ifndef NEW_UI
    shell = gtDialogTemplate::create(parent, "prompt", title);
    shell->add_button("ok", TXT("OK"), &Prompt::OK_CB, this);
    if (OK2_label) shell->add_button("ok", OK2_label, &Prompt::OK2_CB, this);
    shell->add_button("cancel", TXT("Cancel"), &Prompt::Cancel_CB, this);
    shell->add_help_button();
 
    label = gtLabel::create(shell, "label", prompt);
    label->alignment(gtCenter);
    label->attach(gtLeft);
    label->attach(gtRight);
    label->attach(gtTop);
    label->manage();
 
    answer_text = gtStringEditor::create(shell, "answer", NULL);
    answer_text->attach(gtTop, label, 10);
    answer_text->attach(gtLeft);
    answer_text->attach(gtRight);
    answer_text->manage();

    err_message = err_msg;
#endif
}
 


Prompt::~Prompt()
{
    if (shell) delete shell;
}

int Prompt::oneshot(genString& answer)
{

    if ((const char*)answer)
    	answer_text->text(answer);

    confirm = 0;

#ifndef NEW_UI
    shell->popup(3);
    shell->take_control(&Prompt::take_control_CB, this);
    shell->popdown();

    if (confirm > 0)
	answer = answer_text->text();
#endif

    return confirm;
}

void Prompt::OK_CB(gtPushButton *, gtEventPtr, void *cd, gtReason)
{
#ifndef NEW_UI
    Prompt_Ptr pp = Prompt_Ptr(cd);
    if (pp->err_message.not_null()) {
      if ( !(pp->answer_text->text()) || !(*(pp->answer_text->text()))) {
	msg("ERROR: $1 ") << (const char*) pp->err_message << eom;
        return;
      }
    }
    pp->confirm = 1;
    pp->shell->popdown();
#endif
}

void Prompt::OK2_CB(gtPushButton *, gtEventPtr, void *cd, gtReason)
{
#ifndef NEW_UI
    Prompt_Ptr pp = Prompt_Ptr(cd);
    pp->confirm = 2;
    pp->shell->popdown();
#endif
}
    
void Prompt::Cancel_CB(gtPushButton *, gtEventPtr, void *cd, gtReason)
{
#ifndef NEW_UI
    Prompt_Ptr pp = Prompt_Ptr(cd);

    pp->confirm = -1;
    pp->shell->popdown();
#endif
}

int Prompt::take_control_CB(void *cd)
{
#ifndef NEW_UI
    return ((Prompt_Ptr)cd)->confirm;
#else
    return 0;
#endif
}

//////////////////////////////////////////////////////////////////////////
// C function to replace old Prompt.if interface
//////////////////////////////////////////////////////////////////////////

#include <gtBaseXm.h>

extern "C" int Prompt_oneshot(
    Widget* wp, const char *title, const char *prompt, char **answer)
{
#ifndef NEW_UI
    gtNoParent* parent = wp ? gtNoParent::create() : NULL;
    if(parent)
	parent->rep()->widget(*wp);

    Prompt pr(parent, title, prompt);

    genString string(answer ? *answer : NULL);
    const int confirm = pr.oneshot(string);

    *answer = NULL;
    if(confirm > 0  &&  (const char*)string)
	*answer = strdup(string);

    if(parent)
    {
	parent->rep()->widget(NULL);
	delete parent;
    }

    return confirm;
#else
    return 0;
#endif
}

/*
   START-LOG-------------------------------------------

   $Log: NewPrompt.h.C  $
   Revision 1.10 2000/07/12 18:15:28EDT ktrans 
   merge from stream_message branch
Revision 1.2.1.6  1993/08/21  13:22:31  kws
Fix compilation error

Revision 1.2.1.5  1993/08/20  23:29:02  andrea
added Button to Prompt box

Revision 1.2.1.4  1993/08/04  15:44:50  swu
part of the fix for bug 4111

Revision 1.2.1.3  1993/07/09  22:40:17  andrea
fixed bug in scripting

Revision 1.2.1.2  1992/10/09  20:13:35  jon
RCS History Marker Fixup



   END-LOG---------------------------------------------

*/

