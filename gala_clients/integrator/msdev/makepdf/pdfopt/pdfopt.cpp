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
#include <io.h>
#include <iostream.h>
#include <string.h>
#include <stdlib.h>				
#include <stdio.h>
#include <fcntl.h>

/***********************************************************************************/
//		M A I N
/***********************************************************************************/
int main ( int argc, char* argv[] )
{
	FILE*	filePDFOpt;
	char	szOptLine	[ 1024 ];
	char	szOptOut	[ 1024 ];
	char*	pszOptText;
	int		nFileNameLen, nProjNameLen;

	if ( argc < 4 
			|| (nProjNameLen=strlen(argv[2])) == 0
			|| (nFileNameLen=strlen(argv[3])) == 0
		 )
		 return 1;

	if ( !( filePDFOpt = fopen( argv[1], "r+t" ) )  )
		return 2;

	szOptLine[0] = '\0';
	while ( !feof( filePDFOpt )  )
		{
		 if ( fgets( szOptLine, sizeof(szOptLine)-1, filePDFOpt ) == NULL )
			  {
				 fclose( filePDFOpt );
				 return 3;
				}
		 if ( (pszOptText = strstr(szOptLine, argv[2])) )
					break;
		}
	
	if ( feof( filePDFOpt )  )
		{
		 fclose( filePDFOpt );
		 return 4;
		}

	strcpy(szOptOut, pszOptText + nProjNameLen );
	while ( !feof( filePDFOpt )  )
		{
		 if ( fgets( szOptLine, sizeof(szOptLine)-1, filePDFOpt ) == NULL )
			  {
				 fclose( filePDFOpt );
				 return 3;
				}
		 if ( *szOptLine == '*' )
				 break;

		 if ( (pszOptText = strstr(szOptLine, argv[3])) )
			  {
				 strcat( szOptOut, pszOptText + nProjNameLen );
				 break;
			  }
		}
	
	printf(" cl -n %s %s\n", argv[3], szOptOut );
	fclose( filePDFOpt );
	return 0;
}
