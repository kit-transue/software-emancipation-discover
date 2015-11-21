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
#ifndef _STE_Edit_Style_h
#define _STE_Edit_Style_h

// STE_Edit_Style.h
//------------------------------------------
// synopsis:
// STE_Edit_Style dialog in libGT
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

#include <machdep.h>

#ifndef _ui_globals_h
#include <ui-globals.h>
#endif

class STE_Edit_Style_RGB;

class STE_Edit_Style {
    gtHorzBox		*name_slot, *options, *font_slot, *text_slot, *cursor_slot;
    gtPushButton	*style_name, *document_options, *tp_options, *text_color;
    gtPushButton	*cursor_color;	
    gtStringEditor	*style_name_btn;
    gtLabeledStringEditor	*es_level_text, *es_fontname_text, *es_fontsize_text;
    gtOptionMenu	*style_type_options;
    gtSeparator		*sep;
    gtToggleBox		*text_toggles, *cursor_toggles, *text_options;
    STE_Edit_Style_RGB	*TextColor_Editor, *CursorColor_Editor;
    
  public:
    gtDialogTemplate	*shell;
    int			i_style_type;
    ui_style		uis;
    swidget The_swidget_from_HELL;
    STE_Edit_Style(ui_style *, swidget I_really_needed_a_swidget);
    ~STE_Edit_Style();
    double text_val(gtStringEditor *);
    void set_fval(gtStringEditor *, double);
    void set_ival(gtStringEditor *, int);
    void write_style(ui_style *style);
    char *make_hex_name(unsigned long);
    void do_it();
    void disable_level();
    void enable_level();
    void disable_doc_items();
    void enable_doc_items();
    void disable_para_items();
    void enable_para_items();
    void read_style(ui_style *);
    static void read_style_name(ui_RTL_info *);
    static void OK_CB(gtPushButton *, gtEvent *, void *cd, gtReason);
    static void Apply_CB(gtPushButton *, gtEvent *, void *cd, gtReason);
    static void Cancel_CB(gtPushButton *, gtEvent *, void *cd, gtReason);
    static void StyleName_CB(gtPushButton *, gtEvent *, void *cd, gtReason);
    static void Document_CB(gtPushButton *, gtEvent *, void *cd, gtReason);
    static void Title_CB(gtPushButton *, gtEvent *, void *cd, gtReason);
    static void Paragraph_CB(gtPushButton *, gtEvent *, void *cd, gtReason);
    static void Character_CB(gtPushButton *, gtEvent *, void *cd, gtReason);
    static void DocOptions_CB(gtPushButton *, gtEvent *, void *cd, gtReason);
    static void TPOptions_CB(gtPushButton *, gtEvent *, void *cd, gtReason);
    static void Plain_CB(gtToggleButton *, gtEvent *, void *cd, gtReason);
    static void Bold_CB(gtToggleButton *, gtEvent *, void *cd, gtReason);
    static void Italic_CB(gtToggleButton *, gtEvent *, void *cd, gtReason);
    static void Underline_CB(gtToggleButton *, gtEvent *, void *cd, gtReason);
    static void Shaded_CB(gtToggleButton *, gtEvent *, void *cd, gtReason);
    static void Reverse_CB(gtToggleButton *, gtEvent *, void *cd, gtReason);
    static void TextColor_CB(gtPushButton *, gtEvent *, void *cd, gtReason);
    static void CursorColor_CB(gtPushButton *, gtEvent *, void *cd, gtReason);
    static int WMDestroyCancel(void*);
};

typedef STE_Edit_Style* STE_Edit_Style_Ptr;


/*
    START-LOG-------------------------------

   $Log: STE_Edit_Style.h  $
   Revision 1.1 1993/10/05 09:33:43EDT builder 
   made from unix file
 * Revision 1.2.1.4  1993/10/04  20:36:22  kws
 * Port
 *
 * Revision 1.2.1.3  1993/05/27  20:17:16  glenn
 * Protect nested includes using protect_includes script.
 *
 * Revision 1.2.1.2  1992/10/09  19:56:10  kws
 * Fix comments
 *
    END-LOG---------------------------------
*/

#endif // _STE_Edit_Style_h
