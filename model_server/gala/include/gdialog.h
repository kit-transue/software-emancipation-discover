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
#ifndef _gdialog_h
#define _gdialog_h

#include <vport.h>
#include vdialogHEADER
#include vcontainerHEADER


class Viewer;

class MyDialog : public vdialog
{
  public:
    // the standard versions of the necessary constructors/destructor
    vkindTYPED_FULL_DECLARATION(MyDialog);
    vloadableINLINE_CONSTRUCTORS(MyDialog, vdialog);
    virtual void ObserveWindow(vevent *event);
    virtual int HandleEnter(vevent *event);
    virtual int HandleLeave(vevent *event);
    virtual int HandleAcquire(vevent *event);
    virtual int HandleLose(vevent *event);
    virtual void Destroy();
    void SetViewer (Viewer*);
    void HandleContainer(vcontainer*, int, int);
    int HandleConfigure(vevent*);
    virtual void Draw();
    Viewer *GetViewer();
    virtual int HandleMotion(vevent *event);
    virtual void LoadInit(vresource);
		virtual int HookClose( vevent * );
    void DestroyTip();
    void DisplayTip();
  private:
    Viewer* owner_;
    int m_fHasFocus;
    vimage *m_pIconImage;
};

#endif
