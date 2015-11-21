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

#ifndef ERR_MESSAGES_H
#define ERR_MESSAGES_H

#define ERR_DBG       1
#define WNG_DBG       5
#define MSG_DBG     255

/******************************************************************************************
  External messages
******************************************************************************************/

#define	INTEGRATOR_WINDOW_TITLE			"IntegratorServer"

#define EXTMSG_DIS_STARTED				  "DISCOVER Started"
#define EXTMSG_DIS_CMSUPPORT			  "Loading CM Support"
#define EXTMSG_DIS_CONNECTION			  "Connecting to DISCOVER server "
#define	EXTMSG_DIS_LOADING					"Loading DISCOVER "
#define EXTMSG_DIS_CONNECTED				"DISCOVER connected"
#define EXTMSG_DIS_FINISHED					"Integration finished"
#define EXTMSG_ERR_NETMAP				    "Error in UnixNetName2LocalMapped"
#define EXTMSG_ERR_SOCKET						"DISCOVER disconnected"
#define EXTMSG_IDE_FINISHED			    "Integration is finished"
#define EXTMSG_IDE_ERR_STARTING     "Error in integration startup..."
#define EXTMSG_SCC_DUMMYNAME				"DISCOVER Dummy CM"
#define EXTMSG_SCC_NOTSUPPORTED			"Source Control operations is not supported"

#endif  // #ifndef ERR_MESSAGES_H
