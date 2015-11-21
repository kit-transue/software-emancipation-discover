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
// MSDEVIntegratorCtl.cpp : Implementation of the CMSDEVIntegratorCtrl ActiveX Control class.

#include "stdafx.h"
#include "MSDEVIntegrator.h"
#include "MSDEVIntegratorCtl.h"
#include "MSDEVIntegratorPpg.h"
#include "./include/IDE_editor.h"
#include "../../nameserver/include/nameServCalls.h"


#define COMMAND_OPCODE 2
#define INFO_OPCODE    1
#define EXIT_OPCODE    0
static char signature[]={27,0,0,27};

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif



void CIntegratorSocket::SetParent(CMSDEVIntegratorCtrl* p) {
	parent=p;
}


void CIntegratorSocket::OnReceive( int nErrorCode ) {
unsigned char opcode;
	 Receive(&opcode,1);
	 switch(opcode) {
		 // Ctrl+click event, we need to place description into VC window.
	     case '0' : if(parent) parent->CtrlClickProcessor(); break;
		 // Exit button pressed - integration terminated
		 case '1' : break;
		 // Activate event, we need to pop-up Discover window
	     case '2' : if(parent) parent->ActivateProcessor(); break;
         // Query - we need to extract symbol from VC and put into Discover
		 case '3' : if(parent) parent->QueryProcessor(); break;
	     // Open definition - we need to take symbol information and then
		 // go to the symbol definition
		 case '4' : if(parent) parent->DefinitionProcessor(); break;
	 }
}






IMPLEMENT_DYNCREATE(CMSDEVIntegratorCtrl, COleControl)


/////////////////////////////////////////////////////////////////////////////
// Message map

BEGIN_MESSAGE_MAP(CMSDEVIntegratorCtrl, COleControl)
	//{{AFX_MSG_MAP(CMSDEVIntegratorCtrl)
	ON_WM_CREATE()
	ON_WM_CLOSE()
	//}}AFX_MSG_MAP
	ON_OLEVERB(AFX_IDS_VERB_EDIT, OnEdit)
	ON_OLEVERB(AFX_IDS_VERB_PROPERTIES, OnProperties)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// Dispatch map

BEGIN_DISPATCH_MAP(CMSDEVIntegratorCtrl, COleControl)
	//{{AFX_DISPATCH_MAP(CMSDEVIntegratorCtrl)
	DISP_FUNCTION(CMSDEVIntegratorCtrl, "OpenFile", OpenFile, VT_EMPTY, VTS_BSTR VTS_I4)
	DISP_FUNCTION(CMSDEVIntegratorCtrl, "MakeSelection", MakeSelection, VT_EMPTY, VTS_BSTR VTS_I4 VTS_I4 VTS_BSTR)
	DISP_FUNCTION(CMSDEVIntegratorCtrl, "GetTabSize", GetTabSize, VT_I2, VTS_NONE)
	DISP_FUNCTION(CMSDEVIntegratorCtrl, "SetInfo", SetInfo, VT_EMPTY, VTS_BSTR)
	DISP_FUNCTION(CMSDEVIntegratorCtrl, "GetSelection", GetSelection, VT_EMPTY, VTS_PBSTR VTS_PBSTR VTS_PI4 VTS_PI4)
	DISP_FUNCTION(CMSDEVIntegratorCtrl, "Exit", Exit, VT_EMPTY, VTS_NONE)
	DISP_FUNCTION(CMSDEVIntegratorCtrl, "Command", Command, VT_EMPTY, VTS_BSTR)
	//}}AFX_DISPATCH_MAP
	DISP_FUNCTION_ID(CMSDEVIntegratorCtrl, "AboutBox", DISPID_ABOUTBOX, AboutBox, VT_EMPTY, VTS_NONE)
END_DISPATCH_MAP()


/////////////////////////////////////////////////////////////////////////////
// Event map

BEGIN_EVENT_MAP(CMSDEVIntegratorCtrl, COleControl)
	//{{AFX_EVENT_MAP(CMSDEVIntegratorCtrl)
	EVENT_CUSTOM("CtrlClick", FireCtrlClick, VTS_BSTR  VTS_I4  VTS_I4  VTS_I4)
	EVENT_CUSTOM("Query", FireQuery, VTS_BSTR  VTS_I4  VTS_I4  VTS_I4)
	EVENT_CUSTOM("Definition", FireDefinition, VTS_BSTR  VTS_I4  VTS_I4  VTS_I4)
	EVENT_CUSTOM("Activate", FireActivate, VTS_NONE)
	//}}AFX_EVENT_MAP
END_EVENT_MAP()


/////////////////////////////////////////////////////////////////////////////
// Property pages

// TODO: Add more property pages as needed.  Remember to increase the count!
BEGIN_PROPPAGEIDS(CMSDEVIntegratorCtrl, 1)
	PROPPAGEID(CMSDEVIntegratorPropPage::guid)
END_PROPPAGEIDS(CMSDEVIntegratorCtrl)


/////////////////////////////////////////////////////////////////////////////
// Initialize class factory and guid

IMPLEMENT_OLECREATE_EX(CMSDEVIntegratorCtrl, "MSDEVINTEGRATOR.MSDEVIntegratorCtrl.1",
	0xaa00dc08, 0x36de, 0x11d2, 0xaf, 0xf, 0, 0xa0, 0xc9, 0xb7, 0x1d, 0xc4)


/////////////////////////////////////////////////////////////////////////////
// Type library ID and version

IMPLEMENT_OLETYPELIB(CMSDEVIntegratorCtrl, _tlid, _wVerMajor, _wVerMinor)


/////////////////////////////////////////////////////////////////////////////
// Interface IDs

const IID BASED_CODE IID_DMSDEVIntegrator =
		{ 0xc5267913, 0x392e, 0x11d2, { 0xaf, 0x11, 0, 0xa0, 0xc9, 0xb7, 0x1d, 0xc4 } };
const IID BASED_CODE IID_DMSDEVIntegratorEvents =
		{ 0xc5267914, 0x392e, 0x11d2, { 0xaf, 0x11, 0, 0xa0, 0xc9, 0xb7, 0x1d, 0xc4 } };


/////////////////////////////////////////////////////////////////////////////
// Control type information

static const DWORD BASED_CODE _dwMSDEVIntegratorOleMisc =
	OLEMISC_ACTIVATEWHENVISIBLE |
	OLEMISC_SETCLIENTSITEFIRST |
	OLEMISC_INSIDEOUT |
	OLEMISC_CANTLINKINSIDE |
	OLEMISC_RECOMPOSEONRESIZE;

IMPLEMENT_OLECTLTYPE(CMSDEVIntegratorCtrl, IDS_MSDEVINTEGRATOR, _dwMSDEVIntegratorOleMisc)


/////////////////////////////////////////////////////////////////////////////
// CMSDEVIntegratorCtrl::CMSDEVIntegratorCtrlFactory::UpdateRegistry -
// Adds or removes system registry entries for CMSDEVIntegratorCtrl

BOOL CMSDEVIntegratorCtrl::CMSDEVIntegratorCtrlFactory::UpdateRegistry(BOOL bRegister)
{
	// TODO: Verify that your control follows apartment-model threading rules.
	// Refer to MFC TechNote 64 for more information.
	// If your control does not conform to the apartment-model rules, then
	// you must modify the code below, changing the 6th parameter from
	// afxRegInsertable | afxRegApartmentThreading to afxRegInsertable.

	if (bRegister)
		return AfxOleRegisterControlClass(
			AfxGetInstanceHandle(),
			m_clsid,
			m_lpszProgID,
			IDS_MSDEVINTEGRATOR,
			IDB_MSDEVINTEGRATOR,
			afxRegInsertable | afxRegApartmentThreading,
			_dwMSDEVIntegratorOleMisc,
			_tlid,
			_wVerMajor,
			_wVerMinor);
	else
		return AfxOleUnregisterClass(m_clsid, m_lpszProgID);
}


/////////////////////////////////////////////////////////////////////////////
// CMSDEVIntegratorCtrl::CMSDEVIntegratorCtrl - Constructor

CMSDEVIntegratorCtrl::CMSDEVIntegratorCtrl()
{
	InitializeIIDs(&IID_DMSDEVIntegrator, &IID_DMSDEVIntegratorEvents);

	// TODO: Initialize your control's instance data here.
}


/////////////////////////////////////////////////////////////////////////////
// CMSDEVIntegratorCtrl::~CMSDEVIntegratorCtrl - Destructor

CMSDEVIntegratorCtrl::~CMSDEVIntegratorCtrl()
{
	// TODO: Cleanup your control's instance data here.
}


/////////////////////////////////////////////////////////////////////////////
// CMSDEVIntegratorCtrl::OnDraw - Drawing function

void CMSDEVIntegratorCtrl::OnDraw(
			CDC* pdc, const CRect& rcBounds, const CRect& rcInvalid)
{
	// TODO: Replace the following code with your own drawing code.
	pdc->FillRect(rcBounds, CBrush::FromHandle((HBRUSH)GetStockObject(WHITE_BRUSH)));
	pdc->Ellipse(rcBounds);
}


/////////////////////////////////////////////////////////////////////////////
// CMSDEVIntegratorCtrl::DoPropExchange - Persistence support

void CMSDEVIntegratorCtrl::DoPropExchange(CPropExchange* pPX)
{
	ExchangeVersion(pPX, MAKELONG(_wVerMinor, _wVerMajor));
	COleControl::DoPropExchange(pPX);

	// TODO: Call PX_ functions for each persistent custom property.

}


/////////////////////////////////////////////////////////////////////////////
// CMSDEVIntegratorCtrl::OnResetState - Reset control to default state

void CMSDEVIntegratorCtrl::OnResetState()
{
	COleControl::OnResetState();  // Resets defaults found in DoPropExchange

	// TODO: Reset any other control state here.
}


/////////////////////////////////////////////////////////////////////////////
// CMSDEVIntegratorCtrl::AboutBox - Display an "About" box to the user

void CMSDEVIntegratorCtrl::AboutBox()
{
	CDialog dlgAbout(IDD_ABOUTBOX_MSDEVINTEGRATOR);
	dlgAbout.DoModal();
}


/////////////////////////////////////////////////////////////////////////////
// CMSDEVIntegratorCtrl message handlers

void CMSDEVIntegratorCtrl::OpenFile(LPCTSTR filename, long line)  {
   dis_integration_open_file ((char *)filename,line);
}

//----------------------------------------------------------------------------------------
// This METHOD will open in the DeveloperStudio editor the file with the filename
// "filename" and will make selection in it starting at the line "line" at the position
// "column" strlen(token) character length.
//----------------------------------------------------------------------------------------
void CMSDEVIntegratorCtrl::MakeSelection(LPCTSTR filename, 
										 long line, 
										 long column, 
										 LPCTSTR token)  {
   dis_integration_make_selection((char *)filename, (int)line, (int)column, (char *)token) ;

}
//---------------------------------------------------------------------------------------


//---------------------------------------------------------------------------------------
// This METHOD returns current selection:as follows:
// 1.Currently active file name
// 2.Selection line number
// 3.Selection position
// 4.Selected token
//---------------------------------------------------------------------------------------
void CMSDEVIntegratorCtrl::GetSelection(BSTR FAR* filename, 
										BSTR FAR* token, 
										long FAR* line, 
										long FAR* column)  {
CString file;
CString tok;

    dis_integration_read_selection();
    *line=dis_integration_getsel_line();
    *column=dis_integration_getsel_col();
	file=dis_integration_getsel_fname();
    *filename=file.AllocSysString();
	tok=dis_integration_getsel_token();
	*token=tok.AllocSysString();
}
//----------------------------------------------------------------------------------------



///---------------------------------------------------------------------------------------
// This METHOD returns currently selected Developer Studio Editor TAB size.
//----------------------------------------------------------------------------------------
short CMSDEVIntegratorCtrl::GetTabSize()  {
   return (short) dis_integration_get_tabsize();
}
//----------------------------------------------------------------------------------------


//----------------------------------------------------------------------------------------
// We will query nameserver service when creating the integrator ActiveX for the 
// integrator. If OK, we will open a socket "m_IntegratorServerConnection" which will
// inform us about Ctrl+Click event in the DeveloperStudio. (By sendisg ' ').
//----------------------------------------------------------------------------------------
int CMSDEVIntegratorCtrl::OnCreate(LPCREATESTRUCT lpCreateStruct)  {
	if (COleControl::OnCreate(lpCreateStruct) == -1)
		return -1;
	switch(ReceveIntegrationService()) {
	    case  -1 : AfxMessageBox("Can't connect to the DIScover Name Server"); return 0;
	    case  0  : AfxMessageBox("Can't connect to the DIScover Visual Studio Integrator"); return 0;
	}
	AfxSocketInit();
	// Opening integration socket
    m_IntegratorServerConnection.SetParent(this);
    m_IntegratorServerConnection.Create();
    int res = m_IntegratorServerConnection.Connect((SOCKADDR *)&m_WhereIntegrator,sizeof(m_WhereIntegrator));
	res=CSocket::GetLastError();
	return 0;
}
//----------------------------------------------------------------------------------------



//----------------------------------------------------------------------------------------
// This private function will receve integrator IP address and port.
// It will return FALSE if no integrator detected.
// It will receve TRUE and it will fill m_WhereIntegrator structure with port and IP 
// address of the integrator if everything is OK.
// RUN THIS FUNCTION DIRECTLY AFTER QueryIntegrationService
//----------------------------------------------------------------------------------------
int CMSDEVIntegratorCtrl::ReceveIntegrationService() {
char* serviceName="Discover:Integrator";
CString fullName;
int port;
int address;

     fullName=serviceName;
	 fullName+=":";
	 char hostName[1024];
	 gethostname(hostName,1024);
	 fullName+=hostName;
     int res = NameServerGetService(fullName.GetBuffer(20), port, address);
	 memset(&m_WhereIntegrator,0,sizeof(SOCKADDR_IN));
	 m_WhereIntegrator.sin_family=AF_INET;
	 m_WhereIntegrator.sin_port=htons(port);

	 char buf[1024];
     gethostname(buf, 1024);
     static struct hostent *ent = gethostbyname(buf);
	 m_WhereIntegrator.sin_addr=*(in_addr *)*ent->h_addr_list;
	 return res;


}
//----------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------
// When integration window is closing it will close socket to the integrator
//----------------------------------------------------------------------------------------
void CMSDEVIntegratorCtrl::OnClose()  {
	
    m_IntegratorServerConnection.Close();
	CWnd::OnClose();
}
//---------------------------------------------------------------------------------------


//----------------------------------------------------------------------------------------
// Call this METHOD if you want to display text string "info" in the Developer Studio
// output window.
//----------------------------------------------------------------------------------------
void CMSDEVIntegratorCtrl::SetInfo(LPCTSTR info)  {
register unsigned i;
unsigned char byte;
char *p = (char *)info;


	 for(i=0;i<strlen(p);i++) {
		 if(p[i]=='[') p[i]='<';
		 if(p[i]==']') p[i]='>';
	 }

     m_IntegratorServerConnection.Send(signature,4);
     byte=INFO_OPCODE;
	 m_IntegratorServerConnection.Send(&byte,1);

	 int len = strlen(p);
#error Change to network byte order everywhere in this file!
	 byte = (len & 255);
	 m_IntegratorServerConnection.Send((const char *)&byte,1);

	 byte = (len>>8) & 255;
	 m_IntegratorServerConnection.Send((const char *)&byte,1);

	 byte = (len>>16) & 255;
	 m_IntegratorServerConnection.Send((const char *)&byte,1);

	 byte = (len>>24) & 255;

	 m_IntegratorServerConnection.Send((const char *)&byte,1);
	 m_IntegratorServerConnection.Send(p,len);
}
//----------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------
// Call this METHOD if yoi want to send command to the integrator.
//----------------------------------------------------------------------------------------
void CMSDEVIntegratorCtrl::Command(LPCTSTR string)  {
unsigned char byte;

	 m_IntegratorServerConnection.Send(signature,4);
     byte=COMMAND_OPCODE;
	 m_IntegratorServerConnection.Send(&byte,1);
	 int len = strlen(string);
	 byte = len & 255;
	 m_IntegratorServerConnection.Send(&byte,1);
	 byte = (len>>8) & 255;
	 m_IntegratorServerConnection.Send(&byte,1);
	 byte = (len>>16) & 255;
	 m_IntegratorServerConnection.Send(&byte,1);
	 byte = (len>>24) & 255;
	 m_IntegratorServerConnection.Send(&byte,1);
	 m_IntegratorServerConnection.Send(string,len);
}
//----------------------------------------------------------------------------------------


//----------------------------------------------------------------------------------------
// Call this METHOD if you want to inform integrator that you are exiting
//----------------------------------------------------------------------------------------
void CMSDEVIntegratorCtrl::Exit()  {
unsigned char byte;
char* info = "";

	 m_IntegratorServerConnection.Send(signature,4);
     byte=EXIT_OPCODE;
	 m_IntegratorServerConnection.Send(&byte,1);
	 int len = strlen(info);
	 byte = len & 255;
	 m_IntegratorServerConnection.Send(&byte,1);
	 byte = (len>>8) & 255;
	 m_IntegratorServerConnection.Send(&byte,1);
	 byte = (len>>16) & 255;
	 m_IntegratorServerConnection.Send(&byte,1);
	 byte = (len>>24) & 255;
	 m_IntegratorServerConnection.Send(&byte,1);
	 m_IntegratorServerConnection.Send(info,len);
}
//----------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------
// This private function is called when CtrlClick command comes to us from integrator,
// We are forming information strings and send them in event.
//----------------------------------------------------------------------------------------
void CMSDEVIntegratorCtrl::CtrlClickProcessor(void) {
LPCTSTR pFile;
LPCTSTR pToken;
int line;
int column;

    dis_integration_read_selection();
    pFile  = dis_integration_getsel_fname();
	pToken = dis_integration_getsel_token();
	line=dis_integration_getsel_line();
	column=dis_integration_getsel_col();
	FireCtrlClick(pFile,line,column,strlen(pToken));
}
//----------------------------------------------------------------------------------------


//----------------------------------------------------------------------------------------
// This private function is called when "Query" command comes to us from integrator,
// We are forming information strings and send them in event.
//----------------------------------------------------------------------------------------
void CMSDEVIntegratorCtrl::QueryProcessor(void) {
LPCTSTR pFile;
LPCTSTR pToken;

    dis_integration_read_selection();
    pFile  = dis_integration_getsel_fname();
	pToken = dis_integration_getsel_token();
	FireQuery(pFile,dis_integration_getsel_line(),dis_integration_getsel_col(),strlen(pToken));
}
//----------------------------------------------------------------------------------------


//----------------------------------------------------------------------------------------
// This private function is called when "OpenDefinition" command comes to us from integrator,
// We are forming information strings and send them in event.
//----------------------------------------------------------------------------------------
void CMSDEVIntegratorCtrl::DefinitionProcessor(void) {
LPCTSTR pFile;
LPCTSTR pToken;

    dis_integration_read_selection();
    pFile  = dis_integration_getsel_fname();
	pToken = dis_integration_getsel_token();
	FireDefinition(pFile,dis_integration_getsel_line(),dis_integration_getsel_col(),strlen(pToken));
}
//----------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------
// This private function is called when "Activate" command comes to us from integrator,
//----------------------------------------------------------------------------------------
void CMSDEVIntegratorCtrl::ActivateProcessor(void) {
	FireActivate();
}
//----------------------------------------------------------------------------------------



