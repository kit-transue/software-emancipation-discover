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
#ifndef _displayApp_H_
#define _displayApp_H_

// standard
#include "x11_intrinsic.h"
#include <Xm/Form.h>
#include <stdarg.h>
// inheritance
#include "display.H"
#include "displayControl.H"
#include "displayBoard.H"
#include "displayStatus.H"
#include "displayScore.H"

// usage 
#include "application.H"
#include "funcTypes.H"

class displayScore;

class DisplayApp: public UIObject {
public:
  DisplayApp(UIObject& parent, int height, int width, color& col, application& app, 
	     boardFunc brdFunc, int numCtrlFuncs, int numScores ...);
  ~DisplayApp();

  // board area functions
  virtual void fillBoardSquare(location& loc, color& col);
  virtual void clearBoardSquare(location& loc);
  virtual void deactivateBoardSquare(location& loc);
  virtual void activateBoardSquare(location& loc);
  virtual void deactivateBoard();
  virtual void activateBoard();
  virtual void deemphasizeBoardSquare(location& loc);
  virtual void emphasizeBoardSquare(location& loc);
  virtual void deemphasizeBoard();
  virtual void emphasizeBoard();

  // status area functions
  virtual void fillStatus(char* first ...);
  virtual void clearStatus();
  static void printName();

protected:
  displayControl* dpyControl;
  DisplayBoard* dpyBoard;
  displayStatus* dpyStatus;
  displayScore* dpyScore;

private:
  virtual DisplayBoard* getBoard(int index) = 0;

};

#endif
