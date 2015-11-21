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
// This is a test of threads, in SAThreads,
// Since a Thread is an Excludable, one of its jobs is to
// resolve contention between other threads. 
// This tests that property by running 1 thread which 4 other threads
// use whenver they wish to write anything to stdout.  And they do
// a lot of writing.

// When this test is working, the standard output displays lines of
// 4 types: all 0s, all 1s, all 2s, and all 3s. These come from the 4
// writing threads respectively.  The 4 threads gradually die off,
// until the program terminates after about 1 minute.

#include <stdlib.h>
#include <vector>

#include "threads.h"

#ifdef ISO_CPP_HEADERS
#include <iostream>
#include <sstream>
#else
#include <iostream.h>
#include <strstream.h>
#endif

using namespace SAThreads;
using namespace std;

Thread output_thread("output");
class Output : public Procedure {
public:
    Output(const string s, bool delete_self) : str(s), suicidal(delete_self) {}
    void run() {
	cout << str << endl;
	if (suicidal) {
	    delete this;
	}
    }
private:
    string str;
    bool suicidal;
};

// Number of writers.
int n_writers = 4;

class Writer : public Procedure {
public:
    Writer(int i, int d) : n(i), duration(d) {}
    void run();
private:
    int n;
    int duration;
};

void Writer::run() {
#ifdef ISO_CPP_HEADERS
    stringstream ss;
#else
    strstream ss;
#endif
    for (int i = 0; i < 78; i += 1) {
	ss << n;
    }
#ifdef ISO_CPP_HEADERS
    ss << ends;
#endif
    string s = ss.str();
    while (duration > 0) {
#if 0
	// This alternative is disabled. Enable it to skip the use of the
	// output_thread, and demonstrate what goes wrong: the lines are
	// commingled so as to be unreadable.
	cout << s << endl;
#else
	// Normal use of the output_thread to run a procedure which outputs 1 line.
	output_thread.start(*(new Output(s, true)));
#endif
	duration -= 1;
    }
}

vector<Thread *> writers;

void add_writers() {
    for (int i = 0; i < n_writers; i += 1) {
	// The first writer is set up to write the longest.
	Writer *d = new Writer(i, (n_writers - i) * 10000);
	Thread *h = new Thread("writer");
	h->start(*d);
	writers.push_back(h);
    }
}

void wait_with_writers() {
    // Each writer has a different duration.
    for (int i = 0; i < n_writers; i += 1) {
	writers[i]->lock();
	// The i-th writer is gone.
    }
}

void check_args(int argc, char *argv[]) {
}

int main(int argc, char* argv[])
{
    check_args(argc, argv);
    add_writers();
    wait_with_writers();

    // The destructor for static output_thread makes sure
    // the output thread is finished with its request,
    // but doesn't guarantee it's the final request.
    return 0;
}
