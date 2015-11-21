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
// This program notifies diverse threads simultaneously.

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
Mutex w;

struct MarkTime : Procedure {
    MarkTime(unsigned seconds_to_sleep, bool use_lock)
	: secs(seconds_to_sleep)
	, use_lock(use_lock) {}
    void run() {
        sleep_in_seconds(secs);
	if (use_lock) {
	    m.lock();
	}
        m.notify();
	if (use_lock) {
	    m.unlock();
	}
	delete this;
    }
    unsigned secs;
    bool use_lock;
};

struct UnlockedWaitAndNotify : Procedure {
    UnlockedWaitAndNotify() {}
    void run() {
	m.wait();
        m.notify();
        w.lock();
	cerr << "UnlockedWaitAndNotify done." << endl;
	w.unlock();
        delete this;
    }
};

struct LockedWaitAndNotify : Procedure {
    LockedWaitAndNotify() {}
    void run() {
	m.lock();
	m.wait();
	m.unlock();
        m.notify();
        w.lock();
	cerr << "LockedWaitAndNotify done." << endl;
	w.unlock();
        delete this;
    }
};

struct UnlockedWaitAndWait : Procedure {
    UnlockedWaitAndWait() {}
    void run() {
	m.wait();
        m.wait();
        w.lock();
	cerr << "UnlockedWaitAndWait done." << endl;
	w.unlock();
        delete this;
    }
};

struct LockedWaitAndWait : Procedure {
    LockedWaitAndWait() {}
    void run() {
	m.lock();
	m.wait();
	m.unlock();
        m.wait();
	w.lock();
	cerr << "LockedWaitAndWait done." << endl;
	w.unlock();
        delete this;
    }
};

struct LockedOtherWait : Procedure {
    LockedOtherWait() {}
    void run() {
	sleep_in_seconds(4);
	m.lock();
	m.wait();
	m.unlock();
	w.lock();
	cerr << "LockedOtherWait done." << endl;
	w.unlock();
        delete this;
    }
};

struct UnlockedOtherWait : Procedure {
    UnlockedOtherWait() {}
    void run() {
	sleep_in_seconds(4);
	m.wait();
	cerr << "UnlockedOtherWait done." << endl;
        delete this;
    }
};

int main (int argc, char *argv[]) {
    Thread t1, t2, t3, t4, t5, t6;
    t1.start(*(new UnlockedWaitAndNotify()));
    t2.start(*(new LockedWaitAndNotify()));
    t3.start(*(new UnlockedWaitAndWait()));
    t4.start(*(new LockedWaitAndWait()));
    t5.start(*(new UnlockedOtherWait()));
    t6.start(*(new LockedOtherWait()));
    cerr << "Started threads." << endl;
    sleep_in_seconds(1);
    cerr << "UnlockedWaitAndNotify and LockedWaitAndNotify should complete" << endl
	<< "at 1 second (now)." << endl
	<< "UnlockedWaitAndWait and LockedWaitAndWait may complete" << endl
	<< "either at 1 second (now) or at 5 seconds." << endl;
	    m.notify();
    sleep_in_seconds(1);
    cerr << "2 seconds" << endl;
    sleep_in_seconds(3);
    cerr << "UnlockedOtherWait and LockedOtherWait should complete now (5 seconds)." << endl;
    m.notify();
    sleep_in_seconds(1);
    cerr << "6 seconds" << endl;
    sleep_in_seconds(1);
    cerr << "7 seconds; terminating" << endl;
    return 0;
}
