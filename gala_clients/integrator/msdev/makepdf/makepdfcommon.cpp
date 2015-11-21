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
#include	"MakePDF.h"
#include	"MapNet.h"

/***********************************************************************************/
//		COMMON
/***********************************************************************************/
static	char					gszThisComputerName[_MAX_PATH+1];
static	char					gcSlash				= '\\';
static	char*					gpszTargetOS	= NULL;
static	char*					gpszPlatform	= NULL;
static	char*					gpszTarget		= NULL;

//---------------------------------------------------
//
void	SetCommonVar( char* pszOS, char* pszPlatform, char* pszTarget )
{
	DWORD nLen = sizeof(gszThisComputerName);
	GetComputerName(gszThisComputerName, &nLen);
	gpszTargetOS	= pszOS;
	gpszPlatform	= pszPlatform;
	gpszTarget		= pszTarget;
	if ( stricmp( gpszTargetOS, "UNIX" ) == 0 )
		gcSlash = '/';
}

char*	GetThisComputerName		()	{ return gszThisComputerName; }
char*	GetPlatform						()	{ return gpszPlatform; }
char*	GetTarget							()	{ return gpszTarget; }
char*	GetTargetOS						()	{ return gpszTargetOS; }
char	GetSlash							()	{ return gcSlash; }

//---------------------------------------------------
//
int	CommonFilePath( char* pszFName1, int nFName1Len, char* pszFName2 )
{
		char		szDrive1	[_MAX_PATH	+1], szDrive2		[_MAX_PATH	+1];
		char		szDir1		[ MAX_PATH	+1], szDir2			[ MAX_PATH	+1];
		char		szFname		[ MAX_PATH	+1];
		char		szExt			[ _MAX_EXT		+1];
		char		*pszSlash, *pszDir1, *pszDir2;
		int			nCommonPart;
		char		szFNameCommon[_MAX_PATH];

    //------------------------------
		szFNameCommon[0] = '\0';
   _splitpath( pszFName1, szDrive1, szDir1, szFname, szExt );
     if ( szDir1[0] == '\\' )
     {
       if ( szDir1[1] == '\\' )
       {
         pszSlash = strchr( &(szDir1[2]), '\\' );
         if ( pszSlash )
         {
           *pszSlash = '\0';
           strcpy( szDrive1, szDir1 );
           *pszSlash = '\\';
           memcpy( szDir1, pszSlash, strlen(pszSlash)+1 );
           pszDir1 = &( szDir1[1] );
         }
         else
         	 return 0;
       }
       else
         pszDir1 = &( szDir1[1] );
     }
     else
       pszDir1 = &( szDir1[0] );

    //------------------------------
   _splitpath( pszFName2, szDrive2, szDir2, szFname, szExt );
     if ( szDir2[0] == '\\' )
     {
       if ( szDir2[1] == '\\' )
       {
         pszSlash = strchr( &(szDir2[2]), '\\' );
         if ( pszSlash )
         {
           *pszSlash = '\0';
           strcpy( szDrive2, szDir2 );
           *pszSlash = '\\';
           memcpy( szDir2, pszSlash, strlen(pszSlash)+1 );
           pszDir2 = &( szDir2[1] );
         }
         else
         	 return 0;
       }
       else
         pszDir2 = &( szDir2[1] );
     }
     else
       pszDir2 = &( szDir2[0] );


    //------------------------------
		if ( stricmp(szDrive1,szDrive2) == 0 )
		{
			if ( szDrive1[0] )
			{
				strcpy( szFNameCommon, szDrive1 );
				strcat( szFNameCommon, "\\" );
			}

			szDir1[nFName1Len-strlen(szDrive1)] = '\0';

			while ( pszDir1 && (pszSlash= strchr( pszDir1, '\\')) != NULL )
			{
				nCommonPart = pszSlash-pszDir1;
				if ( memicmp( pszDir1, pszDir2, nCommonPart ) == 0 )
				{
					*pszSlash = '\0';
					strcat( szFNameCommon, pszDir1);
					strcat( szFNameCommon, "\\" );
					pszDir2 += nCommonPart +1;
					pszDir1 += nCommonPart +1;
				}
				else
					break;
			}	// while ( pszDir1 && (pszSlash= strchr( pszDir1, '\\')) )

		}	// if ( stricmp(szDrive1,szDrive2) == 0 )

		return strlen(szFNameCommon);
}
//---------------------------------------------------
//
void	InternalConvertFileName( char* pszName, char* pszNewName )
{
	int			nNameLen;
	*pszNewName = '\0';
	if ( pszName && (nNameLen = strlen(pszName)) != NULL )
		{
		 if ( *(pszName + nNameLen-1) == '\\' )
			 *(pszName + nNameLen-1) = '\0';
		 if ( *pszName != '\0' )
		 {
			 int	nRez = LocalMapped2UnixNetName	( pszName, pszNewName, _MAX_PATH );
			 //-------------------------------------------------------------
			 if ( GetSlash() != '\\' )
			 {
				 if ( nRez < 0 )
					{
					 strcpy( pszNewName, pszName );
					 ChangeSlashForward( pszNewName );
					}
			 }
			 //-------------------------------------------------------------
			 else
				{
					strcpy( pszNewName, pszName );
					ChangeSlashForward( pszNewName );
				}
		 }	// if ( *pszName != '\0' ) 
		}	// if ( pszName && (nNameLen = strlen(pszName)) )
}

char*		GetPassword(HINSTANCE, HWND, char* )
{
	return NULL;
}
