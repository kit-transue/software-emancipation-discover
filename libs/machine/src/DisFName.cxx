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
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <stdio.h>
#include <io.h>
#include <cctype>
#include "DisFName.h"

//int _access( const char *path, int mode );
//char *_fullpath( char *absPath, const char *relPath, size_t maxLength );

extern "C" int g_DisFN_RealCase = 0;

/***************************************************************
  DrvLetterMap
****************************************************************/
class DrvLetterMap
{
private:
  char* m_pszDrvLetterMap['z'-'a' +1];

public:

   DrvLetterMap();
  ~DrvLetterMap();

   char*  getDrvLetterMap( char cDriveLetter );
};

static DrvLetterMap* gpDrvLetterMap = NULL;
//-------------------------
DrvLetterMap::DrvLetterMap()
{
  char  szDrvConnected[128], *pszDrvConnected = szDrvConnected;
  char* pszDrvPath['z'-'a' +1];

  char  c, d, e;
  char* psz;
  char  szDrive[] = "c:";
  char  szNetPath[_MAX_PATH];
  DWORD  nLen;
  DWORD nGetConnResult;
  GetLogicalDriveStrings( sizeof(szDrvConnected), pszDrvConnected );

  for ( c='a'; c<='z'; c++ )
  {
    pszDrvPath        [c-'a'] = NULL;
    m_pszDrvLetterMap [c-'a'] = NULL;
  }

  while ( (c= *pszDrvConnected) )
  {
	pszDrvConnected += strlen(pszDrvConnected)+1;
    if ( isalpha(c) )
	{
    szDrive[0] = c = tolower(c);
    nLen = sizeof(szNetPath);
	nGetConnResult = WNetGetConnection( szDrive, szNetPath, &nLen );
	if( nGetConnResult == NO_ERROR )
		{
		  if ( (e=szNetPath[ strlen(szNetPath)-1 ]) == '\\' || e == '/' )
			szNetPath[ strlen(szNetPath)-1 ] = '\0';
		  pszDrvPath[c-'a'] = new char [ strlen(szNetPath) +1 ];
		  szNetPath[0] = tolower(szNetPath[0]);
		  strcpy( pszDrvPath[c-'a'], szNetPath );
		}
	} // if ( isalpha(c) )
  }
  
  char* pszSrcDrive;
  char* pszDstDrive;
  int   nLenSrc, nLenDst, nLenSubst;
  char  cSubst;
  for ( c='c'; c<= 'z'; c++ )
  {
    if ( (pszSrcDrive = pszDrvPath[c-'a']) )
    {
      nLenSrc = strlen( pszSrcDrive );
      nLenSubst = nLenSrc;
      cSubst = '\0';
      for ( d= 'c'; d<= 'z'; d++ )
      {
        if ( d == c || (pszDstDrive = pszDrvPath[d-'a']) == NULL  )
          continue;
        if ( (nLenDst=strlen(pszDstDrive)) > nLenSrc )
          continue;
        if ( memcmp( pszSrcDrive, pszDstDrive, nLenDst ) )
          continue;
        if ( (e= *(pszSrcDrive+nLenDst)) != '\\' && e != '\0' )
          continue;
        if ( nLenDst <= nLenSubst )
        {
          nLenSubst = nLenDst;
          cSubst = d;
        }

      } // for ( d= 'c'; d<= 'z'; d++ )

      if ( cSubst )
      {
      szNetPath[0]= cSubst;
      szNetPath[1] = ':';
      strcpy( &(szNetPath[2]), (pszDrvPath[c-'a'])+nLenSubst );
      while ( (psz=strchr(szNetPath, '\\') ) )
        *psz = '/';

      m_pszDrvLetterMap[c-'a'] = new char [ strlen(szNetPath) +1 ];
      strcpy( m_pszDrvLetterMap[c-'a'], szNetPath );
      
      char msg[MAX_PATH];
      sprintf(msg, "%c: -> %s\n", c, szNetPath );
      }

    } // if ( (pszSrcDrive = pszDrvPath[c-'a']) )
   }  // for ( c='c'; c<= 'z'; c++ )

}

//-------------------------
DrvLetterMap::~DrvLetterMap()
{
  for ( char c='a'; c<='z'; c++ )
  {
    if ( m_pszDrvLetterMap [c-'a'] )
      delete [] m_pszDrvLetterMap [c-'a'];
    m_pszDrvLetterMap [c-'a'] = NULL;
  }
}

char*  DrvLetterMap::getDrvLetterMap( char cDriveLetter )
{
  if ( !isalpha(cDriveLetter) )
    return NULL;
  else
    return m_pszDrvLetterMap[ tolower(cDriveLetter)-'a' ];
}

//-------------------------------------------------------



char* get_drv_letter_map( char* path )
{
  if ( !gpDrvLetterMap )
    gpDrvLetterMap = new DrvLetterMap;

  if ( !path || *path == '\0' || *(path+1) != ':' )
    return NULL;

  return gpDrvLetterMap->getDrvLetterMap( *path );
}

/***************************************************************
  OSPATH
****************************************************************/
int DisFN_isOSPATH()
{
  static int nOSPATH= -1;
	if ( nOSPATH < 0 )
		{
     char* pszOSPATH;
		 if ( (pszOSPATH = ::getenv("DIS_NO_OS_PATH")) != NULL && atoi(pszOSPATH) != 0 )
			 nOSPATH = 0;
		 else
			 nOSPATH = 1;
		}
	return nOSPATH;
}

//----------------------------------------------
int DisFN_is_abs_path( char * path )
{
	return ( path && *(path+1) == ':' && (*(path+2) == '/' || *(path+2) == '\\') && isalpha(*path) );
}

//----------------------------------------------

char* DisFN_realpath( const char *path, char *resolved )
{
  char* path_map;
  char* ret;

	if ( DisFN_is_abs_path((char*)path) && strlen(path) == 3 && DisFN_isOSPATH() )
	{
	  if ( (path_map=get_drv_letter_map( (char*)path )) )
        strcpy( resolved, path_map );
    else
        strcpy(resolved, (char*)path);
    ret = (char*)resolved;
	} 
	else
	{
    char  new_path[MAX_PATH];
    char* ret = _fullpath(new_path, DisFN_cnv_path_2OS((char*)path), MAX_PATH );
    if ( (path_map=get_drv_letter_map( new_path )) )
    	{
      	strcpy( resolved, path_map );
      	strcat( resolved, new_path + 2 );
    	}
    else
      strcpy( resolved, new_path );

	if ( _access(resolved,0) < 0 )
		ret = 0;
	DisFN_trn_path_2DIS( resolved );
	}

	return ret; 
}

//----------------------------------------------
char* DisFN_real_slashedOSPATH( char * path )
{
    return DisFN_convert_forward_slashes(DisFN_realOSPATH(path));
}

//----------------------------------------------
char* DisFN_cnv_path_2OS( char * path )
{
  if (DisFN_isOSPATH() && path && *path== '/' 
      && ( DisFN_is_abs_path(path+1) || 
           (*(path+2)== ':' && strlen(path) == 3 && isalpha(*(path+1)) ) 
         ) 
     )
		return (char*)path+1;
	else
		return (char*)path;
}

//----------------------------------------------
char* DisFN_realOSPATH( char * path )
{
  static char szPATH[_MAX_PATH] ="";

  if ( !path || !*path || *path == '*' || (*path == '('&& *(path+1) == '*') )
    return path;

  if ( DisFN_is_abs_path((char*)path) && strlen(path) == 3 && DisFN_isOSPATH() )
  {
    strcpy(szPATH, (char*)path);
    *szPATH = toupper( *szPATH );
    *(szPATH+2) = '/';
  }
	else
	{
    char  new_path[MAX_PATH];
	  _fullpath(new_path, DisFN_cnv_path_2OS(path), MAX_PATH );

    szPATH[0] = toupper(*new_path);
    szPATH[1] = ':';
    szPATH[2] = '\0';

    WIN32_FIND_DATA FindFileData;
    char *pathBeg = new_path+3, *pathEnd, *pEND_PATH;

    while ( *pathBeg )
    {
      pEND_PATH= szPATH + strlen(szPATH);
      *pEND_PATH++ = '/'; *pEND_PATH = 0;
      pathEnd = strchr(pathBeg,'\\');
      if ( pathEnd )
        *pathEnd = 0;

      if ( strchr(pathBeg, '?') || strchr(pathBeg, '*') )
      {
        if ( pathEnd )
          *pathEnd++ = '\\';
        strcat( pEND_PATH, pathBeg );
        DisFN_convert_back_slashes( szPATH );
        break;
      }

      strcat( pEND_PATH, pathBeg );
      if ( pathEnd )
        *pathEnd++ = '\\';
      else
        pathEnd = pathBeg + strlen(pathBeg);
      
      ZeroMemory( &FindFileData, sizeof(FindFileData) );
      HANDLE h = FindFirstFile( szPATH, &FindFileData );
      if ((h != INVALID_HANDLE_VALUE) && (*FindFileData.cFileName)) 
      {
		   (void)FindClose(h);
       if ( strcmp( pEND_PATH, FindFileData.cFileName ) )
		      strcpy( pEND_PATH, FindFileData.cFileName ); 
       else
       {
         char szItemName[_MAX_PATH];
         strcpy( szItemName, FindFileData.cFileName );
         szPATH[strlen(szPATH)-1] = '?';
         ZeroMemory( &FindFileData, sizeof(FindFileData) );
         h = FindFirstFile( szPATH, &FindFileData );
         while ( stricmp( szItemName, FindFileData.cFileName ) && FindNextFile(h, &FindFileData) );
         (void)FindClose(h);
         strcpy( pEND_PATH, FindFileData.cFileName ); 
       }
      }  
      else
      {
      if ( pathEnd && *pathEnd ) {
        strcat( szPATH, "/" );
        strcat( szPATH, pathEnd );
        DisFN_convert_back_slashes( szPATH );
       }
      break;
    }
      
     pathBeg = pathEnd;
    }   // while ( *pathBeg )
  }

  return szPATH;
}

//----------------------------------------------
static char* DisFN_create_path_2DIS_internal( char * path, char* dis_path )
{
  if ( !path || !dis_path ) return 0;

  if ( DisFN_isOSPATH() && strlen(path) >= 2 && *(path+1) == ':' && isalpha(*path) ) {
      *dis_path = '/';
      strcpy( dis_path+1, path );
  } else
      strcpy( dis_path, path );

  return DisFN_convert_to_uniform_case( dis_path );
}

//----------------------------------------------
char* DisFN_create_path_2DIS_no_realpath( char * path, char** dis_path )
{
  if ( !path || !*path || !dis_path ) return 0;

  char* buf = path;
  *dis_path = 0;
  if ( DisFN_isOSPATH() && strlen(path) >= 2 && *(path+1) == ':' && isalpha(*path) ) {
      *dis_path = (char*)malloc(strlen(path) + 2);
      if ( *dis_path == 0) return 0;

      buf = *dis_path;
      buf[0] = '/';
      strcpy( buf+1, path );
      if ( g_DisFN_RealCase )
	  buf[1] = toupper( buf[1] );
      else
	  buf[1] = tolower( buf[1] );
  }

  return DisFN_convert_back_slashes( buf );
}

//----------------------------------------------
char* DisFN_create_path_2DIS( char * path, char** new_path )
{
  if ( !path || !path[0] || !new_path) return 0;

  *new_path = (char*)malloc(strlen(path) + 2);
  return DisFN_create_path_2DIS_internal( path, *new_path );
}

//----------------------------------------------
char* DisFN_trn_path_2DIS ( char * path )
{
  if ( !path || !path[0] ) return 0;
  char* old_path = (char*)malloc(strlen(path) + 1);
  if ( old_path )
  {
    strcpy( old_path, path );
    DisFN_create_path_2DIS_internal( old_path, path );
    free( old_path);
    return path;
  }
  return 0;
}

//----------------------------------------------
char* DisFN_convert_to_uniform_case( char* path )
{
  if ( !path || path[0] == 0 ) return 0;
  if ( !g_DisFN_RealCase ) // convert path to lowcase
  {
    for (char* p = path ; p && *p ; p++ )
      *p = tolower(*p);
    DisFN_convert_back_slashes(path);
  }
  else // find real case
  {
    char* os_path = DisFN_cnv_path_2OS( path );
    if ( isalpha(os_path[0]) && os_path[1] == ':' ) // absolute path?
    {
      DisFN_convert_forward_slashes(os_path);
      if (os_path[2] == 0 || os_path[2] != '\\' ) // the path looks like "c:" or "c:foo.c"
      {
        toupper(os_path[0]);
      } 
      else 
      {
        char* real_path = DisFN_realOSPATH( os_path );
        if ( strlen(os_path) >= strlen(real_path) )
          strcpy( os_path, real_path );
        else
          strcpy( os_path, real_path + strlen(real_path)-strlen(os_path) );
      }
    }
  }
  DisFN_convert_back_slashes(path);
  return path;
}

//----------------------------------------------
char* DisFN_convert_back_slashes( char * path )
{
	if ( path )
  {
  char *pslash;
	while ( (pslash=strchr(path, '\\')) )
		 *pslash = '/';
  }
	return path;
}

//----------------------------------------------
char* DisFN_convert_forward_slashes( char * path )
{
	if ( path )
  {
	char *pslash;
	while ( (pslash=strchr(path, '/')) )
		 *pslash = '\\';
	}
	return path;
}
