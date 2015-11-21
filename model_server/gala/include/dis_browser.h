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
// dis_browser.h
//
// dis/tcl commands for the Browser layers.
//-----------------------------------------------------------------------------

#ifndef _dis_browser_h
#define _dis_browser_h

#ifndef vportINCLUDED
#include <vport.h>
#endif

#ifndef _interp_h
#include "ginterp.h"
#endif

#ifndef _gString_h
#include "gString.h"
#endif



// TCL Command Procedures.
extern Tcl_CmdProc dis_browser_get_askList;
extern Tcl_CmdProc dis_browser_get_categoriesList;
extern Tcl_CmdProc dis_browser_update_categoriesList;
extern Tcl_CmdProc dis_browser_update_askList_for_categories;
extern Tcl_CmdProc dis_browser_update_askList_for_rtl;
extern Tcl_CmdProc dis_browser_convert_ask_selection;
extern Tcl_CmdProc dis_browser_convert_category_selection;




#endif // _dis_browser_h
