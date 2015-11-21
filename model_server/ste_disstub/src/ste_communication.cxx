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
////////////////////   FILE ste_communication.C  //////////////////////
//
// Containes functions and variables for STE-EPOCH communication.

#include <charstream.h>
#include <genError.h>
#include <objArr_Int.h>

objArr_Int ste_button_arr;       // temporary integer array for buttons
objArr_Int ste_style_arr;        // temporary integer array for styles
objArr_Int ste_action_arr;       // temporary integer array for actions
objArr_Int ste_format_arr;       // temporary integer array for format

objArr_Int steParArray;          // const Epoch parameter array

ocharstream ste_temp_str;        // temporary stream for insertion string.
int        ste_temp_index = 0;   // relative point for creating buttons.
int        epoch_parent_flag = 0;// Epoch parent flag;

int ste_transaction_flag = 0;  // Start transaction STE View flag;

static boolean EdtTimedOut = false;
boolean edt_timed_out( boolean tf) {    return EdtTimedOut = tf;  }
boolean edt_timed_out()            {    return EdtTimedOut;  }

// ste_temp_arr - integer array that contains integers for whole format or
// insertion portion. For insertion Marks are relative and consider that
// start of a first button = 0; For formatting array containes portions 
// integers, each portion is encoding format command parameters.

// This routine returns string with all integers from ste_temp_array, delemeted
// by spaces. It assumes that each integer has NO MORE than 10 digits.

/**********
* ste_prepare_array_string()
**********/
char *ste_prepare_array_string(int){
    return 0;
}

void edt_free_editor_shmem()
{
}

/**********
* ste_prepare_array_segment()
**********/
void *ste_prepare_array_segment(int *, int){
    return 0;
}

// Tis routine fills out block of shared memory with integers of 
// ste_temp_array and returns memory address and integer size of the block.
// Each time when it starts it frees previous shared memory block and 
// allocates new one with nessesary size and remembers returned address in
// the  variable old_memseg for future cleaning;


void edt_set_ceiling_on_shmem_queue()
{
}

char *ste_str_to_shmem (char const *, int)
{
    return 0;
}

////////
// Descr: Get N characters from editor
//
// Notes: This routine allocates the buffer for the data.  It is up to the caller
//        to clean this data up.
//        qid == -1 by default.  This means get data not an answer.
////////
int ste_get_from_epoch(unsigned char *&, int, int)
{
    return -1;
}

int ste_get_from_epoch( objArr_Int&, int, int )
{
    return 0;
}

int ste_get_from_epoch(int, int){
    return 0;
}

////////
//  Descr:   This static var and two methods set and unset the editor question id.
//  Returns: The command id of most recent paraset command.
//  Notes:   Id`s must always (for validation reasons) be positive.
////////
static int EdtLastQuestionIdOut = 0;
 
int edt_new_question_id_out()
{
    if( ++EdtLastQuestionIdOut < 0 ) EdtLastQuestionIdOut = 0;
    return EdtLastQuestionIdOut;
}
 
int edt_last_question_id_out()
{
    return EdtLastQuestionIdOut;
}

////////
//          The data in inserted in an array of ints SteParArray[].
//
//  Returns: 0 = Success, -1 on error.
//
////////
int edt_get_request()
{
    return -1;
}

////////
//  Descr:  This routine reads a command/report/ping (request) from from emacs.  
//          The data in inserted in an array of ints SteParArray[].
//
//  Returns: 1 if answer, 0 if request, -1 on error.
//
////////
int edt_get_request_or_answer( int [], int, int )
{
    return -1;
}

////////
//  Descr:  This routine reads an answer from editor pipe.
//
//  Returns: 0 on succesful read, -1 on error.
//
////////
int edt_get_answer( int *, int, int )
{
    return -1;
}
