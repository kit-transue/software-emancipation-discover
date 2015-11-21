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
#ifndef _CONFIGURATOR_H_
#define _CONFIGURATOR_H_

#include <stdio.h>
#include <stdlib.h>
#include <io.h>
#include <string.h>
#include <sys/stat.h>
#include <windows.h>
#include "scc.h"
#include "DisRegistry.h"
#include "ErrMessages.h"
#include <objbase.h>
#include "ComIface.h"
#include "ComUnknown.h" // Base class for IUnknown
#include <assert.h>

class ConfigShell;
class Configurator;
class ConfigShellResult;
class ConfEnv;
class CMProvider;

/////////////////////////////////////////////////////////////////////////
//		CM
/////////////////////////////////////////////////////////////////////////

#define CALLER_NAME                   "Microsoft Developer Studio"
#define PVCS_NAME                     "PVCS"
#define SOURCESAFE_NAME               "SourceSafe"
#define CONTINUUS_NAME								"Continuus"
#define CLEARCASE_NAME                "ClearCase"
#define CLEARCASE_CO_VER_SIGNATURE    "from version \""

typedef SCCRTN (* PFNSCC)(...);

class CMProvider
{
	LPVOID					m_pSccContext;
	HINSTANCE				m_ghInstSrvr; 							// Loaded instance of the provider
	PFNSCC					m_gpFuncs[SCC_CMD_COUNT]; 	// Array of call-backs for dispatch
  long            m_SccCaps;                  // SCC capavilities flags holder
  unsigned long   m_lCoCommentLen;            // Checkout comment length
  unsigned long   m_lCommentLen;              // Any other comment length
  char            m_szProviderName[ SCC_NAME_LEN + 1 ];
  char            m_szUserName    [ SCC_USER_LEN + 1 ];
  char            m_szSccServerPath[_MAX_PATH +1];
  BOOL            m_fIsLoaded;

public:
	CMProvider();
 ~CMProvider();

  BOOL            Init();
  void            Uninit();
  BOOL			  LoadServer();
  char*           GetSccServerPath   () { return m_szSccServerPath; }
  void*           GetContext         () { return m_pSccContext; }
  char*           GetProviderName    () { return m_szProviderName; }
  char*           GetSccUserName     () { return m_szUserName; }
  void            SetSccUserName     (char* pszUserName);
  
  PFNSCC          GetInterface( int nCommand )
                              { return (nCommand >= 0 && nCommand < SCC_CMD_COUNT) ?
                              m_gpFuncs[ nCommand ] : NULL; }

  BOOL            IsLoaded      ()  { return m_fIsLoaded; }
  int             GetFileStatus (char *szFilePath );

};

long    PrintCallbackFunc (char *pszMsg, DWORD nType );
char*   GetLastSlash      (char *);
void    SetTrapVersion    (BOOL f, Configurator* pConfigurator=NULL);
char*   GetVersionFromCM  ();

/////////////////////////////////////////////////////////////////////////
//		Environment
/////////////////////////////////////////////////////////////////////////

enum ENVVARS
{
  CM_C_FILE,        /* cm_c_file is used by put_pset only */       
  CM_DESCRIPTION_FILE,  
  CM_REVISION_FILE,
  CM_SYSFILE,      
  CM_WORKFILE,     
  CM_FILES,        
  CM_PROJECT,        
  CM_LOCALDIR,        
  CM_FNAME,        
  CM_USER,        
  CM_PROVIDER,        

  CM_RMPSET,
  CM_ISCLEARCASE,
  CM_RESULT,
  CM_DISPDIFF,
  CM_PSETCFG,
  CM_BUGNO,         
  CM_COMMENT,      
  CM_DATE,         
  CM_DESCRIPTION,  
  CM_FILE_TYPE,    
  CM_LOCAL_PROJ,   
  CM_OPTIONS,      
  CM_PSET_SEARCH,  
  CM_VERSION,      
  CM_SYS_PROJ,     

  CM_OPERATION,     

  ENV_VARS_NUM
};

//--------------------------------------------------------------------
class ConfEnv
{
  char *m_pVars[ ENV_VARS_NUM ];

public:
  
  ConfEnv();
 ~ConfEnv();
  
  char *  GetEnv      ( ENVVARS index ){ return m_pVars[ index ]; }
  BOOL    SetEnv      ( ENVVARS index, char *pszNewValue );
  void *  GetEnvBlock ( int *nBlockLen );
};


//--------------------------------------------------------------------
class TargetFileDef
{
  char *m_pszSysFile;
  char *m_pszWorkFile;
  long  m_nStatus;
  long  m_nFlag;
  char  m_szSysFilePath[MAX_PATH+1];

public:

  TargetFileDef(char* pszSysFile, char* pszWorkFile=NULL );
 ~TargetFileDef();

  char*  GetSysFile()  { return m_pszSysFile; }
  char*  GetWorkFile() { return m_pszWorkFile; }
  char*  GetSysFilePath() { return m_szSysFilePath; }
  long   GetStatus()   { return m_nStatus; }
  long   GetFlag()   { return m_nFlag; }

  long   SetStatus( long l ) { return (m_nStatus=l); }
  long   SetFlag( long l ) { return (m_nFlag=l); }
};

/////////////////////////////////////////////////////////////////////////
//		ConfigShell
/////////////////////////////////////////////////////////////////////////

class ConfigShell : public CUnknown, public IConfigShell
{
public:	
	// Creation
	static HRESULT CreateInstance(IUnknown* pUnknownOuter,
	                              CUnknown** ppNewComponent) ;

private:
	// Declare the delegating IUnknown.
	DECLARE_IUNKNOWN

	// IUnknown
	virtual HRESULT __stdcall 
		NondelegatingQueryInterface( const IID& iid, void** ppv) ;			
	
	// Interface IConfigShell
	virtual HRESULT __stdcall Run (wchar_t* wcsCmd) ; 
	virtual HRESULT __stdcall GetResult (wchar_t** wcsResult) ; 
	virtual HRESULT __stdcall InitCM (wchar_t** wcsCMName) ; 
	virtual HRESULT __stdcall SetPSETCFG (wchar_t* wcsCmd) ; 

	// Notify derived classes that we are releasing.
	virtual void FinalRelease() ;

	Configurator*				m_pConfigurator;
	ConfigShellResult*	m_pConfigShellResult;

public:

  ConfigShell(IUnknown* pUnknownOuter);
 ~ConfigShell();

};

class ConfigShellResult
{
private:
	char*								m_pszText;
	ConfigShellResult*	m_pNext;
	ConfigShellResult*	m_pPrev;

public:
	ConfigShellResult(ConfigShellResult* pPrev = NULL);
 ~ConfigShellResult();

	ConfigShellResult*	GetNext()	{ return m_pNext;}
	ConfigShellResult*	GetPrev()	{ return m_pPrev;}
	char*								GetText()	{ return m_pszText;}
	void								SetNext(ConfigShellResult* pNext) { m_pNext = pNext; }
	void								SetPrev(ConfigShellResult* pPrev) { m_pPrev = pPrev; }
	void								SetText(char* pText = NULL);
	wchar_t*						GetWCText();
};

/////////////////////////////////////////////////////////////////////////
//		Configurator
/////////////////////////////////////////////////////////////////////////

#define COMMAND_LEN                   1024
#define SETUP_SCRIPT_OUTPUT_LEN       1024
#define SCRIPT_EXECUTION_TIMEOUT      60000000 /* 10 minutes */

char*   get_option    (char *, char *);
int     find_option   (char *, char *);
void    cfg_splitpath (char *, char *, char *);
BOOL    CreateParentDirectory(char * );
char ** ParseCommandLine( char *pszCmdLine, int *pArgc );
char*   GetIniKeyString(char* pszFile, char* pszSect, char* pszKey, char* pszResult, int nResultLen );

FILE* CM_OpenPipe( char *pszCmd );
int   CM_ReadPipe(char* pszLine, int nLineLen, FILE* pfOut);
void  CM_ClosePipe(FILE* pfOut );

//--------------------------------------------------------------------
class Configurator
{
protected:

  ConfEnv				m_ConfEnv;
  CMProvider*			m_pCMProvider;
  BOOL					m_fScriptInvoked;
  ConfigShellResult**	m_ppConfigShellResult;
  TargetFileDef*		m_ptfdFile;

  void        send_back_files( enum ENVVARS nSysFile, enum ENVVARS nRevisFile );

public:

  Configurator(CMProvider* pCMProvider);
 ~Configurator();

	static Configurator*			Create();

	ConfigShellResult*	ConfigShellOutput(char* pszText);
	ConfigShellResult*	ConfigShellLastOutput();
	ConfigShellResult*	ConfigShellFindOutput( char* pszText );
	void				ConfigShellDeleteOutput(ConfigShellResult*);
	int					Run( char* pszCmd, ConfigShellResult** ppCResult );
	BOOL				SetPSETCFG( char* pszPSETCFG );

  char *    GetEnv      ( ENVVARS index )
                        { return m_ConfEnv.GetEnv(index); }

  BOOL      SetEnv      ( ENVVARS index, char *pszNewValue ) 
                        { return m_ConfEnv.SetEnv(index,pszNewValue); }

  PFNSCC    GetSccInterface( int nCommand )
                        { return m_pCMProvider->GetInterface(nCommand); }
  void*     GetSccContext(){ return m_pCMProvider->GetContext(); }
  int       GetSccFileStatus (char *pszFilePath )
                        { return m_pCMProvider->GetFileStatus(pszFilePath); }
  virtual char*     GetSccUserName() { return m_pCMProvider->GetSccUserName(); }
  virtual char*     GetSccName()		 { return m_pCMProvider->GetProviderName(); }

  int Exec  ( int argc, char **argv );

  int RunScript( char *pszScriptName, char *pszScriptParameters, int &nRetCode, 
              BOOL fIsSetupScript = FALSE, BOOL fIsVersionScript = FALSE, char* pszPrefix = NULL);
  
  int Run_query_make_output( TargetFileDef* ptdfFile1, TargetFileDef* ptdfFile2 );
 
  int Run_query_get      ( int argc, char **argv );  
  int Run_query_put      ( int argc, char **argv );  
  int Run_setup_src      ( int argc, char **argv );  
  int Run_copy_pset      ( int argc, char **argv );  
  int Run_delete_src     ( int argc, char **argv );
  int Run_copy_src       ( int argc, char **argv );
  int Run_get_src        ( int argc, char **argv );
  int Run_unget_src      ( int argc, char **argv );
  int Run_put_src        ( int argc, char **argv );
  int Run_fast_put_src   ( int argc, char **argv );
  int Run_fast_merge_src ( int argc, char **argv );
  int Run_obsolete_src   ( int argc, char **argv );
  int Run_src_version    ( int argc, char **argv );

  int Run_delete_pset    ( int argc, char **argv );
  int Run_obsolete_pset  ( int argc, char **argv );
  int Run_put_pset       ( int argc, char **argv );
  int Run_unget_pset     ( int argc, char **argv );

  int Run_create         ( int argc, char **argv );
  int Run_create_dir     ( int argc, char **argv );
  int Run_create_file    ( int argc, char **argv );

  int Run_get_ver        ( int argc, char **argv );
  int Run_stat           ( int argc, char **argv );
  int Run_diff           ( int argc, char **argv );
  int Run_lock           ( int argc, char **argv );
  int Run_unlock         ( int argc, char **argv );
  int Run_is_locked      ( int argc, char **argv );
  
  int Run_put_end        ( int argc, char **argv );
  int Run_put_start      ( int argc, char **argv );
  int Run_cm_info        ( int argc, char **argv );
  int Run_lsco           ( int argc, char **argv );
  int Run_exec           ( int argc, char **argv );
  
  void  SetSysLocalProjEnv_for_put(int argc, char **argv);
  void  SetSysLocalProjEnv        (int argc, char **argv);

  virtual void  SetOptEnv         (int argc, char **argv);

  virtual int cm_setup_src ( char *pszWorkPath, char *pszSysPath, char *pszCommand, char *pszOpt );
  virtual int cm_setup_pset( char *pszWorkPath, char *pszSysPath, char *pszCommand, char *pszOpt );

  virtual int cm_query_put      ();
  virtual int cm_query_get      ();

  virtual int cm_copy_pset      ();  
  virtual int cm_delete_src     ();  
  virtual int cm_copy_src       ();  
  virtual int cm_get_src        ();  
  virtual int cm_unget_src      ();  
  virtual int cm_put_src        ();  
  virtual int cm_fast_put_src   ();  
  virtual int cm_fast_merge_src ();  
  virtual int cm_obsolete_src   ();  
  virtual int cm_src_version    ();  

  virtual int cm_delete_pset    ();  
  virtual int cm_obsolete_pset  ();  
  virtual int cm_put_pset       ();  
  virtual int cm_unget_pset     ();  

  virtual int cm_create         ();  
  virtual int cm_create_dir     ();  
  virtual int cm_create_file    ();  

  virtual int cm_stat           ();  
  virtual int cm_diff           ();  
  virtual int cm_lock           ();  
  virtual int cm_unlock         ();  
  virtual int cm_is_locked      ();  
  
  virtual int cm_put_end        ();  
  virtual int cm_put_start      ();  

  virtual int cm_version        ();  
  virtual int cm_version_list   ();  
  virtual int cm_version_date   ();  
  virtual int cm_info           ();  
  virtual int cm_lsco           ();  
  virtual int cm_exec           ();  


};


class Configurator_MSAPI : public Configurator
{
protected:
  SCCRTN  m_sOpenedFileRet;
  char    m_szOpenedFileName	[ _MAX_PATH ];
  char    m_szProjectName		[ _MAX_PATH ];
  char    m_szLocalDir			[ _MAX_PATH ];
  char    m_szAuxProjPath		[ _MAX_PATH ];

public:

  Configurator_MSAPI(CMProvider* pCMProvider);
  virtual BOOL FindProjAndLocalDir(char *cm_workfile, char *cm_revfile);

  void ClearOpenedFileName()
  {
	  m_sOpenedFileRet = SCC_E_UNKNOWNPROJECT;
	  m_szOpenedFileName[0]= ':';
	  m_szOpenedFileName[1]= '\0';
  }
  BOOL		IsOpenedFileName	(char* psz) { return (stricmp(m_szOpenedFileName, psz) == 0); }
  SCCRTN	SccRtnOpendFile		() { return m_sOpenedFileRet = m_sOpenedFileRet; }

  virtual SCCRTN  OpenProj        ( BOOL bPrintError = TRUE );
  virtual SCCRTN  CloseProj       ();

  virtual int cm_setup_src ( char *pszWorkPath, char *pszSysPath, char *pszCommand, char *pszOpt );
  virtual int cm_setup_pset( char *pszWorkPath, char *pszSysPath, char *pszCommand, char *pszOpt );

  virtual int cm_query          (SCCCOMMAND sccCommand);
  virtual int cm_query_put      ();
  virtual int cm_query_get      ();

  virtual int cm_copy_pset      ();  
  virtual int cm_delete_src     ();  
  virtual int cm_copy_src       ();  
  virtual int cm_get_src        ();  
  virtual int cm_unget_src      ();  
  virtual int cm_put_src        ();  
  virtual int cm_obsolete_src   ();  

  virtual int cm_fast_put_src   ();  
  virtual int cm_is_locked      ();  

  virtual int cm_delete_pset    ();  
  virtual int cm_put_pset       ();  
  virtual int cm_unget_pset     ();  

  virtual int cm_lock           ();  
  virtual int cm_unlock         ();  

  virtual int cm_create_dir     ();  
  virtual int cm_create_file    ();  

  virtual int cm_diff           ();  
  virtual int cm_stat           ();  
  virtual int cm_src_version	();  
  
  virtual int cm_version_list   ();  
  virtual int cm_info           ();  
  virtual int cm_lsco           ();  
  virtual int cm_exec           ();  
};


//----------------------
class Configurator_ClearCase : public Configurator_MSAPI
{
  void SetEnvRevisionFile();

public:

  Configurator_ClearCase(CMProvider* pCMProvider);
  
  BOOL	  FindProjAndLocalDir(char *cm_workfile, char *cm_revfile);
  
  void    SetOptEnv       (int argc, char **argv);
  int     cm_setup_src    ( char *pszWorkPath, char *pszSysPath, char *pszCommand, char *pszOpt );
  int     cm_setup_pset   ( char *pszWorkPath, char *pszSysPath, char *pszCommand, char *pszOpt );

  int     cm_copy_pset		();  
  int     cm_copy_src		  ();  
  int	  cm_stat           ();  
  int	  cm_lsco           ();  
};

//----------------------
class Configurator_PVCS : public Configurator_MSAPI
{
  char    m_szPVCSMainPUB   [_MAX_PATH];
  char    m_szArcDir        [_MAX_PATH];
  char    m_szWrkFile       [_MAX_PATH];
  char    m_szArcFile       [_MAX_PATH];

  char*   GetIniLine          (FILE* pFile, char* pszBuffer, int nBufferLen );
  char*   GetIniNextSection   (FILE* pFile, char* pszSect, char* pszResult, int nResultLen );
  char*   GetIniNextKeyString (FILE* pFile, char* pszKey, char* pszBuff, int nBuffLen );
  BOOL    IsAbsPath           (char*);
  void    ParseDocumentLine   ( char* szDocument, char* szLocalDir, 
                                char* szArcDir, char* szArcFile, char* szWrkDir, char* szWrkFile );
  int     CmpFilePath         ( char* szWrkDir, char* szWrkFile, char* szFileName);
  int	  CmpFileUnicName	  ( char* szLocalDir, char* szWrkDir, char* szWrkFile, char* szFileName);

public:

  Configurator_PVCS(CMProvider* pCMProvider);

  BOOL	  FindProjAndLocalDir(char *cm_workfile, char *cm_revfile);
  int	  cm_stat			 ();  
};

//----------------------
class Configurator_SourceSafe : public Configurator_MSAPI
{
  char    m_szToolSS      [_MAX_PATH];
  char    m_szUserIniFile [_MAX_PATH];
  char    m_szSrcFName    [_MAX_PATH];
  char    m_szSrcProject  [_MAX_PATH];
  char    m_szPsetFName   [_MAX_PATH];
  char    m_szPsetProject [_MAX_PATH];
  char	  m_szTmpFile	  [_MAX_PATH];
  char**  m_ppszSSiniEnv;
  int	  m_nSSiniEnv;

public:

  Configurator_SourceSafe(CMProvider* pCMProvider);
 ~Configurator_SourceSafe();
  
  BOOL	  FindProjAndLocalDir(char *cm_workfile, char *cm_revfile);

 
  int     cm_setup_src    ( char *pszWorkPath, char *pszSysPath, char *pszCommand, char *pszOpt );
  int     cm_setup_pset   ( char *pszWorkPath, char *pszSysPath, char *pszCommand, char *pszOpt );

  int	  cm_stat         ();  
  int	  cm_lsco         ();  
};

//----------------------
class Configurator_Continuus : public Configurator_MSAPI
{
public:

  Configurator_Continuus(CMProvider* pCMProvider);
  
  BOOL	  FindProjAndLocalDir(char *cm_workfile, char *cm_revfile);
//  char	  GetProjLocalDir (char* pszFilePath, char* pszProjName, char* pszLocalDir);

  SCCRTN  OpenProj        ( char *cm_workfile, char *cm_revfile, BOOL bPrintError = TRUE );
  
  int     cm_setup_src    ( char *pszWorkPath, char *pszSysPath, char *pszCommand, char *pszOpt );
  int     cm_setup_pset   ( char *pszWorkPath, char *pszSysPath, char *pszCommand, char *pszOpt );

  int	  cm_stat         ();  
  int	  cm_exec         ();  
};


//----------------------
class Configurator_Custom : public Configurator_MSAPI
{
  void  cm_copy_internal();
  void  cm_unget_internal();
  void  cm_put_internal();
  void  cm_delete_internal();

public:

  Configurator_Custom(CMProvider* pCMProvider);
 ~Configurator_Custom();
  
  BOOL	  FindProjAndLocalDir(char *cm_workfile, char *cm_revfile);

  int     cm_setup_src    ( char *pszWorkPath, char *pszSysPath, char *pszCommand, char *pszOpt );
  int     cm_setup_pset   ( char *pszWorkPath, char *pszSysPath, char *pszCommand, char *pszOpt );

  int     cm_info         ();
  int     cm_copy_pset    ();  
  int     cm_copy_src	  ();  
  int     cm_get_src	  ();  
  int     cm_put_pset	  ();  
  int     cm_put_src	  ();  
  int     cm_fast_put_src ();
  int	  cm_lsco         ();
  int     cm_unget_pset   ();  
  int     cm_unget_src    ();  
  int     cm_delete_pset  ();  
  int     cm_delete_src   ();
  
  int     cm_stat         ();  
  int     cm_lock         ();
  int     cm_unlock       ();

  int cm_obsolete_src   () { return 0; }
  int cm_is_locked      () { return 0; }
  int cm_create_dir     () { return 0; }
  int cm_create_file    () { return 0; }
  int cm_diff           () { return 0; }
  int cm_src_version	() { return 0; }
  int cm_version_list   () { return 0; }

  char*     GetSccUserName() { return (char*)"User"; }
  char*     GetSccName()		 { return (char*)"Custom"; }
  
};



#endif /*_CONFIGURATOR_H_*/
