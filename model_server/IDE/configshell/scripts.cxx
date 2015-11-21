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
#include "configurator.h"

#define	_THIS_FILE_		"Configurator_scripts"


//-----------------------------------
int Configurator::cm_setup_src( char *pszWorkPath, char *pszSysPath, char *pszCommand, char *pszOpt )
{
  int   nResult = 0;

  char  szWorkDir  [_MAX_PATH] = "";
  char  szWorkFile [_MAX_PATH] = "";
  char  szSysDir   [_MAX_PATH] = "";
  char  szSysFile  [_MAX_PATH] = "";
  char  szCmd      [ COMMAND_LEN ];

  cfg_splitpath(pszWorkPath, szWorkDir, szWorkFile);
  cfg_splitpath(pszSysPath,  szSysDir,  szSysFile);

  SetEnv( CM_SYSFILE, pszSysPath );
  SetEnv( CM_REVISION_FILE, pszWorkPath );

  sprintf( szCmd, "%s %s %s %s %s %s", szWorkDir, szWorkFile,
           szSysDir, szSysFile, pszCommand, pszOpt );
  LogDebug( "Configurator::cm_setup_src", "RunScript", szCmd );
  RunScript( "cm_setup_src", szCmd, nResult, TRUE );

  if ( nResult == 1 )
    ConfigShellOutput("%%Setup script failed\n");
  return nResult;
}

//-----------------------------------
int Configurator::cm_setup_pset( char *pszWorkPath, char *pszSysPath, char *pszCommand, char *pszOpt )
{
  //Set CM_SYSFILE and CM_REVISION_FILE here (using ConfEnv::SetEnv())
  
  char  szWorkDir  [_MAX_PATH] = "";
  char  szWorkFile [_MAX_PATH] = "";
  char  szSysDir   [_MAX_PATH] = "";
  char  szSysFile  [_MAX_PATH] = "";
  int   nResult = 0;
  char  szCmd[ COMMAND_LEN ];

  cfg_splitpath(pszWorkPath, szWorkDir, szWorkFile);
  cfg_splitpath(pszSysPath,  szSysDir,  szSysFile);

  SetEnv( CM_SYSFILE,  pszSysPath );
  SetEnv( CM_WORKFILE, pszWorkPath );

  sprintf( szCmd, "%s %s %s %s %s %s", szWorkDir, szWorkFile,
           szSysDir, szSysFile, pszCommand, pszOpt );
//TMP FIX
  char archive[ _MAX_PATH ];
  sprintf( archive, "%s\\PVCS\\%s", szWorkDir, szWorkFile );
  char *pDot = strrchr( archive, '.' );
  if( pDot )
  {
    pDot[3] = 'v';
    pDot[4] = '\0';
    SetEnvironmentVariable( "ARCH_MFC", archive );
  }
  
  SetEnvironmentVariable( "ARCH_MFC_DIR", szWorkDir );

  RunScript( "cm_setup_pset", szCmd, nResult, TRUE );

  if ( nResult == 1 )
    ConfigShellOutput("%%Setup script failed\n");
  return nResult;
}


//-----------------------------------
int Configurator::cm_info() {
  int nResult = 0; RunScript( "cm_info", "", nResult);
  return nResult;
}

int Configurator::cm_lsco() {
  int nResult = 0; RunScript( "cm_lsco", "", nResult);
  return nResult;
}

int Configurator::cm_query_put() {
  int nResult = 0; RunScript( "cm_query_put", "", nResult);
  return nResult;
}

int Configurator::cm_query_get() {
  int nResult = 0; RunScript( "cm_query_get", "", nResult);
  return nResult;
}

int Configurator::cm_copy_pset() {
  int nResult = 0; RunScript( "cm_copy_pset", "", nResult);
  return nResult;
}

int Configurator::cm_copy_src() {
  LogDebug( "Configurator::cm_copy_src", "RunScript", "" );
  int nResult = 0; RunScript( "cm_copy_src", "", nResult);
  LogDebugInt( "Configurator::cm_copy_src", nResult );
  return nResult;
}

int Configurator::cm_create() {
  int nResult = 0; RunScript( "cm_create", "", nResult);
  return nResult;
}

int Configurator::cm_create_dir() {
  int nResult = 0; RunScript( "cm_create_dir", "", nResult);
  return nResult;
}

int Configurator::cm_create_file() {
  int nResult = 0; RunScript( "cm_create_file", "", nResult);
  return nResult;
}

int Configurator::cm_delete_src() {
  int nResult = 0; RunScript( "cm_delete_src", "", nResult);
  return nResult;
}

int Configurator::cm_delete_pset() {
  int nResult = 0; RunScript( "cm_delete_pset", "", nResult);
  return nResult;
}

int Configurator::cm_diff() {
  int nResult = 0; RunScript( "cm_diff", "", nResult, FALSE, FALSE, "%%");
  return nResult;
}

int Configurator::cm_get_src() {
  int nResult = 0; RunScript( "cm_get_src", "", nResult);
  return nResult;
}

int Configurator::cm_lock()  {
  int nResult = 0; RunScript( "cm_lock", "", nResult, FALSE, FALSE, "%%");
  return nResult;
}

int Configurator::cm_obsolete_src() {
  int nResult = 0; RunScript( "cm_obsolete_src", "", nResult);
  return nResult;
}

int Configurator::cm_obsolete_pset() {
  int nResult = 0; RunScript( "cm_obsolete_pset", "", nResult);
  return nResult;
}

int Configurator::cm_put_end() {
  int nResult = 0; RunScript( "cm_put_end", "", nResult);
  return nResult;
}

int Configurator::cm_put_pset() {
  int nResult = 0; RunScript( "cm_put_pset", "", nResult);
  return nResult;
}

int Configurator::cm_put_src() {
  int nResult = 0; RunScript( "cm_put_src", "", nResult);
  return nResult;
}

int Configurator::cm_put_start() {
  int nResult = 0; RunScript( "cm_put_start", "", nResult);
  return nResult;
}

int Configurator::cm_fast_put_src() {
  int nResult = 0; RunScript( "cm_fast_put_src", "", nResult);
  return nResult;
}

int Configurator::cm_fast_merge_src() {
  int nResult = 0; RunScript( "cm_fast_merge_src", "", nResult);
  return nResult;
}

int Configurator::cm_src_version() {
  int nResult = 0; RunScript( "cm_src_version", "", nResult);
  return nResult;
}

int Configurator::cm_unget_src() {
  int nResult = 0; RunScript( "cm_unget_src", "", nResult);
  return nResult;
}

int Configurator::cm_unget_pset() {
  int nResult = 0; RunScript( "cm_unget_pset", "", nResult);
  return nResult;
}

int Configurator::cm_unlock() {
  int nResult = 0; RunScript( "cm_unlock", "", nResult, FALSE, FALSE, "%%");
  return nResult;
}

int Configurator::cm_version() {
  int nResult = 0; RunScript( "cm_version", "", nResult);
  return nResult;
}

int Configurator::cm_version_list() {
  int nResult = 0; RunScript( "cm_version_list", "", nResult, 0, 1);
  return nResult;
}

int Configurator::cm_version_date() {
  int nResult = 0; RunScript( "cm_version_date", "", nResult);
  return nResult;
}

int Configurator::cm_stat() {
  int nResult = 0; RunScript( "cm_stat", "", nResult);
  return nResult;
}

int Configurator::cm_is_locked() {
  int nResult = 0; RunScript( "cm_is_locked", "", nResult);
  return nResult;
}

int Configurator::cm_exec() {
  return 0;
}

