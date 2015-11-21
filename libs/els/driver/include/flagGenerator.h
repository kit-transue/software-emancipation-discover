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

#ifndef _flagGenerator_h
#define _flagGenerator_h

#ifndef _language_h
//++ELS
class language;
//--ELS
#endif

class flagGenerator
{
public:

  // Constructor(s) / Destructor(s)
  flagGenerator(const language *);
  virtual ~flagGenerator(void);

  // Public Interface
  const char *getStaticFlags(void) const;
  const char *getDynamicFlags(const char *filename) const;

protected:
  static const char * const filterInputFileMarker;
  static const char * const filterOutputFileMarker;

  static const char * const staticFlagsPrefKey;
  static const char * const staticFlagsPrefDefaultVal;

  static const char * const filterExecPrefKey;
  static const char * const filterExecPrefDefaultVal;

  static const char * const filterExecFlagsPrefKey;
  static const char * const filterExecFlagsPrefDefaultVal;

  

private:
  const language *lang;

private:
  const char *getFlagsFromPDF(const char *source) const;
  const char *filterFlags(const char *source) const;


};

#endif 
// _flagGenerator_h

