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

#ifndef _gmaker_h
#define _gmaker_h

#ifndef _language_h
//++ELS
class language;
//--ELS
#endif

class gmaker
{
public:

  // Constructor(s) / Destructor(s)
  gmaker(const language *forLanguage);
  ~gmaker(void);

  // Public Interface
  int execute(const char *source, const char *iffile) const;

protected:
  static const char * const debugGmakerPrefKey;
  static const char * const gmakeExecPrefKey;
  static const char * const parserExecPrefKey;
  static const char * const makefileStubPrefKey;
  static const char * const gmakeFlagsPrefKey;

  static const char * const gmakeExecPrefDefaultVal;
  static const char * const parserExecPrefDefaultVal;
  static const char * const makefileStubPrefDefaultVal;
  static const char * const gmakeFlagsPrefDefaultVal;

  static const char * const makefileMarker;

private:
  int              debuggingGmaker;
  const language  *lang;

private:
  int   buildMakeFile(const char *source, const char *iffile, const char **makefile, const char **intermediatesFilename) const;
  int   buildCommandLine(const char *makefile, char **commandline) const;
  int   execute(const char *commandline) const;
  int   buildIntermediates(const char *intermediatesFilename, const char *iffile) const;
  int   catFiles(const char *file1, const char *file2) const;
  char *resolveFilename(const char *filename) const;
};

#endif 
// _gmaker_h
