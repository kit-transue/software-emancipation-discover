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

#include <RTL_Names.h>
#include <RTL.h>
#include <RTL_apl_extern.h>
#include <graResources.h>

RTLNode *apl_RTL_get_rtl (char *rtl_name)
{
   Initialize (apl_RTL_get_rtl);

   RTLPtr header = checked_cast(RTL, app::get_header (App_RTL, rtl_name));

   RTLNodePtr rtl = header ? checked_cast(RTLNode, header->get_root()): NULL;

   ReturnValue (rtl);
}


RTLNode *apl_RTL_create (char *rtl_name)
{
   RTLNodePtr rtl;

   Initialize (apl_RTL_create);

   rtl = apl_RTL_get_rtl (rtl_name);

   if (!rtl) {
     RTLPtr rtl_head;

     rtl_head = db_new (RTL, (rtl_name));

     rtl = checked_cast(RTLNode, rtl_head->get_root());
    }    
         
   ReturnValue (rtl);
}

// Keep this function in sync with RTL_Names.h

void RTL_init ()
{
    Initialize (RTL_init);
 
    // Create other system rtls.
    apl_RTL_create (ASET_DOCUMENTS);
    apl_RTL_create (ASET_FUNCTIONS);
    apl_RTL_create (ASET_CATEGORIES);
    apl_RTL_create (ASET_FORMATS);
    apl_RTL_create (ASET_STYLES);
    apl_RTL_create (ASET_VARIABLES);
    apl_RTL_create (ASET_USER_CATEGORIES);
    apl_RTL_create (ASET_CLASSES);
    apl_RTL_create (ASET_PROGRAM_FILE_NAMES);
    apl_RTL_create (ASET_OODT_RELATIONS);
    apl_RTL_create (ASET_SCRAPBOOKS);
    apl_RTL_create (ASET_SUBSYSTEMS);
    apl_RTL_create (ASET_SOFTASSOC);
    apl_RTL_create (ASET_SOFTASSOC_NOTIFY);
    apl_RTL_create (ASET_USER_TYPE);
 
    RTLNodePtr rtlnode = apl_RTL_create (ASET_PROJECTS);
 
    const char* format = ui_get_value(ALPHASET_CLASS, RTL_PROJECT_FORMAT);
    if (!format)
        format = "%os";
 
    rtlnode->set_format_spec ("%os");
}
 
