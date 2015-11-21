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
// Question.h.C
//------------------------------------------
// synopsis:
// Question dialog in libGT
// Uses GT style of User Interface. See Question.h for details.
//------------------------------------------

// INCLUDE FILES

#ifndef NEW_UI

#include <genError.h>

#include <gtDlgTemplate.h>
#include <gtForm.h>
#include <gtLabel.h>
#include <gtDisplay.h>

#include <gtQuestion.h>

#define TXT(a) a

// FUNCTION DEFINITIONS

gtQuestion::gtQuestion(gtBase* parent, const char* title, const char* question)
{
    Initialize(gtQuestion::gtQuestion);

    defaultAnswer = 0;

    init(parent, title, question, NULL, NULL, NULL);
}


gtQuestion::gtQuestion(const char* title, const char* question)
{
    Initialize(gtQuestion::gtQuestion);

    defaultAnswer = 0;

    init(NULL, title, question, NULL, NULL, NULL);
}


gtQuestion::gtQuestion(const char* title, const char* question, const int defalt)
{
    Initialize(gtQuestion::gtQuestion);

    defaultAnswer = defalt;

    init(NULL, title, question, NULL, NULL, NULL);
}


gtQuestion::gtQuestion(const char* title, const char* question,
		   const char* OK_label, const char* Cancel_label)
{
    Initialize(gtQuestion::gtQuestion);

    defaultAnswer = 0;

    init(NULL, title, question, OK_label, NULL, Cancel_label);
}

gtQuestion::gtQuestion(const char* title, const char* question,
		   const char* OK_label, const char* OK2_label,  const char* Cancel_label)
{
    Initialize(gtQuestion::gtQuestion);

    defaultAnswer = 0;

    init(NULL, title, question, OK_label, OK2_label, Cancel_label);
}


void gtQuestion::init(gtBase* parent, const char* title, const char* question,
		    const char* OK_label, const char* OK2_label, const char* Cancel_label)
{
    Initialize(gtQuestion::init);

    shell = gtDialogTemplate::create(parent, "question", title);
    shell->add_button("ok", OK_label ? OK_label : TXT("OK"), OK_CB, this);
    if (OK2_label) shell->add_button("ok", OK2_label , OK2_CB, this);
    shell->add_button("cancel", Cancel_label ? Cancel_label : TXT("Cancel"),
		      Cancel_CB, this);
    shell->add_help_button();

    gtLabel* label = gtLabel::create(shell, "label", question);
    label->alignment(gtCenter);
    gtForm::vertStack(label);
    label->manage();
}


gtQuestion::~gtQuestion()
{
    Initialize(gtQuestion::~gtQuestion);

    delete shell;
}


int gtQuestion::ask()
{
    Initialize(gtQuestion::ask);

    answer = 0;
    if (gtDisplay::is_open() ) {
        shell->popup(1);
        shell->take_control_top(answer_func, this);
        shell->popdown();
    } else {
        answer = defaultAnswer;
    }

    return answer;
}


void gtQuestion::OK_CB(gtPushButton*, gtEvent*, void* cd, gtReason)
{
    Initialize(gtQuestion::OK_CB);

    ((gtQuestion*)cd)->answer = 1;
}

void gtQuestion::OK2_CB(gtPushButton*, gtEvent*, void* cd, gtReason)
{
    Initialize(gtQuestion::OK_CB);

    ((gtQuestion*)cd)->answer = 2;
}
    
void gtQuestion::Cancel_CB(gtPushButton*, gtEvent*, void* cd, gtReason)
{
    Initialize(gtQuestion::Cancel_CB);

    ((gtQuestion*)cd)->answer = -1;
}


int gtQuestion::answer_func(void* cd)
{
    Initialize(gtQuestion::answer_func);

    return ((gtQuestion*)cd)->answer;
}

#endif // NEW_UI

