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
// AddBaseClassAction.h: interface for the CAddBaseClassAction class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ADDBASECLASSACTION_H__C1E518D6_A3C7_4F4A_B49E_0BD20817860C__INCLUDED_)
#define AFX_ADDBASECLASSACTION_H__C1E518D6_A3C7_4F4A_B49E_0BD20817860C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "ImpactAction.h"

class CAddBaseClassAction : public CImpactAction  
{
public:
	virtual void Undo();
	DECLARE_DYNCREATE(CAddBaseClassAction)

	CAddBaseClassAction();
	CAddBaseClassAction(CImpactCtrl* pCtrl,TAcomplishedHash* pInheritedClassesHash);
	virtual ~CAddBaseClassAction();
	bool ActionPerformed(CEntity* pEntity);
};

#endif // !defined(AFX_ADDBASECLASSACTION_H__C1E518D6_A3C7_4F4A_B49E_0BD20817860C__INCLUDED_)
