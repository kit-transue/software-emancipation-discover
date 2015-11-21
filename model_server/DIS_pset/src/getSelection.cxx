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
#include "symbolArr.h"
#include "driver.h"
#include "gRTListServer.h"

//
// getSelection : gets the selection from the UI.  The ID passed in is the id of the RTL.
// If the ID is -1, then get the selection from the Selection Stack.
// 

int InsertSymbolsFromGlobalStack( symbolArr & );

void getSelection (int id, symbolArr &symbols) {

    if (id == -1) 
    { // Work on the viewer selections
      
      int nInserted = InsertSymbolsFromGlobalStack( symbols );
    }

    else if (id == -2) { // Work on the current item in the view (from popup menus)

    }

    else {
        RTListServer* server = RTListServer::find (id);
        if (server) server->getSelArr (symbols);
    }
}


int InsertSymbolsFromGlobalStack( symbolArr &symbols )
{
  int nInserted = 0;
  Obj *pObj = 0;
	OperPoint *pPoint = 0;

  ForEach( pObj, driver_instance->sel_list.oper_pts )
	{
		if( pObj )
		{
			pPoint = (OperPoint *)pObj;
			symbolPtr sym = pPoint->node.get_xrefSymbol();
			if( !sym.isnull() )
			{
				symbols.insert_last( sym );
				nInserted++;
			}
		}	
	}
  return nInserted;
}
