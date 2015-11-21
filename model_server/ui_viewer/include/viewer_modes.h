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
#ifndef _viewer_modes_h
#define _viewer_modes_h

/* File viewer_modes.h
  eumerations relating to the viewerShell
   until uim/x is removed, this file needs to be compatible
   with K&R C (it is included in viewerShell.if) */

enum viewerEditMode
{
    VE_base,
    VE_1_1_relation,
    VE_1_many_relation,
    VE_many_1_relation,
    VE_many_many_relation
};


enum viewerCustMode
{
    VC_normal,
    VC_customize,
    VC_remove
};

/* If you change enum viewType, you */
/* must also edit the file viewerData.C */
/* If you add a viewType, make sure you save */
/* its button bar in the customize class. */
enum viewType			
{				
    view_STE,		
    view_SMT,
    view_CallTree,
    view_Class,
    view_ERD,
    view_SubsysBrowser,
    view_SubsysMap,
    view_DC,
    view_Raw,
    view_DGraph,

    view_Last
};


/*
    START-LOG-------------------------------

    $Log: viewer_modes.h  $
    Revision 1.4 2000/04/17 10:44:47EDT ktrans 
    Periodic merge from message branch:
    clean up virtual function hierarchies
    remove dormant code (including hyperlinks/help)
    etc.
    For details for this file, see contributor version on stream_message branch.
 * Revision 1.2.1.7  1994/07/27  22:53:37  bakshi
 * Bug track: n/a
 * epoch list project
 *
 * Revision 1.2.1.6  1993/05/03  19:57:32  glenn
 * Add view_Raw to enum viewType.
 *
 * Revision 1.2.1.5  1993/02/10  16:13:36  oak
 * Added some comments.
 *
 * Revision 1.2.1.4  1992/12/17  20:56:26  wmm
 * Add view_DC for data charts.
 *
 * Revision 1.2.1.3  1992/12/12  07:09:57  glenn
 * viewerMode -> viewerEditMode.
 * Add viewerCustMode.
 * Remove viewerShell_mode.
 *
 * Revision 1.2.1.2  1992/10/09  19:57:29  kws
 * Fix comments
 *
    END-LOG---------------------------------
*/

#endif /* _viewer_modes_h */
