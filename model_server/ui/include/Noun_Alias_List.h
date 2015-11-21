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
#ifndef _Noun_Alias_List_h
#define _Noun_Alias_List_h

// Noun_Alias_List.h
//------------------------------------------
// synopsis:
// ...
//
//------------------------------------------

#ifndef _gt_h
#include <gt.h>
#endif

extern "C" void popup_noun_alias_list();

struct ui_list;

class nounAliasList
{
    gtList*		noun_list;
    gtList*		alias_list;
    gtArrowButton*	arrowButton1;
    gtArrowButton*	arrowButton2;

  public:
    gtDialogTemplate*	shell;
    void*		noun_wl;
    ui_list*		nouns;
    ui_list*		aliases;

    nounAliasList(gtBase* parent);

    void init();
    void context();
    void cleanup();

    static void OK_CB(gtPushButton *, gtEvent *, void *cd, gtReason);
    static void Context_CB(gtPushButton *, gtEvent *, void *cd, gtReason);
    static void Cancel_CB(gtPushButton *, gtEvent *, void *cd, gtReason);
    static void List_CB(gtList *, gtEvent *, void *cd, gtReason);
    static void Left_CB(gtArrowButton *, gtEvent *, void *cd, gtReason);
    static void Right_CB(gtArrowButton *, gtEvent *, void *cd, gtReason);
};

typedef nounAliasList* nounAliasListPtr;

#endif // _Noun_Alias_List_h

/*
    START-LOG-------------------------------

    $Log: Noun_Alias_List.h  $
    Revision 1.1 1993/05/27 16:22:49EDT builder 
    made from unix file
 * Revision 1.2.1.2  1992/10/09  19:55:54  kws
 * Fix comments
 *

    END-LOG---------------------------------
*/

