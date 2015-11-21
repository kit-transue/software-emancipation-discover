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
#include "game.H"


game::game(char* name, char* filename, int numBoards, int numDisplays, int numPlayers): application(name)
{
  game::filename = new char[strlen(filename) + 1];
  strcpy(game::filename, filename);

  // create pointers for boards, displays, and players
  game::numBoards = numBoards;
  game::numDisplays = numDisplays;
  game::numPlayers = numPlayers;

  gmBoards = new gameBoard* [numBoards];
  dpyGames = new DisplayApp* [numDisplays];
  players = new player* [numPlayers];
}

game::~game()
{
  delete filename;
  delete players;
  delete gmBoards;
  delete dpyGames;
}

void game::save()
{
//  try { for(int i=0; i<numBoards; i++)
//	  gmBoards[i]->save(filename);
//
//	for(i=0; i<numDisplays; i++)
//	  dpyGames[i]->save(filename);
//	
//	for(i; i<numPlayers; i++)
//	  players[i]->save(filename);
//     }
//  catch (cannotSave) { dpy->fillStatus("Cannot save game", NULL); }

  for(int i=0; i<numDisplays; i++)
    dpyGames[i]->fillStatus("Cannot save game", NULL);
}

void game::restore()
{
//  try { for(int i=0; i<numBoards; i++)
//	  gmBoards[i]->restore(filename);
//
//	for(i=0; i<numDisplays; i++)
//	  dpyGames[i]->restore(filename);
//	
//	for(i=0; i<numPlayers; i++)
//	  players[i]->restore(filename);
//     }
//  catch (cannotRestore) {   
//    for(int i=0; i<numDisplays; i++) 
//      dpyGames[i]->fillStatus("Cannot restore game", NULL); 
//  }

  for(int i=0; i<numDisplays; i++)
    dpyGames[i]->fillStatus("Cannot restore game", NULL);
}


// callbacks

void game::resetCallback(game& gm) { gm.reset(); }
void game::quitCallback(game& gm) { gm.quit(); }
void game::execTurnCallback(game& gm, location& loc) { gm.execTurn(loc); }
void game::undoTurnCallback(game& gm) { gm.undoTurn(); }
void game::saveCallback(game& gm) { gm.save(); }
void game::restoreCallback(game& gm) { gm.restore(); }
