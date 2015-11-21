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
#ifndef __EMACSADAPTER_HEADER_FILE
#define __EMACSADAPTER_HEADER_FILE

#include "EditorInterface.h"
#ifdef _WIN32
// On NT _eof returns true if empty or end of file reached.
// On Unix and Win95 return false on empty.
//    This causes read to hang.
#define EOFILE(strm) (_eof(fileno(strm)))
#else
// On Unix _eof returns true if empty or end of file reached.
// We add function which sets read to non blocking mode.
// On Win95 it currently still hangs.
#define EOFILE(strm) (feof(strm))
#endif

#define MAXCOMMANDLENGTH 1026
#define EMACSOK "OK"

class CEmacsAdapter:public EditorInterface {
 public:
  CEmacsAdapter(string name);
  virtual ~CEmacsAdapter();

  bool terminate(){ return m_bExit;};
  bool checkSocket(int socket);
  bool processEditorCommand();
  int WaitForEvent(long usec,long timeout); // timeout in hund. of sec and seconds
                                   // returns 1 - if some event occured
								   // or 0 - if timed out  

  virtual bool acceptClientConnection();
  virtual string getEditorName(){return m_szName;};
  virtual int startEditor() {return 1;};
  virtual int unregisterInterface();
  virtual int registerInterface();
  virtual int open(string filename, int line);
  virtual int mark(string filename, int line, int column, int selectionLength){return 1;};
  virtual int info(string message);
  virtual int freeze(){return 1;};
  virtual string file(){return "null";};
  virtual int line(){return -1;};
  virtual int lineoffset(){return -1;};
  virtual int closeDriver(int);
  virtual int privateLine2ModelLine(string localFile, string baseFile, int line) {return line;};
  virtual int modelLine2PrivateLine(string localFile, string baseFile, int line){return line;};
 protected:
  bool isEditorMessageAvailable();
  int getFromEditor(char* message);
  bool sendToEditor(const char* message);

  string m_szName;
  bool m_bExit;
  int m_nConnections;
  char	m_szEmacsVersion[256];
};

#endif //__EMACSADAPTER_HEADER_FILE
