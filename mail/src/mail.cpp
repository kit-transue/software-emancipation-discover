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
// mail.cpp : Defines the entry point for the console application.
//
#include <windows.h>
#include <stdio.h>
#include "mapi.h"

int main(int argc, char* argv[]) {
MapiRecipDesc* to_list;
MapiRecipDesc* resolved_name;
HMODULE mapiModule;
int recip;

    mapiModule=::LoadLibrary("MAPI32.DLL");

	if(mapiModule==NULL) {
		printf("No Simple MAPI DLL detected (MAPI32.DLL)\n");
		return -1;
	}


	LPMAPIRESOLVENAME resolve;
	LPMAPISENDMAIL    send;
	LPMAPIFREEBUFFER  freeb;

	resolve  = (LPMAPIRESOLVENAME) GetProcAddress(mapiModule,"MAPIResolveName");
	send     = (LPMAPISENDMAIL)    GetProcAddress(mapiModule,"MAPISendMail");
	freeb    = (LPMAPIFREEBUFFER)  GetProcAddress(mapiModule,"MAPIFreeBuffer");
	if(resolve==NULL || send==0 || freeb==0) {
		printf("Incorrect MAPI32.DLL\n");
		return -1;
	}
	to_list= new MapiRecipDesc[argc-1];

    ZeroMemory(to_list,(argc-1)*sizeof(MapiRecipDesc));

	int j;
	int empty_lines;
	recip = 0;
	for(j=0;j<argc-1;j++) {
        (*resolve)(0,0,argv[j+1],0,0,&resolved_name);
		if(resolved_name!=NULL) {
	        to_list[j].ulRecipClass=MAPI_TO;
	        to_list[j].lpszName = strdup(resolved_name->lpszName);
	        to_list[j].lpszAddress="";
			recip++;
		    (*freeb)(resolved_name);
		}

	}
    printf("Subject : ");

	char line[1024];
	char subject[5000];
	subject[0]=0;
	char text[100000];
	text[0]=0;

    empty_lines=0;

	do {
	   int count = 0;
	   while(gets(line)==NULL) {
		   count++;
		   if(count==100) break;
	   }
	   if(count==100) break;
	   if(strlen(line)>0) {
		   strcat(subject,line);
		   empty_lines = 0;
	   } else {
		   empty_lines++;
	   }
	} while(empty_lines < 1);

	do {
		int count =0;
		while(gets(line)==NULL) {
			count++;
		    if(count==100) break;
		}
		if(count==100) break;
        strcat(text,line);
        strcat(text," \n");
	} while(1);


    static MapiMessage message;
    ZeroMemory(&message,sizeof(MapiMessage));
	message.lpszSubject=subject;
	message.lpszNoteText=text;
	message.flFlags=0;
	message.nRecipCount=recip;
	message.lpRecips=to_list;
    (*send)(0,0,&message,0,0);
	return 0;
}
