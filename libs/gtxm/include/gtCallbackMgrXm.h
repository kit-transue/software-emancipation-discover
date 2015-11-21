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
#ifndef _gtCallbackMgrXm_h
#define _gtCallbackMgrXm_h

// gtCallbackMgrXm.h
//------------------------------------------
// synopsis:
// 
// Callback manager for GT objects.
//------------------------------------------

//++X11
#include <x11_intrinsic.h>
#include <Xm/Xm.h>
//--X11

//++GT
#include <gt.h>
#include <gtBaseXm.h>
//--GT


////////////////////////////////////////////////////////////////////////////////

// typedef of generic callback function

typedef void  (*gtCallbackFunc)(gtBase*, gtEventPtr, void*, gtReason);

////////////////////////////////////////////////////////////////////////////////

// callback debugging routines

extern "C"
{
  void enable_callback_debug(int);
  void enable_verbose_push_button_feedback(int);
}

////////////////////////////////////////////////////////////////////////////////

class gtCallbackMgrXm;

class gtCallbackXm
{
public:
  inline gtCallbackXm(void);
  inline ~gtCallbackXm(void);

public:  
  inline void add(void);
  inline void remove(void);
  static void handler(Widget w, gtCallbackXm* callback, XmAnyCallbackStruct* xmcb);

public:
  gtCallbackFunc    function;
  gtBase           *gt_base;
  void             *data;
  gtRep            *rep;
  const char       *name;
  gtCallbackMgrXm  *manager;
};

gtCallbackXm::gtCallbackXm(void)
{
}

gtCallbackXm::~gtCallbackXm(void)
{
}

void gtCallbackXm::add(void)
{
    XtAddCallback(rep->widget(), name, XtCallbackProc(handler), XtPointer(this));
}


void gtCallbackXm::remove(void)
{
    XtRemoveCallback(rep->widget(), name, XtCallbackProc(handler), XtPointer(this));
}

////////////////////////////////////////////////////////////////////////////////

class gtCallbackMgrXm
{
public:
  void                        add_callback(const char* name, gtBase*, gtCallbackFunc, void* data, gtRep* = NULL);
  inline void                 set_xmcallback_data(XmAnyCallbackStruct *);
  inline XmAnyCallbackStruct *get_xmcallback_data(void);

protected:
  gtCallbackMgrXm(void);
  ~gtCallbackMgrXm(void);
  
private:
  unsigned short       count;
  gtCallbackXm        *callback_array;
  XmAnyCallbackStruct *xmcallback_data;
};

void gtCallbackMgrXm::set_xmcallback_data(XmAnyCallbackStruct *xmcbs)
{
  xmcallback_data = xmcbs;
}

XmAnyCallbackStruct *gtCallbackMgrXm::get_xmcallback_data(void)
{
  return xmcallback_data;
}

#endif // _gtCallbackMgrXm_h

