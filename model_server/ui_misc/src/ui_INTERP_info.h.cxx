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

//-------------------------------------------
// handler class for Interprocess communication
// via pipes.
//-------------------------------------------

#include "ui_INTERP_info.h"

#define XTFUNCPROTO
#include <X11/Intrinsic.h>
#undef XTFUNCPROTO

#include <objArr_Int.h>

// external variables
extern XtAppContext UxAppContext;

static objArr_Int pset_infos;
int ui_INTERP_info::turn_all_hooks (int flag)
{
    int sz = pset_infos.size();
    for (int ii = 0 ; ii < sz ; ii++) {
	int curr = pset_infos[ii];
	ui_INTERP_info *inf = (ui_INTERP_info *)curr;
	if (flag)
	    inf->set_hook (inf->hook, inf->mask, inf->client_data);
	else
	    inf->remove_hook ();
    }
    return sz;
}

ui_INTERP_info::ui_INTERP_info (int fd)
{
    this->fd = fd;
    xt_input_id = 0;
    hook = NULL;
    mask = 0;
    client_data = NULL;
    pset_infos.insert_last ((const int)this);
}

ui_INTERP_info::~ui_INTERP_info()
{
    if(xt_input_id)
        XtRemoveInput(xt_input_id);

    pset_infos.remove ((const int)this);
}

static void hook_handler (XtPointer c_data, int* fd, XtInputId *info)
{
    if (c_data) {
	hook_type hook;
	ui_INTERP_info *info = (ui_INTERP_info *) c_data;
	if (hook = info->get_hook())
	    hook (*fd, info->get_client_data());
    }
}

void ui_INTERP_info::remove_hook ()
{
    if(xt_input_id) {
        XtRemoveInput(xt_input_id);
	xt_input_id = 0;
    }
}

void ui_INTERP_info::set_hook (hook_type hook, XtInputMask input_mask, XtPointer data)
{
    if (fd >= 0 && UxAppContext != NULL)
	xt_input_id =
	    XtAppAddInput (UxAppContext, fd, (XtPointer) input_mask,
			   hook_handler, (XtPointer) this);
    this->hook = hook;
    mask = input_mask;
    client_data = data;
}

hook_type ui_INTERP_info::get_hook ()
{
    return hook;
}

XtPointer ui_INTERP_info::get_client_data()
{
    return client_data;
}




    

/*
  START-LOG-------------------------------------------

  $Log: ui_INTERP_info.h.C  $
  Revision 1.3 1998/07/29 16:34:30EDT azaparov 
  
Revision 1.2.1.4  1993/10/26  23:18:46  boris
Bug track: Testing
Debugger ParaSET synchronization

Revision 1.2.1.3  1992/11/14  00:04:46  smit
debugger cleanup

Revision 1.2.1.2  1992/10/09  17:21:11  oak
Moved log to file end.

  Revision 1.2.1.1  92/10/07  21:19:46  smit
  *** empty log message ***

  Revision 1.2  92/10/07  21:19:45  smit
  *** empty log message ***

  Revision 1.1  92/10/07  18:24:10  smit
  Initial revision

  Revision 1.1  92/05/15  09:16:01  smit
  Initial revision

  END-LOG---------------------------------------------
  */
