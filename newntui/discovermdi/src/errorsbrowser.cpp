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
// ErrorsBrowser.cpp : implementation file
//

#include "stdafx.h"
#include "afxpriv.h"
#include "discovermdi.h"
#include "ErrorsBrowser.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define HEADER 10
#define BORDER 2
#define SASH_SPACE 4


/////////////////////////////////////////////////////////////////////////////
// CErrorsBrowser dialog


CErrorsBrowser::CErrorsBrowser() {
	//{{AFX_DATA_INIT(CErrorsBrowser)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_SpyRunning = FALSE;
	m_OnSize=FALSE;
	m_SpyThread = NULL;
}



BEGIN_MESSAGE_MAP(CErrorsBrowser, CDialogBar)
	//{{AFX_MSG_MAP(CErrorsBrowser)
	ON_WM_CREATE()
	ON_WM_PAINT()
	ON_WM_SIZE()
	ON_MESSAGE(WM_SASHMOVED,OnSashMoved)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CErrorsBrowser message handlers


int CErrorsBrowser::OnCreate(LPCREATESTRUCT lpCreateStruct)  {
	if (CDialogBar::OnCreate(lpCreateStruct) == -1)
		return -1;
	m_RightSash.Create("", WS_CHILD | WS_VISIBLE | SS_NOTIFY, CRect(0,0,0,0),this,20000);
	m_RightSash.ModifyStyleEx(0,WS_EX_DLGMODALFRAME);
	m_LeftSash.Create("", WS_CHILD | WS_VISIBLE | SS_NOTIFY, CRect(0,0,0,0),this,20001);
	m_LeftSash.ModifyStyleEx(0,WS_EX_DLGMODALFRAME);
	m_TopSash.Create("", WS_CHILD | WS_VISIBLE | SS_NOTIFY, CRect(0,0,0,0),this,20002);
	m_TopSash.ModifyStyleEx(0,WS_EX_DLGMODALFRAME);
	m_BottomSash.Create("", WS_CHILD  | WS_VISIBLE | SS_NOTIFY, CRect(0,0,0,0),this,20003);
	m_BottomSash.ModifyStyleEx(0,WS_EX_DLGMODALFRAME);
	return 0;
}


//--------------------------------------------------------------------------------
// This function returns the current side of the frame window to which this 
// dockable bar docked. The values are DOCK_LEFT, DOCK_RIGHT, DOCK_TOP, DOCK_BOTTOM.
//--------------------------------------------------------------------------------
int CErrorsBrowser::GetDockSide() {
	if(IsFloating()) return DOCK_FLOAT;
    if(m_pDockBar==GetDockingFrame()->GetControlBar(AFX_IDW_DOCKBAR_TOP) )    
		return DOCK_TOP;
    if(m_pDockBar==GetDockingFrame()->GetControlBar(AFX_IDW_DOCKBAR_BOTTOM) ) 
		return DOCK_BOTTOM;
    if(m_pDockBar==GetDockingFrame()->GetControlBar(AFX_IDW_DOCKBAR_LEFT) )   
		return DOCK_LEFT;
    if(m_pDockBar==GetDockingFrame()->GetControlBar(AFX_IDW_DOCKBAR_RIGHT) )  
		return DOCK_RIGHT;
	return -1;
}
//--------------------------------------------------------------------------------


//--------------------------------------------------------------------------------
// This function will position all controls in the dockable window according to 
// the  current window size.
//--------------------------------------------------------------------------------
void CErrorsBrowser::LayoutControls() {
    if(!::IsWindow(*this)) return;
	m_RightSash.SetContextWindow(GetParent()->GetParent());
	m_LeftSash.SetContextWindow(GetParent()->GetParent());
	m_TopSash.SetContextWindow(GetParent()->GetParent());
	m_BottomSash.SetContextWindow(GetParent()->GetParent());
    SetControlsPos();
}
//--------------------------------------------------------------------------------



//---------------------------------------------------------------------------------
// This function will position dockable window sash bar to the sizable edge
// according to the window sizes and docking side.
//---------------------------------------------------------------------------------
void CErrorsBrowser::PositionSashBars() {
       
	m_sizeDefault.cx-=3;
	m_sizeDefault.cy-=3;
    switch(GetDockSide()) {
	       case DOCK_LEFT : m_LeftSash.ShowWindow(SW_HIDE);
			                m_RightSash.ShowWindow(SW_SHOW);
						    m_TopSash.ShowWindow(SW_HIDE);
						    m_BottomSash.ShowWindow(SW_HIDE);
	                        m_RightSash.MoveWindow(m_sizeDefault.cx-5, 0, 6, m_sizeDefault.cy);
							m_RightSash.Invalidate();
							break;
	       case DOCK_RIGHT: m_LeftSash.ShowWindow(SW_SHOW);
			                m_RightSash.ShowWindow(SW_HIDE);
						    m_TopSash.ShowWindow(SW_HIDE);
						    m_BottomSash.ShowWindow(SW_HIDE);
	                        m_LeftSash.MoveWindow(1, 0, 6, m_sizeDefault.cy);
							m_LeftSash.Invalidate();
							break;
	       case DOCK_TOP :  m_LeftSash.ShowWindow(SW_HIDE);
			                m_RightSash.ShowWindow(SW_HIDE);
						    m_TopSash.ShowWindow(SW_HIDE);
						    m_BottomSash.ShowWindow(SW_SHOW);
	                        m_BottomSash.MoveWindow(0, m_sizeDefault.cy-5, m_sizeDefault.cx,6);
							m_BottomSash.Invalidate();
							break;
	       case DOCK_BOTTOM:m_LeftSash.ShowWindow(SW_HIDE);
			                m_RightSash.ShowWindow(SW_HIDE);
						    m_TopSash.ShowWindow(SW_SHOW);
						    m_BottomSash.ShowWindow(SW_HIDE);
	                        m_TopSash.MoveWindow(0, 1, m_sizeDefault.cx,6);
							m_TopSash.Invalidate();
							break;
	       case DOCK_FLOAT:m_LeftSash.ShowWindow(SW_HIDE);
			                m_RightSash.ShowWindow(SW_HIDE);
						    m_TopSash.ShowWindow(SW_HIDE);
						    m_BottomSash.ShowWindow(SW_HIDE);
							break;
	}
	m_sizeDefault.cx+=3;
	m_sizeDefault.cy+=3;

}
//---------------------------------------------------------------------------------




//---------------------------------------------------------------------------------
// This function will calculate variable size of the dockable window.
// (This dimention can be changed by shifting a splitter)
//---------------------------------------------------------------------------------
void CErrorsBrowser::CalculateVariableSize() {
CRect frameRect;
      if(!::IsWindow(*this)) return;
       GetParent()->GetWindowRect(&frameRect); 
	   switch(GetDockSide()) {
	       case DOCK_LEFT : m_sizeDefault.cy=frameRect.Height()+3;
			                break;
	       case DOCK_RIGHT: m_sizeDefault.cy=frameRect.Height()+3;
			                break;
	       case DOCK_TOP :  m_sizeDefault.cx=frameRect.Width()+3;						
			                break;
	       case DOCK_BOTTOM:m_sizeDefault.cx=frameRect.Width()+3;
			                break;
	   }
}
//---------------------------------------------------------------------------------

//---------------------------------------------------------------------------------
// This function will calculate fixed size of the dockable window.
// (This dimention depends only from the frame size)
//---------------------------------------------------------------------------------
void CErrorsBrowser::CalculateFixedSize() {
CRect frameRect;
      
       GetParent()->GetWindowRect(&frameRect); 
	   switch(GetDockSide()) {
	       case DOCK_LEFT : m_sizeDefault.cx=100;
			                break;
	       case DOCK_RIGHT: m_sizeDefault.cx=100;
			                break;
	       case DOCK_TOP :  m_sizeDefault.cy=100;
			                break;
	       case DOCK_BOTTOM:m_sizeDefault.cy=100;
			                break;
	   }
}
//---------------------------------------------------------------------------------


//--------------------------------------------------------------------------------
// This function will position all sash bars. If initialize flag set the fixed size
// of the dockablle window will be calculated
//--------------------------------------------------------------------------------
void CErrorsBrowser::UpdateSashes(BOOL initialize) {
    CalculateVariableSize();
	if(initialize) {
		CalculateFixedSize();
	}
    GetDockingFrame()->RecalcLayout(); 
	PositionSashBars();
	LayoutControls();
}
//--------------------------------------------------------------------------------

//--------------------------------------------------------------------------------
// Paint will aloso update sashes positions if the docking stater changed.
//--------------------------------------------------------------------------------
void CErrorsBrowser::OnPaint()  {
     CPaintDC dc(this);  

					 
	 if(m_DockSide!=GetDockSide()) {
		 UpdateSashes(TRUE);
		 m_DockSide=GetDockSide();
	 }

	 /*CBrush bkgnd( GetSysColor(COLOR_3DLIGHT));
	 dc.FillRect(CRect(0,0,m_sizeDefault.cx,HEADER),&bkgnd);*/

	 switch(GetDockSide()) {
	       case DOCK_LEFT :  
	                      dc.Draw3dRect(  BORDER, 
		                                  HEADER/2-2, 
					                      m_sizeDefault.cx-2*BORDER-SASH_SPACE, 
		                                  3, 
					                      GetSysColor(COLOR_3DHILIGHT),
					                      GetSysColor(COLOR_3DSHADOW)
				                        );
					 
	                      dc.Draw3dRect(  BORDER, 
		                                  HEADER/2+2, 
					                      m_sizeDefault.cx-2*BORDER-SASH_SPACE, 
		                                  3, 
					                      GetSysColor(COLOR_3DHILIGHT),
					                      GetSysColor(COLOR_3DSHADOW)
				            );

			                break;
	       case DOCK_RIGHT :  
	                      dc.Draw3dRect(  BORDER+SASH_SPACE, 
		                                  HEADER/2-2, 
					                      m_sizeDefault.cx-2*BORDER-SASH_SPACE, 
		                                  3, 
					                      GetSysColor(COLOR_3DHILIGHT),
					                      GetSysColor(COLOR_3DSHADOW)
				                        );
					 
	                      dc.Draw3dRect(  BORDER+SASH_SPACE, 
		                                  HEADER/2+2, 
					                      m_sizeDefault.cx-2*BORDER-SASH_SPACE, 
		                                  3, 
					                      GetSysColor(COLOR_3DHILIGHT),
					                      GetSysColor(COLOR_3DSHADOW)
				            );

			                break;
	       case DOCK_TOP :  
	                      dc.Draw3dRect(  BORDER, 
		                                  HEADER/2-2, 
					                      m_sizeDefault.cx-2*BORDER, 
		                                  3, 
					                      GetSysColor(COLOR_3DHILIGHT),
					                      GetSysColor(COLOR_3DSHADOW)
				                        );
					 
	                      dc.Draw3dRect(  BORDER, 
		                                  HEADER/2+2, 
					                      m_sizeDefault.cx-2*BORDER, 
		                                  3, 
					                      GetSysColor(COLOR_3DHILIGHT),
					                      GetSysColor(COLOR_3DSHADOW)
				            );

			                break;
	       case DOCK_BOTTOM :  
	                      dc.Draw3dRect(  BORDER, 
		                                  HEADER/2-2+SASH_SPACE, 
					                      m_sizeDefault.cx-2*BORDER, 
		                                  3, 
					                      GetSysColor(COLOR_3DHILIGHT),
					                      GetSysColor(COLOR_3DSHADOW)
				                        );
					 
	                      dc.Draw3dRect(  BORDER, 
		                                  HEADER/2+2+SASH_SPACE, 
					                      m_sizeDefault.cx-2*BORDER, 
		                                  3, 
					                      GetSysColor(COLOR_3DHILIGHT),
					                      GetSysColor(COLOR_3DSHADOW)
				            );

			                break;
	   }
     m_OnSize=TRUE;

}
//--------------------------------------------------------------------------------



//--------------------------------------------------------------------------------
//--------------------------------------------------------------------------------
void CErrorsBrowser::SetControlsPos()  {

	CRichEditCtrl*     ErrorLog;
	ErrorLog=(CRichEditCtrl*)      GetDlgItem(IDC_ERRORSLOG);
	if(ErrorLog==NULL) return;
    ErrorLog->SetReadOnly();
	m_sizeDefault.cx-=3;
	m_sizeDefault.cy-=3;
    switch(GetDockSide()) {
	   case DOCK_LEFT  : ErrorLog      -> MoveWindow(BORDER,
		                                            HEADER+BORDER,
		                                            m_sizeDefault.cx-2*BORDER-SASH_SPACE,
								                    m_sizeDefault.cy-2*BORDER-HEADER);
		                 break;
	   case DOCK_RIGHT : ErrorLog      -> MoveWindow(BORDER+SASH_SPACE,
		                                             HEADER+BORDER,
		                                             m_sizeDefault.cx-2*BORDER-SASH_SPACE,
								                     m_sizeDefault.cy-2*BORDER-HEADER);
		                 break;
	   case DOCK_TOP   : ErrorLog      -> MoveWindow(BORDER,
		                                             HEADER+BORDER,
		                                             m_sizeDefault.cx-2*BORDER,
								                     m_sizeDefault.cy-2*BORDER-HEADER-SASH_SPACE);
		                 break;
	   case DOCK_BOTTOM: ErrorLog      -> MoveWindow(BORDER,
		                                             HEADER+BORDER+SASH_SPACE,
		                                             m_sizeDefault.cx-2*BORDER,
								                     m_sizeDefault.cy-2*BORDER-HEADER-SASH_SPACE);
		                 break;
	   default         : ErrorLog      -> MoveWindow(BORDER,
		                                             HEADER+BORDER,
		                                             m_sizeDefault.cx-2*BORDER,
								                     m_sizeDefault.cy-2*BORDER-HEADER);
	}
	ErrorLog->Invalidate(TRUE);
	m_sizeDefault.cx+=3;
	m_sizeDefault.cy+=3;
}
//--------------------------------------------------------------------------------



//--------------------------------------------------------------------------------
void CErrorsBrowser::OnSize(UINT nType, int cx, int cy) {
	LayoutControls();
}


//---------------------------------------------------------------------------------
// Callback runs every time  sash changed it's position.
// We need to resize our dockable window, position all controls in it and update
// our sash position.
//---------------------------------------------------------------------------------
LRESULT CErrorsBrowser::OnSashMoved(WPARAM wparam, LPARAM lparam) {
CRect winRect;
CRect sashRect;
int x = wparam;
int y= lparam;

    // Calculating new window sizes from the nes sash position
    GetWindowRect(&winRect);
    switch(GetDockSide()) {
	    case DOCK_LEFT : m_RightSash.GetWindowRect(&sashRect);
			             m_sizeDefault.cx=x+(sashRect.left-winRect.left)+6;
			             break;
	    case DOCK_RIGHT: m_LeftSash.GetWindowRect(&sashRect);
			             m_sizeDefault.cx=(winRect.right-sashRect.right)-x+6;
			             break;
	    case DOCK_TOP :  m_BottomSash.GetWindowRect(&sashRect);
			             m_sizeDefault.cy=y+(sashRect.top-winRect.top)+6;
			             break;
	    case DOCK_BOTTOM:m_TopSash.GetWindowRect(&sashRect);
			             m_sizeDefault.cy=(winRect.bottom-sashRect.bottom)-y+6;
			             break;
	}
    SetControlsPos();
    // Setting new position for the sash bar
	PositionSashBars();
	// Re-calculating frame window dockables layout.
    GetDockingFrame()->RecalcLayout(); 
	Invalidate();
	return TRUE;
}
//---------------------------------------------------------------------------------

void CErrorsBrowser::SetErrorLog(const CString &log) {
CRichEditCtrl*     ErrorLog;
ErrorLog=(CRichEditCtrl*)      GetDlgItem(IDC_ERRORSLOG);
   ErrorLog->SetWindowText(log);
}



//-----------------------------------------------------------------------------------
// This callback will read the file we'v trying to spy and place the results into 
// the buffer, Edit control will use it later.
//-----------------------------------------------------------------------------------
DWORD CALLBACK LogStreamReadCallback(DWORD dwCookie,LPBYTE pbBuff,LONG cb,LONG *pcb) {
CArchive* storage = (CArchive *) dwCookie;
int i=0;
char ch;

       char *p = (char *)pbBuff;
	   TRY {
		      for(i=0;i<cb;i++)  {
				 (*storage) >> ch;
				// if(ch!='\r')
			         *p++=ch;
			  }
	   } CATCH(CArchiveException, ae) {
		    // End of file found
		    *pcb=i;
		    return 0;
	   }
       END_CATCH
	   *pcb=i;
	   return 0;
}
//-----------------------------------------------------------------------------------


//------------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------------
void CErrorsBrowser::UpdateFile(void) {
CFile file;
CRichEditCtrl*     ErrorLog;

      ErrorLog=(CRichEditCtrl*)      GetDlgItem(IDC_ERRORSLOG);
      if(file.Open(m_LogFile, CFile::modeRead | CFile::shareDenyNone) ) {
		  int len = file.GetLength();
		  if(len!=m_OldFileLength) {
			  m_OldFileLength=len;
              CArchive storage(&file,CArchive::load);
		      EDITSTREAM es;
		      es.dwCookie=(DWORD) &storage;
		      es.pfnCallback=&LogStreamReadCallback;
              CHARFORMAT cf;
	          cf.cbSize=sizeof(CHARFORMAT);
		      cf.dwMask = CFM_BOLD | CFM_COLOR | CFM_FACE | CFM_ITALIC | CFM_SIZE | CFM_STRIKEOUT | CFM_UNDERLINE;
		      cf.dwEffects=0;
		      cf.yHeight=200;
		      cf.crTextColor=RGB(0,0,0);
		      strcpy(cf.szFaceName,"Arial");
		      ErrorLog->StreamIn(SF_TEXT,es);
		  }
	  }
}
//------------------------------------------------------------------------------------




DWORD WINAPI SpyRunThread(LPVOID param) {
CErrorsBrowser* browser = (CErrorsBrowser *) param;
     while(browser->m_SpyRunning==TRUE) {
          browser->UpdateFile();
	 }
	 return 0;
}


void CErrorsBrowser::StartFileLog(const CString &filename) {
DWORD code;
DWORD dwThreadId;
    SetErrorLog("");
    m_OldFileLength=0;
    m_LogFile=filename;
//	m_SpyRunning=TRUE;
//    m_SpyThread=::CreateThread(NULL,0,SpyRunThread,(LPVOID)this,0,&dwThreadId);
//	::SetThreadPriority(m_SpyThread,THREAD_PRIORITY_LOWEST);
}

void CErrorsBrowser::StopFileLog() {
	m_SpyRunning=FALSE;
	UpdateFile();
	WaitForSingleObject(m_SpyThread,INFINITE);
	CloseHandle(m_SpyThread);
}

