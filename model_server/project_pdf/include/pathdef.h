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
/*
   file  pathdef.h
   define enums used for projMap
*/

#ifndef _pathdef_h
#define _pathdef_h

enum PATH_RW_FLAG
{
    NO_RW_FLAG = 0,
    R_FLAG,
    W_FLAG
};

/* The next enum will be used by YACC and LEXER */
enum
{
    NO_IMPLICIT_DIR_PROJ,
    IMPLICIT_DIR_PROJ
};

/* The next enum will be used by YACC and LEXER */
enum ARROW_WAY
{
    LEFT_WAY,
    RIGHT_WAY,
    TWO_WAY,
    REAL_RIGHT_WAY
};

/* the next enum is used by selector_type in the data structure selector_entry. */
enum SELECTOR_TYPE
{
    PURE_FILE_TYPE,
    LINK_MAP_FILE_TYPE,
    REALPATH_MAP_FILE_TYPE,
    SEARCH_FILE_TYPE,
    SCRIPT_TYPE,
    FILTER_TYPE,
    TCL_BOOLEAN_TYPE,
    EQUAL_TYPE,
    NOT_EQUAL_TYPE,
    /* boris 980421 */
    PMOD_TYPE,
    INCLUDED_TYPE,

    EMPTY_TYPE
};

enum PMOD_FLAG_KIND
{
    PMOD_UNKNOWN    = 0,
    PMOD_PDF        = 1,
    PMOD_MODELBUILD = 2,
    PMOD_HOME       = 4,
    PMOD_NO         = 8
};

enum
{
    FILE_UNKNOWN,
    FILE_INCLUDED
};
#endif

/*
$Log: pathdef.h  $
Revision 1.3 1998/05/01 09:55:08EDT boris 
PDF  selectors [[ pmod ]] and [[ i ]]. reviewed by: mg test log: n/a Ran and deleted. Fixed the broken scripts
*/
