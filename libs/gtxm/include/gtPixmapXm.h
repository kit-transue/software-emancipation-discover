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
#ifndef _gtPixmapXm_h
#define _gtPixmapXm_h

// gtPixmapXm.h
//------------------------------------------
// synopsis:
// 
// 1-24 bit deep image, with optional
// transparant areas.
//
//------------------------------------------

#include <lib/xpm.h>

#include <gtPixmap.h>
#include <gtBitmapXm.h>

class gtBase;

class gtPixmapXm : public gtPixmap
{
public:
  gtPixmapXm(gtBase *parent, const char *name, const char *xpm_filename, int width, int height);
  gtPixmapXm(gtBase *parent, const char *name, const char **xpm_data, int width, int height);
  ~gtPixmapXm();


public:
  inline Pixmap get_image(void);
  inline Pixmap get_mask(void);

private:
  int init_pixmaps(gtBase *, bool, char const *, char const **);
  int place_in_label(gtBase *, int, int);

private:
  Pixmap         image;
  Pixmap         mask;
  XpmAttributes  attributes;
};

Pixmap gtPixmapXm::get_image(void)
{
  return image;
}

Pixmap gtPixmapXm::get_mask(void)
{
  return mask;
}

#endif 
