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
//
//-----------------------------------------------
//
// Synopsis: Reporting soft association
//
//



#ifndef _soft_report_h__
#define _soft_report_h__

#include "SoftAssoc.h"
#include "RTL.h"
#include "RTL_Names.h"
#include "RTL_externs.h"

// global RTL
extern RTLNodePtr get_softassoc_rtl();
extern RTLNodePtr get_softassoc_notify_rtl();


// soft association reporting functions
extern void softassoc_report_add (SoftId *);
extern void softassoc_report_delete (SoftId *);
extern void softassoc_report_replace (SoftId *, SoftId *);


// soft association notify reporting functions
extern void softassoc_notify_report_add (SoftAssoc *);
extern void softassoc_notify_report_delete (SoftAssoc *);
extern void softassoc_notify_report_replace (SoftAssoc *, SoftAssoc *);
/*
   START-LOG-------------------------------------------

   $Log: soft-report.h  $
   Revision 1.1 1993/07/28 19:52:59EDT builder 
   made from unix file
 * Revision 1.2.1.2  1992/10/09  19:18:35  smit
 * *** empty log message ***
 *

   END-LOG---------------------------------------------

*/

#endif
