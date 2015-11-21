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
#include <fstream.h>
#include <stdlib.h>
#include <ctype.h>
#else /* ISO_CPP_HEADERS */
#include <fstream>
using namespace std;
#include <cstdlib>
#include <cctype>
#endif /* ISO_CPP_HEADERS */
#ifndef ISO_CPP_HEADERS
#include <signal.h>
#include <strstream.h>
#else /* ISO_CPP_HEADERS */
#include <csignal>
#endif /* ISO_CPP_HEADERS */

#include "BrowserController.h"
#include "Interpreter.h"
#include "Question.h"
#include "SocketClient.h"
#include "api_internal.h"
#include "attribute.h"
#include "cLibraryFunctions.h"
#include "charstream.h"
#include "customize.h"
#include "ddKind.h"
#include "driver_mode.h"
// #include "exec_path.h"
#include "format.h"
#include "genError.h"
#include "genString.h"
#include "genStringPlus.h"
#include "genTry.h"
#include "general.h"
#include "machdep.h"
#include "mpb.h"
#include "msg.h"
#include "objArr.h"
#include "scopeMgr.h"
#include "tcl.h"
#include "tk.h"
#include "xref_queries.h"
#include "xxinterface.h"

#undef DBG
#define DBG_MEMORY 1
#define DBG_TRACE  2
#define DBG_TRANS  4


#if 0
extern int wftask_commands_flag;
extern int wfnode_commands_flag;
extern int xxaccess_dummy_var;
extern int message_dummy_var;
extern int querytree_init_flag;
extern int subchk_init_flag;
#endif

extern "C" int TclRenameCommand(Tcl_Interp *interp, char const *oldName, char const *newName);
Interpreter *InitializeCLIInterpreter(void);

extern Tcl_ObjType set_type;

static genStringPlus cli_buffer;

int scope_is_on = 0;

extern "C" void pdf_cli_start()
{
  cli_buffer.reset();
  scope_is_on = 1;
}

extern "C" void pdf_cli_line(char*text)
{
    cli_buffer += text;
}

typedef cliCommandBase *CommandInfoPtr;
objArr * cli_comm_array;

cliCommandBase::cliCommandBase(const char*name): objDictionary(name)
{
    if (!cli_comm_array) cli_comm_array = new objArr;
    cliCommandBase* vthis = this;
    cli_comm_array->insert_last(vthis);
};

int cliCommandInfo::eval (Interpreter*i, int argc, char const **argv)
{
    return (*_cptr)((ClientData)i, i->interp, argc, argv);
}

cliCommandInfo::cliCommandInfo(char* name, CB cb) :cliCommandBase(name), _cptr(cb) {} 

int cliCommand::eval (Interpreter*i, int argc, char const **argv)
{
    return (*_cptr)(i, argc, argv);
}

int cliQueryCommand::eval (Interpreter*i, int argc, char const **argv)
{
    symbolArr in;
    if(i->ParseArguments(1, argc, argv, in) != 0)
	return TCL_ERROR;

    ostream&os = i->GetOutputStream();
    symbolArr out;
    int code = query(os, in, out);
    if(code != TCL_OK)
	return code;
    
    DI_object results;
    DI_object_create(&results);
    (*get_DI_symarr(results)) = out;
    
    i->SetDIResult(results);
    return TCL_OK;  
}

struct cliLinkCommand : public cliQueryCommand {
  cliLinkCommand(const char*nm, int ln) : cliQueryCommand(nm), link(ln) {}
  int query(ostream&os, const symbolArr& in, symbolArr& out);
  int link;
};

void fill_array_with_xref_query(symbolArr&arr, int link, symbolArr&res);
int cliLinkCommand::query(ostream&os, const symbolArr& in, symbolArr& out)
{
  fill_array_with_xref_query((symbolArr&)in, link, out);
  return TCL_OK;
}

int cli_selected_dbg_output = 0;

#define DBG(class,command) if((class & (cli_selected_dbg_output)) != 0) { \
                              command; \
			      fflush(stdout); \
			      }
extern "C" char selection_parse_error[];
extern "C" char* cli_error_use();

Interpreter *interpreter_instance = NULL;

static Interpreter *active_interpreter = NULL;

void SetActiveInterpreter(Interpreter *i)
{
    active_interpreter = i;
    if(i)
	setClient(i->client_number);
}

Interpreter*  GetActiveInterpreter()
{
   return active_interpreter ? active_interpreter 
                             : interpreter_instance;
}

int cli_set_global_var(Interpreter*i, const char* name, const char*val)
{
  if(!i)
    i =  GetActiveInterpreter();
  char const* v = Tcl_SetVar(i->interp, (char*)name, (char*)val, TCL_GLOBAL_ONLY);   
  return (v==NULL);
}

static void PrintDIObject(DI_object obj, symbolFormat *sf, genString& str)
{
    Initialize(PrintDIObject);
    
    symbolArr *source_arr = get_DI_symarr(obj);
    ocharstream tmp_stream;
    sf->printOut(tmp_stream, *source_arr);
    tmp_stream << ends;
    char *out = tmp_stream.str();
    str       = out;
    delete [] out;
}


// ----------------------------------------------------------------------

Interpreter *Interpreter::Create(void)
{
    Initialize(Interpreter::Create);

    Interpreter *i = new Interpreter();
    if(!i->interp){
	delete i;
	return NULL;
    }
    i->client_number = newClient();
    setClient(i->client_number);
    i->SaveOldInfoCmd();
    i->InitAttributes();
    i->InitBuiltinFunctions();
    DI_object null_var;
    DI_object_create(&null_var);
    Tcl_Obj *cur_set_name = Tcl_NewStringObj("CurSet", -1);
    Tcl_IncrRefCount(cur_set_name);
    Tcl_ObjSetVar2(i->interp, cur_set_name, NULL, null_var, TCL_GLOBAL_ONLY);
    Tcl_DecrRefCount(cur_set_name);
    Tcl_SetVar(i->interp, "prompt_canceled", "0", 0);

    symbolFormat *fmt = new symbolFormat("default");
    objArr fmt_attrs;
    fmt_attrs.insert_last(symbolAttribute::get_by_name("kind"));
    fmt_attrs.insert_last(symbolAttribute::get_by_name("name"));
    fmt->setAttributes(fmt_attrs);
    fmt->setFormat("(%s)  %s\n");
    i->current_print_format = fmt;
    i->sort_flag = false;

    i->SetPrompt("% ");

    return i;
}

void Interpreter::SetOutputStream(ostream& ostr)
{
    Initialize(Interpreter::SetOutputStream);
    
    output_stream = &ostr;
}

ostream& Interpreter::GetOutputStream()
{
    Initialize(Interpreter::GetOutputStream);
    
    return *output_stream;
}

void Interpreter::SetPrompt(char const *p)
{
    prompt = p;
}

int Interpreter::EvalCmd(char const *cmd)
{
    Tcl_Obj     *cmdPtr;
    Interpreter *prev_interp = active_interpreter;

    SetActiveInterpreter(this);
    cmdPtr = Tcl_NewStringObj(cmd, -1);
    Tcl_IncrRefCount(cmdPtr);
    /*
     * Compile and execute the bytecodes.
     */
    code                = Tcl_EvalObj(interp, cmdPtr);
    Tcl_Obj *obj_result = Tcl_GetObjResult(interp);
    if(obj_result->typePtr != NULL && obj_result->typePtr == &set_type){
	Tcl_Obj *cur_set_name = Tcl_NewStringObj("CurSet", -1);	      
        Tcl_IncrRefCount(cur_set_name);
	Tcl_ObjSetVar2(interp, cur_set_name, NULL, obj_result, TCL_GLOBAL_ONLY);
	Tcl_DecrRefCount(cur_set_name);
    }
    /*
     * Move the interpreter's object result to the string result, 
     * then reset the object result.
     * FAILS IF OBJECT RESULT'S STRING REPRESENTATION CONTAINS NULLS.
     */
//    int len;
//    Tcl_SetResult(interp, Tcl_GetStringFromObj(obj_result, &len), TCL_VOLATILE);
    /*
     * Discard the Tcl object created to hold the command and its code.
     */
    Tcl_DecrRefCount(cmdPtr);
    SetActiveInterpreter(prev_interp);
    return code;
}

int Interpreter::EvalFile(char const *fname)
{
    Interpreter *prev_interp = active_interpreter;

    SetActiveInterpreter(this);
    code = Tcl_EvalFile(interp, fname);
    SetActiveInterpreter(prev_interp);
    return code;
}

void Interpreter::SetDIResult(DI_object res)
{
    Tcl_SetObjResult(interp, res);
}

int Interpreter::GetResult(genString& str)
{
    DI_object res_obj = DecodeDIResult();
    if(res_obj != NULL){
	PrintDIObject(res_obj, current_print_format, str);
    } else
	str = Tcl_GetStringResult(interp);
    return code;
}

void Interpreter::PrintPrompt(void)
{
    if(prompt)
	(*output_stream) << prompt << flush;
}

int Interpreter::PrintResult(void)
{
    DI_object res_obj = DecodeDIResult();
    if(res_obj != NULL){
	symbolArr *res = get_DI_symarr(res_obj);
	PrintSymbolArr(*res, 1);
    } else {
	char const *tmp = Tcl_GetStringResult(interp);
	if(tmp != NULL && tmp[0] != 0)
	    (*output_stream) << tmp << endl;
    }
    return code;
}

void Interpreter::SaveResult(void)
{
}

int Interpreter::ReferenceObject(DI_object obj)
{
    Tcl_IncrRefCount(obj);
    return 1;
}

int Interpreter::DereferenceObject(DI_object obj)
{
//    if(object_refs.size() <= val){
//	printf("Critical error in interpreter\n");
//	return 1;
//    }
    Tcl_DecrRefCount(obj);
    return 1;
}

void Interpreter::DeleteDIObject(DI_object val)
{
    DBG(DBG_MEMORY,printf("Will free _DI_%d\n", val));
    DereferenceObject(val); // Tcl will take care of object destruction
}

DI_object Interpreter::DecodeDIResult(void)
{
    DI_object ret_val = Tcl_GetObjResult(interp);
    if(ret_val->typePtr && ret_val->typePtr == &set_type)
	return ret_val;
    else
	return NULL;
}

int Interpreter::TraceCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[])
{
    Interpreter *i = (Interpreter *)cd;
    if(argc < 2){
	Tcl_AppendResult(i->interp, "wrong number of arguments", NULL);
	return TCL_ERROR;
    }
    int  turn_on   = 1;
    char const *switches = argv[1];
    while(*switches){
	switch(*switches){
	  case '+':
	    turn_on = 1;
	    break;
	  case '-':
	    turn_on = 0;
	    break;
	  case 'T':
	    if(turn_on)
		cli_selected_dbg_output |= DBG_TRACE;
	    else
		cli_selected_dbg_output &= ~DBG_TRACE;
	    break;
	  case 'M':
	    if(turn_on)
		cli_selected_dbg_output |= DBG_MEMORY;
	    else
		cli_selected_dbg_output &= ~DBG_MEMORY;
	    break;
	  case 'L':
	    if(turn_on)
		cli_selected_dbg_output |= DBG_TRANS;
	    else
		cli_selected_dbg_output &= ~DBG_TRANS;
	    break;
	  default:
	    Tcl_AppendResult(i->interp, "wrong switch", NULL);
	    return TCL_ERROR;
	}
	switches++;
    }
    return TCL_OK;
}

static int archCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[])
{
    Interpreter *i = (Interpreter *)cd;
#ifdef _WIN32
    static char const *arch = "windows";
#else
    static char const *arch = "unix";
#endif
    Tcl_SetResult(i->interp, (char *)arch, TCL_STATIC);
    return TCL_OK;
}

static int is_guiCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[])
{
    Interpreter *i = (Interpreter *)cd;
    char const *res = "0";

    if (is_gui())
	res = "1";

    Tcl_SetResult(i->interp, (char *)res, TCL_STATIC);
    return TCL_OK;
}

static int is_model_serverCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[])
{
    Interpreter *i = (Interpreter *)cd;
    char const *res = "0";

    if (is_model_server())
	res = "1";

    Tcl_SetResult(i->interp, (char *)res, TCL_STATIC);
    return TCL_OK;
}

static int is_model_buildCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[])
{
    Interpreter *i = (Interpreter *)cd;
    char const *res = "0";

    if (is_model_build())
	res = "1";

    Tcl_SetResult(i->interp, (char *)res, TCL_STATIC);
    return TCL_OK;
}

static int exitCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[])
{
    Interpreter *i = (Interpreter *)cd;
    Tcl_SetResult(i->interp, (char *)"Access:exit: disabled", TCL_STATIC);
    return TCL_ERROR;
}

void Interpreter::SaveOldInfoCmd(void)
{
    // This is simplest way to avoid problem of getting info about 
    // command, substituting and than calling original command using 
    // object call methods.  Problem is that TCL keeps pointers to 
    // hash table entry which is invalidated by our new procedure
    // so we end with TCL calling our function back instead of original one.
    TclRenameCommand(interp, "info", "__InFo__");     
    Tcl_GetCommandInfo(interp, "__InFo__", &orig_info_cmd);
}

int Interpreter::PutsCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[])
{
    int result;

    Interpreter *i = (Interpreter *)cd;
    int     ii  = 1;
    int newline = 1;
    if ((argc >= 2) && (strcmp(argv[1], "-nonewline") == 0)) {
	newline = 0;
	ii++;
    }
    if ((ii < (argc-3)) || (ii >= argc)) {
	Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
			 " ?-nonewline? ?fileId? string\"",  NULL);
	return TCL_ERROR;
    }
    
    if (ii == (argc-1)) {
	(*i->output_stream) << argv[ii] << flush;
	if (newline)
	    (*i->output_stream) << endl;
	result = TCL_OK;
	Tcl_SetResult(i->interp, (char *)"", TCL_STATIC);
    } else
	result = (*i->puts_info.proc)(i->puts_info.clientData, i->interp, argc, argv);
    return result;
}

int Interpreter::ChangePromptCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[])
{
    int result;

    Interpreter *i = (Interpreter *)cd;
    if (argc == 2) {
	char* newString = new char[strlen(argv[1]) + 1];
	strcpy(newString, argv[1]);
	i->SetPrompt(newString);
	result = TCL_OK;
    }
    else
	result = TCL_ERROR;

    return result;
}

void Interpreter::OverrideLLengthCommand()
{
    Initialize(Interpreter::OverrideLLengthCommand);

    TclRenameCommand(interp, "llength", "__LlEnGtH__");
    if(Tcl_GetCommandInfo(interp, "__LlEnGtH__", &llength_info) == 0)
	return;
    Tcl_CreateObjCommand(interp, "llength", LLengthCmd, (ClientData)this, NULL);
}

void Interpreter::OverrideLIndexCommand()
{
    Initialize(Interpreter::OverrideLIndexCommand);

    TclRenameCommand(interp, "lindex", "__LiNdEx__");
    if(Tcl_GetCommandInfo(interp, "__LiNdEx__", &lindex_info) == 0)
	return;
    Tcl_CreateObjCommand(interp, "lindex", LIndexCmd, (ClientData)this, NULL);
}

void Interpreter::OverrideForEachCommand()
{
    Initialize(Interpreter::OverrideForEachCommand);

    TclRenameCommand(interp, "foreach", "__FoReAcH__");
    if(Tcl_GetCommandInfo(interp, "__FoReAcH__", &foreach_info) == 0)
	return;
    Tcl_CreateCommand(interp, "foreach", ForEachCmd, (ClientData)this, NULL);
}

void Interpreter::OverridePutsCommand()
{
    Initialize(Interpreter::OverridePutsCommand);

    TclRenameCommand(interp, "puts", "__pUtS__");
    if(Tcl_GetCommandInfo(interp, "__pUtS__", &puts_info) == 0)
	return;
    Tcl_CreateCommand(interp, "puts", PutsCmd, (ClientData)this, NULL);
}

void Interpreter::InitAttributes()
{
    return;

    // sschmidt: this code is rather odd, eh?  Investigate . . .
    Obj *int_attrs = symbolAttribute::get_int_attrs();
    Obj *el1;
    ForEach(el1,*int_attrs){
	symbolAttribute *attr    = (symbolAttribute *)el1;
	Tcl_CreateCommand(interp, attr->get_name(), AttributeCmd, (ClientData)attr, NULL);
    }
    Obj *str_attrs = symbolAttribute::get_string_attrs();
    Obj *el2;
    ForEach(el2,*str_attrs){
	symbolAttribute *attr    = (symbolAttribute *)el2;
	Tcl_CreateCommand(interp, attr->get_name(), AttributeCmd, (ClientData)attr, NULL);
    }
}

int Interpreter::ParseArgString(char const *str, symbolArr& res)
{
    int result = TCL_ERROR;

    int  argc;
    char const **argv;

    if (Tcl_SplitList(interp, str, &argc, &argv) == TCL_OK) {
	result = ParseArguments(0, argc, argv, res);
//
// -- NOTE: leak this memory until we figure out why the Tcl
//          allocated memory cannot be free'd
//
//#ifndef _WIN32
	Tcl_Free((char *)argv);
//#endif
    }

    return result;
}

int Interpreter::AttributeCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[])
{
    Interpreter *i        = GET_INTERPRETER(interp);
    symbolAttribute* attr = (symbolAttribute*)cd;
    symbolArr res;
    if(i->ParseArguments(1, argc, argv, res) != 0)
	return TCL_ERROR;
    if(res.size() == 0)
        Tcl_SetResult(interp, (char *)NULL, NULL);
    else {
	char const *attr_value = attr->value(res[0]);
	Tcl_SetResult(interp, (char *)attr_value, TCL_VOLATILE);
    }
    return TCL_OK;
}

extern "C" int Tcl_ProcObjCmd(ClientData cd, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[]);
extern char* cli_service_name; 
static Tcl_Interp * shared_interp;

int cli_init_shared_interp(bool create_symbol_cache)
{
  Initialize(init_shared_interp);
  if(!(is_model_server() && cli_service_name && *cli_service_name))
    return TCL_OK;

  InitializeCLIInterpreter();
  int code = TCL_ERROR;
  Interpreter* shared_i = Interpreter::Create();
  Assert(shared_i);
  shared_interp = shared_i->interp;

  shared_i->SetOutputStream(cout);

  code = Tcl_Init(shared_interp);
  
  cli_set_global_var(shared_i, "cli_service", cli_service_name);
  if (create_symbol_cache) {
      //create symbol cache by sourcing "model_server.dis" script
      code = shared_i->SourceStandardScript("model_server.dis");
      if(code != TCL_OK) {
          cerr << "Failed to initialize model_server." << endl;
          OS_dependent::_exit(1);  // exit
      }
  }
  return code;
}

static int procObjCmd(ClientData cd, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[])
{
  return shared_interp ?
    Tcl_ProcObjCmd(cd, shared_interp, objc, objv) : TCL_ERROR;
}

extern "C" Tcl_Command	Tcl_FindCommand(Tcl_Interp *interp,
			    char const *name, Tcl_Namespace *contextNsPtr,
			    int flags);
int cli_handle_error(Tcl_Interp *interp, const char* cmd, const char* mesg)
{
  Tcl_AppendResult(interp, cmd, ": error: ", mesg, NULL);
  return TCL_ERROR;
}

#define unknownAssert(cond,mesg) if(!(cond)) return cli_handle_error(interp, name, mesg)

static int unknownObjCmd(ClientData cd, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[])
{
  char*name = Tcl_GetStringFromObj(objv[1], NULL);
  Tcl_CmdInfo cmd_info;
  if(shared_interp && Tcl_GetCommandInfo(shared_interp, name, &cmd_info)){
    return cmd_info.objProc(cmd_info.objClientData, interp, objc-1, objv+1);
  }

  int code;
  symbolAttribute* attr = symbolAttribute::get_by_name(name);
  if(attr){
    int argc = objc - 2;
    unknownAssert(argc < 4, "too many arguments");
    char const *argv[6];
    for(int ii=2; ii<objc; ++ii)
      argv[ii-2] = Tcl_GetStringFromObj(objv[ii], NULL);
    Interpreter*i         = GET_INTERPRETER(interp);
    symbolArr res;
    unknownAssert(i->ParseArguments(0, argc, argv, res) == 0, "wrong input");
    unknownAssert(res.size() > 0, "empty input");

    char const *attr_value = attr->value(res[0]);
    Tcl_SetResult(interp, (char *)attr_value, TCL_VOLATILE);
    code =  TCL_OK;
  } else {
    if(Tcl_GetCommandInfo(interp, "tcl_unknown", &cmd_info))
       code = cmd_info.objProc(cmd_info.objClientData, interp, objc, objv);
    else {
      genString mesg;
      mesg.printf("*** invalid command name \"%s\"", name);
      Tcl_SetResult(interp, (char *)mesg, TCL_VOLATILE);
      code =  TCL_ERROR;
    }
  }
  return code;
}
    
static int unknownCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[])
{
    Interpreter*i         = GET_INTERPRETER(interp);
    symbolAttribute* attr = symbolAttribute::get_by_name(argv[1]);
    if(!attr){
      Tcl_AppendResult(interp, "unknown command: ", argv[1], NULL);
      return TCL_ERROR;
    }

    symbolArr res;
    if(i->ParseArguments(2, argc, argv, res) != 0)
	return TCL_ERROR;
    if(res.size() == 0)
	Tcl_SetResult(interp, (char *)NULL, NULL);
    else {
      char const *attr_value = attr->value(res[0]);
      Tcl_SetResult(interp, (char *)attr_value, TCL_VOLATILE);
    }
    return TCL_OK;
}


void Interpreter::PrintSymbolArr(symbolArr& x, int print_indexes)
{
    Initialize(PrintSymbolArr);

    if (GetSortFlag()) {
      current_print_format->sort(x,x);
    }
    current_print_format->printOut(*output_stream, x, print_indexes);
}

int Interpreter::PrintCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[])
{
    CHECK_ARG_NUM("PrintCmd", 0);
    Interpreter *i = (Interpreter *)cd;
    symbolArr res;
    if(i->ParseArguments(1, argc, argv, res) != 0)
	return TCL_ERROR;
    i->PrintSymbolArr(res, 0);
    Tcl_SetResult(i->interp, (char *)NULL, NULL);
    return TCL_OK;
}

int Interpreter::NameCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[])
{
    CHECK_ARG_NUM("name", 0);
    Interpreter *i = (Interpreter *)cd;
    symbolArr res;
    if(i->ParseArguments(1, argc, argv, res) != 0)
	return TCL_ERROR;
    if(res.size() == 0)
        Tcl_SetResult(i->interp, (char *)NULL, NULL);
    else
	Tcl_SetResult(i->interp, (char *)res[0].get_name(), TCL_VOLATILE);
    return TCL_OK;
}

int Interpreter::SizeCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[])
{
    CHECK_ARG_NUM("size", 0);
    Interpreter *i = (Interpreter *)cd;
    symbolArr res;
    if(i->ParseArguments(1, argc, argv, res) != 0)
	return TCL_ERROR;
    char tmp[30];
    sprintf(tmp, "%d", res.size());
    Tcl_SetResult(i->interp, (char *)tmp, TCL_VOLATILE);
    return TCL_OK;
}

static int apply_callback(symbolArr& , int , int , int , char*val,  Interpreter*,void*data)
{
  Tcl_DString * res = (Tcl_DString *) data;
  Tcl_DStringAppendElement(res, val);
  return 0;
}

int test_evalCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[])
{
    Interpreter *i       = (Interpreter *)cd;
    if (argc < 2) {
      Tcl_SetResult(i->interp, (char *)"wrong number of arguments", TCL_STATIC);
      return TCL_ERROR;
    }
    symbolArr source_arr;
    if(i->ParseArguments(2, argc, argv, source_arr) != 0)
	return TCL_ERROR;
    int tcl_res   = TCL_OK;  
  
    char const *exp = argv[1];
    expr* expression = api_parse_expression(exp);
    
    if(!expression) {
      Tcl_AppendResult(i->interp, "bad attribute ", exp, NULL);
      return TCL_ERROR;
    }
    exprAttribute* attr = new exprAttribute(NULL, expression);
extern void tree_cur_init(symbolArr* arr);
    tree_cur_init(&source_arr);
    const char*val = attr->value(source_arr[0]);
    tree_cur_init(NULL);
    Tcl_SetResult(i->interp, (char *)val, TCL_VOLATILE);
    return tcl_res;
}

int NamesCmd(Interpreter *i, int argc, char const *argv[])
{
  int ret = TCL_ERROR;

  symbolArr res;
  if (i->ParseArguments(1, argc, argv, res) == 0) {
    Tcl_DString names;
    Tcl_DStringInit(&names);
    int sz = res.size();
    genString nm;
    for (int ii = 0; ii < sz; ii++) {
      res[ii].get_name(nm);
      Tcl_DStringAppend(&names, nm, nm.length());
      if (ii + 1 < sz)
        Tcl_DStringAppend(&names, "\n", 1);
    }
    Tcl_SetResult(i->interp, (char *)Tcl_DStringValue(&names), TCL_VOLATILE);
    Tcl_DStringFree(&names);
    ret = TCL_OK;
  }

  return ret;
}
    
int Interpreter::ApplyCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[])
{
    CHECK_ARG_NUM("apply", 2);
    Interpreter *i       = (Interpreter *)cd;
    if (argc < 2) {
      Tcl_AppendResult(i->interp, "wrong number of arguments", NULL);
      return TCL_ERROR;
    }
    symbolArr source_arr;
    if(i->ParseArguments(2, argc, argv, source_arr) != 0)
	return TCL_ERROR;
    int tcl_res   = TCL_OK;
    Tcl_DString result;
    Tcl_DStringInit(&result);

    int err_code = 0;api_sort_apply(source_arr, argv[1], true, apply_callback, NULL,&result);

    if(err_code==0){
	Tcl_SetResult(i->interp, (char *)Tcl_DStringValue(&result), TCL_VOLATILE);
    } else {
      tcl_res = TCL_ERROR;
    }
    Tcl_DStringFree(&result);
    return tcl_res;
}

int Interpreter::LLengthCmd(ClientData cd, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[])
{
    Interpreter *i = (Interpreter *)cd;

    if (objc != 2) {
	Tcl_WrongNumArgs(interp, 1, objv, "list");
	return TCL_ERROR;
    }
    Tcl_Obj *list = objv[1];
    if(list->typePtr != NULL && list->typePtr == &set_type){
	Tcl_SetIntObj(Tcl_GetObjResult(interp), 1);
	return TCL_OK;
    } else {
	return (*i->llength_info.objProc)(i->llength_info.objClientData, interp, objc, objv);
    }
}

int Interpreter::LIndexCmd(ClientData cd, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[])
{
    Interpreter *i = (Interpreter *)cd;

    if (objc < 3) {
	return (*i->lindex_info.objProc)(i->llength_info.objClientData, interp, objc, objv);
    }
    Tcl_Obj *list  = objv[1];
    Tcl_Obj *index = objv[2];
    if(list->typePtr != NULL && list->typePtr == &set_type){
	int len;
	char const *str = Tcl_GetStringFromObj(index, &len);
	if((str != NULL && strcmp(str, "end") == 0) || (str[0] == '0' && str[1] == 0))
	    Tcl_SetObjResult(interp, list);
	else
	    Tcl_SetResult(interp, (char *)"", TCL_STATIC);
	return TCL_OK;
    } else {
	return (*i->lindex_info.objProc)(i->llength_info.objClientData, interp, objc, objv);
    }
}

struct foreachgroupData {
    ocharstream inds;
    char const* exp_var;
    char const* ind_var;
    char const* body;

    foreachgroupData(char const *e, char const *i, char const *b) : 
	body(b), ind_var(i), exp_var(e) {}
    ~foreachgroupData() {}
};

static int foreachgroup_callback(symbolArr& , int gi, int li , int sz, char*val,Interpreter* i,void*d)
{
    int result = 0;

    foreachgroupData* data = (foreachgroupData *) d;
    data->inds << ' ' << gi+1;  // ind starts from 1

    if(li==(sz-1)){
	data->inds << ends;
	char const * inds = (data->inds).ptr() + 1;
	//     cout << "setvar: " << data->ind_var << ' ' << inds << endl;
	Tcl_SetVar(i->interp, data->ind_var, inds, 0);
	Tcl_SetVar(i->interp, data->exp_var, val, 0);
	result = Tcl_Eval(i->interp, data->body);
	data->inds.reset();
    }
    if(result == TCL_CONTINUE)
	result = TCL_OK;
    return result;
}

int
foreachgroupCmd(ClientData cd,
		Tcl_Interp *interp,
		int argc,
		char const *argv[])
{
  Initialize(foreachgroupCmd(ClientData, Tcl_Interp, int, char const *[]));

  if (argc > 5)
    CHECK_ARG_NUM("foreachgroup", 6);

  Interpreter *i = (Interpreter *) cd;
  ostream &os = i->GetOutputStream();

  if (argc < 5) {
    os << "foreachgroup: too few arguments" << endl;
    return TCL_OK;
  }

  symbolArr src_arr;

  if (i->ParseArguments(argc - 2, argc - 1, argv, src_arr) != 0)
    return TCL_ERROR;

  //    cout << "foreachgroup: " << argc << ' ' << argv[0] << ' ' << argv[1] << ' ' << argv[2] << endl;
  char const *en; char const *in; char const *body;

  if (argc == 6) {
    en = argv[2];
    in = argv[3];
    body = argv[5];
  } else {
    en = argv[1];
    in = argv[2];
    body = argv[4];
  }

  foreachgroupData dt(en,in,body);
  int err_code = api_sort_apply(src_arr, argv[1], false,
				foreachgroup_callback, i, &dt);

  if (err_code != TCL_ERROR) {
    Tcl_SetResult(i->interp, (char *)"", TCL_STATIC);

    if (err_code != TCL_RETURN)
      err_code = TCL_OK;
  }

  return err_code;
}

static Tcl_Obj* make_set_element(Interpreter*i,  int ii, DI_object iter_obj, Tcl_Obj *iterator_var_name)
{
  Tcl_Obj *iterator_var = Tcl_NewListObj(0, NULL);
  Tcl_IncrRefCount(iterator_var);
  Tcl_ListObjAppendElement(i->interp, iterator_var, iter_obj);
  Tcl_Obj *index_var = Tcl_NewIntObj(ii + 1);
  Tcl_ListObjAppendElement(i->interp, iterator_var, index_var);
  if(Tcl_ObjSetVar2(i->interp, iterator_var_name, NULL, iterator_var, 0) == NULL){
    Tcl_SetResult(i->interp, (char *)"couldn't set loop variable", TCL_STATIC);
    Tcl_DecrRefCount(iterator_var);
    iterator_var = NULL;
  } else {
    Tcl_Obj *objPtr = iterator_var;
    objPtr->bytes = Tcl_Alloc(16); 
    sprintf(objPtr->bytes, "_DI_%d/%d", (int)iter_obj->internalRep.longValue, ii+1);
    objPtr->length = strlen(objPtr->bytes);
  }
  return iterator_var;
}

int Interpreter::ForEachCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[])
{
    if(argc>=5)
	return foreachgroupCmd(cd,interp,argc,argv);

    int result;
    Interpreter *i = (Interpreter *)cd;
    if((argc != 4) || (strncmp(argv[2], "_DI_", 4) != 0)){
	result = (*i->foreach_info.proc)(i->foreach_info.clientData, i->interp, argc, argv);
	return result;
    }
    int val        = atoi(&argv[2][4]);
    symbolArr *src = get_DI_symarr(val);
    if(!src){
      Tcl_AppendResult(i->interp, "foreach set variable is not valid : ", argv[2], NULL);
      result = TCL_ERROR;
      return result;
    }
    int sz = src->size();
    if(sz == 0)
      return TCL_OK;

    Tcl_Obj *iterator_var_name = Tcl_NewStringObj(argv[1], -1);
    DI_object iter_obj;
    DI_object_create(&iter_obj);
    symbolArr *iter_arr = get_DI_symarr(iter_obj);
    iter_arr->insert_last(*src);
    Tcl_IncrRefCount(iter_obj);

    for(int ii = 0; ii < sz; ii++){
      Tcl_Obj *iterator_var  = make_set_element(i, ii, iter_obj, iterator_var_name);
      if(!iterator_var){
	result = TCL_ERROR;
	break;
      }
      result = Tcl_Eval(i->interp, argv[3]);
      Tcl_DecrRefCount(iterator_var);
      if (result != TCL_OK) {
	if (result == TCL_CONTINUE) {
	  result = TCL_OK;
	} else if (result == TCL_BREAK) {
	  result = TCL_OK;
	  break;
	} else if (result == TCL_ERROR) {
	  char mesg[100];
	  sprintf(mesg, "\n    (\"foreach\" body line %d)",
		  i->interp->errorLine);
	  Tcl_AddErrorInfo(interp, mesg);
	  break;
	} else {
	  break;
	}
      } /* if */
    }  /* for */
    Tcl_DecrRefCount(iter_obj);
    return result;
}

int Interpreter::OutCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[])
{
    CHECK_ARG_NUM("out", 2);
    Interpreter *i = (Interpreter *)cd;
    if(argc < 3) {
	Tcl_AppendResult(interp, "wrong number of arguments", NULL);
	return TCL_ERROR;
    }
    char const *fname = argv[1];
    ios_base::openmode omode = ios::out;
    ofstream new_stream(fname, omode);
    if(new_stream.bad()){
	Tcl_AppendResult(interp, "unable to create stream : ", fname, NULL);
	return TCL_ERROR;
    }
    ostream *cur_stream = i->output_stream;
    i->SetOutputStream(new_stream);
    int tcl_res = Tcl_Eval(interp, argv[2]);
    i->SetOutputStream(*cur_stream);
    return tcl_res;
}

int Interpreter::OutAppendCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[])
{
    CHECK_ARG_NUM("out", 2);
    Interpreter *i = (Interpreter *)cd;
    if(argc < 3) {
	Tcl_AppendResult(interp, "wrong number of arguments", NULL);
	return TCL_ERROR;
    }
    char const *fname = argv[1];
    ios_base::openmode omode = ios::out | ios::app;
    ofstream new_stream(fname, omode);
    if(new_stream.bad()){
	Tcl_AppendResult(interp, "unable to create stream : ", fname, NULL);
	return TCL_ERROR;
    }
    ostream *cur_stream = i->output_stream;
    i->SetOutputStream(new_stream);
    int tcl_res = Tcl_Eval(interp, argv[2]);
    i->SetOutputStream(*cur_stream);
    return tcl_res;
}

int Interpreter::PerformAttributeFilter(symbolAttribute *filter_attribute, symbolArr& arr)
{
    DI_object result;
    DI_object_create(&result);
    symbolArr *result_arr = get_DI_symarr(result);
    for(int ii = 0; ii < arr.size(); ii++){
	symbolPtr sym = arr[ii];
	if(filter_attribute->boolValue(sym))
	    result_arr->insert_last(sym);
    }
    SetDIResult(result);
    return TCL_OK;
}

int Interpreter::PerformExpressionFilter(char const *filter, symbolArr& arr)
{
    expr *expression = api_parse_expression(filter);
    if(expression == NULL){
	Tcl_AppendResult(interp, "expression parse error: \n", cli_error_use(), NULL);
	return TCL_ERROR;
    }
    DI_object result;
    DI_object_create(&result);
    symbolArr *result_arr = get_DI_symarr(result);
    for(int ii = 0; ii < arr.size(); ii++){
	symbolPtr sym = arr[ii];
	if(expression->boolValue(sym))
	    result_arr->insert_last(sym);
    }
    SetDIResult(result);
    delete expression;
    return TCL_OK;
}    

int Interpreter::FilterCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[])
{
    CHECK_ARG_NUM("filter", 2);
    Interpreter *i        = (Interpreter *)cd;
    if (argc < 2) {
      Tcl_AppendResult(i->interp, "wrong number of arguments", NULL);
      return TCL_ERROR;
    }
    char const *filter_function = argv[1];
    symbolArr src_arr;
    if(i->ParseArguments(2, argc, argv, src_arr) != 0)
	return TCL_ERROR;
    int tcl_res = TCL_OK;
    // First check if we have symbolAttribute with that name
    symbolAttribute *att = symbolAttribute::get_by_name(filter_function);
    if(att != NULL)
	tcl_res = i->PerformAttributeFilter(att, src_arr);
    else {
	Tcl_CmdInfo filter_info;

    	if(Tcl_GetCommandInfo(interp, filter_function, &filter_info) == 0)
	    tcl_res = i->PerformExpressionFilter(filter_function, src_arr);
	else {
	    DI_object result;
	    DI_object_create(&result);
	    DI_object target;
	    DI_object_create(&target);
	    symbolArr *result_arr = get_DI_symarr(result);
	    symbolArr *target_arr = get_DI_symarr(target);
	    char tmp[20];
	    sprintf(tmp, " _DI_%d\n", GET_DI_SET_INDEX(target)); // Potential problem here (need to pass real Tcl8.0 object)
	    genString cmd = filter_function;
	    cmd          += tmp;
	    i->ReferenceObject(target);
	    for(int ii = 0; ii < src_arr.size(); ii++){
		symbolPtr sym = src_arr[ii];
		target_arr->removeAll();
		target_arr->insert_last(sym);
		if(Tcl_Eval(i->interp, cmd) != TCL_OK){
		    DI_object_delete(result);
		    tcl_res = TCL_ERROR;
		    break;
		}
		char const *result = Tcl_GetStringResult(i->interp);
		if(result && result[0] != '0' || result[1] != 0)
		    result_arr->insert_last(sym);
	    }
	    i->DeleteDIObject(target);
	    if(tcl_res == TCL_OK)
		i->SetDIResult(result);
	}
    }
    return tcl_res;
}

int Interpreter::SourceMainScript()
{
  int nRet = SourceStandardScript("discover.dis");
  
  if (cli_buffer.size()){
      char* str = cli_buffer;
      int ret_code = interpreter_instance->EvalCmd(str);
      if(ret_code != TCL_OK){
	char const *err_info = Tcl_GetVar(interpreter_instance->interp, "errorInfo", TCL_GLOBAL_ONLY);
	if(err_info)
	  cerr << err_info << endl;
      }
      cli_buffer.reset();
    }
  return nRet;
}

int dis_script_file(char const *script, genString& fname)
{
  static char const *env_psethome = OSapi_getenv("PSETHOME");
  static char const *env_home = OSapi_getenv("HOME");
  static char const *env_fosterhome = OSapi_getenv("DISCOVER_SCRIPTS");
  static char const *env_optscripts = customize::getStrPref("configuratorOptionalScriptsDir");
  int fFileFound = 0;
  if( env_home )
  {
    fname = env_home;
    fname += "/.";
    fname += script;
    fFileFound = (OSapi_access(fname, R_OK) == 0);
  }
  if (!fFileFound && env_fosterhome) 
  {
    fname  = env_fosterhome;
    fname += "/";
    fname += script;
    fFileFound = (OSapi_access(fname, R_OK) == 0);
  }

  if (!fFileFound && env_optscripts) 
  {
    fname  = env_optscripts;
    fname += "/";
    fname += script;
    fFileFound = (OSapi_access(fname, R_OK) == 0);
  }

  if (!fFileFound && env_psethome) 
  {
    fname  = env_psethome;
    fname += "/lib/";
    fname += script;
    fFileFound = (OSapi_access(fname, R_OK) == 0);
  }
  if(!fFileFound)
      fname = NULL;
  else {
      genString tmp = OSPATH(fname);
      fname         = tmp;
  }
  return fFileFound;
}

static const char* indent(int depth)
{
  static char const *indents[6] = {"", " ", "  ", "   ", "    ", "     "};
  if (depth > 5) depth = 5;
  return indents[depth];
}

int dis_source_script(Interpreter*i, char const *script, genString&fname, int ignore)
{
    static int depth = 0;
    int nRet = TCL_ERROR;

    if (!script)
	return nRet;

    int fFileFound = dis_script_file(script, fname); 
    if( fFileFound ) {
	msg("Sourcing $1", normal_sev) << (char *)fname << eom;
	++ depth;
	nRet = i->EvalFile(fname);
	-- depth;
	if(nRet != TCL_OK){
	    char const *err_info = Tcl_GetVar(i->interp, "errorInfo", TCL_GLOBAL_ONLY);
	    if(err_info)
		cerr << err_info << endl;
	    depth = 0;
	}
    } else {
	if(ignore) 
	    nRet = TCL_OK;
	else
	    cerr << "File " << script << " does not exist" << endl;
    }
    mpb_refresh();
    return nRet;
}

int Interpreter::SourceStandardScript(char const *script)
{
  genString fname;
  return dis_source_script(this, script, fname, 0);
}

void start_metering();
static int start_meteringCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[])
{
  start_metering();
  return 0;  
}

void stop_metering(char const * root_fcn);
static int stop_meteringCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[])
{
  if(argc==1)
    stop_metering(0);
  else for (int ii=1;ii<argc; ++ii)
    stop_metering(argv[ii]);
  return 0;  
}

static int SetNewCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[])
{
  return (argc==1) ?
    Interpreter::SetNilCmd(cd,interp,argc,argv)
      :
    Interpreter::SetCopyCmd(cd,interp,argc,argv);
}

static int source_disCmd(Interpreter *i, int argc, char const *argv[])
{
  if(argc < 2)
    return TCL_OK;
  int ii = 1;
  int ignore = 0;
  if(strcmp(argv[1], "-ignore") == 0){
    ++ii;
    ignore = 1;
  }
  genString fname;

  for(; ii<argc; ++ii){
    const char* name = argv[ii];
    int err = dis_source_script(i, name, fname, ignore);
    if(err != TCL_OK){
      msg("Errors in file $1. Aborting sourcing.", error_sev) << name << eoarg << eom;
      return err;
    }
  }
  Tcl_SetResult(i->interp, (char *)fname, TCL_VOLATILE);
  return TCL_OK;
}

static int puts_verboseCmd(Interpreter *i, int argc, char const *argv[])
{
  if(argc != 2)
    return TCL_ERROR;

  const char* mesg = argv[1];
  if( (mesg!=NULL) && (strlen(mesg)>0) )
      msg("$1", normal_sev) << mesg << eoarg << eom;
  return TCL_OK;
}
static int source_findCmd(Interpreter *i, int argc, char const *argv[])
{
  if(argc < 2)
    return TCL_OK;
  genString path;
  for(int ii=1; ii<argc; ++ii){
    const char* name = argv[ii];
    int found = dis_script_file(name, path);
    if(found){
      if(ii > 1)
	Tcl_AppendResult(i->interp, " ", NULL);
      Tcl_AppendResult(i->interp, (char*)path, NULL);
    }
  }
  return TCL_OK;
}

static int forkCmd(Interpreter* i, int argc, char const *argv[])
{
  if(argc != 2) {
    ostream& os = i->GetOutputStream();
    os << "usage: fork <command>" << endl;
    return (argc==1) ? TCL_OK : TCL_ERROR;
  }
#ifndef _WIN32
  // if call to fork is removed, or made machine-independent, remove inclusion of unistd.h!
  int pid = fork();

  int code;
  if(pid) {   // this process
    if (pid < 0) {
      ostream & os = i->GetOutputStream();
      os << "*** fork failed" << endl;
      code = i->EvalCmd(argv[1]);
    } else {
      code = TCL_OK;
    }
    return code;
  } else {       // child
    code = i->EvalCmd(argv[1]);
    i->GetOutputStream() << flush;
    setpgrp(0, 0);   // reparent to avoid zombie state on exit
    _exit(code); // 'low level' exit to avoid 'cleanup'
  }
  return 0; // Unreachable;  we return, or exit, above
#else
  return 0;
#endif
}

extern void node_prt(const Obj* obj, ostream& os);

int nodePrtCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[])
{
  Interpreter *i = (Interpreter *)cd;
  symbolArr res;
  int start = 1;
  bool obj_prt = false;
  if(argc > 1){
     char const * arg = argv[1];
     if(strcmp(arg, "-obj")==0){
       start++;
       obj_prt = true;
     }
  }
  if(i->ParseArguments(start, argc, argv, res) != 0)
    return TCL_ERROR;
  int sz = res.size();
  ostream& os = i->GetOutputStream();
  for(int ii=0; ii<sz; ++ii){
    symbolPtr& sym = res[ii];
    if(obj_prt || sym.relationalp()){
      Obj*obj = sym;
      node_prt(obj, os);
    } else {
      sym.print(os);
    }
  }
  return TCL_OK;
}

extern void InitializeSocketServer(const char* service);
extern int check_socket_client_canceled(Interpreter *i);

static int clientCanceledCmd(Interpreter* i, int argc, char const **argv)
{
  if(argc != 1) {
    Tcl_SetResult(i->interp, (char *)"client_canceled: does not take any arguments", TCL_STATIC);
    return TCL_ERROR;
  }
  int canceled = check_socket_client_canceled(i);
  char const *res    = "0";
  if(canceled)
    res = "1";
  Tcl_SetResult(i->interp, (char *)res, TCL_STATIC);
  return TCL_OK;
}

extern int add_client_notificator(int port, Interpreter *i);

static int addClientNotificatior(Interpreter* i, int argc, char const **argv)
{
    if(argc != 2) {
	Tcl_SetResult(i->interp, (char *)"usage: add_client_notificator port", TCL_STATIC);
	return TCL_ERROR;
    }
    int port   = atoi(argv[1]);
    int socket = add_client_notificator(port, i);
    genString result;
    result.printf("%d", socket);
    Tcl_SetResult(i->interp, (char *)result, TCL_VOLATILE);
    return TCL_OK;
}

static int sendNotification(Interpreter* i, int argc, char const **argv)
{
    if(argc < 2) {
	Tcl_SetResult(i->interp, (char *)"usage: send_notification string", TCL_STATIC);
	return TCL_ERROR;
    }
    char const *reply = send_client_notification(argv[1]);
    if(reply == NULL)
	Tcl_SetResult(i->interp, (char *)"", TCL_STATIC);
    else 
	Tcl_SetResult(i->interp, (char *)reply, TCL_VOLATILE);
    return TCL_OK;
}

extern int get_server_port(void);

static int getServerPortCmd(Interpreter* i, int argc, char const **argv)
{
    if(argc != 1) {
	Tcl_SetResult(i->interp, (char *)"usage: get_server_port", TCL_STATIC);
	return TCL_ERROR;
    }
    genString port_str;
    port_str.printf("%u", get_server_port());
    Tcl_SetResult(i->interp, (char *)port_str, TCL_VOLATILE);
    return TCL_OK;
}

extern "C" char* OSapi_setenv(const char* name, const char* val)
{
  if (!val) return 0;
  int len = strlen(name) + strlen(val) + 2;
  char *new_val = (char *)malloc(len);
  OSapi_sprintf(new_val,"%s=%s",name,val);

  int nonzeroerror = OSapi_putenv(new_val);
  if (nonzeroerror) {
    free(new_val);
    new_val = NULL;
  }
  return new_val;
}

// extern "C" char const *tclExecutableName;

Interpreter *InitializeCLIInterpreter(void)
{
    Initialize(InitializeInterpreter);
    if(interpreter_instance)
      return interpreter_instance;

    
#if 0
    wftask_commands_flag = 1; // dummy operation (to make stupid linker link thouse files)
    wfnode_commands_flag = 1; 
    xxaccess_dummy_var = 1;  // see above comment, and the comment in License.C
    message_dummy_var = 1;
    querytree_init_flag = 1;
    subchk_init_flag = 1;
#endif

    //    tclExecutableName =  get_exec_path();

// move to service_register //    init_shared_interp();

    interpreter_instance = Interpreter::Create();
    if(interpreter_instance == NULL){
	printf("Can`t create Access interpreter\n");
	return NULL;
    }
    if (is_gui())
      Tk_Init(interpreter_instance->interp);
    interpreter_instance->SetOutputStream(cout);
    scope_mgr.update();

    mpb_incr();
    interpreter_instance->SourceMainScript();
    mpb_incr();
    interpreter_instance->SourceStandardScript("smgr.dis");
    mpb_incr();
    InitializeSocketServer("");

    return interpreter_instance;
}

static SIG_TYP old_sigchild = NULL;

void release_sigchild(void)
{
#ifndef _WIN32
    if(old_sigchild != NULL)
	return;
    old_sigchild = OSapi_signal(SIGCHLD, SIG_DFL);
#endif
}

void restore_sigchild(void)
{
#ifndef _WIN32
    OSapi_signal(SIGCHLD, (SIG_TYP)old_sigchild);
    old_sigchild = NULL;
#endif
}

int cli_eval_string(const char* cmd, Interpreter *i)
{
    release_sigchild();
    int retval = i->EvalCmd((char*)cmd);
    restore_sigchild();
    return retval;
}

int cli_eval_string(const char* cmd)
{
    InitializeCLIInterpreter();
    return cli_eval_string(cmd, interpreter_instance);
}

void eval_test()
{
    Interpreter *i  = interpreter_instance;
    genString fname = getenv("HOME");
    fname          += "/tcltest.tcl";
    i->EvalFile(fname);
    i->SaveResult();
    genString result;
    int code = i->GetResult(result);
    if(code != TCL_OK){
	char const *err_info = Tcl_GetVar(i->interp, "errorInfo", TCL_GLOBAL_ONLY);
	if(err_info)
	    cout << err_info << endl;
    } else
	cout << (char *)result << flush;
}

void cli_eval_file(char const *fname, ostream& out_stream, Interpreter *interpreter)
{
    Interpreter *i;
    if(interpreter == NULL)
	i = InitializeCLIInterpreter();
    else 
	i = interpreter;
    i->SetOutputStream(out_stream);
    release_sigchild();
    int code = i->EvalFile(fname);
    restore_sigchild();
    if(code != TCL_OK){
	char const *err_info = Tcl_GetVar(i->interp, "errorInfo", TCL_GLOBAL_ONLY);
	if(err_info)
	    out_stream << err_info << endl;
    } else{
	i->PrintResult();
	i->SaveResult();
	out_stream << flush;
    }   
}

extern void client_set_scope(liteScope * scope);

int cli_process_input(char const *data, ostream& out_stream, Interpreter *interpreter, int do_prompt_and_print) {
   	Initialize(cli_process_input);
/*
	// we do not check access license anymore
	static int has_licence = 0;
	if(has_licence == 0){
	    if(is_gui()) {
    		if(_lo(LIC_ACCESS)){  // check out 
				_lm(LIC_ACCESS);   // error msg 
				return 8;
	      	}
    	}
	    has_licence = 1;
	}
 */ 
	Interpreter *i;
	if(interpreter == NULL)
	  i = interpreter_instance;
	else
	  i = interpreter;
	i->command += data;
	int code = -1;  // command not complete
	if(Tcl_CommandComplete(i->command)){
    	i->SetOutputStream(out_stream);
    	release_sigchild();
    	genError* err;
    	genTry {
      		client_set_scope(i->GetScope());
      		code = i->EvalCmd(i->command);
      		if(code != TCL_OK){
				char const *err_info = Tcl_GetVar(i->interp, "errorInfo", TCL_GLOBAL_ONLY);
				if(err_info)
	  				out_stream << err_info << endl;        
      		} else {
				if(do_prompt_and_print)
	  				i->PrintResult();
				i->SaveResult();
      		}
    	} genCatch(err) {
      		code = TCL_ERROR;
      		out_stream << "***error" << endl;
    	} genEndtry;
    
    	restore_sigchild();
    	if(do_prompt_and_print)
      		i->PrintPrompt();
    	i->command = "";
  	}
  	return code;
}

static int mainInterpreterEvalCmd(Interpreter* i, int argc, char const **argv)
{
    if(argc <= 1)
	return TCL_OK;
    genString tmp = argv[1];
    for(int ii = 2; ii < argc; ii++) {
	tmp += " ";
	tmp += argv[ii];
    }
    int ret_code = cli_eval_string(tmp, interpreter_instance);
    interpreter_instance->GetResult(tmp);
    Tcl_SetResult(i->interp, (char *)tmp, TCL_VOLATILE);
    return ret_code;
}

extern "C" int api_arg_to_obj(const char* str)
{
    int obj= -1;
    int nob = OSapi_sscanf(str, "_DI_%d", &obj);
    return nob ? obj : -1;
}

static int sappendCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[])
{
    Interpreter *i = (Interpreter *)cd;
    
    if(argc < 2) {
	Tcl_AppendResult(interp, argv[0], " usage: sappend <reference to set> ...", NULL);
	return TCL_ERROR;
    }
    int obj = api_arg_to_obj(argv[1]);
    if(obj < 0) {
	Tcl_AppendResult(interp, argv[0], ": first argument should reference set", NULL);
	return TCL_ERROR;
    }
    symbolArr& arg1 = * get_DI_symarr(obj);
    symbolArr add;
    if(i->ParseArguments(2, argc, argv, add) != 0)
	return TCL_ERROR;
    arg1.insert_last(add);
    return TCL_OK;
}

static int set_uniqueCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[])
{
    Interpreter *i = (Interpreter *)cd;
    symbolArr arr;
    if(i->ParseArguments(1, argc, argv, arr) != 0)
	return TCL_ERROR;
    DI_object results;
    DI_object_create(&results);
    symbolArr & result_arr = *get_DI_symarr(results);
    symbolPtr sym;
    symbolSet reset;
    reset.insert(arr);  
    result_arr = reset;
    i->SetDIResult(results);
    return TCL_OK;
}

static int set_reverseCmd(Interpreter *i, int argc, char const *argv[])
{
    symbolArr arr;
    if(i->ParseArguments(1, argc, argv, arr) != 0)
	return TCL_ERROR;
    DI_object results;
    DI_object_create(&results);
    symbolArr & result_arr = *get_DI_symarr(results);
    int sz = arr.size();
    for(int ii=sz-1; ii>=0;--ii){
      result_arr.insert_last(arr[ii]);
    }

    i->SetDIResult(results);
    return TCL_OK;
}

struct objValue : public objDictionary {
  genString value;
  objValue(const char*name, const char*val) : objDictionary(name), value(val)
    {}
};

static objNameSet globals;
static void var_set(const char*name, const char*val)
{
  objValue* ov = (objValue*) globals.lookup(name);
  if(!ov){
    ov = new objValue(name, val);
    globals.insert(ov);
  } else {
    ov->value = val;
  }
}

static const char* var_get(const char*name)
{
    const char*val = "";
    objValue* ov = (objValue*) globals.lookup(name);
    if(!ov){
	ov = new objValue(name, val);
	globals.insert(ov);
    } else {
	val = ov->value;
    }
    return val;
}

static int setvarCmd(Interpreter *i, int argc, char const *argv[])
{
    symbolArr arr;
    if(argc !=3){
      	Tcl_AppendResult(i->interp, "wrong number of arguments", NULL);
	return TCL_ERROR;
    }
    const char* name = argv[1];
    const char* val = argv[2];

    var_set(name, val);
    Tcl_AppendResult(i->interp, val, NULL);
    return TCL_OK;
}

static int getvarCmd(Interpreter *i, int argc, char const *argv[])
{
    symbolArr arr;
    if(argc !=2){
      	Tcl_AppendResult(i->interp, "wrong number of arguments", NULL);
	return TCL_ERROR;
    }
    const char* name = argv[1];
    const char* val = var_get(name);
    Tcl_AppendResult(i->interp, val, NULL);
    return TCL_OK;
}

extern "C" int cli_get_link(int src, int targ, int link);

int cli_get_link_cmd(int link, ClientData cd, Tcl_Interp *interp, int argc, char const *argv[])
{
    Interpreter *i = (Interpreter *)cd;
    symbolArr arr;
    if(i->ParseArguments(1, argc, argv, arr) != 0)
	return TCL_ERROR;
    
    DI_object src_obj;
    DI_object_create(&src_obj);
    (*get_DI_symarr(src_obj)) = arr;
    
    DI_object results;
    DI_object_create(&results);
    
    cli_get_link(GET_DI_SET_INDEX(src_obj), GET_DI_SET_INDEX(results), link);
    DI_object_delete(src_obj);
    i->SetDIResult(results);
    
    return TCL_OK;  
}

void *make_tcl_string(char const *str)
{
    Tcl_DString *tcl_str;

    tcl_str = new Tcl_DString;
    if(tcl_str == NULL)
	return NULL;
    Tcl_DStringInit(tcl_str);
    Tcl_DStringAppendElement(tcl_str, str);
    return tcl_str;
}

char const *get_tcl_string(void *_tcl_str)
{
    return Tcl_DStringValue((Tcl_DString *)_tcl_str);
}

void delete_tcl_string(void *_tcl_str)
{
    Tcl_DString *tcl_str = (Tcl_DString *)_tcl_str;
    Tcl_DStringFree(tcl_str);
    delete tcl_str;
}

static DI_object object_stack[5];

static int diGetArgCmd(Interpreter* i, int argc, char const **argv)
{
    if(argc != 2){
	Tcl_SetResult(i->interp, (char *)"__diGetArg: wrong number of arguments", TCL_STATIC);
	return TCL_ERROR;
    }
    int arg_num = atoi(argv[1]);
    if(object_stack[arg_num] == NULL){
	Tcl_SetResult(i->interp, (char *)"__diGetArg: argument does not exists", TCL_STATIC);
	return TCL_ERROR;
    }
    i->SetDIResult(object_stack[arg_num]);
    return TCL_OK;
}

int call_cli_callback(Interpreter *i, char const *command, symbolArr *param_arr1, symbolArr *param_arr2)
{
    Interpreter * prev_interpreter = GetActiveInterpreter();
    if(i == NULL)
	i = interpreter_instance;
    SetActiveInterpreter(i);
    DI_object arr1;
    DI_object_create(&arr1);
    symbolArr *arr1_arr = get_DI_symarr(arr1);
    DI_object arr2;
    DI_object_create(&arr2);
    symbolArr *arr2_arr = get_DI_symarr(arr2);
    i->ReferenceObject(arr1);
    i->ReferenceObject(arr2);
    object_stack[0] = arr1;
    object_stack[1] = arr2;
    genString cmd;
    if(param_arr1){
	arr1_arr->insert_last(*param_arr1);
	if(param_arr2){
	    arr2_arr->insert_last(*param_arr2);
	    cmd.printf("%s [__diGetArg 0] [__diGetArg 1]", command);
	} else {
	    cmd.printf("%s [__diGetArg 0]", command);
	}
    } else {
	cmd.printf("%s", command);
    }
    int ret_val = 0;
    release_sigchild();
    int result = i->EvalCmd(cmd) == TCL_OK;
    restore_sigchild();
    if(result) {
	// Try to preserve object returned from callback
	// Do not convert it to string, because somebody may need it later
	DI_object val = Tcl_GetObjResult(i->interp);
	if(val->typePtr && val->typePtr == &set_type) {
	    ret_val = 1;
	}  else {
	    char const *str = Tcl_GetStringResult(i->interp);
	    if(str)
		ret_val = OSapi_atoi(str);
	}
    } else {
	char const *str = Tcl_GetStringResult(i->interp);
	if(str == NULL)
	    str = "";
	OSapi_printf("Following error(s) returned from callback \"%s\" :\n%s\n", command, str);
	ret_val = 0;
    }
    i->DeleteDIObject(arr1);
    i->DeleteDIObject(arr2);
    object_stack[0] = NULL;
    object_stack[1] = NULL;
    SetActiveInterpreter(prev_interpreter);
    return ret_val;
}

int call_cli_callback(Interpreter *i, char const *proc, char const *chr_args, symbolArr *param_arr1, symbolArr *param_arr2)
{
    Tcl_CmdInfo cmd_info;

    if(i == NULL)
	    i = interpreter_instance;
    if(Tcl_GetCommandInfo(i->interp, proc, &cmd_info) == 0)
	    return 0;
    genString command;
    command.printf("%s %s", proc, chr_args);
    int ret_val = call_cli_callback(i, (char const *)command, 
				    param_arr1, param_arr2);
    return ret_val;
}

static int get_prefValueCmd(Interpreter *i, int argc, char const *argv[])
{
    if(argc != 2){
	Tcl_SetResult(i->interp, (char *)"get_prefValueCmd needs 1 argument", TCL_STATIC);
	return TCL_ERROR;
    }
    char const *key = argv[1];
    const char*val = customize::getStrPref(key);
    if(val)
	Tcl_SetResult(i->interp, (char *)val, TCL_VOLATILE);
    return TCL_OK;
}
static int set_prefValueCmd(Interpreter *i, int argc, char const *argv[])
{
    if(argc != 3){
	Tcl_SetResult(i->interp, (char *)"set_prefValueCmd needs 2 arguments", TCL_STATIC);
	return TCL_ERROR;
    }
    char const *key = argv[1];
    genString   val = argv[2];
    customize::putPref(key, val);
    return TCL_OK;
}

#define SHOW_GRP_CMD(nm) \
static int paste3(SHOW_GRP_,nm,_CMD)(ClientData cd, Tcl_Interp *itp, int argc, char const **argv)\
{ return cli_get_link_cmd(paste(SHOW_GRP_,nm),cd,itp,argc,argv);}

SHOW_GRP_CMD(MEMBERS)
SHOW_GRP_CMD(PEERS)
SHOW_GRP_CMD(CLIENTS)
SHOW_GRP_CMD(SERVERS)
SHOW_GRP_CMD(OF_MEMBER)

static int cliCommandEval(ClientData cd, Tcl_Interp *interp, int argc, char const **argv)
{
    cliCommandBase*c = (cliCommandBase*) cd;
    Interpreter* i   = GET_INTERPRETER(interp);
    int code;
    genError* err;
    genTry {
	code = c->eval(i, argc, argv);
    } genCatch(err) {
	code = TCL_ERROR;
    } genEndtry;
    return code;
}

static void cliInstallCommand(Interpreter*i, cliCommandBase*c)
{
  Tcl_CreateCommand(i->interp, c->get_name(), cliCommandEval, (ClientData)c, NULL);
}

void Interpreter::InitBuiltinFunctions()
{
    Initialize(Interpreter::InitBuiltinFunctions);
    
    OverrideForEachCommand();
    OverridePutsCommand();
    OverrideLLengthCommand();
    OverrideLIndexCommand();
    // Add  commands 
    if (cli_comm_array) {
	int size = cli_comm_array->size();
	for (int ii=0; ii<size; ii++) {
	    cliCommandBase *c = (cliCommandBase*)(*cli_comm_array)[ii];
	    cliInstallCommand(this, c);
	}
    }
    if(!is_model_build()){
      Tcl_Init(interp);
      TclRenameCommand(interp, "unknown", "tcl_unknown");
    }

    Tcl_CreateObjCommand(interp, "unknown", unknownObjCmd, (ClientData)NULL, NULL);
    Tcl_CreateObjCommand(interp, "shared_proc", procObjCmd, (ClientData)NULL, NULL);

}

Interpreter::~Interpreter() {
  if(interp){
	// delete interpreter
	Tcl_DeleteInterp(interp);
	interp=NULL;
  }
}

Interpreter::Interpreter() :  interp(Tcl_CreateInterp())
{
  if(interp){
    Tcl_SetAssocData(interp, "Interpreter", NULL, this);
    scope = NULL;
  }
}

static symbolSet setset;
static int setinitCmd(Interpreter*i, int argc, char const **argv)
{
    setset.removeAll();
    return TCL_OK;
}

static int setinsertCmd(Interpreter*i, int argc, char const **argv)
{
    symbolArr arr;
    if(i->ParseArguments(1, argc, argv, arr) != 0)
	return TCL_ERROR;
    int sz = arr.size();
    DI_object obj;
    DI_object_create(&obj);
    symbolArr& res = *get_DI_symarr(obj);

    for(int ii=0; ii<sz; ++ii){
	const symbolPtr& sym = arr[ii];
	if(setset.insert(sym))
	    res.insert_last(sym);
    }
    i->SetDIResult(obj);
    return TCL_OK;
}

static int setincludesCmd(Interpreter*i, int argc, char const **argv)
{
    symbolArr arr;
    if(i->ParseArguments(1, argc, argv, arr) != 0)
	return TCL_ERROR;
    int sz = arr.size();

    DI_object obj;
    DI_object_create(&obj);
    symbolArr& res = *get_DI_symarr(obj);

    for(int ii=0; ii<sz; ++ii){
	const symbolPtr& sym = arr[ii];
	if(setset.includes(sym))
	    res.insert_last(sym);
    }
    i->SetDIResult(obj);
    return TCL_OK; 
}

static int setgetCmd(Interpreter*i, int argc, char const **argv)
{
    DI_object obj;
    DI_object_create(&obj);
    symbolArr& arr = *get_DI_symarr(obj);
    arr = setset;
    i->SetDIResult(obj);
    return TCL_OK;
}

static symbolArr arrarr;
static int arrinitCmd(Interpreter*i, int argc, char const **argv)
{
    arrarr.removeAll();
    return TCL_OK;
}

static int arrinsertCmd(Interpreter*i, int argc, char const **argv)
{
    symbolArr arr;
    if(i->ParseArguments(1, argc, argv, arr) != 0)
	return TCL_ERROR;

    arrarr.insert_last(arr);

    return TCL_OK;
}

static int arrgetCmd(Interpreter*i, int argc, char const **argv)
{
    DI_object obj;
    DI_object_create(&obj);
    symbolArr& arr = *get_DI_symarr(obj);
    if (argc==1)
      arr = arrarr;
    else
      for(int ii=1, sz=arrarr.size(); ii<argc; ++ii){
	int ind = OSapi_atoi(argv[ii]);
	if(ind>0 && ind <=sz)
	  arr.insert_last(arrarr[ind-1]);
      }
    i->SetDIResult(obj);
    return TCL_OK;
}

static int file_rmCmd(Interpreter*i, int argc, char const **argv)
{
    for(int ii=1; ii<argc; ++ii)
	OSapi_unlink(argv[ii]);
    return TCL_OK;
}

extern bool get_type_sym(const symbolPtr &, symbolPtr &);

static int TypesymCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[])
{
  Interpreter *i = GET_INTERPRETER(interp);

  DI_object obj;
  DI_object_create(&obj);
  symbolArr &res = *get_DI_symarr(obj);

  symbolArr sel;
  if (i->ParseArguments(1, argc, argv, sel) == 0) {
    symbolPtr sym, type;
    ForEachS(sym, sel) {
      if (get_type_sym(sym, type) == true)
        res.insert_last(type);
    }
  }

  i->SetDIResult(obj);

  return TCL_OK;
}

//----------------------------
xrefSymbol* addSymbolCmd_impl( ddKind kind, const char* sym_str );

int addSymbolCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[])
{
  Initialize(addSymbolCmd);

  int ret = TCL_ERROR;
  Interpreter *i = (Interpreter *)cd;

  genString message;
  DI_object obj;
  DI_object_create(&obj);
  symbolArr& arr = *get_DI_symarr(obj);
  ddSelector sel;

  int argpos = i->SetupSelector(sel, argc, argv);

  if (argc < 3 || argpos < 0 ) {
    message.printf("Usage: %s selector symbol", argv[0]);
    Tcl_AppendResult(interp, message.str(), NULL);
  }
  else {
    char const *sym_str = argv[argpos];
    int k;
    for ( k= 0; k < SELECT_MAX; k++  ) {
      if ( sel.selects( ddKind(k) ) )
	break;
    }

    if ( k == SELECT_MAX ) 
      Tcl_AppendResult(interp, "not valid selector", NULL);
    else {
      symbolPtr sym = addSymbolCmd_impl( ddKind(k), sym_str );
      if ( sym.isnotnull() ) {
	  arr.insert_last(sym);
	  ret = TCL_OK;
        }
    }
  }

  i->SetDIResult(obj);
  return ret;
}

//---------------------
xrefSymbol* addLinkCmd_impl( linkType lt, symbolPtr sym1, symbolPtr sym2 );

int addLinkCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[])
{
  Initialize(newSymbolCmd);

  int ret = TCL_ERROR;
  Interpreter *i = (Interpreter *)cd;

  DI_object obj;
  DI_object_create(&obj);
  symbolArr& arr = *get_DI_symarr(obj);

  int ilt=NUM_OF_LINKS;
  symbolArr arg2, arg3;

  genString message;
  if ( argc > 3 )
  {
    for ( ilt=0; ilt< NUM_OF_LINKS; ilt++ )
    {
      if ( strcmp( argv[1], linkType_name((enum linkType)ilt) ) == 0 )
	break;
    }
  }
  if(ilt >= NUM_OF_LINKS || i->ParseArgString(argv[2], arg2) != 0 || i->ParseArgString(argv[3], arg3) != 0)
  {
    message.printf("Usage: %s link_type symbol symbol", argv[0]);
    Tcl_AppendResult(interp, message.str(), NULL);
  }
  else {
    symbolPtr sym = addLinkCmd_impl( (enum linkType)ilt, arg2[0], arg3[0] );
    ret = TCL_OK;
  }

  i->SetDIResult(obj);
  return ret;
}

//---------------------
int att_off(char const *name)
{
  symbolAttribute* att = symbolAttribute::get_by_name(name);
  if(!att || !is_xrefAttribute(att))
    return -1;
  xrefAttribute*xatt = (xrefAttribute*) att;
  return xatt->offset;

}

int addAttrCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[])
{
  Initialize(newSymbolCmd);

  int ret = TCL_ERROR;
  Interpreter *i = (Interpreter *)cd;

  DI_object obj;
  DI_object_create(&obj);
  symbolArr& arr = *get_DI_symarr(obj);
  symbolArr sel;

  genString message;
  int isym=1;
  for ( ; isym<argc && *argv[isym] == '-'; isym++);

  if (argc < 3 || isym == 1 || isym >= argc || i->ParseArgString(argv[isym], sel)!=0 ) {
    message.printf("Usage: %s -attribute symbol", argv[0]);
    Tcl_AppendResult(interp, message.str(), NULL);
  }
  else
  {
    for ( int iattr =1; iattr< isym; iattr++)
    {
      int nattr_offset = att_off(argv[iattr]+1);
      if ( nattr_offset == -1 )
      {
	message.printf("Attribute: '%s' is not found", argv[iattr]);
	Tcl_AppendResult(interp, message.str(), NULL);
      }
      else
      {
	symbolPtr sym, type;
	ForEachS(sym, sel) {
	  sym->set_attribute(1, nattr_offset, 1);
	}
      }
    }
    ret = TCL_OK;
  }

  i->SetDIResult(obj);
  return ret;
}

/////////////////////////////////////////////////////////////////////////////
//  Standard dispatch ID constants
struct tagDISPID_TEXT {
  const char*	text;
  int		id;
} dispidStandard[] =
{
  { "DISPID_AUTOSIZE",                 (-500) },
  { "DISPID_BACKCOLOR",                (-501) },
  { "DISPID_BACKSTYLE",                (-502) },
  { "DISPID_BORDERCOLOR",              (-503) },
  { "DISPID_BORDERSTYLE",              (-504) },
  { "DISPID_BORDERWIDTH",              (-505) },
  { "DISPID_DRAWMODE",                 (-507) },
  { "DISPID_DRAWSTYLE",                (-508) },
  { "DISPID_DRAWWIDTH",                (-509) },
  { "DISPID_FILLSTYLE",                (-511) },
  { "DISPID_FONT",                     (-512) },
  { "DISPID_FORECOLOR",                (-513) },
  { "DISPID_ENABLED",                  (-514) },
  { "DISPID_HWND",                     (-515) },
  { "DISPID_TABSTOP",                  (-516) },
  { "DISPID_TEXT",                     (-517) },
  { "DISPID_CAPTION",                  (-518) },
  { "DISPID_BORDERVISIBLE",            (-519) },
  { "DISPID_APPEARANCE",               (-520) },
  { "DISPID_MOUSEPOINTER",             (-521) },
  { "DISPID_MOUSEICON",                (-522) },
  { "DISPID_PICTURE",                  (-523) },
  { "DISPID_VALID",                    (-524) },
  { "DISPID_READYSTATE",               (-525) },

  { "DISPID_REFRESH",                  (-550) },
  { "DISPID_DOCLICK",                  (-551) },
  { "DISPID_ABOUTBOX",                 (-552) },

  { "DISPID_CLICK",                    (-600) },
  { "DISPID_DBLCLICK",                 (-601) },
  { "DISPID_KEYDOWN",                  (-602) },
  { "DISPID_KEYPRESS",                 (-603) },
  { "DISPID_KEYUP",                    (-604) },
  { "DISPID_MOUSEDOWN",                (-605) },
  { "DISPID_MOUSEMOVE",                (-606) },
  { "DISPID_MOUSEUP",                  (-607) },
  { "DISPID_ERROREVENT",               (-608) },
  { "DISPID_READYSTATECHANGE",         (-609) },

  { "DISPID_AMBIENT_BACKCOLOR",        (-701) },
  { "DISPID_AMBIENT_DISPLAYNAME",      (-702) },
  { "DISPID_AMBIENT_FONT",             (-703) },
  { "DISPID_AMBIENT_FORECOLOR",        (-704) },
  { "DISPID_AMBIENT_LOCALEID",         (-705) },
  { "DISPID_AMBIENT_MESSAGEREFLECT",   (-706) },
  { "DISPID_AMBIENT_SCALEUNITS",       (-707) },
  { "DISPID_AMBIENT_TEXTALIGN",        (-708) },
  { "DISPID_AMBIENT_USERMODE",         (-709) },
  { "DISPID_AMBIENT_UIDEAD",           (-710) },
  { "DISPID_AMBIENT_SHOWGRABHANDLES",  (-711) },
  { "DISPID_AMBIENT_SHOWHATCHING",     (-712) },
  { "DISPID_AMBIENT_DISPLAYASDEFAULT", (-713) },
  { "DISPID_AMBIENT_SUPPORTSMNEMONICS",(-714) },
  { "DISPID_AMBIENT_AUTOCLIP",         (-715) },
  { "DISPID_AMBIENT_APPEARANCE",       (-716) },
  { "DISPID_AMBIENT_PALETTE",          (-726) },
  { "DISPID_AMBIENT_TRANSFERPRIORITY", (-728) },

  { "DISPID_Name",                     (-800) },
  { "DISPID_Delete",                   (-801) },
  { "DISPID_Object",                   (-802) },
  { "DISPID_Parent",                   (-803) },
  { "",                                (0)    }
};

int ComMemidCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[])
{
  char szRez[24];
  Initialize(newSymbolCmd);
  Interpreter *i = (Interpreter *)cd;
  if (argc < 2 )
	  return TCL_ERROR;

  char const *p=argv[1];
  while ( *p == ' ' || *p == '\t' ) p++;
  if ( *p == '0' && ( *(p+1) == 'x' || *(p+1) == 'X') )
  {
    strcpy( szRez, p+2);
    char *p1 = szRez;
    while ( *p1 ) { *p1 = toupper(*p1); p1++; }
	  Tcl_SetResult(i->interp, (char *)szRez, TCL_VOLATILE);
    return TCL_OK;
  }

  int nMemid = atoi( argv[1] );
  if ( nMemid == 0 )
  {
    struct tagDISPID_TEXT *pdispid = dispidStandard;
    while ( pdispid->id )
    {
    	if ( OSapi_strcasecmp( argv[1], pdispid->text) == 0 )
      {
        nMemid = pdispid->id;
    	  break;
      }
    	pdispid++;
    }
  }
  if ( nMemid != 0 )
  {
    sprintf( szRez, "%X", nMemid);
	  Tcl_SetResult(i->interp, (char *)szRez, TCL_VOLATILE);
  }
  else
    Tcl_SetResult(i->interp, (char *)argv[1], TCL_VOLATILE);
   return TCL_OK;
}

int dis_infoCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[]);
static int add_commands()
{
#define CMD(x,y) new cliCommandInfo(x,y)

    CMD("TRACE", Interpreter::TraceCmd);
    CMD("roots", Interpreter::RootsCmd);
    CMD("subprojects", Interpreter::SubprojectsCmd);
    CMD("parent", Interpreter::ParentCmd);
    CMD("modules", Interpreter::FilesCmd);
    CMD("contents", Interpreter::ContentsCmd);
    CMD("defines", Interpreter::DefinesCmd);
    CMD("uses", Interpreter::UsesCmd);
    CMD("where", Interpreter::WhereCmd);
    CMD("get_include_files", Interpreter::GetIncludeFilesCmd);
    CMD("get_member_functions", Interpreter::GetMemberFunctionsCmd);
    CMD("get_member_data", Interpreter::GetMemberDataCmd);
    CMD("get_friends", Interpreter::GetFriendsCmd);
    CMD("get_generated", Interpreter::GetGeneratedCmd);
    CMD("get_macros", Interpreter::GetMacrosCmd);
    CMD("get_super_classes", Interpreter::GetSuperClassesCmd);
    CMD("get_sub_classes", Interpreter::GetSubClassesCmd);
    CMD("get_declares", Interpreter::GetDeclaresCmd);
    CMD("set_copy", Interpreter::SetCopyCmd);
    CMD("set_add", Interpreter::SetAddCmd);
    CMD("set_union", Interpreter::SetUnionCmd);
    CMD("set_unique", set_uniqueCmd);
    CMD("set_subtract", Interpreter::SetSubtractCmd);
    CMD("set_intersect", Interpreter::SetIntersectCmd);
    CMD("nil_set", Interpreter::SetNilCmd);
    CMD("new_set", SetNewCmd);
    CMD("change_prompt", Interpreter::ChangePromptCmd);
    CMD("set_intersect", Interpreter::SetIntersectCmd);
    CMD("modified_entities", Interpreter::ModifiedEntitiesCmd);
    CMD("instances", Interpreter::InstancesCmd);
    CMD("validate_inst_arr", Interpreter::ValidateInstArrCmd);
    CMD("declarations", Interpreter::DeclarationsCmd);
    CMD("where_declared", Interpreter::WhereDeclaredCmd);
    CMD("get_instance_scope", Interpreter::GetInstanceScopeCmd);
    CMD("get_instance_symbol", Interpreter::GetInstanceSymbolCmd);
    CMD("get_template", Interpreter::GetTemplateCmd);
    CMD("get_instantiations", Interpreter::GetInstantiationsCmd);
    CMD("instance_from_offset",Interpreter::InstanceFromOffsetCmd);
    CMD("packages", Interpreter::PackagesCmd);

    CMD("print", Interpreter::PrintCmd);
    CMD("printformat", Interpreter::PrintFormatCmd);
    CMD("sort", Interpreter::SortCmd);
    CMD(">", Interpreter::OutCmd);
    CMD(">>", Interpreter::OutAppendCmd);
    CMD("filter", Interpreter::FilterCmd);
    CMD("add_attribute", Interpreter::AddAttributeCmd);
    CMD("test_eval", test_evalCmd);
    CMD("apply", Interpreter::ApplyCmd);
    CMD("name", Interpreter::NameCmd);
    CMD("size", Interpreter::SizeCmd);
    CMD("node_prt", nodePrtCmd);
    CMD("sappend", sappendCmd);
    CMD("start_metering", start_meteringCmd);    
    CMD("stop_metering", stop_meteringCmd);
    CMD("exit", exitCmd);
    CMD("OS", archCmd);
    CMD("is_gui", is_guiCmd);
    CMD("is_model_server", is_model_serverCmd);
    CMD("is_model_build", is_model_buildCmd);
    
    new cliCommand("fork", forkCmd);
    new cliLinkCommand("get_subsystem_members", SHOW_GRP_MEMBERS);
    new cliLinkCommand("get_subsystem_peers", SHOW_GRP_PEERS);
    new cliLinkCommand("get_subsystem_clients", SHOW_GRP_CLIENTS);
    new cliLinkCommand("get_subsystem_servers", SHOW_GRP_SERVERS);
    new cliLinkCommand("get_subsystem_of_member", SHOW_GRP_OF_MEMBER);

    new cliLinkCommand("get_nested", SHOW_NESTED);
    new cliLinkCommand("get_nested_classes", SHOW_NESTED_CLASSES);
    new cliLinkCommand("get_nested_interfaces", SHOW_NESTED_INTERFACES);
    new cliLinkCommand("get_nested_unions", SHOW_NESTED_UNIONS);
    new cliLinkCommand("get_nested_enums", SHOW_NESTED_ENUMS);
    new cliLinkCommand("get_nested_typedefs", SHOW_NESTED_TYPEDEFS);
    new cliLinkCommand("get_containing", SHOW_CONTAINING);

    new cliLinkCommand("get_decl_usage", SHOW_DECL_USAGE);
    new cliLinkCommand("get_links", SHOW_ASSOC_OF_SYM);
    new cliLinkCommand("get_linked_entities", SHOW_SYM_OF_ASSOC);
    new cliLinkCommand("get_documents", SHOW_DOCUMENTS);

    new cliLinkCommand("get_relations", SHOW_IS_DEFINING_RELATIONS);
    new cliLinkCommand("get_overrides", SHOW_OVERRIDES);
    new cliLinkCommand("get_reference_files", SHOW_REF_FILE); //test

    new cliLinkCommand("get_super_extends", SHOW_EXTENDS_SUPERCLASS);
    new cliLinkCommand("get_super_implements", SHOW_IMPLEMENTS_SUPERINTERFACE);
    new cliLinkCommand("get_sub_extenders", SHOW_EXTENDED_BY_SUBINTERFACE);
    new cliLinkCommand("get_sub_implementers", SHOW_IMPLEMENTED_BY_SUBCLASS);

    CMD("argument_of", Interpreter::ArgumentOfCmd);
    CMD("friend_of", Interpreter::FriendOfCmd);

    CMD("info", dis_infoCmd);

    new cliCommand("set_reverse", set_reverseCmd);
    new cliCommand("setvar", setvarCmd);
    new cliCommand("getvar", getvarCmd);
    new cliCommand("source_dis", source_disCmd);
    new cliCommand("source_find", source_findCmd);
    new cliCommand("puts_verbose", puts_verboseCmd);
    new cliCommand("get_pref_value", get_prefValueCmd);
    new cliCommand("set_pref_value", set_prefValueCmd);

    new cliCommand("setinit", setinitCmd);
    new cliCommand("setinsert", setinsertCmd);
    new cliCommand("setget", setgetCmd);
    new cliCommand("setincludes", setincludesCmd);

    new cliCommand("arrinit", arrinitCmd);
    new cliCommand("arrinsert", arrinsertCmd);
    new cliCommand("arrget", arrgetCmd);

    new cliCommand("file_rm", file_rmCmd);
    // Unsorted list of names with newlines and no braces:
    new cliCommand("names", NamesCmd);

    CMD("get_category_command", BrowserController::GetCategoryCommandCmd);
    CMD("get_query_command", BrowserController::GetQueryCommandCmd);
    CMD("get_categories", BrowserController::GetCategoriesCmd);
    CMD("get_queries", BrowserController::GetQueriesCmd);

    new cliCommand("client_canceled", clientCanceledCmd);
    new cliCommand("add_client_notifier", addClientNotificatior);
    new cliCommand("send_notification", sendNotification);
    new cliCommand("get_server_port", getServerPortCmd);
    new cliCommand("main_interpreter_eval", mainInterpreterEvalCmd);
    new cliCommand("__diGetArg", diGetArgCmd);

    CMD("get_type", TypesymCmd);
    CMD("typesym", TypesymCmd);

    new cliCommandInfo("pmod_add_symbol", addSymbolCmd);
    new cliCommandInfo("pmod_add_link", addLinkCmd);
    new cliCommandInfo("pmod_add_attribute", addAttrCmd);
    new cliCommandInfo("com_memid", ComMemidCmd);

    return 0;
}

static int dummy = add_commands();
