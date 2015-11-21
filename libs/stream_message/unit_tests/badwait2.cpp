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
// lock ... sleep ... notify can notify a waiter that commences
// to wait during the sleep.

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
    if (seconds > 0) {
#ifdef _WIN32
        _sleep(seconds*1000);
#else
        sleep(seconds);
#endif
    }
    return;
}

Monitor m;

struct MarkTime : Procedure {
    MarkTime(unsigned seconds_to_sleep_before, unsigned seconds_to_sleep_after)
	: secs(seconds_to_sleep_before), secs2(seconds_to_sleep_after) {}
    void run() {
	m.lock();
        sleep_in_seconds(secs);
        m.notify();
	sleep_in_seconds(secs2);
	m.unlock();
	delete this;
    }
    unsigned secs, secs2;
};

int main (int argc, char *argv[]) {
    {
	cout << "First part." << endl;
	Thread t;
	t.start(*new MarkTime(5, 3));
	cout << "Giving thread 2 seconds to start up." << endl;
	sleep_in_seconds(2);
	cout << "Waiting for locked notify in 3 seconds." << endl;
	m.wait();
	cout << "Received notification. Waiting 3 seconds to obtain lock." << endl;
	m.lock();
	cout << "Done waiting." << endl;
    }
    
    return 0;
}
