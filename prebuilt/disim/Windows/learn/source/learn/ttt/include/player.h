#ifndef _player_h_
#define _player_h_

// standard
#include <sys/stat.h>

// inheritance
#include "application.H"

// usage
#include "color.H"
#include "move.H"
#include "board.H"

class player: public application {

public:

  player(char* name, const color& c): application(name),
                                      col(new color(c)) {  }

  virtual color& colorOf();

  virtual void reset() { }
  virtual move calculateMove(move& lastMove, Board& brd) = 0;
  virtual void uncalculateMove() = 0;

  virtual void save(char* filename) = 0;
  virtual void restore(char* filename) = 0;

protected:

  color* col;
};

#endif
