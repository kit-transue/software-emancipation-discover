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

#ifndef ISO_CPP_HEADERS
#include <stdio.h>
#else /* ISO_CPP_HEADERS */
#include <cstdio>
using namespace std;
#endif /* ISO_CPP_HEADERS */
#include <genString.h>

class ioErrorCheck {
  private:
    FILE *theStream;
    int last_error;
    genString thePath;

  public:
    ioErrorCheck(void);
    ioErrorCheck(FILE *stream);
    ~ioErrorCheck();

    void init(FILE *stream);
    FILE* file_open(const char* path, const char *type);
    int file_close(void);
    void file_write(const void *void_ptr, int size, int elems); 
    void file_put_string(const char *theString); 
    void put_char(int theChar);
    int get_last_error() {return last_error; };
    const char* get_path() { return thePath; };
    void reset(void) { last_error = 0; };
};

/*
  START-LOG-------------------------------------------
  
  $Log: ioErrorCheck.h  $
  Revision 1.2 2000/07/10 23:01:32EDT ktrans 
  mainline merge from Visual C++ 6/ISO (extensionless) standard header files
// Revision 1.1  1992/11/06  18:45:49  oak
// Initial revision
//
  
  END-LOG---------------------------------------------
*/
