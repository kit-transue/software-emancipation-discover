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
#ifndef _make_out_h
#define _make_out_h

#include <cLibraryFunctions.h>
#ifndef _gt_h
#include <gt.h>
#endif

#ifndef _ui_INTERP_info_h
#include <ui_INTERP_info.h>
#endif


class make_output_ui
{
  public:
    make_output_ui (gtBase *parent, char *name, char *target, char *path,
		    int modality = 0);
    ~make_output_ui();

    const gtTextEditor* text () { return text_output; }
    gtDialogTemplate* get_ui() { return ui; }
    FILE* get_pipe() { return pipe; }

    int open_pipe(const char* cmd, bool wait_for_completion = false);
    void close_pipe ();
    void read_from_pipe ();
    void append(const char *str);

  private:
    gtDialogTemplate *ui;
    gtTextEditor     *text_output;
    FILE	     *pipe;
    ui_INTERP_info   *x_info;
    bool	     is_modal;
    int*	     time_to_return;

    static void	     done_callback(gtPushButton*, gtEvent*, void*, gtReason);
    static void	     abort_callback(gtPushButton*, gtEvent*, void*, gtReason);
    static int	     is_done(void*);
};


/*
    START-LOG-------------------------------

    $Log: make-out.h  $
    Revision 1.2 1996/03/24 12:36:02EST rajan 
    isolate system dependencies - I
 * Revision 1.2.1.4  1993/05/27  22:10:51  wmm
 * Fix bugs 3482, 3480,  and 3476.
 *
 * Revision 1.2.1.3  1993/04/07  00:03:06  glenn
 * Use const char* interface.
 * Move include files into makeInterface.h.C.
 *
    END-LOG---------------------------------
*/

#endif
