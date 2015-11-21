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
// ClassActions.h: interface for the CClassActions class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CLASSACTIONS_H__8221C8C3_8DFD_43A1_819F_64771DFF9D70__INCLUDED_)
#define AFX_CLASSACTIONS_H__8221C8C3_8DFD_43A1_819F_64771DFF9D70__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "ImpactActionsSet.h"

class CClassActions : public CImpactActionsSet  
{
public:
	CClassActions(CString& language,CImpactCtrl* pCtrl);
	virtual ~CClassActions();
private:
	TAcomplishedHash m_RemovedFields;
	TAcomplishedHash m_RemovedMethods;
	TAcomplishedHash m_RemovedBaseClasses;
	TAcomplishedHash m_AddedBaseClasses;
	TAcomplishedHash m_AddedBaseInterfaces;
	TAcomplishedHash m_RemovedBaseInterfaces;
	TAcomplishedHash m_MovedToPackage;
};

#endif // !defined(AFX_CLASSACTIONS_H__8221C8C3_8DFD_43A1_819F_64771DFF9D70__INCLUDED_)
