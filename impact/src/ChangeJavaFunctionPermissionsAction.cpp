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
#include <Xm/RowColumn.h>
#include <Xm/Frame.h>
#include <Xm/ToggleBG.h>

#include "ChangeJavaFunctionPermissionsAction.h"
#include "QueryResult.h"

bool CChangeJavaFunctionPermissionsAction::isClassFinal = false;

CChangeJavaFunctionPermissionsAction::CChangeJavaFunctionPermissionsAction():CImpactAction("Change Declaration"){
}

CChangeJavaFunctionPermissionsAction::~CChangeJavaFunctionPermissionsAction() {
}

Widget CChangeJavaFunctionPermissionsAction::GetAttributesArea(Widget parent, symbolPtr* pSym) {
	XmString labelStr;
	
	string result;
	string isClassFinalStr;
	string command;

	setWait(true);

	// get current attributes of the entity
	string id = prepareQuery(CEntityInfo::etag(pSym));
	command = string("printformat \"1 %s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\" public protected package_prot private abstract final static native;print ") + id;
	CEntityInfo::exec(command,result);
	CQueryResult Attributes; 
	Attributes.parse(result); 	
	TRecord* pRecord = *Attributes.getStartRecord();
	Boolean bPublic = *(*pRecord)[1]=="1";
	Boolean bProtected = *(*pRecord)[2]=="1";
	Boolean bPackage = *(*pRecord)[3]=="1";
	Boolean bPrivate = *(*pRecord)[4]=="1";

	// check to see if the class the function belongs to is final.
	// if so, then set final to true, and prohibit deselecting it
	command = string("final [ GetClassOf ") + id + string(" ] ");
	CEntityInfo::exec("source_dis impacts.dis", isClassFinalStr);
	CEntityInfo::exec(command, isClassFinalStr);
	
	Boolean bAbstract = *(*pRecord)[5]=="1";
	Boolean bFinal = *(*pRecord)[6]=="1";
	if (isClassFinalStr.compare("1\n") == 0) {
		bFinal = true;
		CChangeJavaFunctionPermissionsAction::isClassFinal = true;
	} else {
		CChangeJavaFunctionPermissionsAction::isClassFinal = false;
	}
	Boolean bStatic = *(*pRecord)[7]=="1";
	Boolean bNative = *(*pRecord)[8]=="1";

	Widget fullArea = XtVaCreateWidget("area",
			xmRowColumnWidgetClass, parent,
			XmNwidth, 290,
			NULL);
	

	Widget border = XtVaCreateManagedWidget("etchedBorder",
			xmFrameWidgetClass, fullArea,
			XmNshadowType, XmSHADOW_ETCHED_IN,
			NULL);
	Widget permissionsArea = XmCreateRadioBox(border,"permisArea",NULL,0);
	m_publicInd = XtVaCreateManagedWidget("Public",
			xmToggleButtonGadgetClass,permissionsArea,
			NULL);
	XmToggleButtonGadgetSetState(m_publicInd,bPublic,true);
	XtAddCallback(m_publicInd,XmNvalueChangedCallback,OnPermissionChanged,this);
	m_protectedInd = XtVaCreateManagedWidget("Protected",
			xmToggleButtonGadgetClass,permissionsArea,
			NULL);
	XmToggleButtonGadgetSetState(m_protectedInd,bProtected,true);
	XtAddCallback(m_protectedInd,XmNvalueChangedCallback,OnPermissionChanged,this);
	m_packageInd = XtVaCreateManagedWidget("\"Package\"",
			xmToggleButtonGadgetClass,permissionsArea,
			NULL);
	XmToggleButtonGadgetSetState(m_packageInd,bPackage,true);
	XtAddCallback(m_packageInd,XmNvalueChangedCallback,OnPermissionChanged,this);
	m_privateInd = XtVaCreateManagedWidget("Private",
			xmToggleButtonGadgetClass,permissionsArea,
			NULL);
	XmToggleButtonGadgetSetState(m_privateInd,bPrivate,true);
	XtAddCallback(m_privateInd,XmNvalueChangedCallback,OnPermissionChanged,this);

	m_abstractInd = XtVaCreateManagedWidget("Abstract",
			xmToggleButtonGadgetClass,fullArea,
			NULL);
	XmToggleButtonGadgetSetState(m_abstractInd,bAbstract,true);
	XtAddCallback(m_abstractInd,XmNvalueChangedCallback,OnPermissionChanged,this);

	m_staticInd = XtVaCreateManagedWidget("Static",
			xmToggleButtonGadgetClass,fullArea,
			NULL);
	XmToggleButtonGadgetSetState(m_staticInd,bStatic,true);
	XtAddCallback(m_staticInd,XmNvalueChangedCallback,OnPermissionChanged,this);

	m_finalInd = XtVaCreateManagedWidget("Final",
			xmToggleButtonGadgetClass,fullArea,
			NULL);
	XmToggleButtonGadgetSetState(m_finalInd,bFinal,true);
	XtAddCallback(m_finalInd,XmNvalueChangedCallback,OnPermissionChanged,this);

	m_nativeInd = XtVaCreateManagedWidget("Native",
			xmToggleButtonGadgetClass,fullArea,
			NULL);
	XmToggleButtonGadgetSetState(m_nativeInd,bNative,true);
	XtAddCallback(m_nativeInd,XmNvalueChangedCallback,OnPermissionChanged,this);

	XtManageChild(permissionsArea);

	setWait(false);
		
	return fullArea;	
}

void CChangeJavaFunctionPermissionsAction::OnPermissionChanged(Widget widget,XtPointer client_data,XtPointer call_data) {
	CChangeJavaFunctionPermissionsAction* pThis = (CChangeJavaFunctionPermissionsAction*)client_data;
	XmToggleButtonCallbackStruct* state = (XmToggleButtonCallbackStruct*) call_data;

	if(!state->set) {
		if (CChangeJavaFunctionPermissionsAction::isClassFinal && !XmToggleButtonGadgetGetState(pThis->m_finalInd)) {
                	pThis->PopupErrorDialog( "The class this function belongs to is final, you can not make this function non-final.");
			XmToggleButtonGadgetSetState(pThis->m_finalInd, true, true);
		}
	}
	if(state->set) {
		if(widget == pThis->m_publicInd) pThis->setPermission(PUBLIC);
		else if(widget == pThis->m_privateInd)
        {
            Boolean bAbstract = XmToggleButtonGadgetGetState(pThis->m_abstractInd);
            if ( bAbstract )
            {
                pThis->PopupErrorDialog( "Function cannot be both Private and Abstract." );
                if ( pThis->getPermission() == PACKAGE )
                    XmToggleButtonGadgetSetState(pThis->m_packageInd,true,true);
                else if ( pThis->getPermission() == PROTECTED )
                    XmToggleButtonGadgetSetState(pThis->m_protectedInd,true,true);
                else if ( pThis->getPermission() == PUBLIC )
                    XmToggleButtonGadgetSetState(pThis->m_publicInd,true,true);

                //XmToggleButtonGadgetSetState(pThis->m_privateInd,false,true);
                //XmToggleButtonGadgetSetState(pThis->m_packageInd,(pThis->getPermission()==PACKAGE),true);
                //XmToggleButtonGadgetSetState(pThis->m_protectedInd,(pThis->getPermission()==PROTECTED),true);
                //XmToggleButtonGadgetSetState(pThis->m_publicInd,(pThis->getPermission()==PUBLIC),true);
            }
            else
                pThis->setPermission(PRIVATE);   
        }
		else if(widget == pThis->m_protectedInd) pThis->setPermission(PROTECTED);
		else if(widget == pThis->m_packageInd) pThis->setPermission(PACKAGE);
        else if(widget == pThis->m_abstractInd)
        {
            Boolean bStatic = XmToggleButtonGadgetGetState(pThis->m_staticInd);
            Boolean bFinal = XmToggleButtonGadgetGetState(pThis->m_finalInd);
            Boolean bNative = XmToggleButtonGadgetGetState(pThis->m_nativeInd);

            if ( pThis->getPermission() == PRIVATE )
            {
                pThis->PopupErrorDialog( "Function cannot be both Private and Abstract." );
                XmToggleButtonGadgetSetState(pThis->m_abstractInd,false,true);
            }
            else if ( bStatic )
            {
                pThis->PopupErrorDialog( "Function cannot be both Static and Abstract." );
                XmToggleButtonGadgetSetState(pThis->m_abstractInd,false,true);
            }
            else if ( bFinal )
            {
                pThis->PopupErrorDialog( "Function cannot be both Final and Abstract." );
                XmToggleButtonGadgetSetState(pThis->m_abstractInd,false,true);
            }
            else if ( bNative )
            {
                pThis->PopupErrorDialog( "Function cannot be both Native and Abstract." );
                XmToggleButtonGadgetSetState(pThis->m_abstractInd,false,true);
            }
        }
        else if(widget == pThis->m_staticInd)
        {
            Boolean bAbstract = XmToggleButtonGadgetGetState(pThis->m_abstractInd);
            if ( bAbstract )
            {
                pThis->PopupErrorDialog( "Function cannot be both Static and Abstract." );
                XmToggleButtonGadgetSetState(pThis->m_staticInd,false,true);
            }
        }
        else if(widget == pThis->m_finalInd)
        {
            Boolean bAbstract = XmToggleButtonGadgetGetState(pThis->m_abstractInd);
            if ( bAbstract )
            {
                pThis->PopupErrorDialog( "Function cannot be both Final and Abstract." );
                XmToggleButtonGadgetSetState(pThis->m_finalInd,false,true);
            }
        }
        else if(widget == pThis->m_nativeInd)
        {
            Boolean bAbstract = XmToggleButtonGadgetGetState(pThis->m_abstractInd);
            if ( bAbstract )
            {
                pThis->PopupErrorDialog( "Function cannot be both Native and Abstract." );
                XmToggleButtonGadgetSetState(pThis->m_nativeInd,false,true);
            }
        }
	}
}

bool CChangeJavaFunctionPermissionsAction::ActionPerformed(symbolPtr* pSym) {
  CImpactAction::ActionPerformed(pSym);
  
  Boolean bAbstract = XmToggleButtonGadgetGetState(m_abstractInd);
  string szAbstract = (bAbstract?"1":"0");
  Boolean bStatic = XmToggleButtonGadgetGetState(m_staticInd);
  string szStatic = (bStatic?"1":"0");
  Boolean bFinal = XmToggleButtonGadgetGetState(m_finalInd);
  string szFinal = (bFinal?"1":"0");
  Boolean bNative = XmToggleButtonGadgetGetState(m_nativeInd);
  string szNative = (bNative?"1":"0");

  char strPerm[10];
  sprintf(strPerm,"%d", m_nPermission);
  string command = "Impact:ChangeJavaFunctionDeclaration " + prepareQuery(CEntityInfo::etag(pSym)) +
                          " " + strPerm + " " + szAbstract + " " + szFinal + " " + szStatic + " " + szNative;
  
  string szDesc = GetName()+" to ";
  if(bAbstract) szDesc += " abstract";
  if(bFinal) szDesc += " final";
  if(bStatic) szDesc += " static";
  if(bNative) szDesc += " native";

  if(m_nPermission==PUBLIC) szDesc += " public";
  else if(m_nPermission==PRIVATE) szDesc += " private";
  else if(m_nPermission==PROTECTED) szDesc += " protected";

  SetDescription(szDesc);

  string results;
  CEntityInfo::exec(command,results);
  parseResult(results);
  return true;
}
