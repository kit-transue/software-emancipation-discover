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
// init_gt_flush.h.C
//------------------------------------------
// synopsis:
// 
// Initialize callbacks used when flushing
// output before handling next X event.
//------------------------------------------

// INCLUDE FILES

#include "machdep.h"
#include <genError.h>
#include <gtBase.h>
#include <viewUIexterns.h>
#include <steView.h>
#ifdef NEW_UI
#include <vport.h>
#endif
/* graWindow.h must come after inclusion of Object.h, since nil is #defined to 0 */
#include <graWindow.h>
#ifdef NEW_UI
#include veventHEADER
#endif


// EXTERNAL FUNCTIONS
void smt_map_after_parsing(void*);
void CPprop_control_exec(void*);

// FUNCTION DEFINITIONS

static void cast_ste_freeze(void *arg)
{
    ste_freeze((int)arg);
}

#ifndef NEW_UI
void view_create_flush_finish();

void view_flush_creates (void*)
{
    view_create_flush_finish();
}

void init_gt_flush()
// flush changes before entering event loop
{
    Initialize(init_gt_flush);

    gtBase::flush_output_callback(cast_ste_freeze, 0);
    gtBase::flush_output_callback(view_update_raw_views, 0);
    gtBase::flush_output_callback(view_flush_rtls, 0);
    gtBase::flush_output_callback(graWindow::flush_all, 0);
    gtBase::flush_output_callback(view_flush_creates, 0);
}
#else

static void PreBlockHandler()
{

    cast_ste_freeze (0);
    smt_map_after_parsing(0);
    CPprop_control_exec(0);
}

void init_gt_flush()
{
    vevent::SetPreBlockHandlerProc (PreBlockHandler);
}

#endif
