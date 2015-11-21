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
#include <stdlib.h>
#include <time.h>
#include <ctype.h>
#include <sys/types.h>
#include <string.h>
#include <sys/stat.h>

static char* replyBuffer = NULL;


//---------------------------------------------------------------------------------------------------------
// This function will unwrap client request and will extract following information from it:
// 1. Service name
// 2. Access command with all data
//---------------------------------------------------------------------------------------------------------
void unwrapRequest( char* httpRequest) {

}


//---------------------------------------------------------------------------------------------------------
// This function will wrap all data we need to send as a reply string into the HTTP headers.
// --------------------------------------------------------------------------------------------------------
char* WrapReply(char* reply, int len) {
char* header = "HTTP/1.0 200 OK";
char* rlen   = "Content-Length: ";
char buf[100];

   if(replyBuffer!=NULL) delete replyBuffer;
   replyBuffer = new char[len+1000]; // We are adding some space for the HTTP headers

   // Reply status OK
   strcpy(replyBuffer,header);
   strcat(replyBuffer,"\r\n");

   // Reply type
   //strcat(replyBuffer,"Content-Type: application/discover");
   strcat(replyBuffer,"Content-Type: application/discover\r\n");
   // Reply length
   if(len>0) {
      strcat(replyBuffer,rlen);
      sprintf(buf,"%d",len);
      strcat(replyBuffer,buf);
      strcat(replyBuffer,"\r\n");
   }

   // Reply separator
   strcat(replyBuffer,"\r\n");

   // Reply itself
   strcat(replyBuffer,reply);

   return replyBuffer;
}
//---------------------------------------------------------------------------------------------------------
