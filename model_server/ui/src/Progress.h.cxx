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
// Progress.h.C
//------------------------------------------
// synopsis:
// Dialog box with progress bar and message areas
// used in sub-class of waiter.
//------------------------------------------

// INCLUDE FILES

#include "machdep.h"
#ifndef ISO_CPP_HEADERS
#include <stdlib.h>
#include <string.h>
#else /* ISO_CPP_HEADERS */
#include <cstdlib>
using namespace std;
#include <cstring>
#endif /* ISO_CPP_HEADERS */

#include <gtDlgTemplate.h>
#include <gtLabel.h>
#include <gtScrollbar.h>
#include <gtPushButton.h>

#include <genError.h>
#include <representation.h>
#include <driver.h>

#include <Progress.h>

#include <psetmem.h>
#include <machdep.h>

#define TXT(a) a

// FUNCTION DEFINITIONS

Progress::Progress(gtBase *parent, const char *title, const char *label_string,
		   int (*arg_callback)(void *, int), void *arg_callback_data)
{
    Initialize(Progress::Progress);
#ifndef NEW_UI
    callback = arg_callback;
    callback_data = arg_callback_data;

    if (!title)
	title = TXT("Progress");
    shell = gtDialogTemplate::create(parent, "Progress", title);
    shell->add_button("cancel", TXT("Cancel"), Progress::Cancel_CB, this);
    shell->add_help_button();
    shell->pos(378,270);

    if (!label_string)
	label_string = "";
    label = gtLabel::create(shell, "label", label_string);
    label->attach_side_top();
    label->attach_side_left();
    label->alignment(gtCenter);

    scrollBar = gtScrollbar::create(shell, "scrollBar", NULL, NULL, gtHORZ);
    scrollBar->attach(gtTop, label, 30);
    scrollBar->attach_side_left();
    scrollBar->attach_side_right();
    scrollBar->set_sensitive(0);
    scrollBar->arrows(0);
    scrollBar->manage();

    status = gtLabel::create(shell, "status", " ");
    status->attach(gtTop, scrollBar, 30);
    status->attach_side_left();
    status->attach_side_right();
    status->attach(gtBottom, NULL, 10);
    status->alignment(gtCenter);

    cancel_button = shell->button("cancel");
    help_button = shell->button("help");
#endif
}

Progress::~Progress()
{
    Initialize(Progress::~Progress);
#ifndef NEW_UI
    shell->unmap();
    shell->popdown();
    delete shell;
#endif
}

void Progress::event_loop()
{
    Initialize(Progress::event_loop);
#ifndef NEW_UI
    int bell_not_rung = 1;
    shell->flush();
    shell->update_display();

    int done = 0;
    while(!done)
    {
	gtEvent event;

	if (shell->check_window_event(0xFFFFFFFF, &event))
	{
	    gtBase::dispatch(&event);
	}
	else if(shell->check_mask_event(
	    ButtonPressMask|ButtonReleaseMask|KeyPressMask|KeyReleaseMask,
	    &event))
	{
	    if(help_button->my_window(&event) ||
	       cancel_button->my_window(&event))
	    {
		gtBase::dispatch(&event);
	    }
	    else if(bell_not_rung &&
		    ((event.type == KeyPress) || (event.type == ButtonPress)))
	    {
		shell->bell(50);
		shell->flush();
		bell_not_rung = 0;
	    }
	}
	else
	    done = 1;
    }
#endif
}

void Progress::set_status (const char *string)    
{
    Initialize(Progress::set_status);
#ifndef NEW_UI
    char* string_copy = NULL;
    int len = (string ? strlen(string) : 0);
    
    if(len <= 0)
    {
	string = "";
    }
    else if(string[len - 1] == '\n') /* strip trailing newline */
    {
	if(string_copy = psetmalloc(len))
	{
	    OS_dependent::bcopy(string, string_copy, len - 1);
	    string_copy[len - 1] = '\0';
	    string = string_copy;
	}
    }
    
    status->text(string);
    status->remanage();
    
    if(string_copy)
	free(string_copy);
    event_loop();
#endif
}

void Progress::set_title(const char *new_title)
{
    Initialize(Progress::set_title);
#ifndef NEW_UI
    shell->title(new_title);
    shell->flush();
    shell->update_display();
#endif
}

void Progress::set_label(const char *new_title)
{
    Initialize(Progress::set_label);
#ifndef NEW_UI
    label->text(new_title);
    label->remanage();
    shell->flush();
    shell->update_display();
#endif
}

void Progress::set_percent(int percent)
{
    Initialize(Progress::set_percent);
#ifndef NEW_UI
    scrollBar->slider_size(percent);
    event_loop();
#endif
}

void Progress::popup()
{
    Initialize(Progress::popup);
#ifndef NEW_UI
    shell->popup(0);
    shell->flush();
    shell->update_display();
#endif
}

void Progress::Cancel_CB(gtPushButton *, gtEvent *, void *cd, gtReason)
{
    Initialize(Progress::Cancel_CB);
   
#ifndef NEW_UI 
    Progress* PRP = (Progress*)cd;
    if (PRP->callback && (*(PRP->callback))(PRP->callback_data, -1))
	Return;
    delete PRP;
#endif
}


/*
  START-LOG-------------------------------------------
  
  $Log: Progress.h.C  $
  Revision 1.3 2000/07/10 23:11:35EDT ktrans 
  mainline merge from Visual C++ 6/ISO (extensionless) standard header files
Revision 1.2.1.8  1994/03/24  15:55:34  kws
Bug track: psetmem
- compliance

Revision 1.2.1.7  1993/07/20  18:56:31  harry
HP/Solaris Port

Revision 1.2.1.6  1993/06/14  20:05:48  jon
Modified event_loop to ring bell only on key or button press to prevent the
extranous beep that happened when the user released the button or key that
launched whatever action required the progress dialog.

Revision 1.2.1.5  1993/03/11  16:07:32  glenn
Insert include files that used to be in Progress.h
Add more Initialize macros.
Remove references to globdef.h

Revision 1.2.1.4  1993/02/22  22:23:40  oak
Fixed problem with cancel button.

  Revision 1.2.1.3  1992/10/23  17:40:52  glenn
  Minor cleanup.
  
  Revision 1.2.1.2  92/10/09  20:13:39  jon
  RCS History Marker Fixup
  
  END-LOG---------------------------------------------
*/
