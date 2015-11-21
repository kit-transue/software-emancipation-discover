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
/* fd_fstream.h */

/*	Provides stream buffer and iostream that is associated with a file
	descriptor instead of explicitly opening a file by name.
	Similar functionality was available with pre-ISO C++ iostreams; this
	is intended to replicate that functionality.

	It is particularly useful when creating a stream based on cin or cout.
*/

/* 21.June.2000 Kit Transue */


#ifndef ISO_CPP_HEADERS

#include <fstream.h>
class fd_ostream : public ofstream
{
};

#else /* ISO_CPP_HEADERS */

#include <fstream>
using namespace std;
#include "fd_streambuf.h"

class fd_ostream : public std::ostream
{
public:
	fd_ostream();	/* create ostream with no associated file */
	fd_ostream(int fd);	/* associate stream with file descriptor */

	void close();	/* close the file */
	void open(char const *filename, ios_base::openmode mode = ios_base::out);
	int fd() const;	/* file desriptor associated with stream */
private:
	fd_streambuf *rdbuf() const;
};
#endif /* ISO_CPP_HEADERS */
