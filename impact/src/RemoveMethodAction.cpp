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
#include <Xm/Form.h>
#include <Xm/LabelG.h>

#include "RemoveMethodAction.h"
#include "ComboBox.h"

CRemoveMethodAction::CRemoveMethodAction():CImpactAction("Remove Method"){
        m_MethodsList = NULL;
        m_pMethods = NULL;
}

CRemoveMethodAction::~CRemoveMethodAction() {
        m_MethodsList=NULL;
        if(m_pMethods!=NULL) delete m_pMethods;
}

Widget CRemoveMethodAction::GetAttributesArea(Widget parent, symbolPtr* pSym){
        XmString labelStr;
        Widget area = XtVaCreateWidget("area", 
                xmFormWidgetClass, parent,
                XmNwidth, 300,
                XmNheight, 30,
                XmNnumColumns, 2,
                XmNpacking, XmPACK_COLUMN,
                XmNorientation, XmVERTICAL,
                NULL);
        string result;
        string command;
        string id = prepareQuery(CEntityInfo::etag(pSym));

        setWait(true);
        CEntityInfo::exec("printformat \"%s\t%s\" cname etag",result);

        command = string("sort [set_subtract [get_member_functions ") + id + string("] [set_copy ") + id + string("]]");
        CEntityInfo::exec(command,result);
        m_pMethods = new CQueryResult(); 
        m_pMethods->parse(result);      
        if(m_pMethods->getRecordsCount()>0) {
                labelStr = XmStringCreateLocalized("Method:");
                Widget labelWidget = XtVaCreateManagedWidget("label_1",xmLabelGadgetClass, area,
                        XmNlabelString, labelStr,
                        XmNwidth, 100,
                        XmNtopAttachment,XmATTACH_FORM, 
                        XmNleftAttachment,XmATTACH_FORM,
                        XmNbottomAttachment,XmATTACH_FORM, 
                        NULL);
                XmStringFree(labelStr);
                m_MethodsList = XtVaCreateManagedWidget("methodslist",
                                dtComboBoxWidgetClass, area,
                                XmNtopAttachment,XmATTACH_FORM, 
                                XmNleftAttachment,XmATTACH_WIDGET,
                                XmNleftWidget, labelWidget,
                                XmNrightAttachment,XmATTACH_FORM, 
                                XmNbottomAttachment,XmATTACH_FORM, 
                                NULL);

                TRecordSetIterator iter = m_pMethods->getStartRecord();
                TRecordSetIterator end = m_pMethods->getEndRecord();
                while(iter!=end) {
                        TRecord* pRecord = *iter;
                        labelStr = XmStringCreateLocalized((char*)(*pRecord)[1]->c_str());
                        DtComboBoxAddItem(m_MethodsList, labelStr, 0, false);
                        XmStringFree(labelStr);
                        iter++;
                }
        } else {
                labelStr = XmStringCreateLocalized("Nothing to remove.");
                Widget labelWidget = XtVaCreateManagedWidget("label_1",xmLabelGadgetClass, area,
                        XmNlabelString, labelStr,
                        XmNwidth, 100,
                        XmNtopAttachment,XmATTACH_FORM, 
                        XmNleftAttachment,XmATTACH_FORM,
                        XmNbottomAttachment,XmATTACH_FORM, 
                        XmNrightAttachment,XmATTACH_FORM, 
                        NULL);
                XmStringFree(labelStr);
        }

        setWait(false);
        
        return area;
}

bool CRemoveMethodAction::ActionPerformed(symbolPtr* pSym) {
  if(m_pMethods->getRecordsCount()>0) {
        CImpactAction::ActionPerformed(pSym);
  
        int selectedPosition;
        XtVaGetValues(m_MethodsList, XmNselectedPosition, &selectedPosition, NULL);

        TRecord* pRecord = m_pMethods->getRecord(selectedPosition);
        string command = "Impact:DeleteMemberFunction " + prepareQuery(*(*pRecord)[2]);

  	SetDescription(GetName()+string(" ")+*(*pRecord)[1]);

        string results;
        CEntityInfo::exec(command,results);
        parseResult(results);
        return true;
  } else
        return false;
}





