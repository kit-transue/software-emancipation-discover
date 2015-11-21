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
// ImpactActionsSet.h: interface for the CImpactActionsSet class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_IMPACTACTIONSSET_H__A79B84D6_2580_4066_9DCB_79685E20F921__INCLUDED_)
#define AFX_IMPACTACTIONSSET_H__A79B84D6_2580_4066_9DCB_79685E20F921__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "ImpactAction.h"

typedef CList<CImpactAction*,CImpactAction*> TActionsList;

class CImpactActionsSet {
public:
  CImpactActionsSet(CString& language,bool autoClean = true);
  virtual ~CImpactActionsSet();
  bool Add(CImpactAction* action);
  TActionsList& GetActions();
  CString& GetLanguage(){ return m_szLanguage; };
  CImpactAction* GetAction(int idx);

protected:
  CString m_szLanguage;
  bool m_bAutoClean;
  TActionsList m_ActionsList;
};

#endif // !defined(AFX_IMPACTACTIONSSET_H__A79B84D6_2580_4066_9DCB_79685E20F921__INCLUDED_)
