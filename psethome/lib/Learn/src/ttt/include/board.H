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
#ifndef _board_h_
#define _board_h_

// usage
#include "square.H"
#include "location.H"
#include "color.H"

class Board {

public:

  Board(int h, int w): height(h), width(w), squares(new Square *[h*w]) {  };
  virtual ~Board();

  virtual void fillSquare(location& loc, color& col);
  virtual void clearSquare(location& loc);

  virtual void save(char* filename);
  virtual void restore(char* filename);

  virtual void reset();

  virtual void activate();
  virtual void deactivate();

  virtual void clear();
  virtual void fill(color& col);

  virtual color& getSquareColor(location& loc);
  virtual int isFilledSquare(location& loc);

  virtual int rows();
  virtual int cols();

protected:

  virtual Square& getSquare(location& loc) = 0;
  int validLocation(location& loc)
  {
    return (!((loc.x() < 0) || (loc.x() >= width) ||
              (loc.y() < 0) || (loc.y() >=height)));
  }

  // The layout of the board should be as illustrated below:
  //
  //   Width, x -------------------->
  //    _____________________________ _ _ _ _ _ 
  // L |         |         |         |
  // e |         |         |         |
  // n |         |         |         |
  //   |_________|_________|_________|_ _ _ _ _
  // y |         |         |         |
  //   |         |         |         |
  // | |         |         |         |
  // | |_________|_________|_________|_ _ _ _ _
  // | |         |         |         |
  // | |         |         |         |
  // | |         |         |         |
  // V |_________|_________|_________|_ _ _ _ _
  //   |         |         |         |
  //
  //   |         |         |         |
  //
  //   |         |         |         |

  Square** squares;
  int height;
  int width;

};

#endif
