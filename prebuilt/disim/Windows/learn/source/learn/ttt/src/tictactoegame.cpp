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
#include "TicTacToeGame.H"
#include "displayTicTacToeGame.H"

ticTacToeGame::ticTacToeGame(int argc, char** argv):
                          game("TicTacToeGame", "./.TTT.sav", 1, 1, 2)
{
  int			i;
  // create toplevel window
  toplevel = new UIObject("TicTacToeDisplay", context, argc, argv);

  // create scores for players and one for ties
  scores = new int[numPlayers+1];
  for(i=0; i<numPlayers; i++)
    scores[i] = 0;
  scores[numPlayers] = 0;

  createComponents();
  initComponents();

  toplevel->realize();
  dpyGames[0]->map();

  start();

  XtAppMainLoop(context);
}


void ticTacToeGame::start()
{
  if (types[turn] != HUMAN) {
    char* tempName = players[turn]->getName();
    dpyGames[0]->fillStatus(tempName, "`s turn!", NULL);
    location loc(0,0);
    execTurn(loc);
  }
}

player* ticTacToeGame::createPlayer(playerType type, const char* name)
{
  color c(name);

  switch(type) {
  case COMPUTER:
   	return new myComputerPlayer("George", c);
  default:
	return new humanPlayer("Bill", c);
  }
}

ticTacToeGame::~ticTacToeGame()
{
  deleteComponents();
  delete [] scores;
}
  
void ticTacToeGame::updateToNextTurn() { turn = nextTurn(); }
int ticTacToeGame::nextTurn() { return ((turn + 1) % numPlayers); }

void ticTacToeGame::updateToLastTurn() { turn = lastTurn(); }
int ticTacToeGame::lastTurn() { return ((turn + numPlayers - 1) % numPlayers); }

void ticTacToeGame::undoTurn()
{
  while (!(gmStatus == NEW)) {

#if 0
    // get the move to undo
    try {
      TicTacToeMove& current = gmHistory->backward();
    }
    catch (beginingOfTime) {
      dpyGames[0]->fillStatus("No moves to undo", NULL);
      return;
    }
#endif

    ticTacToeMove* current = (ticTacToeMove*) gmHistory->backward();
    if (!current) {
      dpyGames[0]->fillStatus("No moves to undo", NULL);
      return;
    }
  
    // print what is happening
    dpyGames[0]->fillStatus("Undoing ",  players[turn]->getName(), "`s turn!", NULL);

    // update the turn variable
    updateToLastTurn();

    // undo both game and display board
    location& loc = current->loc();
    dpyGames[0]->activateBoardSquare(loc);
    gmBoards[0]->clearSquare(loc);
    dpyGames[0]->clearBoardSquare(loc);
    
    // deemphasize any of the board that is emphasized
    if (gmStatus == COMPLETE) {
      int i;

      // deemphasize column if necessary
      if (cols[loc.x()] == (gmBoards[0]->cols()-1))
        for (i=0; i<gmBoards[0]->cols(); i++) {
          location& temp = *(new location(loc.x(),i));
          dpyGames[0]->deemphasizeBoardSquare(temp);
        }
    
      // deemphasize row if necessary
      if (rows[loc.y()] == (gmBoards[0]->cols()-1))
        for (i=0; i<gmBoards[0]->cols(); i++) {
          location& temp = *(new location(i,loc.y()));
          dpyGames[0]->deemphasizeBoardSquare(temp);
        }

      // deemphasize diags if necessary
      if ((loc.x() == loc.y()) && (diags[0] == (gmBoards[0]->cols()-1)))
        for (i=0; i<gmBoards[0]->cols(); i++) {
          location& temp = *(new location(i,i));
          dpyGames[0]->deemphasizeBoardSquare(temp);
        }

      if ((loc.x() == (gmBoards[0]->cols()-1-loc.y())) && (diags[1] == (gmBoards[0]->cols()-1)))
        for (i=0; i<gmBoards[0]->cols(); i++) {
          location& temp = *(new location(i,gmBoards[0]->cols()-1-i));
          dpyGames[0]->deemphasizeBoardSquare(temp);
        }

      // activate board squares that are not filled
      for (int r=0; r<gmBoards[0]->rows(); r++) 
	for (int c=0; c<gmBoards[0]->cols(); c++) {
	  location& tmp = *(new location(r, c));
	  if (!(gmBoards[0]->isFilledSquare(tmp)))
	    dpyGames[0]->activateBoardSquare(tmp);
	}
    }

    // undo the mechanism for updating status
    unfillGameStatus(current->loc());
    gmStatus = CONTINUE;
    winner = 0;

    // undo the player 
    players[turn]->uncalculateMove();

    if (types[turn] == HUMAN) return;
  }
  
  dpyGames[0]->fillStatus(players[nextTurn()]->getName(), "`s turn!", NULL);

  start();
}


void ticTacToeGame::execTurn(location& currentLoc)
{
  int			i;

  if (gmBoards[0]->isFilledSquare(currentLoc)) return;

  ticTacToeMove* initialMove = new ticTacToeMove(currentLoc);
  ticTacToeMove* lastMove = initialMove;
  ticTacToeMove* newMove = initialMove;

  while (!(gmStatus==COMPLETE)) {

    // get new move
    newMove = new ticTacToeMove(players[turn]->calculateMove(*lastMove, *(gmBoards[0])));

    // update the boards
    gmBoards[0]->fillSquare(newMove->loc(), players[turn]->colorOf());
    dpyGames[0]->fillBoardSquare(newMove->loc(), players[turn]->colorOf());
    dpyGames[0]->deactivateBoardSquare(newMove->loc());

    gmHistory->forward(*newMove);
    updateToNextTurn();

    // fill, update, and check status
    fillGameStatus(newMove->loc());
    updateStatus(newMove->loc());
    if (gmStatus == COMPLETE) {

      // if no one is the winner indicate so
      if (winner==NULL) {
        (scores[numPlayers])++;
        dpyGames[0]->fillStatus("The game is a tie!", NULL);
        return;
      }

      // check who the winner is
      for(i=0; i<numPlayers; i++)
        if (*winner==players[i]->colorOf()) {
          (scores[i])++;
          dpyGames[0]->fillStatus("The winner of the game is ", players[i]->getName(), NULL);
          return;
        }
    }

    lastMove = newMove;

    char* tempName = players[turn]->getName();
    dpyGames[0]->fillStatus(tempName, "`s turn!", NULL);
    if (types[turn] == HUMAN) return;
//    else 
//      sleep(1);
  }
}

void ticTacToeGame::fillGameStatus(location& loc)
{
  cols[loc.x()]++;
  rows[loc.y()]++;

  if (loc.x()==loc.y()) { diags[0]++;}
  if ((loc.x()+loc.y()) == (gmBoards[0]->cols()-1)) { diags[1]++; }
}
      
void ticTacToeGame::unfillGameStatus(location& loc)
{
  cols[loc.x()]--;
  rows[loc.y()]--;

  if (loc.x()==loc.y()) { diags[0]--;}
  if ((loc.x()+loc.y()) == (gmBoards[0]->cols()-1)) { diags[1]--; }
}
      
void ticTacToeGame::updateStatus(location& newLoc)
{
  int i;
  gmStatus = COMPLETE;

  // Check to see if the modified column has a winner
  if (cols[newLoc.x()] == (gmBoards[0]->rows() - 1)) {

    // Check each square in the column
    for(i=0; i<gmBoards[0]->rows(); i++) {
      location& loc = *(new location(newLoc.x(),i));
      if (winner==0) winner = new color(gmBoards[0]->getSquareColor(loc));
      if (*winner!=gmBoards[0]->getSquareColor(loc)) {
        delete winner;
        winner = 0;
        break;
      } // if
    } // for

    // yeah! someone won
    if (winner!=0) {
      // update board display
      dpyGames[0]->deactivateBoard();
      for (i=0; i<gmBoards[0]->rows(); i++) {
        location& loc = *(new location(newLoc.x(),i));
        dpyGames[0]->emphasizeBoardSquare(loc);
      }
      return;
    }
  } // if

  // check to see if the modified row has a winner

  if (rows[newLoc.y()] == (gmBoards[0]->cols()-1)) {
    for(i=0; i<gmBoards[0]->cols(); i++) {
      location& loc = *(new location(i,newLoc.y()));
      if (winner==0) winner = new color(gmBoards[0]->getSquareColor(loc));
      if (*winner!=gmBoards[0]->getSquareColor(loc)) {
        delete winner;
        winner = 0;
        break;
      } // if
    } // for

    // yeah! someone won
    if (winner!=0) {
      // update board display
      dpyGames[0]->deactivateBoard();
      for (i=0; i<gmBoards[0]->cols(); i++) {
        location& loc = *(new location(i,newLoc.y()));
        dpyGames[0]->emphasizeBoardSquare(loc);
      }
      return;
    }
  } // if

  // check to see if any of the diags are effected and has a winner

  // left to right diagonal
  if ((newLoc.x() == newLoc.y()) && (diags[0] == (gmBoards[0]->cols()-1))) {
    for(i=0; i<gmBoards[0]->cols(); i++) {
      location& loc = *(new location(i,i));
      if (winner==0) winner = new color(gmBoards[0]->getSquareColor(loc));
      if (*winner!=gmBoards[0]->getSquareColor(loc)) {
        delete winner;
        winner = 0;
        break;
      } // if
    } // for

    // yeah! someone won
    if (winner!=0) {
      // update board display
      dpyGames[0]->deactivateBoard();
      for (i=0; i<gmBoards[0]->cols(); i++) {
        location& loc = *(new location(i,i));
        dpyGames[0]->emphasizeBoardSquare(loc);
      }
      return;
    }
  } // if

  // right to left diagonal
  if ((newLoc.x()==(gmBoards[0]->cols()-1-newLoc.y())) && (diags[1] == (gmBoards[0]->cols()-1))) {
    for(i=0; i<gmBoards[0]->cols(); i++) {
      location& loc = *(new location(i,gmBoards[0]->cols()-1-i));
      if (winner==0) winner = new color(gmBoards[0]->getSquareColor(loc));
      if (*winner!=gmBoards[0]->getSquareColor(loc)) {
        delete winner;
        winner = 0;
        break;
      } // if
    } // for

    // yeah! someone won
    if (winner!=0) {
      // update board display
      dpyGames[0]->deactivateBoard();
      for (i=0; i<gmBoards[0]->cols(); i++) {
        location& loc = *(new location(i,(gmBoards[0]->cols()-1-i)));
        dpyGames[0]->emphasizeBoardSquare(loc);
      }
      return;
    }
  }

  
  // check for a tie
  for(i=0; i<gmBoards[0]->cols(); i++)
    if (cols[i] != (gmBoards[0]->cols()-1)) {
      gmStatus = CONTINUE;
      return;
    }

  for(i=0; i<gmBoards[0]->rows(); i++)
    if (rows[i] != (gmBoards[0]->rows()-1)) {
      gmStatus = CONTINUE;
      return;
    }

  if ((diags[0] != (gmBoards[0]->rows()-1)) || (diags[1] != (gmBoards[0]->rows()-1))) {
    gmStatus = CONTINUE;
    return;
  }

}

void ticTacToeGame::deleteMoves()
{
  // delete moves then history
  for(;;) {

//    try { delete gmHistory->backward(); }
//    catch (beginningOfTime) { break; }

    ticTacToeMove* m = (ticTacToeMove*) gmHistory->backward();
    if (!m) break;
    delete m;
  }
}

void ticTacToeGame::reset()
{
  deleteComponents();
  createComponents();
  initComponents();

  dpyGames[0]->map();
  start();
}

void ticTacToeGame::deleteComponents()
{
  int			i;
  // delete game boards
  for(i=0; i<numBoards; i++)
  {
    delete gmBoards[i];
    gmBoards[i] = 0;
  }

  // delete game displays
  for(i=0; i<numBoards; i++)
  {
    delete dpyGames[i];
    dpyGames[i] = 0;
  }

  // delete history stuff
  deleteMoves();
  delete gmHistory;
  gmHistory = 0;

  // delete players
  for (i=0; i<numPlayers; i++)
    delete players[i];

  // miscellaneous
  delete [] types;
  delete [] cols;
  delete [] rows;
  delete [] diags;
}
    
void ticTacToeGame::quit()
{
  dpyGames[0]->fillStatus("Quit!", NULL);
  exit(1);
}


void ticTacToeGame::save()
{
  dpyGames[0]->fillStatus("Save is not implemented, yet!", NULL);
}

void ticTacToeGame::restore()
{
  dpyGames[0]->fillStatus("Restore is not implemented, yet!", NULL);
}

void ticTacToeGame::createComponents()
{
  // values for player types and board size
  int boardSize = 3;

  // create board
  gmBoards[0] = new gameBoard(boardSize,boardSize);
  cols = new int[gmBoards[0]->cols()];
  rows = new int[gmBoards[0]->rows()];
  diags = new int[2];

  // create history
  gmHistory = new history(gmBoards[0]->cols() * gmBoards[0]->rows());

  // set types of players
  playerType type1 = HUMAN;
  playerType type2 = COMPUTER;

  // record types kludge
  types = new playerType[numPlayers];

  // initialize type
  types[0] = type1;
  types[1] = type2;

  // create players
  char tmp[1024];
  strcpy(tmp, "xlogo64");
  players[0] = createPlayer(types[0], get_path(tmp));

  strcpy(tmp, "opendot");
  players[1] = createPlayer(types[1], get_path(tmp));

  // create display
  strcpy(tmp, "black");
  color defaultColor(get_path(tmp));
  dpyGames[0] = new displayTicTacToeGame(*toplevel, boardSize, boardSize,
                                         defaultColor, *this,
                                         players[0]->getName(), scores[0],
                                         players[1]->getName(), scores[1],
                                         "Tie", scores[2]);
}
  
char* get_path(char* file)
{
  char tmp[1024];

  strcpy(tmp, file);
  strcpy(file, getenv("ParaLearnHome"));
  strcat(file, "/ParaLearn/src/ttt/src/shared/src/");
  strcat(file, tmp);

  return file;
}

void ticTacToeGame::initComponents()
{
  // initialize status information
  int			i;
  turn = 0;
  winner = 0;
  gmStatus = NEW;

  for(i=0; i<gmBoards[0]->cols(); i++)
    cols[i] = -1;
  
  for(i=0; i<gmBoards[0]->rows(); i++)
    rows[i] = -1;

  diags[0] = -1;
  diags[1] = -1;

  // initialize status display
  dpyGames[0]->fillStatus("Welcome", NULL);
}

