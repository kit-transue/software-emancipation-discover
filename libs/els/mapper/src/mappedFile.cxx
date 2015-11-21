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
#ifndef ISO_CPP_HEADERS
#include <iostream.h>
#else /* ISO_CPP_HEADERS */
#include <iostream>
using namespace std;
#endif /* ISO_CPP_HEADERS */
//--C++

//++OSPORT
#include <pdumem.h>
#include <pdustring.h>
#include <pdufile.h>
#include <pdupath.h>
//--OSPORT

//++ADS
#include <parray.h>
#include <raFile.h>
//--ADS

//++ELS
#include <elsMapper.h>
#include <mappedFile.h>
//--ELS

// static inits
const int  mappedFile::DEFAULT_NUMBER_OF_LINES  = 128;
const int  mappedFile::DEFAULT_SCAN_BUFFER_SIZE = 256 * 1024;
const int  mappedFile::DEFAULT_NUMBER_OF_FILES  = 32;
parray    *mappedFile::currentFiles             = 0;

mappedFile *mappedFile::getFile(char *file_name)
{
  static int  lastfile = 0;
  mappedFile *retval   = 0;
  mappedFile *tmpval   = 0;

  if ( file_name )
    {
      int n = currentFiles ? currentFiles->size() : 0;
      for ( int i = lastfile; i < n && retval == 0; i++ )
	{
	  tmpval = (mappedFile *)(*currentFiles)[i];
	  if ( tmpval->the_filename == file_name )
	    {
	      retval = tmpval;
	      lastfile = i;
	    }
	}
      for ( int i = 0; i < lastfile && retval == 0; i++ )
	{
	  tmpval = (mappedFile *)(*currentFiles)[i];
	  if ( tmpval->the_filename == file_name )
	    {
	      retval = tmpval;
	      lastfile = i;
	    }
	}
      if ( ! retval )
	{
	  retval = new mappedFile(file_name);
	  if ( retval )
	    {
	      if ( currentFiles )
		{
		  currentFiles->insert((void *)retval);
		  lastfile = n;
		}
	    }
	}
    }
  
  return retval;
}

mappedFile::mappedFile(void)
{
  the_filename = 0;
  lines        = 0;
  scanned      = 0;
  exists       = 0;
}

mappedFile::mappedFile(char *file_name)
{
  the_filename = file_name;
  lines        = new parray(DEFAULT_NUMBER_OF_LINES);
  scanned      = 0;
  if ( ! currentFiles )
    currentFiles = new parray(DEFAULT_NUMBER_OF_FILES);
  exists       = pdisfile(file_name);
}

mappedFile::~mappedFile(void)
{
  // remove myself from static list of all mapped files
  if ( currentFiles )
    {
      int i = currentFiles->find((void *)this);
      if ( i >= 0 && i < currentFiles->size() )
	currentFiles->remove(i);
      if ( currentFiles->size() == 0 )
	FREE_OBJECT(currentFiles);
    }
  FREE_OBJECT(lines);
}

int mappedFile::reset(void)
{
  int retval = 1;

 // destroy all mapped files
  if ( currentFiles )
    {
      for ( int i = currentFiles->size() - 1; i >= 0; i-- )
	{
	  mappedFile *f = (mappedFile *)(*currentFiles)[i];
	  if ( f )
	    FREE_OBJECT(f);
	}
      FREE_OBJECT(currentFiles);
    }

  return retval;
}

int mappedFile::scanFile(void)
{
  int retval = -1;
  
  if ( the_filename && ! scanned )
    {
      if ( lines )
	{
	  lines->insert((void *)0); // lines[0] = 0;

	  char buffer[DEFAULT_SCAN_BUFFER_SIZE];
	  raFile *file = new raFile(the_filename, 0);
	  if ( file )
	    {
	      if ( file->open(raFile_ACCESS_READ, 0) >= 0 )
		{
		  int byteOffset = 1; // byte offsets start at 1
		  int bytesRead;
		  int eol = 1;
		  while ( (bytesRead = file->read(buffer, DEFAULT_SCAN_BUFFER_SIZE)) > 0 )
		    {
		      for ( int i = 0; i < bytesRead; i++ )
			{
			  if ( eol == 1 )
			    {
			      lines->insert((void *)(byteOffset+i));
			    }
			  if ( buffer[i] == '\n' )
			    {
			      eol = 1;
			    }
			  else
			    {
			      eol = 0;
			    }
			}
		      byteOffset += bytesRead;
		    }
		  
		  // store the byte offset (file size in the last element)
		  lines->insert((void *)(byteOffset+1));

		  scanned = 1;
		  retval = 1;
		}
	      delete file;
	    }
	}
    }
  
  return retval;
}

int mappedFile::toBytePos(unsigned int row, unsigned int col, unsigned int &bp_out)
{
  int retval = -1;

  if ( row > 0 && col > 0 )
    {
      if ( fileExists() && ! fileScanned() )
	scanFile();

      if ( fileExists() && fileScanned() )
	if ( row < lines->size() )
	  if ( col < (unsigned int)((*lines)[row+1]) - 1 )
	    {
	      bp_out = (unsigned int)((*lines)[row]) + col - 1;
	      retval = 1;
	    }
    }

  if ( retval != 1 )
    bp_out = 0;

  return retval;
}

int mappedFile::toRowCol(unsigned int bp, unsigned int &row, unsigned int &col)
{
  int retval = -1;

  if ( bp > 0 )
    {
      if ( fileExists() && ! fileScanned() )
	scanFile();

      if ( fileExists() && fileScanned() )
	{
	  int low  = 1;
	  int high = lines->size() - 1; 
	  int done = 0;

	  while ( ! done )
	    {
	      int mid = (low + high) / 2;

	      unsigned int r1_bp = (unsigned int)(*lines)[mid];
	      unsigned int r2_bp = (unsigned int)(*lines)[mid+1];
	      
	      if ( bp >= r1_bp && bp < r2_bp )
		{
		  row    = mid;
		  col    = bp - r1_bp + 1;
		  done   = 1;
		  retval = 1;
		}
	      else if ( bp < r1_bp )
		{
		  high = mid - 1;
		  if ( high < low )
		    done = 1;
		}
	      else 
		{
		  low = mid + 1;
		  if ( low > high )
		    done = 1;
		}
	    }
	}
    }
  
  if ( retval != 1 )
    {
      row = 0;
      col = 0;
    }

  return retval;
}

int mappedFile::resolveRC(unsigned int r_in, unsigned int c_in, long int amount, unsigned int &r_out, unsigned int &c_out)
{
  int retval = -1;

  if ( r_in > 0 && c_in >= 0 )
    {
      if ( fileExists() && ! fileScanned() )
	scanFile();

      if ( fileExists() && fileScanned() )
	{
	  if ( c_in != 0 )
	    {
	      unsigned int tmpbp = 0;
	      if ( toBytePos(r_in, c_in, tmpbp) >= 0 )
		{
		  tmpbp += amount;
		  if ( toRowCol(tmpbp, r_out, c_out) >= 0 )
		    retval = 1;
		}
	    }
	  else
	    {
	      // trying to determine the last byte on a line
	      unsigned int tmpbp = 0;
	      if ( toBytePos(r_in+1, 1, tmpbp) >= 0 ) // determine the first byte of the following line
		{
		  tmpbp += amount;
		  if ( toRowCol(tmpbp-1, r_out, c_out) >= 0 ) // determine the R/C of the previous byte
		    retval = 1;
		}
	    }
	}
    }
  
  if ( retval != 1 )
    {
      r_out = 0;
      c_out = 0;
    }

  return retval;
}

unsigned int mappedFile::fileSize(void) 
{
  unsigned int retval = 0L;

  if ( fileExists() )
    {
      if ( ! fileScanned() )
	scanFile();
      if ( fileScanned() )
	retval = (unsigned int)(*lines)[lines->size()-1] - 2; 
      // -2 because the array contains the
      // what would be the byte offset of
      // the n+1st byte in the file if it existed.
      // and the indexing starts at 1, so -1 for 
      // 0 based, and -1 for the last byte.
    }
  
  return retval;
}

void mappedFile::dumpTo(ostream &o) const
{
  o << '"';
  o << filename();
  o << '"';
}
