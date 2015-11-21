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
// pipebuf.h
// 7.nov.97 kit transue

#if !defined(_PIPEBUF_H_)
#define _PIPEBUF_H_

/* maintains a ring of buffers that hold the contents between
	the get and put pointers.  Buffers are a fixed length, and
	are not freed until the streambuf is destroyed.  Once a buffer
	has been read, it is made available for storage of put
	data.  If there is an overflow (during put), and no free
	buffers are available, a new one is allocated and used.
	The use of a fixed buffer is currently not supported.
*/

#include <streambuf>
#include <deque>
#include <stack>

using namespace std;


// damn, if this doesn't look like a class-specific new/delete!
template<typename charT, int size>
class buffer_pool
{
public:
	// public types
	// construct/copy/destroy
	virtual ~buffer_pool();

	// access
	charT *get_buffer();
	void release_buffer(charT *);

private:
	stack<charT *> free_stack;
};
	

template<typename charT, typename traits = char_traits<charT> >
class basic_pipebuf : public basic_streambuf<charT, traits>
{
public:
	enum {BUFFERSIZE = 1024};  // class constant

	// construct/copy/destroy
	basic_pipebuf(buffer_pool<charT, BUFFERSIZE> * = & default_buffer_pool);
	virtual ~basic_pipebuf();

	// get area
	virtual int_type underflow();
	virtual int showmanyc();

	// put area
	virtual int_type overflow(int_type c);

#if defined(MICROSOFT_COMPILER_NOT_BROKEN)
private:
	enum {BUFFERSIZE = 1024};  // class constant
#else
private:
#endif
	typedef deque<char_type *> buffer_queue_type;
	buffer_queue_type buffer_queue;
	buffer_pool<charT, BUFFERSIZE> *buffer_pool_mgr;
	static buffer_pool<charT, BUFFERSIZE> default_buffer_pool;
};

typedef basic_pipebuf<char> pipebuf;
#endif // !defined(_PIPEBUF_H_)
