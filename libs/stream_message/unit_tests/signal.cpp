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
// This program should wait 5 seconds, print "Done waiting in second thread,"
// wait 5 more seconds, print "Done waiting in main thread,"
// wait 5 more seconds, and exit.

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

Monitor m;

struct MarkTime : Procedure {
    void run() {
        m.wait();
	cout << "Done waiting in second thread." << endl;
	sleep_in_seconds(5);
	m.notify();
	sleep_in_seconds(5);
    }
};

int main (int argc, char *argv[]) {
    Thread t;
    t.start(*new MarkTime);
    sleep_in_seconds(5);
    m.notify();
    m.wait();
    cout << "Done waiting in main thread." << endl;
    t.lock();
    return 0;
    // It will take a while for the return to complete, because it
    // locks t, which waits on the completion of its thread.
}
