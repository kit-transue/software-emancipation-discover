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
// regularTimer
//----------------------------------------------------------
#ifdef NEW_UI

#include <tipwin.h>
#include "Application.h"
#include "../../DIS_ui/interface.h"
#undef printf
extern Application* DISui_app;
#include <cLibraryFunctions.h>

#ifndef _mytimer_h
#include <mytimer.h>
#endif

#include <genString.h>
#include <driver.h>

extern "C" {
             char* get_main_option(char *);
           }

// regularTimer Observer function.
void regularTimer::ObserveTimer()  {
vchar *retValue;
//    Initialize(regularTimer::ObserveTimer);

    char* clientName = get_main_option ("-client");

    if(clientName) {
       genString command;
       command.printf ("dis_TimerReg");
       retValue = (vchar *)rcall_dis_DISui_eval (DISui_app, (vstr *)command.str());
       if(retValue==NULL || Application::IsConnectionLost() ) {
	   OSapi_printf("\nClient connection is closed.\n");
	   OSapi_printf("Server is shutting down..\n");

	   tipWindow::ResetTimer();

	   // Stop this timer.
	   this->Stop();
	   // Trying to exit GALAXY message loop. If OK, qTimer will be killed by GALAXY.
	   vevent::StopProcessing();
	   // If GALAXY will not finish it will not kill qTimer, so in 5 seconds the timer
	   // exit sequence will take place.
	 }
    }
}
#endif // NEW_UI


