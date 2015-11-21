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
#include <vport.h>
#include vdialogHEADER
#include vmenubarHEADER
#include vapplicationHEADER
#include vstdlibHEADER
#include veventHEADER

#include <ggeneric.h>
#include <gdialog.h>
#include <gviewer.h>
#include <gview.h>    // Needed only for DrawBorders in Draw method.
#include <gtoolbar.h>    // Needed only for DrawBorders in Draw method.
#include <tipwin.h>    // Needed only for DrawBorders in Draw method.
#include "gglobalFuncs.h"
#include "Application.h"

extern fIsUIClient;


/*****************************************************************************
 * MyDialog

*****************************************************************************/

// definitions for overrides from vloadable
vkindTYPED_FULL_DEFINITION(MyDialog, vdialog, "MyDialog");


// Deletes viewer, shutting down the application.
void MyDialog::ObserveWindow(vevent *event) 
{
// Note - This assumes that HookClose is called first.

	if (event->IsClose()) {
		// If this is the UI client (gala) then
		//	We should have handled this in the HookClose.
		// For non-UI-clients 
		// 	We just delete the owner.

		if(!fIsUIClient)
			delete owner_;
	}

	vdialog::ObserveWindow(event);
}

static int prevx = -100, prevy = -100;
static int previtem = -1;

int MyDialog::HandleMotion (vevent* event)
{
  DisplayTip();
  return vdialog::HandleMotion (event);
}

void MyDialog::DisplayTip()
{
  int			curx, cury;
  //If dialog has focus
  if( m_fHasFocus )
  {
    // get current pointer position with respect to the dialog
    //
    GetPointer(&curx,&cury);

    // for each dialog item in the dialog ...
    //
    int cnt = GetItemCount();
    for (int item=0; item<cnt; item++)
    {

      vdialogItem *ditem = GetItemAt(item);

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
	          const vchar* title = ditem->GetTitle();
            if ((!title || !*title) && ditem->IsKindOf (&ToolBarButton::Kind))
	            title = ((ToolBarButton*)ditem)->GetTip();
			  tipWindow::SetContainer(vcontainer::CastDown(ditem));
	          tipWindow::Display (ditem, this, title);
	          previtem = item;
	          prevx = curx;
	          prevy = cury;
	        } 
          else 
          {
#if 0
	          if ((abs (curx-prevx) > 8) ||
	            (abs (cury-prevy) > 8))
	              tipWindow::Display (NULL, this, NULL);
#endif
	        }
        }
        return;
      }
    }
  }
  // if program control gets to this point, the pointer is 
  // not positioned over any item, set message to nothing
  //
  tipWindow::SetContainer(NULL);
  DestroyTip();
}

int MyDialog::HandleAcquire (vevent* event)
{
  m_fHasFocus = 1;
  return vdialog::HandleAcquire( event );
}

int MyDialog::HandleLose (vevent* event)
{
  m_fHasFocus = 0;
  DestroyTip();
  return vdialog::HandleLose( event );
}

void MyDialog::DestroyTip()
{
  previtem = -1;
  prevx = -100;
  prevy = -100;
  tipWindow::Display (NULL, this, NULL);  
}
//
// following function is called when mouse is found to be over a container
//

void MyDialog::HandleContainer(vcontainer *container, int curx, int cury)
{
  int			containerx, containery;
  vrect		        rect;
  static int contitem;
  static vcontainer* oldcontainer;

  // transform pointer position to its position with respect to
  // the container (all items in container are positioned this way)
  //
  rect = *(container->GetBounds());
  containerx = curx - rect.x;
  containery = cury - rect.y;
  // for each item in container...
  //
  int cnt = container->GetItemCount();
  for (int item=0; item<cnt; item++) {
     vdialogItem *ditem = container->GetItemAt(item);
     // check if pointer is over "ditem"
     if(ditem->ContainsPoint(containerx,containery)) {
        // check if dialog item is a container
        if (ditem->IsKindOf(&vcontainer::Kind)) {	
	       // look inside of container, send coordinates with respect
	       // to the current container
	       tipWindow::SetContainer(vcontainer::CastDown(ditem));
	       HandleContainer(vcontainer::CastDown(ditem),containerx,containery);
        } else {
	       // don't set the idlabel again if positioned on same item
	       if((item != contitem) || (container != oldcontainer)) {
              const vchar* title = ditem->GetTitle();
              if ((!title || !*title) && ditem->IsKindOf (&ToolBarButton::Kind))
	             title = ((ToolBarButton*)ditem)->GetTip();
	          // let know tip window the current container. It will reset timer if the container changed.
	          tipWindow::Display (ditem, this, title);
	          contitem = item;
	          oldcontainer = container;
	          prevx = curx;
	          prevy = cury;
           }
        }
        return;
     }
  }

  // if program control gets to this point, the pointer is 
  // not positioned over any item, set message to title of
  // the container itself
  //
  tipWindow::Display (NULL, this, container->GetTitle());
  contitem = -1;
  oldcontainer = NULL;

  return;
}

int MyDialog::HandleEnter (vevent* event)
{
    return vdialog::HandleEnter (event);
}

int MyDialog::HandleLeave (vevent* event)
{
    return vdialog::HandleLeave (event);
}

// Destroys all of the elements inside the window.
void MyDialog::Destroy()
{
    vdialog::Destroy();
    if( m_pIconImage )
      delete m_pIconImage;
    vapplication::GetCurrent()->SetIcon( NULL );
    Application::ShowTaskbarWindow(0);//hides taskbar window on NT, does nothing on UNIX
}

/*  Changes the viewer for this dialog box */

void MyDialog::SetViewer(Viewer* owner)
{
    owner_ = owner;
}


int MyDialog::HookClose( vevent *event )
{
	int nRet = vdialog::HookClose( event );

	if (fIsUIClient) {
    	Viewer *vr = GetViewer();
      if( vr )
      {
	      int idCode = vr->GetViewCode();
	      View *v = vr->find_view(idCode);
	      if (v) 
	      {
		      Viewer::SetExecutingView (vr, v);
		      if( Tcl_Eval( v->interp(), "LayerSystemMenuCloseHandler" ) == TCL_OK &&
			      Tcl_GetStringResult(v->interp())[0] == '1' )
			      nRet = 1;
	      }
      }
      if( !nRet )
      {
        int result = gala_eval( g_global_interp,
			        "SystemMenuCloseHandler" );
        if ( result == TCL_OK  && Tcl_GetStringResult(g_global_interp)[0] == '1') {
	        nRet = 1;
        }
      }
	}

	return nRet;
}


/*
If the new configuration results in a resize of the dialog box, it will call resizeWindow
on its owner, passing in the ratio`s for the x and y lengths to change, as well as the new
hieght and width.
*/


int MyDialog::HandleConfigure(vevent *event)
{
    const vrect* myRect = GetRect();
    
    int oldw = myRect->GetWidth();
    int oldh = myRect->GetHeight();

    int r = vdialog::HandleConfigure(event);

    int neww = event->GetWidth();
    int newh = event->GetHeight();
    if ((neww - oldw != 0) || (newh - oldh != 0))
	if (owner_) owner_->ResizeWindow((float)neww/(float)oldw,(float)newh/(float)oldh, newh, neww);
    
    return r;
}


void MyDialog::Draw()
{
    vdialog::Draw();
    if (owner_ && owner_->GetView() )
        owner_->GetView()->DrawBorders();
}

void MyDialog::LoadInit(vresource res)
{ 
  vdialog::LoadInit( res );

  m_fHasFocus = 1;

  //set default icon
  vresource IconRes;
  m_pIconImage = NULL;
  if( getResource("MainIcon", &IconRes) )
  {
    m_pIconImage = new vimage( IconRes );
    if( m_pIconImage )
      vapplication::GetCurrent()->SetIcon( m_pIconImage );
  }

  //set default application title
  vstr *pTitle;
  if( getPreference("StringTable.ApplicationName", &pTitle) )
  {
    vapplication::GetCurrent()->SetTitle( pTitle );
    vstrDestroy( pTitle );  
  }
  Application::ShowTaskbarWindow(1);//shows taskbar window on NT, does nothing on UNIX
}

Viewer *MyDialog::GetViewer()
{
	return owner_;
}




