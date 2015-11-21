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
/* This is a -*- C++ -*- header file */

#ifndef _elsMapper_h
#define _elsMapper_h

#ifdef __cplusplus

#include <iosfwd>

#include "mappingList.h"

//++ELS
struct transformation;
struct mapline;
struct location;
class  region;
class  position;
//--ELS

class elsMapper
{
public:
  virtual ~elsMapper(void);

protected:
  elsMapper(void);
  elsMapper(const char *filename);

public:
         void            dumpTo(ostream &) const;
  static const position *getFileBoundary(char *);
  static int             addTransformation(const mapline  *);
  static int             addTransformation(const transformation *, int);
  static int             getTrueLoc(const location *, location **);
  static int             reset(void);
  static void            dumpAllMappingInfo(ostream &);
  static void            showAllUnmappings(int = 1);
  static void            showBadLocs(int = 1);

private:
  static elsMapper  *findMapper(const char *);
  static int         getTrueLoc2(region *, location **);

private:
  const char    *forFile;      
  mappingList   *mappings;     
  static int     showbadlocs;
  static int     showunmappings;
    //  static parray *currentMaps;  
};

extern "C" void elsMapper_dumpAllMappingInfo(void);
extern "C" int  elsMapper_reset(void);

#else

/* C include file here */
void elsMapper_dumpAllMappingInfo();
int  elsMapper_reset();

#endif

#endif
