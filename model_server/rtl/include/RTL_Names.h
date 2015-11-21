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
#ifndef _RTL_Names_h
#define _RTL_Names_h

/*
 * Keep this file usable for "C"
 */

/*
 * For each of the following names a system rtl is
 * created in RTL_init.C.
 */

#define ASET_DOCUMENTS 		"Alpha Set Documents"
#define ASET_FUNCTIONS 		"Alpha Set Functions"
#define ASET_CATEGORIES 	"Alpha Set Categories"
#define ASET_FORMATS 		"Alpha Set Formats"
#define ASET_STYLES		"Alpha Set Styles"
#define ASET_VARIABLES		"Alpha Set Variables"
#define ASET_LISTS		"Alpha Set Lists"
#define ASET_USER_CATEGORIES	"User Categories"
#define ASET_CLASSES		"Alpha Set Classes"
#define ASET_PROGRAM_FILE_NAMES	"Alpha Set Program File Names"
#define ASET_OODT_RELATIONS	"Alpha Set OODT Relations"
#define ASET_SCRAPBOOKS         "Alpha Set Scrapbooks"
#define ASET_SUBSYSTEMS		"Alpha Set Subsystems"
#define ASET_PROJECTS		"Alpha Set Projects"
#define ASET_SOFTASSOC		"Soft Association"
#define ASET_SOFTASSOC_NOTIFY	"Touched Soft Association"
#define FUNCTION_HOTSPOT	"Functions to HotSpot"
#define ASET_USER_TYPE          "Alpha Set User Defined Types"

/*
 * The search criteria constants are defined here
 */
typedef enum
{
   Filter_Undefined = 1,
   Filter_Name,
   Filter_Title,
   Filter_Data,
   Filter_Value
} FilterDomainType;
   
#endif /* _RTL_Names_h */


