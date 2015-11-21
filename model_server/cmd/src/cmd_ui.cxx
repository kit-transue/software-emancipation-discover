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
// file cmd_ui.C
// 
// Group of user interface functions for cmd
//
// History:  09/04/91    M.Furman   Initial coding.
//           10/01/91    M.Furman   change to connect to UI
#include <cLibraryFunctions.h>
#include "genError.h"
#include "cmd_internal.h"
#include "driver.h"
#include "graWorld.h"
#include <machdep.h>
#include <customize.h>

#include <cmd_enums.h>
#include <genString.h>
#include <ste_communication.h>
int ste_epoch_evaluate (genString& form);

#ifndef _genTmpfile_h
#include <genTmpfile.h>
#endif
#include <proj.h>

//boris: to get symtab status before start to run script
#include <setprocess.h>
extern "C" char *get_exec_path();
extern int get_epoch_parent_flag();

char * cmd_current_journal_name = NULL;
char * cmd_execute_journal_name = NULL;
char * cmd_xterm_input_file = NULL;
int    cmd_logging_on = 0;

const  int cmdVersion         = 10;
static int cmd_script_version = -1;

extern "C"
{
    void xdisplay_set_running_auto_test();
    void xdisplay_unset_running_auto_test();
}

extern "C" void cmd_set_script_version (int ver) { cmd_script_version = ver; };
int get_cmdVersion() { return cmdVersion; }

struct cmdAvailFunc {
    int func;
    int avail_version;
};

static cmdAvailFunc avail_arr [] =  {
{cmdGraValidation, 1},
{cmdGraDescription, 2},
{cmdViewName, 3},
{cmdSymbolAttr, 4},
{cmdNewGroups, 5},
{cmdCreateDD_VERSION, 6},
{cmdBuildShortStrings, 7},
{cmdNewFlowcharBehavior, 8},
{cmdPrjWritableProjects, 9},
{cmdCharBased2fileMerge, 10}
};

static int get_func_version (int func)
{
    int sz = sizeof(avail_arr) / sizeof(cmdAvailFunc);
    int ver = -1;
    for (int ii = 0; ii < sz ; ii++) {
	cmdAvailFunc av_f = avail_arr[ii];
	if (av_f.func == func) {
	    ver = av_f.avail_version;
	    break;
	}
    }
    return ver;
}

int cmd_available_p (int func)
{
    Initialize(cmd_available_p);
    
    if(!cmd_execute_journal_name )
      if(cmd_script_version < 0)
	return 1;

    int res = 0;
    
    int func_vers = get_func_version (func);
    if (func_vers >= 0 && cmd_script_version >= func_vers)
	res = 1;

    return res;
}

const int defaultValidation = FullValidation;
static int currentValidation = defaultValidation;

extern "C" void cmd_set_validation_level (int lev)
{
    Initialize(cmd_set_validation_level);
    currentValidation = (lev > 0) ? lev : defaultValidation;
}

int cmd_validate_p (int lev)
{
    Initialize(cmd_validate_p);
    lev =  (lev > 0) ? lev : defaultValidation;
    int res = 0;
    if (cmd_current_journal_name || cmd_execute_journal_name)
	if (lev && lev >= currentValidation)
	    res = 1;

    return res;
}

void cmd_execute(char *);
void cmd_reset();
extern "C" void cmd_set_options();
extern "C" void cmd_reset_options();

static void clr(char*& v)
  {
  if(v != NULL) delete v;
  v = NULL;
  }

void reset_inst_arr_num();

static void set(char*& v, char *t)
  {
  clr(v);
  int l = strlen(t);
  v = /*malloc(l + 1)*/ new char[l+1];
  strcpy(v, t);
  reset_inst_arr_num();
  }

static void create_xterm_file()
{
    Initialize(create_xterm_file);

    static genTmpfile cmd_log("script");
    cmd_log.open();
    cmd_log.close();
    set(cmd_xterm_input_file, (char *)cmd_log.name());
}

extern "C" void unlink_xterm_file()
{
    Initialize(unlink_xterm_file);
}

extern "C" void cmd_journal_start(char *f, char *bug, char *sub, char *pdf, char *proj)
{
      Initialize(cmd_journal_start);
        Assert(f && *f);

        set(cmd_current_journal_name, f);
        create_xterm_file();

        char const * user = customize::user();
        if (!(user && *user)) {
    	    // Prefs for user name are not set.
    	    user = OSapi_getenv("USER");
    	}
        Assert(user && *user);

        if(!sub || !*sub)
              sub = "EVERYTHING";
        cmd_set_validation_group(sub);

        if(!proj || !*proj){
	  const projNode *proj_node = projNode::get_home_proj();
	  if(proj_node) 
	    proj = proj_node->get_name();
        }
        cmdJournal_create(f, user, pdf, bug, sub, proj, 1); 

	if( get_epoch_parent_flag() )
	{
	    /* force emacs to start recording... */
	    genString startRecording;
	    startRecording.printf("(di-force-macro-recording t)");
	    ste_epoch_evaluate(startRecording);         
	}
    cmd_set_options();
}

extern "C" void cmd_journal_log(char* f)
{
    cmd_logging_on = 1;
    cmd_journal_start(f,0,"NOTHING",0,0);
}

extern "C" void cmd_journal_save(char *f)
    {
    if(f != NULL && *f != 0)
    {
         set(cmd_current_journal_name, f);
         create_xterm_file();
    }

    cmdJournal_finish();
    cmd_reset();
    cmd_reset_options();

    unlink_xterm_file();
    }

extern "C" void cmd_journal_execute(char *f)
{
	cmd_script_version = -1;
	cmd_exec_break();
	set(cmd_execute_journal_name, f);
	create_xterm_file();
	xdisplay_set_running_auto_test();
	cmd_execute(f);
	driver_instance->run();
	clr(cmd_execute_journal_name);
	xdisplay_unset_running_auto_test();
}

extern "C" void cmd_journal_exec_within(char *f)
    {
	if( get_epoch_parent_flag() )
	{
	    /* stop emacs from recording while replaying */
	    genString endRecording;
	    endRecording.printf("(di-force-macro-recording nil)");
	    ste_epoch_evaluate(endRecording);        
	}
    set(cmd_execute_journal_name, f);
    create_xterm_file();
    xdisplay_set_running_auto_test();
    cmd_execute(f);
    }

extern "C" void cmd_journal_insert_line(char *)
    {
    }

