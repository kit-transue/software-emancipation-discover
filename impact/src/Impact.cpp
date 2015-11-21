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
/*
** Generated by WorkShop Visual
*/
/*
**LIBS: -lXm -lXt -lX11
*/

#include "Impact.h"
#include "ClassActions.h"
#include "FieldActions.h"
#include "InterfaceActions.h"
#include "ModuleActions.h"
#include "EnumActions.h"
#include "MacroActions.h"
#include "TypedefActions.h"
#include "TemplateActions.h"
#include "UnionActions.h"
#include "VariableActions.h"
#include "FunctionActions.h"

CImpactComponent* pComp = NULL;

void cleanAtExit(void) {
	if(pComp!=NULL)	delete pComp;
	pComp=NULL;
}

void OnFileMenuActionPerformed(Widget widget, XtPointer client_data, XtPointer call_data) {
	int nItem = (int)client_data;
	
	switch(nItem) {
		case 0: 
			cleanAtExit();
			exit(0); 
			break;
	}
}

void OnEntityMenuActionPerformed(Widget widget, XtPointer client_data, XtPointer call_data) {
	int nItem = (int)client_data;
	CImpactActionsSet* pSet = NULL;
	XmToggleButtonCallbackStruct* callbackData = (XmToggleButtonCallbackStruct*)call_data;

	if(callbackData->set == 0) return;
	
	switch(nItem) {
		case 0: // class
			pSet = new CClassActionsSet("JAVA");  
			printf("Class\n");
			break;
		case 1: // field
			pSet = new CFieldActionsSet("JAVA");  
			printf("Field\n");
			break;
		case 2:	// interface
			pSet = new CInterfaceActionsSet("JAVA");  
			printf("Interface\n");
			break;
		case 3: // module
			pSet = new CModuleActionsSet("JAVA");  
			printf("Module\n");
			break;
		case 4: // variable
			pSet = new CVariableActionsSet("JAVA", false);  
			printf("Variable\n");
			break;
		case 5: // union
			pSet = new CUnionActionsSet("JAVA");  
			printf("Union\n");
			break;
		case 6: // typedef
			pSet = new CTypedefActionsSet("JAVA");  
			printf("Typedef\n");
			break;
		case 7: // template
			pSet = new CTemplateActionsSet("JAVA");  
			printf("Template\n");
			break;
		case 8:	// macro
			pSet = new CMacroActionsSet("JAVA");  
			printf("Macro\n");
			break;
		case 9: // function
			pSet = new CFunctionActionsSet("JAVA");  
			printf("Function\n");
			break;
		case 10: // enum
			pSet = new CEnumActionsSet("JAVA");  
			printf("Enum\n");
			break;
	}
	pComp->SetActions(pSet);
}

#ifdef __STANDALONE_TEST
int main(int argc, char **argv) {
	Widget		mainWidget;
	XtAppContext	app;

    	XtSetLanguageProc(NULL, NULL, NULL);

	// initializing main shell
    	mainWidget = XtVaAppInitialize(&app, "Impact", NULL, 0, &argc, argv, NULL, NULL);
 	
	pComp = new CImpactComponent(mainWidget);
	pComp->SetVisible(true);
	// start application
	XtAppMainLoop(app);
}
#endif // __STANDALONE_TEST





















