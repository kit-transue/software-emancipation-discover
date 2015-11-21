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
#ifndef _SelectFromRTL_h
#define _SelectFromRTL_h

// SelectFromRTL.h
//------------------------------------------
// synopsis:
// Dialog window to select one or more objects from a run-time list.
//
//------------------------------------------

#ifndef _gt_h
#include <gt.h>
#endif

class objArr;

class SelectFromRTL
{
  public:
    static void List_CB(gtList*, gtEventPtr, void*, gtReason);
    static void Action_CB(gtList*, gtEventPtr, void*, gtReason);
    static void OK_CB(gtPushButton*, gtEventPtr, void*, gtReason);
    static void Cancel_CB(gtPushButton*, gtEventPtr, void*, gtReason);
    static void Apply_CB(gtPushButton*, gtEventPtr, void*, gtReason);
    static int confirm_func(void*);

    gtDialogTemplate*	shell;
    gtLabel*		list_label;
    gtRTL*		rtlist;
    gtLabel*		select_label;
    gtStringEditor*	select_text;

    void		(*select_callback)(void*, int, int, const char*);
    void*		select_callback_data;

    unsigned int	modality : 1;
    unsigned int	text_editor_enabled : 1;
    unsigned int	confirm : 1;

    SelectFromRTL(int modal, const char* title,
		  void (*callback)(void* callback_data, int, int, const char*),
		  void* callback_data);
    ~SelectFromRTL();

    void buttons(const char*, const char*, const char*);
    void mode(gtListStyle, int enable_text_editor);
    void init(const char*, void*, int sort_order = 1);
    void popup();
    void popdown();
    void selection(int);
    void* run(objArr*);
    void do_it(int);
    void help_context_name(const char*);
};

typedef SelectFromRTL* SelectFromRTL_Ptr;

#endif // _SelectFromRTL_h

/*
    START-LOG-------------------------------

    $Log: SelectFromRTL.h  $
    Revision 1.1 1993/05/27 16:22:58EDT builder 
    made from unix file
 * Revision 1.2.1.4  1993/04/28  12:41:03  jon
 * Added member function SelectFromRTL::help_context_name()
 *
 * Revision 1.2.1.3  1993/01/26  19:37:30  jon
 * Added action callback to fix bug #1534
 *
 * Revision 1.2.1.2  1992/10/09  19:56:17  kws
 * Fix comments
 *

    END-LOG---------------------------------
*/

