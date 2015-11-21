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
#ifndef _pdf_parser_h
#define _pdf_parser_h
#define YY_CHAR char

struct flex_state_table
{

 long  * yy_acclist;
 long  * yy_accept;
 YY_CHAR * yy_ec;
 YY_CHAR * yy_meta;
 long  * yy_base;
 long  * yy_def;
 long  * yy_nxt;
 long  * yy_chk;


 long  yy_acclist_len;
 long  yy_accept_len;
 long  yy_ec_len;
 long  yy_meta_len;
 long  yy_base_len;
 long  yy_def_len;
 long  yy_nxt_len;
 long  yy_chk_len;

 long  yy_END_OF_BUFFER;
 long  yy_NO_STATES;
};

void pdf_prt_state();

#endif

/*
$Log: pdf_parser.h  $
Revision 1.2 1994/09/09 22:53:08EDT aharlap 
short => long
 * Revision 1.1  1994/05/23  20:16:33  so
 * Initial revision
 *
*/
