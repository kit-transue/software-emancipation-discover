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
#include "displayApp.H"

DisplayApp::DisplayApp(UIObject& parent, int height, int width, color& col, application& app, 
                       boardFunc brdFunc, int numCtrlFuncs, int numPlayers ...):
                       UIObject("DisplayApp", parent, xmFormWidgetClass,
                               XmNmappedWhenManaged, FALSE,
                               XmNhorizontalSpacing, 10, XmNverticalSpacing, 10,
			       XmNallowShellResize, True, NULL)
{
  // create board, status, control
  int max = 64;
  dpyBoard = new DisplayBoard(*this, app, brdFunc, height, width, col);
  dpyStatus = new displayStatus(*this, max);
  dpyControl = new displayControl(*this);

  va_list args;
  va_start(args, brdFunc);

  for(int i=0; i<numCtrlFuncs; i++) {

    // get the name of the function
    char* funcName = va_arg(args, char*);
    if (funcName==0) break;

    // get the function
    controlFunc func = va_arg(args, controlFunc);
    if (func==0) break;
    dpyControl->addButton(funcName, app, func);
  }

//==========================================================================//


//==========================================================================//

  va_end(args);

  // color allocation

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

  // position status

  dpyStatus->setValues(XmNbottomAttachment, XmATTACH_FORM,
                       XmNleftAttachment, XmATTACH_FORM,
                       XmNrightAttachment, XmATTACH_FORM,
                       XmNtopAttachment, XmATTACH_NONE,
                       XmNbottomOffset, 10,
                       XmNrightOffset, 10,
                       XmNleftOffset, 10,
                       NULL);

  // position control

  dpyControl->setValues(XmNtopAttachment, XmATTACH_FORM,
                        XmNbottomAttachment, XmATTACH_NONE,
                        XmNrightAttachment, XmATTACH_FORM,
                        XmNleftAttachment, XmATTACH_WIDGET,
                        XmNleftWidget, dpyBoard->getWidget(),
                        XmNtopOffset, 10,
                        XmNrightOffset, 10,
                        XmNleftOffset, 10,
                        NULL);

  // position board

  dpyBoard->setValues(XmNtopAttachment, XmATTACH_FORM,
                      XmNbottomAttachment, XmATTACH_WIDGET,
                      XmNbottomWidget, dpyStatus->getWidget(),
                      XmNleftAttachment, XmATTACH_FORM,
                      XmNrightAttachment, XmATTACH_NONE,
                      XmNtopOffset, 10,
                      XmNbottomOffset, 10,
                      XmNleftOffset, 10,
                      NULL);

}

DisplayApp::~DisplayApp()
{
  delete dpyControl;
  delete dpyBoard;
  delete dpyStatus;
}

void DisplayApp::fillStatus(char* first ...)
{
  va_list args;
  va_start(args, first);
  dpyStatus->fill(first, args);
  va_end(args);
}

// board area functions
void DisplayApp::fillBoardSquare(location& loc, color& col) { dpyBoard->fillSquare(loc, col); }
void DisplayApp::clearBoardSquare(location& loc) { dpyBoard->clearSquare(loc); }
void DisplayApp::deactivateBoardSquare(location& loc) { dpyBoard->deactivateSquare(loc); }
void DisplayApp::activateBoardSquare(location& loc) { dpyBoard->activateSquare(loc); }
void DisplayApp::deactivateBoard() { dpyBoard->deactivate(); }
void DisplayApp::activateBoard() { dpyBoard->activate(); }
void DisplayApp::deemphasizeBoardSquare(location& loc) { dpyBoard->deemphasizeSquare(loc); }
void DisplayApp::emphasizeBoardSquare(location& loc) { dpyBoard->emphasizeSquare(loc); }
void DisplayApp::deemphasizeBoard() { dpyBoard->deemphasize(); }
void DisplayApp::emphasizeBoard() { dpyBoard->emphasize(); }

// status area functions
void DisplayApp::clearStatus() { dpyStatus->clear(); }

void DisplayApp::printName() { };
