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
#ifndef _save_app_dialog_h
#define _save_app_dialog_h

// save_app_dialog.h
//------------------------------------------
// synopsis:
//  define the declarations of routines
//------------------------------------------

#ifdef _WIN32
class gtBase;
#else
#ifndef _gt_h
#include <gt.h>
#endif
#endif

#ifndef _Save_Apps_h
#include <Save_Apps.h>
#endif

void offer_to_save_apps(gtBase*, dlgFlavor flavor,
			void (*OK_callback)(void*), void* OK_data);
void offer_to_save_list(gtBase*, dlgFlavor flavor, objArr* objList,
			void (*OK_callback)(void*), void* OK_data);
int offer_to_save_list_wait(gtBase*, dlgFlavor flavor, objArr* objList,
			    void (*OK_callback)(void*), void* OK_data, int wait);
int offer_to_save_wait(gtBase* parent, dlgFlavor flavor, objArr *objList,
			      void (*OK_callback)(void*), void* OK_data, int wait);

#endif // _save_app_dialog_h

/*
    START-LOG-------------------------------

    $Log: save_app_dialog.h  $
    Revision 1.5 2002/03/15 12:56:24EST builder 
    Fixing argument name 'list'
// Revision 1.2  1994/07/11  17:17:43  so
// Bug track: n/a
// project for re-load pdf
//
// Revision 1.1  1993/02/10  01:00:19  jon
// Initial revision
//

    END-LOG---------------------------------
*/

