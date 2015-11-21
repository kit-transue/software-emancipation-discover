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
#ifndef _rubberGraphic_h
#define _rubberGraphic_h

#include <InterViews/enter-scope.h>
#include <InterViews/rubrect.h>
#include <InterViews/leave-scope.h>
#include "ivfix.h"

class Graphic;

class rubberGraphic : public iv3(RubberRect)
{
  public:
    rubberGraphic(iv3(Painter)* p, iv3(Canvas)* c) 
: iv3(RubberRect)(p, c, 0, 0, 0, 0), graphic(NULL) {}
    virtual void Draw();
    void Erase();
    void SetGraphic(Graphic*);

  private:
    Graphic* graphic;
};

#endif // _rubberGraphic_h

/*
// rubberGraphic.h
//------------------------------------------
// synopsis:
// class rubberGraphic is used for quick hiliting.
// 
// description:
// ...
//------------------------------------------
// $Log: rubberGraphic.h  $
// Revision 1.1 1993/07/28 19:47:22EDT builder 
// made from unix file
 * Revision 1.2.1.2  1992/10/09  18:52:36  builder
 * fixed rcs heaqer
 *
 * Revision 1.2.1.1  92/10/07  20:42:19  smit
 * *** empty log message ***
 * 
 * Revision 1.2  92/10/07  20:42:18  smit
 * *** empty log message ***
 * 
 * Revision 1.1  92/10/07  18:20:32  smit
 * Initial revision
 * 
 * Revision 1.1  92/10/07  17:56:11  smit
 * Initial revision
 * 
// Revision 1.3  92/09/04  18:03:06  oak
// Initialized the "Graphic* graphic;" pointer to NULL
// in rubberGraphic constructor to fix a memory read
// before write error.  (Fixes bug 529)
// 
// Revision 1.2  92/07/29  23:47:52  jon
// InterViews scope fix
// 
// Revision 1.1  92/03/03  18:38:42  glenn
// Initial revision
// 
//------------------------------------------
// Restrictions:
// ...
//------------------------------------------
*/
