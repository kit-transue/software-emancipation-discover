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

#ifndef _qualityItem_h
#define _qualityItem_h

#ifndef _reportItem_h
#include <reportItem.h>
#endif

class qualityItem : public reportItem
{
public:
  qualityItem(char const *, char const *, char const *, char const *, char const *, char const *, char const *);
  virtual ~qualityItem(void);

public:
  inline char const *score(void);
  inline char const *weight(void);
  inline char const *found(void);
  inline char const *outof(void);
  inline char const *units(void);

public:
  virtual int onTag(int);

private:
  char const *__score;
  char const *__weight;
  char const *__found;
  char const *__outof;
  char const *__units;
};

char const *qualityItem::score(void)
{
  return __score;
}

char const *qualityItem::weight(void)
{
  return __weight;
}

char const *qualityItem::found(void)
{
  return __found;
}

char const *qualityItem::outof(void)
{
  return __outof;
}

char const *qualityItem::units(void)
{
  return __units;
}

#endif 
// _qualityItem_h

