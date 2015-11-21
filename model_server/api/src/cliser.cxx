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
#include "Interpreter.h"
#include "Relational.h"
#include "cLibraryFunctions.h"
#include "charstream.h"
#include "machdep.h"
#include "objArr.h"

struct clientData : public Relational {
  genString user;
  genString host;
  int pid;
  void * data;
  Interpreter *interp;
  clientData(char*,char*,int,int);
  ~clientData();
};

extern char* cli_service_name; 
ostream* pmod_log_stream = &cout;

clientData::clientData(char*u,char*h,int p, int id):user(u),host(h),pid(p)
{
  this->data = 0;
  this->interp = Interpreter::Create();
  char buf[12];
  OSapi_sprintf(buf, "%d\0", id);
  Tcl_SetVar(this->interp->interp, (char *)"cli_client", buf, TCL_GLOBAL_ONLY);   
  Tcl_SetVar(this->interp->interp, (char *)"cli_service", cli_service_name, TCL_GLOBAL_ONLY);   
}
clientData::~clientData()
{
  delete interp;
}

objArr clients; 

void   cli_client_exit(int client_id)
{ 
   (*pmod_log_stream) << "client " << client_id << ": exit" << endl;  
   clientData* data = (clientData*)clients[client_id];
   delete data;
   clients[client_id] = NULL;
}

