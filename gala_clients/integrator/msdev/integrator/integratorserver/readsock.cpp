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
#include <windows.h>
#include <winsock.h>
#include "readsock.h"


ReadSocket::ReadSocket( SOCKET &NewSock, PEXECUTEPROC pExecuteProc )
{
  m_Sock = NewSock;
  m_pDataBuffer = NULL;
  m_Mode = START_SEEK_MODE;
  m_nBytesToRead = 0;
  m_nBytesAlreadyRead = 0;
  m_pExecute = pExecuteProc;
}


ReadSocket::~ReadSocket()
{
  if( m_pDataBuffer )
  {
    delete m_pDataBuffer;
    m_pDataBuffer = NULL;
  }
}

void ReadSocket::ProcessData()
{
  int nRecvResult;
  switch( m_Mode )
  {
  case START_SEEK_MODE:
    {
      // Check if recovery buffer is filled up. If yes, copy last TRANSMIT_START_SEQ_LEN - 1 bytes
      // to the beginning of the buffer, and start to fill it from the next byte
      if( m_nBytesAlreadyRead >= (RECOVERY_BUFFER_SIZE - 1) )
      {
        memmove( (void *)m_RecoveryBuffer, 
                 (void *)(m_RecoveryBuffer + RECOVERY_BUFFER_SIZE - TRANSMIT_START_SEQ_LEN),
                 TRANSMIT_START_SEQ_LEN - 1 );
        m_nBytesAlreadyRead = TRANSMIT_START_SEQ_LEN - 1;
      }
      nRecvResult = recv( m_Sock, m_RecoveryBuffer + m_nBytesAlreadyRead,
                        1 /*read 1 byte at a time*/, 0 );
      // if read was successful
      if( nRecvResult == 1 )
      {
        m_nBytesAlreadyRead++;
        //m_RecoveryBuffer[ m_nBytesAlreadyRead ] = '\0';
        // if transmission start sequence was successfuly received
        if( (m_nBytesAlreadyRead >= TRANSMIT_START_SEQ_LEN) &&
            MyMemStr( (void *)m_RecoveryBuffer, m_nBytesAlreadyRead, (void *)TRANSMIT_START_SEQ, 
                      TRANSMIT_START_SEQ_LEN ) )
        {
          m_Mode = LENGTH_FILL_MODE;
          m_nBytesToRead = sizeof( long );
          m_nBytesAlreadyRead = 0;

        }
      }

    }
    break;
  case LENGTH_FILL_MODE:
    {
      nRecvResult = recv( m_Sock, m_RecoveryBuffer + m_nBytesAlreadyRead,
                        m_nBytesToRead - m_nBytesAlreadyRead, 0 );
      if( nRecvResult > 0 )
        m_nBytesAlreadyRead += nRecvResult;
      if( m_nBytesAlreadyRead >= m_nBytesToRead )
      {
        long *pNum = (long *)m_RecoveryBuffer;
        m_nBytesToRead = (UINT) ntohl( *pNum );
        m_pDataBuffer = new char [ m_nBytesToRead ];
        if( m_pDataBuffer )
        {
          m_Mode = DATA_FILL_MODE;
        }
        else
        {
          m_Mode = START_SEEK_MODE;
          memset( (void *)m_RecoveryBuffer, '\0', RECOVERY_BUFFER_SIZE );
          m_nBytesToRead = 0;
        }
        m_nBytesAlreadyRead = 0;
      }

    }
    break;
  case DATA_FILL_MODE:
    {
      if( m_pDataBuffer )
        nRecvResult = recv( m_Sock, m_pDataBuffer + m_nBytesAlreadyRead,
                        m_nBytesToRead - m_nBytesAlreadyRead, 0 );
      if( nRecvResult > 0 )
        m_nBytesAlreadyRead += nRecvResult;
      if( m_nBytesAlreadyRead >= m_nBytesToRead )
      {

         if( m_pExecute )
           (*m_pExecute)( m_pDataBuffer, m_nBytesToRead );
         
         //Now we are deleting a data buffer;
         delete m_pDataBuffer;
         m_pDataBuffer = NULL;
         m_Mode = START_SEEK_MODE;
         memset( (void *)m_RecoveryBuffer, '\0', RECOVERY_BUFFER_SIZE );
         m_nBytesToRead = 0;
         m_nBytesAlreadyRead = 0;
      }
    }
    break;
  }
}

BOOL MyMemStr( void *pvBuffer, UINT nBufferSize, void *pvString, UINT nStringSize )
{
  BOOL fFound = FALSE;

  for( UINT i = 0; i <= (nBufferSize - nStringSize); i++ )
  {
    if( !memcmp( (void *)((char *)pvBuffer + i), pvString, nStringSize ) )
    {
      fFound = TRUE;
      break;
    }
  }
  return fFound;
}
