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
//++C++
#include <fstream.h>
//--C++

//++OSPORT
#include <pdustring.h>
#include <pdufile.h>
#include <pdumem.h>
#include <pdupath.h>
#include <pdutime.h>
//--OSPORT

//++ADS
#include <parray.h>
//--ADS

//++IPC
#include <commChannel.h>
#include <commFDPipe3.h>
#include <commFDPipe.h>
//--IPC

//++ELS
#include <jobNode.h>
#include <iffProducer.h>
#include <language.h>
#include <langNode.h>
#include <langNodeGmakeAsync.h>
#include <langNodeSorterAsync.h>
#include <langNodeFilterAsync.h>
//--ELS

langNode::langNode(jobNode *parentNode, const char *forFile) : iffProducer(parentNode), language(forFile)
{
  damaged       = 1;

  // default binding for children
  stdiomap[STD_INPUT]  = commChannel::BIND_CHILD;
  stdiomap[STD_OUTPUT] = commChannel::BIND_CHILD;
  stdiomap[STD_ERROR]  = commChannel::BIND_CHILD;

  sourceFile    = resolveFilename(forFile);
  ifFile        = pdtempname(0);
  makefile      = pdtempname(0);

  gmakeCommand  = getGmakeCommand(makefile);
  gmakePipe     = new commFDPipe(gmakeCommand);
  gmakeReader   = new gmakeAsync(commChannel::CHANNEL_READ, this);

  sorterCommand = getSorterCommand();
  sorterPipe    = new commFDPipe(sorterCommand);
  sorterReader  = new sorterAsync(commChannel::CHANNEL_READ, this);

  filterCommand = getFilterCommand();
  filterPipe    = new commFDPipe(filterCommand);
  filterReader  = 0;  // cannot allocate yet

  gmakeFinished = 0;
  doneFlag      = 0;
  newFlags      = new char [ 4096 ];

  if (sourceFile && ifFile && makefile && gmakeCommand && gmakePipe && gmakeReader 
      && sorterCommand && sorterPipe && sorterReader && filterCommand && filterPipe && newFlags )
    {
      if ( generateMakefile(sourceFile, makefile) >= 0 )
	{
	  damaged = 0;
	}
    }

  if ( damaged )
    dispose();
}

langNode::~langNode(void)
{
  dispose();
}

void langNode::dispose(void)
{
  // pdrmfile(ifFile);
  // pdrmfile(makefile);

  FREE_MEMORY(sourceFile);
  FREE_MEMORY(ifFile);
  FREE_MEMORY(makefile);

  pdstring_delete(gmakeCommand);
  FREE_OBJECT(gmakeCommand);
  pdstring_delete(sorterCommand);
  FREE_OBJECT(sorterCommand);
  pdstring_delete(filterCommand);
  FREE_OBJECT(filterCommand);

  FREE_OBJECT(gmakePipe);
  FREE_OBJECT(sorterPipe);
  FREE_OBJECT(filterPipe);
  FREE_OBJECT(gmakeReader);
  FREE_OBJECT(sorterReader);
  FREE_OBJECT(filterReader);

  damaged = 1;
}  

int langNode::produceIff(void)
{
  int retval = -1;

  if ( ! damaged )
    {
      if ( sorterPipe->open(stdiomap, STD_IO_LIST_SIZE) >= 0 )
	if ( sorterPipe->async(sorterReader) >= 0 )
	  if ( gmakePipe->open(stdiomap, STD_IO_LIST_SIZE) >= 0 )
	    if ( gmakePipe->async(gmakeReader) >= 0 )
	      retval = 1;
      
      if ( retval != 1 )
	dispose();
    }
  
  return retval;
}

const char *langNode::resolveFilename(const char *filename) const
{
  const char *retval = 0;
  const char  sep[]  = { PATH_FILE_SEP_CH, '\0' };

  char       temp[4096];

  if ( filename )
    {
      if ( ! pdabspath(filename) )
	{
	  if ( pdgetcwd(temp, 4096) != 0 )
	    if ( pdstrcat(temp, sep) != 0 )
	      if ( pdstrcat(temp, filename) != 0 )
		retval = pdrealpath(temp);
	}
      else
	{
	  retval = pdrealpath(filename);
	}
    }

  return retval;
}

int langNode::generateMakefile(const char *filename, char *makefileName)
{
  int retval = -1;

  if ( ldfIndex >= 0 )
    {
      const char *parserPref   = ((ldfInfo *)(*cachedLDFs)[ldfIndex])->getPref(ldfInfo::parserPref);
      const char *parser       = pdexecpath(parserPref);
      const char *statFlags    = ((ldfInfo *)(*cachedLDFs)[ldfIndex])->getPref(ldfInfo::parserFlagsPref);
      const char *dynFlags     = getDynamicFlags(filename);
      const char *source       = filename;
      const char *iff          = ifFile;
      const char *stub         = ((ldfInfo *)(*cachedLDFs)[ldfIndex])->getPref(ldfInfo::makefileStubPref);
      const char *stubContents = pdloadfile(stub);
      
      if ( parser && stub && statFlags && dynFlags && source && iff && stub && stubContents )
	{
	  ofstream Makefile(makefileName);
	  if ( Makefile )
	    {
	      Makefile << "#\n";
	      Makefile << "# This makefile generated automatically by DISCOVER\n";
	      Makefile << "# Do not edit this file!\n";
	      Makefile << "#\n\n";
	      Makefile << "PARSER=" << parser << '\n';
	      Makefile << "STATIC_FLAGS=" << statFlags << '\n';
	      Makefile << "DYNAMIC_FLAGS=" << dynFlags << '\n';
	      Makefile << "SOURCE_FILE=" << source << '\n';
	      Makefile << "IF_FILE=" << iff << '\n';
	      Makefile << "\n\n";
	      Makefile << stubContents << '\n';
	      Makefile.close();
	      retval = 1;
	    }
	}
      
      FREE_MEMORY(dynFlags);
      FREE_MEMORY(stubContents);
      FREE_MEMORY(parser);
      FREE_MEMORY(parserPref);
      FREE_MEMORY(statFlags);
      FREE_MEMORY(stub);
    }

  return retval;
}

const char *langNode::getDynamicFlags(const char *file)
{
  const char *retval = 0;

  const char *flags = 0;
  if ( (flags = getFlagsFromPDF(file)) !=0 )
    {
      retval = filterFlags(flags);
    }

  FREE_MEMORY(flags);
  return retval;
}

const char *langNode::getFlagsFromPDF(const char *filename) const
{
  // need to figure out how to do this for real!!!!
  return pdstrdup("CC -g -O2 --all-warnings -I. -I../.. -DTHIS_IS_A_TEST -DFLAG=1234\n");
}

const char *langNode::filterFlags(const char *flags)
{
  char *retval = 0;

  if ( flags )
    {
      if ( (filterReader = new filterAsync(commChannel::CHANNEL_READ, &doneFlag, &newFlags)) != 0 )
	if ( filterPipe->open(stdiomap, STD_IO_LIST_SIZE) >= 0 )
	  if ( filterPipe->async(filterReader) >= 0 )
	    {
	      int error      = 0;
	      int amtWritten = 0;
	      int length     = pdstrlen(flags);
	      for ( int i = 0; i < length && !error; i += amtWritten )
		{
		  int amtToWrite = (length-i) > commFDPipe3::PACKET_SIZE ? commFDPipe3::PACKET_SIZE : (length-i);
		  if ( (amtWritten = filterPipe->write(&flags[i], amtToWrite)) < 0 )
		    error = 1;
		}

	      // signal end of input
	      filterPipe->close(commChannel::CHANNEL_WRITE);
	      
	      while ( ! doneFlag )
		pdsleep(1);  // wait for all input
	      
	      // close the whole pipe
	      filterPipe->close(commChannel::CHANNEL_NULL);

	      retval = newFlags;
	    }

      FREE_OBJECT(filterReader);
    }

  return retval;
}

parray *langNode::getFilterCommand(void) const
{
  parray *retval = 0;

  if ( ldfIndex >= 0 )
    {
      int error = 1;

      const char *cmd = ((ldfInfo *)(*cachedLDFs)[ldfIndex])->getPref(ldfInfo::flagTranslationPref);
      const char *fullCmd = pdexecpath(cmd);
      FREE_MEMORY(cmd);

      if ( fullCmd )
	if ( (retval = new parray(2)) != 0 )
	  if ( retval->insert((void *)fullCmd) >= 0 )
	    error = 0;
      
      if ( error )
	{
	  FREE_MEMORY(fullCmd);
	  FREE_OBJECT(retval);
	}
    }

  return retval;
}

parray *langNode::getSorterCommand(void) const
{
  parray *retval = 0;

  if ( ldfIndex >= 0 )
    {
      int error = 1;
      const char *cmd = ((ldfInfo *)(*cachedLDFs)[ldfIndex])->getPref(ldfInfo::sorterPref);
      const char *fullCmd = pdexecpath(cmd);
      FREE_MEMORY(cmd);

      if ( fullCmd )
	if ( (retval = new parray(2)) != 0 )
	  if ( retval->insert((void *)fullCmd) >= 0 )
	    error = 0;
      
      if ( error )
	{
	  FREE_MEMORY(fullCmd);
	  FREE_OBJECT(retval);
	}
    }
  
  return retval;
}

parray *langNode::getGmakeCommand(const char *mkfile) const
{
  parray *retval = 0;

  if ( ldfIndex >= 0 )
    {
      if ( mkfile )
	{
	  int   error = 1;
	  char *temp  = 0;
	  
	  const char *cmd = ((ldfInfo *)(*cachedLDFs)[ldfIndex])->getPref(ldfInfo::gmakePref);
	  const char *fullCmd = pdexecpath(cmd);
	  FREE_MEMORY(cmd);

	  if ( fullCmd )
	    if ( (retval = new parray(3)) != 0 )
	      if ( retval->insert((void *)fullCmd) >= 0 )
		if ( (temp = pdstrdup("-f")) != 0 )
		  if ( retval->insert((void *)temp) >= 0 )
		    if ( (temp = pdstrdup(mkfile)) != 0 )
		      if ( retval->insert((void *)temp) >= 0 )
			error = 0;
	  
	  if ( error )
	    {
	      FREE_MEMORY(temp);
	      FREE_MEMORY(fullCmd);
	      FREE_OBJECT(retval);
	    }
	}
    }
  
  return retval;
}
