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
#define RECOVERY_BUFFER_SIZE    128
#define TRANSMIT_START_SEQ      "Z0R0"
#define TRANSMIT_START_SEQ_LEN  4 // Make sure that this constant is less than RECOVERY_BUFFER_SIZE!

typedef void (*PEXECUTEPROC) (char *pszCommand, UINT nLen);

class ReadSocket
{

public:
  ReadSocket( SOCKET &, PEXECUTEPROC );
  ~ReadSocket();
  void ProcessData();
  void ChangeSocket( SOCKET &NewSocket ){ m_Sock = NewSocket; }; 

protected:
  SOCKET m_Sock;
  enum Mode { START_SEEK_MODE, LENGTH_FILL_MODE, DATA_FILL_MODE };
  char m_RecoveryBuffer[ RECOVERY_BUFFER_SIZE ];
  Mode m_Mode;
  char *m_pDataBuffer;
  UINT m_nBytesToRead;
  UINT m_nBytesAlreadyRead;
  PEXECUTEPROC m_pExecute;
};

BOOL MyMemStr( void *, UINT, void *, UINT );
