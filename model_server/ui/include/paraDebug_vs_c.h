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
/* --------- C style include file ----------- */
/* intended to be used for .if files, and other C written support */
/* (06/24/1994) */

#ifndef _paraDebug_vs_c_h
#define _paraDebug_vs_c_h

extern void   ext_change_debug_buttons();
extern void   ext_debug_select_one();
extern void   viewer_debug_mode();
extern int    viewerShell_dbgDefault();
extern char * viewerShell_dbgName();

/* ----------------------------------- */
/* viewerShell debugger menu callbacks */
/* ----------------------------------- */

/* session setup callbacks */
/* ----------------------- */
extern void viewerShell_setProgram();
extern void viewerShell_setArguments();
extern void viewerShell_attachProcess();
extern void viewerShell_runProgram();
extern void viewerShell_dbgSelection();
extern void viewerShell_sessionsDisplay();

/* router callbacks */
/* ---------------- */
extern void viewerShell_showInterpreted();
extern void viewerShell_interpretSelected();
extern void viewerShell_interpretFileInView();
extern void viewerShell_interpretModified();

/* breakpoints submenu callbacks */
/* ----------------------------- */
extern void viewerShell_breakAt();
extern void viewerShell_breakInfo();
extern void viewerShell_deleteBreakAll();
extern void viewerShell_deleteBreakAt();
extern void viewerShell_enableBreakAt();
extern void viewerShell_disableBreakAt();

/* start/quit session, kill process */
/* -------------------------------- */
extern void viewerShell_killProcess();
extern void viewerShell_dbgSession();

/* --------------------------------- */
/* viewer debugger buttons callbacks */
/* --------------------------------- */

extern void viewer_button_STEP();
extern void viewer_button_NEXT();
extern void viewer_button_CONT();
extern void viewer_button_CONT_FROM();
extern void viewer_button_FINISH();
extern void viewer_button_FRAME_UP();
extern void viewer_button_FRAME_DOWN();
extern void viewer_button_STACK_INFO();
extern void viewer_button_INTERRUPT();
extern void viewer_button_BREAK_AT();
extern void viewer_button_EVAL_DISPLAY();

#endif

/*
   START-LOG-------------------------------------------

   $Log: paraDebug_vs_c.h  $
   Revision 1.1 1994/09/27 19:45:12EDT pero 
   ParaDEBUG MDMS Project

   END-LOG---------------------------------------------
*/
