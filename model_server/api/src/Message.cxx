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
#include <string.h>
#else /* ISO_CPP_HEADERS */
#include <cstring>
namespace std {};
using namespace std;
#endif /* ISO_CPP_HEADERS */

#include <tcl.h>
#include "msg.h"
#include "Interpreter.h"

#ifdef WIN32
// This dummy variable is here for the braindead linker on NT.  If it 
// doesn't have a symbol referenced from another object file, it won't 
// link the file in.  So Interpreter.C sets this dummy variable to 1 
// so NT will pull this .obj file in.
int message_dummy_var = 0;
#endif

static int msgCmd(ClientData, Tcl_Interp *interp, int argc, char const **argv)
{
    int retval = TCL_ERROR;

    if ( argc >= 3 )
    {
	if ( interp )
	{
	    // Figure out severity
	    msg_sev severity = unknown_sev;
	    bool sev_ok = true;

	    if (   (strcmp(argv[1], "normal_sev") == 0)
		|| (strcmp(argv[1], "-normal") == 0))
	    {
		severity = normal_sev;
	    }
	    else if (   (strcmp(argv[1], "warning_sev") == 0)
		     || (strcmp(argv[1], "-warning") == 0))
	    {
		severity = warning_sev;
	    }
	    else if (   (strcmp(argv[1], "error_sev") == 0)
		     || (strcmp(argv[1], "-error") == 0))
	    {
		severity = error_sev;
	    }
	    else if (   (strcmp(argv[1], "catastrophe_sev") == 0)
		     || (strcmp(argv[1], "-catastrophe") == 0))
	    {
		severity = catastrophe_sev;
	    }
	    else if (   (strcmp(argv[1], "unknown_sev") == 0)
		     || (strcmp(argv[1], "-unknown") == 0))
	    {
		severity = unknown_sev;
	    }
	    else
	    {
		sev_ok = false;
	    }

	    if (sev_ok)
	    {
		ostream *msg_strm = &(msg (argv[2], severity));
		for (int i = 3; i < argc; i++)
		{
		    if (msg_strm)
		    {
			msg_strm = &(*msg_strm << argv[i] << eoarg);
		    }
		}
		if (msg_strm)
		{
		    *msg_strm << eom;
		    retval = TCL_OK;
		}
		else
		{
		    Tcl_AppendResult(interp, "message could not be generated", 0);
		}
	    }
	    else
	    {
		Tcl_AppendResult(interp, "invalid severity", 0);
	    }
	}
	else
	{
		Tcl_AppendResult(interp, "command interpreter unavailable", 0);
	}
    }
    else
    {
	Tcl_AppendResult(interp, "too few arguments", 0);
    }

    return retval;
}

static int init_message(void)
{
    new cliCommandInfo("msg", msgCmd);
    return 0;
}

static int dummy = init_message();

