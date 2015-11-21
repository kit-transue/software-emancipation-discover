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
#include "machdep.h"
#include "cLibraryFunctions.h"
#include <msg.h>
#include <proj.h>
#ifdef _WIN32
#define max __max
#define min __min
#endif
#include <viewerShell.h>
#ifdef _WIN32
#undef max
#undef min
#endif
#include <gtDisplay.h>
#include <top_widgets.h>
#include <smt.h>
#include <ddict.h>
#ifndef _WIN32
#include <gtBase.h>
#endif
#include <messages.h>
#include <browserShell.h>

extern int ignore_prop;

int update_browser ()
{
    browserShell::browser_manage_menus(projNode::home_is_writeable() && !customize::no_cm());
    viewerShell::enable_managers();
    if (gtDisplay::is_open())
      pop_cursor();

    return 0;
}

int check_cursor()
{
    int retval = gtDisplay::is_open();
    if (retval)
	push_busy_cursor();

    return retval;
}

int browserShell_update_existing_project_node(projNode* pn)
{
    browserShell::update_existing_project_node(pn);
    return 0;
}
