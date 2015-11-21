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
#ifndef CIRCLEBUF_H
#define CIRCLEBUF_H

// A buffer to hold the recent history of bytes read from a file.
// A CircleBuf object is created with a fixed limit on how
// many bytes it can hold.  'track' is called to keep the buffer up-to-date.

class CircleBuf {
public:
    CircleBuf(unsigned long);
    ~CircleBuf();
    void track(char ch);
    unsigned long count();
    unsigned long trackCount();
    char operator[](long idx);
private:
    CircleBuf(const CircleBuf &); // unimplemented
    unsigned long bufsize;
    char *circle;
    const char *circle_back;
    unsigned long circle_count;
    unsigned long track_count;
    char *circle_p;
};

extern short getBufShort(CircleBuf &buf, long offset);
extern void reportBuf(CircleBuf &c, long num);
extern bool bufAt(CircleBuf &buf, const char *p, long count);

#endif // CIRCLEBUF_H
