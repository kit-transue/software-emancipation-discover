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
#ifndef _smode_categories_h
#define _smode_categories_h

// smode_categories
//------------------------------------------
// synopsis:
// Constants defining the default S-mode categories
//
// description:
// Identifiers in S-mode text are displayed with different attributes
// depending on their data type and storage class.  This file declares
// constants for all of the built-in categories into which identifiers
// are placed.
//------------------------------------------
//------------------------------------------
// Restrictions:
// ...
//------------------------------------------

enum smode_storage_class {
   SMODE_FILE_STATIC,
   SMODE_LOCAL_STATIC,
   SMODE_LOCAL_AUTO,
   SMODE_GLOBAL,
   SMODE_MEMBER,
   SMODE_PARAMETER,

// The following two are not "storage classes" per se, but they function
// in approximately the same fashion:

   SMODE_KEYWORD,
   SMODE_FUNCTION
};

enum smode_datatype {
   SMODE_CHAR,
   SMODE_SHORT,
   SMODE_INT,
   SMODE_LONG,
   SMODE_STRING,
   SMODE_FLOAT,
   SMODE_DOUBLE,
   SMODE_USER_TYPE,
   SMODE_POINTER,
   SMODE_REFERENCE,
   SMODE_ARRAY,
   SMODE_UNKNOWN_TYPE
};

#endif // _smode_categories_h

/*
   START-LOG-------------------------------------------

   $Log: smode_categories.h  $
   Revision 1.1 1993/07/28 18:35:23EDT builder 
   made from unix file
 * Revision 1.2  1992/12/18  19:10:48  glenn
 * Transfer from STE
 *
 * Revision 1.2.1.2  92/10/09  20:00:36  boris
 * Fix comment
 * 


   END-LOG---------------------------------------------

*/
