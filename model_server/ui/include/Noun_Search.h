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
#ifndef _Noun_Search_h
#define _Noun_Search_h

// Noun_Search.h
//------------------------------------------
// synopsis:
// Noun_Search dialog in libGT
//------------------------------------------

#ifndef _prototypes_h
#include <prototypes.h>
#endif
#ifndef _gt_h
#include <gt.h>
#endif
#ifndef _ui_globals_h
#include <ui-globals.h>
#endif

#define FWD_FROM_BEGINNING 0
#define FWD_FROM_CURSOR 1
#define BACK_FROM_CURSOR 2
#define BACK_FROM_END 3
#define UNDEFINED 4

class Noun_Search {
    gtList	*noun_list;
    
  public:
    gtDialogTemplate	*shell;
    gtRadioBox		*radbox;
    void		*nouns_rtl;
    ui_list	   	*noun_names_list;
    int			search_method;

    Noun_Search(char *);
    ~Noun_Search();
    void do_it();
    static void OK_CB(gtPushButton *, gtEvent *, void *cd, gtReason);
    static void Apply_CB(gtPushButton *, gtEvent *, void *cd, gtReason);
    static void Cancel_CB(gtPushButton *, gtEvent *, void *cd, gtReason);
    static void Beginning_CB(gtToggleButton *, gtEvent *, void *cd, gtReason);
    static void Previous_CB(gtToggleButton *, gtEvent *, void *cd, gtReason);
    static void Next_CB(gtToggleButton *, gtEvent *, void *cd, gtReason);
    static void From_CB(gtToggleButton *, gtEvent *, void *cd, gtReason);
};

typedef Noun_Search* Noun_Search_Ptr;


/*
    START-LOG-------------------------------

    $Log: Noun_Search.h  $
    Revision 1.1 1993/05/27 16:22:49EDT builder 
    made from unix file
 * Revision 1.2.1.3  1993/05/27  20:17:16  glenn
 * Protect nested includes using protect_includes script.
 *
 * Revision 1.2.1.2  1992/10/09  19:55:56  kws
 * Fix comments
 *
    END-LOG---------------------------------
*/

#endif // _Noun_Search_h
