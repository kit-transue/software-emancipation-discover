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
#include "InterfaceActions.h"
#include "RenameAction.h"
#include "AddFieldAction.h"
#include "RemoveFieldAction.h"
#include "AddMethodAction.h"
#include "RemoveMethodAction.h"
#include "AddBaseInterfaceAction.h"
#include "RemoveBaseInterfaceAction.h"
#include "DeleteEntityAction.h"

CInterfaceActionsSet::CInterfaceActionsSet(string language) : 
					CImpactActionsSet(language) {
	if(m_szLanguage=="JAVA") {
		Add(new CRenameAction());
		Add(new CDeleteEntityAction());
		Add(new CAddFieldAction());
		Add(new CRemoveFieldAction());
		Add(new CAddMethodAction());
		Add(new CRemoveMethodAction());
		Add(new CAddBaseInterfaceAction());
		Add(new CRemoveBaseInterfaceAction());
	}
}

CInterfaceActionsSet::~CInterfaceActionsSet() {
}
