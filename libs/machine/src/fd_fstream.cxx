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
// fd_fstream.cxx
// provides ISO-compatible streams that provide visibility to the file descriptor
// 26.jun.2000 kit transue

#ifdef ISO_CPP_HEADERS
#include "fd_fstream.h"
#include "fd_streambuf.h"

void
fd_ostream::open(char const *name, ios_base::openmode mode)
{
	rdbuf()->open(name, mode);
}

fd_ostream::fd_ostream() :
	ostream(new fd_streambuf)
{
}

fd_ostream::fd_ostream(int filedes) :
	ostream(new fd_streambuf(filedes))
{
}

int
fd_ostream::fd() const
{
	return rdbuf()->fd();
}

void
fd_ostream::close()
{
	rdbuf()->close();
}

fd_streambuf *
fd_ostream::rdbuf() const
{
	return static_cast < fd_streambuf* > (ostream::rdbuf());
}
#endif /* ISO_CPP_HEADERS */
