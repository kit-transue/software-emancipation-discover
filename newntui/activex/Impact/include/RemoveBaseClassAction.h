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
// RemoveBaseClassAction.h: interface for the CRemoveBaseClassAction class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_REMOVEBASECLASSACTION_H__7767420C_5447_43D4_AC73_D3862ABB02DC__INCLUDED_)
#define AFX_REMOVEBASECLASSACTION_H__7767420C_5447_43D4_AC73_D3862ABB02DC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "ImpactAction.h"

class CRemoveBaseClassAction : public CImpactAction  
{
public:
	DECLARE_DYNCREATE(CRemoveBaseClassAction)
	CRemoveBaseClassAction();
	CRemoveBaseClassAction(CImpactCtrl* pCtrl,TAcomplishedHash* pRemovedClasesHash);
	virtual ~CRemoveBaseClassAction();
	bool ActionPerformed(CEntity* pEntity);
	virtual void Undo();
};

#endif // !defined(AFX_REMOVEBASECLASSACTION_H__7767420C_5447_43D4_AC73_D3862ABB02DC__INCLUDED_)
