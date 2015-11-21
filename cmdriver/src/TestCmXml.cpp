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
#include <stdio.h>

#ifdef _WIN32
	#include <windows.h>
	#include <winbase.h>
	#include <winsock.h>
	#include <io.h>
#else
   #include <signal.h>
   #include <arpa/inet.h>
   #include <netinet/tcp.h>
   #include <netdb.h>
   #include <sys/socket.h>
   #include <netinet/in.h>
   #include <unistd.h>
   #include <sys/types.h>
   #include <sys/wait.h>
   #ifdef sun4
      #include <poll.h>
      #include <time.h>
      #include <sys/time.h>
   #endif
   #ifdef irix6
      #include <sys/poll.h>
   #endif
   #ifdef hp700
      #include <sys/poll.h>
      #include <time.h>
      extern "C" int select(int, int*, int*, int*, const struct timeval*);
   #endif
#endif

#include <dom/DOM.hpp>
#include <parsers/DOMParser.hpp>
#include <util/PlatformUtils.hpp>

#include "DevXLauncher/debug.h"
#include "CmXml.h"
#include "CmXmlException.h"

// For debug output
int		do_debug = 0;
FILE*	log_fd = NULL;
char	dbgFileName[1024];


int main(int argc, char* argv[]) {

	// Initialize debug output system
	do_debug = 1;
	strcpy(dbgFileName, "screen");
	_DBG_OPEN();

	// Construct CmXml
	CmXml *cmXml = NULL;
	try {
		cmXml = new CmXml("D:/Xlam/cm.xml");
	}
	catch(const CmXmlException &ex) {
		printf(ex.getMessage());
		return 1;
	}

	// Check loaded 'cm.xml' whether it has no inconsistencies
    char *attr_val = NULL;
    char *cmdLine = NULL;
    char *result = NULL;
	try {
		cmXml->checkConsistency();
		cmXml->getCmSystems();
		cmXml->getAttributes("ClearCase");
		cmXml->getCommands("ClearCase");
        if(cmXml->isAttribute("Source Integrity", "sandbox")) {
            char *attr_val = cmXml->getAttributeValue("Source Integrity", "sandbox");
            if(attr_val == NULL || attr_val[0] == 0) {
                char *args[] = {
                                    "get_attr_value",
                                    "sandbox",
                                    NULL
                                };

                char reply[] = "<cm_attr>"                                    \
                               "  <cm_li key=\"red\" display=\"Red\"/>\n"     \
                               "  <cm_li key=\"green\" display=\"Green\"/>\n" \
                               "  <cm_li key=\"blue\" display=\"Blue\"/>\n"   \
                               "</cm_attr>";

                cmdLine = cmXml->translateCommand("Source Integrity", args);
                result = cmXml->translateResult("Source Integrity", "sandbox", reply);
            }
        }
        if(cmXml->isAttribute("ClearCase", "comment")) {
            attr_val = cmXml->getAttributeValue("ClearCase", "comment");
            if(attr_val == NULL || attr_val[0] == 0) {
                ;
            }
        }
/*
        if(cmXml->isCommand("ClearCase", "co")) {

            char **args = new char* [6];
//
//            args[0] = strdup("lsco");
//            args[1] = strdup("comment=\"Temporary checking out!\"");
//            args[2] = strdup("reserved=\"unreserved\"");
//            args[3] = strdup("C:/Work/Proj/foo.c");
//            args[4] = strdup("C:/Work/Proj/bar.h");
//            args[5] = NULL;
//
            args[0] = strdup("co");
            args[1] = strdup("Comment=\"Temporary checking out!\"");
            args[2] = strdup("reserved=\"unreserved\"");
            args[3] = strdup("C:/Work/Proj/foo.c");
            args[4] = NULL;

            int files_start = -1;
            char *cmdLine = cmXml->translateCommand("ClearCase", args, &files_start);
        }
*/
	}
	catch(const CmXmlException &ex) {
        printf("\nFATAL ERROR!!!\n");
		printf(ex.getMessage());
		return 1;
	}

	// Free resources
	delete cmXml;

	return 0;

} //main


// END OF FILE
