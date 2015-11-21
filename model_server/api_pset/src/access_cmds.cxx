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
#include <fstream>
using namespace std;
#ifdef _WIN32
#include <windows.h>
#include <mapi.h>
#endif 

#include "SocketClient.h"
#include "TclList.h"
#include "api_internal.h"
#include "cLibraryFunctions.h"
#include "cliClient.h"
#include "db.h"
#include "ddict.h"
#include "genError.h"
#include "genString.h"
#include "general.h"
#include "machdep.h"
#include "msg.h"
#include "objArr.h"
#include "symbolSet.h"
#include "xref.h"

int globalConnectedToNewUI=0;

Interpreter* GetActiveInterpreter();
int          cli_process_input(char const *data, ostream&, class Interpreter *, int do_propmt_and_print);
int          cli_eval_string(char const* cmd);
 
void handle_pipe_buffer(char const *buffer, int len, cliClientData *cd)
{
     static ofstream null_stream("/dev/null", ios::out);
     

     int send_value = 0;
     int send_size = 0;
     char const * ptr      = buffer;
     int command_ch = ptr[0];
     if(command_ch == '\3'){
          cd->osp    = &null_stream;
          send_value = 0;
     } else if(command_ch == '\1'){
          cd->osp    = cd->os;
          send_value = 0;
     } else if(command_ch == '\2'){
          cd->osp    = &null_stream;
          send_value = 1;
     } else if (command_ch == '\4') {
          cd->osp    = &null_stream;
          send_value = 1;
          send_size = 1;
     }
     ptr++;
     Interpreter *prev_interpreter = GetActiveInterpreter();
     SetActiveInterpreter(cd->interp);
     cli_process_input(ptr, *(cd->osp), cd->interp, 1);
     if(send_value && (cd->interp->command.length()==0)) {
          genString res;
          cd->interp->GetResult(res);
          if (send_size == 1) {
            *(cd->os) << (res.length() + 1) << ends;  // dont flush yet
          }
          *(cd->os) << (char *) res << ends << flush;
     }   
     SetActiveInterpreter(prev_interpreter);
}

projModule* symbol_get_module(symbolPtr&sym);

extern int els_reparse_modules(symbolArr &, symbolArr &);

static int ParseModuleCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[])
{
  int ret = TCL_ERROR;

  Interpreter *shell = (Interpreter *)cd;

  symbolArr res;

  symbolArr sel;
  symbolArr *mods = &sel, rest;

  if (shell->ParseArguments(1, argc, argv, sel) == 0) {
      symbolPtr sym;
      ForEachS (sym, sel) {
        if (sym.get_kind() == DD_MODULE) {
          symbolArr modules;
          symbolArr arr;
          modules.insert_last(sym);
          if (!els_reparse_modules(modules, arr)){
            res.insert_last(modules);
          } else {
            rest.insert_last(modules);
          }
        }
        mods = &rest;
        ret = TCL_OK;
      }
  } 

  if (res.size ()) {
    DI_object obj;
    DI_object_create(&obj);
    symbolArr *syms = get_DI_symarr(obj);
    /* assert(syms); */
    for (int i = 0; i < res.size(); i++) {
      syms->insert_last(res[i]);
    }
    shell->SetDIResult(obj);
    ret = TCL_OK;
  }

  return ret;
}

static int update_moduleCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[])
{
    return ParseModuleCmd (cd,interp,argc,argv);
}

extern int get_modified_objects(symbolArr &, symbolArr & );
static int modified_objectsCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[])
{
    Interpreter *i = (Interpreter *)cd;
    
    symbolArr arr;
    if(i->ParseArguments(1, argc, argv, arr) != 0)
        return TCL_ERROR;
    
    symbolArr xsyms;
    int sz = arr.size();
    for(int ii = 0; ii < sz; ++ii){
        symbolPtr sym  = arr[ii];
        symbolPtr xsym = sym.get_xrefSymbol();
        if(xsym.xrisnotnull())
            xsyms.insert_last(xsym);
    }
    DI_object modified_objects;
    DI_object_create(&modified_objects);
    symbolArr *objects_arr = get_DI_symarr(modified_objects);
    get_modified_objects(xsyms, *objects_arr);
    i->SetDIResult(modified_objects);
    return TCL_OK;
}


extern char const * get_statement_of_inst(symbolPtr& sym);

int  GetInstanceStatementCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[])
{
    CHECK_ARG_NUM("get_instance_statement",0);

    Interpreter *i = (Interpreter *)cd;
    symbolArr res;
    if(i->ParseArguments(1, argc, argv, res) != 0)
        return TCL_ERROR;
    symbolPtr sym;
    ForEachS(sym, res) {
      if (sym.is_instance()) {
        //MetricElement mel = sym.get_MetricElement();
        char const * statement = get_statement_of_inst(sym);
        if (statement && *statement) 
          Tcl_AppendResult(i->interp, statement, NULL);
      }
    }
    //i->SetDIResult(results);
    return TCL_OK;
}

extern char const * inst_get_region( symbolPtr& sym );

int inst_parseInputString( char const * src_line , char * fname , int* startOffset , int* endOffset )
{
  Initialize( inst_parceInputString );

  char const * srcPtr  = src_line;
  char * destPtr = fname;
  while( *srcPtr && ( ' ' ==  *srcPtr || '\n' == *srcPtr ) ) srcPtr++;
  if( ! *srcPtr )
    return 0;

  while( *srcPtr &&  ' ' != *srcPtr )  *destPtr++ = *srcPtr++;
  *destPtr = NULL;

  while( *srcPtr &&  ' ' == *srcPtr ) srcPtr++;
  if( ! *srcPtr )
    return 0;

  char buf[33];
  destPtr = buf;
 
  while( *srcPtr && ' ' != *srcPtr )  *destPtr++ = *srcPtr++;
  *destPtr = NULL;
  *startOffset =  OSapi_atoi( buf );

  while( *srcPtr &&  ' ' == *srcPtr ) srcPtr++;
  if( ! *srcPtr )
    return 0;

  destPtr = buf;
  while( *srcPtr && ' ' != *srcPtr && '\n' != *srcPtr )  *destPtr++ = *srcPtr++;
  *destPtr = NULL;
  *endOffset =  OSapi_atoi( buf );

  return 1;
}

static int instances_in_regionCmd( ClientData cd , Tcl_Interp *interp , int argc , char const *argv[] )
{
  CHECK_ARG_NUM( "instances_in_regionCmd" , 0 );

  Interpreter *i = (Interpreter *)cd;

  if ( 2 != argc ) {
      Tcl_AppendResult(i->interp, "Usage: instances_in_region <filename> <start offset> <end offset> ...", NULL);
      return TCL_ERROR;
  }

  DI_object results;
  DI_object_create(&results);
  symbolArr *results_arr = get_DI_symarr(results);

  char fname[1024];
  int startOffset = 0;
  int endOffset =0;

  if(!inst_parseInputString( argv[ 1 ] , fname , &startOffset , &endOffset ) ){
      Tcl_AppendResult(i->interp, "Bad filename or offset", NULL);
      return TCL_ERROR;
  }

  symbolArr arr;
  arr.removeAll();
  MetricElement::get_instances( fname , startOffset , endOffset , arr );  
        
  symbolPtr sym;
  ForEachS(sym, arr) {
      results_arr->insert_last(sym);
  }

  i->SetDIResult(results);
  return TCL_OK;
}

static int region_of_statementCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[])
{
    CHECK_ARG_NUM("region_of_statementCmd",0);

    Interpreter *i = (Interpreter *)cd;
    symbolArr res;
    if(i->ParseArguments(1, argc, argv, res) != 0)
        return TCL_ERROR;
    symbolPtr sym;
    ForEachS(sym, res) {
      if (sym.is_instance()) {
        char const * statement = inst_get_region( sym );
        
        if (statement && *statement) 
          Tcl_AppendResult(i->interp, statement, NULL);
  
      }
    }
    return TCL_OK;
}

static smtTree* symbol_get_def_smt(symbolPtr&sym)
{
 Initialize(symbol_get_def_smt);
  smtTree*smt = NULL;
  ddKind kind = sym.get_kind();
  if(kind==DD_PROJECT || kind==DD_SCOPE)
    return NULL;

  if(kind==DD_MODULE) {
    app* h = sym.get_def_app();
    if(h)
      smt = checked_cast(smtTree,h->get_root());
  } else {
    ddElement* dd = sym.get_def_dd();
    if(dd)
      smt = (smtTree*)get_relation(def_smt_of_dd, dd);
  }

  return smt;
}

static int src_linesCmd(ClientData cd, Tcl_Interp *interp, int argc, char const **argv)
{
  Interpreter *i = (Interpreter *)cd;
  symbolArr arr;
  if(i->ParseArguments(1, argc, argv, arr) != 0)
    return TCL_ERROR;
  
  if(arr.size() != 1){
    Tcl_AppendResult(i->interp, "size(input) != 1", NULL);
    return TCL_ERROR;
  }      
  smtTree* smt = symbol_get_def_smt(arr[0]);
  if(!smt){
    return TCL_OK;
  }
  int st_line = smt->src_line_num;
  int en_line = smt->get_last_leaf()->src_line_num;
  
  static genString buf;
  buf.printf("%d %d", st_line, en_line);
  Tcl_AppendResult(i->interp, (char *)buf, NULL);
  
  return TCL_OK;
}

static int src_loadCmd(ClientData cd, Tcl_Interp *interp, int argc, char const **argv)
{
  Interpreter *i = (Interpreter *)cd;
  symbolArr arr;
  if(i->ParseArguments(1, argc, argv, arr) != 0)
    return TCL_ERROR;
  
  int sz = arr.size();
  for (int ii=0; ii<sz; ++ii){
    app* h = arr[ii].get_def_app();
  }
  return TCL_OK;
}

extern void proj_get_modules(projNode *proj, objArr& modules, int depth); 
extern void scope_get_modules(scopeNode *sc, objArr& modules, int depth); 
extern projModule * symbol_get_module( symbolPtr& sym);
extern void obj_unload(Obj *root);

static void collect_modules(symbolArr&arr, objArr& modules)
{
  int sz = arr.size();
  int ii;
  for (ii=0; ii<sz; ++ii){
    symbolPtr sym = arr[ii];
    ddKind kind = sym.get_kind();
    if(kind == DD_PROJECT){
      projNode*proj = (projNode*)(Relational*)sym;
      proj_get_modules(proj, modules, -1);
    } else if(kind == DD_SCOPE) {
      scopeNode*scope = (scopeNode*) (Relational*)sym;
      scope_get_modules(scope, modules, -1);
    } else {
      projModule*mod = symbol_get_module(sym);
      if(mod)
        modules.insert_last(mod);
    }
  }
} 

static int src_unloadCmd(ClientData cd, Tcl_Interp *interp, int argc, char const **argv)
{
 Initialize(src_unloadCmd);
  Interpreter *i = (Interpreter *)cd;
  symbolArr arr;
  if(i->ParseArguments(1, argc, argv, arr) != 0)
    return TCL_ERROR;
  
  objArr modules;
  collect_modules(arr, modules);
  int ii;
  int no_mod = modules.size();
  for(ii=0; ii<no_mod; ++ii){
    projModule*mod = checked_cast(projModule, modules[ii]);
    if(mod->is_loaded()){
      app* h = mod->get_app();
      if(h && is_smtHeader(h)){
        cout << "Unloading " << h->get_name() << endl; 
        obj_unload(h);
      }
    }
  }
  return TCL_OK;
}

int outdated_pset_time( char const * src_file, char const *pset_file);
app *proj_restore_file( projNode *, char const *fn, char const *ln,
                        projModule *mod, int(*)(void *), void *);

static smtHeader *dis_reload_header(app *h)
{
    smtHeader *ret_h = NULL;
    if (h && is_smtHeader(h)) {
        projModule *mod = appHeader_get_projectModule(h);
        if (mod) {
            char const *pset_name = mod->paraset_file_name();
            genString f_name = h->get_phys_name();
            int outdated = outdated_pset_time(f_name, pset_name);
            int merge_res = 0;
            if (outdated) {
                if (h->is_modified()) {
                    msg("discarded DISCOVER changes in '$1'", warning_sev) << (char *)f_name << eom;
                    obj_unload(h);
 
                    projNode *prj = mod->get_project();
                    genString lnm = mod->get_name();
                    h = proj_restore_file(prj, f_name, lnm, mod, NULL, NULL);
                } else {
                    int do_not_save = 1;
                    merge_res = projModule::merge_module(mod, (char *)f_name, pset_name, 0, 0, 0, do_not_save);
                }
            }
 
            if (h && is_smtHeader(h)) {
                ret_h =  (smtHeader *)h;
            }
        }
    }
    return ret_h;
}

static int src_reloadCmd(ClientData cd, Tcl_Interp *interp, int argc, char const **argv)
{
 Initialize(src_unloadCmd);
  Interpreter *i = (Interpreter *)cd;
  symbolArr arr;
  if(i->ParseArguments(1, argc, argv, arr) != 0)
    return TCL_ERROR;
  
  objArr modules;
  collect_modules(arr, modules);
  int ii;
  int no_mod = modules.size();
  for(ii=0; ii<no_mod; ++ii){
    projModule*mod = checked_cast(projModule, modules[ii]);
    if(mod->is_loaded()){
      app* h = mod->get_app();
      if(h && is_smtHeader(h)){
        cout << "Reloading " << h->get_name() << endl; 
        dis_reload_header(h);
      }
    }
  }
  return TCL_OK;
}

int parse_menu_options(int argc, char const **argv, 
   int& sel_opt, int& res_opt, char const**arg1, char const**arg2, char const**arg3)
{  
  int narg = 0;
  for(int ii=0; ii<argc;++ii){
    char const*arg = argv[ii];
    if(arg[0]=='-'){
      int len = strlen(arg);
      if(!strncmp(arg, "-scope", len))
        sel_opt = 1;
      else if(!strncmp(arg, "-filter", len))
        sel_opt = 2;
      else if(!strncmp(arg, "-selection", len))
        sel_opt = 0;
      else if(!strncmp(arg, "-optional", len))
        sel_opt = 5;
      else if(!strncmp(arg, "-none", len))
        sel_opt = 4;
      else if(!strncmp(arg, "-elements", len))
        res_opt = 1;
      else if(!strncmp(arg, "-results", len))
        res_opt = 2;
      else if(!strncmp(arg, "-printformat", len))
	res_opt = 3;
    } else switch(++narg) {
       case 1: if(arg1) *arg1 = arg; break;
       case 2: if(arg2) *arg2 = arg; break;
       case 3: if(arg3) *arg3 = arg; break;
       default: break;
    }
  }
  return narg;
}

static int OspathCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[])
{
  int ret = TCL_ERROR;

  Interpreter *i = (Interpreter *)cd;

  if (argc == 2) {
    genString path = argv[1];
    genString trans = OSPATH(path.str());
    Tcl_SetResult(i->interp, (char *)trans, TCL_VOLATILE);
    ret = TCL_OK;
  }
  else {
    genString usage;
    usage.printf("Usage: %s pathname", argv[0]);
    Tcl_AppendResult(i->interp, usage.str(), NULL);
  }

  return ret;
}

int LoadedUnparsedFilesCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[])
{
  int ret = TCL_ERROR;

  DI_object result;
  DI_object_create(&result);
  symbolArr *arr = get_DI_symarr(result);
  loadedFiles::get_unparsed_files(*arr);

  Interpreter *i = (Interpreter *)cd;
  i->SetDIResult(result);

  ret = TCL_OK;

  return ret;
}

int LoadedFilesCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[])
{
  int ret = TCL_ERROR;

  DI_object result;
  DI_object_create(&result);
  symbolArr *arr = get_DI_symarr(result);
  loadedFiles::get_loaded_files(*arr);

  Interpreter *i = (Interpreter *)cd;
  i->SetDIResult(result);

  ret = TCL_OK;

  return ret;
}

int SaveModuleCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[])
{
  int ret = TCL_ERROR;

  Interpreter *i = (Interpreter *)cd;

  symbolArr modarr;
  bool valid = true;
  symbolArr sel;
  if ((argc > 1) && (i->ParseArguments(1, argc, argv, sel) == 0)) {
    symbolPtr sym;
    ForEachS(sym, sel) {
      if (sym.get_kind() == DD_MODULE)
        modarr.insert_last(sym);
    }
  }
  else
    valid = false;

  if ((valid) && (modarr.size() > 0)) {
    symbolArr good;
    symbolArr bad;
    symbolPtr sym;
    ForEachS(sym, modarr) {
      bool saved = false; 
      projNode *proj = sym.get_projNode();
      if (proj->is_writable()) {
        projNode *root = proj->root_project();
        char const *name = sym.get_name();
        projModule *mod = root->find_module(name);
        if (!mod)
            mod = projHeader::find_module(name);
        if (mod) {
          app *ah = mod->restore_module(0);
          if (ah) {
            if (ah->is_modified()) {
              int success;
              if (success = db_save_or_cancel(ah, 0, 1)) {
                if (success != -1) {
                  ah->clear_modified();
                  saved = true;    // Success
                }
              }
            }
            else
              saved = true;            // Success (if it's not modified, the work is done) 
          }
        }
      }
      if (saved)
        good.insert_last(sym);
      else
        bad.insert_last(sym);
    }

    if (good.size() > 0) {
      DI_object result;
      DI_object_create(&result);
      symbolArr *arr = get_DI_symarr(result);
      *arr = good;
      i->SetDIResult(result);
    }

    ret = TCL_OK;
  }
  else {
    genString usage;
    usage.printf("Usage: %s module1 [ module2 module3 ... ]", argv[0]);
    Tcl_AppendResult(interp, usage.str(), NULL);
  }

  return ret;
}

int UnmodCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[])
{
  int ret = TCL_ERROR;

  Interpreter *i = (Interpreter *)cd;

  symbolArr modarr;
  bool valid = true;
  symbolArr sel;
  if ((argc > 1) && (i->ParseArguments(1, argc, argv, sel) == 0)) {
    symbolPtr sym;
    ForEachS(sym, sel) {
      if (sym.get_kind() == DD_MODULE)
        modarr.insert_last(sym);
    }
  }
  else
    valid = false;

  if ((valid) && (modarr.size() > 0)) {
                symbolPtr sym;
    ForEachS(sym, modarr) {
      projNode *proj = sym.get_projNode();
      projNode *root = proj->root_project();
      char const *name = sym.get_name();
      projModule *mod = root->find_module(name);
      if (!mod)
              mod = projHeader::find_module(name);
      if (mod) {
        app *ah = mod->restore_module(0);
        if (ah) {
          if (ah->is_modified()) {
            ah->clear_modified();
          }
              }
            }
          }
    ret = TCL_OK;
        }
  else {
    genString usage;
    usage.printf("Usage: %s module1 [ module2 module3 ... ]", argv[0]);
    Tcl_AppendResult(interp, usage.str(), NULL);
  }

  return ret;
}

extern smtTree *GetSmtFromMarker(symbolPtr&);
extern char const *ste_smt_get_native_string(smtTreePtr);

int SaveAsCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[])
{
  int ret = TCL_ERROR;

  Interpreter *i = (Interpreter *)cd;

  bool valid = true;

  symbolArr sel;
  symbolPtr mod;
  char const *fname = NULL;

  if ((argc == 3) && (i->ParseArgString(argv[1], sel) == 0)) {
    symbolPtr sym;
    ForEachS(sym, sel) {
      if (sym.get_kind() == DD_MODULE)
        mod = sym;
    }
    fname = OSPATH(argv[2]);
  }
  else
    valid = false;

  if (valid && mod.isnotnull() && fname) {
    valid = false;

    symbolPtr xsym = mod.get_xrefSymbol();
    if (!xsym.xrisnull()) {
      if(xsym->is_loaded_or_cheap(mod.get_xref()->get_lxref())) {
        appPtr app = xsym.get_def_app();
        if (app && is_smtHeader(app)) {
          if (xsym.get_kind() == DD_MODULE) {
            smtTree *smt = (smtTree *)app->get_root();
            if (smt) {
              if ((smt = (smtTree *)smt->get_root()) != NULL) {
                char const *buf = ste_smt_get_native_string(smt);
                if (buf) {
                  ofstream os(fname, ios::out);
                  if (!os.bad()) {
#ifndef _WIN32
                    os << buf;
#else // _WIN32
                    char const *p = buf;
                    while (*p && !os.bad()) {
                      if (!((*p == '\r') && (*(p + 1) == '\n')))
                        os << *p;
                      p++;
                    }
#endif // _WIN32
                    if (!os.bad())
                      valid = true;
                  }
                }
              }
            }
          }
        }
      }
    }

    if (valid)
      Tcl_SetResult(interp, (char *)"1", TCL_VOLATILE);
    else
      Tcl_SetResult(interp, (char *)"0", TCL_VOLATILE);
    ret = TCL_OK;
  }
  else {
    genString usage;
    usage.printf("Usage: %s module path", argv[0]);
    Tcl_AppendResult(interp, usage.str(), NULL);
  }
  
  return ret;
}

extern app *get_create_app_by_name(char const *);

int TempNameCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[])
{
  Initialize(TempNameCmd);

  int nRet = TCL_ERROR;
  
  char const *pszDir = NULL;
  char const *pszPrefix = NULL;
  
  if( argc > 1 )
    pszDir = argv[1];
  if( argc > 2 )
    pszPrefix = argv[2];
  
  char *pszName = OSapi_tempnam( pszDir, pszPrefix );

  if( pszName ) {
    Tcl_SetResult( interp, (char *)OSPATH( pszName ), TCL_VOLATILE );
    free( pszName );
    nRet = TCL_OK;
  }

  return nRet;
}


extern "C" int smtIFL_iff_parseCMD(ClientData cd, Tcl_Interp *interp, int argc, char const **argv);

int cli_question_save_all(symbolArr& arr)
{
  Initialize(cli_question_save_all);

  int answer = 0; 
  genString cmd = "list_save_all \"Save before update\" ";
  call_cli_callback(NULL, (char const *)cmd, &arr, NULL);
  genString res;
  interpreter_instance->GetResult(res);
  if (res.str() && !strcmp(res.str(), "all"))
    answer = 1;
  return answer;
}

static int RunMailCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[]) {
    Interpreter *i = (Interpreter *)cd;
#ifdef _WIN32
    if (argc<3) {
      Tcl_AppendResult(i->interp, "Usage: sent_mail <address> <subject> <text>", NULL);
      return TCL_ERROR;
        }

        int addrCount;
        char const **addrPtrs;
        if(Tcl_SplitList(i->interp,argv[1],&addrCount,&addrPtrs)!=TCL_OK) {
      Tcl_AppendResult(i->interp, "Can't extract from address list.", NULL);
      return TCL_ERROR;
        }
    MapiRecipDesc* to_list;
        to_list= new MapiRecipDesc[addrCount];
    ZeroMemory(to_list,addrCount*sizeof(MapiRecipDesc));
        int j;
        for(j=0;j<addrCount;j++) {
            to_list[j].ulRecipClass=(ULONG)MAPI_TO;
            to_list[j].lpszName = (CHAR*)addrPtrs[j];
            to_list[j].lpszAddress=(CHAR*)"";
        }
    HMODULE mapiModule;
    MapiMessage message;
    ZeroMemory(&message,sizeof(MapiMessage));
        message.lpszSubject=(CHAR*)argv[2];
        message.lpszNoteText=(CHAR*)argv[3];
        message.flFlags=MAPI_RECEIPT_REQUESTED;
        message.nRecipCount=addrCount;
        message.lpRecips=to_list;
    mapiModule=::LoadLibrary("MAPI32.DLL");
        if(mapiModule!=NULL) {
                LPMAPISENDMAIL p;
                p = (LPMAPISENDMAIL) GetProcAddress(mapiModule,"MAPISendMail");
        if(p!=NULL) {
            (*p)(0,0,&message,0,0);
                } else {
                        delete to_list;
            Tcl_AppendResult(i->interp, "Can't find <MAPISendMail> function in the  MAPI32.DLL", NULL);
            return TCL_ERROR;
                }
        } else {
                delete to_list;
        Tcl_AppendResult(i->interp, "Can't find library MAPI32.DLL", NULL);
        return TCL_ERROR;
        }
        delete to_list;
    return TCL_OK;
#else 
   Tcl_AppendResult(i->interp, "Not implemented under UNIX. Available only under Windows NT", NULL);
   return TCL_ERROR;
#endif
}

static int EchoDateCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[])
{
  Interpreter *shell = (Interpreter *)cd;

  char const *date = customize::date();

  if (date) {
    Tcl_SetResult(shell->interp, (char *)date, TCL_VOLATILE);
  }

  return TCL_OK;
}

//==============================================================================

/***************************************************************
 *
 * Routines and commands for managing an interpreter-independent
 * table of symbol arrays:
 *
 ***************************************************************/

class namedSymbolArray : public objDictionary {
  public:
    namedSymbolArray(char const *name) : objDictionary(name) {}
    symbolArr syms;
    define_relational(namedSymbolArray, Relational);
};
generate_descriptor(namedSymbolArray, Relational);

init_relational(namedSymbolArray, Relational);

static objSet api_global_syms;

static int api_put_syms(char const *key, symbolArr &syms)
{
  int ret = 0;
  namedSymbolArray *bucket = (namedSymbolArray *)obj_search_by_name(key, api_global_syms);
  if (!bucket) {
    bucket = new namedSymbolArray(key);
    if (bucket) {
      api_global_syms.insert(bucket);
    }
  }
  if (bucket) {
    //bucket->syms = syms;
    bucket->syms.removeAll();
    bucket->syms.insert_last(syms);
    ret = 1;
  }
  return ret;
}

static int api_get_syms(char const *key, symbolArr &syms)
{
  int ret = 0;
  namedSymbolArray *bucket = (namedSymbolArray *)obj_search_by_name(key, api_global_syms);
  if (bucket) {
    syms = bucket->syms;
    ret = 1;
  }
  return ret;
}

static int api_rem_syms(char const *key)
{
  int ret = 0;
  namedSymbolArray *bucket = (namedSymbolArray *)obj_search_by_name(key, api_global_syms);
  if (bucket) {
    api_global_syms.remove(bucket);
    delete bucket;
    ret = 1;
  }
  return ret;
}

int SetSymCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[])
{
  Initialize(SetSymCmd);

  int ret = TCL_ERROR;
  Interpreter *shell = (Interpreter *)cd;

  if (argc > 2) {
    char const *key = argv[1];
    symbolArr syms;
    if (shell->ParseArguments(2, argc, argv, syms) == 0) {
      api_put_syms(key, syms);
      ret = TCL_OK;
    }
  } else {
    genString usage;
    usage.printf("Usage: %s key value", argv[0]);
    Tcl_AppendResult(interp, usage.str(), NULL);
  }

  return ret;
}

int GetSymCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[])
{
  Initialize(GetSymCmd);

  int ret = TCL_ERROR;
  Interpreter *shell = (Interpreter *)cd;

  if (argc > 1) {
    TclList list;
    int i;
    for (i = 1; i < argc; i++) {
      list += argv[i];
    }
    if (list.Validate()) {
      symbolArr result;
      int size = list.Size();
      for (i = 0; i < size; i++) {
        char const *key = list[i];
        symbolArr syms;
        if (api_get_syms(key, syms)) {
          result.insert_last(syms);
        }
      }
      DI_object answer;
      DI_object_create(&answer);
      symbolArr *arrptr = get_DI_symarr(answer);
      *arrptr = result;
      shell->SetDIResult(answer);
      ret = TCL_OK;
    }
  }

  if (ret != TCL_OK) {
    genString usage;
    usage.printf("Usage: %s key1 [ key2 key3 ... ]", argv[0]);
    Tcl_AppendResult(interp, usage.str(), NULL);
  }

  return ret;
}

int RemSymCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[])
{
  Initialize(RemSymCmd);

  int ret = TCL_ERROR;

  if (argc > 1) {
    TclList list;
    int i;
    for (i = 1; i < argc; i++) {
      list += argv[i];
    }
    if (list.Validate()) {
      int size = list.Size();
      for (i = 0; i < size; i++) {
        api_rem_syms(list[i]);
      }
      ret = TCL_OK;
    }
  }

  if (ret != TCL_OK) {
    genString usage;
    usage.printf("Usage: %s key1 [ key2 key3 ... ]", argv[0]);
    Tcl_AppendResult(interp, usage.str(), NULL);
  }

  return ret;
}

int ListSymCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[])
{
  Initialize(ListSymCmd);

  Interpreter *shell = (Interpreter *)cd;

  TclList list;
  Obj *obj;
  ForEach (obj, api_global_syms) {
    namedSymbolArray *item = (namedSymbolArray *)obj;
    char const *name = item->get_name();
    if (name) {
      list += name;
    }
  }

  Tcl_SetResult(shell->interp, (char *)list, TCL_VOLATILE);

  return TCL_OK;
}

extern "C" void driver_exit(int exit_status);
static int ServerStopCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[]) {
  Interpreter *i = (Interpreter *)cd;
  if(argc<2) {
          Tcl_AppendResult(i->interp, "Usage : stop_server -y",NULL);
          return TCL_ERROR;
  }
  send_client_notification("stop");
  if(strcmp(argv[1],"-y")==0) driver_exit(0);
  return TCL_OK;
}

static int add_pset_access_cmds()
{
  new cliCommandInfo("modified_objects", modified_objectsCmd);
  new cliCommandInfo("get_instance_statement", GetInstanceStatementCmd);
  new cliCommandInfo("region_of_statement",region_of_statementCmd);
  new cliCommandInfo("instances_in_region",instances_in_regionCmd);
  new cliCommandInfo("update_module", ParseModuleCmd);
  new cliCommandInfo("src_update", update_moduleCmd);
  new cliCommandInfo("src_lines", src_linesCmd);
  new cliCommandInfo("src_load", src_loadCmd);
  new cliCommandInfo("src_unload", src_unloadCmd);
  new cliCommandInfo("src_reload", src_reloadCmd);
  new cliCommandInfo("lunp", LoadedUnparsedFilesCmd);
  new cliCommandInfo("get_loaded_files", LoadedFilesCmd);
  new cliCommandInfo("save", SaveModuleCmd);
  new cliCommandInfo("unmod", UnmodCmd);
  new cliCommandInfo("save_as", SaveAsCmd);
  new cliCommandInfo("tempnam", TempNameCmd);
  new cliCommandInfo("iff_parse", smtIFL_iff_parseCMD);
  new cliCommandInfo("ospath", OspathCmd);
  new cliCommandInfo("send_mail", RunMailCmd);
  new cliCommandInfo("echo_date", EchoDateCmd);
  new cliCommandInfo("setsym", SetSymCmd);
  new cliCommandInfo("getsym", GetSymCmd);
  new cliCommandInfo("remsym", RemSymCmd);
  new cliCommandInfo("listsym", ListSymCmd);
  new cliCommandInfo("stop_server", ServerStopCmd);
  return 0;
}

static int dummy = add_pset_access_cmds();
