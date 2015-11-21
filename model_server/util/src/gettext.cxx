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
//----------
//
// gettext.C - DISCOVER gettext.
//    Converts "Para" into "Discover".
//
///// Note - This replaces the Unix gettext libC function.
//
//////////

#include <cLibraryFunctions.h>
#include <machdep.h>
#include <psetmem.h>
#ifndef ISO_CPP_HEADERS
#include <string.h>
#else /* ISO_CPP_HEADERS */
#include <cstring>
using namespace std;
#endif /* ISO_CPP_HEADERS */

extern "C" {
char* gettext(char* text)
{
    static char* buffer1 = 0;
    static char* buffer2 = 0;
    static int buflen;
    
    int len;
    char *s, *p;

    if (!text) return text;
    
    len = strlen(text);
    if (!buffer1) {
	buffer1 = (char *)psetmalloc(len*2);
	buffer2 = (char *)psetmalloc(len*2);
    } else if ((len+5) > buflen) {
	buffer1 = (char *)psetrealloc(buffer1, len*2);
	buffer2 = (char *)psetrealloc(buffer2, len*2);
	buflen = len*2;
    }

    if (!strstr(text,"Para")) 
	return text;
   
    strcpy (buffer2, text);
    
    while ((s=strstr(buffer2,"Para")) && isupper(*(s+4))) {

	if ((s = strstr(buffer2, "ParaSET"))) {
	    strncpy(buffer1, buffer2, s-buffer2);
	    buffer1[s-buffer2] = 0;
	    strcat (buffer1, "DIScover");
	    strcat (buffer1, s+7);
	    strcpy(buffer2, buffer1);
	    continue;
	} else
	    strcpy (buffer1, buffer2);
	
	if ((s = strstr(buffer1, "Para")) && isupper(*(s+4))) {
	    strncpy(buffer2, buffer1, s-buffer1);
	    buffer2[s-buffer1] = 0;
	    s += 4;
	    p = buffer2+strlen(buffer2);
	    
	    while (*s && isalpha(*s)) *(p++) = *(s++);
	    *p = 0;
	   //no more "/sw" stuff...  
	    // strcat (buffer2, "/sw");
	    strcat (buffer2, s);
	} else 
	    strcpy (buffer2, buffer1);
    }
    return buffer2;
}


}    // extern "C"





//////////     end of gettext.C     //////////
