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
#include "integrator.h"
#include <stdio.h>

//----------------------------------
//
extern "C" int CALLBACK WinMain(
        HINSTANCE hInstance,
        HINSTANCE hPrevInstance,
        LPSTR lpCmdLine,
        int nCmdShow)
{
	DDEIntegratorClient DDEClient;
	char			*pszParam, *pszSelection, *pszLine= NULL, *pcLine, *pszFName, *pszLineNum;
	HGLOBAL		ghszLine;
	int				nColumn, nPosition, nOffset, nTab;

  nTab = GetRegistryInt("Software\\Microsoft\\Developer\\Text Editor\\Tabs/Language Settings\\C/C++",
                        "TabSize", 8) -1;

	if ( lpCmdLine == NULL )						// CmdLine = "<IDE> <ProjectName> <Param1> <Param2> ..."
		return 1;
	
	pszParam = strchr( lpCmdLine, ' ');		// pszParam = "<ProjectName> <Column> <Selection> <FName> <Line>"
	if ( pszParam == NULL )
		return 1;

	pszParam = strchr( ++pszParam, ' ');	// pszParam = " <Column> <Selection> <FName> <Line>"
	if ( pszParam == NULL )
		return 1;

	if ( !DDEClient.ConnectToServer( DDE_TOPIC_PARAMRECIEVER ) )
			{
			MessageBox( NULL, "Server not started!", "IDE Param", MB_OK);
			return 1;
			}

  *(pszParam++) = '\0';
	nOffset = nColumn = nPosition = atoi(pszParam) -1;
	pszParam = strchr( pszParam, ' ');
 	pszSelection = ++pszParam;						// pszParam = "<Selection> <FName> <Line>"
	pszFName = strchr( pszParam, ' ');
	*(pszFName++) = '\0';									// pszFName = "<FName> <Line>"
  pszLineNum = strchr( pszFName, ' ' );
  *(pszLineNum++) = '\0';

//-- Get source line from Clipboard
  if ( GetDisRegistryInt(REG_USE_CLIPBOARD, 0)           &&
       IsClipboardFormatAvailable(CF_TEXT)							 && 
			 OpenClipboard(NULL)															 &&
			 ( ghszLine = GetClipboardData(CF_TEXT) )
		 )
		{
		pszLine = pcLine = (char*)GlobalLock(ghszLine); 
		if (pszLine != NULL)
			{ 
				while ( nColumn >0 )
				{
					if ( *pcLine++ == '\t' )
					{
						nColumn -= nTab;
						nOffset -= nTab;
					}
					nColumn--;
				}
				if ( memcmp( pszLine + nOffset, pszSelection, strlen(pszSelection) ) != 0 )
					nOffset = nPosition;

				GlobalUnlock(ghszLine); 
			} 
			CloseClipboard();
		}
 				
//-- Send result to Server
	char	szBuffer[512];
	sprintf(szBuffer, "%s %s %d %d %s", pszFName, pszLineNum, nTab+1, nOffset, pszSelection);
	if ( !DDEClient.SendDataToServer( (char*)lpCmdLine, szBuffer ) )
		return 1;

	DDEClient.Disconnect();
	return 0;
}

