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
// gtCallbackMgrXm.h.C
//------------------------------------------
// synopsis:
// 
// Callback manager for GT objects.
//------------------------------------------
 
// INCLUDE FILES

#include <cLibraryFunctions.h>
#include <msg.h>
#include <gtCallbackMgrXm.h>
#ifndef ISO_CPP_HEADERS
#include <stdlib.h>
#include <stdio.h>
#else /* ISO_CPP_HEADERS */
#include <cstdlib>
using namespace std;
#include <cstdio>
#endif /* ISO_CPP_HEADERS */
#include <psetmem.h>
#include <gtBaseXm.h>
#include <machdep.h>


// VARIABLE DEFINITIONS

static Widget gt_callback_parent;
static int    callback_debug;

// DEBUGGING FUNCTION DEFINITIONS

extern "C" void enable_callback_debug(int e)
{ 
  callback_debug = e; 
}

extern "C" void enable_verbose_push_button_feedback(int e)
{ 
  enable_callback_debug(e); 
}

//////////////////////////////////////////////////////////////////////////
//									//	
//			      gtCallbackXm				//
//									//	
//////////////////////////////////////////////////////////////////////////

static void parent_destroyed(Widget, XtPointer, XtPointer);

static void add_parent_destroyed_callback(Widget w)
{
  while(w && !XtIsShell(w))
    w = XtParent(w);
  
  if (gt_callback_parent = w)
    XtAddCallback(gt_callback_parent, XmNdestroyCallback, parent_destroyed, NULL);
}

static void rem_parent_destroyed_callback()
{
  if(gt_callback_parent)
    {
      XtRemoveCallback(gt_callback_parent, XmNdestroyCallback, parent_destroyed, NULL);
      gt_callback_parent = NULL;
    }
}

static void parent_destroyed(Widget w, XtPointer, XtPointer)
{
  if(gt_callback_parent == w)
    add_parent_destroyed_callback(XtParent(w));
}

void gtCallbackXm::handler(Widget w, gtCallbackXm *callback, XmAnyCallbackStruct *xmcb)
{
  gtReasonXm rea(xmcb->reason);
  
  if(callback_debug)
    msg("CALLBACK $1: $2") << callback->name << eoarg << callback->gt_base->rep()->pathname() << eom;
  
  add_parent_destroyed_callback(w);
  
  callback->manager->set_xmcallback_data(xmcb);
  (*(callback->function))(callback->gt_base, xmcb->event, callback->data, rea.reason());
  callback->manager->set_xmcallback_data(0);
  
  rem_parent_destroyed_callback();
}

//////////////////////////////////////////////////////////////////////////
//									//	
//			      gtCallbackMgrXm				//
//									//	
//////////////////////////////////////////////////////////////////////////

gtCallbackMgrXm::gtCallbackMgrXm()
    : callback_array(NULL), count(0), xmcallback_data(0)
{
}


gtCallbackMgrXm::~gtCallbackMgrXm()
{
  if(count)
    OSapi_free(callback_array);
}

void gtCallbackMgrXm::add_callback(const char* name, gtBase* gt_base, gtCallbackFunc function, void* data, gtRep* rep)
{
  if(name && gt_base && function)
    {
      gtCallbackXm* new_array =
	(gtCallbackXm*)psetmalloc((count + 1) * sizeof(gtCallbackXm));
      if(count)
	{
	  OS_dependent::bcopy(callback_array, new_array, count * sizeof(gtCallbackXm));
	  for(int i = 0; i < count; ++i)
	    {
	      callback_array[i].remove();
	      new_array[i].add();
	    }
	  OSapi_free(callback_array);
	}
      callback_array = new_array;
      
      if(rep == NULL)
	rep = gt_base->rep();
      
      gtCallbackXm& callback = callback_array[count++];
      callback.gt_base  = gt_base;
      callback.function = function;
      callback.data     = data;
      callback.rep      = rep;
      callback.name     = name;
      callback.manager  = this;
      
      callback.add();
    }
}

