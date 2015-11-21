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
#ifndef _gdbFSM_states
#define _gdbFSM_states

enum GDB_FSM_states {
    GDB_S = 0,

    GDB_FRAME,
    GDB_FRAME_1,
    GDB_FRAME_2,
    GDB_FRAME_3,
    GDB_FRAME_4,
    GDB_FRAME_ARGS,
    
    GDB_MAYBE_A_LINE,
    GDB_LINE,
    GDB_LINE_NUM,
    GDB_LINE_IGNORE,
    
    GDB_EXITED_1,
    GDB_EXITED_2,
    GDB_EXITED_3,
    GDB_EXITED_4,
    GDB_EXITED_5,

    GDB_RET_1,
    GDB_RET_2,
    GDB_RET_3,
    GDB_RET_4,
    GDB_RET_5,
    GDB_RET_6,
    GDB_RET_7,
    GDB_RET_8,
    GDB_RET_9,
    GDB_RET_10,
    GDB_RET_11,
    GDB_RET_12,
    GDB_RET_13,
    
    GDB_TRAP1,
    GDB_TRAP1_1,
    GDB_TRAP1_2,
    GDB_TRAP1_3,
    GDB_TRAP1_4,
    GDB_TRAP1_5,
    GDB_TRAP1_6,
    GDB_TRAP1_7,
    GDB_TRAP1_8,
    GDB_TRAP1_9,
    GDB_TRAP1_10,
    GDB_TRAP1_11,
    GDB_TRAP1_12,
    GDB_TRAP1_13,
    GDB_TRAP1_14,
    GDB_TRAP1_15,
    GDB_TRAP1_16,
    GDB_TRAP1_17,
    GDB_TRAP1_18,
    GDB_TRAP1_19,
    GDB_TRAP1_20,
    GDB_TRAP1_21,
    GDB_TRAP1_22,
    GDB_TRAP1_23,

    GDB_TRAP1_24,
    GDB_TRAP1_25,
    GDB_TRAP1_26,
    GDB_TRAP1_27,
    GDB_TRAP1_28,
    GDB_TRAP1_29,

    GDB_TRAP0_1,
    GDB_TRAP0_2,
    
    GDB_STATUS_REG,
    GDB_STATUS_REG_1,
    GDB_STATUS_REG_2,
    GDB_STATUS_REG_3,
    GDB_STATUS_REG_4,
    GDB_STATUS_REG_5,
    GDB_STATUS_REG_6,
    GDB_STATUS_REG_7,
    GDB_STATUS_REG_8,
    GDB_STATUS_REG_9,
    GDB_STATUS_REG_10,
    GDB_STATUS_REG_11,
    GDB_STATUS_REG_12,
    GDB_STATUS_REG_13,
    GDB_STATUS_REG_14,
    GDB_STATUS_REG_15,
    GDB_STATUS_REG_16,
    GDB_STATUS_REG_17,
    GDB_STATUS_REG_18,
    GDB_STATUS_REG_19,
    GDB_STATUS_REG_20,

    GDB_STATUS_REG_MX_1, 
    GDB_STATUS_REG_MX_2,
    GDB_STATUS_REG_MX_3,

    GDB_STATUS_REG_21,
    GDB_STATUS_REG_22,
    GDB_STATUS_REG_23,
    
    GDB_CHILD_PID,
    GDB_CHILD_PID_1,
    GDB_CHILD_PID_2,
    GDB_CHILD_PID_3,
    GDB_CHILD_PID_4,
    GDB_CHILD_PID_5,
    GDB_CHILD_PID_6,
    GDB_CHILD_PID_7,
    GDB_CHILD_PID_8,
    GDB_CHILD_PID_9,
    GDB_CHILD_PID_10,
    GDB_CHILD_PID_11,
    GDB_CHILD_PID_12,
    GDB_CHILD_PID_13,
    
    GDB_IGNORE,
    GDB_IGNORE_PROMPT_1,
    GDB_IGNORE_PROMPT_2,
    GDB_IGNORE_PROMPT_3,
    GDB_IGNORE_PROMPT_4,
    GDB_IGNORE_PROMPT_5,
    
    GDB_PROMPT_1,
    GDB_PROMPT_2,
    GDB_PROMPT_3,
    GDB_PROMPT_4,
    GDB_PROMPT_5
};
#endif
/*
    START-LOG-------------------------------

    $Log: gdbFSM_states.h  $
    Revision 1.1 1994/03/01 15:22:20EST builder 
    made from unix file
 * Revision 1.3  1994/03/01  15:00:08  pero
 * (renaming "functions" and "data" for mixed library; making parsing of
 * new names available in gdb::input)
 *
 * Revision 1.2  1994/02/08  21:28:29  pero
 * Changes that accomodate the way GDB 4.11 reports a signal
 * that has been received by the program
 *
 * Revision 1.1  1994/01/26  15:04:04  pero
 * Initial revision
 *

*/
