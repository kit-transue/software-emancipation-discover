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
// Synopsis: Reporting functions for sscrapbook
//
//
// events like addition, deletion, replacement
// of scrapbooks are reported here.
//-----------------------------------------------


#include "genError.h"
#include "RTL.h"
#include "RTL_Names.h"
#include "RTL_externs.h"
#include "RTL_apl_extern.h"
#include "scrapbook-report.h"

static RTLNodePtr scrapbook_rtl = NULL;

#define SET_SCRAPBOOK_RTL \
    if (scrapbook_rtl == NULL) \
       scrapbook_rtl = apl_RTL_get_rtl (ASET_SCRAPBOOKS)

// Report creation of scrapbook
void scrapbook_report_create (scrapbook *sb)
{
    Initialize (scrapbook_report_create);

    SET_SCRAPBOOK_RTL;

    if (scrapbook_rtl)
	rtl_add_obj (scrapbook_rtl, sb);

    Return;
}

// Report deletion of scrapbook
void scrapbook_report_delete (scrapbook *sb)
{
    Initialize (scrapbook_report_delete);

    SET_SCRAPBOOK_RTL;

    if (scrapbook_rtl)
        rtl_delete_obj (scrapbook_rtl, sb);

    Return;
}

/*
   START-LOG-------------------------------------------

   $Log: scrapbook-report.cxx  $
   Revision 1.1 1992/11/21 18:33:40EST builder 
   made from unix file
Revision 1.2.1.3  1992/11/21  23:33:28  builder
typesafe casts.

Revision 1.2.1.2  1992/10/09  19:20:57  smit
*** empty log message ***


   END-LOG---------------------------------------------

*/
