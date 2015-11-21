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
#ifndef _gpaneObjects_h
#define _gpaneObjects_h


#include <vport.h>
#include vlistitemHEADER
#include vbuttonHEADER
#include vtextitemHEADER
#include vtimerHEADER


class View;
class gscrollButton;
class gscrollButtonTimer;

/* A list of multiple items that can be loaded in from the vr file */

class glistItem : public vlistitem
{
  public:
    // the standard versions of the necessary constructors/destructor
    vkindTYPED_FULL_DECLARATION(glistItem);
    vloadableINLINE_CONSTRUCTORS(glistItem,vlistitem);
    void Initialize (vchar*, vchar*, View*);
    void ObserveSelect (int);
    
  private:
    vchar *selectCmd_, *activateCmd_;  // Storage for select and double-click actions
    View* owner_;          // The owner view
    int id_;               // the id
    static int UniqId_;    // a variable for the unique id system
};


class gbutton : public vbutton
{
  public:
    // the standard versions of the necessary constructors/destructor
    vkindTYPED_FULL_DECLARATION(gbutton);
    vloadableINLINE_CONSTRUCTORS(gbutton,vbutton);

    void Initialize(vchar*, View*);
    int HandleButtonUp(vevent*);

  private:
    vchar* activateCmd_;
    View* owner_;
};

#include <gcontrolObjects.h>

class filterButton : public gdButton
{
  public:
    // the standard versions of the necessary constructors/destructor
    vkindTYPED_FULL_DECLARATION(filterButton);
    vloadableINLINE_CONSTRUCTORS(filterButton,gdButton);
    
    void SetDisplay(int,int,int);
	void SetrtlClinet (int id) {rtlclientid = id;}
	virtual int  HandleButtonUp(vevent*);

  private:
	int rtlclientid;
};

class filterResetButton : public gdButton
{
  public:
    // the standard versions of the necessary constructors/destructor
    vkindTYPED_FULL_DECLARATION(filterResetButton);
    vloadableINLINE_CONSTRUCTORS(filterResetButton,gdButton);
    
	void SetrtlClient (int id) {rtlclientid = id;}
	virtual int  HandleButtonUp(vevent*);

  private:
	int rtlclientid;
};

class gmessageBox : public vtextitem
{
  public:
    // the standard versions of the necessary constructors/destructor
    vkindTYPED_FULL_DECLARATION(gmessageBox);
    vloadableINLINE_CONSTRUCTORS(gmessageBox,vtextitem);

    void Initialize(vchar*, View*);
    void DisplayQuickHelp(vchar*);
    void DisplayMessage(vchar*);
    void AddNewLine();
    virtual void Open();
    void ScrollMessageBox(vchar*);
    int HandleButtonDown(vevent*);

  private:
    int inQHelp_;
    int first_;
    vtextSelection* selection_;
    vtext* text_;
    vchar* tclCmd_;
    View* owner_;
};



class gscrollButton : public vbutton
{
  public:
    // the standard versions of the necessary constructors/destructor
    vkindTYPED_FULL_DECLARATION(gscrollButton);
    vloadableINLINE_CONSTRUCTORS(gscrollButton,vbutton);

    virtual void Open();
    void Destroy();
    void SetCommand(vchar*, gmessageBox*);
    virtual int HandleButtonUp(vevent*);
    virtual int HandleButtonDown(vevent*);
    void gActivate();

  private:
    vchar* command_;
    gmessageBox* owner_;
    gscrollButtonTimer* timer_;
};


class gscrollButtonTimer : public vtimer
{
  public:
    void SetOwner(gscrollButton*);
    void ObserveTimer();

  private:
    gscrollButton* owner_;
    vtimestamp startTime_, speedUpTime_;
    int hasAccelerated_;
};


#endif
