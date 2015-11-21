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
////////////////////   FILE ste_communication.h  //////////////////////
//
// Containes functions and variables for STE-EPOCH communication.
//------------------------------------------
#ifndef ste_communication_h
#define ste_communication_h

#include "objArr_Int.h"
#include <charstream.h>

enum {
    For_Insert,
    For_Format,
    For_Style,
    For_Action
};

extern char *ste_prepare_array_string(int flag);
extern void *ste_prepare_array_segment(int *seg_size, int flag);
extern char *ste_str_to_shmem (char const *str, int len);

extern int ste_get_from_epoch(unsigned char *&,int lim, int qid = -1);
extern int ste_get_from_epoch( objArr_Int& buf, int lim, int qid = -1);
extern int ste_get_from_epoch(int lim = 1,  int qid = -1);

extern objArr_Int ste_button_arr;
extern objArr_Int ste_format_arr;
extern objArr_Int ste_style_arr;
extern objArr_Int ste_action_arr;

extern objArr_Int steParArray;;

extern ocharstream ste_temp_str;

extern int        ste_memseg_flag;
extern int        ste_temp_index;
extern int        epoch_parent_flag;
extern int        ste_transaction_flag;

#define EPOCH_MAX_PAR    13     // steParArray max size;
#define EPOCH_PAR_CONST -913    // start cont for filling steParArray
#define StartStringChar  1      // start char in string from epoch

#endif

/*
   START-LOG-------------------------------------------

   $Log: ste_communication.h  $
   Revision 1.6 2000/07/10 23:10:00EDT ktrans 
   mainline merge from Visual C++ 6/ISO (extensionless) standard header files
 * Revision 1.2.1.5  1993/04/12  20:32:25  boris
 * Fixed bug #3282. Added StartStringChar at the each epoch-parset string begin
 *
 * Revision 1.2.1.4  1993/04/09  01:11:11  boris
 * Fixed bug #3188. Changed Epoch <-> ParaSET interfcae
 *
 * Revision 1.2.1.3  1992/11/20  19:39:17  boris
 * Added Decorate Source facilities
 *
 * Revision 1.2.1.2  1992/10/09  20:01:26  boris
 * Fix comment
 *


   END-LOG---------------------------------------------

*/
