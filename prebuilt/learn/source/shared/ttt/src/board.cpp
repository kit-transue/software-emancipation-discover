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
#include "board.H"

Board::~Board() { delete squares; }

int Board::rows() { return width; }
int Board::cols() { return height; }

void Board::fillSquare(location& loc, color& col)
{
//  if (!(validLocation(loc))) throw notValid();

  if (!(validLocation(loc))) return;
  getSquare(loc).fill(col);
}

void Board::clearSquare(location& loc)
{
//  if (!(validLocation(loc))) throw notValid();

  if (!(validLocation(loc))) return;
  getSquare(loc).clear();
}

color& Board::getSquareColor(location& loc)
{
//  if (!(validLocation(loc))) throw notValid();

  return (getSquare(loc).getColor());
}

int Board::isFilledSquare(location& loc)
{
//  if (!(validLocation(loc))) throw notValid();

  if (!(validLocation(loc))) return -1;
  return (getSquare(loc).isFilled());
}

void Board::save(char* filename)
{
  for(int i=0; i<height; i++)
    for(int j=0; j<width; j++) {
      location loc(j,i);
      getSquare(loc).save(filename);
    }
}

void Board::restore(char* filename)
{
  for(int i=0; i<height; i++)
    for(int j=0; j<width; j++) {
      location loc(j,i);
      getSquare(loc).restore(filename);
    }
}

void Board::clear()
{
  for(int i=0; i<height; i++)
      for(int j=0; j<width; j++) {
        location loc(j,i);
        getSquare(loc).clear();
      }
}

void Board::fill(color& c)
{
  for(int i=0; i<height; i++)
      for(int j=0; j<width; j++) {
        location loc(j,i);
        getSquare(loc).fill(c);
      }
}

void Board::activate()
{
  for(int i=0; i<height; i++)
      for(int j=0; j<width; j++) {
        location loc(j,i);
        getSquare(loc).activate();
      }
}

void Board::deactivate()
{
  for(int i=0; i<height; i++)
      for(int j=0; j<width; j++) {
        location loc(j,i);
        getSquare(loc).deactivate();
      }
}

void Board::reset()
{
  activate();
  clear();
}
