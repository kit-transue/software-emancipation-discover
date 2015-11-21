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
// fd_streambuf.h
// 25.jun.2000 kit transue

#if !defined(_FD_STREAMBUF_H_)
#define _FD_STREAMBUF_H_

/* 
	Provides stream buffers that write to a file.  Differ from ISO ofstreams
	in that instead of requiring a filename to create, these may be
	connected to an open file descriptor.

	They may also return the file descriptor they are writing to.

	Seek operations are not supported.
*/

#include <iostream>
#include <streambuf>

using namespace std;


template<typename charT, typename traits = char_traits<charT> >
class basic_fd_streambuf : public basic_streambuf<charT, traits>
{
public:
	typedef basic_streambuf<charT,traits> BASE;
	typedef typename BASE::int_type int_type;
	typedef typename BASE::char_type char_type;
	using BASE::epptr;
	using BASE::pptr;
	using BASE::pbase;
	using BASE::pbump;

	// construct/copy/destroy
	basic_fd_streambuf();
	basic_fd_streambuf(int filedes);
	virtual ~basic_fd_streambuf();
	
	// file operations
	basic_fd_streambuf<charT,traits> *open(char const * filename, ios_base::openmode mode );
	void close();
	int fd() const;
	virtual int sync();

	// get area (unimplemented)
	virtual int_type underflow();
	// virtual int showmanyc();

	// put area
	virtual int_type overflow(int_type c);

private:
	void allocate_buffer();
	void free_buffer();
	int file;
	short mode;
	bool fd_is_mine;
	bool buffer_is_mine;
	enum {BUFFERSIZE = 1024};  // class constant
};

typedef basic_fd_streambuf<char> fd_streambuf;

#endif // !defined(_FD_STREAMBUF_H_)
