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
// implementation of classes and functions in time_log.h
//
// description:
// Definitions of static data members and non-time-critical functions
// of the execution time logging facility.
//------------------------------------------
// Restrictions:
// ...
//------------------------------------------


#ifndef ISO_CPP_HEADERS
#include <stdio.h>
#include <string.h>
#else /* ISO_CPP_HEADERS */
#include <cstdio>
namespace std {};
using namespace std;
#include <cstring>
#endif /* ISO_CPP_HEADERS */

#include <cLibraryFunctions.h>
#include <msg.h>
#include "time_log.h"
#include "avl_tree.h"


//------------------------------------------
// time_log::init()
//------------------------------------------

void time_log::init() {
   if (head) {
      time_log* next;
      for (time_log* p = head->next_chunk; p; p = next) {
         next = p->next_chunk;
         delete p;
      }
      head->next_chunk = NULL;
      head->next_entry = head + 1;
      tail = head;
   }
   else get_new_chunk();
}

//------------------------------------------
// time_log::get_new_chunk()
//------------------------------------------

void time_log::get_new_chunk() {
   time_log* chunk = (time_log*) new char[ENTRIES_PER_CHUNK *
         sizeof(time_log)];
   chunk->next_chunk = NULL;
   chunk->next_entry = chunk + 1;
   if (tail)
      tail->next_chunk = chunk;
   else head = chunk;
   tail = chunk;
}

//------------------------------------------
// time_log_iter::time_log_iter()
//------------------------------------------

time_log_iter::time_log_iter() {
   chunkp = time_log::head;
   if (chunkp)
      entryp = chunkp + 1;
}

//------------------------------------------
// time_log_iter::next()
//------------------------------------------

time_log* time_log_iter::next() {
   if (!chunkp)
      return NULL;
   if (entryp >= chunkp->next_entry) {
      if (chunkp == time_log::tail)
         return NULL;
      chunkp = chunkp->next_chunk;
      entryp = chunkp + 1;
   }
   return entryp++;
}

//------------------------------------------
// static data definitions
//------------------------------------------

time_log* time_log::head;
time_log* time_log::tail;
bool call_trace::logging;

//------------------------------------------
// The following declarations are used in the analysis of trace data.
// Two data structures are used to process the raw event stream: an
// AVL tree with one entry per function appearing in the trace is used
// to accumulate execution time from each call, and a stack, parallelling
// the original call stack, is used to calculate the duration of each
// call and to separate time actually spent in the routine from time
// spent in its dynamic descendents.
//
// Recursion is handled by keeping track of the number of instances
// on the stack at any given time.  If the instance count is not zero
// after processing an exit event, the local time (i.e., the difference
// between entry and exit times, less the amount of time spent in called
// functions) is accumulated in a "recursion" category for that routine,
// and the total and local time accumulators are not touched.  When the
// "outermost" instance of that function exits, the recursion accumulator
// is added to the local time accumulator.
//------------------------------------------

//------------------------------------------
// Class time_stat_key is the sorting key for the AVL tree.
//------------------------------------------

class time_stat_key: public avl_key {
public:
   time_stat_key(const char* fcn_name) : nm(fcn_name) { }
   const char* name() const { return nm; }
private:
   const char* nm;
};

//------------------------------------------
// Class time_stat_node represent the function nodes in the AVL tree.
// It performs the accumulation procedure described above, using enter()
// and exit() member functions.  Because there will be many duplicate
// node creations (one for each invocation of a function after the
// first), a free-list is managed to reduce allocation overhead.
//------------------------------------------

class time_stat_node: public avl_node {
public:
   time_stat_node(avl_tree*, const time_stat_key&);
   void* operator new(size_t);
   void operator delete(void*);
   int compare(const avl_key&) const;
   time_stat_node* dup() { return (time_stat_node*) avl_node::dup(); }
   void enter();
   void exit(float elapsed_time, float children_time);
   long count() { return u.cnt; }
   float global_time() { return glob_time; }
   float local_time() { return loc_time; }
   const char* name() { return key.name(); }
   
private:
   static time_stat_node* freelist;
   union uu {
      uu(int i): cnt(i) { }
      long cnt;
      time_stat_node* next;    // freelist chain
   } u;
   time_stat_key key;
   int instance_count;        
   float glob_time;
   float recursive_time;
   float loc_time;
};                                          

//------------------------------------------
// time_stat_node::time_stat_node(avl_tree*, const time_stat_key&)
//------------------------------------------

time_stat_node::time_stat_node(avl_tree* treep, const time_stat_key& k):
      avl_node(treep, k), key(k), instance_count(0), u(0), glob_time(0.0),
      recursive_time(0.0), loc_time(0.0) { }
      
//------------------------------------------
// time_stat_node::operator new(size_t)
//------------------------------------------

void* time_stat_node::operator new(size_t sz) {
   if (freelist) {
      void* retval = freelist;
      freelist = freelist->u.next;
      return retval;
   }
   return new char[sz];
};

//------------------------------------------
// time_stat_node::operator delete(void*)
//------------------------------------------

void time_stat_node::operator delete(void* p) {
   ((time_stat_node*) p)->u.next = freelist;
   freelist = (time_stat_node*) p;
}

//------------------------------------------
// time_stat_node::compare(const avl_key&) const
//------------------------------------------

int time_stat_node::compare(const avl_key& k) const {
   int result = strcmp(((time_stat_key&) k).name(), key.name());
   if (result > 0)
      return 1;
   if (result < 0)
      return -1;
   return 0;
}

//------------------------------------------
// time_stat_node::enter()
//------------------------------------------

void time_stat_node::enter() {
   instance_count++;                                                     
   u.cnt++;
}

//------------------------------------------
// time_stat_node::exit(float, float)
//------------------------------------------

void time_stat_node::exit(float elapsed, float children) {
   if (--instance_count > 0) {
      recursive_time += elapsed - children;
   }
   else {
      glob_time += elapsed;
      loc_time += elapsed - children + recursive_time;
      recursive_time = 0.0;
   }
}

//------------------------------------------
// static data members
//------------------------------------------

time_stat_node* time_stat_node::freelist;

//------------------------------------------
// Class time_stat_iter is an iterator over the time_stat AVL tree providing
// convenience type-casting from the result of the plain avl_iterator.
//------------------------------------------

class time_stat_iter: public avl_iterator {
public:
   time_stat_iter(const avl_tree* t): avl_iterator(t) { }
   time_stat_node* next() { return (time_stat_node*) avl_iterator::next(); }
};

//------------------------------------------
// Class stack_entry represents an element on the log-processing call
// stack.  An entry is pushed on the stack by "new"ing it; the pop()
// member function processes the exit event and deletes the corresponding
// stack entry.
//------------------------------------------

class stack_entry {
public:
   stack_entry(avl_tree*, time_log*);
   void pop(time_log*);
   void* operator new(size_t);
   void operator delete(void*);
   static stack_entry* top() { return head; }
   const char* name() { return start->get_name(); }
private:
   static stack_entry* head;
   static stack_entry* freelist;
   stack_entry* next;
   time_log* start;
   time_stat_node* accum;
   float children_time;
};
                                            
//------------------------------------------
// The following routine calculates the difference (in seconds) between
// two time_log events, allowing for wraparound in the seconds fields
//------------------------------------------

static float time_diff(time_log* start, time_log* end) {
   entry_exit_time stime = start->get_time();
   entry_exit_time etime = end->get_time();
   float diff = int(etime.sec) - int(stime.sec) +
	 (int(etime.usec) - int(stime.usec)) / 1e6;
   if (diff < 0.0)
      diff += 2048.0;   // assumes 11-bit field for seconds
   diff += 43e-6;	// add fudge for pre-Initialize/post-Return overhead
   return diff;
}

//------------------------------------------
// stack_entry::stack_entry(avl_tree*, time_log*)
//------------------------------------------

stack_entry::stack_entry(avl_tree* treep, time_log* log): next(head),
      start(log), children_time(0.0) {
   head = this;
   time_stat_node* p = new time_stat_node(treep,
         time_stat_key(log->get_name()));
   if (p->dup()) {
      accum = p->dup();
      delete p;
   }
   else accum = p;
   accum->enter();
}

//------------------------------------------
// stack_entry::pop(time_log*)
//------------------------------------------

void stack_entry::pop(time_log* end) {
   float elapsed = time_diff(start, end);
   accum->exit(elapsed, children_time);
   if (next)
      next->children_time += elapsed;
   head = next;
   delete this;
}

//------------------------------------------
// stack_entry::operator new(size_t)
//------------------------------------------

void* stack_entry::operator new(size_t sz) {
   if (freelist) {
      void* retval = freelist;
      freelist = freelist->next;
      return retval;
   }
   return new char[sz];
}

//------------------------------------------
// stack_entry::operator delete(void*)
//------------------------------------------

void stack_entry::operator delete(void* p) {
   ((stack_entry*) p)->next = freelist;
   freelist = (stack_entry*) p;
}

//------------------------------------------
// Static data members
//------------------------------------------

stack_entry* stack_entry::head;
stack_entry* stack_entry::freelist;

//------------------------------------------
// dump_time_log_statistics()
//------------------------------------------
                           
void dump_time_log_statistics(const char* root_fcn) {
   avl_tree statistics;
   time_log_iter iter;

   if (root_fcn && !*root_fcn)
      root_fcn = NULL;
   int root_nesting = !root_fcn;   // if no root, set nesting to 1 to force all

   for (time_log* p = iter.next(); p; p = iter.next()) {
      if ((p->get_time()).is_entry) {
         if (root_fcn && strcmp(p->get_name(), root_fcn) == 0)
            root_nesting++;
         if (root_nesting > 0)
            new stack_entry(&statistics, p);
      }
      else {
         if (root_nesting) {
            while (stack_entry::top() && strcmp(stack_entry::top()->name(),
                  p->get_name())) {
               msg("Stack discipline violated: $1 exiting with $2 on top of stack") << p->get_name() << eoarg << stack_entry::top()->name() << eom;
               stack_entry::top()->pop(p);
            }
            if (stack_entry::top())
               stack_entry::top()->pop(p);
            else msg("Stack discipline violated: $1 exiting with stack empty") << p->get_name() << eom;
            if (root_fcn && strcmp(p->get_name(), root_fcn) == 0)
               root_nesting--;
         }
      }
   }
   
   time_stat_iter stat_iter(&statistics);
   size_t filename_len = 20;
   if (root_fcn)
      filename_len += strlen(root_fcn);
   char* filename = new char[filename_len];
   strcpy(filename, "execution_times");
   if (root_fcn) {
      strcat(filename, ".");
      strcat(filename, root_fcn);
   }
   FILE* out = OSapi_fopen(filename, "w");
   if (out) {
      for (time_stat_node* nd = stat_iter.next(); nd; nd = stat_iter.next()) {
         OSapi_fprintf(out, "%8.3f %8.3f %6ld   %s\n", nd->global_time(),
               nd->local_time(), nd->count(), nd->name());
      }
      OSapi_fclose(out);
   }
   else msg("Cannot open file $1") << filename << eom;
   delete [] filename;
}

// MG : functional interface to avoid deep nesting of include files

void call_time_log(const char * nm, bool flag)
{
  if(call_trace::logging)
    new time_log(nm, flag);
}

/*
// $Log: time_log.cxx  $
// Revision 1.7 2000/07/13 10:59:47EDT ktrans 
// move msg.h outside of ifdefs
Revision 1.2.1.4  1993/05/26  19:30:55  sergey
testing put.

Revision 1.2.1.3  1993/04/18  01:34:20  wmm
Allow metering to be dumped for a subtree of the call tree.

Revision 1.2.1.2  1992/10/09  18:17:26  kol
moved &Log


//Revision 1.5  92/09/12  13:22:00  wmm
//Add (relatively arbitrary, based on metering of an artificial test case on`
//an IPX) a fudge factor to the execution times of subroutines, to account for
//pre-Initialize/post-Return overhead.
//
//Revision 1.4  92/09/09  12:52:15  wmm
//Fix bug that sometimes prevented metering more than once per session.
//
//Revision 1.3  92/05/20  23:55:31  aharlap
//added const to time_stat_key::name
//
//Revision 1.2  92/02/12  11:25:14  wmm
//Fix potential crash in dumping execution time statistics.
//
//Revision 1.1  92/02/08  12:48:58  wmm
//Initial revision
//
//
//------------------------------------------
*/
