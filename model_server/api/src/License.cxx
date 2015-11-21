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
//++C
#ifndef ISO_CPP_HEADERS
#include <string.h>
#else /* ISO_CPP_HEADERS */
#include <cstring>
namespace std {};
using namespace std;
#endif /* ISO_CPP_HEADERS */
//--C

//++TCL
#include <tcl.h>
//--TCL

//++XXINTERFACE
#include "xxinterface.h"
//--XXINTERFACE

//++API
#include <Interpreter.h>
//--API

#ifdef WIN32
// This dummy variable is here for the braindead linker
// on NT.  If it doesn't have a symbol referenced from
// another object file, it won't link the file in.
// So, /paraset/src/paraset/api/src/Interpreter.cxx sets
// this dummy varaible to 1 so NT will pull this .obj file in.
int xxaccess_dummy_var = 0;
#endif

static int xxaccess(ClientData, Tcl_Interp *interp, int argc, char const **argv)
{
  int retval = TCL_ERROR;

  if ( argc == 3 )
    if ( interp )
      {
	if ( strcmp(argv[2], "has") == 0 )
	  {
	    if ( _lhs(argv[1]) == LIC_SUCCESS )
	      Tcl_AppendResult(interp, "1", 0);
	    else
	      Tcl_AppendResult(interp, "0", 0);
	    retval = TCL_OK;
	  }
	else if ( strcmp(argv[2], "feature") == 0 )
	  {
	    if ( _lfs(argv[1]) == LIC_SUCCESS )
	      Tcl_AppendResult(interp, "1", 0);
	    else
	      Tcl_AppendResult(interp, "0", 0);
	    retval = TCL_OK;
	  }
	else if ( strcmp(argv[2], "message") == 0 )
	  {
	    if ( _lms(argv[1]) == LIC_SUCCESS )
	      Tcl_AppendResult(interp, "1", 0);
	    else
	      Tcl_AppendResult(interp, "0", 0);
	    retval = TCL_OK;
	  }
	else if ( strcmp(argv[2], "co") == 0 )
	  {
	    if ( _los(argv[1]) == LIC_SUCCESS )
	      Tcl_AppendResult(interp, "1", 0);
	    else
	      Tcl_AppendResult(interp, "0", 0);
	    retval = TCL_OK;
	  }
	else if ( strcmp(argv[2], "ci") == 0 )
	  {
	    if ( _lis(argv[1]) == LIC_SUCCESS )
	      Tcl_AppendResult(interp, "1", 0);
	    else
	      Tcl_AppendResult(interp, "0", 0);
	    retval = TCL_OK;
	  }
	else
	  Tcl_AppendResult(interp, "invalid command code", 0);
      }
    else
      Tcl_AppendResult(interp, "command interpreter unavailable", 0);
  else
    Tcl_AppendResult(interp, "wrong number of arguments", 0);

  return retval;
}

static int init_xxaccess(void)
{
  new cliCommandInfo("license", xxaccess);
  return 0;
}

static int dummy = init_xxaccess();

