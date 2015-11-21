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
#include <pdupath.h>
#include <pduproc.h>
#include <pdustring.h>
#include <pdumem.h>
#include <parray.h>

#include <vport.h>
#include <vchar.h>
#include <vstdio.h>
#include <vstdlib.h>
#include <vsignal.h>
#include <vfs.h>

#include vsessionHEADER
#include vdasservHEADER

#include vcommHEADER
#include vscrapHEADER
#include vmemHEADER
#include vstdlibHEADER

#include "vdasservcmds.h"

static vbool bvdasserv_isalive=FALSE;  //global variable denoting status of Vdasserv

/* The following two functions are called when a message comes back from a vdasserver */

static void serviceNotify(vsessionStatement* statement,
				    vsessionStatementEvent *event)
{
	int code = vsessionGetStatementEventCode(event);
	if (code == vsessionStatementEVENT_COMPLETED) 
	{
		bvdasserv_isalive=TRUE;
	} else {
		bvdasserv_isalive=FALSE;
	
	}
}

static void BrokerNotify(vsession *session, vsessionEvent *event)
{
	vsessionEventCode code = vsessionGetEventCode(event);
	switch( code )
	{
	case vsessionEVENT_CANNOT_BEGIN:
		{
			bvdasserv_isalive=FALSE;
			break;
		}
	case vsessionEVENT_BEGIN:
		{
			bvdasserv_isalive=TRUE;
			break;
		}
	default:
		{
			bvdasserv_isalive=FALSE;
		}
	}
}
																							


/*-------------------------------------------------------------------------- 
purging a vdasserv is needed if you  shut down a pmodserver or a client incorrectly.
avoid doing so, as purging is EXTREMELY slow (~60 seconds).
------------------------------------------------------------------------------------*/
int vdasserv_purge(void)
{
	int nRet=-1;
	int nNumCommands=10;

	parray args(0);

	args.insert((void *)"-purge");
	
	if(vdasserv_command(&args) == 0)
	nRet=0;

	return nRet;
}

//extern Server DISuiServer;
//1 if it found a server 0 if it didn't, -1 if a massive error.
//the variable bvdasserv_isalive gets set by the service and broker notify functions.

 int vdasserv_isalive(void)
 {
	 int nRet=0;

	 vsessionStatement *statement;
	 vscrap *scrapSpec;
	 vsession *sessionBroker = new vsession;
	 
	 if( sessionBroker )
	 {
		 
		 sessionBroker->SetAttributesFromScrap(vdasserv::CreateAttributeScrap());
		 
		 sessionBroker->SetAttributeTag(vsession_Transport, vcommAddress::GetTcpTransportName());
		 
		 sessionBroker->SetObserveSessionProc(BrokerNotify);
		 
		 vexWITH_HANDLING 
		 {
			 sessionBroker->Begin();
			 scrapSpec = vscrapCreateDictionary();
			 
			 statement = vdasserv::MakeMatchStatement(sessionBroker, scrapSpec);
			 statement->SetObserveStatementProc(serviceNotify);
			 statement->SetDestroyWhenFinished (vTRUE);
			 
			 statement->Send();
			 statement->Block(NULL);
			 
			 sessionBroker->End();
		 }
		 vexON_EXCEPTION;
		 vexEND_HANDLING;
		 delete sessionBroker;
	 }
	 if (bvdasserv_isalive)
		 nRet = 1;
	 
	 return  nRet;
 }

 



 //used to generate the command for vdasserv
 int vdasserv_command(parray * args)
 {
	 int nRet=-1;
	 const char * vdasserv_exec_command= "$PSETHOME/bin/vdasserv";
	 
	 char * executable = pdexecpath(vdasserv_exec_command);
	 if (executable) {
		 int size = args->size();
		 char ** argv = GET_MEMORY(char *, size+2); //one for the vdasserv one for the null at the end
		 if (argv) {
			 argv[0] = executable;
			 for (int i=0; i<size; i++) {
				 argv[i+1] = (char *) ((*args)[i]);
			 }
			 argv[size+1] = NULL;
			 
			 nRet=pdprocEXECVP((const char * const *)argv, 0);
			 
			 FREE_MEMORY(argv);
		 }
		 FREE_MEMORY(executable);
	 }

	 return nRet;
 }
