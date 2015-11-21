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
// This code is a -*- C++ -*- header file

#ifndef _fileSorter_h
#define _fileSorter_h

#ifndef _parray_h
//++ADS
class parray;
//--ADS
#endif

#ifndef _language_h
//++ELS
class language;
//--ELS
#endif

class fileSorter
{
public:

  // Constructor(s) / Destructor(s)
  fileSorter(const language *);
  virtual ~fileSorter(void);

  // Public Interface
  int sortIntoLines(const char *input, parray *lines) const;

protected:

  static const char * const sorterExecPrefKey;
  static const char * const sorterExecFlagsPrefKey;
  static const char * const debuggingFileSorterPrefKey;

  static const char * const sorterExecPrefDefaultVal;
  static const char * const sorterExecFlagsPrefDefaultVal;

  static const char * const inputFileMarker;
  static const char * const outputFileMarker;

private:
  const char *getSorterExec(void) const;
  const char *getSorterExecFlags(const char *input, const char *output) const;
  int         splitLines(const char *line, parray *lines) const;
  const char *stripNewLines(const char *input) const;
  
private:
  int             debuggingFileSorter;
  const language *lang;

};

#endif 
// _fileSorter_h

