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
#include "viewTypes.h"
#include "cLibraryFunctions.h"
#include "machdep.h"

#include <vport.h>
#include "general.h"
#include "genError.h"
#include "tcl.h"
#include "Interpreter.h"
#include "api_internal.h"
#include "genString.h"
#include "gRTListServer.h"
#ifndef ISO_CPP_HEADERS
#include <stdlib.h>
#include <ctype.h>
#else /* ISO_CPP_HEADERS */
#include <cstdlib>
using namespace std;
#include <cctype>
#endif /* ISO_CPP_HEADERS */
#include "Application.h"
#include vstrHEADER
#include "gRTListServer.h"
#include "ginterp.h"
#include <BrowserController.h>
#ifndef ISO_CPP_HEADERS
#include <fstream.h>
#else /* ISO_CPP_HEADERS */
#include <fstream>
#endif /* ISO_CPP_HEADERS */
#include <charstream.h>
#include <ide_editor.h>

#ifdef _WIN32
#include <windows.h>
#endif

extern Interpreter *interpreter_instance;

extern void  rcall_dis_DISui_eval_async (Application* app, vstr* str);
extern vstr* rcall_dis_DISui_eval(Application* app, vstr* str);
extern int parse_selection(char* str, symbolArr& result);
extern int rcall_get_rtl_info (Application* app, int& last_sel, int& elements, int& results, dyn_vstrptr& scope);

static symbolArr cli_saved_result;

void GetSavedResult(Interpreter *, symbolArr& arr)
{
    arr.insert_last(cli_saved_result);
}

void GetSavedResult(symbolArr& arr)
{
    arr.insert_last(cli_saved_result);
}

void        SetActiveInterpreter(Interpreter *i);
Interpreter *GetActiveInterpreter();

void GetAccessValue(char *var, symbolArr& arr)
{
    Interpreter *prev_int = GetActiveInterpreter();
    SetActiveInterpreter(interpreter_instance);
    char *value           = Tcl_GetVar(interpreter_instance->interp, var, 0);
    if(strncmp(value, "_DI_", 4) == 0){
    	int obj               = atoi(&value[4]);
    	symbolArr *result_arr = get_DI_symarr(obj);
    	arr                   = *result_arr;
    }
    SetActiveInterpreter(prev_int);
}

int GetGalaSelectionCmd(ClientData cd, Tcl_Interp* interp, int argc, char *argv[])
{
  Interpreter *i       = (Interpreter *)cd;
  RTListServer* server = NULL;
  
  if(argc == 1){  // No arguments supplied. Will use most recent active server RTL
    const char *command = NULL;
    symbolArr selection;
    Application *appl = Application::findApplication("DISui");
    if( appl && appl->mySession()){
      int last_sel = -1, elements = -1, results = -1;
      vstr *scope  = NULL;
      if(rcall_get_rtl_info(appl, last_sel, elements, results, scope)){ //get_rtl_info returns non-zero for success
	server = RTListServer::find(last_sel);
      }
    }
  } else {
    server = RTListServer::find (atoi (argv[1]));
  }
  DI_object result;
  DI_object_create(&result);
  if(server != NULL){
    symbolArr *result_arr = get_DI_symarr(result);
    server->getSelArr(*result_arr);
  }
  i->SetDIResult(result);
  return TCL_OK;
}

static void newui_execute_query( RTListServer *result_rtl, symbolArr &selection, const char *cmd_to_eval, int fUseSelection )
{
  Interpreter *prev_interpreter = GetActiveInterpreter();
  SetActiveInterpreter(interpreter_instance);
  release_sigchild();
  
  Application *appl = Application::findApplication("DISui");
  if( interpreter_instance && cmd_to_eval && appl && appl->mySession())
  {
    if( fUseSelection ){
      call_cli_callback(interpreter_instance, (const char *)cmd_to_eval, &selection, NULL);
    } else {
      call_cli_callback(interpreter_instance, (const char *)cmd_to_eval, NULL, NULL);
    }
    if(interpreter_instance->code == TCL_OK)
    {
      DI_object result = interpreter_instance->DecodeDIResult();
      if(result != NULL)
      {
	//Ask UI to activate Model Browser page
        vstr *cmd = vstrClone( (vchar *)"dis_activate_notebook_page ModelBrowser");
        if( cmd )
        {
          rcall_dis_DISui_eval_async(appl, cmd);
          vstrDestroy( cmd );
        }

        symbolArr& res_arr = *get_DI_symarr(result);
        if( result_rtl )
        {
          result_rtl->clear();
          result_rtl->insert( res_arr );
        }        
      }
      else 
      {
        vstr *cmd = vstrClone( (vchar *)"dis_Message {");
        cmd = vstrAppend( cmd, (vchar *)Tcl_GetStringResult(interpreter_instance->interp));
        cmd = vstrAppend( cmd, (vchar *)"}");
        if( cmd )
        {
          rcall_dis_DISui_eval_async (appl, cmd);
          vstrDestroy( cmd );
        }
      }
    } 
    else 
    {
      char *err_info = Tcl_GetVar(interpreter_instance->interp, "errorInfo", TCL_GLOBAL_ONLY);
      if(err_info)
      {
        vstr *cmd = vstrClone( (vchar *)"dis_Message {Error: ");
        cmd = vstrAppend( cmd, (vchar *)err_info);
        cmd = vstrAppend( cmd, (vchar *)"}");
        if( cmd )
        {
          rcall_dis_DISui_eval_async (appl, cmd);
          vstrDestroy( cmd );
        }
      }
    }
  }
  restore_sigchild();
  SetActiveInterpreter(prev_interpreter);
}

void newui_handle_menuitem(int argc, char **argv)
{
  int sel_opt = -1;
  int res_opt = -1;

  const char *command = NULL;
  symbolArr selection;
  int narg =
     parse_menu_options(argc, argv, sel_opt, res_opt, &command, NULL, NULL);
  
  Application *appl = Application::findApplication("DISui");
  if( appl && appl->mySession())
  {
    int last_sel = -1, elements = -1, results = -1;
    vstr *scope = NULL;
    if( rcall_get_rtl_info( appl, last_sel, elements, results, scope ) )//get_rtl_info returns non-zero for success
    {
      int fValidSel = 1;
      if( sel_opt != 4) // -none
      {
        RTListServer *sel_rtl = RTListServer::find ( last_sel );
        switch( sel_opt )
        {
        case 2: //filter
          //get all items from GLOBAL_LastRTL
          if( sel_rtl )
            selection.insert_last( sel_rtl->getArr() );
          break;
        case 1: //scope
          parse_selection( (char *)scope, selection );
          break;
        default: //we default to -selection, get selected from GLOBAL_LastRTL
          if( sel_rtl )
            sel_rtl->getSelArr( selection );
        }
        if(selection.size() == 0 && sel_opt != 5) 
        {
          vstr *cmd = vstrClone( (vchar *)"dis_prompt OKDialog DISCOVER [concat Nothing is selected!]" );
          if( cmd )
          {
            rcall_dis_DISui_eval_async (appl, cmd);
            vstrDestroy( cmd );
          }
          fValidSel = 0;
        }
      }
      if( fValidSel ) //execute query here
      {
        int display = -1;
        if( res_opt == 1 )//elements
          display = elements;
        else
          display = results;
        RTListServer *res_rtl = RTListServer::find ( display );
        newui_execute_query( res_rtl, selection, command, (sel_opt!=4) );
      }
      if( scope )
        vstrDestroy( scope );
    }
  }
}

const char *ACCESS_MENU = "Access";
const char *ACCESS_ENTRY = "Access.";
int newui_add_query_menu(ClientData cd, Tcl_Interp *interp, 
                     int argc, char *argv[])
{
  int nRet = TCL_ERROR;
  
  if( argc > 1 )
  {
    vstr* cmd = vstrClone( (const vchar *)"dis_AccessAddMenu");
    for( int i = 1; i < argc; i++ )
    {
      cmd = vstrAppend( cmd, (const vchar *)" ");
      cmd = vstrAppend( cmd, (const vchar *)"{");
      //Restrict adding menus to Access menu only
      if( i == 1 && strstr(argv[1], ACCESS_MENU) != argv[1] )
        cmd = vstrAppend( cmd, (const vchar *) ACCESS_ENTRY );
      cmd = vstrAppend( cmd, (const vchar *) argv[i] );
      cmd = vstrAppend( cmd, (const vchar *)"}");
    }
    Application *appl = Application::findApplication("DISui");
    if( cmd && appl && appl->mySession() )
    {
      nRet = TCL_OK;
      rcall_dis_DISui_eval_async (appl, cmd);
    }
    if( cmd )
      vstrDestroy( cmd );
  }
  else
    TCL_EXECERR ("wrong number of arguments")
  return nRet;
}

int newui_add_query_item(ClientData cd, Tcl_Interp *interp, 
                     int argc, char *argv[])
{
  int nRet = TCL_ERROR;
  if( argc > 3 )
  {
    vstr* cmd = vstrClone( (const vchar *)"dis_AccessAddItem" );
    for( int i = 1; i < argc; i++ )
    {
      cmd = vstrAppend( cmd, (const vchar *)" ");
      cmd = vstrAppend( cmd, (const vchar *)"{");
      //Restrict adding menus to Access menu only
      if( i == 1 && strstr(argv[1], ACCESS_MENU) != argv[1] )
        cmd = vstrAppend( cmd, (const vchar *) ACCESS_ENTRY );
      cmd = vstrAppend( cmd, (const vchar *) argv[i] );
      cmd = vstrAppend( cmd, (const vchar *)"}");
    }
    Application *appl = Application::findApplication("DISui");
    if( cmd && appl && appl->mySession() )
    {
      nRet = TCL_OK;
      rcall_dis_DISui_eval_async (appl, cmd);
    }
    if( cmd )
      vstrDestroy( cmd );
  }
  else
    TCL_EXECERR ("wrong number of arguments")
  return nRet;
}

int ide_view_file(ClientData cd, Tcl_Interp *interp, int argc, char *argv[])
{
  int ret = TCL_ERROR;
  
  if (argc > 1) {
    for (int ii = 1; ii < argc; ii++) {
      dis_integration_open_file(argv[ii]);
    }
    ret = TCL_OK;
  }
  else {
    vstr *usage = vstrClone((const vchar *)"Usage: ");
    usage = vstrAppend(usage, (const vchar *)argv[0]);
	  usage = vstrAppend(usage, (const vchar *)" file1 [ file2 file3 ... ]");
    Tcl_AppendResult(interp, (char *)usage, NULL);
  }

  return ret;
}

int ide_save_all(ClientData cd, Tcl_Interp *interp, int argc, char *argv[])
{
	int ret = TCL_ERROR;

  vstr *cmd = vstrClone((const vchar *)"dis_ide_send_command \"saveall 0\"");
	rcall_dis_DISui_eval_async (Application::findApplication("DISui"), (vchar *)cmd);

	ret = TCL_OK;  

	return ret;
}

extern int rcall_send_to_global_client_eval (Application* app, vstr *);
extern void rcall_send_to_global_client_eval_async (Application* app, vstr *);

int send_to_global_client_eval(char *str)
{
  Application *appl = Application::findApplication("DISui");
  if( appl && appl->mySession()) 
    return rcall_send_to_global_client_eval(appl, (vstr *)str);
  else
    return TCL_ERROR;
}

int send_to_global_client_eval_async(char *str)
{
  Application *appl = Application::findApplication("DISui");
  if( appl && appl->mySession()) {
    rcall_send_to_global_client_eval_async(appl, (vstr *)str);
    return 1;
  } else
    return 0;
}

extern int rcall_send_notification_string_to_client(Application *, vstr *);

static int send_to_clientCmd(ClientData cd, Tcl_Interp *interp, int argc, char **argv)
{
    if(argc != 2){
	Tcl_AppendResult(interp, argv[0], " should have one argument", NULL);
	return TCL_ERROR;
    }
    Application *appl = Application::findApplication("DISui");
    if( appl && appl->mySession()) 
	rcall_send_notification_string_to_client(appl, (vstr *)argv[1]);
    return TCL_OK;
}

int SetResults(const symbolArr& syms);

int Interpreter::SetResultCmd(ClientData cd, Tcl_Interp *interp, int argc, char *argv[])
{
  Interpreter *i  = (Interpreter *)cd;
  symbolArr arr;
  if(i->ParseArguments(1, argc, argv, arr) != 0)
    return TCL_ERROR;

  ostream& os = i->GetOutputStream();
  os << "This command will be removed in future versions of DISCOVER\n";
  os << "Please use <browser set results> instead\n";

  return SetResults(arr);
}

BrowserController* GetBrowserController();

static void AddCategory(const char* name, const char* command)
{
    GetBrowserController()->add_category(name, command);

    Application *appl = Application::findApplication("DISui");
    if(appl && appl->mySession() )
    {
      rcall_dis_DISui_eval_async (appl, (vstr*)"dis_browser_update_categoriesList");
    }
}

static void AddQuery(const char* name, const char* command)
{
    GetBrowserController()->add_query(name, command);

    Application *appl = Application::findApplication("DISui");
    if(appl && appl->mySession() )
    {
      rcall_dis_DISui_eval_async (appl, (vstr*)"dis_browser_update_askList_for_categories");
    }
}

static void RemoveCategory(const char* name)
{
    GetBrowserController()->remove_category(name);

    Application *appl = Application::findApplication("DISui");
    if(appl && appl->mySession() )
    {
      rcall_dis_DISui_eval_async (appl, (vstr*)"dis_browser_update_categoriesList");
    }
}

static void RemoveQuery(const char* name)
{
    GetBrowserController()->remove_query(name);

    Application *appl = Application::findApplication("DISui");
    if(appl && appl->mySession() )
    {
      rcall_dis_DISui_eval_async (appl, (vstr*)"dis_browser_update_askList_for_categories");
    }
}

int SetResults(const symbolArr& syms)
{        
  Application *appl = Application::findApplication("DISui");
  if(!( appl && appl->mySession()))
    return TCL_ERROR;

  int last_sel = -1, elements = -1, results = -1;
  vstr *scope = NULL;

  //get_rtl_info returns non-zero for success
  if( rcall_get_rtl_info( appl, last_sel, elements, results, scope ) ==0)
    return TCL_ERROR;

  RTListServer *result_rtl = RTListServer::find ( results );
  if( result_rtl ) {
    result_rtl->clear();
    result_rtl->insert( syms );
  }
  //Ask UI to activate Model Browser page
  vstr *cmd = vstrClone( (vchar *)"dis_activate_notebook_page ModelBrowser");
  if( cmd ) {
    rcall_dis_DISui_eval_async (appl, cmd);
    vstrDestroy( cmd );
  }

  return TCL_OK; 
}

int SetElements(const symbolArr& syms)
{        
  Application *appl = Application::findApplication("DISui");
  if(!( appl && appl->mySession()))
    return TCL_ERROR;

  int last_sel = -1, elements = -1, results = -1;
  vstr *scope = NULL;

  //get_rtl_info returns non-zero for success
  if( rcall_get_rtl_info( appl, last_sel, elements, results, scope ) == 0)
    return TCL_ERROR;

  RTListServer *result_rtl = RTListServer::find ( elements );
  if( result_rtl ) {
    result_rtl->clear();
    result_rtl->insert( syms );
  }
  //Ask UI to activate Model Browser page
  vstr *cmd = vstrClone( (vchar *)"dis_activate_notebook_page ModelBrowser");
  if( cmd ) {
    rcall_dis_DISui_eval_async (appl, cmd);
    vstrDestroy( cmd );
  }

  return TCL_OK;
}

int SetNextRtl(const symbolArr& syms)
{
  Application *appl = Application::findApplication("DISui");
  if(!( appl && appl->mySession()))
    return TCL_ERROR;

  int last_sel = -1, elements = -1, results = -1;
  vstr *scope = NULL;

  //get_rtl_info returns non-zero for success
  if( rcall_get_rtl_info( appl, last_sel, elements, results, scope ) == 0)
    return TCL_ERROR;

  RTListServer* next;
  if (last_sel != -1 && last_sel == results)
  {
    next = RTListServer::find(elements);
  }
  else
  {
    next = RTListServer::find(results);
  }

  if( next ) 
  {
    next->clear();
    next->insert( syms );
  }
  //Ask UI to activate Model Browser page
  vstr *cmd = vstrClone( (vchar *)"dis_activate_notebook_page ModelBrowser");
  if( cmd ) {
    rcall_dis_DISui_eval_async (appl, cmd);
    vstrDestroy( cmd );
  }
  return TCL_OK;
}

enum RTLname {RTL_ELEMENTS, 
	      RTL_RESULTS};

int SortRTL(char* sort_string, RTLname nm)
{        
  if (!sort_string)
    return TCL_ERROR;

  Application *appl = Application::findApplication("DISui");
  if(!( appl && appl->mySession()))
    return TCL_ERROR;

  int last_sel = -1, elements = -1, results = -1;
  vstr *scope = NULL;

  //get_rtl_info returns non-zero for success
  if( rcall_get_rtl_info( appl, last_sel, elements, results, scope ) == 0)
    return TCL_ERROR;

  RTListServer *result_rtl = NULL;
  if (nm == RTL_ELEMENTS) {
    result_rtl = RTListServer::find ( elements );
  } else {
    result_rtl = RTListServer::find ( results );
  }
  if( result_rtl ) {
    genString sort;
    genString format;
    genString show;
    genString hide;
    result_rtl->getFilter(sort,format,show,hide);
    result_rtl->setFilter(sort_string,format,show,hide);
	result_rtl->_propagate();
  }
  //Ask UI to activate Model Browser page
  vstr *cmd = vstrClone( (vchar *)"dis_activate_notebook_page ModelBrowser");
  if( cmd ) {
    rcall_dis_DISui_eval_async (appl, cmd);
    vstrDestroy( cmd );
  }

  return TCL_OK;
}

int FormatRTL(char* fmt_string, RTLname nm)
{        
  if (!fmt_string)
    return TCL_ERROR;

  Application *appl = Application::findApplication("DISui");
  if(!( appl && appl->mySession()))
    return TCL_ERROR;

  int last_sel = -1, elements = -1, results = -1;
  vstr *scope = NULL;

  //get_rtl_info returns non-zero for success
  if( rcall_get_rtl_info( appl, last_sel, elements, results, scope ) == 0)
    return TCL_ERROR;

  RTListServer *result_rtl = NULL;
  if (nm == RTL_ELEMENTS) {
    result_rtl = RTListServer::find ( elements );
  } else {
    result_rtl = RTListServer::find ( results );
  }
  if( result_rtl ) {
    genString sort;
    genString format;
    genString show;
    genString hide;
    result_rtl->getFilter(sort,format,show,hide);
    result_rtl->setFilter(sort,fmt_string,show,hide);
	result_rtl->_propagate();
  }
  //Ask UI to activate Model Browser page
  vstr *cmd = vstrClone( (vchar *)"dis_activate_notebook_page ModelBrowser");
  if( cmd ) {
    rcall_dis_DISui_eval_async (appl, cmd);
    vstrDestroy( cmd );
  }

  return TCL_OK;
}

static void BrowserAdd(int argc, char** argv, Interpreter* i)
{
    ostream& os = i->GetOutputStream();
    if (argc < 3)
    {
        os << "The following subcommands are available:\n";
        os << "  browser add category <name>\n";
        os << "  browser add query <name>\n";
        return;
    }
    if (argc < 4)
    {
        os << "A name must be specified\n";
        return;
    }
    const char* name = argv[3];
    const char* cmd;
    if (argc <= 4)
        cmd = name;
    else
        cmd = argv[4];

    if (strcmp(argv[2], "category") == 0)
    {
        AddCategory(name, cmd);
    }
    else if (strcmp(argv[2], "query") == 0)
    {
        AddQuery(name, cmd);
    }
    else
    {
        os << "browser add: " << argv[2] << " -- unknown subcommand\n";
    }
}

static void BrowserRemove(int argc, char** argv, Interpreter* i)
{
    ostream& os = i->GetOutputStream();
    if (argc < 3)
    {
        os << "The following subcommands are available:\n";
        os << "  browser remove category <name>\n";
        os << "  browser remove query <name>\n";
        return;
    }
    if (argc < 4)
    {
        os << "A name must be specified\n";
        return;
    }
    const char* name = argv[3];
    if (strcmp(argv[2], "category") == 0)
    {
        RemoveCategory(name);
    }
    else if (strcmp(argv[2], "query") == 0)
    {
        RemoveQuery(name);
    }
    else
    {
        os << "browser remove: " << argv[2] << " -- unknown subcommand\n";
    }
}

static void BrowserSet(int argc, char** argv, Interpreter* i)
{
    ostream& os = i->GetOutputStream();
    if (argc < 3)
    {
        os << "The following subcommands are available:\n";
        os << "  browser set elements <symbol set>\n";
        os << "  browser set results <symbol set>\n";
        os << "  browser set next <symbol set>\n";
        return;
    }
    if (argc < 4)
    {
        os << "A symbol set must be specified\n";
        return;
    }
    symbolArr res;
    if (i->ParseArguments(3, argc, argv, res) != 0)
    {
        os << "An error occured while processing symbol set\n";
    }
    else if (strcmp(argv[2], "elements") == 0)
    {
        SetElements(res);
    }
    else if (strcmp(argv[2], "results") == 0)
    {
        SetResults(res);
    }
    else if (strcmp(argv[2], "next") == 0)
    {
        SetNextRtl(res);
    }
    else
    {
        os << "browser set: " << argv[2] << " -- unknown subcommand\n";
    }
}

static void BrowserSort(int argc, char** argv, Interpreter* i)
{
    ostream& os = i->GetOutputStream();
    if (argc < 3)
    {
        os << "The following subcommands are available:\n";
        os << "  browser sort elements [ [-a|-d] <attribute> ...]\n";
        os << "  browser sort results  [ [-a|-d] <attribute> ...]\n";
        return;
    }
    if (argc < 4)
    {
        os << "A sort list must be specified\n";
        return;
    }

	RTLname rtl_type;
    if (strcmp(argv[2], "elements") == 0)
    {
        rtl_type = RTL_ELEMENTS;
    }
    else if (strcmp(argv[2], "results") == 0)
    {
        rtl_type = RTL_RESULTS;
    }
    else
    {
        os << "browser sort: " << argv[2] << " -- unknown subcommand\n";
    }

	static ocharstream sort_list;
    sort_list.reset();
    sort_list << '#';
    char nextOrder = 'f';
    for(int ii = 3; ii < argc; ii++){
		if (strcmp(argv[ii],"-d") == 0) {
			nextOrder = 'r';
		}			
		else 
			if (strcmp(argv[ii],"-a") == 0) {
			nextOrder = 'f';
	    } else {
			symbolAttribute *att = symbolAttribute::get_by_name(argv[ii]);
			if(att == NULL){
				os << "attribute '" << argv[ii] << "' does not exist\n";
				return;
			}
			sort_list << '%' << nextOrder << argv[ii];
	    };
    };
    sort_list << ends;


	SortRTL(sort_list.ptr(), rtl_type);
}

static void BrowserFormat(int argc, char** argv, Interpreter* i)
{
    ostream& os = i->GetOutputStream();
    if (argc < 3)
    {
        os << "The following subcommands are available:\n";
        os << "  browser format elements <format string>\n";
        os << "  browser format results <format string>\n";
        return;
    }
    if (argc < 4)
    {
        os << "A format string must be specified\n";
        return;
    }

	RTLname rtl_type;
    if (strcmp(argv[2], "elements") == 0)
    {
        rtl_type = RTL_ELEMENTS;
    }
    else if (strcmp(argv[2], "results") == 0)
    {
        rtl_type = RTL_RESULTS;
    }
    else
    {
        os << "browser format: " << argv[2] << " -- unknown subcommand\n";
    }

	char* format = argv[3];

    static ocharstream format_list;
    format_list.reset();
    ////
    int n = 0;
    char ch;
    
    int no_attr = argc - 4;
    for(char * frmt = (char*)format; (ch=*frmt); ++frmt) {
		if (ch != '%'){
			format_list << ch;
			continue;
		}
		++frmt;
		ch = *frmt;
		if ((ch != 's') && (ch != 'S')) {
			format_list << ch;
			continue;  // loop
		}
      
		if(n >= no_attr)
			continue;
      
		symbolAttribute *att = symbolAttribute::get_by_name(argv[n+4]);
		if(att == NULL){
			os << "attribute '" << argv[n+4] << "' does not exist\n";
			return;
		}

		format_list << '%' << argv[n+4] << ',';
		n++;
    }
    format_list << ends;

	FormatRTL(format_list.ptr(), rtl_type);
}

static int BrowserCmd(ClientData cd, Tcl_Interp* interp, int argc, char** argv)
{
    Interpreter* i = (Interpreter*)cd;
    ostream& os = i->GetOutputStream();
    if (argc < 2)
    {
        os << "The following subcommands are available:\n";
        os << "  browser add category <name> <command>\n";
        os << "  browser add query <name> <command>\n";
        os << "  browser remove category <name>\n";
        os << "  browser remove query <name>\n";
        os << "  browser set elements <symbol set>\n";
        os << "  browser set results <symbol set>\n";
        os << "  browser set next <symbol set>\n";
    }
    else if (strcmp(argv[1], "add") == 0)
    {
        BrowserAdd(argc, argv, i);
    }
    else if (strcmp(argv[1], "remove") == 0)
    {
        BrowserRemove(argc, argv, i);
    }
    else if (strcmp(argv[1], "set") == 0)
    {
        BrowserSet(argc, argv, i);
    }
    else if (strcmp(argv[1], "sort") == 0)
    {
        BrowserSort(argc, argv, i);
    }
    else if (strcmp(argv[1], "format") == 0)
    {
        BrowserFormat(argc, argv, i);
    }
    else
    {
        os << "browser: " << argv[1] << " -- unknown subcommand\n";
    }
    return TCL_OK;
}

void dis_open_view(symbolPtr, const char* mode);
extern void open_view (viewtype, symbolArr&);
static int OpenViewCmd(ClientData cd, Tcl_Interp* interp, int argc, char** argv)
{
	Interpreter *i  = (Interpreter *)cd;
	symbolArr arr;
	genString mode;
	int ret;
	if (argc >= 3)
	{
		mode = argv[1];
		ret = i->ParseArguments(2, argc, argv, arr);
	}
	else if (argc == 2)
	{
		mode = "-definition";
		ret = i->ParseArguments(1, argc, argv, arr);
	}
	else
		return TCL_ERROR;

	if(ret != 0)
		return TCL_ERROR;
 	
	int size = arr.size();
	if(mode == "-definition") {
	   for (int j = 0; j < size; ++j)
		    dis_open_view(arr[j], mode);
	} else {
		  viewtype type;
          if (mode=="-outline") type = viewOutline;
            else if (mode=="-flowchart") type = viewFlowchart;
              else if (mode=="-dependency") type = viewCallTree;
                 else if (mode=="-inheritance") type = viewInheritance;
                   else if (mode=="-relations") type = viewERD;
                      else if (mode=="-datachart") type = viewDatachart;
                         else if (mode=="-subsystems") type = viewSubsysBrowser;
						     else {
								 genString message;
								 message="Unknown vier type: can be -definition,-outline,-flowchart,-dependency,-inheritance,-relations,-datachart,-subsystems";
                                 Tcl_SetResult(i->interp, message, TCL_VOLATILE);
								 return TCL_ERROR;
							 }
			open_view(type, arr);
	}
	return TCL_OK;
}

#ifdef _WIN32
int cli_get_number_of_clients(void);
int cli_start_client(int not_stdio, char* exec, char *sf, char *shutdown_script,
		     char*cmd, ostream& outstream, Interpreter *parent_interpreter);
void cli_wait_for_client(int client);

static int start_clientCmd(ClientData cd, Tcl_Interp *interp, int argc, char **argv)
{
     Interpreter * i     = (Interpreter*) cd;
     ostream& outstream  = i->GetOutputStream();

     int   not_stdio        = 1;
     char  *sfile           = 0;
     char  *cmd             = 0;
     char  *execbuf         = 0;
     char  *shutdown_script = NULL;
     int   err              = TCL_OK;   

     for(int ii=1; ii<argc; ++ii){
	  char * arg = argv[ii];

	  if(strcmp(arg, "-source") == 0){
	       ++ii;
	       sfile = argv[ii];
	  } else if(strcmp(arg, "-shutdown") == 0){
	       ++ii;
	       shutdown_script = argv[ii];
	  } else if(strcmp(arg, "-command") == 0){
	       ++ii;
	       cmd = argv[ii];
	  } else if(strcmp(arg, "-stdio") == 0){
	       not_stdio = 0;
	  } else if (arg[0] == '-') {
	       outstream << "options: -source -command -stdio" << endl;
	       err = TCL_ERROR;
	  } else if (arg[1] == '\0') {
	       outstream << "obsolete option: " << arg << 
		    ". Use -stdio to use 1&2" << endl; 
	  } else if (execbuf) {
	       outstream << "start_client " << execbuf << " " << arg << 
		    ": 2 commands" << endl;
	       err = TCL_ERROR;
	  } else {
	       execbuf = arg;
	  }
     }
     if(!execbuf){
	  outstream << "start_client: no exec specified "  << endl;
	  err = TCL_ERROR;
     }
     if (err == TCL_OK) 
	  err = cli_start_client(not_stdio, execbuf, sfile, shutdown_script, cmd, outstream, i);
     
     if(err == TCL_OK){
	  int client = cli_get_number_of_clients();
	  char buf[12];
	  OSapi_sprintf(buf, "%d", client);
	  Tcl_AppendResult(i->interp,  buf, NULL);
     }
     return err;
}

static int wait_clientCmd(ClientData cd, Tcl_Interp *interp, int argc, char **argv)
{
  if(argc != 2) {
    Tcl_AppendResult(interp,  "wrong number of arguments", NULL);
    return TCL_ERROR;
  }
  char* arg  = argv[1];
  int client = OSapi_atoi(arg);
  arg        = Tcl_GetVar(interp, "dis_client", TCL_GLOBAL_ONLY);
  if(arg) { // client
    int cur_client = OSapi_atoi(arg);
    if(cur_client == client){
      Tcl_AppendResult(interp,  "client ", arg, " cannot wait for itself", NULL);
      return TCL_ERROR;
    }
  } 
  client--; // index in clients
  cli_wait_for_client(client);
  return TCL_OK;
}

#endif
#ifdef _WIN32
static int statusCmd(ClientData cd, Tcl_Interp *, int argc, char *argv[]);
#endif
static int add_commands()
{
#ifdef _WIN32
	 new cliCommandInfo("status", statusCmd);
#endif
     new cliCommandInfo ("set_result", Interpreter::SetResultCmd);
     new cliCommandInfo ("get_gala_selection", GetGalaSelectionCmd);
     new cliCommandInfo ("add_menu", newui_add_query_menu);
     new cliCommandInfo ("add_item", newui_add_query_item);
     new cliCommandInfo ("view_file", ide_view_file);
     new cliCommandInfo ("save_all", ide_save_all);
     new cliCommandInfo ("send_to_client", send_to_clientCmd);
     new cliCommandInfo ("browser", BrowserCmd);
     new cliCommandInfo ("open_view", OpenViewCmd);
#ifdef _WIN32
     new cliCommandInfo("start_client", start_clientCmd);
     new cliCommandInfo("wait_client", wait_clientCmd);
#endif
     return 1;
}

static int add_commands_dummy = add_commands();

void newui_execute_query(symbolArr& results, const symbolArr& selection, const char* cmd_to_eval)
{
     Initialize(newui_execute_query);

     static RTListServer dummy(1024);
     newui_execute_query(&dummy, (symbolArr&)selection, cmd_to_eval, 1);

     RTLNode* n = checked_cast (RTLNode, dummy.get_root());
   
     results = n->rtl_contents ();
}




#ifdef _WIN32

extern int tcl_mrg_commands;
static int link_tcl_commands = 0 + tcl_mrg_commands;

static int statusCmd(ClientData cd, Tcl_Interp *interp, int argc, char *argv[]) {
int retval = TCL_OK;
static int reg=0;
static int msgAmountID;
static int msgProcessedID ;
static PROCESS_INFORMATION pi;
static char *shared_memory;
Interpreter *i  = (Interpreter *)cd;

  // Registrating user-defined messages
  msgAmountID    = RegisterWindowMessage("WM_SETAMOUNT");
  msgProcessedID = RegisterWindowMessage("WM_SETPROCESSED");

  // Start command 
  if(strcmp(argv[1],"start")==0) {
	  STARTUPINFO si;
	  SECURITY_ATTRIBUTES saProcess, saThread;

	  ZeroMemory(&si,sizeof(si));
	  si.cb=sizeof(si);
	  saProcess.nLength = sizeof(saProcess);
	  saProcess.lpSecurityDescriptor=NULL;
	  saProcess.bInheritHandle=TRUE;

	  saThread.nLength=sizeof(saThread);
	  saThread.lpSecurityDescriptor=NULL;
	  saThread.bInheritHandle = TRUE;

	  genString title   = "Progress Indicator";
	  genString message = "To Process";
	  genString amount  = "100";

	  if(strcmp(argv[2],"-title")==0) {
		  title =  argv[3];
		  message= argv[4];
		  amount = argv[5];
	  } else {
	     if(strcmp(argv[2],"-ok") ==0) {
		     if(strcmp(argv[3],"-title")==0) {
		         title =  argv[4];
		         message= argv[5];
		         amount = argv[6];
			 } else {
		         title =  argv[3];
		         message= argv[4];
		         amount = argv[5];
			 }
		 } else {
		     message= argv[2];
		     amount = argv[3];
		 }
	  }
      genString query;
      query.printf("CancelDialog %s -%s -%s",title, message, amount);
	  ::CreateProcess(NULL,query,&saProcess,&saThread,TRUE,0,NULL,NULL,&si,&pi);
	  shared_memory=(char *) ::VirtualAllocEx(pi.hProcess,NULL,1024,MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE);
	  
  }

  // End command 
  if(strcmp(argv[1],"end")==0) {
	  ::VirtualFreeEx(pi.hProcess,shared_memory,0,MEM_RELEASE);
	  ::TerminateProcess(pi.hProcess,0);
  }

  // Query command
  if(strcmp(argv[1],"query")==0) {
      char buf[6];
	  DWORD status;
	  ::GetExitCodeProcess(pi.hProcess,&status);
  	  if(status==STILL_ACTIVE) {
		  DWORD written;
		  char mem[1024];
		  ZeroMemory(mem,1000);
          ::WriteProcessMemory(pi.hProcess,shared_memory,mem,1000,&written);
          ::WriteProcessMemory(pi.hProcess,shared_memory,argv[2],strlen(argv[2]),&written);
		  ::PostMessage(HWND_BROADCAST,msgProcessedID,atoi(argv[3]),(LPARAM)shared_memory);
          sprintf(buf, "%d",0);
	  } else sprintf(buf, "%d",1);
      Tcl_SetResult(i->interp, buf, TCL_VOLATILE);
  }
  return TCL_OK;
}
#endif
