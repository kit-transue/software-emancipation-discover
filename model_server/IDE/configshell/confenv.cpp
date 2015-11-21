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

/***********************************************************/
/***      Implemantation of ConfEnv                     ***/
/***********************************************************/

static char *VarNames[] =
{
  "cm_c_file", 
  "cm_description_file", 
  "cm_revision_file",
  "cm_sysfile", 
  "cm_workfile", 
  "cm_files",        
  "cm_project",        
  "cm_localdir",
  "cm_fname",        
  "cm_user",        
  "cm_provider",        

  "cm_rmpset", 
  "cm_isclearcase",
  "cm_result", 
  "cm_dispdiff",
  "cm_psetcgf",
  "cm_bugno", 
  "cm_comment", 
  "cm_date", 
  "cm_description", 
  "cm_file_type", 
  "cm_local_proj", 
  "cm_options", 
  "cm_pset_search", 
  "cm_version", 
  "cm_sys_proj", 

  "cm_operation", 
};


ConfEnv::ConfEnv()
{
  for( int i=0; i < sizeof( m_pVars )/sizeof( char * ); i++ )
    m_pVars[ i ] = NULL;
}

ConfEnv::~ConfEnv()
{
  for( int i=0; i < sizeof( m_pVars )/sizeof( char * ); i++ )
    if( m_pVars[ i ] )
      delete m_pVars[ i ];
}

BOOL ConfEnv::SetEnv( ENVVARS index, char *pszNewValue )
{
  BOOL fResult = FALSE;
  if ( 0<= index && index < ENV_VARS_NUM )
  {
    char* pOldVar = m_pVars[ index ];
    if ( pOldVar == pszNewValue )
      fResult = TRUE;
    else
    {
      m_pVars[ index ] = NULL;
  
      if( pszNewValue )
      {
	m_pVars[ index ] = new char [ strlen(pszNewValue) + 1 ];
	if( m_pVars[ index ] )
	{
	  strcpy ( m_pVars[ index ], pszNewValue );
	  fResult = TRUE;
	}

	if ( index <= CM_FILES )
	{
	  char* p= m_pVars[ index ];
	  while ( p && (p=strchr(p, '/')) )
	    *p = '\\';
	}
	else if ( index == CM_DESCRIPTION )
	{
	  SetEnv( CM_DESCRIPTION_FILE, NULL );
	  OFSTRUCT	OFS;
	  HFILE hFile = OpenFile( pszNewValue, &OFS, OF_READ );
	  if ( hFile != HFILE_ERROR )
	  {
	    SetEnv( CM_DESCRIPTION_FILE, pszNewValue );
	    DWORD dwLen = GetFileSize( (HANDLE)hFile,	NULL );
	    if ( dwLen )
	    {
	      char* pszBuff = new char [ dwLen +2 ];
	      if ( pszBuff )
	      {
		if ( ReadFile( (HANDLE)hFile, pszBuff, dwLen, &dwLen, NULL) )
		{
		  *(pszBuff + dwLen ) = 0;
		  delete m_pVars[ index ]; 
		  m_pVars[ index ] = pszBuff;
		  SetEnv( CM_DESCRIPTION_FILE, pszNewValue );
		}
		else
		  delete [] pszBuff;
	      }
	    }
	    CloseHandle( (HANDLE)hFile );
	  }
	}
      }
      else //unset environment variable
	fResult = TRUE;

      if( pOldVar )
	delete pOldVar;
    }
  }
  return fResult;
}

void * ConfEnv::GetEnvBlock( int *nBlockLen )
{
  char *pReturn = NULL;
  *nBlockLen = 0;
  int nVars = sizeof(VarNames)/sizeof(char *);
  //calculate length of environment block to be allocated
  int nLen = 0;
  for( int i = 0; i < nVars; i++ )
  {
    nLen+=(strlen(VarNames[i]) + 2);
    char *pVal = GetEnv( (ENVVARS)i );
    if( pVal )
      nLen+=strlen(pVal);
  }
  nLen++; //one more byte for a last 0-terminator
  pReturn = new char[ nLen ];
  if( pReturn )
  {
    *nBlockLen = nLen;
    char *pStr = pReturn;
    for( i=0; i < nVars; i++ )
    { 
      pStr+=sprintf( pStr, "%s=", VarNames[i] );
      char *pVal = GetEnv( (ENVVARS)i );
      if( pVal )
        pStr+=sprintf( pStr, pVal );
      pStr++;
    }
    *pStr = '\0';
  }
  return pReturn;
}

