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

// time_log
//------------------------------------------
// synopsis:
// execution time logging and analysis declarations
//
// description:
// This file includes declarations and inline member function definitions
// for the execution time logging facility.  Class time_log is
// instantiated once for each entry or exit of a metered function.
// Class call_trace provides the type for an object that does the
// time_log instantiation (i.e., each metered function should have an
// auto object of type call_trace as its first executable statement).
// The dump_time_log_statistics entry accumulates all execution time
// statistics both globally and locally on a per-function basis and
// writes the result of the analysis to a file named "execution_times"
// in the current directory.
// Restrictions:
// ...
//------------------------------------------

#ifndef _time_log_h
#define _time_log_h

// include files
#ifndef _general_h
#include "general.h"
#endif

#include <machdep.h>


//------------------------------------------
// Time structure (32 bits): represents the time with microsecond
// resolution and total duration of up to 2048 seconds (about 34
// minutes).  One bit is used to distinguish entries from exits.
//------------------------------------------

struct entry_exit_time {
   unsigned usec : 20;
   unsigned sec : 11;
   unsigned is_entry : 1;
};

//------------------------------------------
// time_log class (8 bytes): one instance per entry or exit from a
// metered function.  Entries are batched into chunks by the
// operator new() member function.  Each entry contains a pointer
// to the name of the function for which it was created and a
// time stamp.
//------------------------------------------

class time_log {
   friend class time_log_iter;

public:
   time_log(const char* fcn_name, bool is_entry);
   inline void* operator new(size_t);
   void operator delete(void*) { }
   static void init();
   const char* get_name() { return nm; }
   entry_exit_time get_time() { return tim; }

private:
   union {
      const char* nm;
      time_log* next_chunk;
   };
   union {
      entry_exit_time tim;
      time_log* next_entry;
   };

   static time_log* head;
   static time_log* tail;

   enum { ENTRIES_PER_CHUNK = 4096 };
                      
   //   time_log() { }    // only for use by operator new()
   static void get_new_chunk();
};


//------------------------------------------
// time_log constructor: simply stores the function name, the current
// time, and whether this is an entry or exit.
//------------------------------------------

inline time_log::time_log(const char* fcn_name, bool is_entry): nm(fcn_name) {
  long usec, sec;
   OSapi_time(&usec, &sec);
   tim.usec = usec;
   tim.sec = sec;
   tim.is_entry = is_entry;
}

//------------------------------------------
// time_log's operator new(): returns the next slot in the current chunk
// or gets a new chunk if needed.
//------------------------------------------

inline void* time_log::operator new(size_t) {
   if (!tail || tail->next_entry >= tail + ENTRIES_PER_CHUNK)
      get_new_chunk();
   return tail->next_entry++;
}

//------------------------------------------
// Class call_trace -- should be instantiated as an auto object as the
// first action of each traced function.
//------------------------------------------

class call_trace {
public:
   call_trace(const char* fcn_name);
   ~call_trace();
   static void start_logging() { logging = true; }
   static void stop_logging() { logging = false; }
   
//private:
   static bool logging;
   const char* nm;
};

//------------------------------------------
// call_trace constructor -- simply adds a new entry to the log
//------------------------------------------

inline call_trace::call_trace(const char* fcn_name): nm(fcn_name) {
   if (logging)
      new time_log(fcn_name, true);
}

//------------------------------------------
// call_trace destructor -- ditto
//------------------------------------------

inline call_trace::~call_trace() {
   if (logging)
      new time_log(nm, false);
}

//------------------------------------------
// Class time_log_iter: steps through the log entries one at a time.
//------------------------------------------

class time_log_iter {
public:
   time_log_iter();
   time_log* next();
   
private:
   time_log* chunkp;
   time_log* entryp;
};

//------------------------------------------
// Analysis function declaration
//------------------------------------------

extern void dump_time_log_statistics(const char* root_fcn = NULL);

#endif // _time_log_h
/*
//------------------------------------------
// $Log: time_log.h  $
// Revision 1.3 1996/04/01 12:10:13EST mg 
// cleanup
 * Revision 1.2.1.4  1994/02/08  15:21:50  builder
 * Port
 *
 * Revision 1.2.1.3  1993/04/18  01:34:08  wmm
 * Allow metering to be dumped for a subtree of the call tree.
 *
 * Revision 1.2.1.2  1992/10/09  18:21:04  kol
 * moved &Log
 *
 * 
 * Revision 1.2  92/10/07  20:41:04  smit
 * *** empty log message ***
 * 
 * Revision 1.1  92/10/07  18:20:24  smit
 * Initial revision
 * 
 * Revision 1.1  92/10/07  17:56:03  smit
 * Initial revision
 * 
//Revision 1.7  92/09/08  14:30:31  mg
//struct timezone
//
//Revision 1.6  92/03/24  17:30:07  glenn
//Update include flag macros.
//
//Revision 1.5  92/03/18  15:57:46  mfurman
// *** empty log message ***
//
//Revision 1.4  92/02/11  16:42:24  wmm
//Protect against multiple definition when included after sys/time.h has been
//included.
//
//Revision 1.3  92/02/11  15:35:55  wmm
//Add size_t typedef (again, can't just include stddef.h because of nesting
//limitations).
//
//Revision 1.2  92/02/11  14:06:39  wmm
//Kludge around nesting level limit.
//
//Revision 1.1  92/02/08  12:48:36  wmm
//Initial revision
//
//
//------------------------------------------
*/
