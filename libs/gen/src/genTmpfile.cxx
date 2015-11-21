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
// genTmpfile.h.C
//------------------------------------------
// synopsis:
// 
// 
//------------------------------------------

// INCLUDE FILES
#include <cLibraryFunctions.h>
#include <machdep.h>

#include <genError.h>
#include <genTmpfile.h>


// EXTERNAL VARIABLES

// EXTERNAL FUNCTIONS

// PRE-PROCESSOR DEFINITIONS

// VARIABLE DEFINITIONS


// FUNCTION DEFINITIONS

genTmpfile::genTmpfile(const char* prefix)
: filename(OSapi_tempnam(NULL, (char*)prefix)),
  filedesc(-1),
  fileptr(NULL),
  created(0)
{
    Initialize(genTmpfile::genTmpfile);
}


genTmpfile::~genTmpfile()
{
    Initialize(genTmpfile::~genTmpfile);

    close();
    unlink();
    if(filename)
	OSapi_free(filename);
}


int genTmpfile::open()
{
    Initialize(genTmpfile::open);

    if(filedesc < 0  &&  filename)
    {
		filedesc = OSapi_open(filename, O_RDWR|O_CREAT|O_TRUNC, 0666);
		created = (filedesc >= 0);
    }

    return filedesc;
}

FILE* genTmpfile::open_for_read()
// This file is not open by genTmpfile::open() or genTmpfile::open_buffered()
// It get the name from genTmpfile::.name() and created by other means.
// close() and unlink() could be called.
{
    Initialize(genTmpfile::open_for_read);

    fileptr = OSapi_fopen(filename, "r");
    created = 1;
    return fileptr;
}

FILE* genTmpfile::open_buffered()
{
    Initialize(genTmpfile::open_buffered);

    if(open() >= 0)
			fileptr = fdopen(filedesc, "r+");

    return fileptr;
}


inline void genTmpfile::reset()
{
    filedesc = -1;
    fileptr = NULL;
}


void genTmpfile::close()
{
    Initialize(genTmpfile::close);

    if(fileptr)
	fclose(fileptr);
    else if(filedesc >= 0)
	OSapi_close(filedesc);
    reset();
}


void genTmpfile::release()
{
    Initialize(genTmpfile::release);

    created = 0;
    reset();
}


void genTmpfile::unlink()
// delete this temporary file.
{
    Initialize(genTmpfile::unlink);

//  if(created)
		OSapi_unlink(filename);
    release();
}

