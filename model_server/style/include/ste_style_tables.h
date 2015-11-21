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
#ifndef ste_style_table_h
#define ste_style_table_h

#include "objOper.h"

#define UNUSED (steNodeType)-1

enum {
  STE_DEFAULT_WORD = 0,
  STE_STRUCTURED_GUIES,
  STE_FUNCTION_DEFINITION = STE_STRUCTURED_GUIES,   // 1
  STE_FUNCTION_DECLARATION,                     // 2
  STE_STATEMENT,                                // 3
  STE_EXPRESSION,                               // 4
  STE_TYPE,                                     // 5
  STE_OPEN_BLOCK_PAREN,                         // 6
  STE_CLOSE_BLOCK_PAREN,                        // 7
  STE_UNSTRUCTURED_GUIES,
  STE_KEYWORD =  STE_UNSTRUCTURED_GUIES,
  STE_OPERATION,
  STE_FUNCTION_NAME,
  STE_VARIABLE,
  STE_DELEMETER,
  STE_OPEN_PAREN,
  STE_CLOSE_PAREN,
  STE_CLOSE_STATEMENT
};

steNodeType translate_table[ STE_UNSTRUCTURED_GUIES ] =  {
  UNUSED,
  UNUSED,
  stePAR,              // DECLAR 2
  stePAR,              // STAT   3
  steREG,              // EXPRESS 4
  steREG,              // TYPE
  steHEAD,
  steFOOT
};

#endif

/*
   START-LOG-------------------------------------------

   $Log: ste_style_tables.h  $
   Revision 1.1 1993/07/28 18:35:24EDT builder 
   made from unix file
 * Revision 1.2  1992/12/18  19:10:48  glenn
 * Transfer from STE
 *
 * Revision 1.2.1.2  92/10/09  20:01:51  boris
 * Fix comment
 * 


   END-LOG---------------------------------------------

*/
