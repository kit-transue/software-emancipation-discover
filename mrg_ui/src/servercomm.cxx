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
#include "machdep.h"
#include "cLibraryFunctions.h"
#include "servercomm.h"

//extern FILE *ffff;

int SendSyncCommand(char *command, gString& reply)
{
    // For 2waypipe:      fdread = 3, fdwrite = 4
    int fdread  = 3;
    int fdwrite = 4;
    int len;
    char head[2];
    
    head[0] = '\2';
    if(OSapi_write(fdwrite, head, 1) != 1) {
        printf("Error: write head failed\n");
        return 0;
    }
    len = strlen(command);
    if(OSapi_write(fdwrite, command, len + 1) == len + 1) {
	char buffer[1024];
	int ll;
	do {
	    ll = OSapi_read(fdread, buffer, 1023);
	    if(ll < 0){
		if(errno == EINTR){
		    errno = 0;
		    continue;
		} else
		    break;
	    }
	    if(ll == 0)
		break;
	    if(buffer[ll - 1] == 0){
		reply += buffer;
		break;
	    } else {
		buffer[ll] = 0;
		reply     += buffer;
	    }
//	    fprintf(ffff, "\n\nReceived input: ");
//	    for(int i = 0; i < ll; i++)
//		fprintf(ffff, "%c", buffer[i]);
//	    fprintf(ffff, "\n");
//	    fflush(ffff);
	} while(1);
//	fprintf(ffff, "\n\nTotal received input: ");
//	fprintf(ffff, "%s", (char *)reply);
//	fprintf(ffff, "\n");
//	fflush(ffff);
        return 1;
    } else {
        printf("Error: write command error\n");
        return 0;
    }
}
