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
// InterfaceActions.cpp: implementation of the CInterfaceActions class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Impact.h"
#include "InterfaceActions.h"

#include "RenameAction.h"
#include "DeleteEntityAction.h"
#include "AddFieldAction.h"
#include "RemoveFieldAction.h"
#include "AddMethodAction.h"
#include "RemoveMethodAction.h"
#include "AddBaseInterfaceAction.h"
#include "RemoveBaseInterfaceAction.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CInterfaceActions::CInterfaceActions(CString& language,CImpactCtrl* pCtrl) :
	 CImpactActionsSet(language)
{
	if(m_szLanguage.CompareNoCase("JAVA")==0) {
		Add(new CRenameAction(pCtrl));
		Add(new CDeleteEntityAction(pCtrl));
		Add(new CAddFieldAction(pCtrl));
		Add(new CRemoveFieldAction(pCtrl,&m_RemovedFields));
		Add(new CAddMethodAction(pCtrl));
		Add(new CRemoveMethodAction(pCtrl,&m_RemovedMethods));
		Add(new CAddBaseInterfaceAction(pCtrl,&m_AddedBaseInterfaces));
		Add(new CRemoveBaseInterfaceAction(pCtrl,&m_RemovedBaseInterfaces));
	}
}

CInterfaceActions::~CInterfaceActions()
{

}
