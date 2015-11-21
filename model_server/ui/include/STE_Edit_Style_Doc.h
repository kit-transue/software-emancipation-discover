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
#ifndef _STE_Edit_Style_Doc_h
#define _STE_Edit_Style_Doc_h

// STE_Edit_Style_Doc.h
//------------------------------------------
// synopsis:
// STE_Edit_Style_Doc dialog in libGT
//------------------------------------------

#ifndef _prototypes_h
#include <prototypes.h>
#endif
#ifndef _ui_struct_h
#include <ui-struct.h>
#endif
#ifndef _gt_h
#include <gt.h>
#endif

class STE_Edit_Style_Doc {
    ui_style	*write_uis;
    gtVertBox	*page_slot, *margin_slot;
    gtLabeledStringEditor	*page_height, *page_width, *margin_top, *margin_bottom;
    gtRadioBox			*units, *orientation;

  public:
    gtDialogTemplate	*shell;
    STE_Edit_Style_Doc(ui_style *);
    ~STE_Edit_Style_Doc();
    void do_it();
    void read_style(ui_style *);
    void write_style(ui_style *);
    void set_fval(gtStringEditor *, double) ;
    static void OK_CB(gtPushButton *, gtEvent *, void *cd, gtReason);
    static void Apply_CB(gtPushButton *, gtEvent *, void *cd, gtReason);
    static void Cancel_CB(gtPushButton *, gtEvent *, void *cd, gtReason);
};

typedef STE_Edit_Style_Doc* STE_Edit_Style_Doc_Ptr;


/*
    START-LOG-------------------------------

    $Log: STE_Edit_Style_Doc.h  $
    Revision 1.1 1993/05/27 16:22:55EDT builder 
    made from unix file
 * Revision 1.2.1.3  1993/05/27  20:17:16  glenn
 * Protect nested includes using protect_includes script.
 *
 * Revision 1.2.1.2  1992/10/09  19:56:11  kws
 * Fix comments
 *
    END-LOG---------------------------------
*/

#endif // _STE_Edit_Style_Doc_h
