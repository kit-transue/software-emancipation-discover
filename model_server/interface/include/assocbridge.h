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
#ifndef _ASSOCBRIDGE_H_
#define _ASSOCBRIDGE_H_

#ifndef _symbolArr_h
#include "symbolArr.h"
#endif

class symbolPtr;

class assocBridge
{
private:
  int m_isEditMode;
  int m_selectionRTL;
  int m_entitiesRTL;
  int m_textRTL;
  symbolPtr *m_Assoc;
  symbolArr m_membersArr;
  static assocBridge *m_Instance;

  assocBridge( symbolPtr * );
  ~assocBridge();
  int Init();
  int Apply(const char *);
  int AddExtSel();
  int DeleteAssoc();
  
public:
  static int popupUI( symbolPtr *assoc = 0 );
  static int destroy();
  static int apply( const char* );
  static int addExtSel();
  static int deleteAssoc();
  static int init( int entRTL, int extRTL );
  static int isNameModifiable();
  static const char *getAssocName();
};

void bridge_browse_assocs();
void bridge_browse_ent();

#endif

