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
#ifndef _MONITORBATCH_H_
#define _MONITORBATCH_H_
class progressBar;


const int MAX_CMDS_NUMBER = 512;
class monitorBatch
{
protected:
	
	//////////////////////////////////////////////////////////////////////////////////////////////////
	//	first constructor (and create method is for a regular monitor batch
	//	second one uses a progress bar. (NT only)
	//////////////////////////////////////////////////////////////////////////////////////////////////
	
	monitorBatch( int nCmds, char **Cmds, char **Cmts, Tcl_Interp *interp, 
		const char *cmd_when_done, const char *results_text_item, const char *log_file );
	monitorBatch( int nCmds, char **Cmds, char **Cmts, Tcl_Interp *interp, 
		const char *cmd_when_done, const char *results_text_item, const char *log_file, char * pszStatusMessages );
	~monitorBatch();
	void commonInit(int nCmds, char **Cmds, char **Cmts, Tcl_Interp *interp, 
		const char *cmd_when_done, const char *results_text_item, const char *log_file);
	int Done();
	int StartNextCommand();
	
	int m_NumCommands;
	int m_CurrCommand;
	int m_CurrPid;
	// note that this will be a stub class on UNIX
	progressBar * m_ProgressBar;
	
	gString m_Commands[ MAX_CMDS_NUMBER ];
	gString m_Comments[ MAX_CMDS_NUMBER ];
	gString m_CmdWhenDone;
	gString m_LogFile;
	int m_fIsTmpFile;
	Tcl_Interp *m_Interp;
	vtimer *m_Timer;
	
	gdTextItem *m_TextItem;
	
	static monitorBatch *m_CurrInst;
	
public:
	static int Create( int nCmds, char **Cmds, char **Cmts, Tcl_Interp *interp, 
		const char *cmd_when_done, const char *results_text_item, 
		const char *log_file );
	static int Create ( int nCmds, char **Cmds, char **Cmts, Tcl_Interp *interp, 
		const char *cmd_when_done, const char *results_text_item, 
		const char *log_file, char * pszStatusMessages );
	static int Cancel();
	static int IsActive();
	static void BatchTimerObserveProc( vtimer * );
	
};

#endif


