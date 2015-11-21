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
// This module provides a thin portability layer on top of native threads.
// At this stage, it is implemented only for MS Windows (ifdeffed to
// indicate the portions which are OS-specific).
// The major features are:
//    class Thread      Handle to a thread.
//    class Mutex       Conventional mechanism for preventing simultaneous
//                      access to a resource shared among threads.
//    class Monitor     One thread can wait for notification from another.
//    class Exclusion   Convenient way to lock a Mutex, Monitor, etc.

#ifndef SATHREADS_H
#define SATHREADS_H

#ifdef _WIN32
#define WIN32_THREAD_IMPLEMENTATION
#endif

#ifdef __linux__
#define PTHREAD_THREAD_IMPLEMENTATION
#endif

#if defined(__sun) || defined(__hpux) || defined(__sgi)
#define PTHREAD_THREAD_IMPLEMENTATION
#ifndef _REENTRANT
#error _REENTRANT must be defined for threads to work.
#endif
#endif

#ifdef WIN32_THREAD_IMPLEMENTATION
#include <windows.h>
#endif

#ifdef PTHREAD_THREAD_IMPLEMENTATION
#if defined(__hpux)
// Prevent the definition of macros like write and send.
// If we needed to call thread-friendly write, we'd have to get
// much more serious than just replace these function calls.
#define _CMA_NOWRAPPERS_
#endif
#include <pthread.h>
#ifndef PTHREAD_MUTEX_INITIALIZER
// For HP 10.
#define PTHREAD_DRAFT_4
#endif
#endif

#ifdef PTHREAD_DRAFT_4
// goofy when broadcasting and num waiters is 0
#define BROADCAST_TO_0 0
#else
#define BROADCAST_TO_0 1
#endif

#include <string>
namespace std {}
#if defined(SATHREAD_DEBUG) && defined(__hpux) && defined(PTHREAD_DRAFT_4)
using namespace std;
#endif

namespace SAThreads {
  class Monitor;
  class Thread;
  class Procedure;
  class Exclusion;
  class CountedMonitor;

  // Call initialize prior to using these functions.
  void initialize();

  // An object of this class indicates a procedure to execute.
  // Derive from this class to provide the procedure to a thread.
  class Procedure {
  public:
     virtual void run() = 0;
     virtual ~Procedure() {}     
  };

  // Any lockable object.
  // Can be a Mutex, a Monitor, or a Thread.
  // When one thread double-locks one resource, it causes a deadlock.
  class Excludable {
  public:
     virtual void lock() = 0;
     virtual void unlock() = 0;
  protected:
     Excludable() {}

     // It is normal to lock an Excludable before destroying.
     virtual ~Excludable() {}
  private:
     Excludable(const Excludable &);  // not copyable
     Excludable &operator=(const Excludable &);  // not copyable
  };

  // Use Exclusion to lock an Excludable for a particular thread.
  // Construction locks; destruction unlocks.
  class Exclusion {
  public:
     // Wait if necessary, then take over the Exclusion
     // and exclude other threads.
     Exclusion(Excludable &, const char *reason = NULL);

     // Free the mutex.
     virtual ~Exclusion();
  private:
     Excludable &mutex;
     bool hasReason;
#ifdef SATHREAD_DEBUG
#if defined(__hpux) && defined(PTHREAD_DRAFT_4)
     string reason;
#else
     std::string reason;
#endif
#endif
  };

  class Mutex : public Excludable {
  public:
     Mutex();
     virtual ~Mutex();
     void lock();
     void unlock();
  private:
     friend class Monitor;
#ifdef WIN32_THREAD_IMPLEMENTATION
     int lockCount;  // to catch recursive locking
     HANDLE mutex;
#endif
#ifdef PTHREAD_THREAD_IMPLEMENTATION
     pthread_t lockId;
     bool isLocked;
     pthread_mutex_t mutex;
     pthread_mutex_t idGuard;
#endif
  };

  class Monitor : public Excludable {
  public:
     // Wait for a notify, even if the monitor is not locked.
     // If it is locked, releases the monitor while waiting.
     void wait();

     // Make all waiting threads eligible to run.
     void notify();

     Monitor();
     virtual ~Monitor();
     void lock();
     void unlock();
  private:
#ifdef WIN32_THREAD_IMPLEMENTATION
     HANDLE mutex;
     HANDLE event;
     Mutex idGuard;
     unsigned long lockId;
#endif
#ifdef PTHREAD_THREAD_IMPLEMENTATION
     Mutex mutex;
     pthread_cond_t cond;
#ifdef PTHREAD_DRAFT_4
     pthread_condattr_t condAttr;
#endif
     pthread_mutex_t altMutex; // fine-grained control
     pthread_cond_t altCond;   // associated with altMutex
#if !BROADCAST_TO_0
     int waiterCount;
     int altWaiterCount;
#endif
#endif
  };

  // To wait for a Thread's termination, use Exclusion or lock.
  // A Thread object behaves as both a thread handle and a Mutex.
  class Thread : public Excludable {
  public:
     Thread(const char *kind = NULL);
     virtual ~Thread();

     // Start a thread, indicating a procedure for the thread to execute.
     // The thread_proc object must persist until the thread terminates.
     // The new thread implicitly holds a lock on this Thread object until
     // the thread terminates.
     // Note that 'start' requires the Thread's lock and so may block.
     void start(Procedure &thread_proc);

     void lock();
     void unlock();
  private:
#ifdef WIN32_THREAD_IMPLEMENTATION
     unsigned long locking_thread;
#endif
#ifdef PTHREAD_THREAD_IMPLEMENTATION
     pthread_t locking_thread;
#endif

     CountedMonitor *guardp;
     bool isLocked;
     bool isThread;
     bool has_kind;
#ifdef SATHREAD_DEBUG
#if defined(__hpux) && defined(PTHREAD_DRAFT_4)
     string kind;
#else
     std::string kind;
#endif
#endif
  };
}

#endif // SATHREADS_H

