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
/*_______________________________________________________________________
!
! ioErrorCheck.h.C
!
! Synopsis:	Builds an error checking wrapper around file 
!		write routines.  If an error does occur all
!		subsequent writes are disabled.  This allows 
!               for clean error checking within a large
!		routine that writes to a file many times.
!
_______________________________________________________________________*/


// INCLUDE FILES

//#include <stdio.h>
//#include <errno.h>
#ifdef _WIN32
#include <errno.h>
#endif
#ifdef hp10
#include <errno.h>
#endif
#include <cLibraryFunctions.h>
#include <genError.h>
#include <genString.h>
#include <ioErrorCheck.h>

/*_______________________________________________________________________
!
! Member:	ioErrorCheck::ioErrorCheck      
!
! Arguments:	FILE *stream
!       
! Returns:	int
!
! Synopsis:	initializes variables
!
_______________________________________________________________________*/ 
ioErrorCheck::ioErrorCheck(void) { 
    Initialize(ioErrorCheck::ioErrorCheck);
    init(NULL);
    Return
};
ioErrorCheck::ioErrorCheck(FILE *stream = NULL) { 
    Initialize(ioErrorCheck::ioErrorCheck);
    init(stream);
    Return
};

/*_______________________________________________________________________
!
! Member:	ioErrorCheck::init      
!
! Arguments:	FILE *stream
!       
! Returns:	int
!
! Synopsis:	initializes variables
!
_______________________________________________________________________*/ 
void ioErrorCheck::init(FILE *stream = NULL) { 
    Initialize(ioErrorCheck::init);
    theStream = stream;
    last_error = 0; 
    thePath = "\0";
    Return
};

/*_______________________________________________________________________
!
! Member:	ioErrorCheck::ioErrorCheck      
!
! Arguments:	int
!       
! Returns:	int
!
! Synopsis:	Here for easy error checking.
!
_______________________________________________________________________*/
ioErrorCheck::~ioErrorCheck() {
};


/*_______________________________________________________________________
!
! Member:	ioErrorCheck::file_open
!
! Arguments:	const char *path	path and file name
!		const char *type	read write permissions
!       
! Returns:	FILE*
!
! Synopsis:	Opens the file and gets error value if any.
!
_______________________________________________________________________*/
FILE* ioErrorCheck::file_open(const char *path, const char *type){
    Initialize(ioErrorCheck::file_open);
    thePath = path;
    if (last_error == 0) {
	theStream = OSapi_fopen(path, type);
	if (theStream == NULL)
	    last_error = errno;
	ReturnValue(theStream);
    }
    ReturnValue(NULL);
}

/*_______________________________________________________________________
!
! Member:	ioErrorCheck::file_close
!
! Arguments:	void
!       
! Returns:	int
!
! Synopsis:	Closes the file and checks if an error occured.
!
_______________________________________________________________________*/
int ioErrorCheck::file_close(void) {
    Initialize(ioErrorCheck::file_close);
    int result = -1;
    if (last_error == 0) {
	result = OSapi_fclose(theStream);
	if (result == -1)
	    last_error = errno;
    }
    ReturnValue(result);
}

/*_______________________________________________________________________
!
! Member:	ioErrorCheck::file_write
!
! Arguments:	const void *void_ptr
!		int size
!		int elements
!       
! Returns:	void
!
! Synopsis:	Wrapper around the fwrite command.
!
_______________________________________________________________________*/
void ioErrorCheck::file_write(const void *void_ptr, int size, int elems) {
    Initialize(ioErrorCheck::file_write);
    if (last_error == 0) {
	   if (OSapi_fwrite(void_ptr, size, elems, theStream) != elems)
	       last_error = errno;
    }
    Return
}


/*_______________________________________________________________________
!
! Member:	ioErrorCheck::file_put_string
!
! Arguments:	const char *theString
!       
! Returns:	void
!
! Synopsis:	Wrapper around the fputs command.
!
_______________________________________________________________________*/
void ioErrorCheck::file_put_string(const char *theString) {
    Initialize(ioErrorCheck::file_put_string);
    if (last_error == 0) {
	if (OSapi_fputs(theString, theStream) == EOF)
	    last_error = errno;
    }
    Return
}


/*_______________________________________________________________________
!
! Member:	ioErrorCheck::put_char
!
! Arguments:	int theChar
!       
! Returns:	void
!
! Synopsis:	Wrapper around the putc command.
!
_______________________________________________________________________*/
void ioErrorCheck::put_char(int theChar) {
    Initialize(ioErrorCheck::put_char);
    if (last_error == 0) {
	if (OSapi_putc(theChar, theStream) == EOF) {
	    last_error = errno;
	    if (last_error == 0)
		last_error = EIO; // input/output error.
	}
    }
    Return
}


/*
  START-LOG-------------------------------------------
  
  $Log: ioErrorCheck.h.cxx  $
  Revision 1.6 1999/09/16 15:46:35EDT sschmidt 
  HP 10 port
// Revision 1.1  1992/11/06  18:44:37  oak
// Initial revision
//
  
  END-LOG---------------------------------------------
*/


