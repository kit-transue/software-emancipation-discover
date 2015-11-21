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
//#include "Impact.h"
#include "EntityInfo.h"
#include "ImpactComponent.h"

#include <fstream.h>
#include <stdlib.h>
#include <Xm/ToggleB.h>
#include <Xm/Protocols.h>

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
#include "LocalVariableActions.h"
#include "PackageActions.h"
#include "EntityInfo.h"
#include "Report.h"
//#include "IMAttachment.h"

#include "viewerShell.h"
#include "top_widgets.h"
#include "ste_interface.h"
#include "view_creation.h"

void CImpactComponent::OnAttachActionPerformed(Widget widget, XtPointer client_data, XtPointer call_data) {
        CImpactComponent* pThis = (CImpactComponent*)client_data;
	if(pThis->m_pAttachment!=NULL) delete pThis->m_pAttachment;
	pThis->m_pAttachment = new CIMAttachment();
	pThis->m_pAttachment->AddCallback(pThis);
	
	pThis->m_pAttachment->AskForParameters(pThis->GetComponent());
}

void CImpactComponent::ReadyToAttach(int nIsOK) {
	if(m_pAttachment!=NULL && nIsOK) {
		char reportName[1024];

		GenerateReport(reportName);
		
		m_pAttachment->Attach(reportName,m_Component);

		remove(reportName);
	}
}

void CImpactComponent::GenerateReport(char* reportName) {
        int i;

        // generating report name....
        struct tm *newtime;
        time_t aclock;
        time( &aclock );
        newtime = localtime( &aclock );
        strcpy(reportName,getenv("HOME"));
        strcat(reportName,"/ImpactReport_");
        strcat(reportName,asctime(newtime));
        reportName[strlen(reportName)-1]=0;
        strcat(reportName,".txt");
        //replacing all spaces to '_'
        int nLen = strlen(reportName);
        for(i=0;i<nLen;i++) {
                char ch = reportName[i];
                if(ch==' ' || ch==':' || ch=='\n')      ch = '_';
                reportName[i]=ch;
        }

        ofstream os(reportName);
        CImpactReport report(&os);

        symbolArr* pEntities = GetSelectedEntities();    
        unsigned nCount = pEntities->size();
        for(i=0;i<nCount;i++) {
		symbolPtr* pSym = &((*pEntities)[i]);
		string szEntryDescr = "Changes for ";
		szEntryDescr += CEntityInfo::kind(pSym);
		szEntryDescr += " ";
		szEntryDescr += CEntityInfo::name(pSym);
		int nChangesCount = 0;
                CImpactActionsSet* pSet = GetEntityActions(i);
		if(pSet!=NULL) {
			TActionsList& Actions = pSet->GetActions();
                	TActionsIterator begin = Actions.begin();
                	TActionsIterator end = Actions.end();
			CReportEntry* pSymbolEntry = new CReportEntry(szEntryDescr,NULL);
                	for(TActionsIterator childIter = begin;childIter!=end;childIter++) {
                        	CImpactAction* action = (CImpactAction*)*childIter;
	                        CResultsTree* pResults = action->getResults();
	                        if(pResults!=NULL) {
        	                	pSymbolEntry->Add(new CReportEntry(action->GetDescription(),pResults));
					nChangesCount++;
				}
			}
			if(nChangesCount > 0)
				report.Add(pSymbolEntry); 
		}
        }
        report.Generate();
        os.close();
}

void CImpactComponent::OnReportActionPerformed(Widget widget, XtPointer client_data, XtPointer call_data) {
        CImpactComponent* pThis = (CImpactComponent*)client_data;
        tempCursor the_cursor;    // busy cursor push/pop
        
        char reportName[100];
	pThis->GenerateReport(reportName);

        ste_finalize((viewPtr)NULL);

        viewerShell* vsh = viewerShell::get_current_vs(0);
        viewerShell::set_current_vs(vsh);
        view_create(reportName);
        view_create_flush();
        vsh->bring_to_top();
        vsh->map();
}

void CImpactComponent::OnExitActionPerformed(Widget widget, XtPointer client_data, XtPointer call_data) {

        CImpactComponent* pThis = (CImpactComponent*)client_data;
        pThis->SetVisible(false);

        // removing 'close' message handler
        Atom WM_DELETE_WINDOW = XmInternAtom(XtDisplay(pThis->m_Component),"WM_DELETE_WINDOW",False);
        XmRemoveWMProtocolCallback(pThis->m_Component,WM_DELETE_WINDOW,OnExitActionPerformed,pThis);
        delete pThis;
}

void CImpactComponent::OnChangeEntity(Widget widget, XtPointer client_data, XtPointer call_data) {
        DtComboBoxCallbackStruct* cb = (DtComboBoxCallbackStruct *)call_data;
#ifndef __STANDALONE_TEST
        CImpactComponent* pThis = (CImpactComponent*)client_data;
        if(pThis!=NULL) {
                symbolArr* entities = pThis->GetSelectedEntities();
                int nIdx = cb->item_position;
                int nCount = entities->symbols();
                if(nIdx >=0 && nCount>cb->item_position) {
                        pThis->SetSymbolIdx(nIdx);
                        symbolPtr el = (*entities)[nIdx];
        
                        CImpactActionsSet* pSet = pThis->GetEntityActions(nIdx);
                        pThis->SetActions(pSet);
                }
        }
#else
#endif //__STANDALONE_TEST
}

static void recursiveDisplay(Widget changesList, CResultsTree* pRoot,string prefix) {
        if(pRoot != NULL) {
                do{
                        CResultEntity* pEntity = pRoot->getEntity();
                        string szMessage = prefix+*pEntity->getMessage();
                        XmString message = XmStringCreateLocalized((char *)szMessage.c_str());
                        XmListAddItem(changesList,message,0);
                        XmStringFree(message);
                        recursiveDisplay(changesList,pRoot->children(),prefix + "    ");
                } while((pRoot=pRoot->getNext())!=NULL); 
        }
}

void CImpactComponent::OnShowChanges(Widget widget, XtPointer client_data, XtPointer call_data) {
        XmListCallbackStruct* pInfo = (XmListCallbackStruct*)call_data;
        if(pInfo==NULL)  return;
        
        CImpactComponent* pThis = (CImpactComponent*)client_data;
        if(pThis==NULL) return;
        
        if(pInfo->reason == XmCR_BROWSE_SELECT) {
                Widget changesList = pThis->getChangesList();
                XmListDeleteAllItems(changesList);
                CResultsTree* pResults = pThis->getResults();
                if(pResults==NULL) return;
                if((pResults=pResults->children())==NULL) return;
                CResultsTree* pRoot = pResults->get(pInfo->item_position-1);
                if(pRoot==NULL) return;
                recursiveDisplay(changesList,pRoot->children(),"");
        }
}

void CImpactComponent::ShowResults(CResultsTree* pResults) {
        Widget todoList = getToDoList();        
        Widget changesList = getChangesList();

        XmListDeleteAllItems(todoList);
        XmListDeleteAllItems(changesList);

        setResults(pResults);
        if(pResults!=NULL) {
                CResultsTree* pRoot = pResults->children();
                if(pRoot) {
                        do{
                                CResultEntity* pEntity = pRoot->getEntity();
                                XmString message = XmStringCreateLocalized((char *)pEntity->getMessage()->c_str());
                                XmListAddItem(todoList,message,0);
                                XmStringFree(message);
                        } while((pRoot=pRoot->getNext())!=NULL); 
                }
        }
}

void CImpactComponent::OnActionRequested(Widget widget, XtPointer client_data, XtPointer call_data) {
        CCallbackInfo* pInfo = (CCallbackInfo*)call_data;
        if(pInfo==NULL)  return;
        
        CImpactComponent* pThis = (CImpactComponent*)client_data;
        if(pThis==NULL) return;

        CImpactActionsSet* pSet = pThis->GetActions();
        if(pSet==NULL)  return;

        int idx = pThis->m_pActionsList->GetItemNumber(widget);
        CImpactAction* action = pSet->GetAction(idx);
        if(action==NULL)  return;
        
        symbolArr* entities = pThis->GetSelectedEntities();
        symbolPtr el = (*entities)[pThis->GetSymbolIdx()];
                
        if(pInfo->GetState() == CCallbackInfo::START) {
                pInfo->SetCallbackWidget(widget);
                action->AddCallback(OnActionRequested,pInfo);
		if (action->HasAttributes(pThis->GetComponent(), &el)) {
                	action->AskAttributes(pThis->GetComponent(),action->GetName().c_str(),&el);
		} else if (CEntityInfo::kind(&el) == "package" && action->GetName() == "Rename")  {
			// in this case the user said they didn't want to proceed			
			return;
			
		} else {

                        bool performed = action->ActionPerformed(&el);
                        CResultsTree* pResults = action->getResults();
                        if(performed) {
                               XtVaSetValues(widget,XmNset,1,NULL);
                               pThis->ShowResults(pResults);
                        } else {
                               XtVaSetValues(widget,XmNset,0,NULL);
                               action->SetExecuted(false);
                        }
		}
		
        } else {
                if(pInfo->GetState() == CCallbackInfo::END) {
                        pThis->setResults(NULL);
                        action->RemoveCallback(OnActionRequested);
                        if(!action->IsOK()) {
                                if(!action->IsExecuted())
                                        XtVaSetValues(widget,XmNset,0,NULL);
                        } else {
                                bool performed = action->ActionPerformed(&el);
                                CResultsTree* pResults = action->getResults();
                                if(performed) {
                                        XtVaSetValues(widget,XmNset,1,NULL);
                                        pThis->ShowResults(pResults);
                                } else {
                                        XtVaSetValues(widget,XmNset,0,NULL);
                                        action->SetExecuted(false);
                                }
                        }
                } else {
                        if(pInfo->GetState() == CCallbackInfo::MOVE_UP) {
                                CImpactAction* action = pSet->GetAction(idx-1);
                                if(action!=NULL) pThis->ShowResults(action->getResults());
                        } else {
                                if(pInfo->GetState() == CCallbackInfo::MOVE_DOWN) {
                                        CImpactAction* action = pSet->GetAction(idx+1);
                                        if(action!=NULL) pThis->ShowResults(action->getResults());
                                } else {
                                        if(pInfo->GetState() == CCallbackInfo::FOCUS) {
                                                pThis->ShowResults(action->getResults());
                                        }
                                }
                        }
                }
        }
}

CImpactComponent::CImpactComponent(Widget topmost):
                        m_pActions(NULL),m_pResults(NULL),
			m_pAttachment(NULL)
{
        XmString        fileMenu;
        XmString        entityMenu;
        Arg             args[10];
        int             nIdxs = 0;
        const int       nTotalHeight = 500; 
        
        Initialize(CImpactComponent::CImpactComponent);

        m_Component = XtVaAppCreateShell(NULL,"impactMainWindow", 
                                topLevelShellWidgetClass, XtDisplay(topmost),
                                XtNtitle, "Impact",
                                XmNshowSeparator, True,
                                XmNscrollingPolicy, XmAPPLICATION_DEFINED, 
                                NULL);
        // adding 'close' message handler
        Atom WM_DELETE_WINDOW = XmInternAtom(XtDisplay(m_Component),"WM_DELETE_WINDOW",False);
        XmAddWMProtocolCallback(m_Component,WM_DELETE_WINDOW,OnExitActionPerformed,this);

        add_top_widget(m_Component);
        // creating main window
        Widget mainWindow = XtVaCreateManagedWidget( "mainWindow", 
                                xmMainWindowWidgetClass, m_Component,
                                XmNshowSeparator, True,
                                XmNscrollingPolicy, XmAPPLICATION_DEFINED, 
                                NULL);


        // creating the main form
        Widget mainForm = XtVaCreateManagedWidget( "mainform", 
                                xmPanedWindowWidgetClass, mainWindow,
                                XmNallowResize, True,
                                //XmNorientation, XmHORIZONTAL,
                                XmNleftAttachment, XmATTACH_FORM, 
                                XmNtopAttachment, XmATTACH_FORM, 
                                XmNrightAttachment, XmATTACH_FORM, 
                                XmNbottomAttachment, XmATTACH_FORM,
                                NULL);

        // creating views...
        Widget actionsForm = XtVaCreateManagedWidget( "actionsform", 
                                xmFormWidgetClass, mainForm,
                                XmNheight, nTotalHeight/2,
                                NULL);
        Widget selectionForm = XtVaCreateManagedWidget( "historyform", 
                                xmFormWidgetClass, actionsForm,
                                XmNtopAttachment,XmATTACH_FORM, 
                                XmNleftAttachment,XmATTACH_FORM, 
                                XmNrightAttachment,XmATTACH_FORM, 
                                NULL); 
        Widget selectionLabel = XtVaCreateManagedWidget( "Selected items", 
                                xmLabelWidgetClass, selectionForm,
                                XmNtopAttachment,XmATTACH_FORM, 
                                XmNleftAttachment,XmATTACH_FORM, 
                                XmNorientation, XmHORIZONTAL,
                                NULL);
        m_SelectedEntities = XtVaCreateManagedWidget( "itemsList", 
                                dtComboBoxWidgetClass, selectionForm,
                                XmNtopAttachment,XmATTACH_FORM, 
                                XmNleftAttachment,XmATTACH_WIDGET, 
                                XmNrightAttachment,XmATTACH_FORM, 
                                XmNleftWidget, selectionLabel,
                                NULL);
        XtAddCallback(m_SelectedEntities, XmNselectionCallback, OnChangeEntity,(XtPointer)this);

        Widget actionsListForm = XtVaCreateManagedWidget( "historyform", 
                                xmFormWidgetClass, actionsForm,
                                XmNtopAttachment,XmATTACH_WIDGET, 
                                XmNtopWidget, selectionForm,
                                XmNleftAttachment,XmATTACH_FORM, 
                                XmNrightAttachment,XmATTACH_FORM, 
                                XmNbottomAttachment,XmATTACH_FORM, 
                                NULL); 

        Widget actionsLabel =  XtVaCreateManagedWidget( "Actions", 
                                xmLabelWidgetClass, actionsListForm,
                                XmNorientation, XmHORIZONTAL,
                                NULL); 

        nIdxs=0;
        m_pActionsList = new CCheckList(actionsListForm);
        m_pActionsList->AddCallback(OnActionRequested,this);
        
        Widget actionsList = m_pActionsList->GetWidget();
        XtVaSetValues(actionsList,
                      XmNleftAttachment, XmATTACH_FORM,
                      XmNtopAttachment, XmATTACH_WIDGET,
                      XmNtopWidget, actionsLabel,
                      XmNrightAttachment, XmATTACH_FORM,
                      XmNbottomAttachment, XmATTACH_FORM,
                      XmNwidth, 500); 
        XtManageChild(actionsList);     

        Widget todoForm = XtVaCreateManagedWidget( "todoform", 
                                xmFormWidgetClass, mainForm,
                                XmNheight, nTotalHeight/4,
                                NULL);
        Widget todoLabel =  XtVaCreateManagedWidget( "Changes which need to be made", 
                                xmLabelWidgetClass, todoForm,
                                XmNorientation, XmHORIZONTAL,
                                NULL); 

        nIdxs=0;
        XtSetArg(args[nIdxs],XmNleftAttachment, XmATTACH_FORM); nIdxs++; 
        XtSetArg(args[nIdxs],XmNtopAttachment, XmATTACH_WIDGET); nIdxs++; 
        XtSetArg(args[nIdxs],XmNrightAttachment, XmATTACH_FORM); nIdxs++;
        XtSetArg(args[nIdxs],XmNbottomAttachment, XmATTACH_FORM); nIdxs++;
        XtSetArg(args[nIdxs],XmNtopWidget, todoLabel); nIdxs++;
        m_todoList =  XmCreateScrolledList(todoForm, "todoList", args, nIdxs);
        XtAddCallback(m_todoList,XmNbrowseSelectionCallback,OnShowChanges,(XtPointer)this);
        XtManageChild(m_todoList);      


        Widget resultsForm = XtVaCreateManagedWidget( "resultsform", 
                                xmFormWidgetClass, mainForm,
                                XmNheight, nTotalHeight/4,
                                NULL);
        Widget resultsLabel =  XtVaCreateManagedWidget( "Actual changes", 
                                xmLabelWidgetClass, resultsForm,
                                XmNorientation, XmHORIZONTAL,
                                NULL); 

        nIdxs=0;
        XtSetArg(args[nIdxs],XmNleftAttachment, XmATTACH_FORM); nIdxs++; 
        XtSetArg(args[nIdxs],XmNtopAttachment, XmATTACH_WIDGET); nIdxs++; 
        XtSetArg(args[nIdxs],XmNrightAttachment, XmATTACH_FORM); nIdxs++;
        XtSetArg(args[nIdxs],XmNbottomAttachment, XmATTACH_FORM); nIdxs++;
        XtSetArg(args[nIdxs],XmNtopWidget, resultsLabel); nIdxs++;
        m_changesList =  XmCreateScrolledList(resultsForm, "resultsList", args, nIdxs);
        XtManageChild(m_changesList);   


        // creating menus
        entityMenu = XmStringCreateLocalized("Entity");
        mainMenuBar = XmCreateMenuBar(mainWindow, "menubar", NULL, 0); 
        XmStringFree(entityMenu);


        //-------- "File" pulldown menu items
        fileMenu = XmStringCreateLocalized("File");
        Widget fileMenuWidget = XmCreatePulldownMenu(mainMenuBar, "file_pulldown_menu", NULL, 0);
        XtVaCreateManagedWidget("file_menu",
                        xmCascadeButtonWidgetClass, mainMenuBar,
                        XmNlabelString, fileMenu,
                        XmNmnemonic, 'F',
                        XmNsubMenuId, fileMenuWidget,
                        NULL); 
        XmStringFree(fileMenu);
        
        XtAddCallback(XtVaCreateManagedWidget("Report",xmPushButtonWidgetClass,fileMenuWidget,NULL),
                XmNactivateCallback,OnReportActionPerformed,this);
        XtAddCallback(XtVaCreateManagedWidget("Attach to IM issue...",xmPushButtonWidgetClass,fileMenuWidget,NULL),
                XmNactivateCallback,OnAttachActionPerformed,this);
        XtAddCallback(XtVaCreateManagedWidget("Close",xmPushButtonWidgetClass,fileMenuWidget,NULL),
                XmNactivateCallback,OnExitActionPerformed,this);


        //creating status bar
        Widget statusForm = XtVaCreateManagedWidget( "statusBar",
                                xmRowColumnWidgetClass, mainWindow,
                                XmNspacing, 0,
                                XmNorientation, XmHORIZONTAL,
                                NULL);
        statusBar = XtVaCreateManagedWidget("", 
                                xmLabelWidgetClass, statusForm, 
                                NULL);
        // registering status bar       
        nIdxs = 0;
        XtSetArg(args[nIdxs], XmNmessageWindow, statusForm); nIdxs++;
        XtSetArg(args[nIdxs], XmNmenuBar, mainMenuBar); nIdxs++;
        XtSetArg(args[nIdxs], XmNworkWindow, mainForm); nIdxs++;
        XtSetValues( mainWindow, args, nIdxs);

        XtManageChild(mainMenuBar);
}

CImpactComponent::~CImpactComponent() {
        if(!m_EntityActions.empty()) { 
                CEntityActions::iterator iter;
                for(iter = m_EntityActions.begin(); iter != m_EntityActions.end(); iter++) {
                        delete *iter;
		}
                m_EntityActions.clear();
        }

        rem_top_widget(m_Component);
	if(m_pAttachment!=NULL) delete m_pAttachment;
		
}

bool CImpactComponent::SetActions(CImpactActionsSet* actions) {
        m_pActions = actions;
        m_pActionsList->DeleteAllItems();
        if(m_pActions!=NULL) {
                TActionsIterator begin = m_pActions->GetActions().begin();
                TActionsIterator end = m_pActions->GetActions().end();
                for(TActionsIterator i = begin;i!=end;i++) {
                        CImpactAction* action = (CImpactAction*)*i;
                        XmString szItem = XmStringCreateLocalized((char*)action->GetName().c_str());
                        m_pActionsList->Add(szItem,action->IsExecuted());
                        XmStringFree(szItem);
                }
        }
        return true;
}

#ifndef __STANDALONE_TEST
void CImpactComponent::SetSelectedEntities(symbolArr& entities) {
        m_Entities = entities;  
        XmString labelStr;

        if(!m_EntityActions.empty()) { 
                CEntityActions::iterator iter;
                for(iter = m_EntityActions.begin(); iter != m_EntityActions.end(); iter++)
                        delete *iter;
                m_EntityActions.clear();
        }
        
        symbolPtr el;
        ForEachS(el, m_Entities) {
                if(el.isnull())  continue; 
                ddKind el_kind = el.get_kind();
                string lang = CEntityInfo::language(&el);
		string id = CEntityInfo::prepareQuery(CEntityInfo::etag(&el));
		string result;
		CEntityInfo::exec("oo_member " + id, result);
		bool isMemberOfClass = false;
		if (result.find("1") != -1) {
			isMemberOfClass = true;
		}
                CImpactActionsSet* pSet = NULL;
                switch(el_kind) {
                        case DD_CLASS: // class
                                pSet = new CClassActionsSet(lang);  
                                break;
                        case DD_FIELD: // field
                                pSet = new CFieldActionsSet(lang);  
                                break;
                        case DD_INTERFACE:      // interface
                                pSet = new CInterfaceActionsSet(lang);  
                                break;
                        case DD_MODULE: // module
                                pSet = new CModuleActionsSet(lang);  
                                break;
                        case DD_VAR_DECL: // variable
                                pSet = new CVariableActionsSet(lang,isMemberOfClass);  
                                break;
                        case DD_UNION: // union
                                pSet = new CUnionActionsSet(lang);  
                                break;
                        case DD_TYPEDEF: // typedef
                                pSet = new CTypedefActionsSet(lang);  
                                break;
                        case DD_TEMPLATE: // template
                                pSet = new CTemplateActionsSet(lang);  
                                break;
                        case DD_MACRO:  // macro
                                pSet = new CMacroActionsSet(lang);  
                                break;
                        case DD_FUNC_DECL: // function
                                pSet = new CFunctionActionsSet(lang);  
                                break;
                        case DD_ENUM: // enum
                                pSet = new CEnumActionsSet(lang);  
                                break;
                        case DD_LOCAL: // local variable
                                pSet = new CLocalVariableActionsSet(lang);
                                break;
                        case DD_PACKAGE: // package
                                pSet = new CPackageActionsSet(lang);
                                break;
                        default:
                                printf( "Missed el_kind: %d\n", el_kind );
                                break;
                }
                m_EntityActions.push_back(pSet);
                labelStr = XmStringCreateLocalized(el.get_name());
                DtComboBoxAddItem(m_SelectedEntities, labelStr, 0, false);
                XmStringFree(labelStr);
        }
}
#endif //__STANDALONE_TEST 


