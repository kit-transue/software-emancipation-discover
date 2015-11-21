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
// InterfaceActions.h: interface for the CInterfaceActions class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_INTERFACEACTIONS_H__CA77AFC4_CE9B_4366_AA85_A7A82C23444C__INCLUDED_)
#define AFX_INTERFACEACTIONS_H__CA77AFC4_CE9B_4366_AA85_A7A82C23444C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "ImpactActionsSet.h"

class CInterfaceActions : public CImpactActionsSet  
{
public:
	CInterfaceActions(CString& language,CImpactCtrl* pCtrl);
	virtual ~CInterfaceActions();
private:
	TAcomplishedHash m_RemovedFields;
	TAcomplishedHash m_RemovedMethods;
	TAcomplishedHash m_AddedBaseInterfaces;
	TAcomplishedHash m_RemovedBaseInterfaces;
};

#endif // !defined(AFX_INTERFACEACTIONS_H__CA77AFC4_CE9B_4366_AA85_A7A82C23444C__INCLUDED_)
