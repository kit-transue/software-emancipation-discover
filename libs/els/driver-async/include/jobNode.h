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
// This code is a -*- C++ -*- header file

#ifndef _jobNode_h
#define _jobNode_h

//++ADS
class parray;
//--ADS

class jobNode
{
public:

  // public constants
  enum syncType { SYNC, ASYNC };
  enum runLevel { UNSTARTED, RUNNING, FINISHED };

  // Constructor(s) / Destructor(s)
  jobNode(jobNode *parent);
  virtual ~jobNode(void);

  // Public Interface
  int doJob(syncType);
  int allDone(void);

protected:

  // callback function typedef
  typedef int (*jobCallback)(jobNode *);

  // pointer to callback function in derived classes
  jobCallback callback;

  // flag to say wheterh this job is sync or async
  syncType syncStatus;

  // member functions to update run status
  int thisFinished(void);      // called when jobNode's own job is done
  int reportCompletion(void);  // called when jobNode's own job is done and all children are done
  
  // member function to test run status
  int childrenDone(void);
  int myJobDone(void);

  // member functions to add new subjobs and remove finished subjobs
  int addChild(jobNode *);
  int removeChild(jobNode *);

  // member callbacks to inform parent that a child has completed
  int childFinished(jobNode *);

  // member functions to get flags
  int getErrorStatus(void);
  
private:

  // status flags
  runLevel runStatus;
  int      errorStatus;   
  int      finishedWithMyJob;

  // parent/child relationships
  jobNode   *parent;
  parray    *workingChildren;   // will use as array of jobNode *'s to children
  parray    *finishedChildren;  // will use as array of jobNode *'s to children
};

#endif 
// _jobNode_h

