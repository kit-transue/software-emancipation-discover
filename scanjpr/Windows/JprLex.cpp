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
/* File:   JprLex.cpp                                                        */
/* Descr.: Lexical analyzer for JBuilder Project (.jpr) files                */
/*                                                                           */
/* 09-14-00    Guillermo    $$1   Created                                    */
/* 09-19-00    Guillermo    $$2   Handle the weird "%|" sequence that appears*/
/*                                in project file strings sometimes          */
/* 09-21-00    Guillermo    $$3   Handle a few more weird situations that    */
/*                                happen only within JBuilder Property files */
/*---------------------------------------------------------------------------*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "JprLex.h"

static bool s_treat_as_string_to_eol = false;
static bool s_pound_starts_comment   = false;
static bool s_allow_dots_in_idents   = false;

bool _jpr_set_treat_as_string_to_eol( bool flag )
{
	bool prev_value = s_treat_as_string_to_eol;
    s_treat_as_string_to_eol = flag;
	return prev_value;
}

bool _jpr_set_pound_starts_comment( bool flag )
{
	bool prev_value = s_pound_starts_comment;
    s_pound_starts_comment = flag;
	return prev_value;
}

bool _jpr_set_allow_dots_in_idents( bool flag )
{
	bool prev_value = s_allow_dots_in_idents;
    s_allow_dots_in_idents = flag;
	return prev_value;
}

short _jpr_get_token( char  *in_ptr, 
                      char **out_ptr,
                      char **out_info,
                      long  *out_info_size )
{
    char     *ptr, *ptr1, c;
    short     token;

    if( in_ptr == NULL || out_ptr == NULL )
        return TOKEN_INVALID_INPUT;

    if( out_info != NULL && out_info_size != NULL )
    {
        if( *out_info != NULL )
            *out_info[0] = (char)0;
    }

    ptr = in_ptr;

    if( s_treat_as_string_to_eol )
    {            
        ptr1 = ptr;
        while( *ptr != (char)0 && *ptr != '\n' )
        {
            if( *ptr == '%' )
            {
		short len = 1;
		if( *(ptr+len) == '%' ) len++;
                if( *(ptr+len) == '|' )
                {              
		    len++;
                    if( out_info != NULL )
                    {
                        *ptr = (char)0;
                        XStrConcat( out_info, out_info_size, ptr1 );
                        XStrConcat( out_info, out_info_size, ":" );
                        *ptr = '%';
                    }                    
                    ptr  += len;
                    ptr1  = ptr;
                }
                else
                    ptr++;
            }
			else {
				if( *ptr == '\\' )
				{
					if( *(ptr+1) == ' ' )
					{                    
						if( out_info != NULL )
						{
							*ptr = (char)0;
							XStrConcat( out_info, out_info_size, ptr1 );
							XStrConcat( out_info, out_info_size, " " );
							*ptr = '\\';
						}                    
						ptr  += 2;
						ptr1  = ptr;
					}
					else
						ptr++;
				}
				else 
					ptr++;
			}
        }

        c = *ptr;

        if( out_info != NULL )
        {            
            *ptr = (char)0;
            XStrConcat( out_info, out_info_size, ptr1 );
            *ptr = c;
        }

        if( c == (char)0 )
            *out_ptr = ptr;
        else
            *out_ptr = ptr + 1;

        token = TOKEN_STRING;
    }
    else
    {        
        while( *ptr == ' ' || *ptr == '\t' || *ptr == '\n' )
            ptr++;

        c = toupper( *ptr );
        if( c >= 'A' && c <= 'Z' )
        {
            ptr1 = ptr;
            ptr++;
            c = toupper( *ptr );
            while( (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_' || 
                   (c == '\\' && *(ptr+1) != (char)0) || c == '"' ||
                   (s_allow_dots_in_idents && c == '.') )
            {
                if( c == '\\' )
                {
                    if( out_info != NULL )
                    {
                        *ptr = (char)0;
                        XStrConcat( out_info, out_info_size, ptr1 );
                        *ptr = '\\';
                    }
                    ptr1 = ptr + 1;
                    ptr += 2;
                }
                else if( c == '"' )
                {
                    ptr++;

                    c = *ptr;
                    while( c != (char)0 && c != '"' && c != '\n' )
                    {
                        if( c == '\\' && *(ptr+1) != (char)0 )
                        {
                            if( out_info != NULL )
                            {
                                *ptr = (char)0;
                                XStrConcat( out_info, out_info_size, ptr1 );
                                *ptr = '\\';
                            }
                            ptr1 = ptr + 1;
                            ptr += 2;
                        }
                        else
                            ptr++;

                        c = *ptr;
                    }

                    if( *ptr != (char)0 )
                        ptr++;

                    if( out_info != NULL )
                    {
                        c = *ptr;
                        *ptr = (char)0;
                        XStrConcat( out_info, out_info_size, ptr1 );
                        *ptr = c;
                    }

                    ptr1 = ptr;
                }                    
                else
                    ptr++;

                c = toupper( *ptr );
            }

            c = *ptr;
            *ptr = (char)0;
            token = TOKEN_IDENT;

            if( out_info != NULL )
                XStrConcat( out_info, out_info_size, ptr1 );

            *ptr = c;

            *out_ptr = ptr;
        } 
        else if( (c >= '0' && c <= '9') || 
                 (c == '-' && *(ptr+1) >= '0' && *(ptr+1) <= '9') )
        {
            ptr1 = ptr;
            ptr++;
            while( *ptr >= '0' && *ptr <= '9' )
                ptr++;

            if( out_info != NULL )
            {
                c = *ptr;
                *ptr = (char)0;
                XStrConcat( out_info, out_info_size, ptr1 );
                *ptr = c;
            }

            *out_ptr = ptr;
            token = TOKEN_NUMBER;
        }    
        else
        {
            if( c == '#' && s_pound_starts_comment )
                c = ';';

            switch( c )
            {
                case '=':
                    token = TOKEN_ASSIGN;
                    *out_ptr = ptr + 1;
                    break;
                case '.':
                    token = TOKEN_DOT;
                    *out_ptr = ptr + 1;
                    break;
                case '[':
                    token = TOKEN_OPENSQB;
                    *out_ptr = ptr + 1;
                    break;
                case ']':
                    token = TOKEN_CLOSESQB;
                    *out_ptr = ptr + 1;
                    break;
                case '#':
                    token = TOKEN_POUND;
                    *out_ptr = ptr + 1;
                    break;
                case (char)0:
                    token = TOKEN_EOS;
                    *out_ptr = ptr;
                    break;
                case ';':
                    /* comment to end of line */
                    ptr++;
                    while( *ptr != '\0' && *ptr != '\n' )
                        ptr++;

                    if( *ptr == '\n' )
                        ptr++;

                    if( *ptr == '\0' )
                    {
                        token = TOKEN_EOS;                
                        *out_ptr = ptr;
                    }
                    else 
                        token = _jpr_get_token( ptr, out_ptr, out_info, 
                                                out_info_size );
                    break;
                default:
                    token = TOKEN_UNKNOWN;
                    *out_ptr = ptr + 1;
                    break;
            }
        }
    }
    return token;
}

