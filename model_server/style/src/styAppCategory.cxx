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
//////////////////////////   FILE steAppCategory_h.C  ///////////////////////////
//
// -- Contains  related Class Definition                  
#include "steAppCategory.h"
#include "objOper.h"
#include "steCategory.h"
#include "customize.h"
#ifndef ISO_CPP_HEADERS
#include <iostream.h>
#include <fstream.h>
#else /* ISO_CPP_HEADERS */
#include <iostream>
using namespace std;
#include <fstream>
#endif /* ISO_CPP_HEADERS */
#include "steHeader.h"
#include "steSlotTable.h"
#include <ste_get_file.h>

#ifndef ISO_CPP_HEADERS
#include <stdio.h>
#else /* ISO_CPP_HEADERS */
#include <cstdio>
#endif /* ISO_CPP_HEADERS */

init_relation(app_of_category,1,NULL,category_of_app,1,relationMode::D);
init_relational(steAppCategory,steSlotTable);

void steAppCategory::set_app_system( appPtr apl) 
         {put_relation(app_of_system_category, this, apl);}

void steAppCategory::set_app_user( appPtr apl) 
         {put_relation(app_of_user_category, this, apl);}

int  tot_length = 31;
char * white_str = new char [ tot_length ];

extern void sty_restore( steAppCategoryPtr, steSlotTablePtr, char *);

int steAppCategory::restore_styles (steSlotTablePtr stl, boolean glob)
{
    Initialize(steAppCategory::restore_styles);
    genString cat_file;
    if (glob)
	cat_file.printf("%s/lib/default_categories", customize::install_root());
    else
	cat_file.printf("%s/.categories", customize::home());

    char *buf = NULL;
    int   len = 0;
    
    int res = ste_get_file_text ((char *)cat_file, buf, len, (char *)0, 1);
    if (res == 0 && len > 0) { 
	sty_restore (this, stl, buf);
	delete buf; 
    } else
	res = -1;
    
    return res;
}

/*
   START-LOG-------------------------------------------

   $Log: styAppCategory.cxx  $
   Revision 1.7 2002/03/04 17:25:50EST ktrans 
   Change M/C/D/W/S macros to enums
 * Revision 1.5  1993/06/21  23:07:33  boris
 * Fixed bug #3770 with purify umr complain
 *
 * Revision 1.4  1993/02/12  16:16:57  oak
 * Removed customize::shadow_root().
 *
 * Revision 1.3  1993/01/19  23:39:39  boris
 * Added Category Save
 *
 * Revision 1.2  1992/12/18  19:05:09  glenn
 * Transferred from STE
 *
Revision 1.2.1.3  1992/11/22  03:15:45  builder
typesafe casts.

Revision 1.2.1.2  1992/10/09  19:48:06  boris
Fix comments



   END-LOG---------------------------------------------

*/
