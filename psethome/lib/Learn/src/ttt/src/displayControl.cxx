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
#include "displayControl.H"

displayControl::displayControl(UIObject& parent):
                UIObject("displayControl",parent, xmRowColumnWidgetClass, NULL),
                        numButtons(0)
{
  XColor fgclr, bgclr;
  allocColor("Linen", &fgclr);
  allocColor("PaleVioletRed3", &bgclr);
  Pixel fgRet, top, bottom, select;
  getColors(bgclr.pixel, &fgRet, &top, &bottom, &select);
  setValues(XmNforeground, fgclr.pixel,
            XmNbackground, bgclr.pixel,
            XmNtopShadowColor, top,
            XmNbottomShadowColor, bottom,
            XmNborderColor, fgRet,
            NULL);
}

void displayControl::addButton(char* name, application& app, controlFunc ctrlFunc)
{
  if (numButtons>=MAX_NUM_BUTTONS) return;
  buttons[numButtons++] = new displayButton(*this, app, ctrlFunc, name);
}

void displayControl::deactivateButton(int i)
{
  if ((i<1)||(i>numButtons)) return;
  (buttons[i-i])->deactivate();
}

void displayControl::activateButton(int i)
{
  if ((i<1)||(i>numButtons)) return;
  (buttons[i-1])->activate();
}

void displayControl::reset()
{
  for(int i=0; i<numButtons; i++)
    activateButton(i);
}

void displayControl::save(char* ) { }
void displayControl::restore(char* ) { }

