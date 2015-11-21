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
#ifndef _NameDescEdit_h
#define _NameDescEdit_h

// NameDescEdit.h
//------------------------------------------

#ifndef _prototypes_h
#include <prototypes.h>
#endif
#ifndef _gtDlgTemplate_h
#include <gtDlgTemplate.h>
#endif
#ifndef _gtLabel_h
#include <gtLabel.h>
#endif
#ifndef _gtTextEd_h
#include <gtTextEd.h>
#endif

BEGIN_C_DECL
void init_NameDescEdit PROTO((const char *name, const char *desc));
void popup_NameDescEdit PROTO(());
void popdown_NameDescEdit PROTO(());
void destroy_NameDescEdit PROTO(());
void create_NameDescEdit PROTO(
    (const char*, void (*)(void*, const char*), void*));
END_C_DECL

class NameDescEdit {
    gtLabel		*name;
    gtTextEditor	*scrolledText;
    void		(*callback)(void*, const char*);
    void		*callback_data;
    char 		*saved_text;
    void                *cancelCB_data;
    void                (*cancelCB)(void *);
    
    void cancel();
  public:
    gtDialogTemplate	*shell;

    NameDescEdit(const char* title);
    void init(void (*)(void*, const char*), void*);
    void addcancelCB(void (*cb)(void *), void *);
    void do_init_NDE(const char*, const char*);
    void do_it();
    static void OK_CB(gtPushButton*, gtEvent*, void*, gtReason);
    static void Revert_CB(gtPushButton*, gtEvent*, void*, gtReason);
    static void Cancel_CB(gtPushButton*, gtEvent*, void*, gtReason);
};

typedef NameDescEdit* NameDescEdit_Ptr;


/*
    START-LOG-------------------------------

    $Log: NameDescEdit.h  $
    Revision 1.1 1993/05/27 16:22:48EDT builder 
    made from unix file
 * Revision 1.2.1.3  1993/05/27  20:16:45  glenn
 * *** empty log message ***
 *
 * Revision 1.2.1.2  1992/10/09  19:55:45  kws
 * Fix comments
 *
    END-LOG---------------------------------
*/

#endif // _NameDescEdit_h
