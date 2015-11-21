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
#include "cLibraryFunctions.h"
#include "machdep.h"
#include "general.h"
#include "genError.h"
#include "genString.h"
#include "ste_interface.h"
#include "NewPrompt.h"
#include "autosubsys.h"
#ifndef ISO_CPP_HEADERS
#include "limits.h"
#else /* ISO_CPP_HEADERS */
#include <limits>
using namespace std;
#endif /* ISO_CPP_HEADERS */
#include "Question.h"
#include "projModule_of_symbol.h"
#include <db.h>
#include <XrefTable.h>
#ifndef ISO_CPP_HEADERS
#include <fstream.h>
#else /* ISO_CPP_HEADERS */
#include <fstream>
#endif /* ISO_CPP_HEADERS */
#include <cliClient.h>
#include "tcl.h"
#include "Interpreter.h"
#include "api_internal.h"
#include "path.h"
#include <autosubsys-macros.h> 
#include <autosubsys-weights.h>
#include "statistics.h"
#include <TclList.h>
#ifdef _WIN32
#include "mapi.h"
#endif 
#include <msg.h>

extern StatInfo* StatisticsList[NUM_OF_STATS];
projModule* symbol_get_module(symbolPtr&sym);

extern int num_subsys;
extern int threshold;

int mrg_projModule_checkout (projModule *);
static int mrg_module_checkoutCmd(ClientData cd, Tcl_Interp *interp, int argc, char **argv)
{
  Interpreter *i = (Interpreter *)cd;
  symbolArr arr;
  if(i->ParseArguments(1, argc, argv, arr) != 0)
    return TCL_ERROR;
  
  if(arr.size() != 1){
    Tcl_AppendResult(i->interp, "size(input) != 1", NULL);
    return TCL_ERROR;
  }      
  
  projModule*module = symbol_get_module(arr[0]);

  if(!module){
    char *name = arr[0].get_name();
    char message[1024];
    if ( name )
      sprintf(message, "Not a module: '%s'", name );
    else
      strcpy(message, "Not a module: '<entity name unavailable>'");
    Tcl_AppendResult(i->interp, message, NULL);
    return TCL_ERROR;
  }

  int res = mrg_projModule_checkout (module);
  char *buf = (char *)"identical";  // 0
  if (res < 0)
      buf = (char *)"failed";
  else if (res > 0)
      buf = (char *)"diffed";
  
  Tcl_AppendResult(i->interp, buf, NULL);
  return TCL_OK;
}

int mrg_projModule_diff (projModule *);
static int mrg_module_diffCmd(ClientData cd, Tcl_Interp *interp, int argc, char **argv)
{
  Interpreter *i = (Interpreter *)cd;
  symbolArr arr;
  if(i->ParseArguments(1, argc, argv, arr) != 0)
    return TCL_ERROR;
  
  if(arr.size() != 1){
    Tcl_AppendResult(i->interp, "size(input) != 1", NULL);
    return TCL_ERROR;
  }      
  
  projModule*module = symbol_get_module(arr[0]);

  if(!module){
    char *name = arr[0].get_name();
    char message[1024];
    if ( name )
      sprintf(message, "Not a module: '%s'", name );
    else
      strcpy(message, "Not a module: '<entity name unavailable>'");
    Tcl_AppendResult(i->interp, message, NULL);
    return TCL_ERROR;
  }

  int res = mrg_projModule_diff (module);
  char *buf = (char *)"identical";  // 0
  if (res < 0)
      buf = (char *)"failed";
  else if (res > 0)
      buf = (char *)"diffed";
  
  Tcl_AppendResult(i->interp, buf, NULL);
  return TCL_OK;
}

static int add_pset_access_cmds()
{
  new cliCommandInfo("mrg_module_checkout", mrg_module_checkoutCmd);
  new cliCommandInfo("mrg_module_diff", mrg_module_diffCmd);
  return 0;
}

int tcl_mrg_commands = add_pset_access_cmds();
