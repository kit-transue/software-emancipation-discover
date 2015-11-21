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
#ifndef _ui_INTERP_info_h
#define _ui_INTERP_info_h

//-------------------------------------------
// handler class for Interprocess communication
// via pipes.
//-------------------------------------------

#ifndef _XtIntrinsic_h
/* -- there is a conflict betwee X11: Object, and nihcl: Object -- */
#ifndef _XtObject_h
#define _XtObject_h
#endif

#include <X11/Intrinsic.h>
#endif

typedef unsigned long   XtInputMask;

typedef void (*hook_type) (int, XtPointer);

class ui_INTERP_info
{
  public:
    ui_INTERP_info(int fd);
    ~ui_INTERP_info();

    void set_hook (hook_type, XtInputMask, XtPointer client_data);
    void remove_hook ();
    hook_type get_hook ();

    XtPointer get_client_data ();
    static int turn_all_hooks (int flag);
  private:
    XtInputId xt_input_id;
    void (*hook) (int fd, XtPointer input_mask);
    int fd;
    XtInputMask	mask;
    XtPointer client_data;
};

#endif

/*
    START-LOG-------------------------------

    $Log: ui_INTERP_info.h  $
    Revision 1.3 2000/07/07 08:18:15EDT sschmidt 
    Port to SUNpro 5 compiler
 * Revision 1.2.1.4  1993/10/26  23:20:00  boris
 * Bug track: Testing
 * Debugger ParaSET synchronization
 *
 * Revision 1.2.1.3  1992/11/14  00:05:22  smit
 * debugger cleanup
 *
 * Revision 1.2.1.2  1992/10/09  19:57:27  kws
 * Fix comments
 *

    END-LOG---------------------------------
*/

