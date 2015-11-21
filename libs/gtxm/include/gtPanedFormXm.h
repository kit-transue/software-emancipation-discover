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
#ifndef _gtPanedFormXm_h
#define _gtPanedFormXm_h

// gtPanedFormXm.h
//------------------------------------------
// synopsis:
// 
//------------------------------------------

#include <gtBase.h>
#include <gtPanedForm.h>
#include <gtCallbackMgrXm.h>

#ifndef _genArr_h
#include <genArr.h>
#endif

class gtPanedFormXm;

typedef struct {
  gtBase        *gt_element;
  Widget        button;
  Widget        separator;
  int           width;
  float         rel_x;
  gtPanedFormXm *form;
  int            index;
} child_info, *child_infoPtr;

genArr(child_infoPtr);

class gtPanedFormXm : public gtPanedForm
{
  public:
    gtPanedFormXm(gtBase *parent, const char *name);
    ~gtPanedFormXm();

    void add_child(gtBase *, int width);
    void layout(void);

  private:
    gtForm                  *form;
    genArrOf(child_infoPtr) childs;
    int                     last_x;
    int                     last_height;
    int                     width;

    void   calculate_relative(void);
    static void map_callback(Widget W, void *data, XEvent *ev);
    static void start_dragging(Widget W, void *data, XEvent *ev);
    static void do_dragging(Widget W, void *data, XEvent *ev);
    static void end_dragging(Widget W, void *data, XEvent *ev);
    static void form_resize(Widget W, void *data, XEvent *ev);
};

#endif
