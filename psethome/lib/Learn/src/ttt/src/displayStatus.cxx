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
#include "displayStatus.H"

displayStatus::displayStatus(UIObject& parent, int max):
    UIObject("displayStatus", parent, xmFrameWidgetClass,
            XmNshadowType, XmSHADOW_ETCHED_IN,
            XmNmarginWidth, 2,
            XmNmarginHeight, 2,
            NULL)
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
            XmNwidth, 400,
            NULL);

  maxLength = max;
  dpyLabel = new UIObject("hello", *this, xmLabelWidgetClass, NULL);

  dpyLabel->allocColor("Linen", &fgclr);
  dpyLabel->allocColor("PaleVioletRed3", &bgclr);

  dpyLabel->getColors(bgclr.pixel, &fgRet, &top, &bottom, &select);
  dpyLabel->setValues(XmNforeground, fgclr.pixel,
                      XmNbackground, bgclr.pixel,
                      XmNtopShadowColor, top,
                      XmNbottomShadowColor, bottom,
                      XmNselectColor, select,
                      XmNarmColor, select,
                      XmNborderColor, fgRet,
                      NULL);

}

int displayStatus::length() { return maxLength; }

void displayStatus::fill(char* first, va_list& args)
{
  char* status = new char[maxLength + 1];
  int currentLength = strlen(first);
  strcpy(status, first);

  if (args)
    for(;;) {
      char* tempStr = va_arg(args, char*);
      if (tempStr==NULL) break;
      currentLength += strlen(tempStr);

      if (currentLength>maxLength) break;
      strcat(status, tempStr);
    }

  if (!status) { clear(); return; }

  XmString str = XmStringCreateSimple(status);
  dpyLabel->setValues(XmNlabelString, (XtArgVal) str, NULL);

  XmStringFree(str);
}

void displayStatus::fill(char* first ...)
{
  char* status = new char[maxLength + 1];
  int currentLength = strlen(first);
  strcpy(status, first);

  va_list args;
  va_start(args, first);

  for(;;) {
    char* tempStr = va_arg(args, char*);
    if (tempStr==NULL) break;
    currentLength += strlen(tempStr);

    if (currentLength>maxLength) break;
    strcat(status, tempStr);
  }

  va_end(tempStr);
  if (!status) { clear(); return; }

  XmString str = XmStringCreateSimple(status);
  dpyLabel->setValues(XmNlabelString, (XtArgVal) str, NULL);

  XmStringFree(str);
}

void displayStatus::clear()
{

  XmString str = XmStringCreateSimple(" ");

  dpyLabel->setValues(XmNlabelString, (XtArgVal) str, NULL);
}

void displayStatus::reset()
{
  clear();
}
