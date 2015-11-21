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
//++OSPORT
#include <pdufile.h>
#include <pdumem.h>
//--OSPORT

//++ELS
#include <iffProducer.h>
#include <language.h>
#include <langIffProducer.h>
#include <gmaker.h>
//--ELS


langIffProducer::langIffProducer(void) : iffProducer()
{
}

langIffProducer::~langIffProducer(void)
{
}

int langIffProducer::produceIff(const char *sourceFile, const char **ifFile) 
{
  int retval = -1;

  if ( sourceFile && ifFile )
    {
      *ifFile = pdtempname(0);
      if ( *ifFile )
	{
	  retval = produceIff(sourceFile, *ifFile);
	}
    }

  return retval;
}

int langIffProducer::produceIff(const char *sourceFile, const char *ifFile) 
{
  int retval = -1;

  if ( sourceFile )

    {
      language *lang = 0;

      if ( (lang = new language(sourceFile)) != 0 )
	{
	  retval = produceIff(sourceFile, lang, ifFile);
	}

      FREE_OBJECT(lang);
    }

  return retval;
}

int langIffProducer::produceIff(const char *sourceFile, const language *lang, const char **ifFile) 
{
  int retval = -1;

  if ( sourceFile && lang && ifFile )
    {
      *ifFile = pdtempname(0);
      if ( *ifFile )
	{
	  retval = produceIff(sourceFile, lang, *ifFile);
	}
    }

  return retval;
}

int langIffProducer::produceIff(const char *sourceFile, const language *lang, const char *ifFile) 
{
  int retval = -1;

  if ( sourceFile && lang )
    {
      gmaker *gmakeObj = new gmaker(lang);
      if ( gmakeObj )
	{
	  retval = gmakeObj->execute(sourceFile, ifFile);
	  FREE_OBJECT(gmakeObj);
	}
      
    }

  return retval;
}


