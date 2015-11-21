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
#include <string.h>
#include "mbmsgser.h"
#include "transport.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "minidom.h"

using namespace nTransport;
using namespace MiniXML;

namespace MBDriver {

// ===========================================================================
//
MessageSerializer::MessageSerializer()
  : count(0),
    dataSize(0),
    valp(NULL),
    datap(NULL)
{
}

// ===========================================================================
//
void MessageSerializer::reset()
{
    count = 0;
    dataSize = 0;
    delete valp;
    delete [] datap;
    valp = NULL;
    datap = NULL;
}

static const int numSizeBytes = 4;

// ===========================================================================
//
Message *MessageSerializer::add(const char *chars, size_t num)
{
    for (size_t n = 0; n < num; n += 1) {
	if (count < numSizeBytes) {
            dataSize <<= 8;
            dataSize |= (unsigned char)(chars[n]);
	}
	else if (count < dataSize + numSizeBytes) {
	    datap[count - numSizeBytes] = chars[n];
	}
	count += 1;
        if (count == numSizeBytes) {
	    if (dataSize < 0 || 1000000 < dataSize) {
		fprintf(stderr, "In MessageSerializer::add, dataSize is %d.\n", (int)dataSize);
	    }
	    datap = new char[dataSize];
	}
        if (count == dataSize + numSizeBytes) {
	    valp = Message::Decode(datap, dataSize);
	}
    }
    return count >= numSizeBytes && count == dataSize + numSizeBytes ? valp : NULL;
}

// ===========================================================================
//
size_t MessageSerializer::require()
{
    size_t r = count < numSizeBytes ? numSizeBytes - count
             : count < dataSize + numSizeBytes ? dataSize + numSizeBytes - count : 0;
    // fprintf (stderr, "requiring %ld\n", (long)r);
    return r;
}

static unsigned char firstByteMark[7] = {0x00, 0x00, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC};

// ===========================================================================
// To serialize the XML object, first use the msgapi to get Unicode,
// then use XML4C's intrinsic transcoding to make UTF-8.  Not that we have
// any need for more than ASCII at the moment.
//
void MessageSerializer::serialize(const Message &t, const char *&data_param, size_t &length)
{
    reset();
    DOMString buffer;
    ((Message &)t).Encode(buffer);
    const XMLCh* DOMStrData = buffer.rawBuffer();

    // Allocate a buffer big enough for the length word, followed by
    // the UTF-8 encoding, with null-termination.
    size_t unilength = buffer.length();
    datap = new char[unilength * (sizeof(XMLCh) / 2 * 3) + 5];

    // XML4C can convert from UTF-8, but not to it.
    // This code taken from the Unicode Standard.
    const XMLCh byteMask = 0xBF;
    const XMLCh byteMark = 0x80;

    const XMLCh *source = DOMStrData;
    char *target = datap + 4;
    for (size_t n = 0; n < unilength; n += 1) {
	int bytesToWrite;
        XMLCh ch = *source++;
        if (ch <= 0x80) {
	    bytesToWrite = 1;
	}
	else if (ch < 0x800) {
	    bytesToWrite = 2;
	}
	else if (ch < 0x10000) {
	    bytesToWrite = 3;
	}
	else if (ch < 0x200000) {
	    bytesToWrite = 4;
	}
	else if (ch < 0x4000000) {
	    bytesToWrite = 5;
	}
	else {
	    bytesToWrite = 6;
	}
	target += bytesToWrite;
	switch (bytesToWrite) {
	    case 6:*--target = (char)((ch | byteMark) & byteMask); ch >>= 6;
	    case 5:*--target = (char)((ch | byteMark) & byteMask); ch >>= 6;
	    case 4:*--target = (char)((ch | byteMark) & byteMask); ch >>= 6;
	    case 3:*--target = (char)((ch | byteMark) & byteMask); ch >>= 6;
	    case 2:*--target = (char)((ch | byteMark) & byteMask); ch >>= 6;
	    case 1:*--target = (char)(ch | firstByteMark[bytesToWrite]);
	}
	target += bytesToWrite;
    }
    length = target - datap;
    *target++ = '\0';

    // Return it.
    data_param = datap;
    size_t len = length - 4;
    // network byte order is big-endian
    datap[3]  = len & 255;
    datap[2]  = (len >> 8) & 255;
    datap[1]  = (len >> 16) & 255;
    datap[0]  = (len >> 24) & 255;
    // fprintf(stderr, "serialize done, string %s, length %d\n", data_param + 4, length);
}

// ===========================================================================
//
MessageSerializer::~MessageSerializer()
{
    reset();
}

}
