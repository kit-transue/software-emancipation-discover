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
// Sometimes threads are created which we don't want to keep track of.
// Such threads need to destroy their Thread on their own.
// Make sure we don't get a space leak.

// This test starts lots of such threads, 10 per second.

#ifdef ISO_CPP_HEADERS
#include <iostream>
#else
#include <iostream.h>
#endif

#ifdef __unix
#include <unistd.h>
#endif
#include "threads.h"

using namespace SAThreads;
namespace std{}
using namespace std;

//=============================================================================
void sleep_in_seconds(unsigned seconds)
{
#ifdef _WIN32
    _sleep(seconds*1000);
#else
    sleep(seconds);
#endif
    return;
}

struct Suicide : Procedure {
    Suicide(Thread &thread) : thread(thread) {};
    void run() {
	delete &thread;
	sleep_in_seconds(1);
	cout << "at end of thread" << endl;
    }
    Thread &thread;
};

int main (int argc, char *argv[]) {
    for (int j = 0; j < 15; j += 1) {
	for (int i = 0; i < 30; i += 1) {
	    Thread &t = *new Thread;
	    t.start(*new Suicide(t));
	}
	sleep_in_seconds(1);
    }
    sleep_in_seconds(3600); // Give the threads a chance to die off.
    return 0;
}
