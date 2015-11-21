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

extern int num_subsys;
extern int threshold;
Interpreter* GetActiveInterpreter();
void         SetActiveInterpreter(Interpreter*);
int          cli_process_input(char *data, ostream&, class Interpreter *, int do_propmt_and_print);
int          cli_eval_string(const char* cmd);
 
#ifdef _WIN32
#define DEFWT(a,b,c,d,e)                \
   defaultweights[a].outgoing = b;              \
   defaultweights[a+NWEIGHTS/2].outgoing=c;     \
   defaultweights[a].count = e;         \
   defaultweights[a+NWEIGHTS/2].count=e;\
   defaultweights[a].m_weight=d;

static void setupdefaults() {
  static int isinitialized;
  if (isinitialized) return;
  DEFWT(AUS_FCALL,      true,   false,  10, -1);
  DEFWT(AUS_DATAREF,    true,   false,  10, -1);
  DEFWT(AUS_INSTANCE,   true,   false,  10, -1);
  DEFWT(AUS_ARGTYPE,    true,   false,  10, -1);
  DEFWT(AUS_RETTYPE,    true,   false,  10, -1);
  DEFWT(AUS_ELEMENT,    false,  true,   10, -1);
  DEFWT(AUS_FRIEND,     false,  false,  15, -1);
  DEFWT(AUS_SUBCLASS,   true,   false,  10, -1);
  DEFWT(AUS_MEMBER,     true,   false,  15, -1);
  DEFWT(AUS_FILENAME,   false,  false,  10, 0);
  DEFWT(AUS_LOGICNAME,  false,  false,  10, 0);
  isinitialized=1;
}
#endif

static void prompt_reply(Interpreter*i, char*reply)
{
  Tcl_SetVar(i->interp, (char *)"prompt_canceled", (char *)"0", 0);
  Tcl_SetResult(i->interp, reply, TCL_VOLATILE);
}
 
static void prompt_canceled(Interpreter*i)
{
  Tcl_SetVar(i->interp, (char *)"prompt_canceled", (char *)"1", 0);
  Tcl_SetResult(i->interp, (char *)"", TCL_VOLATILE);
}

static int questionCmd(ClientData cd, Tcl_Interp *interp, int argc, char *argv[])
{
  Interpreter *i       = (Interpreter *)cd;
  char *title          = (char *)"";

  int arg_pos = 1;
  if(!strcmp(argv[arg_pos], "-title")){
    if(arg_pos < argc - 1) {
      title = argv[++arg_pos];
      arg_pos ++;
    } else {
      Tcl_AppendResult(i->interp, "prompt: title is missing", NULL);
      return TCL_ERROR;
    }
  }
  const char * question = argv[arg_pos];
  ++ arg_pos;

  int no_vals = argc - arg_pos; 
  char* button1 = NULL;
  char* button2 = NULL;
  char* cancel = NULL;

  switch(no_vals) {
  case 0:
    button1 = (char *)"OK";
    no_vals = 1;
    break;
  case 1:
    button1 = argv[arg_pos];
    break;
  case 2:
    button1 = argv[arg_pos];
    button2 = argv[arg_pos+1];
    break;
  case 3:
    button1 = argv[arg_pos];
    button2 = argv[arg_pos+1];
    cancel  = argv[arg_pos+2];
    break;
  default:
    Tcl_AppendResult(i->interp, "question: wrong number of args", NULL);
    return TCL_ERROR;
    break;
  }

  int status;
  if(no_vals == 1)
    status = dis_question( title, button1, cancel, question );
  else
    status = dis_question3( NULL, title, button1, button2, cancel, question );

  if(status > 0){
    char* reply = (status == 1 ? button1 : button2);
    prompt_reply(i, reply);
  } else if(cancel){
    prompt_reply(i, cancel);
  } else {
    prompt_canceled(i);
  }
  return TCL_OK;
}

extern int dis_prompt_string_OK_cancel( const char *title, const char *prompt_str,
                                 const char *err_msg, genString &result );

static int PromptCmd(ClientData cd, Tcl_Interp *interp, int argc, char *argv[])
{
    CHECK_ARG_NUM("prompt", 0);
    Interpreter *i       = (Interpreter *)cd;
    genString prompt_str = "";
    char *title          = (char *)"";
    char *err_msg        = NULL;
    int arg_pos          = 1;
    genString reply;
    while(arg_pos < argc){
	if(!strcmp(argv[arg_pos], "-title")){
	    arg_pos++;
	    if(arg_pos < argc)
		title = argv[arg_pos++];
	    else {
		Tcl_AppendResult(i->interp, "prompt: title is missing", NULL);
		return TCL_ERROR;
	    }
	} else if(!strcmp(argv[arg_pos], "-errmsg")){
	    arg_pos++;
	    if(arg_pos < argc)
		err_msg = argv[arg_pos++];
	    else {
		Tcl_AppendResult(i->interp, "prompt: error message is missing", NULL);
		return TCL_ERROR;
	    }
	} else {
	    prompt_str += argv[arg_pos++];
	}
    }
    
    int status = dis_prompt_string_OK_cancel( title, prompt_str, err_msg, 
                                              reply );

    if(status > 0){
	prompt_reply(i, (char*)reply);
    } else {
	prompt_canceled(i);
    }
    return TCL_OK;
}

projModule* symbol_get_module(symbolPtr&sym);

static int src_versionCmd(ClientData cd, Tcl_Interp *interp, int argc, char **argv)
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
  static genString buf;
  int tp = 1; // for put
  module->get_rev(buf, tp);
  Tcl_AppendResult(i->interp, (char*)buf, NULL);
  
  return TCL_OK;
}

void projModule_get_built_revision (projModule *, genString&);
static int model_versionCmd(ClientData cd, Tcl_Interp *interp, int argc, char **argv)
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

  static genString buf;
  projModule_get_built_revision (module, buf);
  Tcl_AppendResult(i->interp, (char*)buf, NULL);
  
  return TCL_OK;
}

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

static int SelectFileCmd(ClientData cd, Tcl_Interp *interp, int argc, char *argv[])
{
  Interpreter *i = (Interpreter *)cd;

  genString usage;
  usage.printf("Usage: %s %s", argv[0], "[ -title <name> ]");

  char *title = (char *)"File Browser";  

  for (int ii = 1; ii < argc; ii++) {
    if (!strcmp(argv[ii], "-title"))
      if (++ii < argc)                // Increment counter, move to the next arg
	title = argv[ii];
      else {
	Tcl_AppendResult(i->interp, (char *)usage, NULL);
	return TCL_ERROR;
      }
    else {
      Tcl_AppendResult(i->interp, (char *)usage, NULL);
      return TCL_ERROR;
    }
  }


  genString filename;
  int status = dis_wait_filename(filename, title);

  if (status) {
      prompt_reply(i, (char*)filename);
      return TCL_OK;
  } else {
      prompt_canceled(i);
      return TCL_OK;
  }
}

int FetchModuleCmd(ClientData cd, Tcl_Interp *interp, int argc, char *argv[])
{
  Initialize(FetchModuleCmd);

  int ret = TCL_ERROR;

  Interpreter *i = (Interpreter *)cd;
  char *opt = NULL;
  genString files;
  bool valid = true;

  // Parse the command line:

  for (int ii = 1; ii < argc; ii++) {
    if (!strcmp(argv[ii], "-opt")) {
      if (!opt) {
	if (ii < argc - 1) {
	  opt = argv[++ii];
	}
	else { valid = false; break; }  
      }
      else { valid = false; break; }
    }
    else { 
      files += " ";
      files += argv[ii];
    }
  }

  // Grab the symbols:

  symbolArr symarr;
  symbolArr filearr;
  if (valid) {
    if (files.not_null()) {
      if (i->ParseArgString((char *)files, symarr) == 0) {
	symbolPtr sym;
	ForEachS(sym, symarr) {
	  if (sym.get_kind() == DD_MODULE) {
	    projModulePtr mod = NULL;
						if (sym.is_xrefSymbol())
							mod = projModule_of_symbol(sym);
						else if (is_projModule(sym))
							mod = projModulePtr(RelationalPtr(sym));
						if (mod)
						  filearr.insert_last(mod);
		}
	}
      }
    }
    else {
      valid = false;
    }
  }

  // Process the files:

  if ((valid) && (filearr.size() > 0)) {

    // Set the CM options:

    char *oldopt = NULL;
    bool modopt = false;
    if (opt) {
      oldopt = (char *)(customize::configurator_get());
      if (strcmp(opt, oldopt) != 0) {
	customize::configurator_get(opt);
	modopt = true;
      }
    }

    // Get each module:

	  filearr.remove_dup_syms();

    symbolArr good;
    symbolArr bad;
    int arrsz = filearr.size();
    for (int ii = 0; ii < arrsz; ii++) {
			projModule *_mod = projModulePtr(RelationalPtr(filearr[ii]));
      symbolPtr sym = _mod;
      projModule *mod = projModule::get_module(sym);
      if (mod) {
	appTree *at = (appTree *)mod;
	if (is_projModule(at)) {
	  projModule *pm = checked_cast(projModule, at);
	  projNode *pr = pm->get_project();
	  Xref *xr = pr->get_xref();
	  XrefTable *xrt = xr->get_lxref();
	  if (!xrt) {
	    xr->init(WRITE_ONLY);
	    xrt = xr->get_lxref();
	  }
	  if (xrt) {
	    if (xr->get_perm_flag() != READ_ONLY) {
	      xrt->insert_symbol(pm);
	    }
	  }
	}
	genString ln, fn;
	sym.get_name(ln);
	projNode *pr = projHeader::get_pmod_project_from_ln(ln);
	if (pr) {
	  pr->ln_to_fn(ln, fn);
	  if (fn.length() > 0) {
	    app *ah = app::find_from_phys_name(fn);
	    if (!ah)
	      ah = mod->get_app();
	    if (ah) {
	      if (ah->is_modified()) {
		mod->save_module();
		obj_unload(ah);
	      }
	    }
	  }
	}
	good.insert_last(sym);
      }
      else {
	bad.insert_last(sym);
      }
    }

    // Return the successfully fetched files:

    if (good.size() > 0) {
      DI_object result;
      DI_object_create(&result);
      symbolArr *arr = get_DI_symarr(result);
      *arr = good;
      i->SetDIResult(result);
    }

    // Reset the CM options:

    if (modopt) {
      customize::configurator_get(OSapi_strdup(oldopt));
    }

    ret = TCL_OK;
  }
  else {
    genString usage;
    usage.printf("Usage: %s module1 [ module2 module3 ... ] [ -opt optstr ]", argv[0]);
    Tcl_AppendResult(interp, (char *)usage, NULL);
  }
    
  return ret;
}

//====================== ACCESS CM INTERFACE ===================================

static int CmGetCmd(ClientData cd, Tcl_Interp *interp, int argc, char *argv[]) {
  Interpreter *i = (Interpreter *)cd;
  if(argc<2) {
	  Tcl_AppendResult(i->interp, "Usage : cm_get <symbol set> { <symbol index> }",NULL);
	  return TCL_ERROR;
  }
  symbolArr sym_arr;
  if (i->ParseArguments(1, argc, argv, sym_arr) == 0) {
      symbolPtr sym;
      ForEachS (sym, sym_arr) {
	  if (sym.get_kind() == DD_MODULE) {
	      projModule* pModule = symbol_get_module(sym);
			  if(pModule) {
				  pModule->get_module(sym);
			  }
		  }
	  }
  }
  return TCL_OK;
}

int dis_cm_event_put_internal(projModule *mod, const char* comment);
static int CmPutCmd(ClientData cd, Tcl_Interp *interp, int argc, char *argv[]) {
  Interpreter *i = (Interpreter *)cd;
  if(argc<4) {
	  Tcl_AppendResult(i->interp, "Usage : cm_put <options> <comment> <symbol set> { <symbol index> }",NULL);
	  return TCL_ERROR;
  }
  symbolArr sym_arr;
  if (i->ParseArguments(3, argc, argv, sym_arr) == 0) {
      symbolPtr sym;
      ForEachS (sym, sym_arr) {
	  if (sym.get_kind() == DD_MODULE) {
	      projModule* pModule = symbol_get_module(sym);
			  if(pModule) {
  		      projNode* pProject=pModule->get_project();
				  if(pProject) {
				      int status = pModule->fast_put_module(pProject,argv[1],argv[2],NULL,NULL);
					  if(status) dis_cm_event_put_internal(pModule,argv[2]);
				  }
			  }
		  }
	  }
  }
  return TCL_OK;
}


static int CmUngetCmd(ClientData cd, Tcl_Interp *interp, int argc, char *argv[]) {
  Interpreter *i = (Interpreter *)cd;
  if(argc<2) {
	  Tcl_AppendResult(i->interp, "Usage : cm_unget <symbol set> { <symbol index> }",NULL);
	  return TCL_ERROR;
  }
  symbolArr sym_arr;
  if (i->ParseArguments(1, argc, argv, sym_arr) == 0) {
      symbolPtr sym;
      ForEachS (sym, sym_arr) {
	  if (sym.get_kind() == DD_MODULE) {
	      projModule* pModule = symbol_get_module(sym);
			  if(pModule) {
				  pModule->unget_module();
			  }
		  }
	  }
  }
  return TCL_OK;
}

static int CmLockCmd(ClientData cd, Tcl_Interp *interp, int argc, char *argv[]) {
  Interpreter *i = (Interpreter *)cd;
  if(argc<2) {
	  Tcl_AppendResult(i->interp, "Usage : cm_lock <symbol set> { <symbol index> }",NULL);
	  return TCL_ERROR;
  }
  symbolArr sym_arr;
  if (i->ParseArguments(1, argc, argv, sym_arr) == 0) {
      symbolPtr sym;
      ForEachS (sym, sym_arr) {
	  if (sym.get_kind() == DD_MODULE) {
	      projModule* pModule = symbol_get_module(sym);
			  if(pModule) {
				  pModule->lock_module();
			  }
		  }
	  }
  }
  return TCL_OK;
}

static int CmIsGetCmd(ClientData cd, Tcl_Interp *interp, int argc, char *argv[]) {
  Interpreter *i = (Interpreter *)cd;
  if(argc<2) {
	  Tcl_AppendResult(i->interp, "Usage : cm_is_get <symbol set> { <symbol index> }",NULL);
	  return TCL_ERROR;
  }
  symbolArr sym_arr;
  if (i->ParseArguments(1, argc, argv, sym_arr) == 0) {
      symbolPtr sym;
      ForEachS (sym, sym_arr) {
	  if (sym.get_kind() == DD_MODULE) {
	      projModule* pModule = symbol_get_module(sym);
			  if(pModule) {
  		      projNode* pProject=pModule->get_project();
				  if(pProject) {
				      projNode* pHomeProject = pProject->get_home_proj();
					  if(pHomeProject) {
						  if(pHomeProject->find_module(pModule->get_name())!=NULL) {
							Tcl_SetResult(interp, (char *)"1", TCL_VOLATILE);
				return TCL_OK;
						  }

					  }
				  }
			  }
		  }
	  }
  }
  Tcl_SetResult(interp, (char *)"0", TCL_VOLATILE);
  return TCL_OK;
}

static int CmUnlockCmd(ClientData cd, Tcl_Interp *interp, int argc, char *argv[]) {
  Interpreter *i = (Interpreter *)cd;
  if(argc<3) {
	  Tcl_AppendResult(i->interp, "Usage : cm_unlock <comment> <symbol set> { <symbol index> }",NULL);
	  return TCL_ERROR;
  }
  symbolArr sym_arr;
  if (i->ParseArguments(2, argc, argv, sym_arr) == 0) {
      symbolPtr sym;
      ForEachS (sym, sym_arr) {
	  if (sym.get_kind() == DD_MODULE) {
	      projModule* pModule = symbol_get_module(sym);
			  if(pModule) {
				  pModule->unlock_module_c(argv[1]);
			  }
		  }
	  }
  }
  return TCL_OK;
}

void  shell_configurator_set_output(genString* pOut);
const char* shell_configurator_get_message();
int shell_configurator ( const char* cmd,  const char *tag,  const char* arg1, const char * arg2,
			const char* arg3, const char* arg4, const char* arg5);

static int CMInfoCmd(ClientData cd, Tcl_Interp *interp, int argc, char *argv[]) {
  Interpreter *shell = (Interpreter *)cd;

  char* cm_name = (char *)"cm_off";

  if ( !customize::no_cm() )
  {
#ifndef _WIN32
    cm_name = (char *)customize::configurator_system();
    if ( !cm_name || *cm_name == 0 )
      cm_name = (char *)"cm_none";
#else
    genString gen_cm_name;
    shell_configurator_set_output( &gen_cm_name );
    if ( shell_configurator ("cm_info", "", 0,0,0,0,0) == 0)
      cm_name = "cm_none";
    else
      cm_name = (char*)gen_cm_name.str();
    shell_configurator_set_output( NULL );

#endif
  }

  Tcl_SetResult(shell->interp, cm_name, TCL_VOLATILE);
  return TCL_OK;
}

static int CMDiffCmd(ClientData cd, Tcl_Interp *interp, int argc, char *argv[]) {
  Interpreter *shell = (Interpreter *)cd;

  if(argc<2) {
	  Tcl_AppendResult(shell->interp, "Usage : cm_diff <file_name>",NULL);
	  return TCL_ERROR;
  }
  char* diff_text = (char *)"";

#ifdef _WIN32
    genString gen_diff_text;
    shell_configurator_set_output( &gen_diff_text );
    if ( shell_configurator ("diff", "", (char*)argv[1],(char*)argv[1],"","","") != 0)
      diff_text = (char*)gen_diff_text.str();
    shell_configurator_set_output( NULL );
#endif

  Tcl_SetResult(shell->interp, diff_text, TCL_VOLATILE);
  return TCL_OK;
}

static int add_pset_access_cmds()
{
  new cliCommandInfo("question", questionCmd);
  new cliCommandInfo("prompt", PromptCmd);
  new cliCommandInfo("src_version", src_versionCmd);
  new cliCommandInfo("model_version", model_versionCmd);
  new cliCommandInfo("mrg_module_checkout", mrg_module_checkoutCmd);
  new cliCommandInfo("mrg_module_diff", mrg_module_diffCmd);
  new cliCommandInfo("filesb", SelectFileCmd);+
  new cliCommandInfo("fetch", FetchModuleCmd);
  new cliCommandInfo("cm_get", CmGetCmd);
  new cliCommandInfo("cm_put", CmPutCmd);
  new cliCommandInfo("cm_unget",CmUngetCmd);
  new cliCommandInfo("cm_lock", CmLockCmd);
  new cliCommandInfo("cm_unlock", CmUnlockCmd);
  new cliCommandInfo("cm_is_get", CmIsGetCmd);
  new cliCommandInfo("cm_info", CMInfoCmd);
  new cliCommandInfo("cm_diff", CMDiffCmd);

  return 0;
}

static int dummy = add_pset_access_cmds();
