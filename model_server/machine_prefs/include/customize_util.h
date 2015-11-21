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
#ifndef _customize_util_h
#define _customize_util_h

// customize_util.h
//------------------------------------------
// synopsis:
// Simple access to customization functions.
//
//------------------------------------------

#ifndef _representation_h
#include <representation.h>
#endif


// Return true if filename is underneath an excluded directory.
//int xref_is_excluded_file(const char* filename);

// Attempt to guess the language of the file based on the suffix.
fileLanguage guess_file_language(const char* filename);

// Return whether file name is a c or c++ header file based on suffix.
bool is_c_hdr_suffix(const char *full_path);
bool is_cplusplus_hdr_suffix(const char *full_path);

#endif // _customize_util_h

/*
    START-LOG-------------------------------

    $Log: customize_util.h  $
    Revision 1.2 1994/12/12 09:11:42EST pgood 
    Bug track: 8914
    Added two new fields to prefs file:  cHdrSuffix, cPlusHdrSuffix.   These two fields contian the valid header file extentions, and facilitate the use
    of unusually named header files (.hxx, .hpp, .def, etc).
 * Revision 1.2.1.2  1992/10/09  19:56:41  kws
 * Fix comments
 *

    END-LOG---------------------------------
*/

