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
#ifndef _genTmpfile_h
#define _genTmpfile_h

// genTmpfile.h
//------------------------------------------
// synopsis:
// 
// 
//------------------------------------------

#ifndef FILE
#ifndef ISO_CPP_HEADERS
#include <stdio.h>
#else /* ISO_CPP_HEADERS */
#include <cstdio>
using namespace std;
#endif /* ISO_CPP_HEADERS */
#endif


class genTmpfile
{
  public:
    genTmpfile(const char* prefix = 0);
    ~genTmpfile();

    const char* name();

    int   open();
    FILE* open_buffered();
    FILE* open_for_read();
    void  close();

    void  release();
    void  unlink();

    operator int();
    
#ifdef cplusplus_2_1    
    operator _iobuf*();		// operator FILE*()
#else
      operator FILE*();
#endif    

  private:
    char* filename;
    int   filedesc;
    FILE* fileptr;
    int   created;

    void  reset();
};


inline const char* genTmpfile::name()
{
    return filename;
}


inline genTmpfile::operator int()
{
    return filedesc;
}


#ifdef cplusplus_2_1
inline genTmpfile::operator _iobuf*()
#else
inline genTmpfile::operator FILE*()
#endif
{
    return fileptr;
}


/*
   START-LOG-------------------------------------------

   $Log: genTmpfile.h  $
   Revision 1.3 2000/07/10 23:04:15EDT ktrans 
   mainline merge from Visual C++ 6/ISO (extensionless) standard header files
// Revision 1.5  1993/09/23  13:29:05  kws
// Bug track: Port
// OS_dependent changes for port
//
// Revision 1.4  1993/07/22  21:16:34  bakshi
// conditional compilation to work around brain-dead cfront 2.1
//
// Revision 1.3  1993/07/22  20:26:54  bakshi
// HP/Solaris Port
//
// Revision 1.2  1993/05/26  19:25:56  glenn
// Add operator int(), operator _iobuf*().
// Make name() inline.
//
// Revision 1.1  1993/05/26  16:45:05  glenn
// Initial revision
//
   END-LOG---------------------------------------------
*/

#endif // _genTmpfile_h
