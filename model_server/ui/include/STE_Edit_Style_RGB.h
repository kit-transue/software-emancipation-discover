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
#ifndef _STE_Edit_Style_RGB_h
#define _STE_Edit_Style_RGB_h

// STE_Edit_Style_RGB.h
//------------------------------------------
// synopsis:
// STE_Edit_Style_RGB dialog in libGT
//------------------------------------------

#ifndef _prototypes_h
#include <prototypes.h>
#endif
#ifndef _gt_h
#include <gt.h>
#endif
#ifndef _gtColormap_h
#include <gtColormap.h>
#endif

class STE_Edit_Style_RGB {
    gtVertBox		*scale_slot;
    gtScale		*red, *green, *blue;
    gtToggleButton	*fore, *back;
    gtRadioBox		*foreback;
    gtList		*color_list;
    gtColormap		*i_cmap;
    gtColor		color[2];
    unsigned long	cell[2];
    
  public:
    gtPrimitive		*color_gt;
    gtDialogTemplate	*shell;
    STE_Edit_Style_RGB(gtPrimitive *);
    ~STE_Edit_Style_RGB();
    void do_it();
    void do_color();
    void set_sliders(unsigned long);
    void init_colors();
    static void OK_CB(gtPushButton *, gtEvent *, void *cd, gtReason);
    static void Apply_CB(gtPushButton *, gtEvent *, void *cd, gtReason);
    static void Cancel_CB(gtPushButton *, gtEvent *, void *cd, gtReason);
    static void Slider_CB(gtScale *, gtEvent *, void *cd, gtReason);
    static void Select_CB(gtList *, gtEvent *, void *cd, gtReason);
    static void Fore_CB(gtToggleButton *, gtEvent *, void *cd, gtReason);
    static void Back_CB(gtToggleButton *, gtEvent *, void *cd, gtReason);
};

typedef STE_Edit_Style_RGB* STE_Edit_Style_RGB_Ptr;


/*
    START-LOG-------------------------------

   $Log: STE_Edit_Style_RGB.h  $
   Revision 1.1 1993/05/27 16:22:55EDT builder 
   made from unix file
 * Revision 1.2.1.3  1993/05/27  20:17:16  glenn
 * Protect nested includes using protect_includes script.
 *
 * Revision 1.2.1.2  1992/10/09  19:56:12  kws
 * Fix comments
 *
    END-LOG---------------------------------
*/

#endif // _STE_Edit_Style_RGB_h
