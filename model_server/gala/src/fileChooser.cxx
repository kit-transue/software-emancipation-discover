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

#include <vport.h>
#include vfilechsrHEADER
#include vapplicationHEADER
#include vresourceHEADER
#include vliststrHEADER
#include vnameHEADER

#include <tcl.h>

#include <gfileChooser.h>
#ifndef _generic_h
   #include <ggeneric.h>
#endif
#ifndef _ggenWild_h
   #include <ggenWild.h>
#endif
#ifndef _gglobalFuncs_h
   #include <gglobalFuncs.h>
#endif
#ifndef _interp_h
   #include <ginterp.h>    // Needed for g_global_interp.
#endif



void fileChooser::SetCallers (int viewer, 
                              int view, 
                              vchar* newTitle, 
                              vchar *newPrompt,
			      const vchar* pathVar, 
                              const vchar* filterVar, 
                              const vchar* defaultFilterVar,
							  int multiSels)
{
    viewer_ = viewer;
    view_ = view;

    if (newTitle) SetTitle (newTitle);
    if (newPrompt) SetPromptName (newPrompt);

    pathVar_ = (vchar*) pathVar;
    filterVar_ = (vchar*) filterVar;
    const vchar* defaultPath =  (const vchar*) Tcl_GetVar (g_global_interp, 
					                   (char*)pathVar, 
                                                           TCL_GLOBAL_ONLY);
    if (defaultPath) {
	vfsPath* newPath = new vfsPath;
    	newPath->Set(defaultPath);
        newPath->ExpandVariables();  // Expand any variables that might be in the path

    	SetDirectory((const vfsPath*) newPath);
    }

    // Load in default system filters

    vresource res;

    const vchar* newFilter =  (const vchar*) Tcl_GetVar (g_global_interp, 
							 (char*)filterVar, 
                                                         TCL_GLOBAL_ONLY);
    const vchar* defaultFilter =  (const vchar*) Tcl_GetVar (g_global_interp, 
					                     (char*)defaultFilterVar, 
                                                             TCL_GLOBAL_ONLY);
#ifndef _WIN32
    if (getResource("FilterList", &res) ) {
	vliststr* filterList = new vliststr(res);
	for (int x =0; x < filterList->GetRowCount(); x++) {
            vchar wildCard[1024], my_title[1024];
	    vcharCopyScribed(filterList->ScribeCellValue(x,0), my_title);
	    vcharCopyScribed(filterList->ScribeCellValue(x,1), wildCard);
	    AddFilterByPattern(vnameInternGlobal(my_title), my_title,
			   glob_to_regexp(wildCard, 1), vFALSE);
	}
    }
    if (newFilter) {
	AddFilterByPattern(vnameInternGlobal((vchar*)newFilter), (vchar*)"Local filter", 
			   glob_to_regexp(newFilter, 1), vTRUE);
    }

    if (defaultFilter) SetActiveFilter (vnameInternGlobal(defaultFilter));
#else
    if (getResource("SystemFilterList", &res) )
    {
      const vname *loadFlt = NULL;
      const vname *dfltFlt = NULL;
      if(newFilter)
        loadFlt = vnameInternGlobal((vchar *)newFilter);
      if(defaultFilter)
        dfltFlt = vnameInternGlobal((vchar *)defaultFilter);
      AddFilterBySystemType(res, loadFlt, dfltFlt); 
    }
#endif /*_WIN32*/	
	multipleSelections = multiSels;

	if (multipleSelections)
		SetSelectMethod (vfilechsrMULTIPLE);
}

int fileChooser::HookApplyFile(vfsPath *path)
{
    if (multipleSelections) {
    	if (returnValue_) returnValue_ = vstrAppend (returnValue_, (vchar*)" {");
	    else returnValue_ = vstrClone ((vchar*)"{");

        returnValue_ = vstrAppend (returnValue_, (vchar*)path->CloneString());
        returnValue_ = vstrAppend (returnValue_, (vchar*)"}");
	} else 
        returnValue_ = vstrClone ((vchar*)path->CloneString());

    Tcl_SetVar (g_global_interp, (char*) pathVar_, (char*) GetDirectory()->CloneString(), TCL_GLOBAL_ONLY);
    if (GetActiveFilter()) Tcl_SetVar (g_global_interp, (char*) filterVar_, (char*) GetActiveFilter(), TCL_GLOBAL_ONLY);   // Warning!  GetActiveFilter does not seem to work!!

    return vTRUE;
}


static vstr * volatile rememberedFile = NULL;

static int RememberFile( vfilechsr *chooser, vfsPath *path )
{
  int canClose = vTRUE;
  rememberedFile = path->CloneString();
  return canClose;
}

const char *choose_file_simple( const char *title )
{
  const char *pszRet = NULL;
  vfilechsr *chsr = new vfilechsr;
  if( chsr )
  {
    if( rememberedFile )
    {
      vstrDestroy( rememberedFile );
      rememberedFile = NULL;
    }
    chsr->SetType( vfilechsrGET );
    chsr->SetHookApplyFileFunc( RememberFile );
    if( title )
      chsr->SetTitle( (const vchar *)title );
    //chsr->Place(vwindow::GetRoot(), vrectPLACE_CENTER, vrectPLACE_CENTER);
    chsr->Process();
    pszRet = (const char *)rememberedFile;
    delete chsr;
  }
  return pszRet;
}
