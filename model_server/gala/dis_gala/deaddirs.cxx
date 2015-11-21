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
#ifndef ISO_CPP_HEADERS
#include <string.h>
#else /* ISO_CPP_HEADERS */
#include <cstring>
using namespace std;
#endif /* ISO_CPP_HEADERS */
#include <pdumem.h>
#include "parray.h"
#include "pdufile.h"
#include "pdupath.h"
#include "pdustring.h"
#include "cstring.h"
#include "pdudir.h"
#include <malloc.h>
#ifndef ISO_CPP_HEADERS
#include <stdlib.h>
#else /* ISO_CPP_HEADERS */
#include <cstdlib>
#endif /* ISO_CPP_HEADERS */
/* a function to build a list of dead discover directories.
* It returns them as a Tcl style space-separated list
* it doesn't work with long filenames
* buffer needs to be deleted after you are done
*/

int build_dead_dirs(char * &buffer )
{
	int retval=0;
	const char * pDiscover="DISCOVER";
	const char * pLock    ="Discover.lock";	
	const char * pTmpDir=NULL;
	
	parray  pDirs(32);
	parray * ppDirs=&pDirs;
	int x=-1;
	
	
	pTmpDir=pdgetenv("TMPDIR");
	if (pTmpDir==NULL)
		pTmpDir=pdgetenv("TMP");
	if (pTmpDir==NULL)
		pTmpDir=pdgetenv("TEMP");

	//figure out the temp dir 
	if (pTmpDir)
	{
		//pero says kill the backslashes...

		char * p=(char *)pTmpDir;
		while (*p)
		{if ( *p=='\\')  *p = '/';
		p++;}

		x=pdlsdir( (char * )pTmpDir, ppDirs);
		
		if (x>0)
		{
			cstring final;
			int i;
			int nDiscoverLen=pdstrlen(pDiscover);
			for ( i=0; i<x ; i++)
			{
				
				
				if (!strncmp(pDiscover, (const char *) pDirs[i], nDiscoverLen ))
				{
					
					cstring cF(pTmpDir);
					cF += '/';  //TCL prefers '/' separated paths, even on NT
					cF += (const char *) pDirs[i];
					cF += '/';
					cF += pLock;
					char * test=pdloadfile(cF);
					if (test)
					{
						//build dir without lockfilename at end
						cstring t2(pTmpDir);
						t2+='/';
						t2+=(const char *)pDirs[i];

						//add it to the list of directories
						final+=t2;
						final+=' ';
						retval++;

						FREE_MEMORY(test);
					}
				}
			}
			pdstring_delete(ppDirs);
			
			buffer=pdstrdup(final);
		}
	} else {
		buffer=NULL;
	}

	
	return retval;
}

