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
// A test of fairness. If several threads wait for a monitor,
// then one thread should't be able to grab it twice before
// another grabs it at all.

// Well, I've downgraded that expectation.  Now it reports the
// average age and RMS age of threads.  You get not-so-nice
// results on Solaris 2.5 but not deadly.

#ifdef ISO_CPP_HEADERS
#include <iostream>
#else
#include <iostream.h>
#endif

#include "threads.h"
#include <math.h>

using namespace SAThreads;
using namespace std;

struct expected {
    int id;
    unsigned long age;
    expected *next;
};
expected *eend = NULL;
expected *next = NULL;

int n_threads;
Mutex mfair;
Monitor target;
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

float age_expectation(float &rms) {
    expected *e;
    float total = 0.0;
    float stotal = 0.0;
    for (e = next; e != eend; e = e->next) {
	total += e->age;
	stotal += float(e->age)*float(e->age);
	e->age += 1;
    }
    rms = sqrt(stotal/n_threads);
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
    float f = age_expectation(rms);
    cout << "average age of " << n_threads << " threads is " << f/n_threads << "," << " rms age is " << rms << "." << endl;
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
    target.lock();
    for (;;) {
	check_fair(id);
	target.notify();
	target.wait();
    }
}

int main (int argc, char *argv[]) {
    n_threads = 1;
    add_expectation(0);
    run(0);
}
