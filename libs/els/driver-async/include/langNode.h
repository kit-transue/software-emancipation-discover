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

#ifndef _langNode_h
#define _langNode_h

class jobNode;
class commFDPipe;
class parray;

#ifndef _language_h
#include <language.h>
#endif

#ifndef _iffProducer_h
#include <iffProducer.h>
#endif

#ifndef _commChannel_h
#include <commChannel.h>
#endif

#ifndef _commChannelAsync_h
#include <commChannelAsync.h>
#endif

class langNode : public language, public iffProducer
{
public:
  
  // Constructor(s) / Destructor(s)
  langNode(jobNode *parent, const char *forFile);
  virtual ~langNode(void);
  
protected:
  
  virtual int  produceIff(void);

private:
  
#ifndef CANT_FWD_DECL_NESTED
  class gmakeAsync;
#else
# include <langNodeGmakeAsync.h>
#endif
  
#ifndef CANT_FWD_DECL_NESTED
  class sorterAsync;
#else
# include <langNodeSorterAsync.h>
#endif

#ifndef CANT_FWD_DECL_NESTED
  class filterAsync;
#else
# include <langNodeFilterAsync.h>
#endif
  
  friend class gmakeAsync;
  friend class sorterAsync;
  friend class filterAsync;
  
  int stdiomap[STD_IO_LIST_SIZE];
  
  int filterDoneFlag;
  
  parray      *gmakeCommand;
  parray      *sorterCommand;
  parray      *filterCommand;

  commFDPipe  *gmakePipe;
  commFDPipe  *sorterPipe;
  commFDPipe  *filterPipe;

  gmakeAsync  *gmakeReader;
  sorterAsync *sorterReader;
  filterAsync *filterReader;

  const char  *sourceFile;
  const char  *ifFile;
  char        *makefile;
  const char  *compilerFlags;

  int    damaged;
  int    doneFlag;
  char  *newFlags;
  int    gmakeFinished;

private:
  
  const char  *getDynamicFlags(const char *filename);
  const char  *resolveFilename(const char *original) const;
  int          generateMakefile(const char *sourcefile, char *makefile);
  parray      *getFilterCommand(void) const;
  parray      *getGmakeCommand(const char *makefile) const;
  parray      *getSorterCommand(void) const;
  void         dispose(void);
  const char  *getFlagsFromPDF(const char *filename) const;
  const char  *filterFlags(const char *flags);

};

#endif 
// _langNode_h

