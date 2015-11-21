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
#include <vport.h>
#include vmemHEADER
#include vcommHEADER

#include <ginterp.h>
#include <tcpServer.h>
#include <Application.h>

#define START_SEEK_BUFF_LEN		128
#define START_SEEK_SEQ			"Z0R0"
#define START_SEEK_SEQ_LEN		4

int MyMemStr( void *, int, void *, int);

extern int atoi(const char*);

static int cmd_inited;

tcpServer* tcpServer::tcp_servers;

vbool tcpServer::tcpTrace = vFALSE;

typedef struct {
    int nBytes;
	void* buffer;
	vchar portnum[4];
	vchar StartSeekBuff[START_SEEK_BUFF_LEN];
	int nBytesAlreadyRead;
	int nStartSeekMode;
} Data;

static Data* createData()
{
   
   Data* d = new Data;
   d->nBytes = 0;
   d->buffer = NULL;
   d->nStartSeekMode = 1;
   d->nBytesAlreadyRead = 0;

   return d;
}

static void destroyData (Data* d)
{
    delete d;
}

tcpServer::tcpServer()
{
    next = tcp_servers;
    tcp_servers = this;
    channel = NULL;
    eval_mode = 0;
}

tcpServer::~tcpServer()
{
}

int tcpServer::EvalMode()
{
    return eval_mode;
}

void tcpServer::SetEvalMode()
{
    eval_mode = 1;
}

tcpServer* tcpServer::FindServer(vcommChannel* chan)
{
    for (tcpServer* s=tcp_servers;s; s=s->next) 
    {
	if (s->channel == chan)
	    return s;
    }

    for ( s=tcp_servers;s; s=s->next) 
    {
	if (s->EvalMode())
	    return s;
    }

    return NULL;
}

int tcpServer::WriteInt (int i, vcommChannel* chan)
{
    vchar num[4];
    num[0] = (i >> 24) & 255;    
    num[1] = (i >> 16) & 255;    
    num[2] = (i >>  8) & 255;    
    num[3] = (i >>  0) & 255;    
 
    if (chan) return chan->Write (num, 4);
    if(channel)
	return channel->Write (num, 4);
    else
	return 0;
}

int tcpServer::WriteBuffer (int size, void* buffer, vcommChannel* chan)
{
    tcpServer::WriteInt (size, chan);
    if (chan) return chan->Write ((vchar*)buffer, size);
    if(channel)
	return channel->Write ((vchar*)buffer, size);
    else
	return 0;
}

int tcpServer::ReadInt (int& i)
{
    vchar num[4];

    int c = channel->Read (num, 4);
    i = num[0] << 24 |
	num[1] << 16 |
	num[2] << 8 |
	num[3];

   return c;
}

int tcpServer::ReadBuffer (int &i, void*& buffer)
{
    tcpServer::ReadInt (i);
    buffer = vmemAlloc (i);
    return channel->Read ((vchar*)buffer, i);
}

void tcpServer::Notify (vcommChannel  *chan, int event)
{
    // MAXPATHLEN is rough bound on port length
    // MAXHOSTNAME is bound on machine length
    // 16 is rough bound on transport name

    vchar         sFormat[2048];
    Data* data; 
    vcommAddress *remote;
    FILE *fp;

    fp = vdebugGetInfoStream();

    fp = stderr;

    remote = chan->GetRemoteAddress( );
    remote->Format(sFormat, sizeof(sFormat));

	if (GetTcpTrace() ) {
		vcharDump(sFormat, fp);
		fprintf( fp, " ==> ");
	}

    switch(event)
    {
 case  vcommOPEN:
	    if (GetTcpTrace() )
		fprintf(fp, "vcommOPEN\n");

	    // Here we illustrate initializing a data pointer
	    //unique to the channel

	    data = createData();
	    chan->SetClientData(data);

	    if (!cmd_inited) {
		cmd_inited = 1;
	    }

	    break;

	case  vcommCLOSE:
	    if (GetTcpTrace() ) 
		fprintf(fp, "vcommCLOSE\n");

	    // Typically in a server, channels are destroyed when
	    // they are closed because they are automatically
	    // created for each session.
	    // Listeners are typically not destroyed when closed,
	    // nor are channels on the client side.
	    // This check is assuming a notification function for
	    // a server whose listener doesn't have any data
	    // pointer.

	    if ((data = (Data *)chan->GetClientData( )) != NULL) {
		    chan->DeleteLater( );
	    }
	    destroyData(data);
	    chan->SetClientData(NULL);
 	    cmd_inited = 0;
	    break;


	case  vcommINPUT:
	    if (GetTcpTrace() ) 
		fprintf(fp, "vcommINPUT\n");

	    data = (Data *) chan->GetClientData();
	    data->nBytes = 0;

	    if(data->nStartSeekMode)
		  {
        // Check if recovery buffer is filled up. If yes, copy last START_SEEK_SEQ_LEN - 1 bytes
        // to the beginning of the buffer, and start to fill it from the next byte
        if( data->nBytesAlreadyRead >= START_SEEK_BUFF_LEN )
        {
          memmove( (void *)(data->StartSeekBuff), 
                   (void *)(data->StartSeekBuff + START_SEEK_BUFF_LEN - START_SEEK_SEQ_LEN + 1),
                   START_SEEK_SEQ_LEN - 1 );
          data->nBytesAlreadyRead = START_SEEK_SEQ_LEN - 1;
        }

        if (chan->Read(data->StartSeekBuff + data->nBytesAlreadyRead, 1) < 1)
          break;

        // if Read was successful
        (data->nBytesAlreadyRead)++;
        // if transmission start sequence was successfuly received
        if( (data->nBytesAlreadyRead >= START_SEEK_SEQ_LEN) &&
            MyMemStr( (void *)(data->StartSeekBuff), data->nBytesAlreadyRead, 
                      (void *)START_SEEK_SEQ, START_SEEK_SEQ_LEN ) )
        {
          data->nStartSeekMode=0;
          data->nBytesAlreadyRead = 0;
        }
		
  		  break;
  		}
  		else
  		{
	    
        // this is a new message
        // first 4 bytes is the size  (protocol we are using)

        if (chan->Read(data->portnum, 4) < 4)
    	    break;  // the whole size is not here yet
  
        // FALL THROUGH
      }

	case vcommREAD_COMPLETE:
	    if (event == vcommREAD_COMPLETE) {
		    data = (Data *) chan->GetClientData( );
		    if (GetTcpTrace() ) 
			fprintf(fp, "vcommREAD_COMPLETE\n");
	    }
            if (data->nBytes< 0)
	        break;

	    if (data->nBytes == 0) {
		    data->nBytes = (data->portnum[0] << 24
				  | data->portnum[1] << 16
			       	  | data->portnum[2] << 8
				  | data->portnum[3]
				  );
		    data->buffer = vmemAlloc(data->nBytes);
		    if (chan->Read((vchar*)data->buffer, data->nBytes)
		                < data->nBytes)
		        break;  // continue reading async
	    }  // end reading new message


	    // either fell through, or got called a second time
	    // (first time was READ_COMPLETE on size)

			data->nStartSeekMode = 1; // we will need	to search for a start sequence again    
            {
	    tcpServer* s = tcpServer::FindServer(chan);
	    if (s && s->EvalMode()) {
        	int i;
	        if (*(char*)data->buffer == '@')
	            i = Tcl_Eval (g_global_interp, (char*)data->buffer+1);
	        else
	            i = Tcl_Eval (g_global_interp, (char*)data->buffer);
	    
	         
	        if (*(char*)data->buffer != '@') {
		    int wi = s->WriteInt (i, chan);
  	            if (GetTcpTrace() ) 
			fprintf ( fp, "Write [4] = %d\n", wi);
		    wi = s->WriteBuffer (strlen(Tcl_GetStringResult(g_global_interp))+1, Tcl_GetStringResult(g_global_interp), chan);
  	            if (GetTcpTrace() ) 
			fprintf ( fp, "Write [4] = %d\n", wi);
   	       }
	    }
 	    }

	    if (GetTcpTrace() ) 
		fprintf(fp, "\tRead %d bytes: \"%s\"\n",
		    data->nBytes, (char *)data->buffer);

	    data->nBytes = -1;

	vcommOUTPUT:
	    if (GetTcpTrace() )
		fprintf(fp, "vcommOUTPUT\n");
	    break;

	vcommWRITE_COMPLETE:
	    if (GetTcpTrace() )
		fprintf(fp, "vcommWRITE_COMPLETE\n");
	    break;

	vcommEXCEPTION:
	    if (GetTcpTrace() )
		fprintf(fp, "vcommEXCEPTION\n");
	    vexDumpChain(fp);
	    break;
    }  // end switch on event

}


int tcpServer::Open(const char* port)
{
  const vname    *transport;
  vcommAddress    addressTmp;

  // Note that vcommAddress::Init is called for addressTmp by
  // the default constructor.
  // Since vcommListen copies the address, there is no
  // need to create a persistent one.

  transport = (vchar*) vcommAddress::GetTcpTransportName();
  addressTmp.SetTransport(transport);


  if (port && port[0])
      addressTmp.SetPort((vchar*)port);

  channel = new vcommChannel;
  vcommAddress* address = channel->GetRemoteAddress();
  channel->SetObserveChannelProc (tcpServer::Notify); 

  address->SetTransport(transport);
  address->SetPort ((vchar*)port);

  char hostname[256];
  hostname[0] = '\0';
  DIS_args->findValue("-cmdhost", hostname);

  address->SetLocation ((vchar*)hostname);
  channel->Open();

  return 0;
} 

int tcpServer::isOpen()
{
    return channel ? 1 : 0;
}

int tcpServer::Close()
{
    if (channel) channel->Close();
    channel = 0;
    return 0;
}


int MyMemStr( void *pvBuffer, int nBufferSize, void *pvString, int nStringSize )
{
  int fFound = 0;

  for( int i = 0; i <= (nBufferSize - nStringSize); i++ )
  {
    if( !memcmp( (void *)((vchar *)pvBuffer + i), pvString, nStringSize ) )
    {
      fFound = 1;
      break;
    }
  }
  return fFound;
}
