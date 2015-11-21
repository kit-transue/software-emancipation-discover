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
#include "model_server_msgser.h"
#include "transport.h"
#include <stdio.h>
#include <stdlib.h>

using namespace nTransport;
namespace MBDriver {

// ===========================================================================
//
ModelMsgSerializer::ModelMsgSerializer()
  : datap(NULL)
{
}

void ModelMsgSerializer::serialize(char* const& cmd, const char *&data_param, size_t &length)
{
    char tmp = 0;
    unsigned char type = 4;
    if (cmd) {
	int cmd_len = strlen(cmd)+1;
	int cmd_len_length = 4; //4 bytes for sending length of command
	int type_len = 1;
	int packet_num_len = 1;
	datap = new char[packet_num_len+type_len+cmd_len_length+cmd_len+1];
	if (datap) {
	    strcpy(datap, "");
	    char *ptr = datap;

	    //picked up the protocol from function SendStop in serverspy.C
	    
	    //packet number
            *ptr++ = tmp;

	    //command type
	    *ptr++ = type; 
       
	    //add length of command, big-endian/network byte order
	    *ptr++ = (cmd_len >> 24) & 255;
	    *ptr++ = (cmd_len >> 16) & 255;
	    *ptr++ = (cmd_len >> 8) & 255;
	    *ptr++ =  cmd_len & 255;

	    //add command
	    strncpy(ptr, cmd, cmd_len);
	    ptr += cmd_len;
 
	    //delimiter
	    *ptr = tmp;
       
	    length = ptr - datap;
	    data_param = datap;
	} 
    }
}

ModelMsgSerializer::~ModelMsgSerializer()
{
    if (datap)
	delete datap;
    datap = 0;
}

}
