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
// This is a -*- C++ -*- header file

#ifndef _mappedFile_h
#define _mappedFile_h

#include <iosfwd>

//++OSPORT
//#ifndef _pdufile_h
//#include <pdufile.h>
//#endif
//--OSPORT

//++ADS
class parray;
//--ADS

class mappedFile
{
public:
  // Public Constructors/Destructors
  virtual ~mappedFile(void);

  // Public Interface
  static mappedFile  *getFile(char *);
  static int          reset(void);

  inline char        *filename(void) const;
  inline int          fileExists(void) const;
  unsigned int        fileSize(void);
  int                 resolveRC(unsigned int, unsigned int, long, unsigned int &, unsigned int &);
  int                 toBytePos(unsigned int, unsigned int, unsigned int &);
  int                 toRowCol(unsigned int, unsigned int &, unsigned int &);
  virtual void        dumpTo(ostream &) const;

protected:
  // Protected Constructors
  mappedFile(void);
  mappedFile(char *);
  
private:
  // Private Constants
  static const int DEFAULT_NUMBER_OF_LINES;
  static const int DEFAULT_NUMBER_OF_FILES;
  static const int DEFAULT_SCAN_BUFFER_SIZE;

  // Private Members
  char          *the_filename;
  int            scanned;
  int            exists;
  parray        *lines;
  static parray *currentFiles;

  // Private Methods
  int scanFile(void);
  inline int fileScanned(void) const;
};

char *mappedFile::filename(void) const
{
  return the_filename;
}

int mappedFile::fileScanned(void) const
{
  return scanned;
}

int mappedFile::fileExists(void) const
{
  return exists;
}
	
#endif
