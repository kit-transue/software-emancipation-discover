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
#include <fcntl.h>
#include "configurator.h"

#define	_THIS_FILE_		"Configurator"

/***********************************************************/
/***      TargetFileDef                                   ***/
/***********************************************************/

TargetFileDef::TargetFileDef(char* pszSysFile, char* pszWorkFile )
  {
    m_pszSysFile = NULL;
    m_pszWorkFile= NULL;
    m_nStatus    = 0;
    if ( pszSysFile && *pszSysFile )
    {
      m_pszSysFile = new char [ strlen(pszSysFile) +1 ];
      if ( m_pszSysFile ) strcpy( m_pszSysFile, pszSysFile );
      _fullpath( m_szSysFilePath, pszSysFile, MAX_PATH );
 
      if ( pszWorkFile== NULL || *pszWorkFile == 0 )
        pszWorkFile = pszSysFile;
      m_pszWorkFile = new char [ strlen(pszWorkFile) +1 ];
      if ( m_pszWorkFile ) strcpy( m_pszWorkFile, pszWorkFile ); 
    }
  }

TargetFileDef::~TargetFileDef()
  {
    if ( m_pszSysFile ) delete [] m_pszSysFile;
    if ( m_pszWorkFile ) delete [] m_pszWorkFile;
  }

/***********************************************************/
/***      Configurator                                   ***/
/***********************************************************/

Configurator::Configurator(CMProvider* pCMProvider)
{
  m_pCMProvider = pCMProvider;
	m_ppConfigShellResult	= NULL;
  m_ptfdFile = NULL;
}

//--------
Configurator::~Configurator()
{
  m_pCMProvider->Uninit();
  if ( m_pCMProvider )
    delete m_pCMProvider;
  m_pCMProvider = NULL;
  if ( m_ptfdFile )
    delete m_ptfdFile;
  m_ptfdFile = NULL;
}

//--------
ConfigShellResult* Configurator::ConfigShellOutput(char* pszText)
{
  ConfigShellResult* pConfigShellResult = NULL;
	if ( pszText && *pszText )
	{
		char *p;
		while ( (p=pszText+strlen(pszText)-1) && (*p == 0x0d || *p == 0x0a) )
			*p = 0;
		pConfigShellResult= *m_ppConfigShellResult;
		if ( pConfigShellResult )
		{
			while ( pConfigShellResult->GetNext() )
				pConfigShellResult= pConfigShellResult->GetNext();
			pConfigShellResult = new ConfigShellResult ( pConfigShellResult );
		}
		else
			pConfigShellResult = *m_ppConfigShellResult = new ConfigShellResult;

		if ( pConfigShellResult )
			pConfigShellResult->SetText(pszText);
	}
	return pConfigShellResult;
}

//--------
ConfigShellResult* Configurator::ConfigShellLastOutput()
{
	ConfigShellResult* pConfigShellResult= NULL;

	pConfigShellResult= *m_ppConfigShellResult;
	while ( pConfigShellResult && pConfigShellResult->GetNext() )
	{
		pConfigShellResult = pConfigShellResult->GetNext();
	}
	return pConfigShellResult;
}

//--------
ConfigShellResult* Configurator::ConfigShellFindOutput( char* pszText )
{
	ConfigShellResult* pConfigShellResult= NULL;
	if ( pszText && *pszText )
	{
		pConfigShellResult= *m_ppConfigShellResult;
		while ( pConfigShellResult )
		{
			if ( strstr( pConfigShellResult->GetText(), pszText ) )
				break;
			pConfigShellResult = pConfigShellResult->GetNext();
		}
	}
	return pConfigShellResult;
}

//--------
void Configurator::ConfigShellDeleteOutput(ConfigShellResult* pResult )
{
	if ( pResult )
	{
		ConfigShellResult* pConfigShellResult= *m_ppConfigShellResult;
		while ( pConfigShellResult )
		{
			if ( pResult == pConfigShellResult )
			{
				ConfigShellResult* pNext = pConfigShellResult->GetNext();
				ConfigShellResult* pPrev = pConfigShellResult->GetPrev();
				pConfigShellResult->SetNext(NULL);
				pConfigShellResult->SetPrev(NULL);
				if ( pResult == *m_ppConfigShellResult )
					*m_ppConfigShellResult = pNext;
				if ( pPrev )
					pPrev->SetNext( pNext );
				if ( pNext )
					pNext->SetPrev( pPrev );
				delete pResult;
				break;
			}
			pConfigShellResult = pConfigShellResult->GetNext();
		}
	}
}

//--------
BOOL	Configurator::SetPSETCFG( char* pszPSETCFG )
{
	return SetEnv(CM_PSETCFG, pszPSETCFG );
}

//--------
extern BOOL g_bCustomCM;

Configurator* Configurator::Create()
{
  Configurator* pConfigurator = NULL;
  CMProvider*   pCMProvider   = new CMProvider;
  char*			pszErrText	= NULL;
  if ( pCMProvider )
  {
    if ( g_bCustomCM )
      pConfigurator = new Configurator_Custom(pCMProvider);
    else if ( pCMProvider->Init() )
    {
      char* pszProviderName = pCMProvider->GetProviderName();
      if ( stricmp(pszProviderName, CLEARCASE_NAME ) == 0 )
        pConfigurator = new Configurator_ClearCase(pCMProvider);
      else if ( stricmp(pszProviderName, PVCS_NAME ) == 0 )
        pConfigurator = new Configurator_PVCS(pCMProvider);
      else if ( stricmp(pszProviderName, SOURCESAFE_NAME ) == 0 )
        pConfigurator = new Configurator_SourceSafe(pCMProvider);
      else if ( stricmp(pszProviderName, CONTINUUS_NAME ) == 0 )
        pConfigurator = new Configurator_Continuus(pCMProvider);
      else
      {
    		pszErrText = "$Unknown CM provider";
        pConfigurator = new Configurator_MSAPI(pCMProvider);
      }
    }
    else
    {
	  pszErrText = "$Couldn't initialize CM provider";
      pCMProvider->Uninit();
      delete pCMProvider;
      pCMProvider = NULL;
    }

  }

  if ( !pCMProvider )
      pConfigurator = new Configurator (NULL);
  else
  {
	pConfigurator->SetEnv(CM_USER, pConfigurator->GetSccUserName());
	pConfigurator->SetEnv(CM_PROVIDER, pConfigurator->GetSccName());
  }
  
  if ( pConfigurator )
  {
	if ( pszErrText )
		pConfigurator->ConfigShellOutput(pszErrText);
  }
  return pConfigurator ;
}

/***********************************************************/
/***      Implemantation of Configurator (Base)          ***/
/***********************************************************/

int Configurator::Run( char* pszCmd, ConfigShellResult** ppConfigShellResult )
{
	LogMessage( "ConfigShellExecute", pszCmd, "" );

  int err = 0;
	if ( *ppConfigShellResult )
		delete (*ppConfigShellResult);
	*ppConfigShellResult = NULL;
	m_ppConfigShellResult = ppConfigShellResult;

  if( pszCmd && *pszCmd )
  {
    int pos = strlen(pszCmd) - 1;
    if( pszCmd[pos ] == '\n' )
      pszCmd[pos ] = '\0';
    if( !strncmp(pszCmd, "exec", 4) )
    {
      char *pszToExec = pszCmd + 4;
      while( *pszToExec == ' ' )
        pszToExec++;
      char *pszCopy = _strdup( pszToExec );
      if( pszCopy )
      {
        int config_argc = 0;
        char **config_argv = ParseCommandLine( pszCopy, &config_argc );
        if( config_argv )
        {
          if( TRUE/*!_stricmp(config_argv[0], CONFIGURATOR_NAME)*/ )
          //execute configurator command here
            err = Exec( config_argc, config_argv );
          else // some other command was received, execute "system" for compatibility
               // with UNIX version
          {
			      LogMessage( "ConfigShellExecute", "system", pszCmd );
            err = system( pszToExec );
          }

          if( err ) 
          {
            LogDebug( "Error", "$error", " " );
						char sz[64];
						sprintf( sz, "$error %d", err );
						ConfigShellOutput(sz);
            //if (!is_batch) putenv("PSET_FORCE_NOWAIT=0");
	        }
	        else 
          {
            LogDebug( "OK", "$ok", " " );
            ConfigShellOutput("$ok");
          }

          delete config_argv;
        }
        free( pszCopy );
      }
    }
	}

  if ( m_ptfdFile )
    delete m_ptfdFile;
  m_ptfdFile = NULL;

	if ( pszCmd )
		delete [] pszCmd;

	return err;
}

//--------
void Configurator::SetSysLocalProjEnv_for_put(int argc, char **argv)
{
    SetEnv( CM_BUGNO,   argv[3] );
    SetEnv( CM_COMMENT, argv[4] );
    SetEnv( CM_FILES,   argv[5] );

    char *local_proj = get_option(argv[2], "-local_proj");
    SetEnv( CM_LOCAL_PROJ, local_proj );
    if( local_proj )
      free( local_proj );

    char *sys_proj = get_option(argv[2], "-sys_proj");
    SetEnv( CM_SYS_PROJ, sys_proj );
    if( sys_proj )
      free( sys_proj );
}

//--------
void Configurator::SetSysLocalProjEnv(int argc, char **argv)
{
    SetEnv( CM_BUGNO,   NULL );
    SetEnv( CM_COMMENT, NULL );
    SetEnv( CM_FILES,   NULL );

    if (find_option(argv[2], "-sys_proj"))
    {
      char *sys_proj = get_option(argv[2], "-sys_proj");
      SetEnv( CM_SYS_PROJ, sys_proj );
      if( sys_proj )
        free( sys_proj );
    }
    else
      SetEnv( CM_SYS_PROJ, NULL );

    if (find_option(argv[2], "-local_proj"))
    {
      char *local_proj = get_option(argv[2], "-local_proj");
      SetEnv( CM_LOCAL_PROJ, local_proj );
      if( local_proj )
        free( local_proj );
    }
    else
      SetEnv( CM_LOCAL_PROJ, NULL );
}

//--------
void Configurator::SetOptEnv(int argc, char **argv)
{
  char *ver = get_option(argv[2], "-ParaRev" );
  SetEnv( CM_VERSION, ver );
  if (ver)
    free(ver);

  
  if( find_option(argv[2], "-RM_PSET") )
  {
	  char *p   = strstr(argv[2], "-RM_PSET");
	  SetEnv( CM_RMPSET, "YES");
	  if( p )
    {
	    char *pp = p + strlen("-RM_PSET");
      memmove( p, pp, strlen(pp) );
	  }
  }

  if( argc >=8 )
    SetEnv( CM_FILE_TYPE, argv[7]) ;
  SetEnv( CM_OPTIONS, argv[2] );
  if( find_option(argv[2], "-pset_search") )
    SetEnv( CM_PSET_SEARCH, "Y" );
}

//--------
int Configurator::Exec  ( int argc, char **argv )
{
  BOOL	put_is_processing = FALSE;
  BOOL	is_batch					= FALSE;
	for( int i=1; i < argc; i++ ) 
	{
		if( !strcmp( argv[i], "-cfg_put" ) ) 
		{
			put_is_processing = TRUE;
			is_batch = TRUE;
		} 
		else if( !strcmp( argv[i], "-batch" ) )
			is_batch = TRUE;
	}

  int nReturn = 1;
	char sz[256 ];

  if (strcmp(argv[1], "cm_exec") == 0)
      nReturn = Run_exec(argc,argv);
  else if (strcmp(argv[1], "cm_info") == 0)
      nReturn = Run_cm_info(argc,argv);
  else if (strcmp(argv[1], "lsco") == 0)
      nReturn = Run_lsco(argc,argv);
  else if ( argc < 8 )
  {
    sprintf(sz, "%%%%Internal error: %d is not enough args for %s command;\n", argc, argv[1]);
    ConfigShellOutput( sz );
  }
  else
  {
    LogDebug( "Exec", argv[1], " " );

    if (strcmp(argv[1], "query_get") == 0)
      nReturn = Run_query_get(argc,argv);
    else if (strcmp(argv[1], "query_put") == 0)
      nReturn = Run_query_put(argc,argv);
    else if (strcmp(argv[1], "copy_pset") == 0)
      nReturn = Run_copy_pset(argc,argv);
    else if (strcmp(argv[1], "copy_src") == 0)
      nReturn = Run_copy_src(argc,argv);
    else if (strcmp(argv[1], "create_dir") == 0)
      nReturn = Run_create_dir(argc,argv);
    else if (strcmp(argv[1], "create_file") == 0)
      nReturn = Run_create_file(argc,argv);
    else if (strcmp(argv[1], "delete_pset") == 0)
      nReturn = Run_delete_pset(argc,argv);
    else if (strcmp(argv[1], "delete_src") == 0)
      nReturn = Run_delete_src(argc,argv);
    else if (strcmp(argv[1], "diff") == 0)
      nReturn = Run_diff(argc,argv);
    else if (strcmp(argv[1], "get_src") == 0)
      nReturn = Run_get_src(argc,argv);
    else if (strcmp(argv[1], "get_ver") == 0)
      nReturn = Run_get_ver(argc,argv);
    else if (strcmp(argv[1], "lock") == 0)
      nReturn = Run_lock(argc,argv);
    else if (strcmp(argv[1], "obsolete_pset") == 0)
      nReturn = Run_obsolete_pset(argc,argv);
    else if (strcmp(argv[1], "obsolete_src") == 0)
      nReturn = Run_obsolete_src(argc,argv);
    else if (strcmp(argv[1], "put_end") == 0)
      nReturn = Run_put_end(argc,argv);
    else if (strcmp(argv[1], "put_pset") == 0)
      nReturn = Run_put_pset(argc,argv);
    else if (strcmp(argv[1], "put_src") == 0)
      nReturn = Run_put_src(argc,argv);
    else if (strcmp(argv[1], "fast_put_src") == 0)
      nReturn = Run_fast_put_src(argc,argv);
    else if (strcmp(argv[1], "fast_merge_src") == 0)
      nReturn = Run_fast_merge_src(argc,argv);
    else if (strcmp(argv[1], "put_start") == 0)
      nReturn = Run_put_start(argc,argv);
    else if (strcmp(argv[1], "setup_src") == 0)
      nReturn = Run_setup_src(argc,argv);
    else if (strcmp(argv[1], "src_version") == 0)
      nReturn = Run_src_version(argc,argv);
    else if (strcmp(argv[1], "stat") == 0)
      nReturn = Run_stat(argc,argv);
    else if (strcmp(argv[1], "unget_pset") == 0)
      nReturn = Run_unget_pset(argc,argv);
    else if (strcmp(argv[1], "unget_src") == 0)
      nReturn = Run_unget_src(argc,argv);
    else if (strcmp(argv[1], "unlock") == 0)
      nReturn = Run_unlock(argc,argv);
    else if (strcmp(argv[1], "is_locked") == 0)
      nReturn = Run_is_locked(argc,argv);
    
    else
    {
      sprintf(sz, "%%%%Internal error: '%s' is an unknown configurator command;\n", argv[1]);
	    ConfigShellOutput( sz );
    }
  }  
    
  return nReturn;
}
    
//--------
int Configurator::Run_cm_info( int argc, char **argv )
{
  SetEnv( CM_OPERATION,   "cm_info" );
  SetEnv( CM_DESCRIPTION,   NULL );
  SetEnv( CM_WORKFILE,      NULL );
  SetEnv( CM_SYSFILE,       NULL );
  SetEnv( CM_REVISION_FILE, NULL );
  SetEnv( CM_SYS_PROJ,      NULL );
  SetEnv( CM_LOCAL_PROJ,    NULL );

  if ( argc >2 )
    SetEnv( CM_OPTIONS, argv[2] );
  else
    SetEnv( CM_OPTIONS, "default" );
  if ( argc > 3 )
  {
	SetEnv( CM_WORKFILE, argv[3] );
	SetEnv( CM_SYSFILE,  argv[3] );
  }

  return cm_info();
}

//--------
int Configurator::Run_exec( int argc, char **argv )
{
  SetEnv( CM_OPERATION,   "exec" );
  SetEnv( CM_DESCRIPTION,   NULL );
  SetEnv( CM_WORKFILE,      NULL );
  SetEnv( CM_SYSFILE,       NULL );
  SetEnv( CM_REVISION_FILE, NULL );
  SetEnv( CM_SYS_PROJ,      NULL );
  SetEnv( CM_LOCAL_PROJ,    NULL );
  if ( argc >2 )
    SetEnv( CM_OPTIONS, argv[2] );
  else
    SetEnv( CM_OPTIONS, "default" );
  if ( argc > 3 )
  {
	SetEnv( CM_WORKFILE, argv[4] );
	SetEnv( CM_SYSFILE,  argv[3] );
  }

  return cm_exec();
}

//--------
int Configurator::Run_lsco( int argc, char **argv )
{
  SetEnv( CM_OPERATION,   "lsco" );
  SetEnv( CM_DESCRIPTION, NULL );
  if ( argc > 3 )
  {
	int len;
	if ( argv[3] && (len=strlen(argv[3])) && *(argv[3]+len-1) == '"' ) *(argv[3]+len-1) = '\\';
	SetEnv( CM_WORKFILE,    argv[3] );
	SetEnv( CM_SYSFILE,     argv[3] );
  }
  else
  {
	SetEnv( CM_WORKFILE,    NULL );
	SetEnv( CM_SYSFILE,     NULL );
  }
  SetEnv( CM_REVISION_FILE, NULL );
  SetSysLocalProjEnv(argc, argv);
  SetOptEnv(argc, argv);

  return cm_lsco();
}

//--------
int Configurator::Run_copy_pset( int argc, char **argv )
{
  int nReturn = 0;

  SetEnv( CM_OPERATION,   "copy_pset" );
  SetEnv( CM_C_FILE,      argv[6] );
  SetEnv( CM_DESCRIPTION, argv[5] );
  SetEnv( CM_WORKFILE,    argv[4] );
  SetEnv( CM_SYSFILE,     argv[3] );
  SetEnv( CM_REVISION_FILE, NULL );
  SetSysLocalProjEnv(argc, argv);
  SetOptEnv(argc, argv);

  nReturn = cm_setup_pset( argv[4], argv[3], argv[1], argv[2] );

  if (nReturn == 99) /* doesn't exist */
    ConfigShellOutput("%%%%stat;not_exist=1;\n");
  else if ( nReturn ==0 )
      nReturn = cm_copy_pset();

  return nReturn;
}

//--------
int Configurator::Run_query_make_output( TargetFileDef* ptfdFile1, TargetFileDef* ptfdFile2 )
{
  int nReturn =0;
  char sz[MAX_PATH+3];
  ConfigShellResult* pOutput;

  while ( (pOutput=ConfigShellLastOutput()) )
    ConfigShellDeleteOutput( pOutput);
  if ( ptfdFile1 && ptfdFile1->GetFlag() == 0 )
  {
    sprintf( sz, "%%%%%s", ptfdFile1->GetSysFile() );
    ConfigShellOutput(sz);
    nReturn =1;
  }
  if ( ptfdFile2 && ptfdFile2->GetFlag() == 0 )
  {
    sprintf( sz, "%%%%%s", ptfdFile2->GetSysFile() );
    ConfigShellOutput(sz);
    nReturn =1;
  }
  return nReturn;
}

//--------
int Configurator::Run_query_get( int argc, char **argv )
{
  TargetFileDef tfdFile1( argv[3], argv[4] );
  TargetFileDef tfdFile2( argv[5], argv[6] );

  SetEnv( CM_OPERATION,   "query_get" );
  SetSysLocalProjEnv(argc, argv);
  SetOptEnv(argc, argv);

  m_ptfdFile = &tfdFile1;
  cm_query_get();
  m_ptfdFile = &tfdFile2;
  cm_query_get();
  m_ptfdFile = NULL;

  return Run_query_make_output( &tfdFile1, &tfdFile2 );
}

//--------
int Configurator::Run_query_put( int argc, char **argv )
{
  int nReturn = 0;
  TargetFileDef tfdFile1( argv[3], argv[4] );
  TargetFileDef tfdFile2( argv[5], argv[6] );
  SetEnv( CM_OPERATION,   "query_put" );

  SetEnv( CM_REVISION_FILE, NULL );
  SetSysLocalProjEnv(argc, argv);
  SetOptEnv(argc, argv);

  m_ptfdFile = &tfdFile1;
  cm_query_put();
  m_ptfdFile = &tfdFile2;
  cm_query_put();
  m_ptfdFile = NULL;

  return Run_query_make_output( &tfdFile1, &tfdFile2 );
}

//--------
int Configurator::Run_copy_src( int argc, char **argv )
{
  int nReturn = 0;

  SetEnv( CM_OPERATION,   "copy_src" );
  SetEnv( CM_DESCRIPTION, argv[5] );
  SetEnv( CM_WORKFILE,    argv[4] );
  SetEnv( CM_SYSFILE,     argv[3] );
  SetEnv( CM_REVISION_FILE, NULL );
  SetSysLocalProjEnv(argc, argv);
  SetOptEnv(argc, argv);

  LogDebug( "Configurator::cm_copy_src", "before cm_setup_src", ""  );

  nReturn = cm_setup_src( argv[4], argv[3], argv[1], argv[2] );

  LogDebugInt( "Configurator::cm_copy_src after cm_setup_src", nReturn );
  if ( nReturn ==0 )
  {
    nReturn = cm_copy_src();
    LogDebugInt( "Configurator::cm_copy_src internal return", nReturn );
    if( nReturn == 0 )
      send_back_files( CM_SYSFILE, CM_REVISION_FILE );
  }
  LogDebugInt( "Configurator::cm_copy_src return", nReturn );
  return nReturn;
}

//--------
int Configurator::Run_create_dir( int argc, char **argv )
{
  SetEnv( CM_OPERATION,   "create_dir" );
  SetEnv( CM_DESCRIPTION,   argv[5] );
  SetEnv( CM_WORKFILE,      argv[4] );
  SetEnv( CM_REVISION_FILE, NULL );
  SetEnv( CM_SYSFILE,       NULL );

  SetSysLocalProjEnv(argc, argv);
  SetOptEnv(argc, argv);

  return cm_create_dir();
}

//--------
int Configurator::Run_create_file( int argc, char **argv )
{
  SetEnv( CM_OPERATION,   "create_file" );
  SetEnv( CM_DESCRIPTION,   argv[5] );
  SetEnv( CM_WORKFILE,      argv[3] );
  SetEnv( CM_REVISION_FILE, NULL );
  SetEnv( CM_SYSFILE,       NULL );

  SetSysLocalProjEnv(argc, argv);
  SetOptEnv(argc, argv);
  
  return cm_create_file();
}

//--------
int Configurator::Run_delete_pset( int argc, char **argv )
{
  SetEnv( CM_OPERATION,   "delete_pset" );
  SetEnv( CM_DESCRIPTION,   argv[5] );
  SetEnv( CM_WORKFILE,      argv[4] );
  SetEnv( CM_REVISION_FILE, NULL );
  SetEnv( CM_SYSFILE,       NULL );

  SetSysLocalProjEnv(argc, argv);
  SetOptEnv(argc, argv);
  
  return cm_delete_pset();
}

//--------
int Configurator::Run_delete_src( int argc, char **argv )
{
  int nReturn = 0;

  SetEnv( CM_OPERATION,   "delete_src" );
  SetEnv( CM_DESCRIPTION, argv[5] );
  SetEnv( CM_WORKFILE,    argv[4] );
  SetEnv( CM_SYSFILE,     argv[3] );

  SetSysLocalProjEnv(argc, argv);
  SetOptEnv(argc, argv);
  
  cm_setup_src( argv[4], argv[3], argv[1], argv[2] );
  
  return cm_delete_src();
}

//--------
int Configurator::Run_diff( int argc, char **argv )
{
  int nReturn = 0;
  
  SetEnv( CM_OPERATION,   "diff" );
  SetEnv( CM_DESCRIPTION, argv[5] );
  SetEnv( CM_WORKFILE,    argv[3] );
  SetEnv( CM_SYSFILE,     argv[4] );
  if (strcmp(argv[5],"1") == 0)
    SetEnv( CM_DISPDIFF, "1" );

  SetSysLocalProjEnv(argc, argv);
  SetOptEnv(argc, argv);
  
  nReturn = cm_setup_src( argv[3], argv[4], argv[1], argv[2] );

  if( nReturn == 99 )
    ConfigShellOutput("%%%%stat;d=1;\n");
  else if ( nReturn == 0 )
  {
    nReturn = cm_diff();
    if( nReturn )
      ConfigShellOutput("%%%%stat;d=1;\n");
  }
  return nReturn;
}

//--------
int Configurator::Run_get_src( int argc, char **argv )
{
  int nReturn = 0;

  SetEnv( CM_OPERATION,   "get_src" );
  SetEnv( CM_DESCRIPTION, argv[5] );
  SetEnv( CM_OPTIONS,     argv[6] );
  SetEnv( CM_WORKFILE,    argv[3] );
  SetEnv( CM_SYSFILE,     argv[4] );
  SetSysLocalProjEnv(argc, argv);
  SetOptEnv(argc, argv);

  nReturn = cm_setup_src( argv[3], argv[4], argv[1], argv[2] );
  if ( nReturn == 0 )
  {
    send_back_files( CM_SYSFILE, CM_REVISION_FILE );
    nReturn = cm_get_src();
		if ( nReturn == 0 )
	    nReturn = cm_lock();
  }
  return nReturn;
}

//--------
int Configurator::Run_get_ver( int argc, char **argv )
{
  int nReturn = 0;
  /*
    cm_version should return a file name
    which contains all interesting versions.
  */
  SetEnv( CM_OPERATION,   "get_ver" );
  SetEnv( CM_DATE, argv[5] );
  SetEnv( CM_DESCRIPTION, argv[5] );
  SetEnv( CM_WORKFILE,    argv[3] );
  SetEnv( CM_SYSFILE,     argv[4] );

  SetSysLocalProjEnv(argc, argv);
  SetOptEnv(argc, argv);
  
  nReturn = cm_setup_src( argv[3], argv[4], argv[1], argv[2] );
  if ( nReturn == 0 )
  {
    if( GetEnv( CM_DATE ) && strlen(GetEnv( CM_DATE )) )
        nReturn = cm_version_date();
    else
        nReturn = cm_version_list();
  }
  return nReturn;
}

//--------
int Configurator::Run_is_locked( int argc, char **argv )
{
  int nReturn = 0;
  
  SetEnv( CM_OPERATION,   "is_locked" );
  SetEnv( CM_DESCRIPTION, argv[5] );
  SetEnv( CM_WORKFILE,    argv[3] );
  SetEnv( CM_SYSFILE,     argv[4] );

  SetSysLocalProjEnv(argc, argv);
  SetOptEnv(argc, argv);
  
  nReturn = cm_setup_src( argv[3], argv[4], argv[1], argv[2] );
  if ( nReturn == 0 )
    nReturn = cm_is_locked();

  return nReturn;
}


//--------
int Configurator::Run_lock( int argc, char **argv )
{
  int nReturn = 0;

  SetEnv( CM_OPERATION,   "lock" );
  SetEnv( CM_DESCRIPTION, argv[2] );
  SetEnv( CM_WORKFILE,    argv[3] );
  SetEnv( CM_SYSFILE,     argv[4] );
  SetEnv( CM_REVISION_FILE, NULL );

  SetSysLocalProjEnv(argc, argv);
  SetOptEnv(argc, argv);
  
  nReturn = cm_setup_src( argv[3], argv[4], argv[1], argv[2] );
  if ( nReturn == 0 )
    nReturn = cm_lock();

  return nReturn;
}

//--------
int Configurator::Run_obsolete_pset( int argc, char **argv )
{
  int nReturn = 0;

  SetEnv( CM_OPERATION,   "obsolete_pset" );
  SetEnv( CM_DESCRIPTION, argv[5] );
  SetEnv( CM_WORKFILE,    argv[3] );
  SetEnv( CM_SYSFILE,     argv[4] );

  SetSysLocalProjEnv(argc, argv);
  SetOptEnv(argc, argv);
  
  nReturn = cm_setup_src( argv[3], argv[4], argv[1], argv[2] );
  if ( nReturn == 0 )
    nReturn = cm_obsolete_pset();
  
  return nReturn;
}

//--------
int Configurator::Run_obsolete_src( int argc, char **argv )
{
  int nReturn = 0;

  SetEnv( CM_OPERATION,   "obsolete_src" );
  SetEnv( CM_DESCRIPTION, argv[5] );
  SetEnv( CM_WORKFILE,    argv[3] );
  SetEnv( CM_SYSFILE,     argv[4] );

  SetSysLocalProjEnv(argc, argv);
  SetOptEnv(argc, argv);
  
  nReturn = cm_setup_src( argv[3], argv[4], argv[1], argv[2] );
  if ( nReturn == 0 )
    nReturn = cm_obsolete_src();

  return nReturn;
}

//--------
int Configurator::Run_put_start( int argc, char **argv )
{
  SetEnv( CM_OPERATION,   "put_start" );
  SetEnv( CM_DESCRIPTION,   argv[5] );
  SetEnv( CM_REVISION_FILE, NULL );
  SetEnv( CM_SYSFILE,       NULL );
  SetSysLocalProjEnv_for_put(argc, argv);
  
  return cm_put_start();
}

//--------
int Configurator::Run_put_end( int argc, char **argv )
{
  SetEnv( CM_OPERATION,   "put_end" );
  SetEnv( CM_DESCRIPTION,   argv[5] );
  SetEnv( CM_REVISION_FILE, NULL );
  SetEnv( CM_SYSFILE,       NULL );
  SetSysLocalProjEnv_for_put(argc, argv);

  return cm_put_end();
}

//--------
int Configurator::Run_put_pset( int argc, char **argv )
{
  int nReturn = 0;

  SetEnv( CM_OPERATION, "put_pset" );
  SetEnv( CM_C_FILE, argv[6] );

  SetEnv( CM_DESCRIPTION, argv[3] );
  SetEnv( CM_WORKFILE,    argv[4] );
  SetEnv( CM_SYSFILE,     argv[5] );
  SetEnv( CM_REVISION_FILE, NULL );
  
  SetSysLocalProjEnv(argc, argv);
  SetOptEnv(argc, argv);
  
  nReturn = cm_setup_pset( argv[4], argv[5], argv[1], argv[2] );
  if ( nReturn == 0 )
    nReturn = cm_put_pset();

  return nReturn;
}

//--------
int Configurator::Run_put_src( int argc, char **argv )
{
  int nReturn = 0;
  /* first time; create a file in source control system */

  SetEnv( CM_OPERATION, "put_src" );

  SetEnv( CM_DESCRIPTION, argv[3] );
  SetEnv( CM_WORKFILE,    argv[4] );
  SetEnv( CM_SYSFILE,     argv[5] );
  SetEnv( CM_REVISION_FILE, NULL );
  
  SetSysLocalProjEnv(argc, argv);
  SetOptEnv(argc, argv);
  
  LogDebug( "Run_put_src", "argv[4]", argv[4] );
  LogDebug( "Run_put_src", "argv[5]", argv[5] );
  LogDebug( "Run_put_src", "argv[1]", argv[1] );
  LogDebug( "Run_put_src", "argv[2]", argv[2] );
  nReturn = cm_setup_src( argv[4], argv[5], argv[1], argv[2] );
  if ( nReturn == 99 )
      nReturn = cm_create();

  if( nReturn == 0)
    send_back_files( CM_SYSFILE, CM_REVISION_FILE );
  nReturn = cm_put_src();

  return nReturn;
}

//--------
int Configurator::Run_fast_put_src( int argc, char **argv )
{
  int nReturn = 0;

  SetEnv( CM_OPERATION, "fast_put_src" );
  SetEnv( CM_DESCRIPTION, argv[3] );
  SetEnv( CM_WORKFILE,    argv[4] );
  SetEnv( CM_SYSFILE,     argv[5] );
  SetEnv( CM_REVISION_FILE, NULL );
    
  SetSysLocalProjEnv(argc, argv);
  SetOptEnv(argc, argv);
  
  nReturn = cm_setup_src( argv[4], argv[5], argv[1], argv[2] );
  if ( nReturn == 0 )
    nReturn = cm_fast_put_src();

  return nReturn;
}

//--------
int Configurator::Run_fast_merge_src( int argc, char **argv )
{
  SetEnv( CM_OPERATION, "fast_merge_src" );
  SetEnv( CM_DESCRIPTION,   argv[3] );
  SetEnv( CM_WORKFILE,      argv[4] );
  SetEnv( CM_REVISION_FILE, NULL );
  SetEnv( CM_SYSFILE,       NULL );

  SetSysLocalProjEnv(argc, argv);
  SetOptEnv(argc, argv);

  return cm_fast_merge_src();
}

//--------
int Configurator::Run_setup_src( int argc, char **argv ) /* for -batch only copy pset */
{
  int nReturn = 0;

  SetEnv( CM_OPERATION, "setup_src" );
  SetEnv( CM_DESCRIPTION, argv[5] );
  SetEnv( CM_WORKFILE,    argv[3] );
  SetEnv( CM_SYSFILE,     argv[4] );

  SetSysLocalProjEnv(argc, argv);
  SetOptEnv(argc, argv);
  
  nReturn = cm_setup_src( argv[3], argv[4], argv[1], argv[2] );
  if ( nReturn == 0 )
    send_back_files( CM_SYSFILE, CM_REVISION_FILE );
  return nReturn;
}

//--------
int Configurator::Run_src_version( int argc, char **argv )
{
  int nReturn = 0;
  /*
    cm_stat will return a text string with the following format
    "%%%%stat;v=version;"
  */

  SetEnv( CM_OPERATION, "src_version" );
  SetEnv( CM_DESCRIPTION, argv[5] );
  SetEnv( CM_WORKFILE,    argv[3] );
  SetEnv( CM_SYSFILE,     argv[4] );

  SetSysLocalProjEnv(argc, argv);
  SetOptEnv(argc, argv);
  
  nReturn = cm_setup_src( argv[3], argv[4], argv[1], argv[2] );
  if ( nReturn == 0 )
  {
    nReturn = cm_src_version();
    if( !nReturn && GetEnv( CM_RESULT ) )
    {
			char  sz[256];
      sprintf( sz, "%%%%stat;v=%s;\n", GetEnv( CM_RESULT ) );
      ConfigShellOutput( sz );
    }
  }

  return nReturn;
}

//--------
int Configurator::Run_stat( int argc, char **argv )
{
  int nReturn = 0;
  /*
    cm_stat will return a text string with the following format
    "%%%%stat;v=version;"
  */
  SetEnv( CM_OPERATION, "stat" );
  SetEnv( CM_DESCRIPTION, argv[5] );
  SetEnv( CM_WORKFILE,    argv[3] );
  SetEnv( CM_SYSFILE,     argv[4] );
  SetEnv( CM_REVISION_FILE, NULL );

  SetSysLocalProjEnv(argc, argv);
  SetOptEnv(argc, argv);
  
  nReturn = cm_setup_src( argv[3], argv[4], argv[1], argv[2] );
  if ( nReturn == 0 )
      nReturn = cm_stat();

  if( !nReturn && GetEnv( CM_RESULT ) )
  {
		char  sz[256];
    sprintf( sz, "%%%%stat;v=%s;\n", GetEnv( CM_RESULT ) );
    ConfigShellOutput( sz );
  }
  return nReturn;
}

//--------
int Configurator::Run_unget_pset( int argc, char **argv )
{
  int nReturn = 0;

  SetEnv( CM_OPERATION, "unget_pset" );
  SetEnv( CM_DESCRIPTION, argv[5] );
  SetEnv( CM_WORKFILE,    argv[4] );
  SetEnv( CM_SYSFILE,     argv[3] );
  SetEnv( CM_REVISION_FILE, NULL );

  SetSysLocalProjEnv(argc, argv);
  SetOptEnv(argc, argv);
  
  nReturn = cm_setup_pset( argv[4], argv[3], argv[1], argv[2] );
  if ( nReturn == 0 )
    nReturn = cm_unget_pset();

  return nReturn;
}

//--------
int Configurator::Run_unget_src( int argc, char **argv )
{
  int nReturn = 0;
  
  SetEnv( CM_OPERATION, "unget_src" );
  SetEnv( CM_DESCRIPTION, argv[5] );
  SetEnv( CM_WORKFILE,    argv[3] );
  SetEnv( CM_SYSFILE,     argv[4] );
  SetEnv( CM_REVISION_FILE, NULL );

  SetSysLocalProjEnv(argc, argv);
  SetOptEnv(argc, argv);
  
  nReturn = cm_setup_src( argv[4], argv[3], argv[1], argv[2] );
  if ( nReturn == 0 )
    nReturn = cm_unget_src();

  return nReturn;
}

//--------
int Configurator::Run_unlock( int argc, char **argv )
{
  int nReturn = 0;

  SetEnv( CM_OPERATION, "unlock" );

  char *pszComment = get_option(argv[2], "-ParaUnlock");
  SetEnv( CM_DESCRIPTION, pszComment );
  if( pszComment )
    free( pszComment );

  SetEnv( CM_WORKFILE,    argv[3] );
  SetEnv( CM_SYSFILE,     argv[4] );
  SetEnv( CM_REVISION_FILE, NULL );

  SetSysLocalProjEnv(argc, argv);
  SetOptEnv(argc, argv);
  
  nReturn = cm_setup_src( argv[3], argv[4], argv[1], argv[2] );
  if ( nReturn == 0 )
      nReturn = cm_unlock();

  return nReturn;
}

    
//--------
void Configurator::send_back_files( enum ENVVARS nSysFile, enum ENVVARS nRevisFile )
{
  char *pszSysFile = GetEnv(nSysFile);
  char *pszRevisFile = GetEnv(nRevisFile); 
  if( !pszSysFile )
    pszSysFile = "";
  if( !pszRevisFile )
    pszRevisFile = "";
	
	char sz[ 2*MAX_PATH +16];
//	sprintf(sz, "%%%%sf=%s;vf=%s;", pszSysFile, pszRevisFile);
//	ConfigShellOutput( sz );
  sprintf(sz, "%%%%stat;v=%s", pszRevisFile );
	ConfigShellOutput( sz );
}


//--------
/* This function will run a script specified by pszCommandLine and will
*  return 1 if script was found and interpreted by a command interpreter.
*  If fIsSetupScript is TRUE, it will use script's output to set CM_SYSFILE
*  and CM_REVISION_FILE
*/

int Configurator::RunScript( char *pszScriptName, char *pszScriptParamenters, int &nRetCode,
               BOOL fIsSetupScript, BOOL fIsVersionScript, char* pszPrefix)
{
  m_fScriptInvoked = FALSE;
  int nReturn = 0;
  nRetCode = -1;
  HANDLE hScriptStdoutRd, hScriptStdoutWr;
  char pszCmd[ COMMAND_LEN ];
  char *pszDir, szDir[ _MAX_PATH ];
  // First check if script file exists. We will look for a file in a directory 
  // specified by PSETCFG
  // environment variable or current directory if it isn't set. We will accept
  // file as a script if it has no extension, or has .bat, .com, .exe or .cmd
  // extension
  int nNameLen = 0;
  if( (pszDir= GetEnv(CM_PSETCFG)) && *pszDir )
  {
		strcpy( szDir, pszDir );
    if ( szDir[0] == '/' && szDir[2] == ':' && isalpha(szDir[1]) )
      strcpy( &(szDir[0]), &(szDir[1]) );
    strcat( szDir, "/" );
    strcat( szDir, pszScriptName );
  }
  else
    strcpy( szDir, pszScriptName );

  nNameLen = strlen( szDir );
  WIN32_FIND_DATA find_data;
  BOOL fFileFound = FALSE;
  HANDLE hFind = FindFirstFile( szDir, &find_data );
  if( hFind == INVALID_HANDLE_VALUE )
  {
    char *ExeExt[]={ ".bat", ".com", ".exe", ".cmd" };
    for( int n=0; n < (sizeof(ExeExt)/sizeof(char *)) && !fFileFound; n++ )
    {
      strcpy( szDir + nNameLen, ExeExt[n] );
      hFind = FindFirstFile( szDir, &find_data );
      if( hFind != INVALID_HANDLE_VALUE )
      {
        FindClose( hFind );
        fFileFound = TRUE;
      }
    }
  }
  else
  {
    FindClose( hFind );
    fFileFound = TRUE;
  }
  if( fFileFound )
  {
    szDir[ nNameLen ] = '\0';
    // CF_SHELL environment variable is used to get a command interpreter name
    // If CF_SHELL is not set, COMSPEC environment var is used. If latter is not set,
    // cmd.exe will be run for WinNT and command.com for Windows 95
    if( !GetEnvironmentVariable( "CF_SHELL", pszCmd, COMMAND_LEN ) )
    {
      if( !GetEnvironmentVariable( "COMSPEC", pszCmd, COMMAND_LEN ) )
      {
        OSVERSIONINFO os_info;
        if( GetVersionEx( &os_info ) && 
          os_info.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS )
          strcpy( pszCmd, "command.com" ); //this is Win95
        else
          strcpy( pszCmd, "cmd.exe" );
      }
      strcat( pszCmd, " /C" );
    }
    strcat( pszCmd, " " );

    strcat( pszCmd, szDir );
    strcat( pszCmd, " " );
    strcat( pszCmd, pszScriptParamenters );
    //Create pipe for script's stdout
    SECURITY_ATTRIBUTES sa;
    ZeroMemory( &sa, sizeof(sa) );
    sa.bInheritHandle = TRUE;
    if( CreatePipe(&hScriptStdoutRd, &hScriptStdoutWr, &sa, 0) )
    {
      FILE *pfOut = NULL;
      int nOut = _open_osfhandle( (long)hScriptStdoutRd, _O_TEXT );
      if( nOut != -1 )
        pfOut = _fdopen( nOut, "r" );
      if( pfOut )
      {
        PROCESS_INFORMATION pi;
        STARTUPINFO si;
        ZeroMemory( &pi, sizeof( pi ) );
        ZeroMemory( &si, sizeof( si ) );
        si.cb = sizeof( si );
        si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
				si.wShowWindow = SW_HIDE;
        si.hStdInput = NULL;
        si.hStdOutput = hScriptStdoutWr;
        si.hStdError = GetStdHandle( STD_ERROR_HANDLE );
        //Create environment block for a script
        void *pSysEnv = GetEnvironmentStrings(); //first get this process's environment
        //calculate length of the buffer
        int nSysLen = 0;
        if( pSysEnv )
        {
          char *pszString = (char *)pSysEnv;
          while( *pszString )
          {
            nSysLen+=(strlen( pszString ) + 1);
            pszString = (char *)pSysEnv + nSysLen;
          }
          nSysLen++;
        }
        int nConfLen = 0;
        void *pConfEnv = m_ConfEnv.GetEnvBlock( &nConfLen ); //now get "fake" environment block created by configurator
        void *pScriptEnv = (void *)new char[ nConfLen + nSysLen - 1 ]; //allocate memory for script's env 
        if( pScriptEnv )
        {
          if( pSysEnv && nSysLen )
            memcpy( pScriptEnv, pSysEnv, nSysLen );
          char *pDest = (nSysLen > 0) ? (char *)pScriptEnv + nSysLen - 1 : (char *)pScriptEnv;
          if( pConfEnv && nConfLen )
            memcpy( pDest, pConfEnv, nConfLen ); 
        }
        FreeEnvironmentStrings( (char *)pSysEnv );
        if( pConfEnv )
          delete pConfEnv; 
        nReturn = CreateProcess( NULL, pszCmd, NULL, NULL, TRUE/*inherit handles*/, 0,
                           pScriptEnv, NULL, &si, &pi);
        // Close pipe's write handle.
        CloseHandle( hScriptStdoutWr );
        if( pScriptEnv )
          delete pScriptEnv;
        if( nReturn ) //script was started successfully
        {
          m_fScriptInvoked = TRUE;

          char buff[ SETUP_SCRIPT_OUTPUT_LEN+1 ];
          int nLine = 0;
          if( fIsSetupScript )
          {
            while( fgets(buff, SETUP_SCRIPT_OUTPUT_LEN, pfOut) && nLine < 2 )
            {
              LogMessage( "ConfigShellRunScript:1", "line:", buff );
              if( strlen( buff ) )
              {
                nLine++;
                int nLast = strlen(buff) - 1;
                if( buff[ nLast ] == '\n' )
                  buff[ nLast ] = '\0';
                switch( nLine )
                {
                case 1:
                  SetEnv( CM_SYSFILE, buff );
                  break;
                case 2:
                  SetEnv( CM_REVISION_FILE, buff );
                  break;
                }
              }
            }
          }
          else if( fIsVersionScript )
          {
            while( fgets(buff, SETUP_SCRIPT_OUTPUT_LEN, pfOut) )
            {
              LogMessage( "ConfigShellRunScript:2", "line:", buff );
              if( strlen( buff ) )
              {
                SetEnv( CM_RESULT, buff );
                break;
              }
            }
          }
	  else
	  {
	    char* pszOutScriptBuff = buff;
	    int	  nBuffLen = SETUP_SCRIPT_OUTPUT_LEN;
	    if ( pszPrefix && *pszPrefix )
	    {
	      strcpy( buff, pszPrefix );
	      pszOutScriptBuff = buff + strlen( buff );
	      nBuffLen = SETUP_SCRIPT_OUTPUT_LEN - strlen(buff);
	    }
        while( fgets(pszOutScriptBuff, SETUP_SCRIPT_OUTPUT_LEN, pfOut) )
        {
          LogMessage( "ConfigShellRunScript:3", "line:", buff );
	        ConfigShellOutput(buff);
        }
	    }
          fclose( pfOut );
          // Wait for a spawned process to terminate
          // If WaitForSingleObject returns with WAIT_TIMEOUT,
          // we will terminate process since it may be hung
          if( WaitForSingleObject( pi.hProcess, SCRIPT_EXECUTION_TIMEOUT ) == WAIT_TIMEOUT )
            TerminateProcess( pi.hProcess, 100/*exit code*/ );
          nRetCode = 0;
//          unsigned long nPr;
//          if( GetExitCodeProcess( pi.hProcess, &nPr ) && nPr != STILL_ACTIVE )
//            nRetCode = nPr;
//          else
//            nRetCode = -1;
        }
        else
          fclose( pfOut );
      }
      else
      {
        CloseHandle( hScriptStdoutRd );
      }
    }
    else
      ConfigShellOutput( "%%Error: Could not redirect stdout for a script" );
  }

  if ( m_fScriptInvoked )
			LogMessage( "ConfigShellExecute", "script was invoked", pszCmd );

  ConfigShellResult* pResult;
  if ( (pResult = ConfigShellFindOutput( "%%cm_continue" )) )
  {
		LogMessage( "ConfigShellExecute", "%%cm_continue processed", pszCmd );
    m_fScriptInvoked = FALSE;
    ConfigShellDeleteOutput(pResult );
  }
  if ( (pResult = ConfigShellFindOutput( "%%error" )) || (pResult = ConfigShellFindOutput( "%%Error" )) )
  {
		LogMessage( "ConfigShellExecute", "catch %%%%error", "" );
    char* p =pResult->GetText();
    nRetCode = atoi(p + 7);
    if ( nRetCode == 0 ) nRetCode=1;
    ConfigShellDeleteOutput(pResult );
  }

	LogMessageInt( "ConfigShellExecute", nReturn );
  return nReturn;
}


//---------------------
/* char ** ParseCommandLine( char *pszCmdLine, int *pArgc )
*  This function uses CommandLineToArgvW() to parse a command line
*  It is a responsibility of a caller to delete memory allocated for
*  argument list.
*  For example: 
*  char **argv = ParseCommandLine( some args );
*  ...
*  Do smth.
*  ...
*  delete argv;
*  A command line IS modified by this function
*/

char ** ParseCommandLine( char *pszCmdLine, int *pArgc )
{
  char ** pRet = NULL;
  int nLength = strlen( pszCmdLine );
  if( nLength )
  {
    //convert pszCmdLine to UNICODE string
    wchar_t *pUStr = new wchar_t[ nLength + 1 ];
    if( pUStr )
    {
      if ( MultiByteToWideChar( CP_ACP, 0, pszCmdLine, nLength + 1, 
                                pUStr, nLength + 1 ) )
      {
        wchar_t **pUniOpts = CommandLineToArgvW( pUStr, pArgc );
        if( pUniOpts )
        {
          pRet = new char *[ *pArgc ];
          if( pRet )
          {
            //count number of bytes to be copied and set array elements
            nLength = 0;
            for( int i = 0; i < (*pArgc); i ++ )
            {
              pRet[ i ] = pszCmdLine + nLength;
              nLength += (wcslen( pUniOpts[i] ) + 1);
            }
            if( !WideCharToMultiByte( CP_ACP, 0, *pUniOpts, nLength, 
                                     pszCmdLine, nLength, NULL, NULL ) )
            {
              delete pRet;
              pRet = NULL;
            }
          }
          //Free memory allocated by CommandLineToArgvW()
          HLOCAL hMem = LocalHandle( pUniOpts );
          LocalFree( hMem );
        }
      }
      delete pUStr;
    }
  }

  return pRet;
}

//---------------------
void cfg_splitpath(char *wf, char *sourcepathpart, char *sourcefilepart)
{
  char drive[_MAX_DRIVE];
  char dir[_MAX_DIR];
  char ext[_MAX_EXT];
  _splitpath( wf, drive, dir, sourcefilepart, ext );
  //if( strlen(dir) > 1 ) //if it is not a root dir
  if( strlen(dir) )
    dir[ strlen( dir ) - 1 ] = '\0'; //get rid of a last slash
  sprintf( sourcepathpart, "%s%s", drive, dir );
  if( !strlen( sourcepathpart ) )
    strcpy( sourcepathpart, "." );
  strcat( sourcefilepart, ext );
}

//---------
/* 
* char * get_option(char *opt, char *sw_ptn) 
* Please NOTE: This functions uses strdup to allocate 
* buffer for result string. It is a responsibility of the caller 
* to deallocate it with a call to free() 
*/ 
 
char * get_option(char *opt, char *sw_ptn) 
{ 
  char seps[]   = " \t\n"; //possible separators for tokens in option line 
  char *pRet = NULL; 
 
  char *pszTok = strtok( opt, seps ); 
  while( pszTok ) 
  { 
    if( !strcmp( pszTok, sw_ptn ) ) //option found, next token should be a value 
    { 
      pszTok = strtok( NULL, seps ); 
      if( pszTok ) 
        pRet = _strdup( pszTok ); 
      break; 
    } 
    else 
      pszTok = strtok( NULL, seps ); 
  } 
  return pRet; 
} 
 
//---------------------
int find_option(char *opt, char *sw_ptn)
{
    return (strstr(opt, sw_ptn) ? 1 : 0);
}


//---------------------
BOOL CreateParentDirectory( char *pszFile )
{
  BOOL fResult = FALSE;
  if( pszFile )
  {
    char szDir[ MAX_PATH ];
    strcpy( szDir, pszFile );
    char *pLastSlash = GetLastSlash( szDir );
    if( pLastSlash )
    {
      *pLastSlash = '\0';
      //Check if parent directory is root, look for another slash
      pLastSlash = GetLastSlash( szDir );
      if( pLastSlash )
      {
        if( _access( szDir, 0 ) ) //directory doesn't exist
        {
          if( !CreateDirectory( szDir, NULL ) )
          {
            //get error code
            long lErr = GetLastError();
            if( lErr == ERROR_PATH_NOT_FOUND )
            {
              //try to create a parent dir
              if( CreateParentDirectory( szDir ) )
                fResult = CreateDirectory( szDir, NULL );
            }
          }
          else //CreateDirectory() succeded
            fResult = TRUE;
        }
        else
          fResult = TRUE; //directory already exists
      }
      else
        fResult = TRUE;  //parent directory is probably root
    }
  }
  return fResult;
}

//--------
 PROCESS_INFORMATION pi;

//--------
FILE* CM_OpenPipe( char *pszCmd )
{
  FILE *pfOut = NULL;

    HANDLE hScriptStdoutRd, hScriptStdoutWr;
    SECURITY_ATTRIBUTES sa;
    ZeroMemory( &sa, sizeof(sa) );
    sa.bInheritHandle = TRUE;
    if( CreatePipe(&hScriptStdoutRd, &hScriptStdoutWr, &sa, 0) )
    {
      int nOut = _open_osfhandle( (long)hScriptStdoutRd, _O_TEXT );
      if( nOut != -1 )
        pfOut = _fdopen( nOut, "r" );
      if( pfOut )
      {
        STARTUPINFO si;
        ZeroMemory( &pi, sizeof( pi ) );
        ZeroMemory( &si, sizeof( si ) );
        si.cb = sizeof( si );
        si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
				si.wShowWindow = SW_HIDE;
        si.hStdInput = NULL;
        si.hStdOutput = hScriptStdoutWr;
        si.hStdError = GetStdHandle( STD_ERROR_HANDLE );
        int nReturn = CreateProcess( NULL, pszCmd, NULL, NULL, TRUE/*inherit handles*/, 0,
                           NULL, NULL, &si, &pi);
        // Close pipe's write handle.
        CloseHandle( hScriptStdoutWr );
        if( nReturn == 0 )
        {
          fclose( pfOut );
          LogDebug( "CM_OpenPipe", "error: CreateProcess", "" );
          pfOut = NULL;
        }
      }
      else
       {
        CloseHandle( hScriptStdoutRd );
        LogDebug( "CM_OpenPipe", "can not open a pipe", "" );
        pfOut = NULL;
       }
    }
    else
      LogDebug( "CM_OpenPipe", "can not create a pipe", "" );

  return pfOut;
}


//--------------------------------
int CM_ReadPipe(char* pszLine, int nLineLen, FILE* pfOut)
{
  if ( pfOut && pszLine && nLineLen > 0 )
  {
    *pszLine= 0;
  	if ( !feof(pfOut) && fgets(pszLine, nLineLen, pfOut) )
    {
      int n=strlen(pszLine)-1;
      char c;
      while ( n>=0 && (c=*(pszLine+n)) == 0x0A || c == 0x0D )
      {
        *(pszLine+n) =0;
        n--;
      }
   	  LogDebug( "CM_ReadPipe", "line==>", pszLine );
      return strlen(pszLine);
    }
  }
  return 0;
}

//--------------------------------
void CM_ClosePipe(FILE* pfOut )
{
  char szLine[3*_MAX_PATH] = "";
  while ( !feof(pfOut) && fgets(szLine, sizeof(szLine), pfOut) );
  fclose( pfOut );

  // Wait for a spawned process to terminate
  // If WaitForSingleObject returns with WAIT_TIMEOUT,
  // we will terminate process since it may be hung
  if( WaitForSingleObject( pi.hProcess, SCRIPT_EXECUTION_TIMEOUT )
    == WAIT_TIMEOUT )
    TerminateProcess( pi.hProcess, 100/*exit code*/ );
}

