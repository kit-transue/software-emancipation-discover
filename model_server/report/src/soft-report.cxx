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


#include "RTL.h"
#include "RTL_Names.h"
#include "RTL_externs.h"
#include "RTL_apl_extern.h"
#include "SoftAssoc.h"

// All Soft association.
static RTLNodePtr softassoc_rtl = NULL;

// All Soft association taht have to be notify.
static RTLNodePtr softassoc_notify_rtl = NULL;

RTLNodePtr get_softassoc_rtl() { return softassoc_rtl; }
RTLNodePtr get_softassoc_notify_rtl() { return softassoc_notify_rtl; }

#define SET_SOFTASSOC_RTL \
    if (softassoc_rtl == NULL) \
	softassoc_rtl = apl_RTL_get_rtl (ASET_SOFTASSOC)

#define SET_SOFTASSOC_NOTIFY_RTL \
    if (softassoc_notify_rtl == NULL) \
	softassoc_notify_rtl = apl_RTL_get_rtl (ASET_SOFTASSOC_NOTIFY)


// ------------------- SOFTASSOC RTL ---------------------
// Reports addition of soft association
void softassoc_report_add (SoftId *softid)
{
   Initialize (softassoc_report_add);

   // validate pointer
   if ( !softid)
      Return;   

   SET_SOFTASSOC_RTL;

   rtl_add_obj (softassoc_rtl, softid);

   Return;
}

// Reports deletion of softassoc
void softassoc_report_delete (SoftId *softid)
{
   Initialize (softassoc_report_delete);

   // validate pointer
   if ( !softid)
      Return;

   SET_SOFTASSOC_RTL;

   rtl_delete_obj (softassoc_rtl, softid);

   Return;
}

// Reports replacement of softassoc
void softassoc_report_replace (SoftId *old_s, SoftId *new_s)
{
   Initialize (softassoc_report_replace);

   // validate pointers
   if ( !old_s || !new_s)
      Return;

   SET_SOFTASSOC_RTL;

   rtl_delete_obj (softassoc_rtl, old_s);
   rtl_add_obj (softassoc_rtl, new_s);

   Return;
}

//------------------- SOFT ASSOCIATION NOTIFY RTL -----------------

// Reports addition of soft association notify
void softassoc_notify_report_add (SoftAssoc *softassoc)
{
   Initialize (softassoc_notify_report_add);

   // validate pointer
   if ( !softassoc)
      Return;   

   SET_SOFTASSOC_NOTIFY_RTL;

   rtl_add_obj (softassoc_notify_rtl, softassoc);

   Return;
}

// Reports deletion of soft association notify
void softassoc_notify_report_delete (SoftAssoc *softassoc)
{
   Initialize (softassoc_notify_report_delete);

   // validate pointer
   if ( !softassoc)
      Return;

   SET_SOFTASSOC_NOTIFY_RTL;

   rtl_delete_obj (softassoc_notify_rtl, softassoc);

   Return;
}

// Reports replacement of softassoc
void softassoc_notify_report_replace (SoftAssoc *old_s, SoftAssoc *new_s)
{
   Initialize (softassoc_notify_report_replace);

   // validate pointers
   if ( !old_s || !new_s)
      Return;

   SET_SOFTASSOC_NOTIFY_RTL;

// only if old exists in RTL than replace it with the new.
   if (softassoc_notify_rtl->rtl_includes(old_s)) {
            rtl_delete_obj (softassoc_notify_rtl, old_s);
            rtl_add_obj (softassoc_notify_rtl, new_s);
   }

   Return;
}

/*
   START-LOG-------------------------------------------

   $Log: soft-report.cxx  $
   Revision 1.1 1992/11/21 18:33:40EST builder 
   made from unix file
Revision 1.2.1.3  1992/11/21  23:33:28  builder
typesafe casts.

Revision 1.2.1.2  1992/10/09  19:20:59  smit
*** empty log message ***


   END-LOG---------------------------------------------

*/
