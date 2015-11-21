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
// cmd_gen.C
//
// Functions to generate Command Journal records
//
// History:  08/14/91    M.Furman   Initial coding for cmd_gen()
//           08/19/91    M.Furman   Clean up
//           08/20/91    M.Furman   cmd_level, cmd_begin, cmd_end

#ifndef ISO_CPP_HEADERS
#include <iostream.h>
#include <aset_stdarg.h>
#else /* ISO_CPP_HEADERS */
#include <iostream>
using namespace std;
#include <cstdarg>
#endif /* ISO_CPP_HEADERS */
#include <genError.h>
#include "cmd_internal.h"
#include <driver.h>
#include <graWorld.h>

// extern "C" unsigned sleep(unsigned);

static int cmd_lev = 0;

void cmd_reset()
{
    if(cmd_current_journal_name){
       delete cmd_current_journal_name;
       cmd_current_journal_name = 0;
    }
    if(cmd_execute_journal_name){
       delete cmd_execute_journal_name;
       cmd_execute_journal_name = 0;
    }
    cmd_lev = 0;
}

int cmd_write() 
{
    return(cmd_current_journal_name && (cmd_lev == 1));
//    return(cmd_current_journal_name && (cmd_lev >= 1));
}

// Test level of command journal (0 if level > 0)
int cmd_level()
    {
    return (cmd_lev);
    }

// Begin of write cmd operation (0 if level was > 0)
int cmd_begin()
    {
 if(!cmd_current_journal_name) return 0;
 Initialize(cmd_begin);
//    IF(cmd_lev < 0);
    DBG cout << " level " << cmd_lev << endl;
    cmd_lev++;
    return cmd_write();
    }

void cmd_flush()
    {
   driver_instance->graphics_world->process_pending_events();
//    sleep(1);
   driver_instance->flush_events();
    }


// End of write cmd operation
void cmd_end()
{
 if(!cmd_current_journal_name) return;
 Initialize(cmd_end);
    cmd_lev--;
    if(cmd_lev < 0) cmd_lev = 0;
    DBG cout << " level " << cmd_lev << endl;
}

extern int cmd_is_no_epoch_macro ();

void cmd_gen_write_force (const char *fmt, ...)
{
    Initialize(cmd_gen_write_force);
    if (cmd_is_no_epoch_macro ()) {
	va_list args;
	va_start (args, fmt);
	cmd_gen_real_write (fmt, args);
	va_end(args);
    }
}

// Generate memory Command Journal element
void cmd_gen(const char *fmt, ...)
{
    Initialize(cmd_gen);

    if(! cmd_write()){
       DBG  cout << endl << " NOT RECORDING " << fmt << endl;
       return;
    }

    if (cmd_is_no_epoch_macro ()) {
	va_list args;

	va_start(args, fmt);
	cmd_gen_real_write (fmt, args);
	va_end(args);
    }
}

/*
   START-LOG-------------------------------------------

   $Log: cmd_gen.cxx  $
   Revision 1.4 2001/07/25 20:48:06EDT Sudha Kallem (sudha) 
   Changes to support model_server.
 * Revision 1.20  1994/07/21  17:46:33  builder
 * proper iostream.h and cmd_internal.h included.
 *
 * Revision 1.19  1994/07/20  20:16:27  mg
 * Bug track: NA
 * restructured for logging
 *
 * Revision 1.18  1994/04/13  13:53:36  builder
 * Port
 *
 * Revision 1.17  1994/04/04  18:39:41  boris
 * Added epoch macro scripting functionality
 *
 * Revision 1.16  1994/03/06  16:29:38  mg
 * Bug track: 0
 * validation groups
 *
 * Revision 1.15  1993/11/24  02:00:20  boris
 * Bug track: #5311
 * Fixed "Scripting broken" problem
 *
 * Revision 1.14  1993/11/19  01:45:10  boris
 * Bug track: ParaSET window buffer
 * Window buffer towards Emacs behavior
 *

   END-LOG---------------------------------------------
*/
