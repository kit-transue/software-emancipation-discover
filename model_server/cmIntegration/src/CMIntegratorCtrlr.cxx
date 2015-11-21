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
#include <Xm/DialogS.h>
#include <Xm/PanedW.h>
#include <Xm/PushBG.h>
#include <Xm/Form.h>
#include <Xm/SashP.h>
#include <Xm/MessageB.h>

#include "customize.h"

#include "CMIntegratorCtrlr.h"
#include "CMBase.h"
#include "CMAttributesDlg.h"
#include "ComboBox.h"
#include "top_widgets.h"

Widget CCMIntegratorCtrlr::m_CMSelectDialog = NULL;
int CCMIntegratorCtrlr::m_IsOK = -1;
CCMIntegratorCtrlr* CCMIntegratorCtrlr::theInstance = NULL;

CCMIntegratorCtrlr* CCMIntegratorCtrlr::getInstance() {
	if(theInstance == NULL) {
		theInstance = new CCMIntegratorCtrlr;
	}
	return theInstance;
}


CCMIntegratorCtrlr::CCMIntegratorCtrlr():m_pCMCommands(NULL),m_pCMAttributes(NULL) {
    char* cmName = customize::getStrPref("currentCM");
    m_CMIntegrator = CCMIntegrator::getIntegrator();
    if(cmName != NULL)	{
	m_szCMName = cmName;
	ConnectToCM();
    }
}

CCMIntegratorCtrlr::~CCMIntegratorCtrlr() {
	if(m_pCMAttributes!=NULL) delete m_pCMAttributes;
	if(m_pCMCommands!=NULL) delete m_pCMCommands;
	CCMIntegrator::removeIntegrator();
	m_CMSelectDialog = NULL;
}

void CCMIntegratorCtrlr::setWait(bool state) {
	if(state) {
		m_pBusyCursor = new tempCursor();
	} else {
		delete m_pBusyCursor;
		m_pBusyCursor=NULL;
	}
}

static void ShowResult(Widget parent,char* szResult) {
	Arg args;
	char* string = "Source Management Result";
	Widget errorDlg = XmCreateInformationDialog(parent, string, &args, 0);
	XtUnmanageChild(XmMessageBoxGetChild(errorDlg,XmDIALOG_CANCEL_BUTTON));
	XtUnmanageChild(XmMessageBoxGetChild(errorDlg,XmDIALOG_HELP_BUTTON));
	XmString message = XmStringCreateLocalized(szResult);
	XtVaSetValues(errorDlg,
		XmNmessageString,message,
		XmNdialogStyle,XmDIALOG_FULL_APPLICATION_MODAL,
		NULL);
	XmStringFree(message);
	XtManageChild(errorDlg);
}

static void ShowError(Widget parent,char* szMsg) {
	if(strlen(szMsg)>0) {
		Arg args;
		char* string = "Source Management Error";
		Widget errorDlg = XmCreateErrorDialog(parent, string, &args, 0);
		XtUnmanageChild(XmMessageBoxGetChild(errorDlg,XmDIALOG_CANCEL_BUTTON));
		XtUnmanageChild(XmMessageBoxGetChild(errorDlg,XmDIALOG_HELP_BUTTON));
		XmString message = XmStringCreateLocalized(szMsg);
		XtVaSetValues(errorDlg,
			XmNmessageString,message,
			XmNdialogStyle,XmDIALOG_FULL_APPLICATION_MODAL,
			NULL);
		XmStringFree(message);
		XtManageChild(errorDlg);
	}
}

int CCMIntegratorCtrlr::OnCMCommand(Widget parent,char* cmd,string** projnames,string** filenames) {
	CCMCommand* pCommand = m_pCMCommands->GetCommand(cmd);
	if(pCommand) {
		TStringList filesList;
		string szFile("file");
		string szFiles("files");
		if(pCommand->HasAttribute(szFile) || 
		   pCommand->HasAttribute(szFiles)) {
			string* fname;
                        string new_fname;
			int i;
			for(i=0,fname=filenames[i];fname!=NULL;i++,fname=filenames[i]) {
                                bool file_has_space = false;
        			int fname_len = fname->length();
                                const char *str = fname->c_str();
		        	for (int i = 0; i < fname_len; i++) {
            				if (isspace(str[i])) {
						file_has_space = true;
                                                break;
                                        }
        			}
				//if file has spaces add a pair of quotes around it
				if (file_has_space) {
                                	new_fname = "\"";
                                	new_fname += fname->c_str();
                                	new_fname += "\"";
                                }
				else {
					new_fname = fname->c_str();
				}
				// Forming file list  for submission
				filesList.push_back(new_fname);
			}
		} else {
			string* pname;
			string  new_pname;
			int i;
			for(i=0,pname=projnames[i];pname!=NULL;i++,pname=projnames[i]) {
				// Forming dir list
                                bool proj_has_space = false;
        			int pname_len = pname->length();
                                const char *str = pname->c_str();
		        	for (int i = 0; i < pname_len; i++) {
            				if (isspace(str[i])) {
						proj_has_space = true;
                                                break;
                                        }
        			}
				//if file has spaces add a pair of quotes around it
				if (proj_has_space) {
                                	new_pname = "\"";
                                	new_pname += pname->c_str();
                                	new_pname += "\"";
                                }
				else {
					new_pname = pname->c_str();
				}
				filesList.push_back(new_pname);
			}
		}
		return OnCMCommand(parent, pCommand, filesList);
	}
	return 0;	
}

int CCMIntegratorCtrlr::OnCMCommand(Widget parent,CCMCommand *pCommand,TStringList &filesList) {
	const string szError = "Error:";
	string szCMCmd;
	int ret = 0;		
	if(pCommand) {
		CCMAttributesList cmdAttrs;

		int nAttrCount = pCommand->GetAttributesCount();
		if(nAttrCount>0) {
			for(int i=0;i<nAttrCount;i++) {
				CCMAttribute* pAttr = NULL; 
				string szCmdAttr = pCommand->GetAttribute(i);
				CCMAttributes::iterator iMapPos = m_pCMAttributes->begin();
				CCMAttributes::iterator iMapEnd = m_pCMAttributes->end();
				while(iMapPos!=iMapEnd) {
					string* pszAttrName = iMapPos->first;
					if(pszAttrName->compare(szCmdAttr)==0) {
						pAttr = iMapPos->second;
						break;
					}
					iMapPos++;
				}
				if(pAttr!=NULL) {
					if(pAttr->AskFor()) {
						string cmd("get_attr_value\t");
						cmd += pAttr->GetKey();
						string szOut = m_CMIntegrator->ExecCommand(cmd);
						
						string szPrefix = szOut.substr(0,szError.length());
						if(szPrefix.compare(szError)==0) {
							ShowError(parent,(char*)szOut.substr(szError.length()).c_str());
							return 1;
									
						}
						if(pAttr->GetType() == CCMAttribute::LIST)
							pAttr->SetType(szOut);
						else
							pAttr->SetValue(szOut);
					}
					cmdAttrs.push_back(pAttr);
				}
			}
		}

		if(filesList.size()>0) {
			TStringList::iterator filePos = filesList.begin();
			TStringList::iterator iEnd = filesList.end();
			while(filePos!=iEnd) {
				int nRet = CCMAttributesDlg::IDC_OKALL;
				if(cmdAttrs.size() != 0) {
					string header = "Command Attributes for ";
					header+=(*filePos);
					CCMAttributesDlg wndDlg(header,&cmdAttrs,parent);
					nRet = wndDlg.DoModal();
				}
			
				if(nRet==CCMAttributesDlg::IDCANCEL) break;

				if(nRet==CCMAttributesDlg::IDOK || nRet==CCMAttributesDlg::IDC_OKALL) {
					szCMCmd = pCommand->GetKey();
					CCMAttributesList::iterator posHead = cmdAttrs.begin();
					CCMAttributesList::iterator iAttrsEnd = cmdAttrs.end();
					while(posHead!=iAttrsEnd) {
						CCMAttribute* pAttr = *posHead;
						posHead++; 
						szCMCmd += "\t" + pAttr->GetKey() + "=\"" + pAttr->GetValue() + '"';
					}
						
					szCMCmd += "\t" + (*filePos);
					filePos++;
					if(nRet==CCMAttributesDlg::IDC_OKALL) {
						while(filePos!=iEnd) {
							szCMCmd += "\t" + (*filePos);
							filePos++;
						}
					}

					string szReply = m_CMIntegrator->ExecCommand(szCMCmd);
					string szPrefix = szReply.substr(0,szError.length());
					if(szPrefix.compare(szError)==0) {
						ShowError(parent,(char*)szReply.substr(szError.length()).c_str());
						ret = 1;
					} else {
						if(pCommand->Returns()) {
							ShowResult(parent,(char*)szReply.c_str());
						}
					}
				}
			}
		} else {
			szCMCmd = pCommand->GetKey();
			CCMAttributesList::iterator posHead = cmdAttrs.begin();
			CCMAttributesList::iterator iEnd = cmdAttrs.end();
			while(posHead!=iEnd) {
				CCMAttribute* pAttr = *posHead;
				posHead++; 
				szCMCmd += "\t" + pAttr->GetKey() + "=\"" + pAttr->GetValue() + '"';
			}
			
			string szReply = m_CMIntegrator->ExecCommand(szCMCmd);
			string szPrefix = szReply.substr(0,szError.length());
			if(szPrefix.compare(szError)==0) {
				ShowError(parent,(char*)szReply.substr(szError.length()).c_str());
				ret = 1;
			} else {
				if(pCommand->Returns()) {
					ShowResult(parent,(char*)szReply.c_str());
				}
			}
		}
	}
	return ret;	
}

void CCMIntegratorCtrlr::OnSelectCM(Widget widget, XtPointer client_data, XtPointer call_data) {
	DtComboBoxCallbackStruct* cb = (DtComboBoxCallbackStruct *)call_data;
	CCMIntegratorCtrlr* pThis = (CCMIntegratorCtrlr*)client_data;
	if(pThis!=NULL) {
		if(cb->item_position >= 0) {
			XmString str = cb->item_or_text;
			char* text = NULL;			
			XmStringGetLtoR(str,XmFONTLIST_DEFAULT_TAG, &text);
			pThis->m_szCMName = text;
			genString cmName = text;
			customize::putPref("currentCM",cmName);
			XtFree(text);
		}
		
	}
}

bool CCMIntegratorCtrlr::ConnectToCM() {
    if(!m_szCMName.empty()) {
	if(m_CMIntegrator->Connect(m_szCMName)) {
	    // retriving CM attributes...
	    m_pCMAttributes = new CCMAttributes(m_CMIntegrator->GetAttributes());
	    // retriving CM commands...
	    m_pCMCommands = new CCMCommandsList(m_CMIntegrator->GetCommands());
	    return true;
	}
    }
    return false;
}

void CCMIntegratorCtrlr::OkPressed(Widget dialog,XtPointer client_data, XtPointer call_data) {
	m_IsOK = 1;
	CCMIntegratorCtrlr* pThis = (CCMIntegratorCtrlr*)client_data;

	pThis->setWait(true);
	pThis->ConnectToCM();
	pThis->setWait(false);

	rem_top_widget(m_CMSelectDialog);
	XtDestroyWidget(m_CMSelectDialog);
	m_CMSelectDialog = NULL;
}

void CCMIntegratorCtrlr::CancelPressed(Widget dialog,XtPointer client_data, XtPointer call_data) {
	m_IsOK = 0;
	CCMIntegratorCtrlr* pThis = (CCMIntegratorCtrlr*)client_data;
	
	pThis->setWait(true);
	pThis->setWait(false);

	rem_top_widget(m_CMSelectDialog);
	XtDestroyWidget(m_CMSelectDialog);
	m_CMSelectDialog = NULL;
}

void CCMIntegratorCtrlr::TurnOffSashTraversal(Widget pane) {
	Widget *children;
	int	nChildren;
	
	XtVaGetValues(pane,
		XmNchildren,&children,
		XmNnumChildren, &nChildren,
		NULL);
	while(nChildren-- > 0) {
		if(XmIsSash(children[nChildren]))
			XtVaSetValues(children[nChildren],
				XmNtraversalOn, False,
				NULL);
	}
}

extern Widget GetTopMost(Widget wnd);
void CCMIntegratorCtrlr::CenterDialog(Widget dialog) {
	Position sX, sY;
	Dimension sWidth, sHeight;
	Position dX, dY;
	Dimension dWidth, dHeight;

	Widget shell = GetTopMost(dialog);

	XtVaGetValues(shell, 
		      XtNwidth, &sWidth, 
		      XtNheight, &sHeight,
		      XtNx, &sX, 
		      XtNy, &sY,
		      NULL);  

	XtVaGetValues(dialog, 
		      XtNwidth, &dWidth, 
		      XtNheight, &dHeight,
		      NULL);
	dX = (sWidth - dWidth)/2;  
	dY = (sHeight - dHeight)/2;  

	// ...translate coordinates to center of the root window
	XtTranslateCoords(shell, dX, dY, &dX, &dY);
 
	// ...move popup shell to this position (it's not visible yet)... 
	XtVaSetValues(dialog, XtNx, dX, XtNy, dY, NULL);
}

int CCMIntegratorCtrlr::SelectCM(Widget parent) {
	m_CMSelectDialog = XtVaCreatePopupShell("Configuration Management",
			xmDialogShellWidgetClass,parent,
			XmNdeleteResponse,XmDESTROY,
			NULL);
	add_top_widget(m_CMSelectDialog);
	Widget dialogForm = XtVaCreateWidget("dialogForm",
				xmFormWidgetClass,m_CMSelectDialog,
				XmNdialogStyle, XmDIALOG_FULL_APPLICATION_MODAL,
				XmNwidth, 300,
			NULL);
	Widget pane = XtVaCreateManagedWidget("pane",xmPanedWindowWidgetClass,dialogForm,
			XmNsashWidth,1,
			XmNsashHeight,1,
			XmNleftAttachment,XmATTACH_FORM,
			XmNtopAttachment,XmATTACH_FORM,
			XmNrightAttachment,XmATTACH_FORM,
			XmNbottomAttachment,XmATTACH_FORM,
			NULL);

	// action specific area
	m_CMSelectionWidget = XtVaCreateWidget( "cmsList", 
			        dtComboBoxWidgetClass, pane,
				NULL);
	XtAddCallback(m_CMSelectionWidget, XmNselectionCallback, OnSelectCM,(XtPointer)this);

	if(!m_CMIntegrator->IsAvailable()) m_CMIntegrator->Reconnect();

        string szCMs = m_CMIntegrator->GetCMs();
	XmString labelStr = XmStringCreateLocalized("");
    	DtComboBoxAddItemSelected(m_CMSelectionWidget, labelStr, 0, false);
	XmStringFree(labelStr);
        if(!szCMs.empty()) {
                int nStart = 0;
                int nEnd = 0;
		string szName;
                while((nEnd = szCMs.find('\n',nStart)) > 0) {
                        szName = szCMs.substr(nStart,nEnd-nStart);  
	 		labelStr = XmStringCreateLocalized((char*)szName.c_str());
    			DtComboBoxAddItem(m_CMSelectionWidget, labelStr, 0, false);
			XmStringFree(labelStr);
                        nStart = nEnd+1;
                }
		szName = szCMs.substr(nStart);
 	 	labelStr = XmStringCreateLocalized((char*)szName.c_str());
    		DtComboBoxAddItem(m_CMSelectionWidget, labelStr, 0, false);
		XmStringFree(labelStr);
        }
	
	if(!m_szCMName.empty()) {
	    XmString selectedItem = XmStringCreateLocalized((char*)m_szCMName.c_str());
	    DtComboBoxSelectItem(m_CMSelectionWidget,selectedItem);
	    XmStringFree(selectedItem);
	}                       

	XtVaSetValues(m_CMSelectionWidget,
		XmNleftAttachment, XmATTACH_FORM,
		XmNtopAttachment, XmATTACH_FORM,
		XmNrightAttachment, XmATTACH_FORM,
		XmNbottomAttachment, XmATTACH_FORM,
		NULL);
	XtManageChild(m_CMSelectionWidget);
	
	// common area
	Widget form = XtVaCreateManagedWidget("form",xmFormWidgetClass, pane,
			XmNfractionBase, 5,
			NULL);
	Widget okButton = XtVaCreateManagedWidget("OK",
		xmPushButtonGadgetClass, form,
		XmNtopAttachment, XmATTACH_FORM,
		XmNbottomAttachment, XmATTACH_FORM,
		XmNleftAttachment, XmATTACH_POSITION,
		XmNleftPosition, 1,
		XmNrightAttachment, XmATTACH_POSITION,
		XmNrightPosition, 2,
		XmNshowAsDefault, True,
		XmNdefaultButtonShadowThickness, 1,
		NULL);
	XtAddCallback(okButton,XmNactivateCallback,OkPressed,this);
	Widget cancelButton = XtVaCreateManagedWidget("Cancel",
		xmPushButtonGadgetClass, form,
		XmNtopAttachment, XmATTACH_FORM,
		XmNbottomAttachment, XmATTACH_FORM,
		XmNleftAttachment, XmATTACH_POSITION,
		XmNleftPosition, 3,
		XmNrightAttachment, XmATTACH_POSITION,
		XmNrightPosition, 4,
		XmNshowAsDefault, False,
		XmNdefaultButtonShadowThickness, 1,
		NULL);
	XtAddCallback(cancelButton,XmNactivateCallback,CancelPressed,this);
		
	Dimension h;
	XtVaGetValues(cancelButton,XmNheight, &h, NULL);
	XtVaSetValues(form,XmNpaneMaximum, h,XmNpaneMinimum, h, NULL);
	XtManageChild(dialogForm);		
	
	TurnOffSashTraversal(pane);
	CenterDialog(m_CMSelectDialog);
	Dimension w;
	XtVaGetValues(m_CMSelectDialog,XmNheight, &h, XmNwidth, &w, NULL);
	XtVaSetValues(m_CMSelectDialog,
		XmNminHeight, h,
		XmNmaxHeight, h,
		NULL);
	m_IsOK = -1;
	XtAppContext appContext = XtWidgetToApplicationContext(m_CMSelectDialog);
	while(m_IsOK==-1) {
		XtInputMask mask = XtAppPending(appContext);
		if(mask != 0) XtAppProcessEvent(appContext, mask);
	}
	return m_IsOK;
}

Widget find_top_widget(char * name, int ind);

bool
CCMIntegratorCtrlr::AutoCO(string const &comment, string const& filename)
{
	if(m_szCMName.empty()) {
		return true;
	} else {
		// Assign 'autocomment' attribute.
		CCMAttributes::iterator iMapPos = m_pCMAttributes->begin();
		CCMAttributes::iterator iMapEnd = m_pCMAttributes->end();
		while(iMapPos!=iMapEnd) {
			string* pszAttrName = iMapPos->first;
			if(pszAttrName->compare("autocomment")==0) {
				CCMAttribute* pAttr = iMapPos->second;
				pAttr->SetValue((string &)comment);
				break;
			}
			iMapPos++;
		}
 
		// Execute 'autoco' command.
		CCMCommand* pCommand = m_pCMCommands->GetCommandByKey("autoco");
		if(pCommand != NULL) {
			Widget w = find_top_widget("browser", 1);
			TStringList filesList;
			filesList.push_back(filename);
			if(OnCMCommand(w, pCommand, filesList) != 0)
				return false;
			return true;
		}
	}
	return false;
}

bool
AutoCO(string const &comment, string const &filename)
{
	return CCMIntegratorCtrlr::getInstance()->AutoCO(comment, filename);
}
