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
#ifndef _viewerShell_manage_h
#define _viewerShell_manage_h

/*
// viewerShell_manage.h
//------------------------------------------
// synopsis:
// enumerated constants for communicating between viewerShell
// Manage menu callbacks and viewerShell.
//------------------------------------------
// Restrictions:
// This file must be C compatible.
//------------------------------------------
*/

#ifndef _prototypes_h
#include <prototypes.h>
#endif


BEGIN_C_DECL

enum viewerShell_MANAGE
{
    MANAGE_GET,
    MANAGE_PUT,
    MANAGE_COPY,
    MANAGE_DIFF,
    MANAGE_LOCK,
    MANAGE_UNLOCK,
    MANAGE_GET2,
    MANAGE_UNGET,
    MANAGE_OBSOLETE,
    MANAGE_UPDATE_MAKEFILE,
    MANAGE_MAKE
};

enum viewerShell_MERGE
{
    MERGE_PREV,
    MERGE_CURR,
    MERGE_NEXT,
    MERGE_DONE,
    MERGE_CANCEL,
    MERGE_STATUS
};

#ifdef __cplusplus
void viewerShell_manage (viewerShell*, viewerShell_MANAGE);
void viewerShell_merge  (viewerShell*, viewerShell_MERGE);
#else
void viewerShell_manage ();
void viewerShell_merge  ();
#endif

END_C_DECL


/*
   START-LOG-------------------------------------------

   $Log: viewerShell_manage.h  $
   Revision 1.6 2000/07/07 08:18:19EDT sschmidt 
   Port to SUNpro 5 compiler
// Revision 1.4  1994/07/29  17:15:56  so
// Bug track: n/a
// cm_create_new_file
// cm_put_start
// cm_put_end
// cm_unget_src
// cm_unget_pset
// cm_obsolete_src
// cm_obsolete_pset
// When do 3file-merge, check out file should be put into control project directly
//
// Revision 1.3  1994/05/12  16:33:10  so
// Bug track: 6758, 71978, 7239
// fix bug 6758, 7197, 7239, and improve put dialog box
//
// Revision 1.2  1993/12/14  17:18:29  andrea
// Bug track: 5466
// I implemented get...
//
// Revision 1.1  1993/05/10  23:18:40  glenn
// Initial revision
//
   END-LOG---------------------------------------------
*/

#endif /* _viewerShell_manage_h */
