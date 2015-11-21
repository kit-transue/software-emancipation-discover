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
#ifndef _interp_h
#define _interp_h

#ifndef _TCL
#include <tcl.h>
#endif

#ifndef vportINCLUDED
#include <vport.h>
#endif

#ifndef veventINCLUDED
#include veventHEADER
#endif

#define TCL_EXECERR(str) {Tcl_AppendResult (interp, argv[0], ": ", str, NULL); \
                          return TCL_ERROR;}
 
 
#define TCL_EXECERR1(str1,str2) {Tcl_AppendResult (interp, argv[0], ": ", str1, \
                                                   " - ", str2, NULL); \
                                 return TCL_ERROR;}

class Viewer;



extern Tcl_Interp* g_global_interp;
extern int interp_trace;  // Trace all TCL calls when non-zero.
extern int interp_errTrace;  // Print all TCL error messages when non-zero.

Tcl_Interp* make_interpreter();
void init_interpreter (Tcl_Interp *interp);

void delete_interpreter(Tcl_Interp*);
void show_interpreter_wait(Viewer*, int);

class Tcl_InterpDeleteEvent : public veventClientEvent {
     public:
   	int Handle(void);
};

#endif
