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
/***************************
  CommandLineView.C
  Created by Jeremy Rothman

  Contains the basic functionality for the commandLine that appears at the bottom of the
  viewer.

***************************/

#include <vport.h>
#include vdialogHEADER
#include vdragHEADER
#include vcontainerHEADER
#include vapplicationHEADER
#include vliststrHEADER
#include vmenubarHEADER
#include vmenuHEADER
#include vwindowHEADER

#include <ggeneric.h>
#include <gpaneObjects.h>
#include <gcommandLine.h>
#include <gviewer.h>


const vchar* DISPLAY_TAG = (vchar*)"messageLine";

CommandLineView::CommandLineView(Viewer* newViewer, Rect newRect, Pane* newPane)
: View(NULL, newViewer, newRect)

{
  vrect rect = *newPane->GetRect();
  rect.SetX (newRect.x);
  rect.SetY (newRect.y);
  rect.SetHeight (newRect.h);
  rect.SetWidth (newRect.w);
  newPane->SetRect (&rect);
  SetPane(newPane);
  hasBorder_ = vFALSE;
  vLock_ = vTRUE;
  Unsplitable_ = vTRUE;

#ifdef OWNCOLOR
  newPane->SetForeground(newViewer->GetCommandLineForeground());
  newPane->SetBackground(newViewer->GetCommandLineBackground());
#endif

  displayLine_ = (gmessageBox*)FindItem((vchar *)DISPLAY_TAG);
}


CommandLineView::~CommandLineView()
{
    displayLine_ = NULL;
}



void CommandLineView::DisplayMessage(vchar* newMessage)
{
  if ((displayLine_) && (newMessage))
      displayLine_->DisplayMessage(newMessage);
}



void CommandLineView::DisplayQuickHelp(vchar* newMessage)
{
  if ((displayLine_) && (newMessage))
      displayLine_->DisplayQuickHelp(newMessage);
}
