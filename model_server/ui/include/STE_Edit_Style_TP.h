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
#ifndef _STE_Edit_Style_TP_h
#define _STE_Edit_Style_TP_h

// STE_Edit_Style_TP.h
//------------------------------------------
// synopsis:
// STE_Edit_Style_TP dialog in libGT
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

class STE_Edit_Style_TP {
    gtRadioBox		*alignment, *spacing, *units;
    gtVertBox		*margin_slot, *spacing_slot, *enumeration_slot;
    gtLabel		*margin_label;
    gtLabeledStringEditor	*margin_left, *margin_right, *margin_first;
    gtLabeledStringEditor	*spacing_before, *spacing_line;
    gtLabeledStringEditor	*enum_terminator, *enum_separator;
    gtOptionMenu	*enum_menu;
    gtSeparator		*sep1, *sep2;
    
  public:
    ui_style		*write_uis;
    int			i_enum_type;
    gtDialogTemplate	*shell;
    STE_Edit_Style_TP(ui_style *);
    ~STE_Edit_Style_TP();
    double text_val(gtLabeledStringEditor *);
    void do_it();
    void set_fval(gtLabeledStringEditor *, double);
    void disable_sp_trm();
    void enable_sp_trm();
    void read_style(ui_style *);
    void write_style(ui_style *);
    static void OK_CB(gtPushButton *, gtEvent *, void *cd, gtReason);
    static void Apply_CB(gtPushButton *, gtEvent *, void *cd, gtReason);
    static void Cancel_CB(gtPushButton *, gtEvent *, void *cd, gtReason);
    static void Enum0_CB(gtPushButton *, gtEvent *, void *cd, gtReason);
    static void Enum1_CB(gtPushButton *, gtEvent *, void *cd, gtReason);
    static void Enum2_CB(gtPushButton *, gtEvent *, void *cd, gtReason);
    static void Enum3_CB(gtPushButton *, gtEvent *, void *cd, gtReason);
    static void Enum4_CB(gtPushButton *, gtEvent *, void *cd, gtReason);
    static void Enum5_CB(gtPushButton *, gtEvent *, void *cd, gtReason);
    static void Enum6_CB(gtPushButton *, gtEvent *, void *cd, gtReason);
    static void Enum7_CB(gtPushButton *, gtEvent *, void *cd, gtReason);
};

typedef STE_Edit_Style_TP* STE_Edit_Style_TP_Ptr;


/*
    START-LOG-------------------------------

   $Log: STE_Edit_Style_TP.h  $
   Revision 1.1 1993/05/27 16:22:56EDT builder 
   made from unix file
 * Revision 1.2.1.3  1993/05/27  20:17:16  glenn
 * Protect nested includes using protect_includes script.
 *
 * Revision 1.2.1.2  1992/10/09  19:56:13  kws
 * Fix comments
 *
    END-LOG---------------------------------
*/

#endif // _STE_Edit_Style_TP_h
