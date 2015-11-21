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
// An unfair test of fairness.

// Intended to help diagnose a bug in HP,
// but this test showed rampant starvation on both Solaris and HP.
// And actually this is an invalid test because this threads library
// was not supposed to prevent starvation in the presence of
// a CPU-hungry thread.

#ifdef ISO_CPP_HEADERS
#include <iostream>
#else
#include <iostream.h>
#endif

#include "threads.h"
#include <math.h>
#include <stdlib.h>

using namespace SAThreads;
using namespace std;

struct expected {
    int id;
    unsigned long age;
    unsigned long fed;
    expected *next;
};
expected *eend = NULL;
expected *next = NULL;

int n_threads;
Mutex mfair;
int hit_count = 0;

extern void check_fair(int id);
extern void run(int id);

void show_expect() {
    cerr << "Expected";
    for (expected *e = next; e != NULL; e = e->next) {
	cerr << " " << e->id;
    }
    cerr << endl;
}

void add_expectation(int i) {
    expected *e = new expected;
    e->id = i;
    e->age = 0;
    e->fed = 0;
    cerr << "Adding " << i << " to ";
    show_expect();
    if (next == NULL) {
	next = e;
	eend = e;
    }
    else {
	eend->next = e;
	eend = e;
    }
    e->next = NULL;
#if 0
    show_expect();
#endif
}

void demote_expectation(int i) {
    expected *e, *prev = NULL;
#if 0
    cerr << "demoting " << i << " from ";
    show_expect();
#endif
    for (e = next; e != eend; e = e->next) {
	if (e->id == i && e != eend) {
	    if (prev == NULL) {
		next = e->next;
	    }
	    else {
		prev->next = e->next;
	    }
	    e->next = NULL;
	    e->age = 0;
	    e->fed += 1;
	    eend->next = e;
	    eend = e;
	    break;
	}
	prev = e;
    }
#if 0
    show_expect();
#endif
}

float age_expectation(float &rms, unsigned long &max_age, unsigned long &n_unfed) {
    expected *e;
    float total = 0.0;
    float stotal = 0.0;
    unsigned long max = 0;
    unsigned long unfed = 0;
    for (e = next; e != eend; e = e->next) {
	total += e->age;
	stotal += float(e->age)*float(e->age);
	if (e->age > max) {
	    max = e->age;
	}
	if (e->fed == 0) {
	    unfed += 1;
	}
	e->age += 1;
    }
    rms = sqrt(stotal/n_threads);
    max_age = max;
    n_unfed = unfed;
    return total;
}

void check_expect(int i) {
#if 0
    if (next == NULL || i != next->id) {
	show_expect();
	cerr << "but got " << i << endl;
    }
    else {
	show_expect();
    }
#endif
    float rms;
    unsigned long max;
    unsigned long unfed;
    float f = age_expectation(rms, max, unfed);
    cout << "avg age of " << n_threads << " thrds after " << hit_count << ": " << f/n_threads << ", rms age: " << rms << ", max age: " << max << ", unfed: " << unfed << endl;
    demote_expectation(i);
}

struct Contender : Procedure {
    Contender(int i) : id(i) {}
    void run() { ::run(id); }
    int id;
};

void add_contender() {
    Contender *c = new Contender(n_threads);
    Thread *t = new Thread;
    t->start(*c);
    add_expectation(n_threads);
    n_threads += 1;
}

void check_fair(int id) {
    mfair.lock();
    hit_count += 1;
    check_expect(id);
    if (n_threads < 2 || hit_count == 20*n_threads*n_threads) {
	add_contender();
    }
    mfair.unlock();
}

void run(int id) {
    for (;;) {
        for (int i = 0; i < 1000; i += 1) {
	    rand();
	}
	check_fair(id);
    }
}

int main (int argc, char *argv[]) {
    n_threads = 1;
    add_expectation(0);
    run(0);
}
