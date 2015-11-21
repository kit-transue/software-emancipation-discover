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
#ifndef _ste_categ_enum_h
#define _ste_categ_enum_h
 
enum ste_categ_no {
   ste_categ_Equates,
   ste_categ_Nouns,
   ste_categ_User_Category_1,
   ste_categ_User_Category_2,
   ste_categ_User_Category_3,
   ste_categ_Execution_Point,
   ste_categ_Break_Point,
   ste_categ_Disabled_Break_Point,
   ste_categ_Change_Propagator,
   ste_categ_Error_Note,
   ste_categ_Search_Hit,
   ste_categ_Contains_Error_Note,
   ste_categ_Contains_Search_Hit,
   ste_categ_Suspended_Parse,
   ste_categ_Grouping,
   ste_categ_Hypertext_Link,
   ste_categ_Index_Hit,
   ste_categ_Note,
   ste_categ_Contains_Note,
    
   ste_categ_first_storage_class,
    
   ste_categ_File_Static = ste_categ_first_storage_class,
   ste_categ_Local_Static,
   ste_categ_Local_Auto,
   ste_categ_Global,
   ste_categ_Member,
   ste_categ_Parameter,
   ste_categ_Keyword,
   ste_categ_Function,
    
   ste_categ_last_storage_class = ste_categ_Function,
    
   ste_categ_first_data_type,
    
   ste_categ_char = ste_categ_first_data_type,
   ste_categ_short,
   ste_categ_int,
   ste_categ_long,
   ste_categ_charptr,
   ste_categ_float,
   ste_categ_double,
   ste_categ_pointer,
   ste_categ_reference,
   ste_categ_array,
   ste_categ_unknown,
    
   ste_categ_last_data_type = ste_categ_unknown,

   ste_categ_Reference_Source,
   ste_categ_Reference_Target
};
#endif

/*
   START-LOG-------------------------------------------

   $Log: ste_categ_enum.h  $
   Revision 1.1 1993/07/29 10:44:32EDT builder 
   made from unix file
 * Revision 1.3  1993/01/06  20:00:14  jon
 * Added catagories for references to enum list
 *
 * Revision 1.2  1992/12/18  19:10:48  glenn
 * Transfer from STE
 *
 * Revision 1.2.1.3  1992/12/09  23:35:24  smit
 * add enum for new category.
 *
 * Revision 1.2.1.2  1992/10/09  20:01:22  boris
 * Fix comment
 *


   END-LOG---------------------------------------------

*/
