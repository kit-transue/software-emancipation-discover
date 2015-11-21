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
/* $Id: subwin.h 1.4 1996/02/18 13:13:47EST kws Exp  $ */

#ifndef subwinINCLUDED
#define subwinINCLUDED

#include <vport.h>		/* for v*HEADER */

#include vdialogHEADER

#include vx11xlibHEADER

#ifdef _WIN32
typedef void* Display;
typedef long Window;
#endif


class subwin : public vdialogItem
{
public:
  // the default versions of the necessary constructors/destructor
  vloadableINLINE_CONSTRUCTORS(subwin, vdialogItem);

  // overrides from vkindTyped
  vkindTYPED_FULL_DECLARATION(subwin);

  // virtual member function overrides

  // vkindTyped
  void Init(void);
  void Destroy(void);

  // vloadable
  void LoadInit(vresource resource);
  void CopyInit(vloadable *original);

  // vdialogItem
  void SetRect(const vrect *rect);
  void Close(void);
  void Open(void);
  void Draw();
  // exported member variables

  Display			*display;
  Window			 xwindow;
  int				 screenNum;
};

#endif

