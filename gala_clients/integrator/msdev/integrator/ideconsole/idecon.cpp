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
#include "idecon.h"
#include <stdio.h>
#include <process.h>

/***************************************************************************
	Main Function
****************************************************************************/
int main(int argc, char *argv[])
{
	if ( GetDisRegistryInt(REG_DEBUG_PRMRECIEVER, 0) )
    _asm int 3;

	if ( argc <= 1 )
		return 1;

	DDEIntegratorClient DDEClient;
	char		szConsoleName[256],	szOutputText[512];

	if ( !DDEClient.ConnectToServer( DDE_TOPIC_CONSOLE ) )
		{
			char	szCmdLine[3*_MAX_PATH];

      GetDisRegistryStr(REG_DISCOVER_DIR, szCmdLine, sizeof(szCmdLine), "c:\\");
      strcat( szCmdLine, EXE_INTEGRATOR );
      strcat( szCmdLine, " ");
      strcat( szCmdLine, DDE_TOPIC_CONSOLE );
      strcat( szCmdLine, " ");
      strcat( szCmdLine, argv[1] );
      strcat( szCmdLine, " ");
      if ( argc < 3 )
        strcat( szCmdLine, "(null)" );
      else
        strcat( szCmdLine, argv[2] );

	    int nCmdShow = SW_HIDE;
      if ( GetDisRegistryInt(REG_SHOW_WINDOW, 0) )
        nCmdShow = SW_MINIMIZE;
    
			if ( WinExec(szCmdLine, nCmdShow) > 31 )
				return 0;
			else
			{
				printf("%s\n", EXTMSG_IDE_ERR_STARTING);
				_flushall();
				return 1;
			}
		}

	if ( argc < 3 )
    sprintf( szConsoleName, "%s (null)", argv[1] );
  else
    sprintf( szConsoleName, "%s %s", argv[1], argv[2] );
	if ( !DDEClient.RecieveDataFromServer( szConsoleName, szOutputText, sizeof(szOutputText) ) )
		return 1;

	printf("%s\n", szOutputText);
	_flushall();
	DDEClient.SendDebugMessage( MSG_DBG, szOutputText );
	DDEClient.Disconnect();

	return 0;
}

