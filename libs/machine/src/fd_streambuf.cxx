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
// fd_streambuf.cxx
// 25.jun.2000 kit transue

#ifdef ISO_CPP_HEADERS
/* entire file is ifdef'd; otherwise compatible with ofstream */

#include <streambuf>
using namespace std;
#include <iostream>
#include <cassert>
#include <cstdio>

#include "cLibraryFunctions.h"
#include "machdep.h"
#include "fd_streambuf.h"

const int FD_FAILURE = -1;


template<typename charT, typename traits>
basic_fd_streambuf<charT, traits>::basic_fd_streambuf() :
	file(FD_FAILURE),
	fd_is_mine(false),
	buffer_is_mine(false)
{
	allocate_buffer();
}

template<typename charT, typename traits>
basic_fd_streambuf<charT, traits>::basic_fd_streambuf(int filedes) :
	file(filedes),
	fd_is_mine(false),
	buffer_is_mine(false)
{
	allocate_buffer();
}


template<typename charT, typename traits>
basic_fd_streambuf<charT, traits>::~basic_fd_streambuf()
{
	close();
	if (buffer_is_mine) {
		free_buffer();
	}
}

template<typename charT, typename traits> void
basic_fd_streambuf<charT, traits>::allocate_buffer()
{
	char_type *buf = new char_type[BUFFERSIZE];
	if (buf) {
		setp(buf, buf + BUFFERSIZE);
		buffer_is_mine = true;
	}
}

template<typename charT, typename traits> void
basic_fd_streambuf<charT, traits>::free_buffer()
{
	assert(buffer_is_mine);
	delete[] pbase();
	buffer_is_mine = false;
}


//////////////////
// file operations
//////////////////

template<typename charT, typename traits> basic_fd_streambuf<charT, traits> *
basic_fd_streambuf<charT, traits>::open(char const * filename, ios_base::openmode io_mode)
{
	basic_fd_streambuf<charT, traits> *ret = 0;
	if (file == FD_FAILURE) {
		int open_mode = 0;
		if (io_mode & (ios::out|ios::app)) {
			if (io_mode & ios::in) {
				open_mode |= O_RDWR;
			} else {
				open_mode |= O_WRONLY;
			}
		} else if (io_mode & ios::in) {
			open_mode |= O_RDONLY;
		}

		if (open_mode) {
			if (io_mode & ios::trunc
			    || (io_mode & ios::out && !(io_mode & ios::in|ios::ate|ios::app))) {
			    	open_mode |= O_TRUNC;
			}

			file = OSapi_open(filename, open_mode);
		}
		if (file != FD_FAILURE) {
			// successfully opened file
			fd_is_mine = true;
			ret = this;
		}
	}

	return ret;
}


template<typename charT, typename traits> void
basic_fd_streambuf<charT, traits>::close()
{
	if (fd_is_mine && file != FD_FAILURE) {
		OSapi_close(file);
		fd_is_mine = false;
		file = FD_FAILURE;
	}
}


template<typename charT, typename traits> typename basic_fd_streambuf<charT, traits>::int_type
basic_fd_streambuf<charT, traits>::overflow(int_type c)
{
	int_type ret = traits::eof();
	int n = (pptr() - pbase()) * sizeof(char_type);
	if (OSapi_write(file, pbase(), n) == n) {
		ret = 0;
		setp(pbase(), epptr());
		if (c != traits::eof()) {  // there was an overflow character to be dealt with
			*pptr() = c;
			pbump(1);
		}
	}
	return ret;
}

template<typename charT, typename traits> int
basic_fd_streambuf<charT, traits>::sync()
{
	overflow(traits::eof());
	// underflow....
	return 0;
}

template<typename charT, typename traits> int
basic_fd_streambuf<charT, traits>::fd() const
{
	return file;
}

template<typename charT, typename traits> typename basic_fd_streambuf<charT, traits>::int_type
basic_fd_streambuf<charT, traits>::underflow()
{
	assert(0);  // input streams not yet supported
	return traits::eof();
}


// Instantiate a char-based fd_streambuf:
template class basic_fd_streambuf<char>;
#endif /* ISO_CPP_HEADERS */
