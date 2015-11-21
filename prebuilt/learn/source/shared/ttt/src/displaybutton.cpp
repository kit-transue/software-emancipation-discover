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
#include "displayButton.H"

displayButton::displayButton(UIObject& parent, application& a,
                             controlFunc ctrlFunc, char* name):
                             UIObject("displayButton", parent, xmPushButtonWidgetClass, NULL)
{
  app = &a;
  function = ctrlFunc;

  XColor fgclr, bgclr;
  allocColor("Linen", &fgclr);
  allocColor("PaleVioletRed3", &bgclr);

  Pixel fgRet, top, bottom, select;
  getColors(bgclr.pixel, &fgRet, &top, &bottom, &select);
  setValues(XmNforeground, fgclr.pixel,
            XmNbackground, bgclr.pixel,
            XmNtopShadowColor, top,
            XmNbottomShadowColor, bottom,
            XmNselectColor, select,
            XmNarmColor, select,
            XmNborderColor, fgRet,
            NULL);

  if (!name) strcpy(name, " ");
  XmString str = XmStringCreateSimple(name);
  XtVaSetValues(window, XmNlabelString, str, NULL);
  XmStringFree(str);

  XtAddCallback(window, XmNactivateCallback, &displayButton::genCallback, (XtPointer) this);
}

void displayButton::genCallback(Widget, XtPointer clientData, XtPointer)
{
  displayButton& db = *(displayButton*) clientData;
  (*(db.function))(*(db.app));
}

void displayButton::deactivate() {

  XtVaSetValues(window,
                XmNsensitive, False,
                XmNpushButtonEnabled, False,
                NULL);
}

void displayButton::activate() {

  XtVaSetValues(window,
                XmNsensitive, True,
                XmNpushButtonEnabled, True,
                NULL);
}

void displayButton::save(char* ) { };
void displayButton::restore(char* ) { };
