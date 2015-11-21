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
//++OSPORT
#include <pdustring.h>
//--OSPORT

//++IPC
#include <commChannel.h>
#include <commChannelAsync.h>
#include <commFDPipe3.h>
//--IPC

//++ELS
#include <langNode.h>
#include <langNodeFilterAsync.h>
//--ELS

langNode::filterAsync::filterAsync(commChannel::CHANNEL c, int *doneFlag, char **retFlags) : commChannel::Async(c)
{
  *doneFlag = 0;
  **retFlags = 0;

   done  = doneFlag;
   flags = retFlags;
}

langNode::filterAsync::~filterAsync(void)
{
}

int langNode::filterAsync::handle(commChannel *chnl)
{
  int retval = -1;

  if ( chnl )
    {
      if ( channel() == commChannel::CHANNEL_READ )
	{
	  // read as a string
	  char *buffer = new char [commFDPipe3::PACKET_SIZE+1];
	  if ( buffer )
	    {
	      int nbytes = chnl->read(buffer, commFDPipe3::PACKET_SIZE);
	      buffer[nbytes] = 0;
	      
	      // translate all newlines into spaces
	      for ( int i = 0; i < nbytes; i++ )
		if ( buffer[i] == '\n' )
		  buffer[i] = ' ';
	      
	      // add to returnable flags
	      pdstrcat(*flags, buffer);
	      
	      // if read was 0 we are done
	      if ( nbytes == 0 )
		*done = 1;

	      delete [] buffer;

	      retval = 1;
	    }
	}
    }

  return retval;
}

