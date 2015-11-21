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
// Entity.cpp: implementation of the CEntity class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Impact.h"
#include "Entity.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CEntity::CEntity(CString& info, TCHAR delim)
{
	int nStartDelim = 0;
	int nEndDelim;
	int nAttr = 0;
	int nLength = info.GetLength();
	while(nStartDelim<nLength) {
		CString szAttr;
		nEndDelim=info.Find(delim,nStartDelim);
		if(nEndDelim>=0)
			szAttr = info.Mid(nStartDelim,nEndDelim-nStartDelim);
		else
			szAttr = info.Mid(nStartDelim);
		switch(nAttr) {
		case 0:	m_szName = szAttr; break;
		case 1:	m_szKind = szAttr; break;
		case 2:	m_szLanguage = szAttr; break;
		case 3: m_szEtag = szAttr; break;
		case 4: m_bPrivate = (szAttr=="1"); break;
		case 5: m_bProtected = (szAttr=="1"); break;
		case 6: m_szEntityName = szAttr; break;
		}
		nAttr++;
		if(nEndDelim>=0)  nStartDelim = nEndDelim +1;
		else nStartDelim = nLength + 1; // to break the loop
	}
}

CEntity::~CEntity()
{

}

CString& CEntity::GetName()
{
	return m_szName;
}

CString& CEntity::GetKind()
{
	return m_szKind;
}

CString& CEntity::GetLanguage()
{
	return m_szLanguage;
}

CString& CEntity::GetEtag() 
{
	if (m_szEtag.Find(".jar") != -1) {
		m_szEtag = CString(m_szKind + "@" + m_szName);
	}
	return m_szEtag;
	
}


CString& CEntity::GetEntityName()
{
	return m_szEntityName;
}
