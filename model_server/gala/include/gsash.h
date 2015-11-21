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
#ifndef _sash_h
#define _sash_h

#include <vport.h>
#include vdragHEADER
#include vbuttonHEADER


class View;
class Pane;
class MyDialog;

/*dictates the width of the sash*/
const int SASH_SIZE = 7;


/*each sash has a private SashType, that is either Vertical (width n and height
  SASH_SIZE) or Horizontal (height n and width SASH_SIZE)*/
typedef enum {
    Horizontal,
    Vertical
} SashType;

#ifdef NEW_UI

/*Used when calling sash spliting and resizeing functions, to dictate whether the
  call comes from the left of right, so the function will know whether to change
  the left or right side of the view*/
typedef enum
{
    gside_right,
    gside_left,
    gside_top,
    gside_bottom
} sideType;

#endif

/* Sashes are used between views and are used to resize and reorient views,
   currently they are vdialogLineItems, but as somepoint, that may change as
   we use our own look for a sash*/

class Sash : public vdialogItem
{
  public:
    Sash(vrect*, Pane*, SashType, View*);

    // the standard versions of the necessary constructors/destructor
    vkindTYPED_FULL_DECLARATION(Sash);
    vloadableINLINE_CONSTRUCTORS(Sash,vdialogItem)
    
    // virtual member function overrides
    virtual int HandleButtonDown(vevent *event);
    virtual void Draw();

#ifdef NEW_UI
    void SetY (int, sideType side=gside_top);
    void SetX (int, sideType side=gside_left);
#endif
    void SetTop (int);
    void SetBottom (int);
    void SetLeft (int);
    void SetRight (int);
    void MoveToX (int);
    void MoveToY (int);
    void SetOwner (View *newOwner);
    View *GetOwner();

  private:
    SashType myType;
    View *myOwner;
    vrect hotSpotRect_;
};


/*SashDrags are objects that are made with a sash is dragged and handle the
  drawing of the dragging object, and the calling of the necessary functions
  when the mouse button is let up and the drag ends.  There is a parent general
  SashDrag method, and two children SashDragHoriz, and SashDragVert which are
  almost identical except for the change for X to Y coordinates*/

class SashDrag : public vdrag
{
  public: 
    // vkindTyped
    vkindTYPED_FULL_DECLARATION(SashDrag);
    
    // the standard versions of the necessary constructor/destructor
    vkindTYPED_INLINE_DEFAULT_CONSTRUCTOR(SashDrag, vdrag);
    vkindTYPED_INLINE_SUPPRESS_CONSTRUCTOR(SashDrag, vdrag);
    vkindTYPED_INLINE_DESTRUCTOR(SashDrag, vdrag);

    // virtual member function override
    // virtual void Init();
    void SetSash (Sash* newSash);

  protected:
    Sash* item;   // a link to the sash being moved
    int x_, y_;
};

class SashDragVert : public SashDrag
{
  public: 
    // vkindTyped
    vkindTYPED_FULL_DECLARATION(SashDragVert);
    
    // the standard versions of the necessary constructor/destructor
    vkindTYPED_INLINE_DEFAULT_CONSTRUCTOR(SashDragVert, SashDrag);
    vkindTYPED_INLINE_SUPPRESS_CONSTRUCTOR(SashDragVert, SashDrag);
    vkindTYPED_INLINE_DESTRUCTOR(SashDragVert, SashDrag);

    // virtual member function override
    virtual void Draw(vrect *rect);
    virtual void ObserveDrag(vwindow *from, vwindow *to, vevent *event);
    virtual int HandleDetail (vwindow* from, vwindow* to, vevent* event);
};


class SashDragHoriz : public SashDrag
{
  public: 
    // vkindTyped
    vkindTYPED_FULL_DECLARATION(SashDragHoriz);
    
    // the standard versions of the necessary constructor/destructor
    vkindTYPED_INLINE_DEFAULT_CONSTRUCTOR(SashDragHoriz, SashDrag);
    vkindTYPED_INLINE_SUPPRESS_CONSTRUCTOR(SashDragHoriz, SashDrag);
    vkindTYPED_INLINE_DESTRUCTOR(SashDragHoriz, SashDrag);

    // virtual member function override
    virtual void Draw(vrect *rect);
    virtual void ObserveDrag(vwindow *from, vwindow *to, vevent *event);
    virtual int HandleDetail (vwindow* from, vwindow* to, vevent* event);
};

#endif
