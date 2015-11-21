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
#include "tcl.h"
#include "Interpreter.h"
#include <machdep.h>
#ifndef ISO_CPP_HEADERS
#include <stdio.h>
#include <iostream.h>
#include <fstream.h>
#else /* ISO_CPP_HEADERS */
#include <cstdio>
using namespace std;
#include <iostream>
#include <fstream>
#endif /* ISO_CPP_HEADERS */
#include "vpopen.h"
#include "cliClient.h"
#include <cLibraryFunctions.h>
#include <gtBase.h>
#include <msg.h>

Interpreter* GetActiveInterpreter();
void         SetActiveInterpreter(Interpreter*);
int          cli_eval_string(const char* cmd, Interpreter *i);

extern "C" void vpclose_pid(int pid);

cliClientData::~cliClientData()
{
    if(shutdown_script.str() && client_dead)
	cli_eval_string(shutdown_script, parent_interp);
    delete os;
    delete auxos;
    vpclose_pid(pid);
    delete interp;
}

genArr(cliClientPtr);
static struct clientArr : public cliClientPtrArr {
  ~clientArr();
  void kill(int);
} clients;

static int client_is_done(void*pclient)
{
  int client = *(int*) pclient;
  cliClientPtr cd =   *clients[client];
  return cd == NULL;
}
extern "C" void ping_paraset ();
void clientArr::kill(int client)
{
  --client;
  if(client < 0 || client >= size())
    return;

  cliClientPtr& cd = * ((*this)[client]);
  if(cd) {
    cd->hook->remove_hook();
    delete cd;
    cd = NULL;
  }
  ping_paraset();
}

clientArr::~clientArr()
{
  int sz = size();
  for(int ii=0; ii<sz; ++ii){
    cliClientData* cd = * ((*this)[ii]);
    if(cd)
      delete cd;
  }
}

static void cli_client_hook(int fd, void *data)
{
    cliClientData *cd = (cliClientData*) data;
    
    if(cd->level > 0){
      msg("Pending read for client $1", normal_sev) << cd->client_id << eoarg << eom;
      return;
    }
    char buf[1024];
    int len = OSapi_read(fd, buf, 1023);
    if(len == 0){
	cd->client_dead = 1;
	clients.kill(cd->client_id);
	return;
    }
    cd->level++;
    for(int i = 0; i < len; i++){
      char ch = buf[i];
      if(ch == 0){
	cd->hook->remove_hook();
	handle_pipe_buffer((char *)cd->string, strlen((char *)cd->string), cd);
	cd->hook->set_hook(cli_client_hook, XtInputReadMask, (char *)cd);
	cd->string.reset();
	break;
      } else
	cd->string += ch;
    }
    cd->level--;
}

int vpopen2w3f(int not_std, const char* sh_command, int* read_fd, int* write_fd, int* aux_fd);

static int cli_start_client(int not_stdio, char* exec, char *sf, char *shutdown_script,
			    char*cmd, ostream& outstream, Interpreter *parent_interpreter){

  int err = TCL_OK;
  Interpreter*i = Interpreter::Create();
  i->SetOutputStream(outstream);

  char buf[12];
  OSapi_sprintf(buf, "%d\0", clients.size()+1);
  Tcl_SetVar(i->interp, (char *)"dis_client", buf, TCL_GLOBAL_ONLY);  

  if(sf){
    err = i->EvalFile(sf);
    if(err != TCL_OK) {
      outstream << "error evaluating file " << sf << endl;
      delete i;
      return err;
    }
  }

  if(cmd){
    err = i->EvalCmd(cmd);
    if(err != TCL_OK) {
      outstream << "error evaluating command " << cmd << endl;
      delete i;
      return err;
    }
  }

  int ifile, ofile, afile;  // in file, out file, aux file
  int client_pid = vpopen2w3f(not_stdio, exec, &ifile, &ofile, &afile);
  if(client_pid == 0) {
    outstream << "error starting " << exec << endl;
    delete i;
    return TCL_ERROR;
  }
  ui_INTERP_info *inf = new ui_INTERP_info (ifile);

  cliClientData* cd = new cliClientData;
  
  cd->pid = client_pid;

  cd->os = new ofstream(ofile);
  cd->auxos = new ofstream(afile);
  if(shutdown_script)
      cd->shutdown_script = shutdown_script;
  cd->hook                = inf;
  cd->client_id           = clients.size() + 1;
  cd->interp              = i;
  cd->parent_interp       = parent_interpreter;
  cd->osp                 = cd->os;
  cd->client_dead         = 0;
  cd->hook->set_hook(cli_client_hook, XtInputReadMask, (char *)cd);
  clients.append(&cd);

  return err;
}

static int send_to_clientCmd(ClientData cd, Tcl_Interp *interp, int argc, char **argv)
{
  int ret = TCL_ERROR;

  Interpreter *i = (Interpreter*)cd;

  char *idp = NULL;
  char *msg = NULL;
  int id = 0;

  bool valid = true;

  for (int ii = 1; ii < argc; ii++) {
    if (!strcmp("-id", argv[ii]))
      if (!idp)
        if (ii < argc - 1)
          idp = argv[++ii];
        else
          {valid = false; break;}
      else
        {valid = false; break;}
    else
      if (!msg)
        msg = argv[ii];
      else
        {valid = false; break;}
  }

  if (valid) {
    if (!msg)
      valid = false;
  }

  if (valid) {
    char *idcl = Tcl_GetVar(i->interp, (char *)"dis_client", TCL_GLOBAL_ONLY);
    if (idcl && !idp)
      id = OSapi_atoi(idcl);
    else if (idp)
      id = OSapi_atoi(idp);
    else
      valid = false;
  }

  if (valid) {
    cliClientData *ccd = NULL;
    for (int ii = clients.size() - 1; ii >= 0; ii--) {
      if ((clients[ii]) && (*(clients[ii])) && ((*(clients[ii]))->client_id == id)) {
        ccd = *(clients[ii]);
	break;
      }
    }
    if (ccd) {
      *(ccd->auxos) << msg << ends << flush;
      Tcl_SetResult(interp, (char *)"1", TCL_VOLATILE);
    }
    else
      Tcl_SetResult(interp, (char *)"0", TCL_VOLATILE);
    ret = TCL_OK;    
  }
  else {
    genString usage;
    usage.printf("Usage (client): %s [ -id <cid> ] <data>\nUsage (server): %s -id <cid> <data>", argv[0], argv[0]);
    Tcl_AppendResult(interp, (char *)usage, NULL);
  }

  return ret;
}

static int kill_clientCmd(ClientData cd, Tcl_Interp *interp, int argc, char **argv)
{

  Interpreter*i = (Interpreter*) cd;
  char* arg = Tcl_GetVar(i->interp, (char *)"dis_client", TCL_GLOBAL_ONLY);
  if(arg) { // client
    if(argc != 1){
      Tcl_AppendResult(i->interp,  "no arguments allowed", NULL);
      return TCL_ERROR;
    }
  } else {
    if(argc != 2) {
      Tcl_AppendResult(i->interp,  "wrong number of arguments", NULL);
      return TCL_ERROR;
    }
    arg = argv[1];
  }

  int client = OSapi_atoi(arg);
  clients.kill(client);
  return TCL_OK;
}

static int wait_clientCmd(Interpreter *i, int argc, char **argv)
{
  if(argc != 2) {
    Tcl_AppendResult(i->interp,  "wrong number of arguments", NULL);
    return TCL_ERROR;
  }
  char* arg = argv[1];

  int client = OSapi_atoi(arg);

  arg = Tcl_GetVar(i->interp, (char *)"dis_client", TCL_GLOBAL_ONLY);
  if(arg) { // client
    int cur_client = OSapi_atoi(arg);
    if(cur_client == client){
      Tcl_AppendResult(i->interp,  "client ", arg, " cannot wait for itself", NULL);
      return TCL_ERROR;
    }
  } 
  client--; // index in clients

  gtBase::take_control(client_is_done, &client);

  return TCL_OK;
}

static int start_clientCmd(ClientData cd, Tcl_Interp *interp, int argc, char **argv)
{
  Interpreter * i = (Interpreter*) cd;
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
    int client = clients.size();
    char buf[12];
    OSapi_sprintf(buf, "%d", client);
    Tcl_AppendResult(i->interp,  buf, NULL);
  }
  return err;
}

static int add_commands()
{
    new cliCommandInfo("start_client", start_clientCmd);
    new cliCommandInfo("kill_client", kill_clientCmd);
    new cliCommand("wait_client", wait_clientCmd);
    new cliCommandInfo("send_to_client", send_to_clientCmd);
    return 0;
}
static int add_commands_dummy = add_commands();
