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
/*****************************************************************************
 * sash.C
 * written by: Nate Kushman
 * 
*****************************************************************************/


#include <vport.h>
#include vdialogHEADER
#include vdragHEADER
#include vcontainerHEADER
#include vstdioHEADER
#include vstdlibHEADER
#include vapplicationHEADER
#include vliststrHEADER
#include vmenubarHEADER
#include vmenuHEADER

#include <ggeneric.h>
#include <gdialog.h>
#include <gsash.h>
#include <gview.h>
#include <gviewer.h>
#include <ginterp.h>
#include <ggeneric.h>
#include <gdraw.h>
#include <gglobalFuncs.h>

/*****************************************************************************
 * Sash

*****************************************************************************/


// definitions for overrides from vloadable
vkindTYPED_FULL_DEFINITION(Sash, vdialogItem, NULL);

const int SASH_INSET = 10;
const int SASH_WIDTH = 10;
const int BORDER_THICKNESS = 2;
const int INSET = 1;


/*Constructor takes in an initial screen placement rect, a dialog box which it
  appends itself to, a SashType (horizontal or vertical), and its owner view 
  (which is either the one below it, or to its right)*/


Sash::Sash (vrect *newRect,Pane* container, SashType newType, View
	    *newOwner) 
{    
  gInit (Sash::Sash);
  SetRect(newRect);
  container->AppendItem (this);
  myType = newType;
  myOwner = newOwner; 
  
  SetForeground(Viewer::GetSashForeground() );
  SetBackground(Viewer::GetSashBackground() );

  vresource res;
  if (myType==Horizontal) {
    getResource ("Cursors:HorzPaneResize", &res);
  } else {
     getResource ("Cursors:VertPaneResize", &res);
  }
  vcursor* cursor = new vcursor (res);
  cursor->SetHot(8,8);
  SetCursorOwned(cursor);
}


void Sash::Draw()
{
    gInit(Sash::Draw);
    vdraw::GSave();
    DrawDevice();
    // Draw a Horizontal sash.
    if (myType == Horizontal) {   
	int borderGap = (GetWidth() - SASH_WIDTH)/2;
	
	vdraw::SetLineWidth(1);
	hotSpotRect_.Set(GetX(), GetY(), GetWidth(), GetHeight());
	vdraw::RectStrokeInside(GetX() + GetWidth()/2, GetY(), 1, 
				GetHeight());
    // Draw a Verticle sash.
    } else  {  
	int borderGap = (GetHeight() - SASH_WIDTH)/2;
	
	vdraw::SetLineWidth(1);
	hotSpotRect_.Set(GetX(), GetY(), GetWidth(), GetHeight());
	vdraw::RectStrokeInside(GetX(), GetY() + GetHeight()/2, 
				GetWidth(), 1);
    }

    if ((myOwner->GetChildView() == NULL) &&
        ((myOwner->GetLeftView() == NULL) ||
        (myOwner->GetLeftView()->GetChildView() == NULL)) )
	hotSpotRect_.Inset(&hotSpotRect_, INSET, INSET);
    
    Draw3DTopLeft(&hotSpotRect_, BORDER_THICKNESS, 
    	  DetermineColor(vdrawCOLOR_SHADOW_TOP));
    Draw3DBottomRight(&hotSpotRect_, BORDER_THICKNESS, 
        DetermineColor(vdrawCOLOR_SHADOW_BOTTOM));
    Draw3DCenter(&hotSpotRect_, BORDER_THICKNESS, GetBackground());
    vdraw::GRestore();
}


//function to Set the view that a Sash belongs to
void Sash::SetOwner (View *newOwner)
{
    gInit (Sash::SetOwner);
    myOwner=newOwner;
}

//function to find out what view a sash belongs to

View *Sash::GetOwner()
{
    gInit (Sash::GetOwner);
    return myOwner;
}


/*Used for dragging sashes, creates an instance of a sashdrag, which takes care
  of the rest of the drag*/


int Sash::HandleButtonDown(vevent *event) 
{
    gInit (Sash::HandleButtonDown);
    
    // If the event binding is not drag then let super class handle it
    if (event->GetBinding() != vname_Select)
	return vdialogItem::HandleButtonDown(event);

    if (hotSpotRect_.ContainsPoint(event->GetX(), event->GetY()))
      {
	// Get image to drag
	const vrect *r = GetRect();
	
	// Create drag and initialize
	SashDrag *drag;
	
	if (myType == Vertical)
	  drag = new SashDragVert;
	else drag = new SashDragHoriz;
	
	drag->SetSash(this);
	
	/*Maxsize is the area that gets dragged around, in case you want to drag
	  an object and 10 pixels on each side or something*/

	if (myType == Vertical) {
	  drag->SetMaxSize(r->GetWidth(), 5);
	  drag->SetOffset(event->GetX(),2);
	}
	else  {
	  drag->SetMaxSize(5, r->GetHeight());
	  drag->SetOffset(2, event->GetY());
	}
	drag->Start(event);
	
	//return vTRUE because it handled whatever we needed it to
      }

      return vdialogItem::HandleButtonDown (event);
}


/*SetX and SetY are called by ObserveDrag, which is called whenever a dragged
  sash is dropped.  They can be called from any side.  If called from the left
  (top), it means that either the sash in question was the one moved or one to
  the left (top) was moved far enough, causing this one to move. The function
  asks its owner what its boundaries are, and then adjustes its rect, its 
  owners rect, and the the rect of the view to the left accordingly adjusting 
  for SASH_SIZES*/

void Sash::SetX(int newX, sideType side) 
{
    gInit (Sash::SetX);
    // Convert from absolute screen coords into relative window coords.
    newX = newX - myOwner->GetParentView()->GetAbsoluteX();
    vrect myRect;
    myRect = *(GetRect());
    if (side==gside_left) {
	newX= myOwner->GetDraggedX(newX, myRect.GetX());
	myOwner->SetLeft(newX + SASH_SIZE);
	if(myOwner->GetLeftView()->GetSash())
	    myOwner->GetLeftView()->GetSash()->SetX(
                     newX + myOwner->GetParentView()->GetAbsoluteX(), gside_right);
	else
	    myOwner->GetLeftView()->SetRight(newX);
	myRect.SetX(newX);
	SetRect(&myRect);
    } else {
	newX= myOwner->GetDraggedX(newX, myRect.GetX(), gside_right);
	if (newX<myRect.GetX() + SASH_SIZE) {
	    myOwner->SetRight(newX);

	    if(myOwner->GetLeftView()->GetSash())
	      myOwner->GetLeftView()->GetSash()->SetX(newX - SASH_SIZE +
              myOwner->GetParentView()->GetAbsoluteX(), gside_right);
	    else
	      myOwner->GetLeftView()->SetRight(newX - SASH_SIZE );
	    myRect.SetX(newX);
	    SetRect (&myRect);
	}
	else
	    myOwner->SetRight(newX);
    }
}

/*
 * Sash::SetY()
 * Explained in comment above
 */

void Sash::SetY(int newY, sideType side) 
{
    gInit (Sash::SetY);
    // Convert from absolute screen coords into relative window coords.
    newY = newY - myOwner->GetParentView()->GetAbsoluteY();
    vrect myRect;
    myRect = *(GetRect());
    if (side==gside_top) {
        int oldY = myRect.GetY() + myRect.GetHeight();
	newY= myOwner->GetDraggedY(newY, myRect.GetY());
	if ((myOwner->GetLeftView()->GetSash())  &&
            (newY > myOwner->GetLeftView()->GetY() + myOwner->GetLeftView()->GetHeight()))
	    myOwner->GetLeftView()->GetSash()->SetY(
                     newY+SASH_SIZE + myOwner->GetParentView()->GetAbsoluteY(), gside_bottom);
	else 
	    myOwner->GetLeftView()->SetBottom(newY + SASH_SIZE);
	myOwner->SetTop(newY);
	myRect.SetY (newY);
	SetRect (&myRect);
    } else {
	newY= myOwner->GetDraggedY(newY, myRect.GetY(), gside_bottom);
      if (newY>myRect.GetY()) {
	  if(myOwner->GetLeftView()->GetSash())
	      myOwner->GetLeftView()->GetSash()->SetY(
                  newY+SASH_SIZE+myOwner->GetParentView()->GetAbsoluteY(),
                  gside_bottom);
	  else
	      myOwner->GetLeftView()->SetBottom(newY + SASH_SIZE);
	  myOwner->SetBottom(newY);
	  myRect.SetY(newY);
	  SetRect (&myRect);
      }
      else
	  myOwner->SetTop(newY);
  }
}


/* All Sash set(whatever side) functions are called to change the length of the
   sash, never to change its position.  Therefore, SetRight and SetLeft are
   called only on vertical Sashes, and SetBottom and SetTop are called only on
   horizontal sashes (horizontal and vertical are explained in view.C).  All the
   functions really do is explicitly set whatever side to be whatever position*/

void Sash::SetRight(int newX)
{
    gInit (Sash::SetRight);
    vrect myRect;
    myRect = * (GetRect());
    if (newX<myRect.GetX())
    {
	myRect.SetX(newX);
	myRect.SetWidth(0);
    }
    else 
	myRect.SetWidth(newX - myRect.GetX());
    SetRect (&myRect);
}

void Sash::SetLeft(int newX)
{
    gInit (Sash::SetLeft);
    vrect myRect;
    myRect = * (GetRect());

    if (myType == Vertical)
      {
	myRect.SetWidth(myRect.GetWidth() - newX);
      }
    else
      {
	if (newX > myRect.GetX()+myRect.GetWidth())
	  myRect.SetWidth(0);
	else
	  myRect.SetWidth(myRect.GetWidth()-newX+myRect.GetX());
	myRect.SetX(newX);
      }
    SetRect (&myRect);
}

void Sash::SetBottom(int newY)
{
    gInit (Sash::SetBottom);
    vrect myRect;
    myRect = * (GetRect());
    if (newY>myRect.GetY()+myRect.GetHeight())
    {
	myRect.SetY(newY);
      myRect.SetHeight(0);
    }
    else
    {
	myRect.SetHeight(myRect.GetY()+myRect.GetHeight()-newY);
	myRect.SetY(newY);
    }
    SetRect (&myRect);
}

void Sash::SetTop(int newY)
{
    gInit (Sash::SetTop);
    vrect myRect;
    myRect = * (GetRect());
    if (myType == Horizontal)
      myRect.SetHeight(myRect.GetHeight() - newY);
    else
      if (newY < myRect.GetY())
	{
	  myRect.SetHeight(0);
	  myRect.SetY(newY);
	}
      else
	myRect.SetHeight(newY-myRect.GetY());
    SetRect (&myRect);
}


void Sash::MoveToX(int newX)
{
  gInit (Sash::MoveToX);
  vrect myRect = * (GetRect());
  myRect.SetX(newX);
  SetRect(&myRect);
}


void Sash::MoveToY(int newY)
{
  gInit (Sash::MoveToY);
  vrect myRect = * (GetRect());
  myRect.SetY(newY);
  SetRect(&myRect);
}




/*****************************************************************************
 * SashDrag
 *
 * There is a parent SashDrag class, and two children, SashDragVert, and
 * SashDragHoriz.  The two children contain exactly the same funcitions, with
 * slight modifications, usually exchanging X coordinates for Y coordinates
 *
*****************************************************************************/

vkindTYPED_FULL_DEFINITION(SashDrag, vdrag, "SashDrag");


/*Overrides vdrags function, calling itjust calls it and setting  the sashdrags
  private item (which is the sash it belongs to) to NULL*/


//function to Set the sash that a SashDrag belongs to
void SashDrag::SetSash (Sash* newSash) 
{ 
    gInit (Sash::SetSash);
    item = newSash;
    const vrect* r = item->GetRect();
    vdialogItemList* il = item->GetItemListIn();
    il->TransformPoint (r->GetX(), r->GetY(), &x_, &y_);
 
}

/*
void SashDrag::Init() 
{
    gInit (SashDrag::Init);    

    vdrag::Init();
    
    item = NULL; 
}
*/


//Galaxy Macro for loading from resource file

vkindTYPED_FULL_DEFINITION(SashDragVert, SashDrag, "SashDragVert");
vkindTYPED_FULL_DEFINITION(SashDragHoriz, SashDrag, "SashDragHoriz");



/*The ObserveDrag functions are called when the button is let up from a drag.
  The check to see if the initial window, and the drop window are the same
  insures that the drag will only be moved if the mouse remains in the window.
  Using DeleteLater allows any recursive references to it before deleting it*/

void SashDragVert::ObserveDrag(vwindow *origin, vwindow *to, vevent *event) 
{
    gInit (SashDragVert::ObserveDrag);
    if (to == origin) {
	item->SetY(event->GetY() - item->GetHeight()/2, gside_top);
	item->Draw();
    }

    DeleteLater(); 
    vdrag::ObserveDrag (origin, to, event);
}


void SashDragHoriz::ObserveDrag(vwindow *origin, vwindow *to, vevent
				    *event) 
{
    gInit (SashDragHoriz::ObserveDrag);
    if (origin == to) { 
	item->SetX(event->GetX() - item->GetWidth()/2, gside_left);
	item->Draw();
    }
    
    DeleteLater(); 
    vdrag::ObserveDrag (origin, to, event);
}

int SashDragVert::HandleDetail (vwindow* src, vwindow* dst, vevent* event)
{
   SetOffset((event->GetX() - x_), 2);
 	
   return vdrag::HandleDetail (src, dst, event);
}

int SashDragHoriz::HandleDetail (vwindow* src, vwindow* dst, vevent* event)
{
    SetOffset(2, (event->GetY() - y_));

    return vdrag::HandleDetail (src, dst, event);
}

/*The draw functions dictate what the dragged implemetation of the Sash will 
  look like as it is dragged across the screen.  Temporarely it is a rectangle,
  but in the future, it will be changed to just a line.  The function, saves the
  graphics state, draws the rect, with the color that is set, and then restores 
  the graphics state, and since it is done so fast, it looks like it is being 
  draw into the current graphics state (but if it actually was drawn into the 
  current graphics state, then it would leave ghosts wherever it went*/

void SashDragVert::Draw(vrect *rect) 
{
    gInit (SashDragVert::Draw);
    vdraw::GSave();
    vdraw::SetLineWidth(1);
    vdraw::MoveTo (rect->GetX(), rect->GetY()+1);
    vdraw::LineTo (rect->GetX()+rect->GetWidth(), rect->GetY()+1);
	vdraw::Stroke();

	vdraw::MoveTo (rect->GetX(), rect->GetY()+5);
   	vdraw::LineTo (rect->GetX()+rect->GetWidth(), rect->GetY()+5);
	vdraw::Stroke();  

    vdraw::GRestore(); 
}


void SashDragHoriz::Draw(vrect *rect) 
    
{
    gInit (SashDragHoriz::Draw);
    vdraw::GSave();
    vdraw::SetLineWidth(1);

    vdraw::SetColor(item->DetermineColor(vdrawCOLOR_FOREGROUND));
    vdraw::MoveTo (rect->GetX(), rect->GetY());
    vdraw::LineTo (rect->GetX(), rect->GetY()+rect->GetHeight());
    vdraw::Stroke();

    vdraw::MoveTo (rect->GetX()+4, rect->GetY());
    vdraw::LineTo (rect->GetX()+4, rect->GetY()+rect->GetHeight());
    vdraw::Stroke();

    vdraw::GRestore(); 
}
