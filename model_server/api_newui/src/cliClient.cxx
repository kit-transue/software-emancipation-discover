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
#include <vport.h>
#include "tcl.h"
#include "Interpreter.h"
#include "charstream.h"
#ifndef ISO_CPP_HEADERS
#include <stdlib.h>
#include <ctype.h>
#include <fstream.h>
#else /* ISO_CPP_HEADERS */
#include <cstdlib>
using namespace std;
#include <cctype>
#include "fd_fstream.h"
#endif /* ISO_CPP_HEADERS */
#include <windows.h>
#include <process.h>
#include <io.h>
#include <genStringPlus.h>
#include <cliClient.h>
#include veventHEADER

Interpreter* GetActiveInterpreter();
void         SetActiveInterpreter(Interpreter*);
int          cli_eval_string(const char* cmd, Interpreter *i);

CRITICAL_SECTION client_array_access;
int              client_array_access_initialized = 0;

genArr(cliClientPtr);
static struct clientArr : public cliClientPtrArr {
     ~clientArr();
     void kill(int);
} clients;

typedef struct _ClientEvent {
  cliClientData *client_data;
  char          *data;
  int           len;
} ClientEvent;

#define EVENT_BUFFER_SIZE 50
ClientEvent *event_buffer[EVENT_BUFFER_SIZE];

class pipeEvent : public veventClientEvent {
public:
     int  Handle(void);
     void SetData(int index);
     int  event_index;  //index inside event_buffer
};

//---------------------------------------------------------------------------

static int InsertClientEvent(ClientEvent *ev)
{
  for(int i = 0; i < EVENT_BUFFER_SIZE; i++){
    if(event_buffer[i] == NULL){
      event_buffer[i] = ev;
      return i;
    }
  }
  return -1;
}

static ClientEvent *GetClientEvent(int i)
{
  ClientEvent *ev = event_buffer[i];
  event_buffer[i] = NULL;
  return ev;
}

static void HandleClientEvent(int i)
{
  //If client_event is NULL it means it's already processed.
  ClientEvent *client_event = GetClientEvent(i);
  if(client_event != NULL){
    handle_pipe_buffer((char *)client_event->data, client_event->len, client_event->client_data);
    free(client_event->data);
    delete client_event;
  }
}

//---------------------------------------------------------------------------

cliClientData::~cliClientData()
{
    if(shutdown_script.length() != 0 && client_dead != 0)
	cli_eval_string(shutdown_script, parent_interp);
     delete os;
     delete auxos;
     delete interp;
}

//---------------------------------------------------------------------------

clientArr::~clientArr()
{
     int sz = size();
     for(int ii=0; ii<sz; ++ii){ 
	  cliClientData* cd = * ((*this)[ii]);
	  if(cd)
	       delete cd;
     }
}

void clientArr::kill(int client)
{
     --client;
     if(client < 0 || client >= size())
	  return;

     cliClientPtr& cd = * ((*this)[client]);
     if(cd) {
	  delete cd;
	  cd = NULL;
     }
}

//---------------------------------------------------------------------------

void pipeEvent::SetData(int index)
{
     this->event_index = index;
}

int pipeEvent::Handle(void)
{
  HandleClientEvent(event_index);
  return vTRUE;
}

//---------------------------------------------------------------------------

void ThreadClientMonitorFunc(void *cd)
{ 
     char          buf[1023];
     genStringPlus string;

     cliClientData *data = (cliClientData *)cd;
     while (1){
	  int num = OSapi_read(data->in_file, buf, sizeof(buf) - 1);
	  if(num == 0)
	       break;
	  // scan for 0 (command terminator)
	  for(int i = 0; i < num; i++){
	       char ch = buf[i];
	       if(ch == 0) {
		    ClientEvent *client_event = new ClientEvent;
		    client_event->client_data = data;
		    client_event->data        = OSapi_strdup((char *)string);
		    client_event->len         = strlen((char *)string) + 1;
		    int event_index           = InsertClientEvent(client_event);
		    if(event_index == -1){
		    } else {
		      pipeEvent *ev = new pipeEvent;
		      ev->SetData(event_index);
		      // Post event in galaxy event queue
		      ev->PostAtInterrupt();
		      // Post same event into windows message queue
		      // If waitclient gets it first it will mark it already proccessed.
		      PostThreadMessage(data->main_thread, WM_NULL, 0x1372, (LPARAM)event_index);
		    }
		    string.reset();
	       } else
		    string += ch;
	  }
	  
     } 
     // Perform actions needed before thread termination.
     EnterCriticalSection(&client_array_access);
     data->client_dead = 1;
     clients.kill(data->client_id);
     LeaveCriticalSection(&client_array_access);

     _endthread();
//     return 0; 
} 

//---------------------------------------------------------------------------

int MonitorClient(cliClientData *data, ostream& outstream)
{
  int threadHandle = _beginthread(ThreadClientMonitorFunc, 0, (void *)data);
  if(threadHandle < 0){
    outstream << "Failure to create client monitor" << endl;
    return -1;
  }
  data->client_thread = threadHandle;
  return 0;
}

int vpopen2w3f(int not_std, const char* sh_command, int* read_fd, int* write_fd, int* aux_fd);

int cli_start_client(int not_stdio, char* exec, char *sf, char *shutdown_script,
		     char*cmd, ostream& outstream, Interpreter *parent_interpreter)
{
  if(client_array_access_initialized == 0){
    InitializeCriticalSection(&client_array_access);
    client_array_access_initialized = 1;
  }
     AllocConsole();
     int err       = TCL_OK;
     Interpreter*i = Interpreter::Create();
     i->SetOutputStream(outstream);

     char buf[12];
     OSapi_sprintf(buf, "%d\0", clients.size() + 1);
     Tcl_SetVar(i->interp, "dis_client", buf, TCL_GLOBAL_ONLY);  

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
     cliClientData* cd = new cliClientData;
     cd->pid           = client_pid;
     cd->os            = new fd_ostream(ofile);
     cd->auxos         = new fd_ostream(afile);
     cd->out_file      = ofile;
     cd->in_file       = ifile;
     cd->shutdown_script     = shutdown_script;
     cd->client_id           = clients.size() + 1;
     cd->interp              = i;
     cd->parent_interp       = parent_interpreter;
     cd->osp                 = cd->os;
     cd->client_dead         = 0;
     cd->main_thread         = (unsigned long)GetCurrentThreadId();
     if(MonitorClient(cd, outstream) < 0){
	  delete cd;
	  delete i;
	  return TCL_ERROR;
     }	  
     clients.append(&cd);
     return err;
}

void cli_wait_for_client(int client)
{
  if(client_array_access_initialized == 0){
    InitializeCriticalSection(&client_array_access);
    client_array_access_initialized = 1;
  }
  EnterCriticalSection(&client_array_access);
  if(clients.size() <= client){
    LeaveCriticalSection(&client_array_access);
    return;
  }
  cliClientPtr& cd = *(clients[client]);
  if(!cd){
    LeaveCriticalSection(&client_array_access);
    return;
  }
  HANDLE thread_handle = (HANDLE)cd->client_thread;
  LeaveCriticalSection(&client_array_access);
  int waiting = 1;
  while(waiting){
    MSG msg;
    DWORD res = WaitForSingleObject(thread_handle, 1);
    if(res == WAIT_OBJECT_0 || res == WAIT_FAILED)
      waiting = 0;
    while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
      if(msg.message == WM_NULL && msg.wParam == 0x1372){
	       HandleClientEvent((int)msg.lParam);
      } else
	    // if(msg.message == WM_PAINT)
	         DispatchMessage(&msg);
  }
}

int cli_get_number_of_clients(void)
{
     return clients.size();
}
