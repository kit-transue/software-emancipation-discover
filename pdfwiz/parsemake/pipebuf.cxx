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
// pipebuf.cxx
// 7.nov.97 kit transue


#include <streambuf>
#include <deque>
#include <cassert>
#include <iostream>

#include "pipebuf.h"

using namespace std;


template<typename charT, typename traits> buffer_pool<charT, basic_pipebuf<charT, traits>::BUFFERSIZE>
basic_pipebuf<charT, traits>::default_buffer_pool;

template<typename charT, int BUFFERSIZE>
buffer_pool<charT, BUFFERSIZE>::~buffer_pool()
{
	while (!free_stack.empty()) {
		delete[] free_stack.top();
		free_stack.pop();
	}
}


template<typename charT, int BUFFERSIZE> charT *
buffer_pool<charT, BUFFERSIZE>::get_buffer()
{
	charT * ret = 0;
	if (free_stack.empty()) {
		// allocate a new buffer
		ret = new charT[BUFFERSIZE];
		if (! ret) {
			cerr << "unable to allocate new pipebuf buffer" << endl;
			abort();
		}
	}
	else {
		ret = free_stack.top();
		free_stack.pop();
	}
	return ret;
}

template<typename charT, int BUFFERSIZE> void
buffer_pool<charT, BUFFERSIZE>::release_buffer(charT * released)
{
	free_stack.push(released);	
}


template<typename charT, typename traits> basic_pipebuf<charT, traits>::int_type
basic_pipebuf<charT, traits>::overflow(int_type c)
{
	buffer_queue.push_back(buffer_pool_mgr->get_buffer());
	setp(buffer_queue.back(), buffer_queue.back() + BUFFERSIZE);
	if (c != traits::eof()) {  // there was an overflow character to be dealt with
		*pptr() = c;
		pbump(1);
	}
	return !traits::eof();
}

template<typename charT, typename traits> basic_pipebuf<charT, traits>::int_type
basic_pipebuf<charT, traits>::underflow()
{
	if (gptr() != eback() + BUFFERSIZE) {  // get area wasn't full last time
		char_type * buffer = eback();
		// last time we tried, the put and get buffer was the same, but that may not still be true
		if (buffer_queue.size() > 1)  // there are more buffers to proceess, so this one must be fully available
			setg(eback(), gptr(), eback() + BUFFERSIZE );
		else   // they're the same, so the put pointer is limiting
			setg(eback(), gptr(), pptr());
	}
	else if (gptr() != pptr()) {
		// must switch buffers
		// add freed buffer to available list
		buffer_pool_mgr->release_buffer(eback());
		// remove old buffer
		buffer_queue.pop_front();
		assert(!buffer_queue.empty());  // overflow should have already allocated next buffer
		char_type * buffer = buffer_queue.front();
		if (buffer_queue.size() > 1)
			setg(buffer, buffer, buffer + BUFFERSIZE);
		else
			setg(buffer, buffer, pptr());
	}
	else {
		// end of buffer, but put hasn't overflowed to next buffer yet
	}
	// confirm that even after this action, we got somewhere:
	if (gptr() == pptr())
		return traits::eof();
	else
		return traits::to_int_type(*gptr());
}


template<typename charT, typename traits>
basic_pipebuf<charT, traits>::basic_pipebuf(buffer_pool<charT, BUFFERSIZE> * poolmgr) : buffer_pool_mgr(poolmgr)
{
	char_type * newbuffer = new char_type[BUFFERSIZE];
	char_type * nbend = newbuffer + BUFFERSIZE;
	buffer_queue.push_back(newbuffer);
	setg(newbuffer, newbuffer, newbuffer);  // nothing yet available for get
	setp(newbuffer, nbend);
}

template<typename charT, typename traits>
basic_pipebuf<charT, traits>::~basic_pipebuf()
{
	// free all the buffers:
	buffer_queue_type::iterator i = buffer_queue.begin();
	while (i != buffer_queue.end()) {
		buffer_pool_mgr->release_buffer(*i);
		++i;
	}
}

template<typename charT, typename traits> int
basic_pipebuf<charT, traits>::showmanyc()
{
	// rough cut: figure all available in the buffers in use:
	int howmany = BUFFERSIZE * buffer_queue.size();
	// adjust for characters already consumed:
	howmany -= gptr() - eback();
	// and for those not yet put:
	howmany -= epptr() - pptr();

	return howmany;
}

pipebuf instantiate_me;
