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
#define BAD_INSERTION_POINT 33
#define NOT_CHILD 55
#define NOT_SYMBOL 66
#define NO_CONNECTION 77
#define TOO_MANY_CONNECTIONS 88
#define NO_TEMPLATE 99
#define NULL_THEN_CLAUSE 110
#define NULL_ELSE_CLAUSE 121
#define NULL_CONDITION 132
#define NULL_LOOP_COND 143
#define NULL_LOOP_INIT 154
#define NULL_LOOP_STEP 165
#define NULL_LOOP_BODY 176
#define NO_ENTRY_OR_EXIT 187
#define MISSING_LDR 198
#define CANNOT_DISCONNECT 209
#define TOO_MANY_APP_NODES 220
#define BAD_LDR_SELECTION  231

extern void print_ldr_error(genError*);

/*
   START-LOG-------------------------------------------

   $Log: ldrError.h  $
   Revision 1.1 1993/07/28 19:50:19EDT builder 
   made from unix file
 * Revision 1.2.1.2  1992/10/09  19:50:17  jon
 * RCS History Marker Fixup
 *


   END-LOG---------------------------------------------

*/
