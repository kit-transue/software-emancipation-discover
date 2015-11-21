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
#ifndef _NewPrompt_h
#define _NewPrompt_h

// NewPrompt.h
//------------------------------------------
// synopsis:
// 
// A simple dialog box to prompt for a line of input
//
//------------------------------------------

#ifndef _gt_h
#include <gt.h>
#endif

#ifndef _genString_h
#include <genString.h>
#endif

class Prompt
{
  public:
    Prompt(gtBase* parent, const char* title, const char* prompt);
    Prompt(gtBase* parent, const char* title, const char* prompt,
           const char* err_msg);
    Prompt(gtBase* parent, const char* title, const char* prompt,
           const char* err_msg, const char* OK2_label);
    ~Prompt();

    int oneshot(genString& answer);

    int ask(genString& answer) { return oneshot(answer); }

  private:
    static void OK_CB (gtPushButton*, gtEventPtr, void*, gtReason);
    static void Cancel_CB (gtPushButton*, gtEventPtr, void*, gtReason);
    static int take_control_CB (void*);
    static void OK2_CB (gtPushButton*, gtEventPtr, void*, gtReason);

    gtDialogTemplate*	shell;
    gtLabel*		label;
    gtStringEditor*	answer_text;
    int			confirm;
    genString           err_message;
    genString           OK2_label;

    void init(gtBase* parent, const char* title, const char* prompt,
	 const char* err_msg, const char* OK2_label);

};

typedef Prompt* Prompt_Ptr;

#endif // _NewPrompt_h

/*
    START-LOG-------------------------------

    $Log: NewPrompt.h  $
    Revision 1.4 1995/04/11 08:51:17EDT so 
    Bug track: n/a
    fix bug 9250
 * Revision 1.2.1.4  1993/08/20  23:30:14  andrea
 * added optional second ok button to prompt box
 *
 * Revision 1.2.1.3  1993/07/09  22:39:24  andrea
 * fixed bug in scripting
 *
 * Revision 1.2.1.2  1992/10/09  19:55:51  kws
 * Fix comments
 *

    END-LOG---------------------------------
*/


