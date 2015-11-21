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
#ifndef _Progress_h
#define _Progress_h

// Progress.h
//------------------------------------------
// synopsis:
// Dialog box with progress bar and message areas
// used in sub-class of waiter.
//------------------------------------------

#ifndef _gt_h
#include <gt.h>
#endif


class Progress
{
  public:
    gtDialogTemplate* shell;
    gtPushButton* help_button;
    gtPushButton* cancel_button;
    
    Progress(gtBase*, const char*, const char*, int (*)(void*, int), void*);
    ~Progress();

    void popup();
    void event_loop();
    void set_status(const char *);
    void set_title(const char *);
    void set_label(const char *);
    void set_percent(int);

  private:
    static void Cancel_CB(gtPushButton*, gtEvent*, void*, gtReason);

    gtLabel* label;
    gtLabel* status;
    gtScrollbar* scrollBar;

    int (*callback)(void*, int);
    void* callback_data;
};


/*
    START-LOG-------------------------------

    $Log: Progress.h  $
    Revision 1.1 1993/05/27 16:22:52EDT builder 
    made from unix file
 * Revision 1.2.1.4  1993/03/11  16:08:50  glenn
 * Remove unnecessary include files.
 * Fix ifndef statement with "." in symbol name.
 *
 * Revision 1.2.1.3  1993/02/22  22:22:20  oak
 * Fixed cancel callback problem.
 *
 * Revision 1.2.1.2  1992/10/09  19:56:01  kws
 * Fix comments
 *
    END-LOG---------------------------------
*/

#endif // _Progress_h
