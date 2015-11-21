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
// AccessEdit.cpp : implementation file
//

#include "stdafx.h"
#include "discovermdi.h"
#include "AccessEdit.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


CVisibleBuffer::CVisibleBuffer(int maxLines) {
	m_MaxLines=maxLines;
	m_LinesInBuffer=0;
	m_VisibleBuffer="";
}
	  
void CVisibleBuffer::AppendBlock(CString& data) {
register i;
int curPos;
int bLen;
int lNum;

     bLen=data.GetLength();
	 if(bLen==0) return;

     curPos=bLen-1;

	 // If the block ends with a newline, we need to bypass it 
	 // because of the % prompt
	 if(data[curPos]=='\n' && curPos>0) {
		 curPos--;
		 bLen--;
	 }

	 // Calculating the line amount in the block - 
	 // stop if > max available GetExitCode
     lNum=0;
	 while((lNum<m_MaxLines) && (curPos>0)) {
		 if(data[curPos]=='\n') {
			 lNum++;
		 }
		 curPos--;
	 }
	 lNum++;
	 if(curPos>0) curPos+=2; // curPos will point to the first 
	                         // non-newline characrer.

	 // Case 1 : the block of data is big to replace all visible buffer;
	 if(lNum>=m_MaxLines) {
		 m_VisibleBuffer="";
		 int l=0;
		 for(i=curPos;i<bLen;i++) {
			 if(data[i]=='\n') {
				 m_VisibleBuffer+="\r\n";
				 l++;
			 } else {
			     if(data[i]==8) m_VisibleBuffer+="\t";
				    else  m_VisibleBuffer+=data[i];
			 }
		 }
		 m_LinesInBuffer=l;
		 return;
	 }
	 // Case 2 : the block of data is small to fit in the rest of space
	 if(lNum<(m_MaxLines-m_LinesInBuffer)) {
		 int l=0;
		 for(i=curPos;i<bLen;i++) {
			 if(data[i]=='\n') {
				 l++;
				 m_VisibleBuffer+="\r\n";
			 } else {
			     if(data[i]==8) m_VisibleBuffer+="\t";
				    else  m_VisibleBuffer+=data[i];
			 }
		 }
		 m_LinesInBuffer+=l;
		 return;
	 }
	 // Case 3 : we need to remove some lines to fil the new block
	 if((lNum+m_LinesInBuffer)>=m_MaxLines) {
		 // Lines to delete;
		 int delLines=(m_LinesInBuffer+lNum+1)-m_MaxLines;
		 m_LinesInBuffer-=delLines;
		 // Calculating position to which we need to delete
		 int delPos=0;
		 int dd=0;
		 while(delPos<m_VisibleBuffer.GetLength() && dd<delLines) {
			 if(m_VisibleBuffer[delPos]=='\n') {
				  lNum++;
				  dd++;
			 }
		     delPos++;
		 }

		 // Removing lines
		 m_VisibleBuffer=m_VisibleBuffer.Right(m_VisibleBuffer.GetLength()-delPos);

		 // Inserting our data block
		 int ll=0;
		 for(i=curPos;i<bLen;i++) {
			 if(data[i]=='\n') {
				 m_VisibleBuffer+="\r\n";
				 ll++;
			 } else {
			     if(data[i]==8) m_VisibleBuffer+="\t";
				    else  m_VisibleBuffer+=data[i];
			 }
		 }
		 m_LinesInBuffer+=ll;
	 }
}


CString& CVisibleBuffer::GetVisibleBuffer(void) {
	return m_VisibleBuffer;
}




/////////////////////////////////////////////////////////////////////////////
// CAccessEdit

CAccessEdit::CAccessEdit() {
	m_Visible=new CVisibleBuffer(2000);
	m_Command="";
	m_CommandPos=0;
	m_InHistory=0;
	m_HistoryCount=-1;
	m_InSelectionMode=FALSE;
	m_CtrlPressed=FALSE;
	m_MouseLock=FALSE;
	m_UserFont=NULL;
}

CAccessEdit::~CAccessEdit() {
	delete m_Visible;
	if(m_UserFont!=NULL) {
		delete m_UserFont;
		m_UserFont=NULL;
	}
}


BEGIN_MESSAGE_MAP(CAccessEdit, CEdit)
	//{{AFX_MSG_MAP(CAccessEdit)
	ON_WM_KEYDOWN()
	ON_WM_KEYUP()
	ON_WM_CHAR()
	ON_WM_LBUTTONUP()
	ON_WM_LBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_WM_RBUTTONDOWN()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAccessEdit message handlers

void CAccessEdit::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)  {
register i;
unsigned short ascii;
unsigned char state[256];
CString s;

	m_Lock=FALSE;
    switch(nChar) {
	   // Shift
	   case 16 :  m_ShiftPressed=TRUE;
		          m_Lock=TRUE;
		          break;
	   // Ctrl
	   case 17 :  m_CtrlPressed=TRUE;
		          m_Lock=TRUE;
		          break;
	   // Backspace
	   case 8 :   for(i=0;i<(int)nRepCnt;i++) {
		             if(m_CommandPos>0) {
					      CString left  = m_Command.Left(m_CommandPos-1);
					      CString right = m_Command.Right(m_Command.GetLength()-m_CommandPos);
			              m_Command=left;
					      m_Command+=right;
						  m_CommandPos--;
	                      CEdit::OnKeyDown(nChar, nRepCnt, nFlags);
					 } else {
					      m_Lock=TRUE;
					      MessageBeep(MB_ICONEXCLAMATION);
						  break;
					 }
				  }
				  break;
       // Arrow up
	   case 38 :  for(i=0;i<(int)nRepCnt;i++) {
		              if(m_InSelectionMode==TRUE) {
                          CEdit::OnKeyDown(nChar, nRepCnt, nFlags);
					  } else {
		                  GetPrevCommand();
						  m_CommandPos=m_Command.GetLength();
				          m_Lock=TRUE;
					  }
				  }
		          break;
       // Arrow dn
	   case 40 :  for(i=0;i<(int)nRepCnt;i++) {
		              if(m_InSelectionMode==TRUE) {
                          CEdit::OnKeyDown(nChar, nRepCnt, nFlags);
					  } else {
					      GetNextCommand();
						  m_CommandPos=m_Command.GetLength();
				          m_Lock=TRUE;
					  }
				  }
				  break;
       // Arrow left
	   case 37 :  for(i=0;i<(int)nRepCnt;i++) {
		              if(m_InSelectionMode==TRUE) {
					  } else {
		                  if(m_CommandPos>0) {
		                      m_CommandPos--;
                              CEdit::OnKeyDown(nChar, nRepCnt, nFlags);		          
						  } else {
					          m_Lock=TRUE;
					          MessageBeep(MB_ICONEXCLAMATION);
							  break;
						  }
					  }
				  }
				  break;
       // Arrow right
	   case 39 :  for(i=0;i<(int)nRepCnt;i++) {
		              if(m_InSelectionMode==TRUE) {
					  } else {
					      if(m_CommandPos<m_Command.GetLength()) {
		                      m_CommandPos++;
                              CEdit::OnKeyDown(nChar, nRepCnt, nFlags);		          
						  } else {
					          m_Lock=TRUE;
					          MessageBeep(MB_ICONEXCLAMATION);
							  break;
						  } 
					  }
				  }
				  break;
	   // Enter
	   case 13 :  m_ExecLineStr=m_Command;
				  s = m_Command;
				  if(s.GetLength()>0) {
				     if(m_InHistory<50) {
					     m_History[m_InHistory++]=s;
					     m_HistoryCount=m_InHistory-1;
					 } else {
					     for(i=1;i<m_InHistory;i++) {
						     m_History[i-1]=m_History[i];
						 }
					     m_History[m_InHistory-1]=s;
					     m_HistoryCount=m_InHistory-1;
					 }
				  }
				  s+="\n\n";
				  m_Visible->AppendBlock(s);
				  m_Command="";
				  m_CommandPos=0;
				  GetParent()->SendMessage(WM_COMMAND,IDC_RETURNPRESSED,0);
                  GetParent()->SendMessage(WM_COMMAND,IDC_STATUSCHANGED,0);
				  m_Lock=TRUE;
				  break;

	   default:   for(i=0;i<(int)nRepCnt;i++) {
		              GetKeyboardState((unsigned char *)&state);
		              if(ToAscii(nChar,nFlags&0xFF,state,&ascii,nFlags )!=0) {
					      if(m_InSelectionMode==TRUE) {
                              SetSel(m_Visible->GetVisibleBuffer().GetLength()+m_Command.GetLength(),
	                                 m_Visible->GetVisibleBuffer().GetLength()+m_Command.GetLength());
						      m_CommandPos=m_Command.GetLength();
						      m_InSelectionMode=FALSE;
						  } else {
							  int start;
							  int stop;
							  GetSel(start,stop);
							  if(start!=stop) 
                                  SetSel(start,start);
						  }
					      CString left  = m_Command.Left(m_CommandPos);
					      CString right = m_Command.Right(m_Command.GetLength()-m_CommandPos);
					      m_Command=left;
					      m_Command+=ascii;
					      m_Command+=right;
					      m_CommandPos++;
                          CEdit::OnKeyDown(nChar, nRepCnt, nFlags);
					  }
				  }
	}
}



CString& CAccessEdit::GetCommand() {
static CString res;
	res=m_ExecLineStr;
	m_ExecLineStr="";
	return res;
}

void CAccessEdit::InsertResults(CString & results) {
   m_Visible->AppendBlock(results);
   SetWindowText(m_Visible->GetVisibleBuffer());
   SetSel(m_Visible->GetVisibleBuffer().GetLength(),m_Visible->GetVisibleBuffer().GetLength());
}

void CAccessEdit::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags)  {
	if(nChar==16) m_ShiftPressed=FALSE;
	if(nChar==17) m_CtrlPressed=FALSE;
    if(m_Lock==TRUE) return;
	CEdit::OnKeyUp(nChar, nRepCnt, nFlags);
}

void CAccessEdit::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)  {
    if(m_Lock==TRUE) return;
	CEdit::OnChar(nChar, nRepCnt, nFlags);
}

void CAccessEdit::GetPrevCommand(void) {
	if(m_HistoryCount>=0) {
	    CString cmd;
		cmd=m_History[m_HistoryCount--];
		ReplaceCommand(cmd);
		m_Command=cmd;
	    GetParent()->SendMessage(WM_COMMAND,IDC_STATUSCHANGED,0);
	}
}

void CAccessEdit::GetNextCommand(void) {
	if(m_HistoryCount<m_InHistory-1) {
	    CString cmd;
		cmd=m_History[++m_HistoryCount];
		ReplaceCommand(cmd);
		m_Command=cmd;
	    GetParent()->SendMessage(WM_COMMAND,IDC_STATUSCHANGED,0);
	}
}


void CAccessEdit::ReplaceCommand(CString& newCmd) {
   SetSel(m_Visible->GetVisibleBuffer().GetLength(),
	      m_Visible->GetVisibleBuffer().GetLength()+m_Command.GetLength());
   ReplaceSel(newCmd);
}




//---------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------
void CAccessEdit::OnLButtonUp(UINT nFlags, CPoint point)  {
    if(m_MouseLock==FALSE) CEdit::OnLButtonUp(nFlags, point);
	else m_MouseLock=FALSE;
    GetParent()->SendMessage(WM_COMMAND,IDC_STATUSCHANGED,0);
}
//---------------------------------------------------------------------------------------



//---------------------------------------------------------------------------------------
// When user press the left mouse button we need to detect if we are going to the upper
// line or we are still in the same line. If we change the line the future typing must
// return us to the last line, if not - we need just to change our cursor position.
//---------------------------------------------------------------------------------------
void CAccessEdit::OnLButtonDown(UINT nFlags, CPoint point)  {
union {
	struct {
		short column;
		short line;
	} xy;
	int val;
} coord;

    m_MouseLock=FALSE;
    coord.val=CharFromPos(point);
	coord.xy.column-=LineIndex(coord.xy.line);
	if(coord.xy.line<GetLineCount()-1) { // line was changed - we need to remember this
	   m_InSelectionMode=TRUE;
	} else { // we are in the same line - we need to change 
		if(coord.xy.column<2) {
			m_MouseLock=TRUE;
			return;
		}
	    m_CommandPos=coord.xy.column-2;
	}
    CEdit::OnLButtonDown(nFlags, point);
}
//---------------------------------------------------------------------------------------


//---------------------------------------------------------------------------------------
// This stubs locks the default editor pop-up menu.
//---------------------------------------------------------------------------------------
void CAccessEdit::OnRButtonUp(UINT nFlags, CPoint point)  {
}

void CAccessEdit::OnRButtonDown(UINT nFlags, CPoint point)  {
}
//---------------------------------------------------------------------------------------


//---------------------------------------------------------------------------------------
// This function will copy current selection to the clipboard.
//---------------------------------------------------------------------------------------
void CAccessEdit::ToClipboard() {
    Copy();
	SetFocus();
    GetParent()->SendMessage(WM_COMMAND,IDC_STATUSCHANGED,0);
}
//---------------------------------------------------------------------------------------



//---------------------------------------------------------------------------------------
// Pasting from clipboard will result in the editor context update + current command 
// update.
//---------------------------------------------------------------------------------------
void CAccessEdit::FromClipboard() {
COleDataObject dataObj;
HGLOBAL dataID;
char* str;
    if(m_InSelectionMode==TRUE) {
        SetSel(m_Visible->GetVisibleBuffer().GetLength()+m_Command.GetLength(),
	           m_Visible->GetVisibleBuffer().GetLength()+m_Command.GetLength());
			   m_CommandPos=m_Command.GetLength();
			   m_InSelectionMode=FALSE;
	}
	dataObj.AttachClipboard();
	if(dataObj.IsDataAvailable(CF_TEXT)==TRUE) {
 	   dataID=dataObj.GetGlobalData(CF_TEXT);
       str=(char *)GlobalLock(dataID);
	   CString left  = m_Command.Left(m_CommandPos);
	   CString right = m_Command.Right(m_Command.GetLength()-m_CommandPos);
	   m_Command=left;
	   m_Command+=str;
	   m_Command+=right;
	   m_CommandPos+=strlen(str);
	   GlobalUnlock(dataID);
	   ::GlobalFree(dataID);
 	   Paste();
       GetParent()->SendMessage(WM_COMMAND,IDC_STATUSCHANGED,0);
	   SetFocus();
	}
}
//---------------------------------------------------------------------------------------


//---------------------------------------------------------------------------------------
// This function will change current command into command from the history, if any.
//---------------------------------------------------------------------------------------
void CAccessEdit::GoPrevCommand() {
    if(m_InSelectionMode==TRUE) return;
	GetPrevCommand();
    m_CommandPos=m_Command.GetLength();
	SetFocus();
}
//---------------------------------------------------------------------------------------



//---------------------------------------------------------------------------------------
// This function will pick the command from the history list, if any, and will replace
// current command with the new one,
//---------------------------------------------------------------------------------------
void CAccessEdit::GoNextCommand() {
    if(m_InSelectionMode==TRUE) return;
	GetNextCommand();
    m_CommandPos=m_Command.GetLength();
	SetFocus();

}
//---------------------------------------------------------------------------------------


//---------------------------------------------------------------------------------------
// This function will return the editor status in the form of bit mask.
// The following masks are available:
//     ACCESS_CAN_PASTE   1
//     ACCESS_CAN_COPY    2
//     ACCESS_HAS_PREV    4
//     ACCESS_HAS_NEXT    8
//     ACCESS_CAN_STOP    16
//---------------------------------------------------------------------------------------
unsigned int CAccessEdit::GetEditorStatus() {
unsigned int mask=0;
COleDataObject dataObj;

	dataObj.AttachClipboard();
	if(dataObj.IsDataAvailable(CF_TEXT)==TRUE) mask |= ACCESS_CAN_PASTE;
	int start;
	int end;
	GetSel(start,end);
	if(start<end)  mask |= ACCESS_CAN_COPY;
	if(m_HistoryCount>=0 && m_InHistory>0) mask |= ACCESS_HAS_PREV;
	if(m_HistoryCount<m_InHistory-1 && m_InHistory>0) mask |= ACCESS_HAS_NEXT;
	return mask;

}
//----------------------------------------------------------------------------------------

void CAccessEdit::UseFont(LOGFONT * fontdescr) {
   m_UserFont=new CFont;
   m_UserFont->CreateFontIndirect(fontdescr);
   SetFont(m_UserFont);
}
