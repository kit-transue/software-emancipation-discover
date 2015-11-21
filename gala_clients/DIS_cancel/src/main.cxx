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
#include <vport.h>
#include vstartupHEADER
#include vstdioHEADER
#include vstdlibHEADER

#include "../../communications/include/Application.h"
#include "../../communications/include/Args.h"

int main (int argc, char**argv)
{
    vdebugTraceEnter(main);
    vstartup(argc, argv);   // Delayed startup of classes.
 
    DIS_args = new Args(argc, argv);
    const char* group = Application::getGroupName();
 
    Application* DIScancel_app;
    extern Server DIScancelServer;
    void DIScanceServerInit();
 
    DIScancel_app = new Application;
    if (DIScancel_app->registerService (group, "DIScancel", DIScancelServer)) {
        fprintf (stderr, "Cannot register service %s:DIScancel; exiting.\n", group);
        exit (EXIT_FAILURE);
        return EXIT_FAILURE;
    }  else
        fprintf (stderr, "Service  %s:DIScancel available.\n", group);
 
    vevent::Process();

    vdebugTraceLeave ();
    exit (EXIT_SUCCESS);
    return EXIT_FAILURE;
}
