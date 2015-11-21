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
//++OSPORT
#include <pdumem.h>
#include <pdutime.h>
//--OSPORT

//++ADS
#include <parray.h>
//--ADS

//++ELS
#include <jobNode.h>
//--ELS

////////////////////////////////////////////////////////////////////////////////
//
// Constructor(s) / Destructor(s)
//
////////////////////////////////////////////////////////////////////////////////

jobNode::jobNode(jobNode *parentPtr)
{
  parent = parentPtr;

  workingChildren  = new parray(0);
  finishedChildren = new parray(0);

  runStatus   = UNSTARTED;
  errorStatus = 0;
}

jobNode::~jobNode(void)
{
  jobNode *child = 0;

  if ( workingChildren )
    {
      for ( int i = workingChildren->size() - 1; i >= 0; i-- )
	if ( (child = (jobNode *)(*workingChildren)[i]) != 0 )
	  FREE_OBJECT(child);
      FREE_OBJECT(workingChildren);
    }

  if ( finishedChildren )
    {
      for ( int i = finishedChildren->size() - 1; i >= 0; i-- )
	if ( (child = (jobNode *)(*finishedChildren)[i]) != 0 )
	  FREE_OBJECT(child);
      FREE_OBJECT(finishedChildren);
    }
}

////////////////////////////////////////////////////////////////////////////////
//
// Public members
//
////////////////////////////////////////////////////////////////////////////////

int jobNode::doJob(syncType sync)
{
  int retval = -1;

  // store the sync status
  syncStatus = sync;

  // set status
  errorStatus = 1;
  runStatus   = RUNNING;

  if ( callback )
    {
      // execute the job of this node
      if ( (retval = (*callback)(this)) < 0 )
	{
	  // if our job was in error, set error status
	  errorStatus = -1;
	}
    }

  // we do not want to return from doJob is sync is SYNC
  // until the flag has been set that this job has finished.
  // finishedWithMyJob is set in thisFinished, which will
  // either be called at the end of a callback for a job
  // that is *always* synchronous, or by some callback 
  // which will eventually call thisFinished().  All of this
  // assumes that pdsleep() can be interrupted for events
  // such as SIGPOLL, etc.
  if ( sync == SYNC )
    {
      while ( ! allDone() )
	pdsleep(1);
    }

  return retval;
}

int jobNode::allDone(void)
{
  return childrenDone() && myJobDone();
}

////////////////////////////////////////////////////////////////////////////////
//
// Protected Members
//
////////////////////////////////////////////////////////////////////////////////

// In some ways this is the most important member of this class.  It *must* be called
// at some point in the processing of this node's job, (preferable at the *end* of
// that job ;-) ) so that it knows when to fire off its children.  No children are
// executed until this node's job is complete.  If there is no job to perform, except
// a few processes that must go in parallel, simply have the job's callback create the
// child nodes, then call thisFinished().  It's job will be done, and the children will
// be fired off.
int jobNode::thisFinished(void)
{
  int retval = -1;

  finishedWithMyJob = 1;

  // OK, our job is done, now see if our job generated any children
  if ( workingChildren )
    {
      // for each child, fire them off
      int n = workingChildren->size();
      for ( int i = 0; i < n; i++ )
	{
	  // if we are in SYNC mode, this will process the children one after another
	  // if we are in ASYNC mode, this will fire off all the childrem at the same time
	  if ( ((jobNode *)(*workingChildren)[i])->doJob(syncStatus) < 0 )
	    {
	      errorStatus = -1;  // if child errors, we are in error
	    }
	}
    }
  
  // at this point we are not sure if we are all done or not, because the
  // children could be executing asynchronously.  This is why we test to
  // see here if we are all done, (i.e. all children are finished now) and
  // we also check for completion in the childFinished callback, because we
  // may not be done until the last asynchronous child has finished
  if ( allDone() )
    if ( reportCompletion() >= 0 )
      retval = 1;

  return retval;
}

int jobNode::getErrorStatus(void)
{
  return errorStatus;
}

int jobNode::childrenDone(void)
{
  int retval = 1;

  if ( workingChildren )
    if ( workingChildren->size() != 0 )
      retval = 0;

  return retval;
}

int jobNode::myJobDone(void)
{
  return ( finishedWithMyJob == 1 );
}

int jobNode::childFinished(jobNode *child)
{
  int retval = -1;

  if ( child )
    {
      if ( removeChild(child) >= 0 )  // remove from list of children waitin on
	if ( child->getErrorStatus() < 0 )    // if child was in error
	  errorStatus = -1;                // then this whole node is in error 
      
      retval = 1;
      
      if ( allDone() )
	reportCompletion();           // report that this node is completed
    }
  
  return retval;
}

int jobNode::reportCompletion(void)
{
  int retval = 0;

  // set run level
  runStatus = FINISHED;

  if ( parent )
    {
      if ( parent->childFinished(this) >= 0 )
	retval = 1;
      else
	retval = -1;
    }

  return retval;
}

int jobNode::addChild(jobNode *child)
{
  int retval = -1;

  if ( workingChildren )
    {
      if ( workingChildren->insert((void *)child) >= 0 )
	retval = 1;
    }

  return retval;
}

int jobNode::removeChild(jobNode *child)
{
  int retval = -1;
  int indx   = -1;

  if ( workingChildren )
    if ( (indx = workingChildren->find((void *)child)) >= 0 )
      if ( workingChildren->remove(indx) >= 0 )
	retval = 1;
  
  if ( finishedChildren )
    {
      if ( finishedChildren->insert((void *)child) < 0 )
	retval = -1;
    }
  else
    {
      retval = -1;
    }
  
  return retval;
}
