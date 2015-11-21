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

#define CBFN
#include "Args.h"
#include "transaction.h"
#include "machdep.h"
#include <linkTypes.h>
#include "xxinterface.h"
#include <tcl.h>
#include <tk.h>
#include <globals.h>
#include "Interpreter.h"

extern Args* DIS_args;
extern char *cli_script;
bool  cli_exit  = true;

extern ostream* pmod_log_stream;

typedef void (*CB_funptr)(int fd);

typedef struct fdata {
  int fd;
  CB_funptr funptr;
} file_data;

// timer handler to send heart beat every 2 seconds
void timerHandler(ClientData cd)
{
  _lhb();
  Tcl_CreateTimerHandler (2000, timerHandler, NULL);
}

int service_init(int argc, char**argv)
{
    DIS_args = new Args(argc, argv);
    return 0; 
}

#ifndef _WIN32
void read_notification_proc(ClientData cd, int mask)
{
  file_data *f_data = (file_data *)cd;
  int fd = f_data->fd;
  CB_funptr func_ptr = (CB_funptr)f_data->funptr;
  
  switch(mask) {
  case TCL_READABLE:
    (*func_ptr)(fd);
    break;
  case TCL_EXCEPTION:
    Tcl_DeleteFileHandler(fd);
    delete f_data;
    Tcl_Exit(1);
    break;
  default:
    break;
  }
}

void register_read_fd(int fd, CB_funptr cb)
{
    file_data *fdata = new file_data;
    fdata->fd = fd;
    fdata->funptr = cb;
    int mask = TCL_READABLE | TCL_EXCEPTION;
    Tcl_CreateFileHandler(fd, mask, read_notification_proc, fdata);
}
#endif

extern "C" void driver_exit(int);

//******************************************************************************
//Handle auto shutting down of service.
//

class Shutdown_service {
public:
    Shutdown_service();
    ~Shutdown_service();
    static void start(long);
    static void shutdownHandler(ClientData);
    static void start_autoShutdown_timer();
    static void cancel_autoShutdown_timer();

    static Tcl_TimerToken autoShutdown_timer;
    static bool auto_shutdown_flag;
    static unsigned long time_in_secs; 
};

Tcl_TimerToken Shutdown_service::autoShutdown_timer=0;
bool Shutdown_service::auto_shutdown_flag = false;
unsigned long Shutdown_service::time_in_secs = 3600; //default 1 hour

Shutdown_service::Shutdown_service() {}

Shutdown_service::~Shutdown_service()
{
    cancel_autoShutdown_timer();
}

void Shutdown_service::start(long shutdown_time)
{
    auto_shutdown_flag = true;

    if (shutdown_time > 0) {
	time_in_secs = shutdown_time;
    }

    start_autoShutdown_timer(); 
}

void Shutdown_service::shutdownHandler(ClientData )
{
    (*pmod_log_stream) << "Auto shutdown timer off, shutting down service." << endl;
    driver_exit(4);
}

void Shutdown_service::start_autoShutdown_timer()
{
    if (auto_shutdown_flag) {
        autoShutdown_timer = Tcl_CreateTimerHandler (time_in_secs*1000, Shutdown_service::shutdownHandler, NULL);    
    }
}

void Shutdown_service::cancel_autoShutdown_timer()
{
    if (auto_shutdown_flag && autoShutdown_timer != 0) {
        Tcl_DeleteTimerHandler(autoShutdown_timer);
        autoShutdown_timer = 0;
    }
}

//There is a new connection to the service, cancel the timer.
void notify_first_client_started()
{
    Shutdown_service::cancel_autoShutdown_timer();
}

//The last connection has been closed, start timer. When the timer goes off
//service will be shutdown. 
void notify_last_client_closed()
{
    Shutdown_service::start_autoShutdown_timer();
}


void tcl_eventloop()
{
  while (1) {
    Tcl_DoOneEvent(0);
  }
}

char* cli_service_name;

int service_loop(long shutdown_time)
{

  cli_set_global_var(NULL, "cli_service", cli_service_name);
  if(cli_script != NULL){
    (*pmod_log_stream) << "Executing " << cli_script << endl;
    start_transaction() {
      cli_eval_file(cli_script, (*pmod_log_stream), NULL);
    } end_transaction();
  }
  if(cli_exit)
    return 0;
    
  //extern void cli_read_input(int fd);
  
  Tcl_TimerToken tim = Tcl_CreateTimerHandler ( 2000, timerHandler, NULL);    
  
  //Enable auto shutdown service to bring the service down if there no
  //connections for a period of time.
  if (shutdown_time >= 0) {
      Shutdown_service::start(shutdown_time);
  }

  tcl_eventloop();
  
  (*pmod_log_stream) << "service exiting" << endl;
  return 0;
}

void fast_exit(int exit_status)
{
    error_levels stat = (exit_status <0) ? CRASH_ERROR : (error_levels) exit_status;
    OS_dependent::_exit(globals::error_status(stat));
}

typedef void (*hook_type) (int);

struct socket_notification_data {
    hook_type proc;
    int       fd;
};

void socket_notification_proc(ClientData cd, int mask)
{
  socket_notification_data *notf_data = (socket_notification_data *)cd;
  int fd = notf_data->fd;
  
  switch(mask) {
  case  TCL_READABLE:
    notf_data->proc(fd);
    break;
  case TCL_EXCEPTION:
    break;
  default:
    break;
  }
}

#ifndef _WIN32
void pmod_server_unregister_listener(void *data)
{
  socket_notification_data *notf_data = (socket_notification_data *)data;
  int fd = notf_data->fd;
  Tcl_DeleteFileHandler(fd);
  delete notf_data;
} 

void *pmod_server_register_listener(int socket, hook_type proc) {

    socket_notification_data *notification_data = new socket_notification_data;
    notification_data->proc = proc;
    notification_data->fd = socket;
   
    int mask = TCL_READABLE | TCL_EXCEPTION;
    Tcl_CreateFileHandler(socket, mask, socket_notification_proc, notification_data);

    return notification_data;
}
#endif
