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
#ifndef _STE_Create_sd_h
#define _STE_Create_sd_h

// STE_Create_sd.h
//------------------------------------------

#ifndef _prototypes_h
#include <prototypes.h>
#endif
#ifndef _gtDlgTemplate_h
#include <gtDlgTemplate.h>
#endif
#ifndef _gtHorzBox_h
#include <gtHorzBox.h>
#endif
#ifndef _gtLabel_h
#include <gtLabel.h>
#endif
#ifndef _gtStringEd_h
#include <gtStringEd.h>
#endif
#ifndef _gtList_h
#include <gtList.h>
#endif
#ifndef _gtForm_h
#include <gtForm.h>
#endif

class STE_Create_sd {
    gtHorzBox		*textslot1;
    gtLabel		*_name;
    gtStringEditor	*sd_text;
    gtList		*sd_catlist;

    struct ui_list 	*foolist;
    
  public:
    gtDialogTemplate 	*shell;
    
    STE_Create_sd();
    void init();
    void do_it();
    static void OK_CB(gtPushButton *, gtEvent *, void *cd, gtReason);
    static void Apply_CB(gtPushButton *, gtEvent *, void *cd, gtReason);
    static void Cancel_CB(gtPushButton *, gtEvent *, void *cd, gtReason);
};

typedef STE_Create_sd* STE_Create_sd_Ptr;


/*
    START-LOG-------------------------------

   $Log: STE_Create_sd.h  $
   Revision 1.1 1993/05/27 16:22:54EDT builder 
   made from unix file
 * Revision 1.2.1.3  1993/05/27  20:17:16  glenn
 * Protect nested includes using protect_includes script.
 *
 * Revision 1.2.1.2  1992/10/09  19:56:08  kws
 * Fix comments
 *
    END-LOG---------------------------------
*/

#endif // _STE_Create_sd_h
