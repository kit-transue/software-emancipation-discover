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
// RemoveMethodAction.h: interface for the CRemoveMethodAction class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_REMOVEMETHODACTION_H__B5C604BA_1C8E_4E7B_B24F_4AB51C778893__INCLUDED_)
#define AFX_REMOVEMETHODACTION_H__B5C604BA_1C8E_4E7B_B24F_4AB51C778893__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "ImpactAction.h"

class CRemoveMethodAction : public CImpactAction  
{
public:
	DECLARE_DYNCREATE(CRemoveMethodAction)
	CRemoveMethodAction();
	CRemoveMethodAction(CImpactCtrl* pCtrl,TAcomplishedHash* pRemovedMethodsHash);
	virtual ~CRemoveMethodAction();
	bool ActionPerformed(CEntity* pEntity);
	virtual void Undo();
};

#endif // !defined(AFX_REMOVEMETHODACTION_H__B5C604BA_1C8E_4E7B_B24F_4AB51C778893__INCLUDED_)
