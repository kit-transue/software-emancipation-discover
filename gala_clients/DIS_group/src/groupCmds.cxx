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
//----------
//
// groupCmds.C - GROUPswApp methods that operate on Discover Groups.
//    Most of these commands take a groupHdrPtr as input and/or
// give a groupHdrPtr as output.
//
// This file should have its contents moved into GROUPswApp.C.
//
//----- Contents
//
// findAllGroups
// findRTLGroups
// importRTLGroups
// importPackageSession
// unionGroupMembers
// intersectGroupMembers
// subtractGroupMembers
//
//----------

// Galaxy Includes.
#include <vport.h>

// Client/Server communication includes.
#include "../../../clients/communications/include/Application.h"

#include "../include/GROUPswApp.h"
#include "../include/CreateGroup.h"
#include <RTL_externs.h>
#include <galaxy_undefs.h>    // Need to adjust some Galaxy defs.

// Discover includes.
#include <genError.h>
#include <xrefSymbol.h>
#include <db.h>
#include <projList.h>
#include <customize.h>
#include <messages.h>
#include <autosubsys.h>
#include <vpopen.h>    // Needed for vsystem.
#ifdef _WIN32
#include <windows.h>
static int spawn_with_new_console( const char * );
#endif
#include <shell_calls.h>

// Find all Groups and insert them into the given array.
vbool GROUPswApp::findAllGroups(symbolArr& allGroups)
{
    Initialize(GROUPswApp::findAllGroups);

    // Get all xrefs from the home project.
    projNodePtr homeProject = projNode::get_home_proj();
    Xref* homeXrefs = (homeProject) ? homeProject->get_xref() : NULL;
    if (homeXrefs) {
        // Get all groups from the home project.
        symbolArr groups;
        ddSelector sel;
        sel.add(DD_SUBSYSTEM);
        homeXrefs->add_symbols(groups, sel);
        symbolPtr groupSym;
        ForEachS(groupSym, groups) {
            if (groupSym.xrisnotnull() && groupSym.get_has_def_file()) {
                allGroups.insert_last(groupSym);
            }
        }
    }

    return(vTRUE);
}


// Find all RTL Groups and insert them into the given array.
vbool GROUPswApp::findRTLGroups(genString& groupNames)
{
    Initialize(GROUPswApp::findRTLGroups);

    groupNames = "";
    // -- load groups
    projNode *pr;
    for(int ii = 0; pr = projList::search_list->get_scan(ii); ++ii) {
        symbolArr modules;
        pr = pr->root_project();
        if (pr) {
    	    symbolPtr el;
    	    pr->search_modules_by_name(".rtl", modules);
    	    ForEachS(el, modules) {
    	        projModulePtr pm = checked_cast(projModule, el);
                if (pm) {
                    if (groupNames.length() > 0) groupNames += " ";
                    groupNames += el.get_name();
    	        }
    	    }
        }
    }

    return(vTRUE);
}



// Import the old-style rtl groups.
vbool GROUPswApp::importRTLGroups()
{
    Initialize(GROUPswApp::importRTLGroups);
    int status = vTRUE;

    // -- load groups
    projNode *pr;
    for(int ii = 0; pr = projList::search_list->get_scan(ii); ++ii) {
        symbolArr modules;
        pr = pr->root_project();
        if (pr) {
    	    symbolPtr el;
    	    pr->search_modules_by_name(".rtl", modules);
    	    ForEachS(el, modules) {
	        genString phys_name;
    	        projModulePtr pm = checked_cast(projModule, el);
                if (pm) {
                    pm->get_phys_filename(phys_name);
   		    importRTL((char *)phys_name);
    	        }
    	    }
        }
    }

    return(status);
}

vbool GROUPswApp::scanRTLDir(char * rtl_def_dir, genString& groupNames)
{
    Initialize(projectBrowser::importRTLDir);

    if (rtl_def_dir) {
	DIR * rtl_dir = opendir(rtl_def_dir);
	if (rtl_dir) {
	    for (struct dirent * entry=readdir(rtl_dir); entry!=0; entry=readdir(rtl_dir)) {
		char * fname = entry->d_name;
		if (fname) {
		    char * fname_dot = strrchr(fname, '.');
		    if (fname_dot) {
			if (!strcmp(fname_dot+1, "rtl")) {
			    genString gnm;
			    
			    gnm.printf("%s/%s", rtl_def_dir, fname);
			    if (fname = (char *)gnm) {
                                if (groupNames.length() > 0) groupNames += " ";
                                groupNames += fname;
			    }
			}
		    }
		}
	    }
	    closedir(rtl_dir);
	}
    }
    return(vTRUE);
}

vbool GROUPswApp::importRTLDir(char * rtl_def_dir)
{
    Initialize(projectBrowser::importRTLDir);

    if (rtl_def_dir) {
	DIR * rtl_dir = opendir(rtl_def_dir);
	if (rtl_dir) {
	    for (struct dirent * entry=readdir(rtl_dir); entry!=0; entry=readdir(rtl_dir)) {
		char * fname = entry->d_name;
		if (fname) {
		    char * fname_dot = strrchr(fname, '.');
		    if (fname_dot) {
			if (!strcmp(fname_dot+1, "rtl")) {
			    genString gnm;
			    
			    gnm.printf("%s/%s", rtl_def_dir, fname);
			    if (fname = (char *)gnm) {
				OSapi_printf(TXT("Importing group from %s ...\n"), fname);
				importRTL(fname);
			    }
			}
		    }
		}
	    }
	    closedir(rtl_dir);
	}
    }
    return(vTRUE);
}


//get RTL_name given a filename
vbool GROUPswApp::extractRTLName(char *filename, genString &name)
{
    name = "<no_name>";
    if (filename) {
	char * curr = strrchr(filename, '/');

	if (curr)
	    curr++;
	else
	    curr = filename;
	char * suff = strstr(curr, ".rtl");
	if (suff) {
	    if (curr != suff) {
		for (name = (char *)0; curr!=suff; curr++)
		    name += (*curr);
	    }
	} else
	    name = curr;
    }
    return(vTRUE);
}

#define GROUP_RD_BUFFER_SIZE 1024


// Get the internal value for name of the kind of entity
ddKind GROUPswApp::ddKind_internal_val(char *obj_kind)
{
    static char  *ch_kinds[] = {"PRO", "MOD", "CLA", "ENU", "MAC", "FUN",
			        "VAR", "TYP", "UNI", "SUB", "PAR", "FIE", "TEM",
				"REL"};
    static ddKind dd_kinds[] = {DD_PROJECT, DD_MODULE, DD_CLASS, DD_ENUM,
                                DD_MACRO, DD_FUNC_DECL, DD_VAR_DECL, DD_TYPEDEF,
                                DD_UNION, DD_SUBSYSTEM, DD_PARAM_DECL, DD_FIELD,
				DD_TEMPLATE, DD_RELATION};
    static int ch_num = sizeof(ch_kinds)/sizeof(ch_kinds[0]);
    int ii;

    for(ii=0; ii<ch_num; ii++)
        if (strcmp(obj_kind, ch_kinds[ii]) == 0)
	    return(dd_kinds[ii]);
    return(DD_UNKNOWN);
}



#ifdef _WIN32
int spawn_with_new_console( const char *cmd )
{
  int nRet = -1;
  char *pszCmd = OSapi_strdup( cmd );
  if( pszCmd )
  {
    PROCESS_INFORMATION pi;
    STARTUPINFO si;
    ZeroMemory( &si, sizeof( si ) );
    ZeroMemory( &pi, sizeof( pi ) );
    si.cb = sizeof( si );
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;

    BOOL fResult = CreateProcess( NULL, pszCmd, NULL, NULL, FALSE, /*don't inherit handles*/ 
                           CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi );
    if( fResult )
    {
      WaitForSingleObject( pi.hProcess, INFINITE ); //wait for termination
      DWORD nExit;
      if( GetExitCodeProcess( pi.hProcess, &nExit ) )
        nRet = nExit;
      CloseHandle( pi.hProcess );
      CloseHandle( pi. hThread );
    }
    free( pszCmd );
  }
  return nRet;
}
#endif

//----------     end of groupCmds.C     ----------//
