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
#ifndef _gdbFSM_actions
#define _gdbFSM_actions

enum GDB_FSM_actions {

    GDB_NOP = 0,

    GDB_PUT,
    GDB_UNPUT,

    GDB_COLLECT_STR,
    GDB_COLLECT_STR_INC,
    GDB_COLLECT_STR_DEC,

    GDB_LINE_ASSIGN_STR,
    GDB_LINE_ASSIGN_NUM,
    
    GDB_EXIT_DETACH,
    GDB_ASSIGN_CHILD_PID,

    GDB_GET_RETURN_VAL,

    GDB_PEEK_FRAME,
    GDB_FRAME_ASSIGN_NUM,
    GDB_FRAME_ASSIGN_FN_PUT,
    GDB_FRAME_ASSIGN_FA_PUT,

    GDB_PROCESS_STATUS_REG,

    GDB_MIXED_TRAPPED,
    
    GDB_INTERPRETER_STOPPED,
    
    GDB_STORE_PROMPT,
    GDB_NOSTORE_PROMPT
};
#endif
/*
    START-LOG-------------------------------

    $Log: gdbFSM_actions.h  $
    Revision 1.1 1994/01/26 10:26:17EST builder 
    made from unix file
 * Revision 1.1  1994/01/26  15:03:01  pero
 * Initial revision
 *

*/
