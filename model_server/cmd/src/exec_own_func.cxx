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
#ifndef ISO_CPP_HEADERS
#include <stdlib.h>
#include <assert.h>
#else /* ISO_CPP_HEADERS */
#include <cstdlib>
using namespace std;
#include <cassert>
#endif /* ISO_CPP_HEADERS */
#include "cLibraryFunctions.h"
#include "msg.h"
#include "astTree.h"
#include "genTry.h"
#include "cmd.h"
#include "cmd_interp.h"


/* *** extern "C" void ast_init_buffr (char *); *** */
 extern "C" int run_exec (astFunction_decl *);
/* astFunction_decl* build_full_function_from_str (char *, char *);*/
 astFunction_decl* build_function_decl_C (char *);
 extern "C" void ast_dbg_ret(astTree*, astStmts*, astFunction_decl*);
 extern "C" void ast_dbg_finish (astFunction_decl*);

extern "C" void cmd_set_options();
extern "C" void cmd_reset_options();

extern "C"
{
    void xdisplay_set_running_auto_test();
    void xdisplay_unset_running_auto_test();
}

static int cmd_pause_lev = 0;
int cmd_break_flag = 0;

static int cmd_dbg_balance = 0;

void cmd_abort ()
{
	cmd_stmt_list_ptr->abort();
}

extern "C" int cmd_pause ()
{
  return cmd_exec_pause();
}

int cmd_exec_break()
{
    Initialize(cmd_exec_break);
    ++cmd_break_flag;
    return cmd_break_flag;
}

int cmd_break()
{
    if(! cmd_break_flag) return 0;
    if (!(cmd_stmt_list_ptr && cmd_stmt_list_ptr->has_next_stmt()) ) cmd_break_flag = 0;
    return (!(cmd_stmt_list_ptr && cmd_stmt_list_ptr->has_next_stmt()));
}
extern  void cmd_save_env();
extern void cmd_restore_env();

int cmd_exec_pause()
{
    if(!cmd_execute_journal_name)
	return 0;
    if (cmd_dbg_balance-- != 0)
	return 0;
    
    Initialize(cmd_exec_pause);


    DBG msg(" cmd_pause_lev $1\n") << cmd_pause_lev << eom;
    
    if(cmd_pause_lev == 0)
    	cmd_save_env();   // -rr

    ++cmd_pause_lev;
    return cmd_pause_lev;
}

int cmd_exec_resume()
{
    if(!cmd_execute_journal_name)
	return 0;
    if (cmd_dbg_balance++ != -1)
	return 0;

    Initialize(cmd_exec_resume);
    --cmd_pause_lev;
    DBG msg(" cmd_pause_lev $1\n") << cmd_pause_lev << eom;
    if(cmd_pause_lev < 0) cmd_pause_lev = 0;

    if(cmd_pause_lev == 0)
    	cmd_restore_env();   // -rr

    return cmd_pause_lev;
}

extern "C" void ste_finalize( view* v = NULL );

int cmd_exec_next_stmt()
{
	int ret = 0;
	if (cmd_stmt_list_ptr) {
		cmd_stmt_list_ptr->exec_next_stmt();
		ret = cmd_stmt_list_ptr->has_next_stmt();
	}
	return ret;
}

int cmd_exec_continue()
{
    VALIDATE {
	if(cmd_current_journal_name) {
	    cmd_validate_flush ();
// -rr	    return 0;
	}
	
	if(cmd_pause_lev)
	    return cmd_pause_lev;

	Initialize(cmd_exec_continue);

	genError* err;
	cmd_flush(); 

	cmd_exec_next_stmt();

#if 0 	/* doesn't do anything with yacc regression parser, but should cleanup */
	if(cur_stmt == 0 && func) {
		/* test finished */
	    ast_dbg_finish (func);
	    func = 0;
	    cmd_reset();
	    cmd_reset_options();
//	   clr(cmd_execute_journal_name);
	    xdisplay_unset_running_auto_test();

	    unlink_xterm_file();
	}
#endif

	return cmd_pause_lev;
    } else
	return 0;
}

/*

   START-LOG---------------------------------------------

   $Log: exec_own_func.C  $
   Revision 1.19 2000/12/01 14:06:42EST ktrans 
   More conversions of old messages to msg()
 * Revision 1.31  1994/07/26  15:29:41  mg
 * Bug track: NA
 * fixed compilation error
 *
 * Revision 1.30  1994/05/17  19:10:53  boris
 * Bug track: Scripting
 * Fixed a number of scripting problems
 *
 * Revision 1.29  1994/02/23  14:08:10  mg
 * Bug track: 0
 * validation groups
 *
 * Revision 1.28  1994/02/21  19:23:59  so
 * Bug track: n/a
 * when record script and script is played back, the keyin for the xterm wintow will be
 * input from a file.
 *
 * Revision 1.27  1994/02/16  15:17:16  boris
 * Bug track: Test
 * Move ste_finalize() before real_stmt execution
 *
 * Revision 1.26  1994/02/09  18:04:24  kol
 * this is temporary return to 1.23 version: bug 6338
 *
 * Revision 1.23  1994/01/10  22:17:11  boris
 * Bug track: Test
 * Fixed test_merge1
 *
 * Revision 1.20  1993/12/31  19:36:59  boris
 * Bug track: Test Validation
 * Test Validation
 *
 * Revision 1.19  1993/10/26  23:16:20  boris
 * Bug track: Testing
 * Debugger ParaSET synchronization
 *
 * Revision 1.18  1993/09/22  19:11:04  boris
 * Bug track: NA
 * Fixed script playing SMOD synchronization problem
 *
 * Revision 1.17  1993/08/30  18:54:59  aharlap
 * *** empty log message ***
 *
 * Revision 1.16  1993/08/30  17:04:43  aharlap
 * *** empty log message ***
 *
 * Revision 1.15  1993/07/22  21:43:10  andrea
 * fix scripting bug
 *
 * Revision 1.14  1993/05/21  19:36:15  mg
 * *** empty log message ***
 *
 * Revision 1.13  1993/05/20  20:29:04  mg
 * *** empty log message ***
 *
 * Revision 1.12  1993/05/20  19:10:27  mg
 * suspension
 *
 * Revision 1.11  1993/05/19  01:49:19  boris
 * Cleaned up getting file text
 *
 * Revision 1.10  1993/05/18  15:17:18  builder
 * *** empty log message ***
 *
 * Revision 1.9  1993/05/18  15:05:52  kol
 * properly finish function
 *
 * Revision 1.8  1993/05/17  21:39:44  mg
 * cmd_flush()
 *
 * Revision 1.7  1993/05/17  17:46:37  mg
 * ste_interface
 *
 * Revision 1.6  1993/05/14  17:02:21  kol
 * *** empty log message ***
 *
 * Revision 1.5  1993/05/14  16:54:20  kol
 * *** empty log message ***
 *
 * Revision 1.4  1993/05/14  16:50:34  kol
 * for cmd - call to compile_unit_C
 *
 * Revision 1.3  1993/05/13  15:59:20  mg
 * *** empty log message ***
 *
 * Revision 1.2  1993/05/12  22:32:53  mg
 * exec_next_stmt
 *
 * Revision 1.1  1993/01/25  22:52:28  builder
 * Initial revision
 *
Revision 1.3  1992/12/29  16:10:36  aharlap
*** empty log message ***

Revision 1.2  1992/11/21  19:30:32  builder
Typesafe casts.

Revision 1.1  1992/10/10  21:03:05  builder
Initial revision


   END-LOG-----------------------------------------------

*/


