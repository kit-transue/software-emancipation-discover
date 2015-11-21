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
#include <pdumem.h>
//--OSPORT

//++ADS
#include <parray.h>
//--ADS

//++IPC
#include <commChannel.h>
#include <commChannelAsync.h>
#include <commFDPipe3.h>
#include <commFDPipe.h>
//--IPC

//++ELS
#include <jobNode.h>
#include <langNode.h>
#include <langNodeSorterAsync.h>
//--ELS

langNode::sorterAsync::sorterAsync(commChannel::CHANNEL c, langNode *obj) : commChannel::Async(c)
{
  owner         = obj;
  leftOver      = new char [ 4096 ];
  buffer        = GET_MEMORY(char, commFDPipe3::PACKET_SIZE+1);
  intermediates = new parray(0);

  if ( leftOver ) 
    leftOver[0]    = 0;
}

langNode::sorterAsync::~sorterAsync(void)
{
  FREE_MEMORY(buffer);
  FREE_MEMORY(leftOver);

  pdstring_delete(intermediates);
  FREE_OBJECT(intermediates);
}

int langNode::sorterAsync::handle(commChannel *chnl)
{
  int retval = -1;

  if ( chnl )
    {
      // check the status of *this
      if ( intermediates && owner && leftOver && buffer )
	{
	  if ( channel() == commChannel::CHANNEL_READ )
	    {
	      int         n        = 0;
	      int         nbytes   = 0;
	      langNode   *ln       = 0;
	      const char *filename = 0;
	      
	      // check to see of gmake has finished, it it has, close down it's pipe
	      if ( owner->gmakeFinished )
		owner->gmakePipe->close(channelChannel::CHANNEL_NULL);

	      // read as a string
	      if ( (nbytes = chnl->read(buffer, commFDPipe3::PACKET_SIZE)) == 0 )
		{
		  // sorter is done
		  owner->thisFinished();
		  retval = 1;
		}
	      else if ( nbytes < 0 )
		{
		  // read error, terminate
		  owner->thisFinished();
		  retval = -1;
		}
	      else // nbytes > 0
		{
		  retval = 1;
		  
		  buffer[nbytes] = 0;
		  if ( pdstrcat(leftOver, buffer) != 0 )
		    if ( parse(leftOver, intermediates) >= 0 )
		      {
			n = intermediates->size();
			for ( int i = 0; i < n && retval != -1; i++ )
			  {
			    if ( (filename = (const char *)(*intermediates)[i]) != 0 )
			      if ( (ln = new langNode(owner, filename)) != 0 )
				if ( owner->addChild(ln) < 0 )
				  retval = -1;
			  }
			
			if ( retval == -1 )
			  {
			    //error, BAIL!
			    owner->thisFinished();
			  }
		      }
		}
	    }
	}
    }

  return retval;
}

int langNode::sorterAsync::parse(char *string, parray *files)
{
  int retval = -1;

  if ( string && files )
    {
      pdstring_delete(files);

      char filebuf[4096];
      filebuf[0] = 0;
      int findex = 0;

      int len = pdstrlen(string);
      for ( int i = 0; i < len; i++ )
	{
	  if ( string[i] == '\n' )
	    {
	      filebuf[findex] = 0;
	      if ( pdstrlen(filebuf) > 0 )
		{
		  files->insert((void *)pdstrdup(filebuf));
		}
	      filebuf[0] = 0;
	      findex = 0;
	    }
	  else
	    {
	      filebuf[findex++] = string[i];
	    }
	}
      
      // copy what was left over back into string
      filebuf[findex] = 0;
      pdstrcpy(string, filebuf);
      
      retval = 1;
    }

  return retval;
}
