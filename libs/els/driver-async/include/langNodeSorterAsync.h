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

#ifndef _langNodeSorterAsync_h
#define _langNodeSorterAsync_h

#if defined(CANT_FWD_DECL_NESTED) && ! defined(_langNode_h)
#error "'langNode.h' must be #included before 'langNodeSorterAsync.h' on this platform."
#endif

#ifndef _langNode_h
#include <langNode.h>
#endif

#ifndef _commChannel_h
#include <commChannel.h>
#endif

#ifndef _commChannelAsync_h
#include <commChannelAsync.h>
#endif

#ifndef CANT_FWD_DECL_NESTED
class langNode::sorterAsync : public commChannel::Async
#else
class sorterAsync : public commChannel::Async
#endif
{
public:

  sorterAsync(commChannel::CHANNEL, langNode *);
  ~sorterAsync(void);

  virtual int handle(commChannel *);

private:

  int parse(char *buf, parray *files);

private:

  langNode  *owner;
  parray    *intermediates;
  char      *leftOver;
  char      *buffer;
};

#endif 
// _langNodeSorterAsync_h

