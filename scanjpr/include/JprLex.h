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
/*---------------------------------------------------------------------------*/
/* File:   JprLex.h                                                          */
/* Descr.: Lexical analyzer for JBuilder Project (.jpr) files                */
/*                                                                           */
/* 09-14-00    Guillermo    $$1   Created                                    */
/* 09-21-00    Guillermo    $$2   Added a couple _jpr_set.. functions        */
/*---------------------------------------------------------------------------*/

#ifndef JPR_LEX_H
#define JPR_LEX_H

#include <stdlib.h>
#include <string.h>

#define TOKEN_IDENT           10
#define TOKEN_NUMBER          11
#define TOKEN_STRING          12

#define TOKEN_ASSIGN          20
#define TOKEN_DOT             21
#define TOKEN_OPENSQB         22
#define TOKEN_CLOSESQB        23
#define TOKEN_POUND           24

#define TOKEN_EOS            -10
#define TOKEN_EOF            -11
#define TOKEN_UNKNOWN        -12
#define TOKEN_INVALID_INPUT  -20

short _jpr_get_token( char  *in_ptr, 
                      char **out_ptr,
                      char **out_info,
                      long  *out_info_size );

bool _jpr_set_treat_as_string_to_eol( bool flag );
bool _jpr_set_pound_starts_comment( bool flag );
bool _jpr_set_allow_dots_in_idents( bool flag );

#ifdef WIN32

#define strcasecmp _stricmp

#endif /*WIN32*/


static void XStrConcat( char       **trg_string,
                        long        *trg_string_size,
                        const char  *src_string )
{
    long len;
    long srclen;
 
    if( trg_string == NULL || trg_string_size == NULL || src_string == NULL )
        return;
 
    if( *trg_string != NULL )
        len = strlen( *trg_string );
    else
        len = 0;
 
    srclen = strlen( src_string );
 
    if( srclen + len >= *trg_string_size )
    {
        char     *new_trg_string;
        unsigned  new_trg_string_size;
 
        /*
         * Need to reallocate
         */
        new_trg_string_size = ((srclen + len) / 64 + 1) * 64;
        new_trg_string      = new char[new_trg_string_size];

        if( *trg_string != NULL )
        {
            memcpy( (void*)new_trg_string, (void*)*trg_string, *trg_string_size);
 
            delete [] (*trg_string);
        }
        else
            new_trg_string[0] = (char)0;
 
        *trg_string      = new_trg_string;
        *trg_string_size = new_trg_string_size;
    }
 
    strcat( *trg_string, src_string );
}


#endif /*JPR_LEX_H*/
