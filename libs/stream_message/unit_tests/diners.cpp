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
// This is a test of monitors and threads, in SAThreads,
// sturner's own imagining of dining philosophers.
// 
// There are 3 folks dining at the round table, set with 3 forks in
// between the 3 folks.
// Each person takes a fork for a while, then puts it down for a while.
// The catch is that person 0 likes to take fork 1,
// person 1 switches between fork 1 or 2, and person 2 prefers fork 2.
// So we get contention which can cause person 1 to have no fork
// available.
//
// The output of this program 
//          diners.exe -no_fork_reports
// should be a list showing mostly diner 1 sleeping .. diner 1 awakened,
// with an occasional occurrence of diner 0 or diner 2.
//
// A tester should also verify that the program is not in a busy wait,
// consuming lots of CPU time.
// 
// This runs slowly due to the sleep time being measured in seconds.
// It was run overnight, filling a console with 2048 lines and no errors.
//

#include <stdlib.h>
#include <vector>

#include "threads.h"

#ifdef ISO_CPP_HEADERS
#include <iostream>
#else
#include <iostream.h>
#endif

#ifdef __unix
#include <unistd.h>
#endif

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

bool no_fork_reports = false;
bool no_sleep_reports = false;

// It turned out that the random numbers were not working out, evidently because
// rand() got initialized to the same sequence for each thread.  So as a further
// test of Monitor, a thread is added which is responsible for all calls to rand().

// Global where the random thread puts its result.
int rnext = -1;

// The monitor controlling communication with the random thread.
Monitor rmon;

// The random thread procedure.
// This stays in the monitor the whole time, except while waiting
// to compute.
class Random : public Procedure {
public:
    void run() {
	Exclusion e(rmon, "generating a random");
	while (1) {
	    while (rnext >= 0) {
		rmon.wait();
	    }
	    rnext = rand();
	    rmon.notify();
	}
    }

};

// Takes the place of rand(), this is called by various threads to communicate
// with the random thread. 
int get_rand() {
    // If locking is fair, a greedy lock will do.
    int result;
    {
        Exclusion e(rmon, "getting a random");
	do {
            result = rnext;
	    if (result >= 0) {
		rnext = -1;
		rmon.notify();
	    }
	    else {
	        rmon.wait();
	    }
        } while (result < 0);
    }
    return result;
}

// Initialize randomness by starting its thread.
void start_random() {
    Random *r = new Random;
    Thread *h = new Thread("random number generation");
    h->start(*r);
}

// Convenient function to return a random number within a range.
int random(int sup) {
    int r = get_rand();
    int x = r % sup;
    if (x < 0) {
	x = x + sup;
    }
    return x;
}

// Number of diners.
int n_places = 3;

class Diner;

// One object for tracking the disposition of forks.
// The mutex it uses was declared as a base class, a design which is poor.
// This will report an error if the diners' Monitor fails in its purpose,
// and two use the same fork simultaneously.
class Forks : public Mutex {
public:
    void take(int, Diner *);
    void put(int, Diner *);
    bool available(int);
    Forks();
private:
    vector<Diner *> forks;
};

Forks::Forks() {
    for (int i = 0; i < n_places; i += 1) {
	forks.push_back(NULL);
    }
}
void Forks::take(int place, Diner *d) {
    Exclusion e(*this);
    if (forks.operator[](place) != NULL) {
	cerr << "Take error." << endl;
    }
    forks.operator[](place) = d;
}
void Forks::put(int place, Diner *d) {
    Exclusion e(*this);
    if (d != forks.operator[](place)) {
	cerr << "Put error." << endl;
    }
    forks.operator[](place) = NULL;
}
bool Forks::available(int place) {
    Exclusion e(*this);
    return forks.operator[](place) == NULL;
}

Forks forks;

// The Monitor we're testing, which coordinates the diners.
Monitor server;

class Diner : public Procedure
{
public:
    Diner(int);
    void run();
    int place() { return p; }
private:
    int p;
};

vector<Thread *> diners;

Diner::Diner(int place) : p(place) {
}
void Diner:: run() {
    while (1) {
	// Each diner has a preference for a fork on one side or the
	// other, but if one is busy will take the other.
	int r = random(2);
	// Suppress randomness for more contention, because
	// we weren't getting any sleeping.
	if (place() % 3 == 2) {
	    r = 0;
	}
	if (place() % 3 == 0) {
	    r = 1;
	}
	int desired_fork_place = (place() + r) % n_places;
	int alternate_fork_place = (place() + (1 - r)) % n_places;
	int have_place;
	{
	    Exclusion e(server);
	    while (!(forks.available(desired_fork_place) || forks.available(alternate_fork_place))) {
		if (!no_sleep_reports) {
		    cout << "Diner " << place() << " sleeping." << endl;
		}
		server.wait();
		if (!no_sleep_reports) {
		    cout << "Diner " << place() << " awakened." << endl;
		}
	    }
	    have_place = forks.available(desired_fork_place)
			 ? desired_fork_place
			 : alternate_fork_place;
	    forks.take(have_place, this);
	}
	if (!no_fork_reports) {
	    cout << "Diner " << place() << " takes fork " << have_place << "." << endl;
	}

	sleep_in_seconds(2 + random(5));
	if (!no_fork_reports) {
	    cout << "Diner " << place() << " drops fork " << have_place << "." << endl;
	}
	forks.put(have_place, this);
	server.notify();
	sleep_in_seconds(2 + random(5));
    }
}


void add_diner(int n_diners) {
    int selection = random(n_places - n_diners);
    // Find the selection-th empty place among the diners.
    int empty_count = 0;
    int i = 0;
    while (diners.operator[](i) != NULL || selection < empty_count) {
	if (diners.operator[](i) == NULL) {
	    empty_count += 1;
	}
	i += 1;
    }
    Diner *d = new Diner(i);
    Thread *h = new Thread("diner");
    h->start(*d);
    diners[i] = h;
}

void seat_diners() {
    for (int i = 0; i < n_places; i += 1) {
	diners.push_back(NULL);
    }
    for (int j = 0; j < n_places; j += 1) {
	add_diner(j);
	sleep_in_seconds(1);
    }
}

void wait_with_diners() {
    for (int i = 0; i < n_places; i += 1) {
	diners[i]->lock();
	// The i-th diner is gone.
    }
}

void check_args(int argc, char *argv[]) {
    for (int i = 1; i < argc; i += 1) {
	if (strcmp(argv[i], "-no_fork_reports") == 0) {
	    no_fork_reports = true;
	}
	if (strcmp(argv[i], "-no_sleep_reports") == 0) {
	    no_sleep_reports = true;
	}
    }
}

int main(int argc, char* argv[])
{
    check_args(argc, argv);
    cout << "About to start random." << endl;
    start_random();
    cout << "Random is started." << endl;
    seat_diners();
    wait_with_diners();
    return 0;
}
