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
#include "pdustring.h"
#include <string.h>
#include <stdio.h>
#include <vport.h>				
#include vstrHEADER
#include <tcl.h>
#include "gString.h"
#include "Application.h"
#define CLIENT
#include "../../../paraset/driver/pmod_server/DIS_inform.def"
#undef CLIENT
#include "machdep.h"

int build_pmod_servers_list( Tcl_Interp* interp, const char *reg_exp )
{
  int nRet = 0;
  char pszServiceName[ 512 ];
  char pszNewItem[ 1024 ];
  char clients_plural[ 10] ;
  gString result = (const vchar *)"";
  Application::buildServiceList( (const vchar *)"DISCOVER", 
                                  (const vchar *)reg_exp );
  int nSize = Application::getServiceListSize();
  for( int i=0; i<nSize; i++ )
  {
    const vchar *title = Application::getServiceListAttributeValue(i, (const vchar*)"Title");

    const char *pColon = NULL;
    if( title )
      pColon = strchr( (const char *)title, ':' );
    if( pColon )
    {
      strcpy( pszServiceName, pColon + 1 );
      if( strlen( pszServiceName ) )
      {
        Application *pmod_inform = new Application;
        if( pmod_inform )
        {
          if( !pmod_inform->connect("inform", pszServiceName) && pmod_inform->mySession())
          {
            char *reg_time = NULL;
            rcall_inform_get_time( pmod_inform, reg_time );
            int nClients = rcall_inform_get_numclients( pmod_inform );
            if( reg_time && nClients >= 0 )
            {
              int nSize = strlen( reg_time );
              if( nSize > 0 && reg_time[nSize - 1]=='\n' )
                reg_time[nSize - 1] = '\0';
			  //this is here to fix the '0 clients 1 clients 2 clients bug..
				
				pdstrcpy(clients_plural, "clients");
				if (nClients==1)
					clients_plural[pdstrlen(clients_plural)-1]=NULL;
              sprintf( pszNewItem, " {{%s} {%d %s. Started %s}}", 
                       pszServiceName, nClients, clients_plural, reg_time );
              result += (vchar *)pszNewItem;
            }
            if( reg_time )
              OSapi_free( reg_time );
          }
          delete pmod_inform;
        }
      }
    }
  }
//  result += (const vchar *)"}";
  Tcl_SetResult( interp, (char *)result.str(), TCL_VOLATILE );
  return nRet;
}

/****************************************************************
* type: 0 - get default user pdf
*       1 - get default user prefs
*       2 - get any local file which name is passed in pszPmodFile
*****************************************************************/
int download_file_from_pmod( int type, const char *pszServiceName, const char *pszPmodFile, 
                            const char *pszSaveAs, int &checksum  )
{
  int nRet = 0;
  Application *pmod_inform = new Application;
  if( pmod_inform )
  {
    if( !pmod_inform->connect("inform", pszServiceName) && pmod_inform->mySession())
    {
      vstr *file_contents = NULL;
      rcall_inform_get_file( pmod_inform, type, (char *)pszPmodFile, file_contents, checksum );
      if( file_contents )
      {
        FILE *new_file = OSapi_fopen( pszSaveAs, "w" );
        if( new_file )
        {
          int nLen = vstrGetSize( file_contents ); 
          nRet = (OSapi_fwrite((char *)file_contents, sizeof(char), nLen, new_file) == nLen); 
          OSapi_fclose( new_file );
        }
        vstrDestroy( file_contents );
      }
    }
    delete pmod_inform;
  }
  return nRet;
}

int shutdown_pmodserver( const char *pszServiceName )
{
  int nRet = 0;
  Application *pmod_inform = new Application;
  if( pmod_inform )
  {
    if( !pmod_inform->connect("inform", pszServiceName) && pmod_inform->mySession())
    {
      rcall_inform_pmod_shutdown( pmod_inform );
      nRet = 1;
    }
    delete pmod_inform;
  }
  return nRet;
}

int get_pmod_pdf_checksum( const char *pszServiceName )
{
  int nRet = 0;
  Application *pmod_inform = new Application;
  if( pmod_inform )
  {
    if( !pmod_inform->connect("inform", pszServiceName) && pmod_inform->mySession())
    {
      nRet = rcall_inform_get_pdf_checksum( pmod_inform );
    }
    delete pmod_inform;
  }
  return nRet;
}


void demarshall_dyn_vstrptr(dyn_vstrptr* x, vscrap* s) 
{ 
    if (s) { 
        *x = vstrCloneScribed(vscrapScribeString(s));delete s;
    } else { 
        *x = 0;
    } 
}
