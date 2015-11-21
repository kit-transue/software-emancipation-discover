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
#ifndef _displayTicTacToeGame_H_
#define _displayTicTacToeGame_H_

// standard

// inheritance
#include "displayApp.H"

// usage
#include "TicTacToeGame.H"
#include "display.H"
#include "funcTypes.H"

class displayTicTacToeGame: public DisplayApp {

public:

  displayTicTacToeGame(UIObject& parent, int height, int width, color& col, game& gm,
                       char* playerName1, int score1,
                       char* playerName2, int score2,
                       char* tie, int score3):
              DisplayApp(parent, height, width, col, gm,
                         (boardFunc) &ticTacToeGame::execTurnCallback, 5, 3,
                         "New",     &ticTacToeGame::resetCallback,
                         "Undo",    &ticTacToeGame::undoTurnCallback,
                         "Save",    &ticTacToeGame::saveCallback,
                         "Restore", &ticTacToeGame::restoreCallback,
                         "Quit",    &ticTacToeGame::quitCallback,
                         playerName1, score1, playerName2, score2, tie, score3,
                         NULL) { };

  virtual DisplayBoard* getBoard(int) { return dpyBoard; };
};

#endif
