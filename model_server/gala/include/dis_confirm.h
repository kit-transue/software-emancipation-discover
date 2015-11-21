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
//-----------------------------------------------------------------------------
// dis_confirm.h
//
// dis/tcl commands for Confirm Dialogs.
//-----------------------------------------------------------------------------

#ifndef _dis_confirm_h
#define _dis_confirm_h

#ifndef vportINCLUDED
#include <vport.h>
#endif

#ifndef _interp_h
#include "ginterp.h"
#endif

#ifndef _gString_h
#include "gString.h"
#endif

#ifndef _ggenConfirmDialog_h
#include "ggenConfirmDialog.h"
#endif


// TCL Command Procedures.
extern Tcl_CmdProc dis_confirm1;
extern Tcl_CmdProc dis_confirm2;
extern Tcl_CmdProc dis_confirm3;
extern Tcl_CmdProc dis_confirm_list;
extern Tcl_CmdProc dis_close_confirmDialog;

extern Tcl_CmdProc dis_prompt_string1;
extern Tcl_CmdProc dis_prompt_string2;
extern Tcl_CmdProc dis_prompt_string3;

extern Tcl_CmdProc dis_prompt_bool1;
extern Tcl_CmdProc dis_prompt_bool2;
extern Tcl_CmdProc dis_prompt_bool3;

extern Tcl_CmdProc dis_prompt_int1;
extern Tcl_CmdProc dis_prompt_int2;
extern Tcl_CmdProc dis_prompt_int3;

extern Tcl_CmdProc dis_prompt_file;

// This file should be obsoleted in the future.
extern Tcl_CmdProc dis_prompt;


// dis_confirm utility functions.
const int MAX_LABELS = 3;
int get_labels(const vchar* input, gString labels[MAX_LABELS]);
vbool place_labels(GConfirmDialog* myDialog,
                   gString labels[MAX_LABELS],
                   int numLabels);

#endif // _dis_confirm_h
