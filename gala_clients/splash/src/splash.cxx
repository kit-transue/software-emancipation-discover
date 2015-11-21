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
// 	$Id: splash.C 1.6 1998/09/02 14:31:33EDT rpasseri Exp  $	

#include <vport.h> // Load the Galaxy preprocessor symbols

// #include the necessary Galaxy headers
// but do it in the most preprocessor-friendly way possible
// using the order defined in vgalaxy.h to keep the depth
// of inclusion at a minimum

#ifndef vstdlibINCLUDED
#include vstdlibHEADER
#endif
#ifndef vdebugINCLUDED
#include vdebugHEADER
#endif
#ifndef vmemINCLUDED
#include vmemHEADER
#endif
#ifndef vrectINCLUDED
#include vrectHEADER
#endif
#ifndef vcharINCLUDED
#include vcharHEADER
#endif
#ifndef vdictINCLUDED
#include vdictHEADER
#endif
#ifndef vnameINCLUDED
#include vnameHEADER
#endif
#ifndef veventINCLUDED
#include veventHEADER
#endif
#ifndef vresourceINCLUDED
#include vresourceHEADER
#endif
#ifndef vclassINCLUDED
#include vclassHEADER
#endif
#ifndef vcommandINCLUDED
#include vcommandHEADER
#endif
#ifndef vcolorINCLUDED
#include vcolorHEADER
#endif
#ifndef vdrawINCLUDED
#include vdrawHEADER
#endif
#ifndef vwindowINCLUDED
#include vwindowHEADER
#endif
#ifndef vapplicationINCLUDED
#include vapplicationHEADER
#endif
#ifndef vdialogINCLUDED
#include vdialogHEADER
#endif
#ifndef vmenubarINCLUDED
#include vmenubarHEADER
#endif
#ifndef vnoticeINCLUDED
#include vnoticeHEADER
#endif
#ifndef vstartupINCLUDED
#include vstartupHEADER
#endif
#ifndef invertINCLUDED
#include "splash.h"
#endif

static char* file;

class callbackEvent : public veventClientEvent
{
  public:
    virtual int Handle();
};

int callbackEvent::Handle ()
{
    vfsPath* stopPath = new vfsPath();
    stopPath->Set((vchar*)file);
    stopPath->ExpandVariables();
    if (stopPath->GetType() != vfsPATH_TYPE_FULL) {
        vfsPath* relativePath = stopPath;
        stopPath = vfsPath::CloneWorkingDir();
        stopPath->Join(relativePath);
    }
    stopPath->MakeCanonical();

    vfsPath* parent = stopPath->Clone();
    parent->Append(vfsPath::GetParentElementString(), NULL);
    parent->MakeCanonical();

    // We need to stop if the parent doesn't exist or the stopPath does.
    if (!parent->Access(vfsFILE_EXISTS)) {
	vevent::StopProcessing();
    } else if (stopPath->Access(vfsCAN_READ)) {
	vevent::StopProcessing();
    } else {
        callbackEvent* cbfn = new callbackEvent ();
        cbfn->PostFuture(1,0);
    }

    delete parent;
    delete stopPath;

    return 0;
}


// main -- main driver function
//
// -> argc  -- number of command line args
// -> argv  -- text of the command line args
// <-       -- EXIT_SUCCESS if the program completed successfully
//             EXIT_FAILURE otherwise

int main(int argc, char **argv)
{
  // Initialize the Galaxy managers
  vstartup(argc, argv);

  vbool status = vTRUE;

  if (argc < 2) {
      printf("*** SPLASH Error:Usage splash dialogName stopFilePath.\n");
      status = vFALSE;
  }

  if (status) {
      if (argc == 3) file = argv[2];
      else file = "StopProcessing";
  }

  // Get dialog from the resource file.
  vresource dialogResource = NULL;
  if (status) {
      vexWITH_HANDLING {
          dialogResource = vresourceGet(vapplication::GetCurrent()->GetResources(),
        		                vnameInternGlobalLiteral(argv[1]) );
      }
      vexON_EXCEPTION{
         printf("*** SPLASH Error:Unable to find dialog '%s'\n", argv[1]);
         status = vFALSE;
      } vexEND_HANDLING;
  }

  if (status) {
      // set our dialog up
      vdialog *mainDialog = new vdialog(dialogResource);

      // get this show on the road
      mainDialog->Place(vwindow::GetRoot(), vrectPLACE_CENTER, vrectPLACE_CENTER);
      mainDialog->SetTitle((vchar*)"DISCOVER");
      mainDialog->Open();

      callbackEvent* cbfn = new callbackEvent ();
      cbfn->PostFuture(1,0);

      vevent::Process();

      // clean up after ourselves
      delete mainDialog;
  }

  // hasta la vista, baby
  exit(EXIT_SUCCESS);
  return EXIT_FAILURE;
}
