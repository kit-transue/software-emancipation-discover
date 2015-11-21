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
#include "cLibraryFunctions.h"
#include "machdep.h"
#include <vport.h>
#include vtimerHEADER
#include veventHEADER


#include "gString.h"
#include "gcontrolObjects.h"
#include "gapl_menu.h"
#include "progressBar.h"
#include "monitorBatch.h"



int spawn_redirected( const char *pszCommand, const char *pszOutFile );
void process_terminate( int nPid );
int process_is_active( int nPid );

monitorBatch* monitorBatch::m_CurrInst = NULL;
//initialization routine

void monitorBatch::commonInit(int nCmds, char **Cmds, char**Cmts, Tcl_Interp *interp,
							const char *cmd_when_done, const char *results_text_item, 
                            const char *log_file )
{
  m_CurrCommand = -1;
  m_CurrPid = -1;
  m_ProgressBar=NULL;
  m_Interp = interp;
  m_CmdWhenDone = (const vchar *)cmd_when_done;
  for( int i = 0,n = 0; n<nCmds && i<MAX_CMDS_NUMBER; i ++, n++ )
  {
    //special 'hack' for -proj pass
    int fParsed = 0;
    if( Cmds[n] )
    {
      char *pProj = strstr( Cmds[n], "-proj " );
      if( pProj )
      {

		  //////////////////////////////////////////////////////////////////////////////////////////////////
		  //  The following code attempts to parse a line of the following form:
		  //  pset_server -blah -blah -blah -proj /proj1 /proj2 -blah blah blah
		  //  by breaking it up into a prefix, a project list, and a suffix.  Be careful not to add empty projects...
		  //////////////////////////////////////////////////////////////////////////////////////////////////

        fParsed = 1;
        pProj[5] = '\0';
        char *pNextOpt = strstr( pProj + 6, " -" );
        if( pNextOpt )
          *pNextOpt = '\0';
        static const char *seps = " ,;\t";
        char *proj = strtok( pProj + 6,  seps );
        int fFirstFlag = 1;
        while( proj && i<MAX_CMDS_NUMBER )
        {
          m_Commands[i] = (vchar *)Cmds[n];
          m_Commands[i] += " ";
          m_Commands[i] += proj;
          if( pNextOpt )
          {
            m_Commands[i] += " ";
            m_Commands[i] += (pNextOpt + 1);
          }
          if( fFirstFlag )
            m_Comments[i] = (vchar *)Cmts[n];
          else
            m_Comments[i] = (const vchar *)"";
          proj = strtok( NULL, seps );
          //this is necessary to prevent a null project.
		  if (proj)
			i++;
          fFirstFlag  = 0;
        }
        if( fFirstFlag )//if project list contained 0 items
          fParsed = 0;
      
	  }
    }
    if( !fParsed )
    {
      m_Comments[i] = (vchar *)Cmts[n];
      m_Commands[i] = (vchar *)Cmds[n];
    }
  }
  m_NumCommands = i;
  if( m_Interp && results_text_item&&*results_text_item )
    m_TextItem = gdTextItem::CastDown((vdialogItem *)dis_findItem( m_Interp, (vchar*)results_text_item));
  else
    m_TextItem = NULL;
 
  m_fIsTmpFile = 1;
  //BEWARE-- StartNextCommand  expects a valid filename.  If you pass an illegal/invalid filename, an 
  // invalid monitorbatch object will be created and it will do nothing.
  // sooner or later I'll fix it properly...(GRB)
  //
  if( log_file && OSapi_strlen (log_file) )
  {
    m_fIsTmpFile = 0;
    m_LogFile = (const vchar *)log_file;
    //truncate old log file
    OSapi_unlink( log_file );
  }
  if( m_fIsTmpFile )
  {
    char *tmp_nam = OSapi_tempnam( ".", "dis" );
    if( tmp_nam )
    {
      m_LogFile = (const vchar *)OSPATH(tmp_nam);
      OSapi_free( tmp_nam );  
    }
    else
      m_LogFile = (const vchar *)"dis_tmp.tmp";
  }

  m_Timer = new vtimer();
  if( m_Timer )
  {
    m_Timer->SetPeriod( 3, 0 ); //3 secs
    m_Timer->SetRecurrent();
    m_Timer->SetObserveTimerProc( BatchTimerObserveProc );
  
  }
}



monitorBatch::monitorBatch( int nCmds, char **Cmds, char **Cmts, Tcl_Interp *interp, 
                            const char *cmd_when_done, const char *results_text_item, 
                            const char *log_file )
{
  commonInit( nCmds, Cmds, Cmts, interp, cmd_when_done, results_text_item, log_file);
   m_ProgressBar=NULL;
  StartNextCommand();

  if( m_TextItem )
    m_TextItem->ShowFile( (vchar *)m_LogFile.str(), 1 );

  if (m_Timer)
    m_Timer->Start();
}
monitorBatch::monitorBatch( int nCmds, char **Cmds, char **Cmts, Tcl_Interp *interp, 
                            const char *cmd_when_done, const char *results_text_item, 
                            const char *log_file, char * pszStatusMessages )
							
{
	commonInit(nCmds,Cmds,  Cmts, interp, cmd_when_done, results_text_item, log_file);
//this should be the only ifdef in here.

#ifdef _WIN32
	
	//////////////////////////////////////////////////////////////////////////////////////////////////
	//	Because of the -proj hack in commonInit, the the project pass is split into multiple invocations
	//	of pset_server.  Consquently, we initialize the progress bar with the known number of invocations, 
	//	not the number of invocations suggested by the caller
	//  commonInit must have run before this function.
	//////////////////////////////////////////////////////////////////////////////////////////////////

	m_ProgressBar = new progressBar(m_NumCommands, pszStatusMessages);
#else
	m_ProgressBar=NULL;
#endif
	 StartNextCommand();
	if( m_TextItem )
		m_TextItem->ShowFile( (vchar *)m_LogFile.str(), 1 );

  if (m_Timer)
    m_Timer->Start();

}

static int handleDestroy(vevent *theEvent)
{
  vtimer *pTimer = (vtimer *)theEvent->GetClientData();
  if( pTimer )
    delete pTimer;
  return 1;
}

monitorBatch::~monitorBatch()
{
  if( m_Timer )
  {
    if( m_Timer->IsActive() )
      m_Timer->Stop();
    //delayed vtimer deletion
    vevent *event = veventCreateClient();
    veventSetPriority(event, veventPRIORITY_MIN);
    veventSetTarget(event, handleDestroy);
    veventSetClientData(event, m_Timer);
    _veventPostLater(event);
    //delete m_Timer;
  }
  if (m_ProgressBar)
  {
	  delete m_ProgressBar;
	  m_ProgressBar=NULL;
  }
  if( m_fIsTmpFile && m_LogFile.not_null() )
    OSapi_unlink( (const char *)m_LogFile.str() );
}

//Static method
int monitorBatch::Create( int nCmds, char **Cmds, char **Cmts, Tcl_Interp *interp, 
                          const char *cmd_when_done, const char *results_text_item, 
                          const char *log_file )
{
  int nRet = 0;
  if( m_CurrInst )
    delete m_CurrInst;
  m_CurrInst = new monitorBatch( nCmds, Cmds, Cmts, interp, cmd_when_done, 
                                 results_text_item, log_file );
  if( m_CurrInst )
    nRet = 1;
  return nRet;
}

int monitorBatch::Create( int nCmds, char **Cmds, char **Cmts, Tcl_Interp *interp, 
                          const char *cmd_when_done, const char *results_text_item, 
                          const char *log_file, char * pszStatusMessages )
{ int nRet = 0;
  if( m_CurrInst )
    delete m_CurrInst;
  m_CurrInst = new monitorBatch( nCmds, Cmds, Cmts, interp, cmd_when_done, 
                                 results_text_item, log_file, pszStatusMessages );
  if( m_CurrInst )
    nRet = 1;
  return nRet;

}
//Static method
int monitorBatch::IsActive()
{
  return ( m_CurrInst!=NULL );
}

int monitorBatch::Done()
{
  int nRet = 0;
  if( m_TextItem )
    m_TextItem->StopUpdating();
  if (m_ProgressBar)
	  m_ProgressBar->StopProgressBar();

  if ( m_Interp && m_CmdWhenDone.not_null() )
    nRet = (Tcl_Eval( m_Interp, (char *)m_CmdWhenDone.str() ) == TCL_OK );
  vwindow::Beep();
  return nRet;
}

int monitorBatch::Cancel()
{
  int nRet = 0;
  if( m_CurrInst )
  {
    int nPid = m_CurrInst->m_CurrPid;
    if( nPid != -1 )
      process_terminate( nPid );
    nRet = m_CurrInst->Done();
    delete m_CurrInst;
    m_CurrInst = NULL;
  }
  return nRet;
}

int monitorBatch::StartNextCommand()
{
  m_CurrCommand++;
  m_CurrPid = -1;
  if( m_CurrCommand < m_NumCommands )
  {
    //write comment first
    if( m_Comments[m_CurrCommand].not_null() )
    {
#ifndef _WIN32
      char *mode = "a+";
#else
      char *mode = "a+t";
#endif
      FILE *out_f = OSapi_fopen( (const char *)m_LogFile.str(), mode );
      if( out_f )
      {
        fprintf( out_f, "\n%s\n", (const char *)m_Comments[m_CurrCommand].str() );
        fflush( out_f ); 
        OSapi_fclose( out_f );
      }
    }
    if( m_Commands[m_CurrCommand].not_null() )
      m_CurrPid = spawn_redirected( (const char *)m_Commands[m_CurrCommand].str(), 
                                    (const char *)m_LogFile.str() );
  //update the progressbar
	if (m_ProgressBar)
		m_ProgressBar->ShowNextCommand();
  
  }
  return m_CurrPid;
}

void monitorBatch::BatchTimerObserveProc( vtimer * )
{
  int nPid = m_CurrInst->m_CurrPid;
  if (m_CurrInst->m_ProgressBar&&m_CurrInst->m_ProgressBar->QueryProgressBar()>0)
  {
	  Cancel();
	  return;
  }


  if( !process_is_active( nPid ) )
  {
    if( m_CurrInst->m_CurrCommand < m_CurrInst->m_NumCommands - 1 )
      m_CurrInst->StartNextCommand();
    else
    {
      m_CurrInst->Done();
      delete m_CurrInst;
      m_CurrInst = NULL;
    }
  }
}
