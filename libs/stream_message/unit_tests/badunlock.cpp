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
// This program should just return quickly, after
// trying to unlock things which are not locked.

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

using namespace SAThreads;

Mutex mlocked;
Mutex munlocked;
Thread tlocked;

struct Second : Procedure {
    void run() {
        mlocked.unlock();  // unlock when lock held by main thread
    }
};

struct Fourth : Procedure {
    void run() {
        tlocked.unlock();  // unlock when lock held by main thread
    }
};

struct Fifth : Procedure {
    void run() {
        sleep_in_seconds(5);
    }
};

int main (int argc, char *argv[]) {
    // Zeroth test: unlock then lock.
    // This one hung on Windows.
    {
	cerr << "Starting zeroth test." << endl;
	Mutex m;
	m.unlock();
	m.lock();
	cerr << "Done with zeroth test." << endl;
    }

    // First test: unlock an unlocked thread.
    cerr << "Starting first test." << endl;
    munlocked.unlock();  // unlock when no lock held
    cerr << "Done with first test." << endl;

    // Second test: unlock a thread locked elsewhere.
    // Ensure the lock continues to work.
    cerr << "Starting second test. Should delay 5 seconds." << endl;
    mlocked.lock();
    Thread tester;
    tester.start(*new Second);
    sleep_in_seconds(5);
    mlocked.unlock();
    cerr << "Done with second test." << endl;

    // Third test: unlock an unlocked Thread.
    cerr << "Starting third test." << endl;
    tester.unlock();
    cerr << "Done with third test." << endl;

    // Fourth test: unlock a locked Thread.
    cerr << "Starting fourth test. Should delay 5 seconds." << endl;
    tlocked.lock();
    tester.start(*new Fourth);
    sleep_in_seconds(5);
    tlocked.unlock();
    cerr << "Done with fourth test." << endl;

    // Fifth test: unlock a running Thread.
    cerr << "Starting fifth test. Should delay 5 seconds." << endl;
    Thread fifth;
    fifth.start(*new Fifth);
    fifth.unlock();
    fifth.lock();  // Ensure thread finishes normally.
    fifth.unlock();
    cerr << "Done with fifth test." << endl;
    return 0;
}
