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
//////////////////////////////////////////////////////////////////////////////////////////////////
//	This class implements a first cut at a progress bar manager
//	In this case, the progress bar is designed to manage the concept of 
//  "Given a set of strings known at initialization time, 
//	display them in a specified order, at user-specified intervals, and update a progress bar
//  while providing a cancel button"
//	
//	This isn't the most flexible way, nor is it the cleanest, but it works quite well for
//  batch processes.
//////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef progressBar_h
#define progressBar_h
#ifdef _WIN32
//have to include this to get the proc  ess_information
#include <windows.h>
class progressBar {
public:
	progressBar(int nCommands, char * pszCommands);
	~progressBar();
	int ShowNextCommand();
	void InitializeDialog();
	void StopProgressBar();
	//returns 1 if the canceldialog is dead.
	int QueryProgressBar();
private:
	int ParseTclString(char * str);
	char** m_pszCmds;
	int m_nCmds;
	int m_nCurCmd;
	PROCESS_INFORMATION pi;
	char *shared_memory;
	int msgAmountID;
	int msgProcessedID ;
};
#else
//stub class for unix to eliminate compiler errors
class progressBar {
public:
	progressBar(int nCommands, char * pszCommands){}
	~progressBar(){}
	int ShowNextCommand(){return 0;}
	void InitializeDialog(){}
	void StopProgressBar(){}
	int QueryProgressBar(){return 0;}
};
#endif 

#endif
