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
#if defined(WIN32)||defined(_WIN32)
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include "progressBar.h"

progressBar::progressBar(int nCmds, char *pszCommands)
{
	m_nCmds=nCmds;
	m_nCurCmd=0;
	m_pszCmds=new char *[m_nCmds];
	for (int i=0; i<m_nCmds; i++)
	{
		m_pszCmds[i]=NULL;
	}
	//parsing is left to a subroutine so that you can plug in a better parser.
	ParseTclString(pszCommands);
	InitializeDialog();
}

progressBar::~progressBar()
{
	StopProgressBar();
	for (int i=0; i<m_nCmds; i++)
	{	
		delete m_pszCmds[i];
		m_pszCmds[i]=NULL;
	}
	//this is an array of primitive types, not an array of objects...
	delete m_pszCmds;
}

int progressBar::ShowNextCommand()
{
	if (m_nCurCmd<m_nCmds)
	{
		m_nCurCmd++;
	}
	char * pCurCommand= m_pszCmds[m_nCurCmd-1];
	return QueryProgressBar();
}


//////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Function: InitializeDialog
//	Author: Gregory R. Bronner
//	Date: 12/17/98
//	Purpose: To set up and display the progress bar with a message of 'initializing'
//	Notes: The syntax for the canceldialog seems to be as follows:
//  canceldialog <stuff in title bar> -title <stuff in status area at top> -<number of elements>

/////////////////////////////////////////////////////////////////////////////////////////////////////

void progressBar::InitializeDialog()
{
	msgAmountID    = RegisterWindowMessage("WM_SETAMOUNT");
	msgProcessedID = RegisterWindowMessage("WM_SETPROCESSED");
	STARTUPINFO si;
	SECURITY_ATTRIBUTES saProcess, saThread;
	
	ZeroMemory(&si,sizeof(si));
	si.cb=sizeof(si);
	saProcess.nLength = sizeof(saProcess);
	saProcess.lpSecurityDescriptor=NULL;
	saProcess.bInheritHandle=TRUE;
	saThread.nLength=sizeof(saThread);
	saThread.lpSecurityDescriptor=NULL;
	saThread.bInheritHandle = TRUE;
	char execcmd[256];
	sprintf(execcmd,"CancelDialog Discover Model Administrator Progress Indicator -Initializing... -%d", m_nCmds);
	::CreateProcess(NULL,&execcmd[0],&saProcess,&saThread,TRUE,0,NULL,NULL,&si,&pi);
	shared_memory=(char *) ::VirtualAllocEx(pi.hProcess,NULL,1024,MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE);
}
//////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Function: StopProgressBar
//	Author: Gregory R. Bronner
//	Date: 12/17/98
//	Purpose: Stop the progress bar and remove it from the screen.
//	Notes: Maybe use ExitProcess?
/////////////////////////////////////////////////////////////////////////////////////////////////////

void progressBar::StopProgressBar()
{
	   ::VirtualFreeEx(pi.hProcess,shared_memory,0,MEM_RELEASE);
	   ::TerminateProcess(pi.hProcess,0);
}

int progressBar::QueryProgressBar()
{
	int nRet=0;
	DWORD status;
	::GetExitCodeProcess(pi.hProcess,&status);
	if(status==STILL_ACTIVE) {
		DWORD written;
		char mem[1024];
		//////////////////////////////////////////////////////////////////////////////////////////////////
		//This wasn't really documented.  It appears that the first wroteprocessmemory clears out the 
		//status bar space (overwrites with 0s), the second one fills in a certain area with the text to display in the progress indicator
		// and the sendmessage tells gives it a numer to update...
		//////////////////////////////////////////////////////////////////////////////////////////////////
		ZeroMemory(mem,1024);
		char * pCurComment=m_pszCmds[m_nCurCmd-1];
		::WriteProcessMemory(pi.hProcess,shared_memory,mem,1024,&written);
		if (pCurComment)
			::WriteProcessMemory(pi.hProcess,shared_memory,pCurComment,strlen(pCurComment),&written);
		int numtodisplay=(m_nCurCmd>=0?m_nCurCmd:0);
		::SendMessage(HWND_BROADCAST,msgProcessedID,m_nCurCmd,(LPARAM)shared_memory);
	} else 
		nRet=1;
	return nRet;
}
//////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Function: ParseTclString
//	Author: Gregory R. Bronner
//	Date: 12/17/98
//	Purpose: generate the comment list from a weird tcl string
//	Notes:
//  The string is positited to be as follows {message1}{message2}{message3}{message..n}
//	This is a bit of a hack, ...oh well.
//	Returns: number of strings parsed (this should match m_nCmds, otherwise weird things will happen
//	Don't try any fancy stuff like {this}{is}{a \{stupid\}}{message} because the parser
//  isn't designed for it...
/////////////////////////////////////////////////////////////////////////////////////////////////////

int progressBar::ParseTclString(char * str)
{	
	
	int nFound=0;
	int nInItem=0;
	const char * LEADING_SEPS = "{}";
	//const char * END_SEPS = "}";
	char * token=NULL;

	//first advance the string
	while (*str!=NULL&&isspace(*str))
		str++;
	token=strtok(str, LEADING_SEPS);
	//nInItem=1;
	//this is very sensitive to spaces... can't do a { } for example :(
	while(token!=NULL&& nFound<m_nCmds)
	{
		m_pszCmds[nFound]=new char[strlen(token)+1];
		strcpy(m_pszCmds[nFound], token);
		token=strtok(NULL, LEADING_SEPS);
		if (token!=NULL&&nFound<m_nCmds-1)
			nFound++;
	}

	return nFound;
}

#endif	
