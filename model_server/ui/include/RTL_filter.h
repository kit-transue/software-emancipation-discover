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
#ifndef _RTL_filter_h
#define _RTL_filter_h

// RTL_filter.h
//------------------------------------------

#ifndef _prototypes_h
#include <prototypes.h>
#endif
#ifndef _gtDlgTemplate_h
#include <gtDlgTemplate.h>
#endif
#ifndef _gtStringEd_h
#include <gtStringEd.h>
#endif
#ifndef _gtRadioBox_h
#include <gtRadioBox.h>
#endif
#ifndef _ui_globals_h
#include <ui-globals.h>
#endif
#ifndef _RTL_Names_h
#include <RTL_Names.h>
#endif

BEGIN_C_DECL
char *skip_RTL_icons(char *initial_pointer);
void popup_Filter PROTO((ui_RTL_info *));
END_C_DECL

class RTL_filter {
    gtRadioBox		*rbox;
    gtStringEditor	*filter_string;
    
  public:
    struct ui_RTL_info	*i_info;
    gtDialogTemplate	*shell;
    RTL_filter();
    void do_it();
    static void OK_CB(gtPushButton *, gtEvent *, void *cd, gtReason);
    static void Apply_CB(gtPushButton *, gtEvent *, void *cd, gtReason);
    static void Cancel_CB(gtPushButton *, gtEvent *, void *cd, gtReason);
};

typedef RTL_filter* RTL_filter_Ptr;


/*
    START-LOG-------------------------------

   $Log: RTL_filter.h  $
   Revision 1.1 1994/02/25 09:32:10EST builder 
   made from unix file
 * Revision 1.2.1.4  1994/02/24  23:11:44  azaparov
 * Bug track: N/A
 * Added skip_RTL_icon function
 *
 * Revision 1.2.1.3  1993/05/27  20:17:16  glenn
 * Protect nested includes using protect_includes script.
 *
 * Revision 1.2.1.2  1992/10/09  19:56:05  kws
 * Fix comments
 *
    END-LOG---------------------------------
*/

#endif // _RTL_filter_h
