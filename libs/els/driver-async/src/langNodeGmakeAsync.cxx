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
#include <langNodeGmakeAsync.h>
//--ELS

const char * const langNode::gmakeAsync::sepString = "|%|";

langNode::gmakeAsync::gmakeAsync(commChannel::CHANNEL c, langNode *obj) : commChannel::Async(c)
{
  owner          = obj;
  leftOver       = new char [ 4096 ];
  buffer         = GET_MEMORY(char, commFDPipe3::PACKET_SIZE+1);
  intermediates  = new parray(0);

  if ( leftOver )
    leftOver[0] = 0;
}

langNode::gmakeAsync::~gmakeAsync(void)
{
  FREE_MEMORY(buffer);
  FREE_MEMORY(leftOver);

  pdstring_delete(intermediates);
  FREE_OBJECT(intermediates);
}

int langNode::gmakeAsync::handle(commChannel *chnl)
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
	      
	      // read as a string
	      if ( (nbytes = chnl->read(buffer, commFDPipe3::PACKET_SIZE)) == 0 )
		{
		  // gmake is done, close the sorters input line
		  owner->gmakeFinished = 1;
		  owner->sorterPipe->close(commChannel::CHANNEL_WRITE);
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
			      // write the intermediate filename onto the sorter pipe
			      if ( owner->sorterPipe->write(filename, pdstrlen(filename)) >= 0 )
				// write a newline
				if ( owner->sorterPipe->write("\n", pdstrlen("\n")) < 0 )
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

int langNode::gmakeAsync::parse(char *string, parray *files)
{
  int retval = -1;
  

  if ( string && files )
    {
      char filebuf[4096];
      const char *seppos1 = 0;
      const char *seppos2 = 0;
      const char *start   = string;

      pdstring_delete(files);

      do
	{
	  seppos2 = 0;
	  if ( (seppos1 = pdstrstr(start, langNode::gmakeAsync::sepString)) != 0 )
	    {
	      if ( (seppos2 = pdstrstr(seppos1 + pdstrlen(langNode::gmakeAsync::sepString), 
				       langNode::gmakeAsync::sepString)) != 0 )
		{
		  if ( pdstrncpy(filebuf, 
				 seppos1 + pdstrlen(langNode::gmakeAsync::sepString), 
				 seppos2 - seppos1 - pdstrlen(langNode::gmakeAsync::sepString)) != 0 )
		    {
		      filebuf[seppos2 - seppos1 - pdstrlen(langNode::gmakeAsync::sepString)] = 0;
		      files->insert((void *)pdstrdup(filebuf));
		    }
		  start = seppos2 + pdstrlen(langNode::gmakeAsync::sepString);
		}
	    }
	}
      while ( seppos1 && seppos2 );
  
      if ( seppos1 )
	{
	  int indx;
	  const char *i;
	  for ( indx = 0, i = seppos1; *i; i++, indx++ )
	    string[indx] = *i;
	  string[indx] = 0;
	}
      else
	{
	  string[0] = 0;
	}

      retval = 1;
    }

  return retval;
}
