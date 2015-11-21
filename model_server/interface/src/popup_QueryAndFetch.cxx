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
// popup_QueryAndFetch.C
//    Functional interface to QueryAndFetch dialogs
// from different UIs.

#include <cLibraryFunctions.h>

#ifndef ISO_CPP_HEADERS
#include <stdlib.h>    // Needed for atoi.
#else /* ISO_CPP_HEADERS */
#include <cstdlib>
using namespace std;
#endif /* ISO_CPP_HEADERS */

#include <msg.h>
#include <messages.h>
#include <genError.h>
#include <cmd.h>
#include <ste_interface.h>
#include <Question.h>

#include <popup_QueryAndFetch.h>


#if defined(NEW_UI)
#include "DIS_main/interface.h"

#else    // NEW_UI not defined.
#include <QueryAndFetch.h>

#endif   // NEW_UI defined.



bool popup_QueryAndFetch( const symbolArr &files_needed)
{
   Initialize(popup_QueryAndFetch);

   bool status = true;

   cmd_validate("QueryAndFetch-input", (symbolArr&) files_needed);
   symbolArr files_to_fetch;
   projNodePtr home = projNode::get_home_proj();
   for (int i = 0; i < files_needed.size(); i++) {
      if (!files_to_fetch.includes(files_needed[i])) {
	 if (!home->find_module(files_needed[i].get_name())) {
	    files_to_fetch.insert_last(files_needed[i]);
	 }
      }
   }
   cmd_validate("QueryAndFetch-final", files_to_fetch);

   if (files_to_fetch.size() > 0) {
        // Get user to confirm the fetch.
#if     defined(NEW_UI)
           genString cmd;
#if defined(GALA_WITHIN_PSET_SERVER)
           cmd.printf("dis_display_query_and_fetch %ld",
                      (const char*)&files_needed );
           char *pszResult = (char *)server_eval( (vchar *)"DISview",
                             (vchar *)cmd.str() );
           int answer = atoi( pszResult );

#else      // Need to talk to gala client.
           int answer = dis_confirm_list("Delete Group", NULL, NULL, files_needed,
                                         "OK to Fetch a group?");
#endif     // Local/remote interface to gala.

           status = answer > 0;

#else   // Not NEW_UI, must be OLD_UI.
           QueryAndFetch q(files_to_fetch);
           status = q.ask() > 0;

#endif // Checking for NEW_UI defined.
   }

   if (status) {
      // checkout the files;
      ste_finalize(0);
      for (int i = 0; i < files_to_fetch.size(); i++) {
         symbolPtr sym = files_to_fetch[i];
         // First, try checking out the module.
         projModulePtr local_mod = projModule::get_module(sym);
         // If check-out failed, try copy.
         if (!local_mod)
            local_mod = projModule::copy_module(sym);
         // If copy failed, display error message.
         if (!local_mod) {
            msg("ERROR: Cannot fetch $1.") << sym.get_name() << eom;
            status = false;
         }
      }
   }


   return(status);
}





//////////     end of popup_QueryAndFetch     //////////
