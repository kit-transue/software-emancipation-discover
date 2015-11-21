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
// This program attempts to notify without holding a lock on the monitor.
// The behavior should be that the notifying thread waits.
// It runs three experiments.  The first should finish after 10 seconds,
// missing the fact that the monitor was notified while it was sleeping.
// The second won't finish until the blocker is done at 10 seconds,
// because the notifier doesn't go active until the blocker is done.
// The third should take 5 seconds.

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
    MarkTime(unsigned seconds_to_sleep) : secs(seconds_to_sleep) {}
    void run() {
        sleep_in_seconds(secs);
        m.notify();
	delete this;
    }
    unsigned secs;
};

struct Blocker : Procedure {
    Blocker(unsigned seconds_to_sleep) : secs(seconds_to_sleep) {}
    void run() {
	m.lock();
        sleep_in_seconds(secs);
	m.unlock();
	delete this;
    }
    unsigned secs;
};

int main (int argc, char *argv[]) {
    m.lock();
    {
	cout << "First experiment." << endl;
	Thread t;
	t.start(*new MarkTime(0));
	Thread killer;
	killer.start(*new MarkTime(10));
	sleep_in_seconds(5);
	cout << "Done sleeping 5 seconds in main thread. Now should wait 5 seconds." << endl;
	m.wait();
	cout << "Done waiting in main thread." << endl;
    }
    {
	cout << "Second experiment." << endl;
	Thread t;
	t.start(*new MarkTime(5));
	Thread b;
	b.start(*new Blocker(10));
	cout << "Launched threads. Should be notified when blocker is done in 10 seconds." << endl;
	m.wait();
	cout << "Done waiting in main thread." << endl;
    }
    {
	cout << "Third experiment." << endl;
	Thread t;
	t.start(*new MarkTime(5));
	cout << "Waiting for unlocked notification in 5 seconds." << endl;
	m.wait();
	cout << "Done waiting in main thread." << endl;
    }
    
    return 0;
}
