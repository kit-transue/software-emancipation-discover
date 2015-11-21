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
#include "threads.h"

#ifdef _WIN32
#include <windows.h>
#include <process.h>
#endif

#ifdef __unix
#include <unistd.h>
#endif

#ifdef ISO_CPP_HEADERS
#include <sstream>
#include <iostream>
#else
#include <strstream.h>
#include <iostream.h>
#endif

//=============================================================================
void sathread_sleep_in_seconds(unsigned seconds)
{
#ifdef _WIN32
    _sleep(seconds*1000);
#else
    sleep(seconds);
#endif
    return;
}

namespace std{}
using namespace std;

#ifdef PTHREAD_THREAD_IMPLEMENTATION
#ifdef PTHREAD_DRAFT_4
#define pthread_attr_init pthread_attr_create
#endif

static pthread_mutex_t sathread_mutex_init
#ifndef PTHREAD_DRAFT_4
 = PTHREAD_MUTEX_INITIALIZER
#endif
                            ;

#ifdef PTHREAD_DRAFT_4
static pthread_mutexattr_t mutex_attr;
static void *init_mutex_attr() {
    pthread_mutexattr_create(&mutex_attr);
    return &mutex_attr;
}
static void *mutex_attr_p = init_mutex_attr();
#endif

#ifdef SATHREAD_DEBUG
static pthread_mutex_t thid_mutex
#ifndef PTHREAD_DRAFT_4
 = PTHREAD_MUTEX_INITIALIZER
#endif
                           ;

static void *init_thid_mutex() {
    pthread_mutex_init(&thid_mutex,
#ifndef PTHREAD_DRAFT_4
			            NULL
#else
			            mutex_attr
#endif
				        );
    return &thid_mutex;
}
static void *thid_mutex_p = init_thid_mutex();
#endif
static pthread_attr_t detach_attr;
static void *init_detach_attr() {
    pthread_attr_init(&detach_attr);
#ifndef PTHREAD_DRAFT_4
    if (pthread_attr_setdetachstate(&detach_attr, PTHREAD_CREATE_DETACHED) != 0) {
#ifdef SATHREAD_DEBUG
	cerr << "Failed to set detach_attr." << endl;
#endif
    }
#endif
    return &detach_attr;
}
static void *detach_attr_p = init_detach_attr();
#endif

namespace SAThreads {

#ifdef SATHREAD_DEBUG
struct Thid {
#ifdef WIN32_THREAD_IMPLEMENTATION
    unsigned long n;
#endif
#ifdef PTHREAD_THREAD_IMPLEMENTATION
    pthread_t pthread;
#endif
    string id;
    struct Thid *next;
} *thids = NULL;

#ifdef WIN32_THREAD_IMPLEMENTATION
static HANDLE thid = CreateMutex(NULL, FALSE, NULL);
#endif

static void lookup_thid(string &s) {
#ifdef WIN32_THREAD_IMPLEMENTATION
    unsigned long n = GetCurrentThreadId();
    WaitForSingleObject(thid, INFINITE);
    for (Thid *p = thids; p != NULL && p->n != n; p = p->next) {
    }
    stringstream ss;
    if (p == NULL) {
	ss << "?";
    }
    else {
	ss << p->id;
    }
    ss << " " << n << ends;
    s = ss.str();
    ReleaseMutex(thid);
#endif
#ifdef PTHREAD_THREAD_IMPLEMENTATION
    pthread_t pthread = pthread_self();
    pthread_mutex_lock(&thid_mutex);
    Thid *p = thids;
    for (; p != NULL && !pthread_equal(p->pthread, pthread); p = p->next) {
    }
#ifdef ISO_CPP_HEADERS
    stringstream ss;
#else
    strstream ss;
#endif
    if (p == NULL) {
	ss << "?";
    }
    else {
	ss << p->id;
    }
#ifdef __hpux
    ss << " ?" << ends;
#else
    ss << " " << pthread << ends;
#endif
    s = ss.str();
#ifdef __linux__
    {
      string s2 = s.c_str();
      s = s2;
    }
#endif

    pthread_mutex_unlock(&thid_mutex);
#endif
}
#endif

// ===========================================================================
void initialize() {
  static bool initialized = false;
  if (initialized == false) {
    // Initialize whatever.
    ; // nothing for now
    initialized = true;
  }
}

class CountedMonitor : public Monitor {
public:
    CountedMonitor() : count(1) {}
    int count;
};

// ===========================================================================
Thread::Thread(const char *k)
:
  isLocked(false),
  isThread(false),
  has_kind(k != 0)
{
#ifdef SATHREAD_DEBUG
    if (has_kind) {
	kind = k;
    }
#endif
    guardp = new CountedMonitor;
}

// ===========================================================================
//
Thread::~Thread() {
    guardp->lock();
    if (--guardp->count > 0) {
        // Another reference to the guard is still held by the
	// ThreadProcedure, which is still running.
        guardp->unlock();
    }
    else {
        delete guardp;
    }
}

// ===========================================================================
class ThreadProcedure {
public:
    ThreadProcedure(Procedure &thread_proc, CountedMonitor *guardp,
		    bool &object_is_thread,
		    const char *s)
          : proc(thread_proc), guardp(guardp),
	    objectIsThread(object_is_thread),
	    hasId(s != NULL)
    {
        if (hasId) {
	    id = s;
	}
    }
    void run() {
        guardp->lock(); // Don't proceed until thread is fully launched.
	guardp->unlock();
#ifdef SATHREAD_DEBUG
	pthread_t pthread = pthread_self();
	bool has_id = hasId;
	if (hasId) {
	    pthread_mutex_lock(&thid_mutex);
	    Thid *p = new Thid;
	    p->pthread = pthread;
	    p->id = id;
	    p->next = thids;
	    thids = p;
	    pthread_mutex_unlock(&thid_mutex);
	}
	string thid_string;
	lookup_thid(thid_string);
	cerr << "Starting thread procedure for " << thid_string << "." << endl;
#endif // SATHREAD_DEBUG
	proc.run();
        // Don't use proc or &proc after this; run is allowed to delete the Procedure.
	guardp->lock();
	if (--guardp->count > 0) {
	    // Another reference to the guard is still held by the Thread.
	    objectIsThread = false;
	    guardp->notify();
	    guardp->unlock();
	}
	else {
	    delete guardp;
	}
	delete this;
#ifdef SATHREAD_DEBUG
	if (has_id) {
	    pthread_mutex_lock(&thid_mutex);
	    Thid *prev_p = NULL;
	    Thid *p = thids;
	    for (; p != NULL && !pthread_equal(p->pthread, pthread); p = p->next) {
		prev_p = p;
	    }
	    if (p != NULL) {
		if (prev_p == NULL) {
		    thids = p->next;
		}
		else {
		    prev_p->next = p->next;
		}
		delete p;
	    }
	    pthread_mutex_unlock(&thid_mutex);
	}
	cerr << "Done running thread procedure for " << thid_string << "." << endl;
#endif // SATHREAD_DEBUG
#ifdef PTHREAD_THREAD_IMPLEMENTATION
	pthread_exit(NULL);
#endif
    }
private:
    Procedure &proc;
    CountedMonitor *guardp;
    bool &objectIsThread;
    bool hasId;
    string id;
};

#ifdef WIN32_THREAD_IMPLEMENTATION
// ===========================================================================
static unsigned __stdcall jump_start_procedure(void *ptr) {
#endif // WIN32_THREAD_IMPLEMENTATION
#ifdef PTHREAD_THREAD_IMPLEMENTATION
extern "C" {
static void *jump_start_procedure (void *ptr) {
#endif // PTHREAD_THREAD_IMPLEMENTATION
#ifdef SATHREAD_DEBUG
    cerr << "Started the thread." << endl;
#endif
    ThreadProcedure &thread_proc2 = *(ThreadProcedure *)ptr;
    thread_proc2.run();
    // Don't use proc or &proc after this; run is allowed to delete the Procedure.
#ifdef WIN32_THREAD_IMPLEMENTATION
    return 0;
}
#endif // WIN32_THREAD_IMPLEMENTATION
#ifdef PTHREAD_THREAD_IMPLEMENTATION
    return NULL;
}
}
#endif // PTHREAD_THREAD_IMPLEMENTATION

// ===========================================================================
void Thread::start(Procedure &proc) {
#ifdef SATHREAD_DEBUG
    cerr << "Starting a thread." << endl;
#endif

    ThreadProcedure *thread_proc = new ThreadProcedure(proc,
					    guardp,
					    isThread,
#ifdef SATHREAD_DEBUG
					    has_kind ? kind.c_str() : NULL);
#else
					    NULL);
#endif // !SATHREAD_DEBUG
    guardp->lock();
    if (isLocked || isThread) {
	for (;;) {
	    guardp->wait();
	    if (!(isLocked || isThread)) {
		break;
	    }
	}
    }
    isThread = true;
    guardp->count += 1;  // should be 2: for Thread and for ThreadProcedure
#ifdef WIN32_THREAD_IMPLEMENTATION
    HANDLE threadHandle = (HANDLE)_beginthreadex( 
	NULL,                        // no security attributes 
	0,                           // use default stack size  
	jump_start_procedure,        // thread function 
	thread_proc,                 // argument to thread function 
	0,                           // use default creation flags 
	(unsigned *)(&locking_thread));// returns the thread identifier
#endif

#ifdef PTHREAD_THREAD_IMPLEMENTATION
    int status = pthread_create(&locking_thread,
#ifndef PTHREAD_DRAFT_4
					  &detach_attr,
#else
					  detach_attr,
#endif
                                          jump_start_procedure, 
                                          thread_proc);
    if (status != 0) {
	// error
    }
#ifdef PTHREAD_DRAFT_4
    pthread_detach(&locking_thread);
#endif
#endif

    guardp->unlock();

#ifdef SATHREAD_DEBUG
#ifdef WIN32_THREAD_IMPLEMENTATION
    cerr << "Thread handle is " << ((unsigned long)threadHandle) << "." << endl;
#endif // WIN32_THREAD_IMPLEMENTATION
#ifdef PTHREAD_THREAD_IMPLEMENTATION
    if (status == 0) {
        cerr << "Thread handle is " << "undetermined" << "." << endl;
    }
    else {
        cerr << "Starting a thread failed." << endl;
    }
#endif
#endif
#ifdef WIN32_THREAD_IMPLEMENTATION
    CloseHandle(threadHandle);
#endif // WIN32_THREAD_IMPLEMENTATION
}

// ===========================================================================
// Return when the thread exits.
//
void Thread::lock() {
    Exclusion l(*guardp);
    if (isLocked || isThread) {
	for (;;) {
	    guardp->wait();
	    if (!(isLocked || isThread)) {
		break;
	    }
	}
    }
    isLocked = true;
#ifdef PTHREAD_THREAD_IMPLEMENTATION
    locking_thread = pthread_self();
#endif
#ifdef WIN32_THREAD_IMPLEMENTATION
    locking_thread = GetCurrentThreadId();
#endif
}

// ===========================================================================
void Thread::unlock() {
    guardp->lock();
#ifdef PTHREAD_THREAD_IMPLEMENTATION
    if (isLocked && pthread_equal(locking_thread, pthread_self())) {
#endif
#ifdef WIN32_THREAD_IMPLEMENTATION
    if (isLocked && locking_thread == GetCurrentThreadId()) {
#endif
	isLocked = false;
        guardp->notify();
    }
    guardp->unlock();
}

// ===========================================================================
//
Mutex::Mutex()
#ifdef WIN32_THREAD_IMPLEMENTATION
  : lockCount(0)
#endif
#ifdef PTHREAD_THREAD_IMPLEMENTATION
  : mutex(sathread_mutex_init)
  , idGuard(sathread_mutex_init)
  , isLocked(false)
#endif
{
#ifdef WIN32_THREAD_IMPLEMENTATION
    mutex = CreateMutex(NULL, FALSE, NULL);
#endif
#ifdef PTHREAD_THREAD_IMPLEMENTATION
    pthread_mutex_init(&mutex,
#ifndef PTHREAD_DRAFT_4
			       NULL
#else
			       mutex_attr
#endif
				   );
    pthread_mutex_init(&idGuard,
#ifndef PTHREAD_DRAFT_4
			       NULL
#else
			       mutex_attr
#endif
				   );
#endif
}

// ===========================================================================
//
Mutex::~Mutex() {
#ifdef WIN32_THREAD_IMPLEMENTATION
    if (mutex != NULL) {
        CloseHandle(mutex);
    }
#endif
#ifdef PTHREAD_THREAD_IMPLEMENTATION
    pthread_mutex_destroy(&mutex);
    pthread_mutex_destroy(&idGuard);
#endif
}

// ===========================================================================
//
void Mutex::lock() {
#ifdef WIN32_THREAD_IMPLEMENTATION
    if (   mutex != NULL
        && WaitForSingleObject(mutex, INFINITE) != WAIT_FAILED
        && lockCount == 0) {
    }
    else {
#ifdef SATHREAD_DEBUG
	cerr << "Failure while locking Mutex." << endl;
#endif
	// Ick!  Don't just exit the thread, hanging is more appropriate.
	for (;;) sathread_sleep_in_seconds(1000000U);
    }
    lockCount = 1;
#endif
#ifdef PTHREAD_THREAD_IMPLEMENTATION
    pthread_mutex_lock(&idGuard);
    bool ok = !(isLocked && pthread_equal(lockId, pthread_self()));
    pthread_mutex_unlock(&idGuard);
    if (!ok) {
#ifdef SATHREAD_DEBUG
	cerr << "Failure while locking Mutex." << endl;
#endif
	// Ick!  Don't just exit the thread, hanging is more appropriate.
	for (;;) sathread_sleep_in_seconds(1000000U);
    }
    pthread_mutex_lock(&mutex);
    pthread_mutex_lock(&idGuard);
    isLocked = true;
    lockId = pthread_self();
    pthread_mutex_unlock(&idGuard);
#endif
}

// ===========================================================================
//
void Mutex::unlock() {
#ifdef WIN32_THREAD_IMPLEMENTATION
    lockCount = 0;
    if (mutex != NULL) {
        ReleaseMutex(mutex);
    }
#endif
#ifdef PTHREAD_THREAD_IMPLEMENTATION
    pthread_mutex_lock(&idGuard);
    isLocked = false;
    pthread_mutex_unlock(&idGuard);
    pthread_mutex_unlock(&mutex);
#endif
}

// ===========================================================================
// Wait if necessary, then take the Mutex exclusively.
//
Exclusion::Exclusion(Excludable &m, const char *s) : mutex(m), hasReason(s != NULL) {
#ifdef SATHREAD_DEBUG
    if (hasReason) {
	reason = s;
	string thid;
	lookup_thid(thid);
	cerr << "Thread " << thid << " locking for " << reason << endl;
    }
#endif
    mutex.lock();
#ifdef SATHREAD_DEBUG
    if (hasReason) {
	reason = s;
	string thid;
	lookup_thid(thid);
	cerr << "Thread " << thid << " obtained lock for " << reason << endl;
    }
#endif
}

// ===========================================================================
// Free the mutex.
//
Exclusion::~Exclusion() {
    mutex.unlock();
#ifdef SATHREAD_DEBUG
    if (hasReason) {
	string thid;
	lookup_thid(thid);
	cerr << "Thread " << thid << " unlocked for " << reason << endl;
    }
#endif
}

// ===========================================================================
//
Monitor::Monitor()
#ifdef WIN32_THREAD_IMPLEMENTATION
    : lockId(0)
#endif
#ifdef PTHREAD_THREAD_IMPLEMENTATION
#if !BROADCAST_TO_0
    : waiterCount(0),
      altWaiterCount(0)
#endif
#endif
{
#ifdef WIN32_THREAD_IMPLEMENTATION
    mutex = CreateMutex(NULL, FALSE, NULL);
    event = CreateEvent(NULL, TRUE /* manual reset */, FALSE /* signalled */,
			NULL);
#endif
#ifdef PTHREAD_THREAD_IMPLEMENTATION
#ifdef PTHREAD_DRAFT_4
    pthread_condattr_create(&condAttr);
#endif
    pthread_cond_init(&altCond,
#ifndef PTHREAD_DRAFT_4
			     NULL
#else
			     condAttr
#endif
				 );
    pthread_cond_init(&cond,
#ifndef PTHREAD_DRAFT_4
			     NULL
#else
			     condAttr
#endif
				 );
    pthread_mutex_init(&altMutex,
#ifndef PTHREAD_DRAFT_4
			       NULL
#else
			       mutex_attr
#endif
				   );
#endif
}

// ===========================================================================
//
Monitor::~Monitor() {
#ifdef WIN32_THREAD_IMPLEMENTATION
    if (mutex != NULL) {
        CloseHandle(mutex);
    }
    if (event != NULL) {
	CloseHandle(event);
    }
#endif
#ifdef PTHREAD_THREAD_IMPLEMENTATION
    pthread_mutex_destroy(&altMutex);
    pthread_cond_destroy(&cond);
    pthread_cond_destroy(&altCond);
#ifdef PTHREAD_DRAFT_4
    pthread_condattr_delete(&condAttr);
#endif
#endif
}

// ===========================================================================
//
void Monitor::lock() {
#ifdef WIN32_THREAD_IMPLEMENTATION
    if (mutex != NULL) {
        if (WaitForSingleObject(mutex, INFINITE) == WAIT_FAILED) {
	    // Ick!
	    _endthreadex(0 /* thread exit code */);
	}
    }
    {
	Exclusion l(idGuard);
	lockId = GetCurrentThreadId();
    }
#endif
#ifdef PTHREAD_THREAD_IMPLEMENTATION
    mutex.lock();
#endif
}

// ===========================================================================
//
void Monitor::unlock() {
#ifdef WIN32_THREAD_IMPLEMENTATION
    {
	Exclusion l(idGuard);
	lockId = 0;
    }
    if (mutex != NULL) {
        ReleaseMutex(mutex);
    }
#endif
#ifdef PTHREAD_THREAD_IMPLEMENTATION
    mutex.unlock();
#endif
}

// ===========================================================================
// Wait for a notify, even if the monitor is not locked.
//
void Monitor::wait() {
#ifdef WIN32_THREAD_IMPLEMENTATION
#ifdef SATHREAD_DEBUG
    string s;
    lookup_thid(s);
    cerr << "Thread " << s << " waiting for monitor notify." << endl;
#endif
    int thread_id = GetCurrentThreadId();
    bool have_lock;
    {
	Exclusion l(idGuard);
        have_lock = (lockId == thread_id);
    }
    if (have_lock) {
	lockId = 0;
    }
    if (have_lock && mutex != NULL) {
	// This function requires NT 4.0
	// (i.e. use of _WIN32_WINNT during compilation)
	// in order to avoid a classic race condition that you risk by doing
	// a ReleaseMutex followed by a Wait for the event.
	// If some other thread takes over in between and signals the event,
	// then this thread could end up dangling.
	SignalObjectAndWait(mutex, event, 10000, FALSE);
    }
    else {
	WaitForSingleObject(event, 10000);
    }
    if (have_lock) {
#ifdef SATHREAD_DEBUG
	cerr << "Thread " << s << " waiting to regain monitor mutex." << endl;
#endif
	if (mutex != NULL) {
	    if (WaitForSingleObject(mutex, INFINITE) == WAIT_FAILED) {
		// Ick!
		_endthreadex(0 /* thread exit code */);
	    }
	}
#ifdef SATHREAD_DEBUG
	cerr << "Thread " << s << " regained monitor mutex." << endl;
#endif
	lockId = thread_id;
    }
#endif // WIN32_THREAD_IMPLEMENATION
#ifdef PTHREAD_THREAD_IMPLEMENTATION
#ifdef SATHREAD_DEBUG
    string s;
    lookup_thid(s);
    cerr << "Thread " << s << " waiting for monitor notify." << endl;
#endif
    pthread_t thread_id = pthread_self();
    bool have_lock;
    pthread_mutex_lock(&mutex.idGuard);
    have_lock = mutex.isLocked && pthread_equal(mutex.lockId, thread_id);
    if (have_lock) {
	mutex.isLocked = false;
    }
    pthread_mutex_unlock(&mutex.idGuard);
    if (have_lock) {
	// Avoid a classic race condition that you risk by doing
	// an unlock followed by a Wait for the event.
	// If some other thread takes over in between and signals the event,
	// then this thread could end up dangling.
#if !BROADCAST_TO_0
	pthread_mutex_lock(&altMutex);
	waiterCount += 1;
	pthread_mutex_unlock(&altMutex);
#endif
	pthread_cond_wait(&cond, &mutex.mutex);

	// Don't continue until notify unlocks the altMutex.
	pthread_mutex_lock(&altMutex);
	pthread_mutex_unlock(&altMutex);
    }
    else {
	// Unusual case: didn't lock the mutex.
	// Use altMutex for the sake of the pthreads requirement.
	pthread_mutex_lock(&altMutex);
#if !BROADCAST_TO_0
	altWaiterCount += 1;
#endif
	pthread_cond_wait(&altCond, &altMutex);
	pthread_mutex_unlock(&altMutex);
    }
    if (have_lock) {
#ifdef SATHREAD_DEBUG
	cerr << "Thread " << s << " regained monitor mutex." << endl;
#endif
	pthread_mutex_lock(&mutex.idGuard);
	mutex.lockId = thread_id;
	mutex.isLocked = true;
	pthread_mutex_unlock(&mutex.idGuard);
    }

#endif // PTHREAD_THREAD_IMPLEMENTATION
}

// ===========================================================================
// Make all waiting threads eligible to run.
// If the current thread does not have the monitor locked,
// obtains the lock before notifying.  This keeps activity related to the
// monitor sequential.
//
void Monitor::notify() {
#ifdef SATHREAD_DEBUG
    string s;
    lookup_thid(s);
    cerr << "Thread " << s << " notifying on mutex." << endl;
#endif
#ifdef WIN32_THREAD_IMPLEMENTATION
    PulseEvent(event);
#endif
#ifdef PTHREAD_THREAD_IMPLEMENTATION
    pthread_mutex_lock(&altMutex);
#if BROADCAST_TO_0
    pthread_cond_broadcast(&cond);
    pthread_cond_broadcast(&altCond);
#else
    // Screwed up signaling; if no threads are waiting, 
    // saves the signal and notifies later.
    if (waiterCount > 0) {
	waiterCount = 0;
	pthread_cond_broadcast(&cond);
    }
    if (altWaiterCount > 0) {
	altWaiterCount = 0;
	pthread_cond_broadcast(&altCond);
    }
#endif
    pthread_mutex_unlock(&altMutex);
#endif
}

}
