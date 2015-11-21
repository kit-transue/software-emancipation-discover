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
#ifndef _LOGPRESENT_H
#define _LOGPRESENT_H

#include <list>
#include <vector>
#include "tptinterfaces.h"
#include "lpinterface.h"
#include "minixml.h"
#include "threads.h"
#include "minidom.h"

class MsgProcessor;

class MsgGroupIdNode {
public:
  void reset();
  MsgGroupIdNode *find_mgid_node(unsigned long);
  unsigned long get_mgid() { return msg_gid;}
  MsgGroupIdNode *get_parent() { return parent;}
  void put_severity(lp_msg_sev sev) { severity = sev;}
  lp_msg_sev get_severity() { return severity;}
  void add_new_mgid(MiniXML::DOMString& child, MiniXML::DOMString &child_name, MiniXML::DOMString& par);
  void add_new_mgid(unsigned long, const string &name, unsigned long, bool&);
  MsgGroupIdNode(unsigned long id, MsgGroupIdNode *parent, const string &name);
  MsgGroupIdNode();
  ~MsgGroupIdNode();
  const string &getName() const;
private:
  MsgGroupIdNode *parent;
  MsgGroupIdNode *first_child;
  MsgGroupIdNode *sibling;

  lp_msg_sev severity;
  unsigned long msg_gid;
  string name;
};

class TrackProgress {
public:
    float done();
    float incr_file_count();
    void initialize_counts(int);
    TrackProgress(unsigned short);
private:
    unsigned short percent;
    float current_percent;
    float current_file_count;
    float files_before_incr;
    float increments;
};

class ProgressMsgProcessor {
public:
  void reset(bool);
  void cleanup();
  bool get_percentage(MiniXML::DOMString&, unsigned short&, MsgProcessor*);
  int  extract_number_of_files(const char*);
  void initialize_parse_count(const char*);
  void initialize_update_count(const char*);
  void initialize_qa_count(const char*);
  ProgressMsgProcessor();
  ~ProgressMsgProcessor();
private:

  //keeps track of percentage done already
  float percent_done;
  
  //variables to hold percentages of some of the major processing steps 
  //in modelbuild
  unsigned short initialization;
  unsigned short flags_translation;
  TrackProgress  *parse_pass_track;
  TrackProgress  *update_pass_track;
  unsigned short finalize_pass;
  TrackProgress  *qa_calliper_track;
};

class TaskBreakdown;
class TaskSteps;

class TaskProgress {
public:
  TaskProgress(const MiniXML::DOMString &name, void *, void (*)(void *, double));
  bool process_progress_message(MBDriver::Message &, MiniXML::DOMString &);   // return whether handled
  bool process_breakdown_message(MBDriver::Message &);   // return whether handled
  void add_breakdown_or_steps(MBDriver::Message &);  // determined to be for this message
  ~TaskProgress();
  void update_progress(double); // Performs the callback if appropriate.
  MiniXML::DOMString get_name() const;
private:
  MiniXML::DOMString name;
  TaskBreakdown *breakdown;
  TaskSteps *steps;
  double current_progress;
  void *callback_object;
  void (*callback)(void *, double);
};

class TaskBreakdown;

class Subtask {
public:
    Subtask(MiniXML::DOMString name, TaskBreakdown &parent, long start, long end);
    TaskProgress task;
private:
    MiniXML::DOMString name;
    TaskBreakdown &parent;
    long start;
    long end;
    static void callback(void *obj, double val);
    void note_progress(double);
};

class TaskBreakdown {
public:
    TaskBreakdown(TaskProgress &, MBDriver::Message &);
    ~TaskBreakdown();
    void update(double val);
    long size;
    bool process_breakdown_message(MBDriver::Message &);   // return whether handled
    bool process_progress_message(MBDriver::Message &, MiniXML::DOMString &);   // return whether handled
private:
    TaskProgress &for_task;
    list<Subtask *> tasks;
};

class TaskSteps {
public:
    TaskSteps(const MiniXML::DOMString &name, TaskProgress &, MBDriver::Message &);
    bool process_breakdown_message(MBDriver::Message &);   // return whether handled
    bool process_progress_message(MBDriver::Message &, MiniXML::DOMString &);   // return whether handled
private:
    TaskProgress &for_task;
    long current;
    long expected;
    MiniXML::DOMString name;
};

class IdPool;
class MsgProcessor : public SAThreads::Procedure {
public:
  MsgProcessor(IdPool &idp) : idPool(idp), doneMessage(NULL), doneWhat(NULL), main_task(NULL) {}
  ~MsgProcessor() { delete main_task; }
  virtual void run();
  void processAllMessages(nTransport::Connection<MBDriver::Message> *conn);
  void reset();
private:
  void propagate_severity_up(MiniXML::DOMString&, MiniXML::DOMString&, MiniXML::DOMString&, vector < ::MBDriver::Message* > &);
  bool processOneMessage(MBDriver::Message*, vector < ::MBDriver::Message* > &);
  bool retrieveDoneMessage(vector < ::MBDriver::Message* > &);
  string substitute_arguments(MBDriver::Message*);
  ProgressMsgProcessor progressProcessor;
  unsigned short percent_done;
  MsgGroupIdNode mgid_tree_root;
  IdPool &idPool;  
  MBDriver::Message *doneMessage;
  MiniXML::DOMString *doneWhat;
  TaskProgress *main_task;
  static void progress_callback(void *, double);
  void note_progress(double);
};

class MessageList : public SAThreads::Monitor {
public:
  void addMessage(::MBDriver::Message *);
  ::MBDriver::Message* popFirstMessage();
  int size() { return messages.size(); }
  MessageList() {}
  ~MessageList() {}
private:
  list < ::MBDriver::Message* > messages;
};

class IdPool : public SAThreads::Mutex {
public:
  IdPool() : available(1) {}
  ~IdPool() {}
  unsigned long get();
private:
  unsigned long available;
};

#endif
