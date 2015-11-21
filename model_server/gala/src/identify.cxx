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
//Include just the Galaxy headers which are needed for this demo

#include <vport.h>
#include vstdlibHEADER
#include veventHEADER
#include vtimerHEADER
#include vmenuHEADER
#include vapplicationHEADER
#include vdialogHEADER
#include vmenubarHEADER
#include vbuttonHEADER
#include vcontainerHEADER
#include vstartupHEADER


#include "identify.h"
#include "tipwin.h"

// Simple macro to make our life easier
#define TAG(t) vnameInternGlobalLiteral(t)

////////////////////////////////////////////////////////////////////////
//// member functions of "positionTimer" subclass
////////////////////////////////////////////////////////////////////////

// default constructor
//
positionTimer::positionTimer()
{
  // the following is done after the parent class, vtimer, constructor
  // has been called

  Init();
}

// constructor with initialization parameters passed
//
positionTimer::positionTimer(vdialog *dialog,
			     vdialogLabelItem *labitem,
			     unsigned long secs,
			     unsigned long nanosecs)
{
  // the following is done after the parent class, vtimer, constructor
  // has been called

  Init();
  SetRecurrent();
  SetPeriod(secs, nanosecs);
  SetDialog(dialog);
  SetIdLabel(labitem);
  Start();
}


// Redefine the "Init" method for "positionTimer" subclass
//
void positionTimer::Init()
{
  //bit
  //Initialize variables added to subclass
  //
  vtimer::Init();
  dialog =  NULL;
  idlabel = NULL;
  oldx = 0;
  oldy = 0;
  previtem = -1;
  contitem = -1;
  oldcontainer = NULL;
}


// Define a convenience function that checks if a point is in a 
// dialog.  Technically this should be part of the dialog class,
// but the "positionTimer" class is designed such that it can be
// associated with any dialog.
//
vbool positionTimer::ContainsPoint(int x, int y)
{
  if ((x >= 0) && (x <= dialog->GetWidth()) &&
      (y >= 0) && (y <= dialog->GetHeight()))
    return TRUE;
  else
    return FALSE;
};


// Redefine "ObserveTimer" method for "positionTimer" subclass
//
// When the timer goes off, this routine is called
//
void positionTimer::ObserveTimer()
{
  int			curx, cury;

  // make sure the dialog is still open
  if (!dialog->IsOpen())
    return;

  // get current pointer position with respect to the dialog
  //
  dialog->GetPointer(&curx,&cury);


  // if pointer has not moved, leave
  //
  if ((curx != oldx) || (cury != oldy))
  {
    // update the pointer trackers
    //
    oldx = curx;
    oldy = cury;
  }
  else
    return;


  // if pointer is not contained in the dialog, leave
  //
  if (!ContainsPoint(curx,cury))
    return;


  // for each dialog item in the dialog ...
  //
  int cnt = dialog->GetItemCount();
  for (int item=0; item<cnt; item++)
  {
    vdialogItem *ditem = dialog->GetItemAt(item);

    // check if pointer is over "ditem"
    //
    if (ditem->ContainsPoint(curx,cury))
    {
      // check if dialog item is a container
      //
      if (ditem->IsKindOf(&vcontainer::Kind))
      {
	// look inside of container
	//
	HandleContainer(vcontainer::CastDown(ditem), curx, cury);
      }
      else
      {
	// don't set the idlabel again if positioned on same item
	//
	if (item != previtem)
	{
	  tipWindow::Display (ditem, dialog, ditem->GetTitle());
	  previtem = item;
	}
      }
      return;
    }
  }


  // if program control gets to this point, the pointer is 
  // not positioned over any item, set message to nothing
  //
  //idlabel->SetTitleScribed(vcharScribeLiteral(""));
  previtem = -1;

  return;
}



//
// following function is called when mouse is found to be over a container
//

void positionTimer::HandleContainer(vcontainer *container, int curx, int cury)
{
  int			containerx, containery;
  vrect		        rect;


  // transform pointer position to its position with respect to
  // the container (all items in container are positioned this way)
  //
  rect = *(container->GetBounds());
  containerx = curx - rect.x;
  containery = cury - rect.y;
  

  // for each item in container...
  //
  int cnt = container->GetItemCount();
  for (int item=0; item<cnt; item++)
  {
    vdialogItem *ditem = container->GetItemAt(item);

    // check if pointer is over "ditem"
    //
    if (ditem->ContainsPoint(containerx,containery))
    {
      // check if dialog item is a container
      //
      if (ditem->IsKindOf(&vcontainer::Kind))
      {	
	// look inside of container, send coordinates with respect
	// to the current container
	//
	HandleContainer(vcontainer::CastDown(ditem),
			containerx,containery);
      }
      else
      {
	// don't set the idlabel again if positioned on same item
	//
	if ((item != contitem) || (container != oldcontainer))
	{
	  tipWindow::Display (ditem, dialog, ditem->GetTitle());
	  contitem = item;
	  oldcontainer = container;
	}
      }
      return;
    }
  }

  // if program control gets to this point, the pointer is 
  // not positioned over any item, set message to title of
  // the container itself
  //
  tipWindow::Display (NULL, dialog, container->GetTitle());
  contitem = -1;
  oldcontainer = NULL;

  return;
}



////////////////////////////////////////////////////////////////////////
//// member functions of "Dialog" subclass
////////////////////////////////////////////////////////////////////////


#if 0
//
// main routine
//
int main(int argc, char *argv[])
{
  unsigned long secs = 0;
  unsigned long nanosecs = 250000000;


  // Initialize Galaxy, since "vstartup.h" is included, all managers will 
  // be started
  vstartup(argc, argv);

  // Get dialog resources, dialog tag must be "Dialog"
  vresource dialogRes = vresourceGet(
			     vapplication::GetCurrent()->GetResources(),
			     vname_Dialog);

  // Create an instance of a "Dialog" but do not load resources yet
  Dialog *dialog = new Dialog(vloadableDONT_LOAD);


  // Get a reference to the quit menu item and tell Galaxy to load 
  // it as type Quit Menu Item, note: menu item must be tagged "Quit",
  // attached to a menu tagged "FileMenu" that is attached to a menu
  // bar tagged "Menu Bar"
  vmenubar *menubar = vmenubar::RequestSubLoad(dialog,  TAG("Menu Bar"));
  vmenu    *menu    = vmenu   ::RequestSubLoad(menubar, TAG("FileMenu"));
  QuitMenuItem::RequestSubLoad(menu, vname_Quit);


  // Now load the dialog resources
  dialog->Load(dialogRes);


  // Set up and start the ptimer
  vdialogItem *item = dialog->FindItem(vnameInternGlobalLiteral("Idlabel"));
  positionTimer *ptimer = new positionTimer(vdialog::CastDown(dialog),
					    vdialogLabelItem::CastDown(item),
					    secs,
					    nanosecs);

  // Open the dialog
  dialog->Place(vwindow::GetRoot(), vrectPLACE_CENTER, vrectPLACE_CENTER);
  dialog->Open();

  // Start processing events
  vevent::Process();

  // delete the timer
  delete ptimer;

  // destroy the dialog
  delete dialog;

  exit(EXIT_SUCCESS);
  return EXIT_FAILURE;
}
#endif
