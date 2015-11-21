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
// the output of this command must be compatible with the "date" command on
// Unix, since Unix date is used in the make system to generate this
// delicately-formatted global variable.

// look in ...paraset/driver/src/main.C and ...paraset/driver/pmod_server/main.cxx for dependencies on build_date

#include <fstream.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

int main()
{
	int result_code = 0;
	_tzset();
	
	// use this because _tzname can contain long version like "Eastern Day..."
	char const * const TZName[] = {"EST", "EDT"};

	time_t cur_time = time( &cur_time );
	char * tmpTime = ctime( &cur_time );
	char pszTime[100];
	struct tm *local_time = localtime(&cur_time);
	sprintf(pszTime, "%.20s%s %.5s", tmpTime, TZName[local_time->tm_isdst], tmpTime + 20);
	// For some reason, ctime() inserts LF at the end of the time string
	// We will check for a LF and put a 0-terminator instead
	int nLastChar = strlen( pszTime ) - 1; 
	if( nLastChar && pszTime[ nLastChar ] == (char)0xA )
		pszTime[ nLastChar ] = '\0';
	
	cout << "extern \"C\" char const build_date[]=\"" << pszTime << "\"" << ";" << endl; 

	char* ADMMAKE=getenv("ADMMAKE");
	char path[2048];
	strcpy(path,ADMMAKE);
	strcat(path,"/market_version");
	ifstream market_version_file(path);
	if (!market_version_file.good()) {
		result_code = 1;
	} else {
		char version[1024];
		market_version_file >> version;
		cout << "extern \"C\" char const build_version[]=\"" << version << "\"" << ";" << endl; 
	}
        return result_code;
}


